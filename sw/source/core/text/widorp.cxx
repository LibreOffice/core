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

#include "hintids.hxx"

#include "layfrm.hxx"
#include "ftnboss.hxx"
#include "ndtxt.hxx"
#include "paratr.hxx"
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/spltitem.hxx>
#include <frmatr.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <rowfrm.hxx>

#include "widorp.hxx"
#include "txtfrm.hxx"
#include "itrtxt.hxx"
#include "sectfrm.hxx"
#include "ftnfrm.hxx"

#undef WIDOWTWIPS


namespace
{


inline bool IsNastyFollow( const SwTxtFrm *pFrm )
{
    OSL_ENSURE( !pFrm->IsFollow() || !pFrm->GetPrev() ||
            ((const SwTxtFrm*)pFrm->GetPrev())->GetFollow() == pFrm,
            "IsNastyFollow: Was ist denn hier los?" );
    return  pFrm->IsFollow() && pFrm->GetPrev();
}

}

/*************************************************************************
 *                  SwTxtFrmBreak::SwTxtFrmBreak()
 *************************************************************************/

SwTxtFrmBreak::SwTxtFrmBreak( SwTxtFrm *pNewFrm, const SwTwips nRst )
    : nRstHeight(nRst), pFrm(pNewFrm)
{
    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )
    nOrigin = (pFrm->*fnRect->fnGetPrtTop)();
    SwSectionFrm* pSct;
    bKeep = !pFrm->IsMoveable() || IsNastyFollow( pFrm ) ||
            ( pFrm->IsInSct() && (pSct=pFrm->FindSctFrm())->Lower()->IsColumnFrm()
              && !pSct->MoveAllowed( pFrm ) ) ||
            !pFrm->GetTxtNode()->GetSwAttrSet().GetSplit().GetValue() ||
            pFrm->GetTxtNode()->GetSwAttrSet().GetKeep().GetValue();
    bBreak = false;

    if( !nRstHeight && !pFrm->IsFollow() && pFrm->IsInFtn() && pFrm->HasPara() )
    {
        nRstHeight = pFrm->GetFtnFrmHeight();
        nRstHeight += (pFrm->Prt().*fnRect->fnGetHeight)() -
                      (pFrm->Frm().*fnRect->fnGetHeight)();
        if( nRstHeight < 0 )
            nRstHeight = 0;
    }

    UNDO_SWAP( pFrm )
}

/* BP 18.6.93: Widows.
 * In contrast to the first implementation the Widows are not calculated
 * in advance but detected when formatting the split Follow.
 * In Master the Widows-calculation is dropped completely
 * (nWidows is manipulated). If the Follow detects that the
 * Widows rule applies it sends a Prepare to its predecessor.
 * A special problem is when the Widow rule applies but in Master
 * there are some lines available.
 *
 */

/*************************************************************************
 *                  SwTxtFrmBreak::IsInside()
 *************************************************************************/

/* BP(22.07.92): Calculation of Widows and Orphans.
 * The method returns true if one of the rules matches.
 *
 * One difficulty with Widows and different formats between
 * Master- and Follow-Frame:
 * Example: If the first column is 3cm and the second is 4cm and
 * Widows is set to 3, the decision if the Widows rule matches can not
 * be done until the Follow is formated. Unfortunately this is crucial
 * to decide if the whole paragraph goes to the next page or not.
 */

bool SwTxtFrmBreak::IsInside( SwTxtMargin &rLine ) const
{
    bool bFit = false;

    SWAP_IF_SWAPPED( pFrm )
    SWRECTFN( pFrm )
    

    SwTwips nTmpY;
    if ( pFrm->IsVertical() )
        nTmpY = pFrm->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , nOrigin );

    
    nLineHeight += (pFrm->*fnRect->fnGetBottomMargin)();

    if( nRstHeight )
        bFit = nRstHeight >= nLineHeight;
    else
    {
        
        SwTwips nHeight =
            (*fnRect->fnYDiff)( (pFrm->GetUpper()->*fnRect->fnGetPrtBottom)(), nOrigin );
        
        bFit = nHeight >= nLineHeight;

        
        if ( !bFit )
        {
            if ( rLine.GetNext() &&
                 pFrm->IsInTab() && !pFrm->GetFollow() && !pFrm->GetIndNext() )
            {
                
                
                nHeight += pFrm->CalcAddLowerSpaceAsLastInTableCell();
                bFit = nHeight >= nLineHeight;
            }
        }
        
        if( !bFit )
        {
            
            
            
            nHeight += pFrm->GrowTst( LONG_MAX );

            
            
            
            bFit = nHeight >= nLineHeight;
        }
    }

    UNDO_SWAP( pFrm );

    return bFit;
}

/*************************************************************************
 *                  SwTxtFrmBreak::IsBreakNow()
 *************************************************************************/

