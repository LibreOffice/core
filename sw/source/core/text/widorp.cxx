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
inline bool IsNastyFollow( const SwTextFrame *pFrame )
{
    OSL_ENSURE( !pFrame->IsFollow() || !pFrame->GetPrev() ||
            static_cast<const SwTextFrame*>(pFrame->GetPrev())->GetFollow() == pFrame,
            "IsNastyFollow: Was ist denn hier los?" );
    return  pFrame->IsFollow() && pFrame->GetPrev();
}

}

SwTextFrameBreak::SwTextFrameBreak( SwTextFrame *pNewFrame, const SwTwips nRst )
    : m_nRstHeight(nRst), m_pFrame(pNewFrame)
{
    SwSwapIfSwapped swap(m_pFrame);
    SWRECTFN( m_pFrame )
    m_nOrigin = (m_pFrame->*fnRect->fnGetPrtTop)();
    m_bKeep = !m_pFrame->IsMoveable() || IsNastyFollow( m_pFrame );
    if( !m_bKeep && m_pFrame->IsInSct() )
    {
        const SwSectionFrame* const pSct = m_pFrame->FindSctFrame();
        m_bKeep = pSct->Lower()->IsColumnFrame() && !pSct->MoveAllowed( m_pFrame );
    }
    m_bKeep = m_bKeep || !m_pFrame->GetTextNode()->GetSwAttrSet().GetSplit().GetValue() ||
        m_pFrame->GetTextNode()->GetSwAttrSet().GetKeep().GetValue();
    m_bBreak = false;

    if( !m_nRstHeight && !m_pFrame->IsFollow() && m_pFrame->IsInFootnote() && m_pFrame->HasPara() )
    {
        m_nRstHeight = m_pFrame->GetFootnoteFrameHeight();
        m_nRstHeight += (m_pFrame->Prt().*fnRect->fnGetHeight)() -
                      (m_pFrame->Frame().*fnRect->fnGetHeight)();
        if( m_nRstHeight < 0 )
            m_nRstHeight = 0;
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
bool SwTextFrameBreak::IsInside( SwTextMargin &rLine ) const
{
    bool bFit = false;

    SwSwapIfSwapped swap(m_pFrame);
    SWRECTFN( m_pFrame )
    // nOrigin is an absolut value, rLine referes to the swapped situation.

    SwTwips nTmpY;
    if ( m_pFrame->IsVertical() )
        nTmpY = m_pFrame->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , m_nOrigin );

    // 7455 und 6114: Calculate extra space for bottom border.
    nLineHeight += (m_pFrame->*fnRect->fnGetBottomMargin)();

    if( m_nRstHeight )
        bFit = m_nRstHeight >= nLineHeight;
    else
    {
        // The Frame has a height to fit on the page.
        SwTwips nHeight =
            (*fnRect->fnYDiff)( (m_pFrame->GetUpper()->*fnRect->fnGetPrtBottom)(), m_nOrigin );
        // If everything is inside the existing frame the result is true;
        bFit = nHeight >= nLineHeight;

        // --> OD #i103292#
        if ( !bFit )
        {
            if ( rLine.GetNext() &&
                 m_pFrame->IsInTab() && !m_pFrame->GetFollow() && !m_pFrame->GetIndNext() )
            {
                // add additional space taken as lower space as last content in a table
                // for all text lines except the last one.
                nHeight += m_pFrame->CalcAddLowerSpaceAsLastInTableCell();
                bFit = nHeight >= nLineHeight;
            }
        }
        // <--
        if( !bFit )
        {
            // The LineHeight exceeds the current Frame height.
            // Call a test Grow to detect if the Frame could
            // grow the requested area.
            nHeight += m_pFrame->GrowTst( LONG_MAX );

            // The Grow() returns the height by which the Upper of the TextFrame
            // would let the TextFrame grow.
            // The TextFrame itself can grow as much as it wants.
            bFit = nHeight >= nLineHeight;
        }
    }

    return bFit;
}

bool SwTextFrameBreak::IsBreakNow( SwTextMargin &rLine )
{
    SwSwapIfSwapped swap(m_pFrame);

    // bKeep is stronger than IsBreakNow()
    // Is there enough space ?
    if( m_bKeep || IsInside( rLine ) )
        m_bBreak = false;
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
        m_bBreak = true;
        if( ( bFirstLine && m_pFrame->GetIndPrev() )
            || ( rLine.GetLineNr() <= rLine.GetDropLines() ) )
        {
            m_bKeep = true;
            m_bBreak = false;
        }
        else if(bFirstLine && m_pFrame->IsInFootnote() && !m_pFrame->FindFootnoteFrame()->GetPrev())
        {
            SwLayoutFrame* pTmp = m_pFrame->FindFootnoteBossFrame()->FindBodyCont();
            if( !pTmp || !pTmp->Lower() )
                m_bBreak = false;
        }
    }

    return m_bBreak;
}

/// OD 2004-02-27 #106629# - no longer inline
void SwTextFrameBreak::SetRstHeight( const SwTextMargin &rLine )
{
    // OD, FME 2004-02-27 #106629# - consider bottom margin
    SWRECTFN( m_pFrame )

    m_nRstHeight = (m_pFrame->*fnRect->fnGetBottomMargin)();

    if ( bVert )
    {
           if ( m_pFrame->IsVertLR() )
              m_nRstHeight = (*fnRect->fnYDiff)( m_pFrame->SwitchHorizontalToVertical( rLine.Y() ) , m_nOrigin );
           else
               m_nRstHeight += m_nOrigin - m_pFrame->SwitchHorizontalToVertical( rLine.Y() );
    }
    else
        m_nRstHeight += rLine.Y() - m_nOrigin;
}

