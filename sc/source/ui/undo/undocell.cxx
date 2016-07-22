/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "undocell.hxx"

#include "scitems.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <sfx2/app.hxx>

#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "formulacell.hxx"
#include "target.hxx"
#include "undoolk.hxx"
#include "detdata.hxx"
#include "stlpool.hxx"
#include "printfun.hxx"
#include "rangenam.hxx"
#include "chgtrack.hxx"
#include "sc.hrc"
#include "docuno.hxx"
#include "stringutil.hxx"

using std::shared_ptr;

namespace HelperNotifyChanges
{
    void NotifyIfChangesListeners(ScDocShell& rDocShell, const ScAddress &rPos,
        const ScUndoEnterData::ValuesType &rOldValues)
    {
        if (ScModelObj* pModelObj = getMustPropagateChangesModel(rDocShell))
        {
            ScRangeList aChangeRanges;

            for (const auto & rOldValue : rOldValues)
            {
                aChangeRanges.Append( ScRange(rPos.Col(), rPos.Row(), rOldValue.mnTab));
            }

            Notify(*pModelObj, aChangeRanges, "cell-change");
        }
    }
}


ScUndoCursorAttr::ScUndoCursorAttr( ScDocShell* pNewDocShell,
            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
            const ScPatternAttr* pApplyPat ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    pOldEditData( static_cast<EditTextObject*>(nullptr) ),
    pNewEditData( static_cast<EditTextObject*>(nullptr) )
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pNewPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>( &pPool->Put( *pNewPat ) ));
    pOldPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>( &pPool->Put( *pOldPat ) ));
    pApplyPattern = const_cast<ScPatternAttr*>(static_cast<const ScPatternAttr*>( &pPool->Put( *pApplyPat ) ));
}

ScUndoCursorAttr::~ScUndoCursorAttr()
{
    ScDocumentPool* pPool = pDocShell->GetDocument().GetPool();
    pPool->Remove(*pNewPattern);
    pPool->Remove(*pOldPattern);
    pPool->Remove(*pApplyPattern);
}

OUString ScUndoCursorAttr::GetComment() const
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
    ScDocument& rDoc = pDocShell->GetDocument();
    ScAddress aPos(nCol, nRow, nTab);
    rDoc.SetPattern( nCol, nRow, nTab, *pWhichPattern );

    if (rDoc.GetCellType(aPos) == CELLTYPE_EDIT && pEditData)
        rDoc.SetEditText(aPos, *pEditData, nullptr);

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );
        pViewShell->AdjustBlockHeight();
    }

    const SfxItemSet& rApplySet = pApplyPattern->GetItemSet();
    bool bPaintExt = ( rApplySet.GetItemState( ATTR_SHADOW ) != SfxItemState::DEFAULT ||
                       rApplySet.GetItemState( ATTR_CONDITIONAL ) != SfxItemState::DEFAULT );
    bool bPaintRows = ( rApplySet.GetItemState( ATTR_HOR_JUSTIFY ) != SfxItemState::DEFAULT );

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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->ApplySelectionPattern( *pApplyPattern );
}

bool ScUndoCursorAttr::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoEnterData::Value::Value() : mnTab(-1), mbHasFormat(false), mnFormat(0) {}

ScUndoEnterData::ScUndoEnterData(
    ScDocShell* pNewDocShell, const ScAddress& rPos, ValuesType& rOldValues,
    const OUString& rNewStr, EditTextObject* pObj ) :
    ScSimpleUndo( pNewDocShell ),
    maNewString(rNewStr),
    mpNewEditData(pObj),
    mnEndChangeAction(0),
    maPos(rPos)
{
    maOldValues.swap(rOldValues);

    SetChangeTrack();
}

ScUndoEnterData::~ScUndoEnterData() {}

OUString ScUndoEnterData::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoEnterData::DoChange() const
{
    // only when needed (old or new Edit cell, or Attribute)?
    for (const auto & i : maOldValues)
        pDocShell->AdjustRowHeight(maPos.Row(), maPos.Row(), i.mnTab);

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo(maPos.Tab());
        pViewShell->MoveCursorAbs(maPos.Col(), maPos.Row(), SC_FOLLOW_JUMP, false, false);
    }

    pDocShell->PostDataChanged();
}

