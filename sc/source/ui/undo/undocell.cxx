/*************************************************************************
 *
 *  $RCSfile: undocell.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

#include "scitems.hxx"
#include <svx/editobj.hxx>
#include <svtools/zforlist.hxx>
#include <sfx2/app.hxx>

#include "undocell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "cell.hxx"
#include "target.hxx"
#include "undoolk.hxx"
#include "detdata.hxx"
#include "stlpool.hxx"
#include "printfun.hxx"
#include "rangenam.hxx"
#include "chgtrack.hxx"
#include "sc.hrc"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoCursorAttr, ScSimpleUndo);
TYPEINIT1(ScUndoEnterData, ScSimpleUndo);
TYPEINIT1(ScUndoEnterValue, ScSimpleUndo);
TYPEINIT1(ScUndoPutCell, ScSimpleUndo);
TYPEINIT1(ScUndoPageBreak, ScSimpleUndo);
TYPEINIT1(ScUndoPrintZoom, ScSimpleUndo);
TYPEINIT1(ScUndoThesaurus, ScSimpleUndo);
TYPEINIT1(ScUndoNote, ScSimpleUndo);
TYPEINIT1(ScUndoEditNote, ScSimpleUndo);
TYPEINIT1(ScUndoDetective, ScSimpleUndo);
TYPEINIT1(ScUndoRangeNames, ScSimpleUndo);


// -----------------------------------------------------------------------
//
//      Attribute auf Cursor anwenden
//

ScUndoCursorAttr::ScUndoCursorAttr( ScDocShell* pNewDocShell,
            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
            const ScPatternAttr* pApplyPat, BOOL bAutomatic ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    bIsAutomatic( bAutomatic )
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pNewPattern = (ScPatternAttr*) &pPool->Put( *pNewPat );
    pOldPattern = (ScPatternAttr*) &pPool->Put( *pOldPat );
    pApplyPattern = (ScPatternAttr*) &pPool->Put( *pApplyPat );
}

__EXPORT ScUndoCursorAttr::~ScUndoCursorAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pPool->Remove(*pNewPattern);
    pPool->Remove(*pOldPattern);
    pPool->Remove(*pApplyPattern);
}

String __EXPORT ScUndoCursorAttr::GetComment() const
{
    //! eigener Text fuer automatische Attributierung

    USHORT nId = STR_UNDO_CURSORATTR;        // "Attribute"
    return ScGlobal::GetRscString( nId );
}

void ScUndoCursorAttr::DoChange( const ScPatternAttr* pWhichPattern ) const
{
    pDocShell->GetDocument()->SetPattern( nCol, nRow, nTab, *pWhichPattern, TRUE );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, FALSE, FALSE );
        pViewShell->AdjustBlockHeight();
    }

    const SfxItemSet& rApplySet = pApplyPattern->GetItemSet();
    BOOL bPaintExt = ( rApplySet.GetItemState( ATTR_SHADOW, TRUE ) != SFX_ITEM_DEFAULT ||
                       rApplySet.GetItemState( ATTR_CONDITIONAL, TRUE ) != SFX_ITEM_DEFAULT );

    USHORT nFlags = SC_PF_TESTMERGE;
    if (bPaintExt)
        nFlags |= SC_PF_LINES;
    pDocShell->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, nFlags );
}

void __EXPORT ScUndoCursorAttr::Undo()
{
    BeginUndo();
    DoChange(pOldPattern);

    if ( bIsAutomatic )
    {
        //  wenn automatische Formatierung rueckgaengig gemacht wird,
        //  soll auch nicht weiter automatisch formatiert werden:

        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        if (pViewShell)
            pViewShell->ForgetFormatArea();
    }

    EndUndo();
}

void __EXPORT ScUndoCursorAttr::Redo()
{
    BeginRedo();
    DoChange(pNewPattern);
    EndRedo();
}

void __EXPORT ScUndoCursorAttr::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->ApplySelectionPattern( *pApplyPattern );
}

BOOL __EXPORT ScUndoCursorAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Daten eingeben
//

ScUndoEnterData::ScUndoEnterData( ScDocShell* pNewDocShell,
            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
            USHORT nNewCount, USHORT* pNewTabs, ScBaseCell** ppOldData,
            BOOL* pHasForm, ULONG* pOldForm,
            const String& rNewStr, EditTextObject* pObj ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    nCount( nNewCount ),
    pTabs( pNewTabs ),
    ppOldCells( ppOldData ),
    pHasFormat( pHasForm ),
    pOldFormats( pOldForm ),
    aNewString( rNewStr ),
    pNewEditData( pObj )
{
    SetChangeTrack();
}

__EXPORT ScUndoEnterData::~ScUndoEnterData()
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    for (USHORT i=0; i<nCount; i++)
        if (ppOldCells[i])
            ppOldCells[i]->Delete();
    delete ppOldCells;
    delete pHasFormat;
    delete pOldFormats;
    delete pTabs;
    delete pNewEditData;
}

String __EXPORT ScUndoEnterData::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Eingabe"
}

void ScUndoEnterData::DoChange() const
{
    //  Zeilenhoehe anpassen
    //! nur wenn noetig (alte oder neue EditZelle, oder Attribute) ??
    for (USHORT i=0; i<nCount; i++)
        pDocShell->AdjustRowHeight( nRow, nRow, pTabs[i] );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, FALSE, FALSE );
    }

    pDocShell->PostDataChanged();
}

void ScUndoEnterData::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        ScAddress aPos( nCol, nRow, nTab );
        for (USHORT i=0; i<nCount; i++)
        {
            aPos.SetTab( pTabs[i] );
            ULONG nFormat = 0;
            if ( pHasFormat && pOldFormats )
            {
                if ( pHasFormat[i] )
                    nFormat = pOldFormats[i];
            }
            pChangeTrack->AppendContent( aPos, ppOldCells[i], nFormat );
        }
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nichts appended
    }
    else
        nEndChangeAction = 0;
}

void __EXPORT ScUndoEnterData::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    for (USHORT i=0; i<nCount; i++)
    {
        ScBaseCell* pNewCell;
        if ( ppOldCells[i] )
        {
            // Formelzelle mit CompileTokenArray() !
            if ( ppOldCells[i]->GetCellType() == CELLTYPE_FORMULA )
                pNewCell = ((ScFormulaCell*)ppOldCells[i])->Clone( pDoc,
                    ScAddress( nCol, nRow, pTabs[i] ) );
            else
                pNewCell = ppOldCells[i]->Clone(pDoc);
        }
        else
            pNewCell = NULL;
        pDoc->PutCell( nCol, nRow, pTabs[i], pNewCell );

        if (pHasFormat && pOldFormats)
        {
            if ( pHasFormat[i] )
                pDoc->ApplyAttr( nCol, nRow, pTabs[i],
                                    SfxUInt32Item( ATTR_VALUE_FORMAT, pOldFormats[i] ) );
            else
            {
                ScPatternAttr aPattern( *pDoc->GetPattern( nCol, nRow, pTabs[i] ) );
                aPattern.GetItemSet().ClearItem( ATTR_VALUE_FORMAT );
                pDoc->SetPattern( nCol, nRow, pTabs[i], aPattern, TRUE );
            }
        }
        pDocShell->PostPaintCell( nCol, nRow, pTabs[i] );
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack && nEndChangeAction >= nCount )
        pChangeTrack->Undo( nEndChangeAction - nCount + 1, nEndChangeAction );

    DoChange();
    EndUndo();
}

void __EXPORT ScUndoEnterData::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    for (USHORT i=0; i<nCount; i++)
    {
        if (pNewEditData)
            pDoc->PutCell( nCol, nRow, pTabs[i], new ScEditCell( pNewEditData,
                pDoc, NULL ) );
        else
            pDoc->SetString( nCol, nRow, pTabs[i], aNewString );
        pDocShell->PostPaintCell( nCol, nRow, pTabs[i] );
    }

    SetChangeTrack();

    DoChange();
    EndRedo();
}

void __EXPORT ScUndoEnterData::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aNewString;
        ((ScTabViewTarget&)rTarget).GetViewShell()->EnterDataAtCursor( aTemp );
    }
}

BOOL __EXPORT ScUndoEnterData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Wert aendern
//

ScUndoEnterValue::ScUndoEnterValue( ScDocShell* pNewDocShell, const ScAddress& rNewPos,
                                    ScBaseCell* pUndoCell, double nVal, BOOL bHeight ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    pOldCell    ( pUndoCell ),
    nValue      ( nVal ),
    bNeedHeight ( bHeight )
{
    SetChangeTrack();
}

__EXPORT ScUndoEnterValue::~ScUndoEnterValue()
{
    if (pOldCell)
        pOldCell->Delete();
}

String __EXPORT ScUndoEnterValue::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Eingabe"
}

void ScUndoEnterValue::SetChangeTrack()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        pChangeTrack->AppendContent( aPos, pOldCell );
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nichts appended
    }
    else
        nEndChangeAction = 0;
}

void __EXPORT ScUndoEnterValue::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScBaseCell* pNewCell;
    if ( pOldCell )
    {
        // Formelzelle mit CompileTokenArray() !
        if ( pOldCell->GetCellType() == CELLTYPE_FORMULA )
            pNewCell = ((ScFormulaCell*)pOldCell)->Clone( pDoc, aPos );
        else
            pNewCell = pOldCell->Clone(pDoc);
    }
    else
        pNewCell = NULL;

    pDoc->PutCell( aPos.Col(), aPos.Row(), aPos.Tab(), pNewCell );

    pDocShell->PostPaintCell( aPos.Col(), aPos.Row(), aPos.Tab() );

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    EndUndo();
}

void __EXPORT ScUndoEnterValue::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetValue( aPos.Col(), aPos.Row(), aPos.Tab(), nValue );
    pDocShell->PostPaintCell( aPos.Col(), aPos.Row(), aPos.Tab() );

    SetChangeTrack();

    EndRedo();
}

void __EXPORT ScUndoEnterValue::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}

BOOL __EXPORT ScUndoEnterValue::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Beliebige Zelle eingeben
//

ScUndoPutCell::ScUndoPutCell( ScDocShell* pNewDocShell, const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell, BOOL bHeight ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    pOldCell    ( pUndoCell ),
    pEnteredCell( pRedoCell ),
    bNeedHeight ( bHeight )
{
    SetChangeTrack();
}

__EXPORT ScUndoPutCell::~ScUndoPutCell()
{
    if (pOldCell)
        pOldCell->Delete();
    if (pEnteredCell)
        pEnteredCell->Delete();
}

String __EXPORT ScUndoPutCell::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Eingabe"
}

void ScUndoPutCell::SetChangeTrack()
{
    ScDocument* pDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        pChangeTrack->AppendContent( aPos, pOldCell );
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nichts appended
    }
    else
        nEndChangeAction = 0;
}

void __EXPORT ScUndoPutCell::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScBaseCell* pNewCell;
    if ( pOldCell )
    {
        // Formelzelle mit CompileTokenArray() !
        if ( pOldCell->GetCellType() == CELLTYPE_FORMULA )
            pNewCell = ((ScFormulaCell*)pOldCell)->Clone( pDoc, aPos );
        else
            pNewCell = pOldCell->Clone(pDoc);
    }
    else
        pNewCell = NULL;

    pDoc->PutCell( aPos.Col(), aPos.Row(), aPos.Tab(), pNewCell );

    pDocShell->PostPaintCell( aPos.Col(), aPos.Row(), aPos.Tab() );

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    EndUndo();
}

void __EXPORT ScUndoPutCell::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
//  pDoc->SetValue( aPos.Col(), aPos.Row(), aPos.Tab(), nValue );

    ScBaseCell* pNewCell;
    if ( pEnteredCell )
    {
        // Formelzelle mit CompileTokenArray() !
        if ( pEnteredCell->GetCellType() == CELLTYPE_FORMULA )
            pNewCell = ((ScFormulaCell*)pEnteredCell)->Clone( pDoc, aPos );
        else
            pNewCell = pEnteredCell->Clone(pDoc);
    }
    else
        pNewCell = NULL;

    pDoc->PutCell( aPos.Col(), aPos.Row(), aPos.Tab(), pNewCell );

    pDocShell->PostPaintCell( aPos.Col(), aPos.Row(), aPos.Tab() );

    SetChangeTrack();

    EndRedo();
}

void __EXPORT ScUndoPutCell::Repeat(SfxRepeatTarget& rTarget)
{
    //  gippsnich
}

BOOL __EXPORT ScUndoPutCell::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Seitenumbrueche
//

ScUndoPageBreak::ScUndoPageBreak( ScDocShell* pNewDocShell,
            USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
            BOOL bNewColumn, BOOL bNewInsert ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    bColumn( bNewColumn ),
    bInsert( bNewInsert )
{
}

__EXPORT ScUndoPageBreak::~ScUndoPageBreak()
{
}

String __EXPORT ScUndoPageBreak::GetComment() const
{
    //"Spaltenumbruch" | "Zeilenumbruch"  "einfuegen" | "loeschen"
    return String ( bColumn ?
        ( bInsert ?
            ScGlobal::GetRscString( STR_UNDO_INSCOLBREAK ) :
            ScGlobal::GetRscString( STR_UNDO_DELCOLBREAK )
        ) :
        ( bInsert ?
            ScGlobal::GetRscString( STR_UNDO_INSROWBREAK ) :
            ScGlobal::GetRscString( STR_UNDO_DELROWBREAK )
        ) );
}

void ScUndoPageBreak::DoChange( BOOL bInsert ) const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, FALSE, FALSE );

        if (bInsert)
            pViewShell->InsertPageBreak(bColumn, FALSE);
        else
            pViewShell->DeletePageBreak(bColumn, FALSE);
    }
}

void __EXPORT ScUndoPageBreak::Undo()
{
    BeginUndo();
    DoChange(!bInsert);
    EndUndo();
}

void __EXPORT ScUndoPageBreak::Redo()
{
    BeginRedo();
    DoChange(bInsert);
    EndRedo();
}

void __EXPORT ScUndoPageBreak::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bInsert)
            rViewShell.InsertPageBreak(bColumn, TRUE);
        else
            rViewShell.DeletePageBreak(bColumn, TRUE);
    }
}

BOOL __EXPORT ScUndoPageBreak::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}

// -----------------------------------------------------------------------
//
//      Druck-Skalierung
//

ScUndoPrintZoom::ScUndoPrintZoom( ScDocShell* pNewDocShell,
            USHORT nT, USHORT nOS, USHORT nOP, USHORT nNS, USHORT nNP ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nT ),
    nOldScale( nOS ),
    nOldPages( nOP ),
    nNewScale( nNS ),
    nNewPages( nNP )
{
}

__EXPORT ScUndoPrintZoom::~ScUndoPrintZoom()
{
}

String __EXPORT ScUndoPrintZoom::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTSCALE );
}

void ScUndoPrintZoom::DoChange( BOOL bUndo )
{
    USHORT nScale = bUndo ? nOldScale : nNewScale;
    USHORT nPages = bUndo ? nOldPages : nNewPages;

    ScDocument* pDoc = pDocShell->GetDocument();
    String aStyleName = pDoc->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    DBG_ASSERT( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

        ScPrintFunc aPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab );
        aPrintFunc.UpdatePages();
    }
}

void __EXPORT ScUndoPrintZoom::Undo()
{
    BeginUndo();
    DoChange(TRUE);
    EndUndo();
}

void __EXPORT ScUndoPrintZoom::Redo()
{
    BeginRedo();
    DoChange(FALSE);
    EndRedo();
}

void __EXPORT ScUndoPrintZoom::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        ScViewData* pViewData = rViewShell.GetViewData();
        pViewData->GetDocShell()->SetPrintZoom( pViewData->GetTabNo(), nNewScale, nNewPages );
    }
}

BOOL __EXPORT ScUndoPrintZoom::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Thesaurus
//

ScUndoThesaurus::ScUndoThesaurus( ScDocShell* pNewDocShell,
                                  USHORT nNewCol, USHORT nNewRow, USHORT nNewTab,
                                  const String& rNewUndoStr, const EditTextObject* pUndoTObj,
                                  const String& rNewRedoStr, const EditTextObject* pRedoTObj) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    aUndoStr( rNewUndoStr ),
    aRedoStr( rNewRedoStr )
{
    pUndoTObject = (pUndoTObj) ? pUndoTObj->Clone() : NULL;
    pRedoTObject = (pRedoTObj) ? pRedoTObj->Clone() : NULL;

    ScBaseCell* pOldCell;
    if ( pUndoTObject )
        pOldCell = new ScEditCell( pUndoTObject, pDocShell->GetDocument(), NULL );
    else
        pOldCell = new ScStringCell( aUndoStr );
    SetChangeTrack( pOldCell );
    pOldCell->Delete();
}

__EXPORT ScUndoThesaurus::~ScUndoThesaurus()
{
    delete pUndoTObject;
    delete pRedoTObject;
}

String __EXPORT ScUndoThesaurus::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_THESAURUS );    // "Thesaurus"
}

void ScUndoThesaurus::SetChangeTrack( ScBaseCell* pOldCell )
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        pChangeTrack->AppendContent( ScAddress( nCol, nRow, nTab ), pOldCell );
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nichts appended
    }
    else
        nEndChangeAction = 0;
}

void __EXPORT ScUndoThesaurus::DoChange( BOOL bUndo, const String& rStr,
            const EditTextObject* pTObj )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, FALSE, FALSE );
    }

    if (pTObj)
    {
        ScBaseCell* pCell;
        pDoc->GetCell( nCol, nRow, nTab, pCell );
        if (pCell)
        {
            if (pCell->GetCellType() == CELLTYPE_EDIT )
            {
                ScEditCell* pNewCell = new ScEditCell( pTObj, pDoc, NULL );
                pDoc->PutCell( nCol, nRow, nTab, pNewCell );
                if ( !bUndo )
                    SetChangeTrack( pCell );
            }
            else
            {
                DBG_ERROR("Nicht CELLTYPE_EDIT bei Un/RedoThesaurus");
            }
        }
    }
    else
    {
        ScBaseCell* pCell;
        if ( !bUndo )
            pDoc->GetCell( nCol, nRow, nTab, pCell );
        pDoc->SetString( nCol, nRow, nTab, rStr );
        if ( !bUndo )
            SetChangeTrack( pCell );
    }

    pDocShell->PostPaintCell( nCol, nRow, nTab );
}

void __EXPORT ScUndoThesaurus::Undo()
{
    BeginUndo();
    DoChange( TRUE, aUndoStr, pUndoTObject );
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    EndUndo();
}

void __EXPORT ScUndoThesaurus::Redo()
{
    BeginRedo();
    DoChange( FALSE, aRedoStr, pRedoTObject );
    EndRedo();
}

void __EXPORT ScUndoThesaurus::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DoThesaurus( TRUE );
}

BOOL __EXPORT ScUndoThesaurus::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// -----------------------------------------------------------------------
//
//      Notizen ein-/ausblenden
//

ScUndoNote::ScUndoNote( ScDocShell* pNewDocShell, BOOL bShow,
                        const ScAddress& rNewPos, SdrUndoAction* pDraw ) :
    ScSimpleUndo( pNewDocShell ),
    bIsShow     ( bShow ),
    aPos        ( rNewPos ),
    pDrawUndo   ( pDraw )

{
}

__EXPORT ScUndoNote::~ScUndoNote()
{
    DeleteSdrUndoAction( pDrawUndo );
}

String __EXPORT ScUndoNote::GetComment() const
{
    if ( bIsShow )
        return ScGlobal::GetRscString( STR_UNDO_SHOWNOTE );     // Notiz anzeigen
    else
        return ScGlobal::GetRscString( STR_UNDO_HIDENOTE );     // Notiz ausblenden
}


void __EXPORT ScUndoNote::Undo()
{
    BeginUndo();

    if (pDrawUndo)
        DoSdrUndoAction(pDrawUndo);

    ScDocument* pDoc = pDocShell->GetDocument();
    ScPostIt aNote;
    pDoc->GetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aNote );
    aNote.SetShown( !bIsShow );
    pDoc->SetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aNote );

    EndUndo();
}

void __EXPORT ScUndoNote::Redo()
{
    BeginRedo();

    if (pDrawUndo)
        RedoSdrUndoAction(pDrawUndo);

    ScDocument* pDoc = pDocShell->GetDocument();
    ScPostIt aNote;
    pDoc->GetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aNote );
    aNote.SetShown( bIsShow );
    pDoc->SetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aNote );

    EndRedo();
}

void __EXPORT ScUndoNote::Repeat(SfxRepeatTarget& rTarget)
{
    //  hammanich
}

BOOL __EXPORT ScUndoNote::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Text von Notiz aendern (ohne Drawing-Krempel)
//

ScUndoEditNote::ScUndoEditNote( ScDocShell* pNewDocShell, const ScAddress& rNewPos,
                                const ScPostIt& rOld, const ScPostIt& rNew ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    aOldNote    ( rOld ),
    aNewNote    ( rNew )
{
}

__EXPORT ScUndoEditNote::~ScUndoEditNote()
{
}

String __EXPORT ScUndoEditNote::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_SHOWNOTE );
}

void __EXPORT ScUndoEditNote::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aOldNote );

    EndUndo();
}

void __EXPORT ScUndoEditNote::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetNote( aPos.Col(), aPos.Row(), aPos.Tab(), aNewNote );

    EndRedo();
}

void __EXPORT ScUndoEditNote::Repeat(SfxRepeatTarget& rTarget)
{
    //  hammanich
}

BOOL __EXPORT ScUndoEditNote::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}


// -----------------------------------------------------------------------
//
//      Detektiv
//

ScUndoDetective::ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList ) :
    ScSimpleUndo( pNewDocShell ),
    pDrawUndo   ( pDraw ),
    nAction     ( 0 ),
    pOldList    ( pUndoList )
{
    bIsDelete = ( pOperation == NULL );
    if (!bIsDelete)
    {
        nAction = (USHORT) pOperation->GetOperation();
        aPos = pOperation->GetPos();
    }
}

__EXPORT ScUndoDetective::~ScUndoDetective()
{
    DeleteSdrUndoAction( pDrawUndo );
    delete pOldList;
}

String __EXPORT ScUndoDetective::GetComment() const
{
    USHORT nId = STR_UNDO_DETDELALL;
    if ( !bIsDelete )
        switch ( (ScDetOpType) nAction )
        {
            case SCDETOP_ADDSUCC:   nId = STR_UNDO_DETADDSUCC;  break;
            case SCDETOP_DELSUCC:   nId = STR_UNDO_DETDELSUCC;  break;
            case SCDETOP_ADDPRED:   nId = STR_UNDO_DETADDPRED;  break;
            case SCDETOP_DELPRED:   nId = STR_UNDO_DETDELPRED;  break;
            case SCDETOP_ADDERROR:  nId = STR_UNDO_DETADDERROR; break;
        }

    return ScGlobal::GetRscString( nId );
}


void __EXPORT ScUndoDetective::Undo()
{
    BeginUndo();

    if (pDrawUndo)
        DoSdrUndoAction(pDrawUndo);

    ScDocument* pDoc = pDocShell->GetDocument();

    if (bIsDelete)
    {
        if ( pOldList )
            pDoc->SetDetOpList( new ScDetOpList(*pOldList) );
    }
    else
    {
        //  Eintrag aus der Liste loeschen

        ScDetOpList* pList = pDoc->GetDetOpList();
        if (pList && pList->Count())
        {
            USHORT nPos = pList->Count() - 1;
            ScDetOpData* pData = (*pList)[nPos];
            if ( pData->GetOperation() == (ScDetOpType) nAction && pData->GetPos() == aPos )
                pList->DeleteAndDestroy( nPos, 1 );
            else
                DBG_ERROR("Detektiv-Eintrag in der Liste nicht gefunden");
        }
    }

    EndUndo();
}

void __EXPORT ScUndoDetective::Redo()
{
    BeginRedo();

    if (pDrawUndo)
        RedoSdrUndoAction(pDrawUndo);

    ScDocument* pDoc = pDocShell->GetDocument();

    if (bIsDelete)
        pDoc->ClearDetectiveOperations();
    else
        pDoc->AddDetectiveOperation( ScDetOpData( aPos, (ScDetOpType) nAction ) );

    EndRedo();
}

void __EXPORT ScUndoDetective::Repeat(SfxRepeatTarget& rTarget)
{
    //  hammanich
}

BOOL __EXPORT ScUndoDetective::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}

// -----------------------------------------------------------------------
//
//      Benannte Bereiche
//

ScUndoRangeNames::ScUndoRangeNames( ScDocShell* pNewDocShell,
                                    ScRangeName* pOld, ScRangeName* pNew ) :
    ScSimpleUndo( pNewDocShell ),
    pOldRanges  ( pOld ),
    pNewRanges  ( pNew )
{
}

__EXPORT ScUndoRangeNames::~ScUndoRangeNames()
{
    delete pOldRanges;
    delete pNewRanges;
}

String __EXPORT ScUndoRangeNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_RANGENAMES );
}

void ScUndoRangeNames::DoChange( BOOL bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->CompileNameFormula( TRUE );   // CreateFormulaString

    if ( bUndo )
        pDoc->SetRangeName( new ScRangeName( *pOldRanges ) );
    else
        pDoc->SetRangeName( new ScRangeName( *pNewRanges ) );

    pDoc->CompileNameFormula( FALSE );  // CompileFormulaString

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
}

void __EXPORT ScUndoRangeNames::Undo()
{
    BeginUndo();
    DoChange( TRUE );
    EndUndo();
}

void __EXPORT ScUndoRangeNames::Redo()
{
    BeginRedo();
    DoChange( FALSE );
    EndRedo();
}

void __EXPORT ScUndoRangeNames::Repeat(SfxRepeatTarget& rTarget)
{
    //  hammanich
}

BOOL __EXPORT ScUndoRangeNames::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return FALSE;
}




