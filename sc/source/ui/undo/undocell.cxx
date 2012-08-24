/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <editeng/editobj.hxx>
#include <svl/zforlist.hxx>
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
#include "docuno.hxx"

using ::boost::shared_ptr;

// STATIC DATA -----------------------------------------------------------

TYPEINIT1(ScUndoCursorAttr, ScSimpleUndo);
TYPEINIT1(ScUndoEnterData, ScSimpleUndo);
TYPEINIT1(ScUndoEnterValue, ScSimpleUndo);
TYPEINIT1(ScUndoPutCell, ScSimpleUndo);
TYPEINIT1(ScUndoPageBreak, ScSimpleUndo);
TYPEINIT1(ScUndoPrintZoom, ScSimpleUndo);
TYPEINIT1(ScUndoThesaurus, ScSimpleUndo);
TYPEINIT1(ScUndoReplaceNote, ScSimpleUndo);
TYPEINIT1(ScUndoShowHideNote, ScSimpleUndo);
TYPEINIT1(ScUndoDetective, ScSimpleUndo);
TYPEINIT1(ScUndoRangeNames, ScSimpleUndo);

ScUndoCursorAttr::ScUndoCursorAttr( ScDocShell* pNewDocShell,
            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
            const ScPatternAttr* pApplyPat, sal_Bool bAutomatic ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    pOldEditData( static_cast<EditTextObject*>(NULL) ),
    pNewEditData( static_cast<EditTextObject*>(NULL) ),
    bIsAutomatic( bAutomatic )
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pNewPattern = (ScPatternAttr*) &pPool->Put( *pNewPat );
    pOldPattern = (ScPatternAttr*) &pPool->Put( *pOldPat );
    pApplyPattern = (ScPatternAttr*) &pPool->Put( *pApplyPat );
}

ScUndoCursorAttr::~ScUndoCursorAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument()->GetPool();
    pPool->Remove(*pNewPattern);
    pPool->Remove(*pOldPattern);
    pPool->Remove(*pApplyPattern);
}

rtl::OUString ScUndoCursorAttr::GetComment() const
{
    //! own text for automatic attribution

    sal_uInt16 nId = STR_UNDO_CURSORATTR;        // "Attribute"
    return ScGlobal::GetRscString( nId );
}

void ScUndoCursorAttr::SetEditData( EditTextObject* pOld, EditTextObject* pNew )
{
    pOldEditData.reset(pOld);
    pNewEditData.reset(pNew);
}

void ScUndoCursorAttr::DoChange( const ScPatternAttr* pWhichPattern, const shared_ptr<EditTextObject>& pEditData ) const
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetPattern( nCol, nRow, nTab, *pWhichPattern, true );

    ScBaseCell* pCell;
    pDoc->GetCell(nCol, nRow, nTab, pCell);
    if (pCell && pCell->GetCellType() == CELLTYPE_EDIT && pEditData.get())
        static_cast<ScEditCell*>(pCell)->SetData(pEditData.get(), NULL);

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );
        pViewShell->AdjustBlockHeight();
    }

    const SfxItemSet& rApplySet = pApplyPattern->GetItemSet();
    sal_Bool bPaintExt = ( rApplySet.GetItemState( ATTR_SHADOW, sal_True ) != SFX_ITEM_DEFAULT ||
                       rApplySet.GetItemState( ATTR_CONDITIONAL, sal_True ) != SFX_ITEM_DEFAULT );
    sal_Bool bPaintRows = ( rApplySet.GetItemState( ATTR_HOR_JUSTIFY, sal_True ) != SFX_ITEM_DEFAULT );

    sal_uInt16 nFlags = SC_PF_TESTMERGE;
    if (bPaintExt)
        nFlags |= SC_PF_LINES;
    if (bPaintRows)
        nFlags |= SC_PF_WHOLEROWS;
    pDocShell->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID, nFlags );
}

void ScUndoCursorAttr::Undo()
{
    BeginUndo();
    DoChange(pOldPattern, pOldEditData);

    if ( bIsAutomatic )
    {
        // if automatic formatting is reversed, then
        // automatic formatting should also not continue to be done

        ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
        if (pViewShell)
            pViewShell->ForgetFormatArea();
    }

    EndUndo();
}

