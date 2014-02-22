/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <wrtsh.hxx>
#include <crsskip.hxx>
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


bool SwWrtShell::TryRemoveIndent()
{
    bool bResult = false;

    SfxItemSet aAttrSet(GetAttrPool(), RES_LR_SPACE, RES_LR_SPACE);
    GetCurAttr(aAttrSet);

    SvxLRSpaceItem aItem = (const SvxLRSpaceItem &)aAttrSet.Get(RES_LR_SPACE);
    short aOldFirstLineOfst = aItem.GetTxtFirstLineOfst();

    if (aOldFirstLineOfst > 0)
    {
        aItem.SetTxtFirstLineOfst(0);
        bResult = true;
    }
    else if (aOldFirstLineOfst < 0)
    {
        aItem.SetTxtFirstLineOfst(0);
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
        
    Push();
    ClearMark();
    SwCrsrShell::LeftMargin();
    SetMark();
    SwCrsrShell::RightMargin();

    long nRet = Delete();
    Pop(sal_False);
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
    
    int nSelType = GetSelectionType();
    const int nCmp = nsSelectionType::SEL_FRM | nsSelectionType::SEL_GRF | nsSelectionType::SEL_OLE | nsSelectionType::SEL_DRW;
    if( nCmp & nSelType )
    {
        
        Point aTmpPt = GetObjRect().TopLeft();

        DelSelectedObj();

        
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

    
    if ( IsSelection() )
    {
        if( !IsBlockMode() || HasSelection() )
        {
            
            
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

    
    bool bSwap = false;
    const SwTableNode * pWasInTblNd = SwCrsrShell::IsCrsrInTbl();

    if( SwCrsrShell::IsSttPara())
    {
        
        
        const SwStartNode * pSNdOld = pWasInTblNd ?
                                      GetSwCrsr()->GetNode()->FindTableBoxStartNode() :
                                      0;

        
        
        if( !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
            return 0;

        
        
        const SwTableNode* pIsInTblNd = SwCrsrShell::IsCrsrInTbl();
        if( pIsInTblNd != pWasInTblNd )
            return 0;

        const SwStartNode* pSNdNew = pIsInTblNd ?
                                     GetSwCrsr()->GetNode()->FindTableBoxStartNode() :
                                     0;

        
        
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
        
        
    long nRet = 0;
    int nSelection = GetSelectionType();
    if(nSelection & nsSelectionType::SEL_TBL_CELLS)
        nSelection = nsSelectionType::SEL_TBL;
    if(nSelection & nsSelectionType::SEL_TXT)
        nSelection = nsSelectionType::SEL_TXT;

    const SwTableNode * pWasInTblNd = NULL;

    switch( nSelection & ~(nsSelectionType::SEL_BEZ) )
    {
    case nsSelectionType::SEL_POSTIT:
    case nsSelectionType::SEL_TXT:
    case nsSelectionType::SEL_TBL:
    case nsSelectionType::SEL_NUM:
            
        if( IsSelection() )
        {
            if( !IsBlockMode() || HasSelection() )
            {
                
                
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

        pWasInTblNd = IsCrsrInTbl();

        if( nsSelectionType::SEL_TXT & nSelection && SwCrsrShell::IsSttPara() &&
            SwCrsrShell::IsEndPara() )
        {
            
            SwCrsrShell::Push();

            bool bDelFull = false;
            if ( SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
            {
                const SwTableNode * pCurrTblNd = IsCrsrInTbl();
                bDelFull = pCurrTblNd && pCurrTblNd != pWasInTblNd;
            }

            
            SwCrsrShell::Pop( sal_False );

            if( bDelFull )
            {
                DelFullPara();
                UpdateAttr();
                break;
            }
        }

        {
            
            const SwStartNode * pSNdOld;
            pSNdOld = GetSwCrsr()->GetNode()->
                FindTableBoxStartNode();

            if ( SwCrsrShell::IsEndPara() )
            {
                
                
                
                
                
                SwCrsrShell::Push();

                if ( SwCrsrShell::Right(1, CRSR_SKIP_CHARS) )
                {
                    if (IsCrsrInTbl() || (pWasInTblNd != IsCrsrInTbl()))
                    {
                        /** #108049# Save the startnode of the current
                            cell. May be different to pSNdOld as we have
                            moved. */
                        const SwStartNode * pSNdNew = GetSwCrsr()
                            ->GetNode()->FindTableBoxStartNode();

                        /** #108049# Only move instead of deleting if we
                            have moved to a different cell */
                        if (pSNdOld != pSNdNew)
                        {
                            SwCrsrShell::Pop( sal_True );
                            break;
                        }
                    }
                }

                
                SwCrsrShell::Pop( sal_False );
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
            
            Point aTmpPt = GetObjRect().TopLeft();

            DelSelectedObj();

            
            SetCrsr(&aTmpPt);

            LeaveSelFrmMode();
            UnSelectFrm();
            
            OSL_ENSURE( !IsFrmSelected(),
                    "<SwWrtShell::DelRight(..)> - <SwWrtShell::UnSelectFrm()> should unmark all objects" );
            
            
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
        Pop(sal_False);
        return 0;
    }
    long nRet = Delete();
    Pop(sal_False);
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
        Pop(sal_False);
        return 0;
    }
    long nRet = Delete();
    Pop(sal_False);
    if( nRet )
        UpdateAttr();
    return nRet;
}





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
    
    
    if (IsEndOfTable())
    {
        Push();
        ClearMark();
        if (SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
        {
            SetMark();
            SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);
            if (!IsEndOfDoc()) 
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
    if(IsEndWrd() && !IsSttWrd())
        _NxtWrdForDelete(); 
    if(IsSttWrd() || IsEndPara())
        _NxtWrdForDelete(); 
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
    if ( !IsSttWrd() ||
         !_PrvWrdForDelete() ) 
    {
        if( IsEndWrd() )
        {
            if ( _PrvWrdForDelete() ) 
            {
                
                short n = 0;
                while( ' ' == GetChar( sal_False, n ))
                    --n;

                if( ++n )
                    ExtendSelection( sal_False, -n );
            }
        }
        else if( IsSttPara())
            _PrvWrdForDelete(); 
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
