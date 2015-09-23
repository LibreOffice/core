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
#include <crsskip.hxx>
#include <swcrsr.hxx>
#include <editeng/lrspitem.hxx>
// #134369#
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

long SwWrtShell::DelLine()
{
    SwActContext aActContext(this);
    ResetCursorStack();
        // remember the old cursor
    Push();
    ClearMark();
    SwCrsrShell::LeftMargin();
    SetMark();
    SwCrsrShell::RightMargin();

    long nRet = Delete();
    Pop(false);
    if( nRet )
        UpdateAttr();
    return nRet;
}

long SwWrtShell::DelToStartOfLine()
{
    OpenMark();
    SwCrsrShell::LeftMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
    return nRet;
}

long SwWrtShell::DelToEndOfLine()
{
    OpenMark();
    SwCrsrShell::RightMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
    return 1;
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
        SetCrsr(&aTmpPt);

        LeaveSelFrmMode();
        UnSelectFrm();

        nSelType = GetSelectionType();
        if ( nCmp & nSelType )
        {
            EnterSelFrmMode();
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
    const SwTableNode * pWasInTableNd = SwCrsrShell::IsCrsrInTable();

    if( SwCrsrShell::IsSttPara())
    {
        // #i4032# Don't actually call a 'delete' if we
        // changed the table cell, compare DelRight().
        const SwStartNode * pSNdOld = pWasInTableNd ?
                                      GetSwCrsr()->GetNode().FindTableBoxStartNode() :
                                      0;

        // If the cursor is at the beginning of a paragraph, try to step
        // backwards. On failure we are done.
        if( !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
            return 0;

        // If the cursor entered or left a table (or both) we are done. No step
        // back.
        const SwTableNode* pIsInTableNd = SwCrsrShell::IsCrsrInTable();
        if( pIsInTableNd != pWasInTableNd )
            return 0;

        const SwStartNode* pSNdNew = pIsInTableNd ?
                                     GetSwCrsr()->GetNode().FindTableBoxStartNode() :
                                     0;

        // #i4032# Don't actually call a 'delete' if we
        // changed the table cell, compare DelRight().
        if ( pSNdOld != pSNdNew )
            return 0;

        OpenMark();
        SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
        SwCrsrShell::SwapPam();
        bSwap = true;
    }
    else
    {
        OpenMark();
        SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
    }
    long nRet = Delete();
    if( !nRet && bSwap )
        SwCrsrShell::SwapPam();
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

    const SwTableNode * pWasInTableNd = NULL;

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

        pWasInTableNd = IsCrsrInTable();

        if( nsSelectionType::SEL_TXT & nSelection && SwCrsrShell::IsSttPara() &&
            SwCrsrShell::IsEndPara() )
        {
            // save cursor
            SwCrsrShell::Push();

            bool bDelFull = false;
            if ( SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
            {
                const SwTableNode * pCurrTableNd = IsCrsrInTable();
                bDelFull = pCurrTableNd && pCurrTableNd != pWasInTableNd;
            }

            // restore cursor
            SwCrsrShell::Pop( false );

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
            pSNdOld = GetSwCrsr()->GetNode().FindTableBoxStartNode();

            if ( SwCrsrShell::IsEndPara() )
            {
                // #i41424# Introduced a couple of
                // Push()-Pop() pairs here. The reason for this is that a
                // Right()-Left() combination does not make sure, that
                // the cursor will be in its initial state, because there
                // may be a numbering in front of the next paragraph.
                SwCrsrShell::Push();

                if ( SwCrsrShell::Right(1, CRSR_SKIP_CHARS) )
                {
                    if (IsCrsrInTable() || (pWasInTableNd != IsCrsrInTable()))
                    {
                        /** #108049# Save the startnode of the current
                            cell. May be different to pSNdOld as we have
                            moved. */
                        const SwStartNode * pSNdNew = GetSwCrsr()
                            ->GetNode().FindTableBoxStartNode();

                        /** #108049# Only move instead of deleting if we
                            have moved to a different cell */
                        if (pSNdOld != pSNdNew)
                        {
                            SwCrsrShell::Pop();
                            break;
                        }
                    }
                }

                // restore cursor
                SwCrsrShell::Pop( false );
            }
        }

        OpenMark();
        SwCrsrShell::Right(1,CRSR_SKIP_CELLS);
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
            SetCrsr(&aTmpPt);

            LeaveSelFrmMode();
            UnSelectFrm();
            // #134369#
            OSL_ENSURE( !IsFrmSelected(),
                    "<SwWrtShell::DelRight(..)> - <SwWrtShell::UnSelectFrm()> should unmark all objects" );
            // #134369#
            // leave draw mode, if necessary.
            {
                if (GetView().GetDrawFuncPtr())
                {
                    GetView().GetDrawFuncPtr()->Deactivate();
                    GetView().SetDrawFuncPtr(NULL);
                }
                if ( GetView().IsDrawMode() )
                {
                    GetView().LeaveDrawCreate();
                }
            }
        }

        // #134369#
        // <IsFrmSelected()> can't be true - see above.
        {
            nSelection = GetSelectionType();
            if ( nsSelectionType::SEL_FRM & nSelection ||
                 nsSelectionType::SEL_GRF & nSelection ||
                 nsSelectionType::SEL_OLE & nSelection ||
                 nsSelectionType::SEL_DRW & nSelection )
            {
                EnterSelFrmMode();
                GotoNextFly();
            }
        }
        nRet = 1;
        break;
    }
    return nRet;
}

long SwWrtShell::DelToEndOfPara()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(fnParaCurr,fnParaEnd))
    {
        Pop(false);
        return 0;
    }
    long nRet = Delete();
    Pop(false);
    if( nRet )
        UpdateAttr();
    return nRet;
}

long SwWrtShell::DelToStartOfPara()
{
    SwActContext aActContext(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(fnParaCurr,fnParaStart))
    {
        Pop(false);
        return 0;
    }
    long nRet = Delete();
    Pop(false);
    if( nRet )
        UpdateAttr();
    return nRet;
}

// All erase operations should work with Find instead with
// Nxt-/PrvDelim, because the latter works with Wrap Around
// -- that's probably not wished.

long SwWrtShell::DelToStartOfSentence()
{
    if(IsStartOfDoc())
        return 0;
    OpenMark();
    long nRet = _BwdSentence() ? Delete() : 0;
    CloseMark( 0 != nRet );
    return nRet;
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
        if (SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
        {
            SetMark();
            if (!IsEndPara()) // can only be at the end if it's empty
            {   // for an empty paragraph this would actually select the _next_
                SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);
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
        nRet = _FwdSentence() ? Delete() : 0;
    }
    CloseMark( 0 != nRet );
    return nRet;
}

long SwWrtShell::DelNxtWord()
{
    if(IsEndOfDoc())
        return 0;
    SwActContext aActContext(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if(IsEndWrd() && !IsStartWord())
        _NxtWrdForDelete(); // #i92468#
    if(IsStartWord() || IsEndPara())
        _NxtWrdForDelete(); // #i92468#
    else
        _EndWrd();

    long nRet = Delete();
    if( nRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
    return nRet;
}

long SwWrtShell::DelPrvWord()
{
    if(IsStartOfDoc())
        return 0;
    SwActContext aActContext(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if ( !IsStartWord() ||
         !_PrvWrdForDelete() ) // #i92468#
    {
        if (IsEndWrd() || IsSttPara())
            _PrvWrdForDelete(); // #i92468#
        else
            _SttWrd();
    }
    long nRet = Delete();
    if( nRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