void ScUndoCursorAttr::Redo()
{
    BeginRedo();
    DoChange(pNewPattern, pNewEditData);
    EndRedo();
}

void ScUndoCursorAttr::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->ApplySelectionPattern( *pApplyPattern );
}

sal_Bool ScUndoCursorAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoEnterData::ScUndoEnterData( ScDocShell* pNewDocShell,
            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
            SCTAB nNewCount, SCTAB* pNewTabs, ScBaseCell** ppOldData,
            sal_Bool* pHasForm, sal_uLong* pOldForm,
            const String& rNewStr, EditTextObject* pObj ) :
    ScSimpleUndo( pNewDocShell ),
    aNewString( rNewStr ),
    pTabs( pNewTabs ),
    ppOldCells( ppOldData ),
    pHasFormat( pHasForm ),
    pOldFormats( pOldForm ),
    pNewEditData( pObj ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    nCount( nNewCount )
{
    SetChangeTrack();
}

ScUndoEnterData::~ScUndoEnterData()
{
    for (sal_uInt16 i=0; i<nCount; i++)
        if (ppOldCells[i])
            ppOldCells[i]->Delete();
    delete[] ppOldCells;

    delete[] pHasFormat;
    delete[] pOldFormats;
    delete[] pTabs;

    delete pNewEditData;
}

rtl::OUString ScUndoEnterData::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoEnterData::DoChange() const
{
    // only when needed (old or new Edit cell, or Attribute)?
    for (sal_uInt16 i=0; i<nCount; i++)
        pDocShell->AdjustRowHeight( nRow, nRow, pTabs[i] );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );
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
        for (sal_uInt16 i=0; i<nCount; i++)
        {
            aPos.SetTab( pTabs[i] );
            sal_uLong nFormat = 0;
            if ( pHasFormat && pOldFormats )
            {
                if ( pHasFormat[i] )
                    nFormat = pOldFormats[i];
            }
            pChangeTrack->AppendContent( aPos, ppOldCells[i], nFormat );
        }
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoEnterData::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    for (sal_uInt16 i=0; i<nCount; i++)
    {
        ScBaseCell* pNewCell = ppOldCells[i] ? ppOldCells[i]->Clone( *pDoc, SC_CLONECELL_STARTLISTENING ) : 0;
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
                pDoc->SetPattern( nCol, nRow, pTabs[i], aPattern, sal_True );
            }
        }
        pDocShell->PostPaintCell( nCol, nRow, pTabs[i] );
    }

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack && nEndChangeAction >= sal::static_int_cast<sal_uLong>(nCount) )
        pChangeTrack->Undo( nEndChangeAction - nCount + 1, nEndChangeAction );

    DoChange();
    EndUndo();

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocShell->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ScRangeList aChangeRanges;
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            aChangeRanges.Append( ScRange( nCol, nRow, pTabs[i] ) );
        }
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
    }
}

void ScUndoEnterData::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    for (sal_uInt16 i=0; i<nCount; i++)
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

    // #i97876# Spreadsheet data changes are not notified
    ScModelObj* pModelObj = ScModelObj::getImplementation( pDocShell->GetModel() );
    if ( pModelObj && pModelObj->HasChangesListeners() )
    {
        ScRangeList aChangeRanges;
        for ( sal_uInt16 i = 0; i < nCount; ++i )
        {
            aChangeRanges.Append( ScRange( nCol, nRow, pTabs[i] ) );
        }
        pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
    }
}

void ScUndoEnterData::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        String aTemp = aNewString;
        ((ScTabViewTarget&)rTarget).GetViewShell()->EnterDataAtCursor( aTemp );
    }
}