void ScUndoEnterData::SetChangeTrack()
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
    {
        mnEndChangeAction = pChangeTrack->GetActionMax() + 1;
        ScAddress aPos(maPos);
        for (Value & rOldValue : maOldValues)
        {
            aPos.SetTab(rOldValue.mnTab);
            sal_uLong nFormat = 0;
            if (rOldValue.mbHasFormat)
                nFormat = rOldValue.mnFormat;
            pChangeTrack->AppendContent(aPos, rOldValue.maCell, nFormat);
        }
        if ( mnEndChangeAction > pChangeTrack->GetActionMax() )
            mnEndChangeAction = 0;       // nothing is appended
    }
    else
        mnEndChangeAction = 0;
}

void ScUndoEnterData::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    for (Value & rVal : maOldValues)
    {
        ScCellValue aNewCell;
        aNewCell.assign(rVal.maCell, rDoc, ScCloneFlags::StartListening);
        ScAddress aPos = maPos;
        aPos.SetTab(rVal.mnTab);
        aNewCell.release(rDoc, aPos);

        if (rVal.mbHasFormat)
            rDoc.ApplyAttr(maPos.Col(), maPos.Row(), rVal.mnTab,
                            SfxUInt32Item(ATTR_VALUE_FORMAT, rVal.mnFormat));
        else
        {
            ScPatternAttr aPattern(*rDoc.GetPattern(maPos.Col(), maPos.Row(), rVal.mnTab));
            aPattern.GetItemSet().ClearItem( ATTR_VALUE_FORMAT );
            rDoc.SetPattern(maPos.Col(), maPos.Row(), rVal.mnTab, aPattern);
        }
        pDocShell->PostPaintCell(maPos.Col(), maPos.Row(), rVal.mnTab);
    }

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    size_t nCount = maOldValues.size();
    if ( pChangeTrack && mnEndChangeAction >= sal::static_int_cast<sal_uLong>(nCount) )
        pChangeTrack->Undo( mnEndChangeAction - nCount + 1, mnEndChangeAction );

    DoChange();
    EndUndo();

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocShell, maPos, maOldValues);
}

void ScUndoEnterData::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    for (Value & rOldValue : maOldValues)
    {
        SCTAB nTab = rOldValue.mnTab;
        if (mpNewEditData)
        {
            ScAddress aPos = maPos;
            aPos.SetTab(nTab);
            // edit text wil be cloned.
            rDoc.SetEditText(aPos, *mpNewEditData, nullptr);
        }
        else
            rDoc.SetString(maPos.Col(), maPos.Row(), nTab, maNewString);

        pDocShell->PostPaintCell(maPos.Col(), maPos.Row(), nTab);
    }

    SetChangeTrack();

    DoChange();
    EndRedo();

    HelperNotifyChanges::NotifyIfChangesListeners(*pDocShell, maPos, maOldValues);
}

void ScUndoEnterData::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        OUString aTemp = maNewString;
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->EnterDataAtCursor( aTemp );
    }
}

bool ScUndoEnterData::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoEnterValue::ScUndoEnterValue(
    ScDocShell* pNewDocShell, const ScAddress& rNewPos,
    const ScCellValue& rUndoCell, double nVal ) :
    ScSimpleUndo( pNewDocShell ),
    aPos        ( rNewPos ),
    maOldCell(rUndoCell),
    nValue      ( nVal )
{
    SetChangeTrack();
}

ScUndoEnterValue::~ScUndoEnterValue()
{
}

OUString ScUndoEnterValue::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_ENTERDATA ); // "Input"
}

void ScUndoEnterValue::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        pChangeTrack->AppendContent(aPos, maOldCell);
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoEnterValue::Undo()
{
    BeginUndo();

    ScDocument& rDoc = pDocShell->GetDocument();
    ScCellValue aNewCell;
    aNewCell.assign(maOldCell, rDoc, ScCloneFlags::StartListening);
    aNewCell.release(rDoc, aPos);

    pDocShell->PostPaintCell( aPos );

    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );

    EndUndo();
}

void ScUndoEnterValue::Redo()
{
    BeginRedo();

    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.SetValue( aPos.Col(), aPos.Row(), aPos.Tab(), nValue );
    pDocShell->PostPaintCell( aPos );

    SetChangeTrack();

    EndRedo();
}

