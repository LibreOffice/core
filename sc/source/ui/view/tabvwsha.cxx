/*************************************************************************
 *
 *  $RCSfile: tabvwsha.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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

// INCLUDE ---------------------------------------------------------------

#define _ZFORLIST_DECLARE_TABLE
#include "scitems.hxx"
#include <svtools/stritem.hxx>
#include <svtools/whiter.hxx>
#include <svtools/zformat.hxx>
#include <svx/boxitem.hxx>
#include <svx/numinf.hxx>
#include <svx/srchitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "document.hxx"
#include "cell.hxx"             // Input Status Edit-Zellen
#include "globstr.hrc"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "attrdlg.hxx"
#include "appoptio.hxx"
#include "sc.hrc"
#include "stlpool.hxx"
#include "tabvwsh.hxx"
#include "dwfunctr.hxx"



BOOL ScTabViewShell::GetFunction( String& rFuncStr )
{
    String aStr;

    ScSubTotalFunc eFunc = (ScSubTotalFunc) SC_MOD()->GetAppOptions().GetStatusFunc();

    USHORT nGlobStrId = 0;
    switch (eFunc)
    {
        case SUBTOTAL_FUNC_AVE:  nGlobStrId = STR_FUN_TEXT_AVG; break;
        case SUBTOTAL_FUNC_CNT:  nGlobStrId = STR_FUN_TEXT_COUNT; break;
        case SUBTOTAL_FUNC_CNT2: nGlobStrId = STR_FUN_TEXT_COUNT2; break;
        case SUBTOTAL_FUNC_MAX:  nGlobStrId = STR_FUN_TEXT_MAX; break;
        case SUBTOTAL_FUNC_MIN:  nGlobStrId = STR_FUN_TEXT_MIN; break;
        case SUBTOTAL_FUNC_SUM:  nGlobStrId = STR_FUN_TEXT_SUM; break;
    }
    if (nGlobStrId)
    {
        ScViewData* pViewData   = GetViewData();
        ScDocument* pDoc        = pViewData->GetDocument();
        ScMarkData& rMark       = pViewData->GetMarkData();
        USHORT      nPosX       = pViewData->GetCurX();
        USHORT      nPosY       = pViewData->GetCurY();
        USHORT      nTab        = pViewData->GetTabNo();

        aStr = ScGlobal::GetRscString(nGlobStrId);
        aStr += '=';

        //  Anzahl im Standardformat, die anderen nach Cursorposition
        ULONG nNumFmt = 0;
        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        if ( eFunc != SUBTOTAL_FUNC_CNT && eFunc != SUBTOTAL_FUNC_CNT2 )
        {
            //  Zahlformat aus Attributen oder Formel
            pDoc->GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
            if ( (nNumFmt % SV_COUNTRY_LANGUAGE_OFFSET) == 0 )
            {
                ScBaseCell* pCell;
                pDoc->GetCell( nPosX, nPosY, nTab, pCell );
                if (pCell && pCell->GetCellType() == CELLTYPE_FORMULA)
                    nNumFmt = ((ScFormulaCell*)pCell)->GetStandardFormat(
                        *pFormatter, nNumFmt );
            }
        }
        ScAddress aCursor( nPosX, nPosY, nTab );
        double nVal;
        if ( pDoc->GetSelectionFunction( eFunc, aCursor, rMark, nVal ) )
        {
            String aValStr;
            Color* pDummy;
            pFormatter->GetOutputString( nVal, nNumFmt, aValStr, &pDummy );
            aStr += aValStr;
        }

        rFuncStr = aStr;
        return TRUE;
    }

    return FALSE;
}



//  Funktionen, die je nach Selektion disabled sind
//  Default:
//      SID_DELETE,
//      SID_DELETE_CONTENTS,
//      FID_DELETE_CELL
//      FID_VALIDATION


void __EXPORT ScTabViewShell::GetState( SfxItemSet& rSet )
{
    ScViewData* pViewData   = GetViewData();
    ScDocument* pDoc        = pViewData->GetDocument();
    ScMarkData& rMark       = pViewData->GetMarkData();
    USHORT      nPosX       = pViewData->GetCurX();
    USHORT      nPosY       = pViewData->GetCurY();
    USHORT      nTab        = pViewData->GetTabNo();
    USHORT      nMyId       = 0;

    SfxViewFrame* pThisFrame = GetViewFrame();
    BOOL bOle = pThisFrame->ISA(SfxInPlaceFrame);

    USHORT nTabCount = pDoc->GetTableCount();
    USHORT nTabSelCount = rMark.GetSelectCount();

    SfxWhichIter    aIter(rSet);
    USHORT          nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        switch ( nWhich )
        {
            case FID_CHG_COMMENT:
                {
                    ScDocShell* pDocSh = GetViewData()->GetDocShell();
                    ScAddress aPos( nPosX, nPosY, nTab );
                    if ( pDocSh->IsReadOnly() || !pDocSh->GetChangeAction(aPos) )
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_DELETE_PRINTAREA:
                if ( !pDoc->GetPrintRangeCount( nTab ) )
                    rSet.DisableItem( nWhich );
                break;

            case SID_STATUS_PAGESTYLE:
            case SID_HFEDIT:
                GetViewData()->GetDocShell()->GetStatePageStyle( *this, rSet, nTab );
                break;

            case SID_SEARCH_ITEM:
                rSet.Put( ScGlobal::GetSearchItem() );
                break;

            case SID_SEARCH_OPTIONS:
                {
                    USHORT nOptions = 0xffff;       // alles erlaubt
                                                    // wenn ReadOnly, kein Ersetzen:
                    if (GetViewData()->GetDocShell()->IsReadOnly())
                        nOptions &= ~( SEARCH_OPTIONS_REPLACE | SEARCH_OPTIONS_REPLACE_ALL );
                    rSet.Put( SfxUInt16Item( nWhich, nOptions ) );
                }
                break;

            case SID_CURRENTCELL:
                {
                    ScAddress aScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), 0 );
                    String  aAddr;
                    aScAddress.Format( aAddr, SCA_ABS );
                    SfxStringItem   aPosItem( SID_CURRENTCELL, aAddr );

                    rSet.Put( aPosItem );
                }
                break;

            case SID_CURRENTTAB:
                //  Tabelle fuer Basic ist 1-basiert
                rSet.Put( SfxUInt16Item( nWhich, GetViewData()->GetTabNo() + 1 ) );
                break;

            case SID_CURRENTDOC:
                rSet.Put( SfxStringItem( nWhich, GetViewData()->GetDocShell()->GetTitle() ) );
                break;

            case FID_TOGGLEINPUTLINE:
                {
                    USHORT nId = ScInputWindowWrapper::GetChildWindowId();

                    if ( pThisFrame->KnowsChildWindow( nId ) )
                    {
                        SfxChildWindow* pWnd = pThisFrame->GetChildWindow( nId );
                        rSet.Put( SfxBoolItem( nWhich, pWnd ? TRUE : FALSE ) );
                    }
                    else
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_DEL_MANUALBREAKS:
                if (!pDoc->HasManualBreaks(nTab))
                    rSet.DisableItem( nWhich );
                break;

            case FID_RESET_PRINTZOOM:
                {
                    //  disablen, wenn schon Default eingestellt

                    ScDocShell* pDocSh = GetViewData()->GetDocShell();
                    ScDocument* pDoc = pDocSh->GetDocument();
                    USHORT nTab = GetViewData()->GetTabNo();
                    String aStyleName = pDoc->GetPageStyle( nTab );
                    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
                    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName,
                                                    SFX_STYLE_FAMILY_PAGE );
                    DBG_ASSERT( pStyleSheet, "PageStyle not found" );
                    if ( pStyleSheet )
                    {
                        SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();
                        USHORT nScale = ((const SfxUInt16Item&)
                                            rStyleSet.Get(ATTR_PAGE_SCALE)).GetValue();
                        USHORT nPages = ((const SfxUInt16Item&)
                                            rStyleSet.Get(ATTR_PAGE_SCALETOPAGES)).GetValue();
                        if ( nScale == 100 && nPages == 0 )
                            rSet.DisableItem( nWhich );
                    }
                }
                break;

            case FID_SCALE:
            case SID_ATTR_ZOOM:
                if ( bOle )
                    rSet.DisableItem( nWhich );
                else
                {
                    const Fraction& rOldY = GetViewData()->GetZoomY();
                    USHORT nZoom = (USHORT)(( rOldY.GetNumerator() * 100 )
                                                / rOldY.GetDenominator());
                    rSet.Put( SvxZoomItem( SVX_ZOOM_PERCENT, nZoom, nWhich ) );
                }
                break;

            case FID_TOGGLESYNTAX:
                rSet.Put(SfxBoolItem(nWhich, GetViewData()->IsSyntaxMode()));
                break;

            case FID_TOGGLEHEADERS:
                rSet.Put(SfxBoolItem(nWhich, GetViewData()->IsHeaderMode()));
                break;

            case FID_PAGEBREAKMODE:
                if ( bOle )
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put(SfxBoolItem(nWhich, GetViewData()->IsPagebreakMode()));
                break;

            case FID_FUNCTION_BOX:
                nMyId = ScFunctionChildWindow::GetChildWindowId();
                rSet.Put(SfxBoolItem(FID_FUNCTION_BOX, pThisFrame->HasChildWindow(nMyId)));
                break;

            case FID_PROTECT_DOC:
                rSet.Put(SfxBoolItem(nWhich, pDoc->IsDocProtected()));
                break;

            case FID_PROTECT_TABLE:
                rSet.Put(SfxBoolItem(nWhich, pDoc->IsTabProtected(nTab)));
                break;

            case SID_AUTO_OUTLINE:
                {
                    ScMarkData& rMark = GetViewData()->GetMarkData();

                    if (pDoc->GetChangeTrack()!=NULL || rMark.IsMultiMarked())
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_OUTLINE_DELETEALL:
                {
                    ScDocument* pDoc = GetViewData()->GetDocument();
                    USHORT nOlTab = GetViewData()->GetTabNo();
                    ScOutlineTable* pOlTable = pDoc->GetOutlineTable( nOlTab );
                    if (pOlTable == NULL)
                        rSet.DisableItem( nWhich );
                }
                break;

            case SID_WINDOW_SPLIT:
                rSet.Put(SfxBoolItem(nWhich,
                            pViewData->GetHSplitMode() == SC_SPLIT_NORMAL ||
                            pViewData->GetVSplitMode() == SC_SPLIT_NORMAL ));
                break;

            case SID_WINDOW_FIX:
                rSet.Put(SfxBoolItem(nWhich,
                            pViewData->GetHSplitMode() == SC_SPLIT_FIX ||
                            pViewData->GetVSplitMode() == SC_SPLIT_FIX ));
                break;

            case FID_CHG_SHOW:
                {
                    if(pDoc->GetChangeTrack()==NULL)
                                rSet.DisableItem( nWhich);
                }
                break;
            case FID_CHG_ACCEPT:
                {
                    rSet.Put(SfxBoolItem(FID_CHG_ACCEPT,
                            pThisFrame->HasChildWindow(FID_CHG_ACCEPT)));
                    if(pDoc->GetChangeTrack()==NULL)
                    {
                        if ( !pThisFrame->HasChildWindow(FID_CHG_ACCEPT) )
                        {
                            rSet.DisableItem( nWhich);
                        }
                    }
                }
                break;

            case SID_FORMATPAGE:
                //! bei geschuetzten Tabellen ???
                if (GetViewData()->GetDocShell()->IsReadOnly())
                    rSet.DisableItem( nWhich );
                break;

            case SID_PRINTPREVIEW:
                // #58924# Toggle-Slot braucht einen State
                rSet.Put( SfxBoolItem( nWhich, FALSE ) );
                break;

        } // switch ( nWitch )
        nWhich = aIter.NextWhich();
    } // while ( nWitch )
}

//------------------------------------------------------------------
void ScTabViewShell::ExecuteCellFormatDlg( SfxRequest& rReq, USHORT nTabPage )
{
    ScAttrDlg*              pDlg    = NULL;
    ScDocShell*             pDocSh  = GetViewData()->GetDocShell();
    ScDocument*             pDoc    = GetViewData()->GetDocument();

    SvxBoxItem              aLineOuter( ATTR_BORDER );
    SvxBoxInfoItem          aLineInner( ATTR_BORDER_INNER );

    SvxNumberInfoItem*      pNumberInfoItem = NULL;
    const ScPatternAttr*    pOldAttrs       = GetSelectionPattern();
    SfxItemSet*             pOldSet         = new SfxItemSet(
                                                    pOldAttrs->GetItemSet() );


    // Umrandungs-Items holen und in den Set packen:
    GetSelectionFrame( aLineOuter, aLineInner );
    pOldSet->Put( aLineOuter );
    pOldSet->Put( aLineInner );

    // NumberFormat Value aus Value und Language erzeugen und eintueten
    pOldSet->Put( SfxUInt32Item( ATTR_VALUE_FORMAT,
        pOldAttrs->GetNumberFormat( pDoc->GetFormatTable() ) ) );

    MakeNumberInfoItem( pDoc, GetViewData(), &pNumberInfoItem );

    pOldSet->MergeRange( SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO );
    pOldSet->Put(*pNumberInfoItem );

    bInFormatDialog = TRUE;
    pDlg = new ScAttrDlg( GetViewFrame(), GetDialogParent(), pOldSet );
    if ( nTabPage != 0xffff )
        pDlg->SetCurPageId( nTabPage );
    short nResult = pDlg->Execute();
    bInFormatDialog = FALSE;

    if ( nResult == RET_OK )
    {
        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

        const SfxPoolItem* pItem=NULL;
        if(pOutSet->GetItemState(SID_ATTR_NUMBERFORMAT_INFO,TRUE,&pItem)==SFX_ITEM_SET)
        {

            UpdateNumberFormatter( pDoc,(const SvxNumberInfoItem&)*pItem);
        }

        ApplyAttributes( pOutSet, pOldSet );

        rReq.Done( *pOutSet );
    }
    delete pOldSet;
    delete pNumberInfoItem;
    delete pDlg;
}

//------------------------------------------------------------------
void ScTabViewShell::ExecuteInputDirect()
{
    ScModule* pScMod = SC_MOD();
    if( !pScMod->IsFormulaMode() )
        pScMod->InputEnterHandler();
}

//------------------------------------------------------------------

void ScTabViewShell::UpdateInputHandler( BOOL bForce /* = FALSE */ )
{
    ScInputHandler* pHdl = pInputHandler ? pInputHandler : SC_MOD()->GetInputHdl();

    if ( pHdl )
    {
        String                  aString;
        const EditTextObject*   pObject     = NULL;
        ScViewData*             pViewData   = GetViewData();
        ScDocument*             pDoc        = pViewData->GetDocument();
        CellType                eType;
        USHORT                  nPosX       = pViewData->GetCurX();
        USHORT                  nPosY       = pViewData->GetCurY();
        USHORT                  nTab        = pViewData->GetTabNo();
        USHORT                  nStartTab   = 0;
        USHORT                  nEndTab     = 0;
        USHORT                  nStartCol   = 0;
        USHORT                  nStartRow   = 0;
        USHORT                  nEndCol     = 0;
        USHORT                  nEndRow     = 0;

        pViewData->GetSimpleArea( nStartCol, nStartRow, nStartTab,
                                  nEndCol,   nEndRow,   nEndTab, FALSE );

        PutInOrder( nStartCol, nEndCol );
        PutInOrder( nStartRow, nEndRow );
        PutInOrder( nStartTab, nEndTab );

        BOOL bHideFormula = FALSE;
        BOOL bHideAll     = FALSE;

        if (pDoc->IsTabProtected(nTab))
        {
            const ScProtectionAttr* pProt = (const ScProtectionAttr*)
                                            pDoc->GetAttr( nPosX,nPosY,nTab,
                                                           ATTR_PROTECTION);
            bHideFormula = pProt->GetHideFormula();
            bHideAll     = pProt->GetHideCell();
        }

        if (!bHideAll)
        {
            pDoc->GetCellType( nPosX, nPosY, nTab, eType );
            if (eType == CELLTYPE_FORMULA)
            {
                if (!bHideFormula)
                    pDoc->GetFormula( nPosX, nPosY, nTab, aString );
            }
            else if (eType == CELLTYPE_EDIT)
            {
                ScBaseCell* pCell;
                pDoc->GetCell( nPosX, nPosY, nTab, pCell );
                ((ScEditCell*)pCell)->GetData( pObject );
            }
            else
            {
                pDoc->GetInputString( nPosX, nPosY, nTab, aString );
                if (eType == CELLTYPE_STRING)
                {
                    //  Bei Bedarf ein ' vorneweg, damit der String nicht ungewollt
                    //  als Zahl interpretiert wird, und um dem Benutzer zu zeigen,
                    //  dass es ein String ist (#35060#).
                    //! Auch bei Zahlformat "Text"? -> dann beim Editieren wegnehmen

                    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
                    ULONG nNumFmt;
                    pDoc->GetNumberFormat( nPosX, nPosY, nTab, nNumFmt );
                    double fDummy;
                    if ( pFormatter->IsNumberFormat(aString, nNumFmt, fDummy) )
                        aString.Insert('\'',0);
                }
            }
        }

        ScInputHdlState aState( ScAddress( nPosX,     nPosY,     nTab ),
                                ScAddress( nStartCol, nStartRow, nTab ),
                                ScAddress( nEndCol,   nEndRow,   nTab ),
                                aString,
                                pObject );

        pHdl->NotifyChange( &aState, bForce );
    }

    SfxBindings& rBindings = SFX_BINDINGS();
    rBindings.Invalidate( SID_STATUS_SUM );         // immer zusammen mit Eingabezeile
    rBindings.Invalidate( SID_ATTR_SIZE );
}

