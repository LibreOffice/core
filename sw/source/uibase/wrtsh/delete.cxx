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

    SfxItemSet aAttrSet(GetAttrPool(), RES_LR_SPACE, RES_LR_SPACE);
    GetCurAttr(aAttrSet);

    SvxLRSpaceItem aItem = static_cast<const SvxLRSpaceItem &>(aAttrSet.Get(RES_LR_SPACE));
    short aOldFirstLineOfst = aItem.GetTextFirstLineOfst();

    if (aOldFirstLineOfst > 0)
    {
        aItem.SetTextFirstLineOfst(0);
        bResult = true;
    }
    else if (aOldFirstLineOfst < 0)
    {
        aItem.SetTextFirstLineOfst(0);
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

    long nRet = Delete();
    Pop(false);
    if( nRet )
        UpdateAttr();
}

void SwWrtShell::DelToStartOfLine()
{
    OpenMark();
    SwCursorShell::LeftMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
}

void SwWrtShell::DelToEndOfLine()
{
    OpenMark();
    SwCursorShell::RightMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
}

long SwWrtShell::DelLeft()
{
    // If it's a Fly, throw it away
    int nSelType = GetSelectionType();
    const int nCmp = nsSelectionType::SEL_FRM | nsSelectionType::SEL_GRF | nsSelectionType::SEL_OLE | nsSelectionType::SEL_DRW;
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

        return 1L;
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
            return 1L;
        }
        else
            EnterStdMode();
    }

    // JP 29.06.95: never erase a table standing in front of it.
    bool bSwap = false;
    const SwTableNode * pWasInTableNd = SwCursorShell::IsCursorInTable();

    if( SwCursorShell::IsSttPara())
    {
        // #i4032# Don't actually call a 'delete' if we
        // changed the table cell, compare DelRight().
        const SwStartNode * pSNdOld = pWasInTableNd ?
                                      GetSwCursor()->GetNode().FindTableBoxStartNode() :
                                      nullptr;

        // If the cursor is at the beginning of a paragraph, try to step
        // backwards. On failure we are done.
        if( !SwCursorShell::Left(1,CRSR_SKIP_CHARS) )
            return 0;

        // If the cursor entered or left a table (or both) we are done. No step
        // back.
        const SwTableNode* pIsInTableNd = SwCursorShell::IsCursorInTable();
        if( pIsInTableNd != pWasInTableNd )
            return 0;

        const SwStartNode* pSNdNew = pIsInTableNd ?
                                     GetSwCursor()->GetNode().FindTableBoxStartNode() :
                                     nullptr;

        // #i4032# Don't actually call a 'delete' if we
        // changed the table cell, compare DelRight().
        if ( pSNdOld != pSNdNew )
            return 0;

        OpenMark();
        SwCursorShell::Right(1,CRSR_SKIP_CHARS);
        SwCursorShell::SwapPam();
        bSwap = true;
    }
    else
    {
        OpenMark();

        // If we are just to the right to a fieldmark, then remove it completely
        SwPosition* aCurPos = GetCursor()->GetPoint();
        SwPosition aPrevChar(*aCurPos);
        --aPrevChar.nContent;
        sw::mark::IFieldmark* pFm = getIDocumentMarkAccess()->getFieldmarkFor(aPrevChar);
        if (pFm && pFm->GetMarkEnd() == *aCurPos)
        {
            *aCurPos = pFm->GetMarkStart();
            getIDocumentMarkAccess()->deleteMark(pFm);
        }
        else
        {
            SwCursorShell::Left(1, CRSR_SKIP_CHARS);
        }
    }
    long nRet = Delete();
    if( !nRet && bSwap )
        SwCursorShell::SwapPam();
    CloseMark( 0 != nRet );
    return nRet;
}