void ScUndoEnterValue::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoEnterValue::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

ScUndoSetCell::ScUndoSetCell( ScDocShell* pDocSh, const ScAddress& rPos, const ScCellValue& rOldVal, const ScCellValue& rNewVal ) :
    ScSimpleUndo(pDocSh), maPos(rPos), maOldValue(rOldVal), maNewValue(rNewVal), mnEndChangeAction(0)
{
    SetChangeTrack();
}

ScUndoSetCell::~ScUndoSetCell() {}

void ScUndoSetCell::Undo()
{
    BeginUndo();
    SetValue(maOldValue);
    pDocShell->PostPaintCell(maPos);

    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if (pChangeTrack)
        pChangeTrack->Undo(mnEndChangeAction, mnEndChangeAction);

    EndUndo();
}

void ScUndoSetCell::Redo()
{
    BeginRedo();
    SetValue(maNewValue);
    pDocShell->PostPaintCell(maPos);
    SetChangeTrack();
    EndRedo();
}

void ScUndoSetCell::Repeat( SfxRepeatTarget& /*rTarget*/ )
{
    // Makes no sense.
}

bool ScUndoSetCell::CanRepeat( SfxRepeatTarget& /*rTarget*/ ) const
{
    return false;
}

OUString ScUndoSetCell::GetComment() const
{
    return ScGlobal::GetRscString(STR_UNDO_ENTERDATA); // "Input"
}

void ScUndoSetCell::SetChangeTrack()
{
    ScDocument& rDoc = pDocShell->GetDocument();
    ScChangeTrack* pChangeTrack = rDoc.GetChangeTrack();
    if (pChangeTrack)
    {
        mnEndChangeAction = pChangeTrack->GetActionMax() + 1;

        pChangeTrack->AppendContent(maPos, maOldValue);

        if (mnEndChangeAction > pChangeTrack->GetActionMax())
            mnEndChangeAction = 0;       // Nothing is appended
    }
    else
        mnEndChangeAction = 0;
}

void ScUndoSetCell::SetValue( const ScCellValue& rVal )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    switch (rVal.meType)
    {
        case CELLTYPE_NONE:
            // empty cell
            rDoc.SetEmptyCell(maPos);
        break;
        case CELLTYPE_VALUE:
            rDoc.SetValue(maPos, rVal.mfValue);
        break;
        case CELLTYPE_STRING:
        {
            ScSetStringParam aParam;
            aParam.setTextInput();
            rDoc.SetString(maPos, rVal.mpString->getString());
        }
        break;
        case CELLTYPE_EDIT:
            rDoc.SetEditText(maPos, rVal.mpEditText->Clone());
        break;
        case CELLTYPE_FORMULA:
            rDoc.SetFormulaCell(maPos, rVal.mpFormula->Clone());
        break;
        default:
            ;
    }
}

ScUndoPageBreak::ScUndoPageBreak( ScDocShell* pNewDocShell,
            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
            bool bNewColumn, bool bNewInsert ) :
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

OUString ScUndoPageBreak::GetComment() const
{
    //"Column break" | "Row break"  "insert" | "delete"
    return OUString ( bColumn ?
        ( bInsert ?
            ScGlobal::GetRscString( STR_UNDO_INSCOLBREAK ) :
            ScGlobal::GetRscString( STR_UNDO_DELCOLBREAK )
        ) :
        ( bInsert ?
            ScGlobal::GetRscString( STR_UNDO_INSROWBREAK ) :
            ScGlobal::GetRscString( STR_UNDO_DELROWBREAK )
        ) );
}

void ScUndoPageBreak::DoChange( bool bInsertP ) const
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

        pDocShell->GetDocument().InvalidatePageBreaks(nTab);
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();

        if (bInsert)
            rViewShell.InsertPageBreak(bColumn);
        else
            rViewShell.DeletePageBreak(bColumn);
    }
}

bool ScUndoPageBreak::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
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

OUString ScUndoPrintZoom::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_PRINTSCALE );
}