sal_Bool ScUndoEnterData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoEnterValue::ScUndoEnterValue( ScDocShell* pNewDocShell, const ScAddress& rNewPos,
                                    ScBaseCell* pUndoCell, double nVal, sal_Bool bHeight ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    pOldCell    ( pUndoCell ),
    nValue      ( nVal ),
    bNeedHeight ( bHeight )
{
    SetChangeTrack();
}

ScUndoEnterValue::~ScUndoEnterValue()
{
    if (pOldCell)
        pOldCell->Delete();
}

rtl::OUString ScUndoEnterValue::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
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
            nEndChangeAction = 0;       // nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoEnterValue::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScBaseCell* pNewCell = pOldCell ? pOldCell->Clone( *pDoc, SC_CLONECELL_STARTLISTENING ) : 0;

    pDoc->PutCell( aPos, pNewCell );

    pDocShell->PostPaintCell( aPos );

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoEnterValue::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->SetValue( aPos.Col(), aPos.Row(), aPos.Tab(), nValue );
    pDocShell->PostPaintCell( aPos );

    SetChangeTrack();

    EndRedo();
}

void ScUndoEnterValue::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoEnterValue::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


ScUndoPutCell::ScUndoPutCell( ScDocShell* pNewDocShell, const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell, sal_Bool bHeight ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    pOldCell    ( pUndoCell ),
    pEnteredCell( pRedoCell ),
    bNeedHeight ( bHeight )
{
    SetChangeTrack();
}

ScUndoPutCell::~ScUndoPutCell()
{
    if (pOldCell)
        pOldCell->Delete();
    if (pEnteredCell)
        pEnteredCell->Delete();
}

rtl::OUString ScUndoPutCell::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
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
            nEndChangeAction = 0;       // Nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoPutCell::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScBaseCell* pNewCell = pOldCell ? pOldCell->Clone( *pDoc, aPos, SC_CLONECELL_STARTLISTENING ) : 0;

    pDoc->PutCell( aPos.Col(), aPos.Row(), aPos.Tab(), pNewCell );

    pDocShell->PostPaintCell( aPos );

    ScChangeTrack* pChangeTrack = pDoc->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoPutCell::Redo()
{
    BeginRedo();

    ScDocument* pDoc = pDocShell->GetDocument();
    ScBaseCell* pNewCell = pEnteredCell ? pEnteredCell->Clone( *pDoc, aPos, SC_CLONECELL_STARTLISTENING ) : 0;

    pDoc->PutCell( aPos.Col(), aPos.Row(), aPos.Tab(), pNewCell );

    pDocShell->PostPaintCell( aPos );

    SetChangeTrack();

    EndRedo();
}

void ScUndoPutCell::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoPutCell::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


ScUndoPageBreak::ScUndoPageBreak( ScDocShell* pNewDocShell,
            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
            sal_Bool bNewColumn, sal_Bool bNewInsert ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    bColumn( bNewColumn ),
    bInsert( bNewInsert )
{
}

ScUndoPageBreak::~ScUndoPageBreak()
{
}

rtl::OUString ScUndoPageBreak::GetComment() const
{
    //"Column break" | "Row break"  "insert" | "delete"
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

void ScUndoPageBreak::DoChange( sal_Bool bInsertP ) const
{
    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();

    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );

        if (bInsertP)
            pViewShell->InsertPageBreak(bColumn, false);
        else
            pViewShell->DeletePageBreak(bColumn, false);

        pDocShell->GetDocument()->InvalidatePageBreaks(nTab);
    }
}

void ScUndoPageBreak::Undo()
{
    BeginUndo();
    DoChange(!bInsert);
    EndUndo();
}

void ScUndoPageBreak::Redo()
{
    BeginRedo();
    DoChange(bInsert);
    EndRedo();
}

void ScUndoPageBreak::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();

        if (bInsert)
            rViewShell.InsertPageBreak(bColumn, sal_True);
        else
            rViewShell.DeletePageBreak(bColumn, sal_True);
    }
}

sal_Bool ScUndoPageBreak::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoPrintZoom::ScUndoPrintZoom( ScDocShell* pNewDocShell,
            SCTAB nT, sal_uInt16 nOS, sal_uInt16 nOP, sal_uInt16 nNS, sal_uInt16 nNP ) :
    ScSimpleUndo( pNewDocShell ),
    nTab( nT ),
    nOldScale( nOS ),
    nOldPages( nOP ),
    nNewScale( nNS ),
    nNewPages( nNP )
{
}

ScUndoPrintZoom::~ScUndoPrintZoom()
{
}

rtl::OUString ScUndoPrintZoom::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTSCALE );
}

