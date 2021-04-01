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

    SfxItemSet aAttrSet(GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>{});
    GetCurAttr(aAttrSet);

    SvxLRSpaceItem aItem = aAttrSet.Get(RES_LR_SPACE);
    short aOldFirstLineOfst = aItem.GetTextFirstLineOffset();

    if (aOldFirstLineOfst > 0)
    {
        aItem.SetTextFirstLineOffset(0);
        bResult = true;
    }
    else if (aOldFirstLineOfst < 0)
    {
        aItem.SetTextFirstLineOffset(0);
        aItem.SetLeft(aItem.GetLeft() + aOldFirstLineOfst);

        bResult = true;
    }
    else if (aItem.GetLeft() != 0)
    {
        aItem.SetLeft(0);
        bResult = true;
    }

    if (bResult)
    {
        aAttrSet.Put(aItem);
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

    bool bRet = Delete();
    Pop(SwCursorShell::PopMode::DeleteCurrent);
    if( bRet )
        UpdateAttr();
}

void SwWrtShell::DelToStartOfLine()
{
    OpenMark();
    SwCursorShell::LeftMargin();
    bool bRet = Delete();
    CloseMark( bRet );
}

void SwWrtShell::DelToEndOfLine()
{
    OpenMark();
    SwCursorShell::RightMargin();
    bool bRet = Delete();
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
                Delete();
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
                                      GetSwCursor()->GetNode().FindTableBoxStartNode() :
                                      nullptr;

        // If the cursor is at the beginning of a paragraph, try to step
        // backwards. On failure we are done.
        bool bDoSomething = SwCursorShell::Left(1,CRSR_SKIP_CHARS);

        if (bDoSomething)
        {
            // If the cursor entered or left a table (or both) we are done.
            const SwTableNode* pIsInTableNd = SwCursorShell::IsCursorInTable();
            bDoSomething = pIsInTableNd == pWasInTableNd;

            if (bDoSomething)
            {
                const SwStartNode* pSNdNew = pIsInTableNd ?
                    GetSwCursor()->GetNode().FindTableBoxStartNode() :
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
        SwCursorShell::Right(1,CRSR_SKIP_CHARS);
        SwCursorShell::SwapPam();
        bSwap = true;
    }
    else
    {
        // If we are just to the right to a fieldmark, then remove it completely
        const SwPosition* pCurPos = GetCursor()->GetPoint();
        SwPosition aPrevChar(*pCurPos);
        --aPrevChar.nContent;
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
        SwCursorShell::Left(1, CRSR_SKIP_CHARS);
        if (SvtScriptType::ASIAN == GetScriptType())
        {
            sal_uInt32 nCode = GetChar(false);
            if ( rtl::isSurrogate( nCode ) )
            {
                OUString sStr = GetSelText();
                nCode = sStr.iterateCodePoints( &o3tl::temporary(sal_Int32(0)) );
            }

            if ( unicode::isIVSSelector( nCode ) )
            {
                SwCursorShell::Push();
                SwCursorShell::Left(1, CRSR_SKIP_CHARS);
                OUString sStr = GetSelText();
                nCode = sStr.iterateCodePoints( &o3tl::temporary(sal_Int32(0)) );
                if ( unicode::isCJKIVSCharacter( nCode ) )
                    SwCursorShell::Pop( SwCursorShell::PopMode::DeleteStack );
                else
                    SwCursorShell::Pop( SwCursorShell::PopMode::DeleteCurrent ); // For the weak script.
            }
        }
    }
    bool bRet = Delete();
    if( !bRet && bSwap )
        SwCursorShell::SwapPam();
    CloseMark( bRet );
    if (!bRet)
    {   // false indicates HasReadonlySel failed
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetView().GetFrameWeld(), "modules/swriter/ui/inforeadonlydialog.ui"));
        std::unique_ptr<weld::MessageDialog> xInfo(xBuilder->weld_message_dialog("InfoReadonlyDialog"));
        xInfo->run();
    }
    return bRet;
}

