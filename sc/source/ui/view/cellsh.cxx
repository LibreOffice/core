/*************************************************************************
 *
 *  $RCSfile: cellsh.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-14 19:29:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#include "scitems.hxx"

#include <svtools/slstitm.hxx>
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>
#include <svtools/moduleoptions.hxx>
#include <offmgr/sbasltid.hrc>
#include <sot/formats.hxx>
#include <svx/hlnkitem.hxx>
#include <sfx2/app.hxx>
//#include <sfx2/objitem.hxx>
#include <svx/langitem.hxx>
#include <vcl/system.hxx>

#include "cellsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "globstr.hrc"
#include "transobj.hxx"
#include "drwtrans.hxx"

//------------------------------------------------------------------

#define ScCellShell
#define CellMovement
#include "scslots.hxx"

#define SearchSettings
#include <svx/svxslots.hxx>

TYPEINIT1( ScCellShell, ScFormatShell );

SFX_IMPL_INTERFACE(ScCellShell, ScFormatShell , ScResId(SCSTR_CELLSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_FORMAT));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, ScResId(RID_OBJECTMENU_CELLS));
    SFX_POPUPMENU_REGISTRATION(ScResId(RID_POPUP_CELLS));
}


ScCellShell::ScCellShell(ScViewData* pData) :
    ScFormatShell(pData)
{
    SetHelpId(HID_SCSHELL_CELLSH);
    SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("Cell")));
}

ScCellShell::~ScCellShell()
{
}

//------------------------------------------------------------------

void ScCellShell::GetBlockState( SfxItemSet& rSet )
{
    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    ScRange aMarkRange;
    BOOL bSimpleArea = GetViewData()->GetSimpleArea( aMarkRange, TRUE );    //! macht TRUE aerger ?
    BOOL bOnlyNotBecauseOfMatrix;
    BOOL bEditable = pTabViewShell->SelectionEditable( &bOnlyNotBecauseOfMatrix );
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    USHORT nCol1, nCol2, nRow1, nRow2;
    nCol1 = aMarkRange.aStart.Col();
    nRow1 = aMarkRange.aStart.Row();
    nCol2 = aMarkRange.aEnd.Col();
    nRow2 = aMarkRange.aEnd.Row();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        BOOL bDisable = FALSE;
        BOOL bNeedEdit = TRUE;      // muss Selektion editierbar sein?
        switch ( nWhich )
        {
            case FID_FILL_TO_BOTTOM:    // Fuellen oben/unten
            case FID_FILL_TO_TOP:       // mind. 2 Zeilen markiert?
                bDisable = (!bSimpleArea) || (nRow1 == nRow2);
                if ( !bDisable && bEditable )
                {   // Matrix nicht zerreissen
                    if ( nWhich == FID_FILL_TO_BOTTOM )
                        bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol2, nRow1, rMark );    // erste Zeile
                    else
                        bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow2, nCol2, nRow2, rMark );    // letzte Zeile
                }
                break;

            case FID_FILL_TO_RIGHT:     // Fuellen links/rechts
            case FID_FILL_TO_LEFT:      // mind. 2 Spalten markiert?
                bDisable = (!bSimpleArea) || (nCol1 == nCol2);
                if ( !bDisable && bEditable )
                {   // Matrix nicht zerreissen
                    if ( nWhich == FID_FILL_TO_RIGHT )
                        bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark );    // erste Spalte
                    else
                        bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol2, nRow1, nCol2, nRow2, rMark );    // letzte Spalte
                }
                break;

            case FID_FILL_SERIES:       // Block fuellen
            case SID_OPENDLG_TABOP:     // Mehrfachoperationen, mind. 2 Zellen markiert?
                if (pDoc->GetChangeTrack()!=NULL &&nWhich ==SID_OPENDLG_TABOP)
                    bDisable = TRUE;
                else
                    bDisable = (!bSimpleArea) || (nCol1 == nCol2 && nRow1 == nRow2);

                if ( !bDisable && bEditable && nWhich == FID_FILL_SERIES )
                {   // Matrix nicht zerreissen
                    bDisable = pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol2, nRow1, rMark ) // erste Zeile
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow2, nCol2, nRow2, rMark ) // letzte Zeile
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol1, nRow1, nCol1, nRow2, rMark ) // erste Spalte
                        ||  pDoc->HasSelectedBlockMatrixFragment(
                            nCol2, nRow1, nCol2, nRow2, rMark );    // letzte Spalte
                }
                break;

            case SID_CUT:               // Ausschneiden,
            case FID_INS_ROW:           // Zeilen einfuegen
            case FID_INS_COLUMN:        // Spalten einfuegen
            case FID_INS_CELL:          // Zellen einfuegen, nur einf. Selektion
            case FID_INS_CELLSDOWN:
            case FID_INS_CELLSRIGHT:
                bDisable = (!bSimpleArea);
                break;

            case SID_COPY:                      // Kopieren
                bDisable = (!bSimpleArea);
                // nur wegen Matrix nicht editierbar? Matrix nicht zerreissen
                //! schlaegt nicht zu, wenn geschuetzt UND Matrix, aber damit
                //! muss man leben.. wird in Copy-Routine abgefangen, sonst
                //! muesste hier nochmal Aufwand getrieben werden
                if ( !(!bEditable && bOnlyNotBecauseOfMatrix) )
                    bNeedEdit = FALSE;          // erlaubt, wenn geschuetzt/ReadOnly
                break;

            case SID_AUTOFORMAT:        // Autoformat, mind. 3x3 selektiert
                bDisable =    (!bSimpleArea)
                           || ((nCol2 - nCol1) < 2) || ((nRow2 - nRow1) < 2);
                break;

            case SID_OPENDLG_CONDFRMT :
            case FID_CONDITIONAL_FORMAT :
            case SID_CELL_FORMAT_RESET :
            case FID_CELL_FORMAT :
                // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
                if ( !bEditable && bOnlyNotBecauseOfMatrix )
                    bNeedEdit = FALSE;
                break;
        }
        if (!bDisable && bNeedEdit && !bEditable)
            bDisable = TRUE;

        if (bDisable)
            rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

//  Funktionen, die je nach Cursorposition disabled sind
//  Default:
//      SID_INSERT_POSTIT, SID_CHARMAP, SID_OPENDLG_FUNCTION

void ScCellShell::GetCellState( SfxItemSet& rSet )
{
    ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();
    ScAddress aCursor( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
                        GetViewData()->GetTabNo() );

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        BOOL bDisable = FALSE;
        BOOL bNeedEdit = TRUE;      // muss Cursorposition editierbar sein?
        switch ( nWhich )
        {
            case SID_THESAURUS:
                {
                    CellType eType = pDoc->GetCellType( aCursor );
                    bDisable = ( eType != CELLTYPE_STRING && eType != CELLTYPE_EDIT);
                    if (!bDisable)
                    {
                        //  test for available languages
                        USHORT nLang = ScViewUtil::GetEffLanguage( pDoc, aCursor );
                        bDisable = !ScModule::HasThesaurusLanguage( nLang );
                    }
                }
                break;
            case SID_OPENDLG_FUNCTION:
                {
                    ScMarkData aMarkData=GetViewData()->GetMarkData();
                    ScRange aRange;
                    aMarkData.GetMarkArea(aRange);
                    if(aMarkData.IsMarked())
                    {
                        if (!pDoc->IsBlockEditable( aCursor.Tab(), aRange.aStart.Col(),aRange.aStart.Row(),
                                            aRange.aEnd.Col(),aRange.aEnd.Row() ))
                        {
                            bDisable = TRUE;
                        }
                        bNeedEdit=FALSE;
                    }

                }
                break;
        }
        if (!bDisable && bNeedEdit)
            if (!pDoc->IsBlockEditable( aCursor.Tab(), aCursor.Col(),aCursor.Row(),
                                        aCursor.Col(),aCursor.Row() ))
                bDisable = TRUE;
        if (bDisable)
            rSet.DisableItem(nWhich);
        nWhich = aIter.NextWhich();
    }
}

//  Einfuegen, Inhalte einfuegen

void __EXPORT ScCellShell::GetClipState( SfxItemSet& rSet )
{
    BOOL bDisable = TRUE;

// SID_PASTE
// FID_PASTE_CONTENTS

    if ( ScTransferObj::GetOwnClipboard() || ScDrawTransferObj::GetOwnClipboard() )
        bDisable = FALSE;
    else
    {
        SvDataObjectRef pClipObj = SvDataObject::PasteClipboard();
        if (pClipObj.Is())
        {
            const SvDataTypeList& rTypeLst = pClipObj->GetTypeList();

            if( rTypeLst.Get( FORMAT_BITMAP ) ||
                rTypeLst.Get( FORMAT_GDIMETAFILE ) ||
                rTypeLst.Get( SOT_FORMATSTR_ID_SVXB ) ||
                rTypeLst.Get( FORMAT_PRIVATE ) ||
                rTypeLst.Get( FORMAT_RTF ) ||
                rTypeLst.Get( SOT_FORMATSTR_ID_EMBED_SOURCE ) ||
                rTypeLst.Get( SOT_FORMATSTR_ID_LINK_SOURCE ) ||
                rTypeLst.Get( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE ) ||
                rTypeLst.Get( SOT_FORMATSTR_ID_LINK_SOURCE_OLE ) ||
                ScImportExport::IsFormatSupported( pClipObj ) )
                bDisable = FALSE;
        }
    }

    //  Zellschutz / Multiselektion

    if (!bDisable)
    {
        USHORT nCol = GetViewData()->GetCurX();
        USHORT nRow = GetViewData()->GetCurY();
        USHORT nTab = GetViewData()->GetTabNo();
        ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();
        if (!pDoc->IsBlockEditable( nTab, nCol,nRow, nCol,nRow ))
            bDisable = TRUE;
        ScMarkData& rMark = GetViewData()->GetMarkData();
        if (rMark.IsMultiMarked())
            bDisable = TRUE;
    }

    if (bDisable)
    {
        rSet.DisableItem( SID_PASTE );
        rSet.DisableItem( FID_PASTE_CONTENTS );
    }
}

//  only SID_HYPERLINK_GETLINK:

void ScCellShell::GetHLinkState( SfxItemSet& rSet )
{
    //  always return an item (or inserting will be disabled)
    //  if the cell at the cursor contains only a link, return that link

    SvxHyperlinkItem aHLinkItem;
    if ( !GetViewData()->GetView()->HasBookmarkAtCursor( &aHLinkItem ) )
    {
        //! put selected text into item?
    }

    rSet.Put(aHLinkItem);
}

void ScCellShell::GetState(SfxItemSet &rSet)
{
    // removed: SID_BORDER_OBJECT (old Basic)

    ScTabViewShell* pTabViewShell   = GetViewData()->GetViewShell();
    BOOL bOle = pTabViewShell->GetViewFrame()->ISA(SfxInPlaceFrame);
    BOOL bTabProt = GetViewData()->GetDocument()->IsTabProtected(GetViewData()->GetTabNo());
    SfxApplication* pSfxApp = SFX_APP();
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScMarkData& rMark       = pViewData->GetMarkData();
    USHORT      nPosX       = pViewData->GetCurX();
    USHORT      nPosY       = pViewData->GetCurY();
    USHORT      nTab        = pViewData->GetTabNo();
    USHORT      nMyId       = 0;

    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nTabSelCount = rMark.GetSelectCount();



    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch ( nWhich )
        {
            case SID_OPENDLG_CHART:
                if ( bOle || bTabProt || !SvtModuleOptions().IsChart() )
                    rSet.DisableItem( nWhich );
                break;

            case SID_DETECTIVE_REFRESH:
                if (!pDoc->HasDetectiveOperations())
                    rSet.DisableItem( nWhich );
                break;

            case SID_RANGE_ADDRESS:
                {
                    ScRange aRange;
                    if ( pViewData->GetSimpleArea( aRange, TRUE ) )
                    {
                        String aStr;
                        USHORT nFlags = SCA_VALID | SCA_TAB_3D;
                        aRange.Format(aStr,nFlags,pDoc);
                        rSet.Put( SfxStringItem( nWhich, aStr ) );
                    }
                }
                break;

            case SID_RANGE_NOTETEXT:
                {
                    USHORT nNoteCol, nNoteRow, nNoteTab;

                    //  #43343# immer Cursorposition
#if 0
                    ScRange aRange;
                    if (GetViewData()->GetSimpleArea(aRange,TRUE))
                    {
                        nNoteCol = aRange.aStart.Col();
                        nNoteRow = aRange.aStart.Row();
                        nNoteTab = aRange.aStart.Tab();
                    }
                    else        // Cursor bei Mehrfachselektion
#endif
                    {
                        nNoteCol = nPosX;
                        nNoteRow = nPosY;
                        nNoteTab = nTab;
                    }

                    String aNoteText;
                    ScPostIt aNote;
                    if ( pDoc->GetNote( nNoteCol, nNoteRow, nNoteTab, aNote ) )
                        aNoteText = aNote.GetText();

                    rSet.Put( SfxStringItem( nWhich, aNoteText ) );
                }
                break;

            case SID_RANGE_ROW:
                rSet.Put( SfxUInt16Item( nWhich, nPosY+1 ) );
                break;

            case SID_RANGE_COL:
                rSet.Put( SfxUInt16Item( nWhich, nPosX+1 ) );
                break;

            case SID_RANGE_TABLE:
                rSet.Put( SfxUInt16Item( nWhich, nTab+1 ) );
                break;

            case SID_RANGE_VALUE:
                {
                    double nValue;
                    pDoc->GetValue( nPosX, nPosY, nTab, nValue );
                    rSet.Put( ScDoubleItem( nWhich, nValue ) );
                }
                break;

            case SID_RANGE_FORMULA:
                {
                    String aString;
                    pDoc->GetFormula( nPosX, nPosY, nTab, aString );
                    if( aString.Len() == 0 )
                    {
                        pDoc->GetInputString( nPosX, nPosY, nTab, aString );
                    }
                    rSet.Put( SfxStringItem( nWhich, aString ) );
                }
                break;

            case SID_RANGE_TEXTVALUE:
                {
                    String aString;
                    pDoc->GetString( nPosX, nPosY, nTab, aString );
                    rSet.Put( SfxStringItem( nWhich, aString ) );
                }
                break;

            case SID_STATUS_SELMODE:
                {
                    /* 0: STD   Click hebt Sel auf
                     * 1: ER    Click erweitert Selektion
                     * 2: ERG   Click definiert weitere Selektion
                     */
                    USHORT nMode = pTabViewShell->GetLockedModifiers();

                    switch ( nMode )
                    {
                        case KEY_SHIFT: nMode = 1;  break;
                        case KEY_MOD1:  nMode = 2;  break; // Control-Taste
                        case 0:
                        default:
                            nMode = 0;
                    }

                    rSet.Put( SfxUInt16Item( nWhich, nMode ) );
                }
                break;

            case SID_STATUS_DOCPOS:
                {
                    String  aStr( ScGlobal::GetRscString( STR_TABLE ) );

                    aStr += ' ';
                    aStr += String::CreateFromInt32( nTab + 1 );
                    aStr.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " / " ));
                    aStr += String::CreateFromInt32( nTabCount );
                    rSet.Put( SfxStringItem( nWhich, aStr ) );
                }
                break;

            //  Summe etc. mit Datum/Zeit/Fehler/Pos&Groesse zusammengefasst

            case SID_TABLE_CELL:
            case SID_ATTR_POSITION:
            case SID_ATTR_SIZE:
                {
                    //  Testen, ob Fehler unter Cursor
                    //  (nicht pDoc->GetErrCode, um keine zirkulaeren Referenzen auszuloesen)

                    // In interpreter may happen via rescheduled Basic
                    if ( pDoc->IsInInterpreter() )
                        rSet.Put( SfxStringItem( nWhich,
                            String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("...")) ) );
                    else
                    {
                        USHORT nErrCode = 0;
                        ScBaseCell* pCell;
                        pDoc->GetCell( nPosX, nPosY, nTab, pCell );
                        if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
                        {
                            ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
                            if (!pFCell->IsRunning())
                                nErrCode = pFCell->GetErrCode();
                        }

                        if ( nErrCode > 0 )
                            rSet.Put( SfxStringItem( nWhich,
                                ScGlobal::GetLongErrorString( nErrCode ) ) );
                        else
                        {
                            String aFuncStr;
                            if ( pTabViewShell->GetFunction( aFuncStr ) )
                                rSet.Put( SfxStringItem( nWhich, aFuncStr ) );
                        }
                    }
                }
                break;

            case SID_DATA_SELECT:
                //  ab 506f wird die ganze Spalte angesehen
                if ( !pDoc->HasStringCells(ScRange(nPosX,0,nTab, nPosX,MAXROW,nTab)) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_STATUS_SUM:
                {
                    String aFuncStr;
                    if ( pTabViewShell->GetFunction( aFuncStr ) )
                        rSet.Put( SfxStringItem( nWhich, aFuncStr ) );
                }
                break;

            case FID_MERGE_ON:
                if ( pDoc->GetChangeTrack() || !pTabViewShell->TestMergeCells() )
                    rSet.DisableItem( nWhich );
                break;

            case FID_MERGE_OFF:
                if ( pDoc->GetChangeTrack() || !pTabViewShell->TestRemoveMerge() )
                    rSet.DisableItem( nWhich );
                break;

            case FID_INS_ROWBRK:
                if ( nPosY==0 || (pDoc->GetRowFlags(nPosY,nTab) & CR_MANUALBREAK) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_INS_COLBRK:
                if ( nPosX==0 || (pDoc->GetColFlags(nPosX,nTab) & CR_MANUALBREAK) )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_ROWBRK:
                if ( nPosY==0 || (pDoc->GetRowFlags(nPosY,nTab) & CR_MANUALBREAK)==0 )
                    rSet.DisableItem( nWhich );
                break;

            case FID_DEL_COLBRK:
                if ( nPosX==0 || (pDoc->GetColFlags(nPosX,nTab) & CR_MANUALBREAK)==0 )
                    rSet.DisableItem( nWhich );
                break;

            case FID_FILL_TAB:
                if ( nTabSelCount < 2 )
                    rSet.DisableItem( nWhich );
                break;

            case SID_SELECT_SCENARIO:
                {
                    ScDocument* pDoc = GetViewData()->GetDocument();
                    USHORT      nTab = GetViewData()->GetTabNo();
                    List        aList;

                    Color   aDummyCol;
                    USHORT  nDummyFlags;

                    if ( !pDoc->IsScenario(nTab) )
                    {
                        String aStr;
                        USHORT nScTab = nTab + 1;

                        while ( pDoc->IsScenario(nScTab) )
                        {
                            pDoc->GetName( nScTab, aStr );
                            aList.Insert( new String( aStr ), LIST_APPEND );
                            pDoc->GetScenarioData( nScTab, aStr, aDummyCol, nDummyFlags );
                            aList.Insert( new String( aStr ), LIST_APPEND );
                            ++nScTab;
                        }
                    }
                    else
                    {
                        String  aComment;
                        pDoc->GetScenarioData( nTab, aComment, aDummyCol, nDummyFlags );
                        DBG_ASSERT( aList.Count() == 0, "List not empty!" );
                        aList.Insert( new String( aComment ) );
                    }

                    rSet.Put( SfxStringListItem( nWhich, &aList ) );

                    ULONG nCount = aList.Count();
                    for ( ULONG i=0; i<nCount; i++ )
                        delete (String*) aList.GetObject(i);
                }
                break;

            case FID_ROW_HIDE:
            case FID_ROW_SHOW:
            case FID_COL_HIDE:
            case FID_COL_SHOW:
            case FID_COL_OPT_WIDTH:
            case FID_ROW_OPT_HEIGHT:
            case FID_DELETE_CELL:
                if ( pDoc->IsTabProtected(nTab) || pDocSh->IsReadOnly())
                    rSet.DisableItem( nWhich );
                break;

/*  Zellschutz bei selektierten Zellen wird bei anderen Funktionen auch nicht abgefragt...
            case SID_DELETE:
                {
                    if ( pDoc->IsTabProtected(nTab) )
                    {
                        const SfxItemSet&       rAttrSet  = GetSelectionPattern()->GetItemSet();
                        const ScProtectionAttr& rProtAttr = (const ScProtectionAttr&)rAttrSet.Get( ATTR_PROTECTION, TRUE );
                        if ( rProtAttr.GetProtection() )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;
*/
            case SID_OUTLINE_MAKE:
                {
                    ScMarkData& rMark = GetViewData()->GetMarkData();

                    if (pDoc->GetChangeTrack()!=NULL || rMark.IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
            case SID_OUTLINE_SHOW:
                if (!pTabViewShell->OutlinePossible(FALSE))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_HIDE:
                if (!pTabViewShell->OutlinePossible(TRUE))
                    rSet.DisableItem( nWhich );
                break;

            case SID_OUTLINE_REMOVE:
                {
                    BOOL bCol, bRow;
                    pTabViewShell->TestRemoveOutline( bCol, bRow );
                    if ( !bCol && !bRow )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_COL_WIDTH:
                {
                    //GetViewData()->GetCurX();
                    SfxUInt16Item aWidthItem( FID_COL_WIDTH, pDoc->GetColWidth( nPosX , nTab) );
                    rSet.Put( aWidthItem );
                    if ( pDocSh->IsReadOnly())
                        rSet.DisableItem( nWhich );

                    //XXX Disablen wenn nicht eindeutig
                }
                break;

            case FID_ROW_HEIGHT:
                {
                    //GetViewData()->GetCurY();
                    SfxUInt16Item aHeightItem( FID_ROW_HEIGHT, pDoc->GetRowHeight( nPosY , nTab) );
                    rSet.Put( aHeightItem );
                    //XXX Disablen wenn nicht eindeutig
                    if ( pDocSh->IsReadOnly())
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DETECTIVE_FILLMODE:
                rSet.Put(SfxBoolItem( nWhich, pTabViewShell->IsAuditShell() ));
                break;

            case FID_INPUTLINE_STATUS:
                DBG_ERROR( "Old update method. Use ScTabViewShell::UpdateInputHandler()." );
                break;

            case SID_SCENARIOS:                                     // Szenarios:
                if (!(rMark.IsMarked() || rMark.IsMultiMarked()))   // nur, wenn etwas selektiert
                    rSet.DisableItem( nWhich );
                break;

            case FID_NOTE_VISIBLE:
                {
                    ScPostIt aNote;
                    if ( pDoc->IsBlockEditable( nTab, nPosX,nPosY, nPosX,nPosY ) &&
                         pDoc->GetNote( nPosX, nPosY, nTab, aNote ) )
                    {
                        BOOL bShown = aNote.IsShown() &&
                                    pDoc->HasNoteObject( nPosX, nPosY, nTab );
                        rSet.Put( SfxBoolItem( nWhich, bShown ) );
                    }
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_OPENDLG_CONSOLIDATE:
            case SCITEM_CONSOLIDATEDATA:
                {
                    if(pDoc->GetChangeTrack()!=NULL)
                                rSet.DisableItem( nWhich);
                }
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

//------------------------------------------------------------------



