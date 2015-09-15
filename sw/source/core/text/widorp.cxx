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

// A Follow on the same page as its master is nasty.
inline bool IsNastyFollow( const SwTextFrm *pFrm )
{
    OSL_ENSURE( !pFrm->IsFollow() || !pFrm->GetPrev() ||
            static_cast<const SwTextFrm*>(pFrm->GetPrev())->GetFollow() == pFrm,
            "IsNastyFollow: Was ist denn hier los?" );
    return  pFrm->IsFollow() && pFrm->GetPrev();
}

}

SwTextFrmBreak::SwTextFrmBreak( SwTextFrm *pNewFrm, const SwTwips nRst )
    : nRstHeight(nRst), pFrm(pNewFrm)
{
    SwSwapIfSwapped swap(pFrm);
    SWRECTFN( pFrm )
    nOrigin = (pFrm->*fnRect->fnGetPrtTop)();
    bKeep = !pFrm->IsMoveable() || IsNastyFollow( pFrm );
    if( !bKeep && pFrm->IsInSct() )
    {
        const SwSectionFrm* const pSct = pFrm->FindSctFrm();
        bKeep = pSct->Lower()->IsColumnFrm() && !pSct->MoveAllowed( pFrm );
    }
    bKeep = bKeep || !pFrm->GetTextNode()->GetSwAttrSet().GetSplit().GetValue() ||
        pFrm->GetTextNode()->GetSwAttrSet().GetKeep().GetValue();
    bBreak = false;

    if( !nRstHeight && !pFrm->IsFollow() && pFrm->IsInFootnote() && pFrm->HasPara() )
    {
        nRstHeight = pFrm->GetFootnoteFrmHeight();
        nRstHeight += (pFrm->Prt().*fnRect->fnGetHeight)() -
                      (pFrm->Frm().*fnRect->fnGetHeight)();
        if( nRstHeight < 0 )
            nRstHeight = 0;
    }
}

/**
 * BP 18.6.93: Widows.
 * In contrast to the first implementation the Widows are not calculated
 * in advance but detected when formatting the split Follow.
 * In Master the Widows-calculation is dropped completely
 * (nWidows is manipulated). If the Follow detects that the
 * Widows rule applies it sends a Prepare to its predecessor.
 * A special problem is when the Widow rule applies but in Master
 * there are some lines available.
 *
 * BP(22.07.92): Calculation of Widows and Orphans.
 * The method returns true if one of the rules matches.
 *
 * One difficulty with Widows and different formats between
 * Master- and Follow-Frame:
 * Example: If the first column is 3cm and the second is 4cm and
 * Widows is set to 3, the decision if the Widows rule matches can not
 * be done until the Follow is formatted. Unfortunately this is crucial
 * to decide if the whole paragraph goes to the next page or not.
 */
bool SwTextFrmBreak::IsInside( SwTextMargin &rLine ) const
{
    bool bFit = false;

    SwSwapIfSwapped swap(pFrm);
    SWRECTFN( pFrm )
    // nOrigin is an absolut value, rLine referes to the swapped situation.

    SwTwips nTmpY;
    if ( pFrm->IsVertical() )
        nTmpY = pFrm->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , nOrigin );

    // 7455 und 6114: Calculate extra space for bottom border.
    nLineHeight += (pFrm->*fnRect->fnGetBottomMargin)();

    if( nRstHeight )
        bFit = nRstHeight >= nLineHeight;
    else
    {
        // The Frm has a height to fit on the page.
        SwTwips nHeight =
            (*fnRect->fnYDiff)( (pFrm->GetUpper()->*fnRect->fnGetPrtBottom)(), nOrigin );
        // If everything is inside the existing frame the result is true;
        bFit = nHeight >= nLineHeight;

        // --> OD #i103292#
        if ( !bFit )
        {
            if ( rLine.GetNext() &&
                 pFrm->IsInTab() && !pFrm->GetFollow() && !pFrm->GetIndNext() )
            {
                // add additional space taken as lower space as last content in a table
                // for all text lines except the last one.
                nHeight += pFrm->CalcAddLowerSpaceAsLastInTableCell();
                bFit = nHeight >= nLineHeight;
            }
        }
        // <--
        if( !bFit )
        {
            // The LineHeight exceeds the current Frm height.
            // Call a test Grow to detect if the Frame could
            // grow the requested area.
            nHeight += pFrm->GrowTst( LONG_MAX );

            // The Grow() returns the height by which the Upper of the TextFrm
            // would let the TextFrm grow.
            // The TextFrm itself can grow as much as it wants.
            bFit = nHeight >= nLineHeight;
        }
    }

    return bFit;
}