void ScUndoPrintZoom::DoChange( bool bUndo )
{
    sal_uInt16 nScale = bUndo ? nOldScale : nNewScale;
    sal_uInt16 nPages = bUndo ? nOldPages : nNewPages;

    ScDocument& rDoc = pDocShell->GetDocument();
    OUString aStyleName = rDoc.GetPageStyle( nTab );
    ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( aStyleName, SfxStyleFamily::Page );
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
    DoChange(true);
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
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
    {
        ScTabViewShell& rViewShell = *static_cast<ScTabViewTarget&>(rTarget).GetViewShell();
        ScViewData& rViewData = rViewShell.GetViewData();
        rViewData.GetDocShell()->SetPrintZoom( rViewData.GetTabNo(), nNewScale, nNewPages );
    }
}

bool ScUndoPrintZoom::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

ScUndoThesaurus::ScUndoThesaurus(
    ScDocShell* pNewDocShell, SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
    const ScCellValue& rOldText, const ScCellValue& rNewText ) :
    ScSimpleUndo( pNewDocShell ),
    nCol( nNewCol ),
    nRow( nNewRow ),
    nTab( nNewTab ),
    maOldText(rOldText),
    maNewText(rNewText)
{
    SetChangeTrack(maOldText);
}

ScUndoThesaurus::~ScUndoThesaurus() {}

OUString ScUndoThesaurus::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_THESAURUS );    // "Thesaurus"
}

void ScUndoThesaurus::SetChangeTrack( const ScCellValue& rOldCell )
{
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
    {
        nEndChangeAction = pChangeTrack->GetActionMax() + 1;
        pChangeTrack->AppendContent(ScAddress(nCol, nRow, nTab), rOldCell);
        if ( nEndChangeAction > pChangeTrack->GetActionMax() )
            nEndChangeAction = 0;       // nothing is appended
    }
    else
        nEndChangeAction = 0;
}

void ScUndoThesaurus::DoChange( bool bUndo, const ScCellValue& rText )
{
    ScDocument& rDoc = pDocShell->GetDocument();

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
    {
        pViewShell->SetTabNo( nTab );
        pViewShell->MoveCursorAbs( nCol, nRow, SC_FOLLOW_JUMP, false, false );
    }

    ScAddress aPos(nCol, nRow, nTab);
    rText.commit(rDoc, aPos);
    if (!bUndo)
        SetChangeTrack(maOldText);

    pDocShell->PostPaintCell( nCol, nRow, nTab );
}

void ScUndoThesaurus::Undo()
{
    BeginUndo();
    DoChange(true, maOldText);
    ScChangeTrack* pChangeTrack = pDocShell->GetDocument().GetChangeTrack();
    if ( pChangeTrack )
        pChangeTrack->Undo( nEndChangeAction, nEndChangeAction );
    EndUndo();
}

void ScUndoThesaurus::Redo()
{
    BeginRedo();
    DoChange(false, maNewText);
    EndRedo();
}

void ScUndoThesaurus::Repeat(SfxRepeatTarget& rTarget)
{
    if (dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr)
        static_cast<ScTabViewTarget&>(rTarget).GetViewShell()->DoThesaurus();
}

bool ScUndoThesaurus::CanRepeat(SfxRepeatTarget& rTarget) const
{
    return dynamic_cast<const ScTabViewTarget*>( &rTarget) !=  nullptr;
}

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
    DoSdrUndoAction( mpDrawUndo, &pDocShell->GetDocument() );
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

bool ScUndoReplaceNote::CanRepeat( SfxRepeatTarget& /*rTarget*/ ) const
{
    return false;
}

OUString ScUndoReplaceNote::GetComment() const
{
    return ScGlobal::GetRscString( maNewData.mpCaption ?
        (maOldData.mpCaption ? STR_UNDO_EDITNOTE : STR_UNDO_INSERTNOTE) : STR_UNDO_DELETENOTE );
}

void ScUndoReplaceNote::DoInsertNote( const ScNoteData& rNoteData )
{
    if( rNoteData.mpCaption )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        OSL_ENSURE( !rDoc.GetNote(maPos), "ScUndoReplaceNote::DoInsertNote - unexpected cell note" );
        ScPostIt* pNote = new ScPostIt( rDoc, maPos, rNoteData, false );
        rDoc.SetNote( maPos, pNote );
    }
}