bool SwWrtShell::DelRight()
{
        // Will be or'ed, if a tableselection exists;
        // will here be implemented on SelectionType::Table
    bool bRet = false;
    SelectionType nSelection = GetSelectionType();
    if(nSelection & SelectionType::TableCell)
        nSelection = SelectionType::Table;
    if(nSelection & SelectionType::Text)
        nSelection = SelectionType::Text;

    switch( nSelection & ~SelectionType::Ornament )
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
                    Delete();
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
                GetSwCursor()->GetNode().FindTableBoxStartNode() : nullptr;
            bool bCheckDelFull = SelectionType::Text & nSelection && SwCursorShell::IsSttPara();
            bool bDelFull = false;
            bool bDoNothing = false;

            // #i41424# Introduced a couple of
            // Push()-Pop() pairs here. The reason for this is that a
            // Right()-Left() combination does not make sure, that
            // the cursor will be in its initial state, because there
            // may be a numbering in front of the next paragraph.
            SwCursorShell::Push();

            if (SwCursorShell::Right(1, CRSR_SKIP_CHARS))
            {
                const SwTableNode* pCurrTableNd = IsCursorInTable();
                bDelFull = bCheckDelFull && pCurrTableNd && pCurrTableNd != pWasInTableNd;
                if (!bDelFull && (IsCursorInTable() || (pCurrTableNd != pWasInTableNd)))
                {
                    // #108049# Save the startnode of the current cell.
                    // May be different to pSNdOld as we have moved.
                    const SwStartNode* pSNdNew = pCurrTableNd ?
                        GetSwCursor()->GetNode().FindTableBoxStartNode() : nullptr;

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
        SwCursorShell::Right(1, CRSR_SKIP_CELLS);
        bRet = Delete();
        CloseMark( bRet );
        if (!bRet)
        {   // false indicates HasReadonlySel failed
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetView().GetFrameWeld(), "modules/swriter/ui/inforeadonlydialog.ui"));
            std::unique_ptr<weld::MessageDialog> xInfo(xBuilder->weld_message_dialog("InfoReadonlyDialog"));
            xInfo->run();
        }
        break;

    case SelectionType::Frame:
    case SelectionType::Graphic:
    case SelectionType::Ole:
    case SelectionType::DrawObject:
    case SelectionType::DrawObjectEditMode:
    case SelectionType::DbForm:
        {
            // #108205# Remember object's position.
            Point aTmpPt = GetObjRect().TopLeft();

            // Remember the anchor of the selected object before deletion.
            std::unique_ptr<SwPosition> pAnchor;
            SwFlyFrame* pFly = GetSelectedFlyFrame();
            if (pFly)
            {
                SwFrameFormat* pFormat = pFly->GetFormat();
                if (pFormat)
                {
                    RndStdIds eAnchorId = pFormat->GetAnchor().GetAnchorId();
                    if ((eAnchorId == RndStdIds::FLY_AS_CHAR || eAnchorId == RndStdIds::FLY_AT_CHAR)
                        && pFormat->GetAnchor().GetContentAnchor())
                    {
                        pAnchor.reset(new SwPosition(*pFormat->GetAnchor().GetContentAnchor()));
                    }
                }
            }

            // Group deletion of the object and its comment together.
            mxDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::EMPTY, nullptr);

            DelSelectedObj();

            if (pAnchor)
            {
                SwTextNode* pTextNode = pAnchor->nNode.GetNode().GetTextNode();
                if (pTextNode)
                {
                    const SwTextField* pField(
                        pTextNode->GetFieldTextAttrAt(pAnchor->nContent.GetIndex(), true));
                    if (pField
                        && dynamic_cast<const SwPostItField*>(pField->GetFormatField().GetField()))
                    {
                        // Remove the comment of the deleted object.
                        *GetCurrentShellCursor().GetPoint() = *pAnchor;
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
    bool bRet = Delete();
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
    bool bRet = Delete();
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
    bool bRet = BwdSentence_() && Delete();
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
        if (SwCursorShell::Right(1,CRSR_SKIP_CHARS))
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
        bRet = FwdSentence_() && Delete();
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

    bool bRet = Delete();
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
    bool bRet = Delete();
    if( bRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