bool SwTxtFrmBreak::IsBreakNow( SwTxtMargin &rLine )
{
    SWAP_IF_SWAPPED( pFrm )

    
    
    if( bKeep || IsInside( rLine ) )
        bBreak = false;
    else
    {
        /* This class assumes that the SwTxtMargin is processed from Top to
         * Bottom. Because of performance reasons we stop splitting in the
         * following cases:
         * If only one line does not fit.
         * Special case: with DummyPortions there is LineNr == 1, though we
         * want to split.
         */
        

        bool bFirstLine = 1 == rLine.GetLineNr() && !rLine.GetPrev();
        bBreak = true;
        if( ( bFirstLine && pFrm->GetIndPrev() )
            || ( rLine.GetLineNr() <= rLine.GetDropLines() ) )
        {
            bKeep = true;
            bBreak = false;
        }
        else if(bFirstLine && pFrm->IsInFtn() && !pFrm->FindFtnFrm()->GetPrev())
        {
            SwLayoutFrm* pTmp = pFrm->FindFtnBossFrm()->FindBodyCont();
            if( !pTmp || !pTmp->Lower() )
                bBreak = false;
        }
    }

    UNDO_SWAP( pFrm )

    return bBreak;
}


void SwTxtFrmBreak::SetRstHeight( const SwTxtMargin &rLine )
{
    
    SWRECTFN( pFrm )

    nRstHeight = (pFrm->*fnRect->fnGetBottomMargin)();

    if ( bVert )
    
    {
           if ( pFrm->IsVertLR() )
              nRstHeight = (*fnRect->fnYDiff)( pFrm->SwitchHorizontalToVertical( rLine.Y() ) , nOrigin );
           else
               nRstHeight += nOrigin - pFrm->SwitchHorizontalToVertical( rLine.Y() );
    }
    else
        nRstHeight += rLine.Y() - nOrigin;
}

/*************************************************************************
 *                  WidowsAndOrphans::WidowsAndOrphans()
 *************************************************************************/