bool SwTextFrmBreak::IsBreakNow( SwTextMargin &rLine )
{
    SwSwapIfSwapped swap(pFrm);

    // bKeep is stronger than IsBreakNow()
    // Is there enough space ?
    if( bKeep || IsInside( rLine ) )
        bBreak = false;
    else
    {
        /* This class assumes that the SwTextMargin is processed from Top to
         * Bottom. Because of performance reasons we stop splitting in the
         * following cases:
         * If only one line does not fit.
         * Special case: with DummyPortions there is LineNr == 1, though we
         * want to split.
         */
        // 6010: include DropLines

        bool bFirstLine = 1 == rLine.GetLineNr() && !rLine.GetPrev();
        bBreak = true;
        if( ( bFirstLine && pFrm->GetIndPrev() )
            || ( rLine.GetLineNr() <= rLine.GetDropLines() ) )
        {
            bKeep = true;
            bBreak = false;
        }
        else if(bFirstLine && pFrm->IsInFootnote() && !pFrm->FindFootnoteFrm()->GetPrev())
        {
            SwLayoutFrm* pTmp = pFrm->FindFootnoteBossFrm()->FindBodyCont();
            if( !pTmp || !pTmp->Lower() )
                bBreak = false;
        }
    }

    return bBreak;
}

