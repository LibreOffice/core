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

#include <hintids.hxx>
#include <wrtsh.hxx>
#include <swcrsr.hxx>
#include <editeng/lrspitem.hxx>
#include <view.hxx>
#include <drawbase.hxx>
#include <unobaseclass.hxx>
#include <fmtanchr.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <docufld.hxx>
#include <IDocumentUndoRedo.hxx>
#include <i18nutil/unicode.hxx>
#include <o3tl/temporary.hxx>
#include <rtl/character.hxx>
#include <osl/diagnose.h>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>

inline void SwWrtShell::OpenMark()
{
    StartAllAction();
    ResetCursorStack();
    KillPams();
    SetMark();
}

inline void SwWrtShell::CloseMark( bool bOkFlag )
{
    if( bOkFlag )
        UpdateAttr();
    else
        SwapPam();

    ClearMark();
    EndAllAction();
}



// #i23725#
bool SwWrtShell::TryRemoveIndent()
{
    bool bResult = false;

    SfxItemSetFixed<RES_MARGIN_FIRSTLINE, RES_MARGIN_FIRSTLINE> aAttrSet(GetAttrPool());
    GetCurAttr(aAttrSet);

    SvxFirstLineIndentItem firstLine(aAttrSet.Get(RES_MARGIN_FIRSTLINE));
    SvxTextLeftMarginItem leftMargin(aAttrSet.Get(RES_MARGIN_TEXTLEFT));
    short aOldFirstLineOfst = firstLine.GetTextFirstLineOffset();

    if (aOldFirstLineOfst > 0)
    {
        firstLine.SetTextFirstLineOffset(0);
        bResult = true;
    }
    else if (aOldFirstLineOfst < 0)
    {
        // this used to call SetLeft() but this should be the same result
        firstLine.SetTextFirstLineOffset(0);
        leftMargin.SetTextLeft(leftMargin.GetTextLeft() + aOldFirstLineOfst);
        bResult = true;
    }
    else if (leftMargin.GetTextLeft() != 0)
    {
        leftMargin.SetTextLeft(0);
        bResult = true;
    }

    if (bResult)
    {
        aAttrSet.Put(firstLine);
        aAttrSet.Put(leftMargin);
        SetAttrSet(aAttrSet);
    }

    return bResult;
}

/** Description: Erase the line. */

void SwWrtShell::DelLine()
{
    SwActContext aActContext(this);
    ResetCursorStack();
        // remember the old cursor
    Push();
    ClearMark();
    SwCursorShell::LeftMargin();
    SetMark();
    SwCursorShell::RightMargin();

    bool bRet = Delete(false);
    Pop(SwCursorShell::PopMode::DeleteCurrent);
    if( bRet )
        UpdateAttr();
}

void SwWrtShell::DelToStartOfLine()
{
    OpenMark();
    SwCursorShell::LeftMargin();
    bool bRet = Delete(false);
    CloseMark( bRet );
}

void SwWrtShell::DelToEndOfLine()
{
    OpenMark();
    SwCursorShell::RightMargin();
    bool bRet = Delete(false);
    CloseMark( bRet );
}