WidowsAndOrphans::WidowsAndOrphans( SwTextFrame *pNewFrame, const SwTwips nRst,
    bool bChkKeep   )
    : SwTextFrameBreak( pNewFrame, nRst ), nWidLines( 0 ), nOrphLines( 0 )
{
    SwSwapIfSwapped swap(m_pFrame);

    if( m_bKeep )
    {
        // 5652: If pararagraph should not be split but is larger than
        // the page, then bKeep is overruled.
        if( bChkKeep && !m_pFrame->GetPrev() && !m_pFrame->IsInFootnote() &&
            m_pFrame->IsMoveable() &&
            ( !m_pFrame->IsInSct() || m_pFrame->FindSctFrame()->MoveAllowed(m_pFrame) ) )
            m_bKeep = false;
        // Even if Keep is set, Orphans has to be respected.
        // e.g. if there are chained frames where a Follow in the last frame
        // receives a Keep, because it is not (forward) movable -
        // nevertheless the paragraph can request lines from the Master
        // because of the Orphan rule.
        if( m_pFrame->IsFollow() )
            nWidLines = m_pFrame->GetTextNode()->GetSwAttrSet().GetWidows().GetValue();
    }
    else
    {
        const SwAttrSet& rSet = m_pFrame->GetTextNode()->GetSwAttrSet();
        const SvxOrphansItem  &rOrph = rSet.GetOrphans();
        if ( rOrph.GetValue() > 1 )
            nOrphLines = rOrph.GetValue();
        if ( m_pFrame->IsFollow() )
            nWidLines = rSet.GetWidows().GetValue();

    }

    if ( m_bKeep || nWidLines || nOrphLines )
    {
        bool bResetFlags = false;

        if ( m_pFrame->IsInTab() )
        {
            // For compatibility reasons, we disable Keep/Widows/Orphans
            // inside splittable row frames:
            if ( m_pFrame->GetNextCellLeaf( MAKEPAGE_NONE ) || m_pFrame->IsInFollowFlowRow() )
            {
                const SwFrame* pTmpFrame = m_pFrame->GetUpper();
                while ( !pTmpFrame->IsRowFrame() )
                    pTmpFrame = pTmpFrame->GetUpper();
                if ( static_cast<const SwRowFrame*>(pTmpFrame)->IsRowSplitAllowed() )
                    bResetFlags = true;
            }
        }

        if( m_pFrame->IsInFootnote() && !m_pFrame->GetIndPrev() )
        {
            // Inside of footnotes there are good reasons to turn off the Keep attribute
            // as well as Widows/Orphans.
            SwFootnoteFrame *pFootnote = m_pFrame->FindFootnoteFrame();
            const bool bFt = !pFootnote->GetAttr()->GetFootnote().IsEndNote();
            if( !pFootnote->GetPrev() &&
                pFootnote->FindFootnoteBossFrame( bFt ) != pFootnote->GetRef()->FindFootnoteBossFrame( bFt )
                && ( !m_pFrame->IsInSct() || m_pFrame->FindSctFrame()->MoveAllowed(m_pFrame) ) )
            {
                bResetFlags = true;
            }
        }

        if ( bResetFlags )
        {
            m_bKeep = false;
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
bool WidowsAndOrphans::FindBreak( SwTextFrame *pFrame, SwTextMargin &rLine,
    bool bHasToFit )
{
    // OD 2004-02-25 #i16128# - Why member <pFrame> _*and*_ parameter <pFrame>??
    // Thus, assertion on situation, that these are different to figure out why.
    OSL_ENSURE( m_pFrame == pFrame, "<WidowsAndOrphans::FindBreak> - pFrame != pFrame" );

    SwSwapIfSwapped swap(m_pFrame);

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
bool WidowsAndOrphans::FindWidows( SwTextFrame *pFrame, SwTextMargin &rLine )
{
    OSL_ENSURE( ! pFrame->IsVertical() || ! pFrame->IsSwapped(),
            "WidowsAndOrphans::FindWidows with swapped frame" );

    if( !nWidLines || !pFrame->IsFollow() )
        return false;

    rLine.Bottom();

    // We can still cut something off
    SwTextFrame *pMaster = pFrame->FindMaster();
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
                const SwTwips nTmpRstHeight = (pMaster->Frame().*fnRect->fnBottomDist)
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
        SwTwips nTmpRstHeight = (pMaster->Frame().*fnRect->fnBottomDist)
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
    // Here it does not matter, if pFrame is swapped or not.
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
    if( !nWidLines && !m_pFrame->IsFollow() )
    {
        // Usually we only have to check for Widows if we are a Follow.
        // On WouldFit the rule has to be checked for the Master too,
        // because we are just in the middle of calculating the break.
        // In Ctor of WidowsAndOrphans the nWidLines are only calced for
        // Follows from the AttrSet - so we catch up now:
        const SwAttrSet& rSet = m_pFrame->GetTextNode()->GetSwAttrSet();
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