void ScUndoPrintZoom::DoChange( sal_Bool bUndo )
{
    sal_uInt16 nScale = bUndo ? nOldScale : nNewScale;
    sal_uInt16 nPages = bUndo ? nOldPages : nNewPages;

    ScDocument* pDoc = pDocShell->GetDocument();
    String aStyleName = pDoc->GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PAGE );
    OSL_ENSURE( pStyleSheet, "PageStyle not found" );
    if ( pStyleSheet )
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALE, nScale ) );
        rSet.Put( SfxUInt16Item( ATTR_PAGE_SCALETOPAGES, nPages ) );

        ScPrintFunc aPrintFunc( pDocShell, pDocShell->GetPrinter(), nTab );
        aPrintFunc.UpdatePages();
    }
}

void ScUndoPrintZoom::Undo()
{
    BeginUndo();
    DoChange(sal_True);
    EndUndo();
}

void ScUndoPrintZoom::Redo()
{
    BeginRedo();
    DoChange(false);
    EndRedo();
}

void ScUndoPrintZoom::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
    {
        ScTabViewShell& rViewShell = *((ScTabViewTarget&)rTarget).GetViewShell();
        ScViewData* pViewData = rViewShell.GetViewData();
        pViewData->GetDocShell()->SetPrintZoom( pViewData->GetTabNo(), nNewScale, nNewPages );
    }
}

sal_Bool ScUndoPrintZoom::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


ScUndoThesaurus::ScUndoThesaurus( ScDocShell* pNewDocShell,
                                  SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
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

ScUndoThesaurus::~ScUndoThesaurus()
{
    delete pUndoTObject;
    delete pRedoTObject;
}

rtl::OUString ScUndoThesaurus::GetComment() const
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
            nEndChangeAction = 0;       // nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoThesaurus::DoChange( sal_Bool bUndo, const String& rStr,
            const EditTextObject* pTObj )
{
    ScDocument* pDoc = pDocShell->GetDocument();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );
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
                OSL_FAIL("Not CELLTYPE_EDIT for Un/RedoThesaurus");
            }
        }
    }
    else
    {
        ScBaseCell* pCell = NULL;
        if ( !bUndo )
            pDoc->GetCell( nCol, nRow, nTab, pCell );
        pDoc->SetString( nCol, nRow, nTab, rStr );
        if ( !bUndo )
            SetChangeTrack( pCell );
    }

    pDocShell->PostPaintCell( nCol, nRow, nTab );
}

void ScUndoThesaurus::Undo()
{
    BeginUndo();
    DoChange( sal_True, aUndoStr, pUndoTObject );
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument()->GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoThesaurus::Redo()
{
    BeginRedo();
    DoChange( false, aRedoStr, pRedoTObject );
    EndRedo();
}

void ScUndoThesaurus::Repeat(SfxRepeatTarget& rTarget)
{
    if (rTarget.ISA(ScTabViewTarget))
        ((ScTabViewTarget&)rTarget).GetViewShell()->DoThesaurus( sal_True );
}

sal_Bool ScUndoThesaurus::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return (rTarget.ISA(ScTabViewTarget));
}


// ============================================================================

ScUndoReplaceNote::ScUndoReplaceNote( ScDocShell& rDocShell, const ScAddress& rPos,
        const ScNoteData& rNoteData, bool bInsert, SdrUndoAction* pDrawUndo ) :
    ScSimpleUndo( &rDocShell ),
    maPos( rPos ),
    mpDrawUndo( pDrawUndo )
{
    OSL_ENSURE( rNoteData.mpCaption, "ScUndoReplaceNote::ScUndoReplaceNote - missing note caption" );
    (bInsert ? maNewData : maOldData) = rNoteData;
}

ScUndoReplaceNote::ScUndoReplaceNote( ScDocShell& rDocShell, const ScAddress& rPos,
        const ScNoteData& rOldData, const ScNoteData& rNewData, SdrUndoAction* pDrawUndo ) :
    ScSimpleUndo( &rDocShell ),
    maPos( rPos ),
    maOldData( rOldData ),
    maNewData( rNewData ),
    mpDrawUndo( pDrawUndo )
{
    OSL_ENSURE( maOldData.mpCaption || maNewData.mpCaption, "ScUndoReplaceNote::ScUndoReplaceNote - missing note captions" );
    OSL_ENSURE( !maOldData.mxInitData.get() && !maNewData.mxInitData.get(), "ScUndoReplaceNote::ScUndoReplaceNote - unexpected unitialized note" );
}