bool SwWrtShell::DelLeft()
{
    // If it's a Fly, throw it away
    SelectionType nSelType = GetSelectionType();
    const SelectionType nCmp = SelectionType::Frame | SelectionType::Graphic | SelectionType::Ole | SelectionType::DrawObject;
    if( nCmp & nSelType )
    {
        // #108205# Remember object's position.
        Point aTmpPt = GetObjRect().TopLeft();

        DelSelectedObj();

        // #108205# Set cursor to remembered position.
        SetCursor(&aTmpPt);

        LeaveSelFrameMode();
        UnSelectFrame();

        nSelType = GetSelectionType();
        if ( nCmp & nSelType )
        {
            EnterSelFrameMode();
            GotoNextFly();
        }

        return true;
    }

    // If a selection exists, erase this
    if ( IsSelection() )
    {
        if( !IsBlockMode() || HasSelection() )
        {
            //OS: Once again Basic: SwActContext must be leaved
            //before EnterStdMode!
            {
                SwActContext aActContext(this);
                ResetCursorStack();
                Delete(false, true);
                UpdateAttr();
            }
            if( IsBlockMode() )
            {
                NormalizePam();
                ClearMark();
                EnterBlockMode();
            }
            else
                EnterStdMode();
            return true;
        }
        else
            EnterStdMode();
    }

    // JP 29.06.95: never erase a table standing in front of it.
    bool bSwap = false;
    const SwTableNode * pWasInTableNd = SwCursorShell::IsCursorInTable();

    if( SwCursorShell::IsSttPara())
    {
        // Start/EndAllAction to avoid cursor flickering
        UnoActionContext c(GetDoc());
        SwCursorShell::Push();

        // #i4032# Don't actually call a 'delete' if we
        // changed the table cell, compare DelRight().
        const SwStartNode * pSNdOld = pWasInTableNd ?
                                      GetCursor()->GetPointNode().FindTableBoxStartNode() :
                                      nullptr;

        // If the cursor is at the beginning of a paragraph, try to step
        // backwards. On failure we are done.
        bool bDoSomething = SwCursorShell::Left(1,SwCursorSkipMode::Chars);

        if (bDoSomething)
        {
            // If the cursor entered or left a table (or both) we are done.
            const SwTableNode* pIsInTableNd = SwCursorShell::IsCursorInTable();
            bDoSomething = pIsInTableNd == pWasInTableNd;

            if (bDoSomething)
            {
                const SwStartNode* pSNdNew = pIsInTableNd ?
                    GetCursor()->GetPointNode().FindTableBoxStartNode() :
                    nullptr;

                // #i4032# Don't actually call a 'delete' if we
                // changed the table cell, compare DelRight().
                bDoSomething = pSNdOld == pSNdNew;
            }
        }

        if (!bDoSomething)
        {
            // tdf#115132 Restore previous position and we are done
            SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);
            return false;
        }

        SwCursorShell::Pop(SwCursorShell::PopMode::DeleteStack);

        OpenMark();
        SwCursorShell::Right(1,SwCursorSkipMode::Chars);
        SwCursorShell::SwapPam();
        bSwap = true;
    }
    else
    {
        // If we are just to the right to a fieldmark, then remove it completely
        const SwPosition* pCurPos = GetCursor()->GetPoint();
        SwPosition aPrevChar(*pCurPos->GetContentNode(), pCurPos->GetContentIndex() - 1);
        sw::mark::IFieldmark* pFm = getIDocumentMarkAccess()->getFieldmarkAt(aPrevChar);
        if (pFm && pFm->GetMarkEnd() == *pCurPos)
        {
            mxDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
            IDocumentMarkAccess::DeleteFieldmarkCommand(*pFm);
            getIDocumentMarkAccess()->deleteMark(pFm);
            mxDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
            return true;
        }

        OpenMark();
        SwCursorShell::Left(1, SwCursorSkipMode::Chars);

        // If we are deleting a variation selector, we want to delete the
        // whole sequence.
        sal_uInt32 nCode = GetChar(false);
        if ( rtl::isSurrogate( nCode ) )
        {
            OUString sStr = GetSelText();
            nCode = sStr.iterateCodePoints( &o3tl::temporary(sal_Int32(0)) );
        }

        if ( unicode::isVariationSelector( nCode ) )
        {
            SwCursorShell::Push();
            SwCursorShell::Left(1, SwCursorSkipMode::Chars);
            SwCursorShell::Pop( SwCursorShell::PopMode::DeleteStack );
        }
    }
    bool bRet = Delete(true, true);
    if( !bRet && bSwap )
        SwCursorShell::SwapPam();
    CloseMark( bRet );

    return bRet;
}