void ScUndoReplaceNote::DoRemoveNote( const ScNoteData& rNoteData )
{
    if( rNoteData.mpCaption )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        OSL_ENSURE( rDoc.GetNote(maPos), "ScUndoReplaceNote::DoRemoveNote - missing cell note" );
        if( ScPostIt* pNote = rDoc.ReleaseNote( maPos ) )
        {
            /*  Forget pointer to caption object to suppress removing the
                caption object from the drawing layer while deleting pNote
                (removing the caption is done by a drawing undo action). */
            pNote->ForgetCaption();
            delete pNote;
        }
    }
}

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
    if( ScPostIt* pNote = pDocShell->GetDocument().GetNote(maPos) )
        pNote->ShowCaption( maPos, !mbShown );
    EndUndo();
}

void ScUndoShowHideNote::Redo()
{
    BeginRedo();
    if( ScPostIt* pNote = pDocShell->GetDocument().GetNote(maPos) )
        pNote->ShowCaption( maPos, mbShown );
    EndRedo();
}

void ScUndoShowHideNote::Repeat( SfxRepeatTarget& /*rTarget*/ )
{
}

bool ScUndoShowHideNote::CanRepeat( SfxRepeatTarget& /*rTarget*/ ) const
{
    return false;
}

OUString ScUndoShowHideNote::GetComment() const
{
    return ScGlobal::GetRscString( mbShown ? STR_UNDO_SHOWNOTE : STR_UNDO_HIDENOTE );
}

ScUndoDetective::ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList ) :
    ScSimpleUndo( pNewDocShell ),
    pOldList    ( pUndoList ),
    nAction     ( 0 ),
    pDrawUndo   ( pDraw )
{
    bIsDelete = ( pOperation == nullptr );
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

OUString ScUndoDetective::GetComment() const
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

    ScDocument& rDoc = pDocShell->GetDocument();
    DoSdrUndoAction(pDrawUndo, &rDoc);

    if (bIsDelete)
    {
        if ( pOldList )
            rDoc.SetDetOpList( new ScDetOpList(*pOldList) );
    }
    else
    {
        // Remove entry from list

        ScDetOpList* pList = rDoc.GetDetOpList();
        if (pList && pList->Count())
        {
            ScDetOpDataVector& rVec = pList->GetDataVector();
            ScDetOpDataVector::iterator it = rVec.begin() + rVec.size() - 1;
            if ( (*it)->GetOperation() == (ScDetOpType) nAction && (*it)->GetPos() == aPos )
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

    ScDocument& rDoc = pDocShell->GetDocument();

    if (bIsDelete)
        rDoc.ClearDetectiveOperations();
    else
        rDoc.AddDetectiveOperation( ScDetOpData( aPos, (ScDetOpType) nAction ) );

    ScTabViewShell* pViewShell = ScTabViewShell::GetActiveViewShell();
    if (pViewShell)
        pViewShell->RecalcPPT();    //! use broadcast instead?

    EndRedo();
}

void ScUndoDetective::Repeat(SfxRepeatTarget& /* rTarget */)
{
    // makes no sense
}

bool ScUndoDetective::CanRepeat(SfxRepeatTarget& /* rTarget */) const
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

OUString ScUndoRangeNames::GetComment() const
{
    return ScGlobal::GetRscString( STR_UNDO_RANGENAMES );
}

void ScUndoRangeNames::DoChange( bool bUndo )
{
    ScDocument& rDoc = pDocShell->GetDocument();
    rDoc.PreprocessRangeNameUpdate();

    if ( bUndo )
    {
        if (mnTab >= 0)
            rDoc.SetRangeName( mnTab, new ScRangeName( *pOldRanges ) );
        else
            rDoc.SetRangeName( new ScRangeName( *pOldRanges ) );
    }
    else
    {
        if (mnTab >= 0)
            rDoc.SetRangeName( mnTab, new ScRangeName( *pNewRanges ) );
        else
            rDoc.SetRangeName( new ScRangeName( *pNewRanges ) );
    }

    rDoc.CompileHybridFormula();

    SfxGetpApp()->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );
}

void ScUndoRangeNames::Undo()
{
    BeginUndo();
    DoChange( true );
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

bool ScUndoRangeNames::CanRepeat(SfxRepeatTarget& /* rTarget */) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