ScUndoReplaceNote::~ScUndoReplaceNote()
{
    DeleteSdrUndoAction( mpDrawUndo );
}

void ScUndoReplaceNote::Undo()
{
    BeginUndo();
    DoSdrUndoAction( mpDrawUndo, pDocShell->GetDocument() );
    /*  Undo insert -> remove new note.
        Undo remove -> insert old note.
        Undo replace -> remove new note, insert old note. */
    DoRemoveNote( maNewData );
    DoInsertNote( maOldData );
    pDocShell->PostPaintCell( maPos );
    EndUndo();
}

void ScUndoReplaceNote::Redo()
{
    BeginRedo();
    RedoSdrUndoAction( mpDrawUndo );
    /*  Redo insert -> insert new note.
        Redo remove -> remove old note.
        Redo replace -> remove old note, insert new note. */
    DoRemoveNote( maOldData );
    DoInsertNote( maNewData );
    pDocShell->PostPaintCell( maPos );
    EndRedo();
}

void ScUndoReplaceNote::Repeat( SfxRepeatTarget& /*rTarget*/ )
{
}

sal_Bool ScUndoReplaceNote::CanRepeat( SfxRepeatTarget& /*rTarget*/ ) const
{
    return false;
}

rtl::OUString ScUndoReplaceNote::GetComment() const
{
    return ScGlobal::GetRscString( maNewData.mpCaption ?
        (maOldData.mpCaption ? STR_UNDO_EDITNOTE : STR_UNDO_INSERTNOTE) : STR_UNDO_DELETENOTE );
}

void ScUndoReplaceNote::DoInsertNote( const ScNoteData& rNoteData )
{
    if( rNoteData.mpCaption )
    {
        ScDocument& rDoc = *pDocShell->GetDocument();
        OSL_ENSURE( !rDoc.GetNotes( maPos.Tab() )->findByAddress(maPos), "ScUndoReplaceNote::DoInsertNote - unexpected cell note" );
        ScPostIt* pNote = new ScPostIt( rDoc, maPos, rNoteData, false );
        rDoc.GetNotes(maPos.Tab())->insert( maPos, pNote );
    }
}

void ScUndoReplaceNote::DoRemoveNote( const ScNoteData& rNoteData )
{
    if( rNoteData.mpCaption )
    {
        ScDocument& rDoc = *pDocShell->GetDocument();
        OSL_ENSURE( rDoc.GetNotes( maPos.Tab() )->findByAddress(maPos), "ScUndoReplaceNote::DoRemoveNote - missing cell note" );
        if( ScPostIt* pNote = rDoc.GetNotes(maPos.Tab())->ReleaseNote( maPos ) )
        {
            /*  Forget pointer to caption object to suppress removing the
                caption object from the drawing layer while deleting pNote
                (removing the caption is done by a drawing undo action). */
            pNote->ForgetCaption();
            delete pNote;
        }
    }
}

// ============================================================================

ScUndoShowHideNote::ScUndoShowHideNote( ScDocShell& rDocShell, const ScAddress& rPos, bool bShow ) :
    ScSimpleUndo( &rDocShell ),
    maPos( rPos ),
    mbShown( bShow )
{
}

ScUndoShowHideNote::~ScUndoShowHideNote()
{
}

void ScUndoShowHideNote::Undo()
{
    BeginUndo();
    if( ScPostIt* pNote = pDocShell->GetDocument()->GetNotes( maPos.Tab() )->findByAddress(maPos) )
        pNote->ShowCaption( maPos, !mbShown );
    EndUndo();
}

void ScUndoShowHideNote::Redo()
{
    BeginRedo();
    if( ScPostIt* pNote = pDocShell->GetDocument()->GetNotes( maPos.Tab() )->findByAddress(maPos) )
        pNote->ShowCaption( maPos, mbShown );
    EndRedo();
}

void ScUndoShowHideNote::Repeat( SfxRepeatTarget& /*rTarget*/ )
{
}