bool SwWrtShell::DelRight(bool const isReplaceHeuristic)
{
        // Will be or'ed, if a tableselection exists;
        // will here be implemented on SelectionType::Table
    bool bRet = false;
    SelectionType nSelection = GetSelectionType();
    if(nSelection & SelectionType::TableCell)
        nSelection = SelectionType::Table;
    if(nSelection & SelectionType::Text)
        nSelection = SelectionType::Text;

    switch( nSelection & ~SelectionType::Ornament & ~SelectionType::Media )
    {
    case SelectionType::PostIt:
    case SelectionType::Text:
    case SelectionType::Table:
    case SelectionType::NumberList:
            //  If a selection exists, erase it.
        if( IsSelection() )
        {
            if( !IsBlockMode() || HasSelection() )
            {
                //OS: And once again Basic: SwActContext must be
                //leaved before EnterStdMode !
                {
                    SwActContext aActContext(this);
                    ResetCursorStack();
                    Delete(isReplaceHeuristic);
                    UpdateAttr();
                }
                if( IsBlockMode() )
                {
                    NormalizePam();
                    ClearMark();
                    EnterBlockMode();
                }
                else
                    EnterStdMode();
                bRet = true;
                break;
            }
            else
                EnterStdMode();
        }

        if (SwCursorShell::IsEndPara())
        {
            // Start/EndAllAction to avoid cursor flickering
            UnoActionContext c(GetDoc());

            const SwTableNode* pWasInTableNd = IsCursorInTable();
            // #108049# Save the startnode of the current cell
            const SwStartNode* pSNdOld = pWasInTableNd ?
                GetCursor()->GetPointNode().FindTableBoxStartNode() : nullptr;
            bool bCheckDelFull = SelectionType::Text & nSelection && SwCursorShell::IsSttPara();
            bool bDelFull = false;
            bool bDoNothing = false;

            // #i41424# Introduced a couple of
            // Push()-Pop() pairs here. The reason for this is that a
            // Right()-Left() combination does not make sure, that
            // the cursor will be in its initial state, because there
            // may be a numbering in front of the next paragraph.
            SwCursorShell::Push();

            if (SwCursorShell::Right(1, SwCursorSkipMode::Chars))
            {
                const SwTableNode* pCurrTableNd = IsCursorInTable();
                bDelFull = bCheckDelFull && pCurrTableNd && pCurrTableNd != pWasInTableNd;
                if (!bDelFull && (IsCursorInTable() || (pCurrTableNd != pWasInTableNd)))
                {
                    // #108049# Save the startnode of the current cell.
                    // May be different to pSNdOld as we have moved.
                    const SwStartNode* pSNdNew = pCurrTableNd ?
                        GetCursor()->GetPointNode().FindTableBoxStartNode() : nullptr;

                    // tdf#115132 Only keep cursor position instead of deleting
                    // if we have moved to a different cell
                    bDoNothing = pSNdOld != pSNdNew;
                }
            }

            // restore cursor
            SwCursorShell::Pop(SwCursorShell::PopMode::DeleteCurrent);

            if (bDelFull)
            {
                DelFullPara();
                UpdateAttr();
            }
            if (bDelFull || bDoNothing)
                break;
        }

        {
            // If we are just ahead of a fieldmark, then remove it completely
            sw::mark::IFieldmark *const pFm = getIDocumentMarkAccess()->getFieldmarkAt(*GetCursor()->GetPoint());
            if (pFm && pFm->GetMarkStart() == *GetCursor()->GetPoint())
            {
                mxDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);
                IDocumentMarkAccess::DeleteFieldmarkCommand(*pFm);
                getIDocumentMarkAccess()->deleteMark(pFm);
                mxDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);
                bRet = true;
                break;
            }
        }

        OpenMark();
        SwCursorShell::Right(1, SwCursorSkipMode::Cells);
        bRet = Delete(true);
        CloseMark( bRet );
        break;

    case SelectionType::Frame:
    case SelectionType::Graphic:
    case SelectionType::Ole:
    case SelectionType::DrawObject:
    case SelectionType::DrawObjectEditMode:
    case SelectionType::DbForm:
        {
            // Group deletion of the object and its comment together
            // (also as-character anchor conversion at track changes)
            mxDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

            // #108205# Remember object's position.
            Point aTmpPt = GetObjRect().TopLeft();

            // Remember the anchor of the selected object before deletion.
            std::optional<SwPosition> oAnchor;
            RndStdIds eAnchorId = RndStdIds::FLY_AT_PARA;
            SwFlyFrame* pFly = GetSelectedFlyFrame();
            SwFrameFormat* pFormat = nullptr;
            if (pFly)
            {
                pFormat = pFly->GetFormat();
                if (pFormat)
                {
                    eAnchorId = pFormat->GetAnchor().GetAnchorId();
                    // to-character anchor conversion at track changes
                    if ( IsRedlineOn() && (eAnchorId != RndStdIds::FLY_AS_CHAR &&
                                           eAnchorId != RndStdIds::FLY_AT_CHAR) )
                    {
                        SfxItemSetFixed<RES_ANCHOR, RES_ANCHOR> aSet(GetAttrPool());
                        GetFlyFrameAttr(aSet);
                        SwFormatAnchor aAnch(RndStdIds::FLY_AT_CHAR);
                        aSet.Put(aAnch);
                        SetFlyFrameAttr(aSet);
                        eAnchorId = pFormat->GetAnchor().GetAnchorId();
                    }
                    if ((eAnchorId == RndStdIds::FLY_AS_CHAR || eAnchorId == RndStdIds::FLY_AT_CHAR)
                        && pFormat->GetAnchor().GetAnchorNode())
                    {
                        oAnchor.emplace(*pFormat->GetAnchor().GetContentAnchor());
                        // set cursor before the anchor point
                        if ( IsRedlineOn() )
                            *GetCurrentShellCursor().GetPoint() = *oAnchor;
                    }
                }
            }

            // track changes: create redline at anchor point of the image to record the deletion
            if ( IsRedlineOn() && oAnchor && SelectionType::Graphic & nSelection && pFormat &&
                    ( eAnchorId == RndStdIds::FLY_AT_CHAR || eAnchorId == RndStdIds::FLY_AS_CHAR ) )
            {
                sal_Int32 nRedlineLength = 1;
                // create a double CH_TXT_TRACKED_DUMMY_CHAR anchor point of the image to record the
                // deletion, if needed (otherwise use the existing anchor point of the image anchored
                // *as* character)
                if ( eAnchorId == RndStdIds::FLY_AT_CHAR )
                {
                    nRedlineLength = 2;
                    LeaveSelFrameMode();
                    UnSelectFrame();
                    RedlineFlags eOld = GetRedlineFlags();
                    SetRedlineFlags( eOld | RedlineFlags::Ignore );
                    Insert( OUStringChar(CH_TXT_TRACKED_DUMMY_CHAR) +
                            OUStringChar(CH_TXT_TRACKED_DUMMY_CHAR) );
                    SwFormatAnchor anchor(RndStdIds::FLY_AT_CHAR);
                    SwCursorShell::Left(1, SwCursorSkipMode::Chars);
                    anchor.SetAnchor(GetCursor()->GetPoint());
                    GetDoc()->SetAttr(anchor, *pFormat);
                    SetRedlineFlags( eOld );
                    SwCursorShell::Left(1, SwCursorSkipMode::Chars);
                }
                OpenMark();
                SwCursorShell::Right(nRedlineLength, SwCursorSkipMode::Chars);
                bRet = Delete(false);
                CloseMark( bRet );
            }
            else
                DelSelectedObj();

            if (oAnchor)
            {
                SwTextNode* pTextNode = oAnchor->GetNode().GetTextNode();
                if (pTextNode)
                {
                    const SwTextField* pField(
                        pTextNode->GetFieldTextAttrAt(oAnchor->GetContentIndex(), ::sw::GetTextAttrMode::Default));
                    if (pField
                        && dynamic_cast<const SwPostItField*>(pField->GetFormatField().GetField()))
                    {
                        // Remove the comment of the deleted object.
                        *GetCurrentShellCursor().GetPoint() = *oAnchor;
                        DelRight();
                    }
                }
            }

            mxDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::EMPTY, nullptr);

            // #108205# Set cursor to remembered position.
            SetCursor(&aTmpPt);

            LeaveSelFrameMode();
            UnSelectFrame();
            OSL_ENSURE( !IsFrameSelected(),
                    "<SwWrtShell::DelRight(..)> - <SwWrtShell::UnSelectFrame()> should unmark all objects" );
            // leave draw mode, if necessary.
            {
                if (GetView().GetDrawFuncPtr())
                {
                    GetView().GetDrawFuncPtr()->Deactivate();
                    GetView().SetDrawFuncPtr(nullptr);
                }
                if ( GetView().IsDrawMode() )
                {
                    GetView().LeaveDrawCreate();
                }
            }
        }

        // <IsFrameSelected()> can't be true - see above.
        {
            nSelection = GetSelectionType();
            if ( SelectionType::Frame & nSelection ||
                 SelectionType::Graphic & nSelection ||
                 SelectionType::Ole & nSelection ||
                 SelectionType::DrawObject & nSelection )
            {
                EnterSelFrameMode();
                GotoNextFly();
            }
        }
        bRet = true;
        break;
    default: break;
    }
    return bRet;
}