//------------------------------------------------------------------

void __EXPORT ScTabViewShell::ExecuteSave( SfxRequest& rReq )
{
    //  nur SID_SAVEDOC / SID_SAVEASDOC

    // Eingabe auf jeden Fall abschliessen, auch wenn eine Formel bearbeitet wird
    SC_MOD()->InputEnterHandler();

    // ansonsten normal weiter
    GetViewData()->GetDocShell()->ExecuteSlot( rReq );
}

void __EXPORT ScTabViewShell::GetSaveState( SfxItemSet& rSet )
{
    SfxShell* pDocSh = GetViewData()->GetDocShell();

    SfxWhichIter aIter(rSet);
    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        // Status von der DocShell holen
        pDocSh->GetSlotState( nWhich, NULL, &rSet );
        nWhich = aIter.NextWhich();
    }
}

//------------------------------------------------------------------

void ScTabViewShell::ExecDrawOpt( SfxRequest& rReq )
{
    ScViewOptions aViewOptions = GetViewData()->GetOptions();
    ScGridOptions aGridOptions = aViewOptions.GetGridOptions();

    SfxBindings& rBindings = SFX_BINDINGS();
    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    USHORT nSlotId = rReq.GetSlot();
    switch (nSlotId)
    {
        case SID_GRID_VISIBLE:
            if ( pArgs && pArgs->GetItemState(nSlotId,TRUE,&pItem) == SFX_ITEM_SET )
            {
                aGridOptions.SetGridVisible( ((const SfxBoolItem*)pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_VISIBLE);
            }
            break;

        case SID_GRID_USE:
            if ( pArgs && pArgs->GetItemState(nSlotId,TRUE,&pItem) == SFX_ITEM_SET )
            {
                aGridOptions.SetUseGridSnap( ((const SfxBoolItem*)pItem)->GetValue() );
                aViewOptions.SetGridOptions(aGridOptions);
                rBindings.Invalidate(SID_GRID_USE);
            }
            break;

        case SID_HELPLINES_MOVE:
            if ( pArgs && pArgs->GetItemState(nSlotId,TRUE,&pItem) == SFX_ITEM_SET )
            {
                aViewOptions.SetOption( VOPT_HELPLINES, ((const SfxBoolItem*)pItem)->GetValue() );
                rBindings.Invalidate(SID_HELPLINES_MOVE);
            }
            break;
    }

    GetViewData()->SetOptions(aViewOptions);
}

void ScTabViewShell::GetDrawOptState( SfxItemSet& rSet )
{
    SfxBoolItem aBool;

    const ScViewOptions& rViewOptions = GetViewData()->GetOptions();
    const ScGridOptions& rGridOptions = rViewOptions.GetGridOptions();

    aBool.SetValue(rGridOptions.GetGridVisible());
    aBool.SetWhich( SID_GRID_VISIBLE );
    rSet.Put( aBool );

    aBool.SetValue(rGridOptions.GetUseGridSnap());
    aBool.SetWhich( SID_GRID_USE );
    rSet.Put( aBool );

    aBool.SetValue(rViewOptions.GetOption( VOPT_HELPLINES ));
    aBool.SetWhich( SID_HELPLINES_MOVE );
    rSet.Put( aBool );
}