sal_Bool ScUndoShowHideNote::CanRepeat( SfxRepeatTarget& /*rTarget*/ ) const
{
    return false;
}

rtl::OUString ScUndoShowHideNote::GetComment() const
{
    return ScGlobal::GetRscString( mbShown ? STR_UNDO_SHOWNOTE : STR_UNDO_HIDENOTE );
}

// ============================================================================

ScUndoDetective::ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList ) :
    ScSimpleUndo( pNewDocShell ),
    pOldList    ( pUndoList ),
    nAction     ( 0 ),
    pDrawUndo   ( pDraw )
{
    bIsDelete = ( pOperation == NULL );
    if (!bIsDelete)
    {
        nAction = (sal_uInt16) pOperation->GetOperation();
        aPos = pOperation->GetPos();
    }
}

ScUndoDetective::~ScUndoDetective()
{
    DeleteSdrUndoAction( pDrawUndo );
    delete pOldList;
}

rtl::OUString ScUndoDetective::GetComment() const
{
    sal_uInt16 nId = STR_UNDO_DETDELALL;
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


void ScUndoDetective::Undo()
{
    BeginUndo();

    ScDocument* pDoc = pDocShell->GetDocument();
    DoSdrUndoAction(pDrawUndo, pDoc);

    if (bIsDelete)
    {
        if ( pOldList )
            pDoc->SetDetOpList( new ScDetOpList(*pOldList) );
    }
    else
    {
        // Remove entry from list

        ScDetOpList* pList = pDoc->GetDetOpList();
        if (pList && pList->Count())
        {
            ScDetOpDataVector& rVec = pList->GetDataVector();
            ScDetOpDataVector::iterator it = rVec.begin() + rVec.size() - 1;
            if ( it->GetOperation() == (ScDetOpType) nAction && it->GetPos() == aPos )
                rVec.erase( it);
            else
            {
                OSL_FAIL("Detective entry could not be found in list");
            }
        }
    }

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->RecalcPPT();    //! use broadcast instead?

    EndUndo();
}

void ScUndoDetective::Redo()
{
    BeginRedo();

    RedoSdrUndoAction(pDrawUndo);

    ScDocument* pDoc = pDocShell->GetDocument();

    if (bIsDelete)
        pDoc->ClearDetectiveOperations();
    else
        pDoc->AddDetectiveOperation( ScDetOpData( aPos, (ScDetOpType) nAction ) );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->RecalcPPT();    //! use broadcast instead?

    EndRedo();
}

void ScUndoDetective::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoDetective::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}


ScUndoRangeNames::ScUndoRangeNames( ScDocShell* pNewDocShell,
                                    ScRangeName* pOld, ScRangeName* pNew, SCTAB nTab ) :
    ScSimpleUndo( pNewDocShell ),
    pOldRanges  ( pOld ),
    pNewRanges  ( pNew ),
    mnTab       ( nTab )
{
}

ScUndoRangeNames::~ScUndoRangeNames()
{
    delete pOldRanges;
    delete pNewRanges;
}

rtl::OUString ScUndoRangeNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_RANGENAMES );
}

void ScUndoRangeNames::DoChange( sal_Bool bUndo )
{
    ScDocument* pDoc = pDocShell->GetDocument();
    pDoc->CompileNameFormula( sal_True );   // CreateFormulaString

    if ( bUndo )
    {
        if (mnTab >= 0)
            pDoc->SetRangeName( mnTab, new ScRangeName( *pOldRanges ) );
        else
            pDoc->SetRangeName( new ScRangeName( *pOldRanges ) );
    }
    else
    {
        if (mnTab >= 0)
            pDoc->SetRangeName( mnTab, new ScRangeName( *pNewRanges ) );
        else
            pDoc->SetRangeName( new ScRangeName( *pNewRanges ) );
    }

    pDoc->CompileNameFormula( false );  // CompileFormulaString

    SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
}

void ScUndoRangeNames::Undo()
{
    BeginUndo();
    DoChange( sal_True );
    EndUndo();
}

void ScUndoRangeNames::Redo()
{
    BeginRedo();
    DoChange( false );
    EndRedo();
}

void ScUndoRangeNames::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

sal_Bool ScUndoRangeNames::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