void SwWrtShell::DelToEndOfPara()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(GoCurrPara,fnParaEnd))
    {
        Pop(SwCursorShell::PopMode::DeleteCurrent);
        return;
    }
    bool bRet = Delete(false);
    Pop(SwCursorShell::PopMode::DeleteCurrent);
    if( bRet )
        UpdateAttr();
}

void SwWrtShell::DelToStartOfPara()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(GoCurrPara,fnParaStart))
    {
        Pop(SwCursorShell::PopMode::DeleteCurrent);
        return;
    }
    bool bRet = Delete(false);
    Pop(SwCursorShell::PopMode::DeleteCurrent);
    if( bRet )
        UpdateAttr();
}

// All erase operations should work with Find instead with
// Nxt-/PrvDelim, because the latter works with Wrap Around
// -- that's probably not wished.

void SwWrtShell::DelToStartOfSentence()
{
    if(IsStartOfDoc())
        return;
    OpenMark();
    bool bRet = BwdSentence_() && Delete(false);
    CloseMark( bRet );
}

bool SwWrtShell::DelToEndOfSentence()
{
    if(IsEndOfDoc())
        return false;
    OpenMark();
    bool bRet(false);
    // fdo#60967: special case that is documented in help: delete
    // paragraph following table if cursor is at end of last cell in table
    if (IsEndOfTable())
    {
        Push();
        ClearMark();
        if (SwCursorShell::Right(1,SwCursorSkipMode::Chars))
        {
            SetMark();
            if (!IsEndPara()) // can only be at the end if it's empty
            {   // for an empty paragraph this would actually select the _next_
                SwCursorShell::MovePara(GoCurrPara, fnParaEnd);
            }
            if (!IsEndOfDoc()) // do not delete last paragraph in body text
            {
                bRet = DelFullPara();
            }
        }
        Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
    else
    {
        bRet = FwdSentence_() && Delete(false);
    }
    CloseMark( bRet );
    return bRet;
}

void SwWrtShell::DelNxtWord()
{
    if(IsEndOfDoc())
        return;
    SwActContext aActContext(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if(IsEndWrd() && !IsStartWord())
        NxtWrdForDelete(); // #i92468#
    if(IsStartWord() || IsEndPara())
        NxtWrdForDelete(); // #i92468#
    else
        EndWrd();

    bool bRet = Delete(false);
    if( bRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
}

void SwWrtShell::DelPrvWord()
{
    if(IsStartOfDoc())
        return;
    SwActContext aActContext(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if ( !IsStartWord() ||
         !PrvWrdForDelete() ) // #i92468#
    {
        if (IsEndWrd() || IsSttPara())
            PrvWrdForDelete(); // #i92468#
        else
            SttWrd();
    }
    bool bRet = Delete(false);
    if( bRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