long SwWrtShell::DelRight()
{
        // Will be or'ed, if a tableselection exists;
        // will here be implemented on nsSelectionType::SEL_TBL
    long nRet = 0;
    int nSelection = GetSelectionType();
    if(nSelection & nsSelectionType::SEL_TBL_CELLS)
        nSelection = nsSelectionType::SEL_TBL;
    if(nSelection & nsSelectionType::SEL_TXT)
        nSelection = nsSelectionType::SEL_TXT;

    const SwTableNode * pWasInTableNd = nullptr;

    switch( nSelection & ~(nsSelectionType::SEL_BEZ) )
    {
    case nsSelectionType::SEL_POSTIT:
    case nsSelectionType::SEL_TXT:
    case nsSelectionType::SEL_TBL:
    case nsSelectionType::SEL_NUM:
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
                nRet = 1L;
                break;
            }
            else
                EnterStdMode();
        }

        pWasInTableNd = IsCursorInTable();

        if( nsSelectionType::SEL_TXT & nSelection && SwCursorShell::IsSttPara() &&
            SwCursorShell::IsEndPara() )
        {
            // save cursor
            SwCursorShell::Push();

            bool bDelFull = false;
            if ( SwCursorShell::Right(1,CRSR_SKIP_CHARS) )
            {
                const SwTableNode * pCurrTableNd = IsCursorInTable();
                bDelFull = pCurrTableNd && pCurrTableNd != pWasInTableNd;
            }

            // restore cursor
            SwCursorShell::Pop( false );

            if( bDelFull )
            {
                DelFullPara();
                UpdateAttr();
                break;
            }
        }

        {
            // #108049# Save the startnode of the current cell
            const SwStartNode * pSNdOld;
            pSNdOld = GetSwCursor()->GetNode().FindTableBoxStartNode();

            if ( SwCursorShell::IsEndPara() )
            {
                // #i41424# Introduced a couple of
                // Push()-Pop() pairs here. The reason for this is that a
                // Right()-Left() combination does not make sure, that
                // the cursor will be in its initial state, because there
                // may be a numbering in front of the next paragraph.
                SwCursorShell::Push();

                if ( SwCursorShell::Right(1, CRSR_SKIP_CHARS) )
                {
                    if (IsCursorInTable() || (pWasInTableNd != IsCursorInTable()))
                    {
                        /** #108049# Save the startnode of the current
                            cell. May be different to pSNdOld as we have
                            moved. */
                        const SwStartNode * pSNdNew = GetSwCursor()
                            ->GetNode().FindTableBoxStartNode();

                        /** #108049# Only move instead of deleting if we
                            have moved to a different cell */
                        if (pSNdOld != pSNdNew)
                        {
                            SwCursorShell::Pop();
                            break;
                        }
                    }
                }

                // restore cursor
                SwCursorShell::Pop( false );
            }
        }

        OpenMark();

        {
            // If we are just ahead of a fieldmark, then remove it completely
            SwPosition* aCurPos = GetCursor()->GetPoint();
            sw::mark::IFieldmark* pFm = GetCurrentFieldmark();
            if (pFm && pFm->GetMarkStart() == *aCurPos)
            {
                *aCurPos = pFm->GetMarkEnd();
                getIDocumentMarkAccess()->deleteMark(pFm);
            }
            else
            {
                SwCursorShell::Right(1, CRSR_SKIP_CELLS);
            }
        }

        nRet = Delete();
        CloseMark( 0 != nRet );
        break;

    case nsSelectionType::SEL_FRM:
    case nsSelectionType::SEL_GRF:
    case nsSelectionType::SEL_OLE:
    case nsSelectionType::SEL_DRW:
    case nsSelectionType::SEL_DRW_TXT:
    case nsSelectionType::SEL_DRW_FORM:
        {
            // #108205# Remember object's position.
            Point aTmpPt = GetObjRect().TopLeft();

            DelSelectedObj();

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
            if ( nsSelectionType::SEL_FRM & nSelection ||
                 nsSelectionType::SEL_GRF & nSelection ||
                 nsSelectionType::SEL_OLE & nSelection ||
                 nsSelectionType::SEL_DRW & nSelection )
            {
                EnterSelFrameMode();
                GotoNextFly();
            }
        }
        nRet = 1;
        break;
    }
    return nRet;
}

void SwWrtShell::DelToEndOfPara()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(GoCurrPara,fnParaEnd))
    {
        Pop(false);
        return;
    }
    long nRet = Delete();
    Pop(false);
    if( nRet )
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
        Pop(false);
        return;
    }
    long nRet = Delete();
    Pop(false);
    if( nRet )
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
    long nRet = BwdSentence_() ? Delete() : 0;
    CloseMark( 0 != nRet );
}

long SwWrtShell::DelToEndOfSentence()
{
    if(IsEndOfDoc())
        return 0;
    OpenMark();
    long nRet(0);
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
                nRet = DelFullPara() ? 1 : 0;
            }
        }
        Pop(false);
    }
    else
    {
        nRet = FwdSentence_() ? Delete() : 0;
    }
    CloseMark( 0 != nRet );
    return nRet;
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

    long nRet = Delete();
    if( nRet )
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
    long nRet = Delete();
    if( nRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