/// OD 2004-02-27 #106629# - no longer inline
void SwTextFrmBreak::SetRstHeight( const SwTextMargin &rLine )
{
    // OD, FME 2004-02-27 #106629# - consider bottom margin
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

WidowsAndOrphans::WidowsAndOrphans( SwTextFrm *pNewFrm, const SwTwips nRst,
    bool bChkKeep   )
    : SwTextFrmBreak( pNewFrm, nRst ), nWidLines( 0 ), nOrphLines( 0 )
{
    SwSwapIfSwapped swap(pFrm);

    if( bKeep )
    {
        // 5652: If pararagraph should not be split but is larger than
        // the page, then bKeep is overruled.
        if( bChkKeep && !pFrm->GetPrev() && !pFrm->IsInFootnote() &&
            pFrm->IsMoveable() &&
            ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
            bKeep = false;
        // Even if Keep is set, Orphans has to be respected.
        // e.g. if there are chained frames where a Follow in the last frame
        // receives a Keep, because it is not (forward) movable -
        // nevertheless the paragraph can request lines from the Master
        // because of the Orphan rule.
        if( pFrm->IsFollow() )
            nWidLines = pFrm->GetTextNode()->GetSwAttrSet().GetWidows().GetValue();
    }
    else
    {
        const SwAttrSet& rSet = pFrm->GetTextNode()->GetSwAttrSet();
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
            // For compatibility reasons, we disable Keep/Widows/Orphans
            // inside splittable row frames:
            if ( pFrm->GetNextCellLeaf( MAKEPAGE_NONE ) || pFrm->IsInFollowFlowRow() )
            {
                const SwFrm* pTmpFrm = pFrm->GetUpper();
                while ( !pTmpFrm->IsRowFrm() )
                    pTmpFrm = pTmpFrm->GetUpper();
                if ( static_cast<const SwRowFrm*>(pTmpFrm)->IsRowSplitAllowed() )
                    bResetFlags = true;
            }
        }

        if( pFrm->IsInFootnote() && !pFrm->GetIndPrev() )
        {
            // Inside of footnotes there are good reasons to turn off the Keep attribute
            // as well as Widows/Orphans.
            SwFootnoteFrm *pFootnote = pFrm->FindFootnoteFrm();
            const bool bFt = !pFootnote->GetAttr()->GetFootnote().IsEndNote();
            if( !pFootnote->GetPrev() &&
                pFootnote->FindFootnoteBossFrm( bFt ) != pFootnote->GetRef()->FindFootnoteBossFrm( bFt )
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
}

/**
 * The Find*-Methodes do not only search, but adjust the SwTextMargin to the
 * line where the paragraph should have a break and truncate the paragraph there.
 * FindBreak()
 */
bool WidowsAndOrphans::FindBreak( SwTextFrm *pFrame, SwTextMargin &rLine,
    bool bHasToFit )
{
    // OD 2004-02-25 #i16128# - Why member <pFrm> _*and*_ parameter <pFrame>??
    // Thus, assertion on situation, that these are different to figure out why.
    OSL_ENSURE( pFrm == pFrame, "<WidowsAndOrphans::FindBreak> - pFrm != pFrame" );

    SwSwapIfSwapped swap(pFrm);

    bool bRet = true;
    sal_uInt16 nOldOrphans = nOrphLines;
    if( bHasToFit )
        nOrphLines = 0;
    rLine.Bottom();
    // OD 2004-02-25 #i16128# - method renamed
    if( !IsBreakNowWidAndOrp( rLine ) )
        bRet = false;
    if( !FindWidows( pFrame, rLine ) )
    {
        bool bBack = false;
        // OD 2004-02-25 #i16128# - method renamed
        while( IsBreakNowWidAndOrp( rLine ) )
        {
            if( rLine.PrevLine() )
                bBack = true;
            else
                break;
        }
        // Usually Orphans are not taken into account for HasToFit.
        // But if Dummy-Lines are concerned and the Orphans rule is violated
        // we make an exception: We leave behind one Dummyline and take
        // the whole text to the next page/column.
        if( rLine.GetLineNr() <= nOldOrphans &&
            rLine.GetInfo().GetParaPortion()->IsDummy() &&
            ( ( bHasToFit && bRet ) || IsBreakNow( rLine ) ) )
            rLine.Top();

        rLine.TruncLines( true );
        bRet = bBack;
    }
    nOrphLines = nOldOrphans;

    return bRet;
}

/**
 * FindWidows positions the SwTextMargin of the Master to the line where to
 * break by examining and formatting the Follow.
 * Returns true if the Widows-rule matches, that means that the
 * paragraph should not be split (keep) !
 */
bool WidowsAndOrphans::FindWidows( SwTextFrm *pFrame, SwTextMargin &rLine )
{
    OSL_ENSURE( ! pFrame->IsVertical() || ! pFrame->IsSwapped(),
            "WidowsAndOrphans::FindWidows with swapped frame" );

    if( !nWidLines || !pFrame->IsFollow() )
        return false;

    rLine.Bottom();

    // We can still cut something off
    SwTextFrm *pMaster = pFrame->FindMaster();
    OSL_ENSURE(pMaster, "+WidowsAndOrphans::FindWidows: Widows in a master?");
    if( !pMaster )
        return false;

    // 5156: If the first line of the Follow does not fit, the master
    // probably is full of Dummies. In this case a PREP_WIDOWS would be fatal.
    if( pMaster->GetOfst() == pFrame->GetOfst() )
        return false;

    // Remaining height of the master
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

    // below the Widows-treshold...
    if( rLine.GetLineNr() >= nWidLines )
    {
        // 8575: Follow to Master I
        // If the Follow *grows*, there is the chance for the Master to
        // receive lines, that it was forced to hand over to the Follow lately:
        // Prepare(Need); check that below nChg!
        // (0W, 2O, 2M, 2F) + 1F = 3M, 2F
        if( rLine.GetLineNr() > nWidLines && pFrame->IsJustWidow() )
        {
            // If the Master is locked, it has probably just donated a line
            // to us, we don't return that just because we turned it into
            // multiple lines (e.g. via frames).
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

    // 8575: Follow to Master II
    // If the Follow *shrinks*, maybe the Master can absorb the whole Orphan.
    // (0W, 2O, 2M, 1F) - 1F = 3M, 0F     -> PREP_ADJUST_FRM
    // (0W, 2O, 3M, 2F) - 1F = 2M, 2F     -> PREP_WIDOWS

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

    // Master to Follow
    // If the Follow contains fewer rows than Widows after formatting,
    // we still can cut off some rows from the Master. If the Orphans
    // rule of the Master hereby comes into effect, we need to enlarge
    // the Frame in CalcPrep() of the Master Frame, as it won't fit into
    // the original page anymore.
    // If the Master Frame can still miss a few more rows, we need to
    // do a Shrink() in the CalcPrep(): the Follow with the Widows then
    // moves onto the page of the Master, but remains unsplit, so that
    // it (finally) moves onto the next page. So much for the theory!
    //
    // We only request one row at a time for now, because a Master's row could
    // result in multiple lines for us.
    // Therefore, the CalcFollow() remains in control until the Follow got all
    // necessary rows.
    sal_uInt16 nNeed = 1; // was: nWidLines - rLine.GetLineNr();

    // Special case: Master cannot give lines to follow
    // #i91421#
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

    pMaster->Prepare( PREP_WIDOWS, static_cast<void*>(&nNeed) );
    return true;
}

bool WidowsAndOrphans::WouldFit( SwTextMargin &rLine, SwTwips &rMaxHeight, bool bTst )
{
    // Here it does not matter, if pFrm is swapped or not.
    // IsInside() takes care of itself

    // We expect that rLine is set to the last line
    OSL_ENSURE( !rLine.GetNext(), "WouldFit: aLine::Bottom missed!" );
    sal_uInt16 nLineCnt = rLine.GetLineNr();

    // First satisfy the Orphans-rule and the wish for initials ...
    const sal_uInt16 nMinLines = std::max( GetOrphansLines(), rLine.GetDropLines() );
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

    // We do not fit
    if( !IsInside( rLine ) )
        return false;

    // Check the Widows-rule
    if( !nWidLines && !pFrm->IsFollow() )
    {
        // Usually we only have to check for Widows if we are a Follow.
        // On WouldFit the rule has to be checked for the Master too,
        // because we are just in the middle of calculating the break.
        // In Ctor of WidowsAndOrphans the nWidLines are only calced for
        // Follows from the AttrSet - so we catch up now:
        const SwAttrSet& rSet = pFrm->GetTextNode()->GetSwAttrSet();
        nWidLines = rSet.GetWidows().GetValue();
    }

    // After Orphans/Initials, do enough lines remain for Widows?
    // #111937#: If we are currently doing a test formatting, we may not
    // consider the widows rule for two reasons:
    // 1. The columns may have different widths.
    //    Widow lines would have wrong width.
    // 2. Test formatting is only done up to the given space.
    //    we do not have any lines for widows at all.
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