WidowsAndOrphans::WidowsAndOrphans( SwTxtFrm *pNewFrm, const SwTwips nRst,
    bool bChkKeep   )
    : SwTxtFrmBreak( pNewFrm, nRst ), nWidLines( 0 ), nOrphLines( 0 )
{
    SWAP_IF_SWAPPED( pFrm )

    if( bKeep )
    {
        
        
        if( bChkKeep && !pFrm->GetPrev() && !pFrm->IsInFtn() &&
            pFrm->IsMoveable() &&
            ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            bKeep = false;
        
        
        
        
        
        if( pFrm->IsFollow() )
            nWidLines = pFrm->GetTxtNode()->GetSwAttrSet().GetWidows().GetValue();
    }
    else
    {
        const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
        const SvxOrphansItem  &rOrph = rSet.GetOrphans();
        if ( rOrph.GetValue() > 1 )
            nOrphLines = rOrph.GetValue();
        if ( pFrm->IsFollow() )
            nWidLines = rSet.GetWidows().GetValue();

    }

    if ( bKeep || nWidLines || nOrphLines )
    {
        bool bResetFlags = false;

        if ( pFrm->IsInTab() )
        {
            
            
            if ( pFrm->GetNextCellLeaf( MAKEPAGE_NONE ) || pFrm->IsInFollowFlowRow() )
            {
                const SwFrm* pTmpFrm = pFrm->GetUpper();
                while ( !pTmpFrm->IsRowFrm() )
                    pTmpFrm = pTmpFrm->GetUpper();
                if ( static_cast<const SwRowFrm*>(pTmpFrm)->IsRowSplitAllowed() )
                    bResetFlags = true;
            }
        }

        if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
        {
            
            
            SwFtnFrm *pFtn = pFrm->FindFtnFrm();
            const bool bFt = !pFtn->GetAttr()->GetFtn().IsEndNote();
            if( !pFtn->GetPrev() &&
                pFtn->FindFtnBossFrm( bFt ) != pFtn->GetRef()->FindFtnBossFrm( bFt )
                && ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            {
                bResetFlags = true;
            }
        }

        if ( bResetFlags )
        {
            bKeep = false;
            nOrphLines = 0;
            nWidLines = 0;
        }
    }

    UNDO_SWAP( pFrm )
}

/*************************************************************************
 *                  WidowsAndOrphans::FindBreak()
 *************************************************************************/

/* The Find*-Methodes do not only search, but adjust the SwTxtMargin to the
 * line where the paragraph should have a break and truncate the paragraph there.
 * FindBreak()
 */

bool WidowsAndOrphans::FindBreak( SwTxtFrm *pFrame, SwTxtMargin &rLine,
    bool bHasToFit )
{
    
    
    OSL_ENSURE( pFrm == pFrame, "<WidowsAndOrphans::FindBreak> - pFrm != pFrame" );

    SWAP_IF_SWAPPED( pFrm )

    bool bRet = true;
    MSHORT nOldOrphans = nOrphLines;
    if( bHasToFit )
        nOrphLines = 0;
    rLine.Bottom();
    
    if( !IsBreakNowWidAndOrp( rLine ) )
        bRet = false;
    if( !FindWidows( pFrame, rLine ) )
    {
        bool bBack = false;
        
        while( IsBreakNowWidAndOrp( rLine ) )
        {
            if( rLine.PrevLine() )
                bBack = true;
            else
                break;
        }
        
        
        
        
        if( rLine.GetLineNr() <= nOldOrphans &&
            rLine.GetInfo().GetParaPortion()->IsDummy() &&
            ( ( bHasToFit && bRet ) || IsBreakNow( rLine ) ) )
            rLine.Top();

        rLine.TruncLines( true );
        bRet = bBack;
    }
    nOrphLines = nOldOrphans;

    UNDO_SWAP( pFrm )

    return bRet;
}

/*************************************************************************
 *                  WidowsAndOrphans::FindWidows()
 *************************************************************************/

/*  FindWidows positions the SwTxtMargin of the Master to the line where to
 *  break by examining and formatting the Follow.
 *  Returns true if the Widows-rule matches, that means that the
 *  paragraph should not be split (keep) !
 */

bool WidowsAndOrphans::FindWidows( SwTxtFrm *pFrame, SwTxtMargin &rLine )
{
    OSL_ENSURE( ! pFrame->IsVertical() || ! pFrame->IsSwapped(),
            "WidowsAndOrphans::FindWidows with swapped frame" );

    if( !nWidLines || !pFrame->IsFollow() )
        return false;

    rLine.Bottom();

    
    SwTxtFrm *pMaster = pFrame->FindMaster();
    OSL_ENSURE(pMaster, "+WidowsAndOrphans::FindWidows: Widows in a master?");
    if( !pMaster )
        return false;

    
    
    if( pMaster->GetOfst() == pFrame->GetOfst() )
        return false;

    
    SWRECTFN( pFrame )

    const SwTwips nDocPrtTop = (pFrame->*fnRect->fnGetPrtTop)();
    SwTwips nOldHeight;
    SwTwips nTmpY = rLine.Y() + rLine.GetLineHeight();

    if ( bVert )
    {
        nTmpY = pFrame->SwitchHorizontalToVertical( nTmpY );
        nOldHeight = -(pFrame->Prt().*fnRect->fnGetHeight)();
    }
    else
        nOldHeight = (pFrame->Prt().*fnRect->fnGetHeight)();

    const SwTwips nChg = (*fnRect->fnYDiff)( nTmpY, nDocPrtTop + nOldHeight );

    
    if( rLine.GetLineNr() >= nWidLines )
    {
        
        
        
        
        
        if( rLine.GetLineNr() > nWidLines && pFrame->IsJustWidow() )
        {
            
            
            
            if( !pMaster->IsLocked() && pMaster->GetUpper() )
            {
                const SwTwips nTmpRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                            ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
                if ( nTmpRstHeight >=
                     SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
                {
                    pMaster->Prepare( PREP_ADJUST_FRM );
                    pMaster->_InvalidateSize();
                    pMaster->InvalidatePage();
                }
            }

            pFrame->SetJustWidow( false );
        }
        return false;
    }

    
    
    
    

    if( 0 > nChg && !pMaster->IsLocked() && pMaster->GetUpper() )
    {
        SwTwips nTmpRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
                             ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if( nTmpRstHeight >= SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
        {
            pMaster->Prepare( PREP_ADJUST_FRM );
            pMaster->_InvalidateSize();
            pMaster->InvalidatePage();
            pFrame->SetJustWidow( false );
            return false;
        }
    }

    
    
    
    
    
    
    
    
    
    


    
    
    
    
    MSHORT nNeed = 1; 

    
    
    if ( !pMaster->GetIndPrev() )
    {
        sal_uLong nLines = pMaster->GetThisLines();
        if(nLines == 0 && pMaster->HasPara())
        {
            const SwParaPortion *pMasterPara = pMaster->GetPara();
            if(pMasterPara && pMasterPara->GetNext())
                nLines = 2;
        }
        if( nLines <= nNeed )
            return false;
    }

    pMaster->Prepare( PREP_WIDOWS, (void*)&nNeed );
    return true;
}

/*************************************************************************
 *                  WidowsAndOrphans::WouldFit()
 *************************************************************************/

bool WidowsAndOrphans::WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight, bool bTst )
{
    
    

    
    OSL_ENSURE( !rLine.GetNext(), "WouldFit: aLine::Bottom missed!" );
    MSHORT nLineCnt = rLine.GetLineNr();

    
    const MSHORT nMinLines = std::max( GetOrphansLines(), rLine.GetDropLines() );
    if ( nLineCnt < nMinLines )
        return false;

    rLine.Top();
    SwTwips nLineSum = rLine.GetLineHeight();

    while( nMinLines > rLine.GetLineNr() )
    {
        if( !rLine.NextLine() )
            return false;
        nLineSum += rLine.GetLineHeight();
    }

    
    if( !IsInside( rLine ) )
        return false;

    
    if( !nWidLines && !pFrm->IsFollow() )
    {
        
        
        
        
        
        const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
        nWidLines = rSet.GetWidows().GetValue();
    }

    
    
    
    
    
    
    
    if( bTst || nLineCnt - nMinLines >= GetWidowsLines() )
    {
        if( rMaxHeight >= nLineSum )
        {
            rMaxHeight -= nLineSum;
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
