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

#include <layfrm.hxx>
#include <ftnboss.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <editeng/orphitem.hxx>
#include <editeng/widwitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/spltitem.hxx>
#include <frmatr.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <rowfrm.hxx>

#include "widorp.hxx"
#include <txtfrm.hxx>
#include "itrtxt.hxx"
#include <sectfrm.hxx>
#include <ftnfrm.hxx>
#include <pagefrm.hxx>

#undef WIDOWTWIPS

namespace
{

// A Follow on the same page as its master is nasty.
bool IsNastyFollow( const SwTextFrame *pFrame )
{
    OSL_ENSURE( !pFrame->IsFollow() || !pFrame->GetPrev() ||
            static_cast<const SwTextFrame*>(pFrame->GetPrev())->GetFollow() == pFrame,
            "IsNastyFollow: What is going on here?" );
    return  pFrame->IsFollow() && pFrame->GetPrev();
}

}

SwTextFrameBreak::SwTextFrameBreak( SwTextFrame *pNewFrame, const SwTwips nRst )
    : m_nRstHeight(nRst), m_pFrame(pNewFrame)
{
    SwSwapIfSwapped swap(m_pFrame);
    SwRectFnSet aRectFnSet(m_pFrame);
    m_nOrigin = aRectFnSet.GetPrtTop(*m_pFrame);
    m_bKeep = !m_pFrame->IsMoveable() || IsNastyFollow( m_pFrame );
    if( !m_bKeep && m_pFrame->IsInSct() )
    {
        const SwSectionFrame* const pSct = m_pFrame->FindSctFrame();
        m_bKeep = pSct->Lower()->IsColumnFrame() && !pSct->MoveAllowed( m_pFrame );
    }
    m_bKeep = m_bKeep || !m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetSplit().GetValue() ||
        m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetKeep().GetValue();
    m_bBreak = false;

    if( !m_nRstHeight && !m_pFrame->IsFollow() && m_pFrame->IsInFootnote() && m_pFrame->HasPara() )
    {
        m_nRstHeight = m_pFrame->GetFootnoteFrameHeight();
        m_nRstHeight += aRectFnSet.GetHeight(m_pFrame->getFramePrintArea()) -
                      aRectFnSet.GetHeight(m_pFrame->getFrameArea());
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
bool SwTextFrameBreak::IsInside( SwTextMargin const &rLine ) const
{
    bool bFit = false;

    SwSwapIfSwapped swap(m_pFrame);
    SwRectFnSet aRectFnSet(m_pFrame);
    // nOrigin is an absolute value, rLine referes to the swapped situation.

    SwTwips nTmpY;
    if ( m_pFrame->IsVertical() )
        nTmpY = m_pFrame->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
    else
        nTmpY = rLine.Y() + rLine.GetLineHeight();

    SwTwips nLineHeight = aRectFnSet.YDiff( nTmpY , m_nOrigin );

    // Calculate extra space for bottom border.
    nLineHeight += aRectFnSet.GetBottomMargin(*m_pFrame);

    if( m_nRstHeight )
        bFit = m_nRstHeight >= nLineHeight;
    else
    {
        // The Frame has a height to fit on the page.
        SwTwips nHeight =
            aRectFnSet.YDiff( aRectFnSet.GetPrtBottom(*m_pFrame->GetUpper()), m_nOrigin );
        SwTwips nDiff = nHeight - nLineHeight;

        // Hide whitespace may require not to insert a new page.
        SwPageFrame* pPageFrame = m_pFrame->FindPageFrame();
        if (!pPageFrame->CheckPageHeightValidForHideWhitespace(nDiff))
            nDiff = 0;

        // If everything is inside the existing frame the result is true;
        bFit = nDiff >= 0;

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
        // Include DropLines

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

void SwTextFrameBreak::SetRstHeight( const SwTextMargin &rLine )
{
    // Consider bottom margin
    SwRectFnSet aRectFnSet(m_pFrame);

    m_nRstHeight = aRectFnSet.GetBottomMargin(*m_pFrame);

    if ( aRectFnSet.IsVert() )
    {
           if ( m_pFrame->IsVertLR() )
              m_nRstHeight = aRectFnSet.YDiff( m_pFrame->SwitchHorizontalToVertical( rLine.Y() ) , m_nOrigin );
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
        // If paragraph should not be split but is larger than
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
            nWidLines = m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet().GetWidows().GetValue();
    }
    else
    {
        const SwAttrSet& rSet = m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet();
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
            if ( m_pFrame->GetNextCellLeaf() || m_pFrame->IsInFollowFlowRow() )
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
 * The Find*-Methods do not only search, but adjust the SwTextMargin to the
 * line where the paragraph should have a break and truncate the paragraph there.
 * FindBreak()
 */
bool WidowsAndOrphans::FindBreak( SwTextFrame *pFrame, SwTextMargin &rLine,
    bool bHasToFit )
{
    // i#16128 - Why member <pFrame> _*and*_ parameter <pFrame>??
    // Thus, assertion on situation, that these are different to figure out why.
    OSL_ENSURE( m_pFrame == pFrame, "<WidowsAndOrphans::FindBreak> - pFrame != pFrame" );

    SwSwapIfSwapped swap(m_pFrame);

    bool bRet = true;
    sal_uInt16 nOldOrphans = nOrphLines;
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

    // If the first line of the Follow does not fit, the master
    // probably is full of Dummies. In this case a PREP_WIDOWS would be fatal.
    if( pMaster->GetOfst() == pFrame->GetOfst() )
        return false;

    // Remaining height of the master
    SwRectFnSet aRectFnSet(pFrame);

    const SwTwips nDocPrtTop = aRectFnSet.GetPrtTop(*pFrame);
    SwTwips nOldHeight;
    SwTwips nTmpY = rLine.Y() + rLine.GetLineHeight();

    if ( aRectFnSet.IsVert() )
    {
        nTmpY = pFrame->SwitchHorizontalToVertical( nTmpY );
        nOldHeight = -aRectFnSet.GetHeight(pFrame->getFramePrintArea());
    }
    else
        nOldHeight = aRectFnSet.GetHeight(pFrame->getFramePrintArea());

    const SwTwips nChg = aRectFnSet.YDiff( nTmpY, nDocPrtTop + nOldHeight );

    // below the Widows-threshold...
    if( rLine.GetLineNr() >= nWidLines )
    {
        // Follow to Master I
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
                const SwTwips nTmpRstHeight = aRectFnSet.BottomDist( pMaster->getFrameArea(),
                    aRectFnSet.GetPrtBottom(*pMaster->GetUpper()) );
                if ( nTmpRstHeight >=
                     SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
                {
                    pMaster->Prepare( PREP_ADJUST_FRM );
                    pMaster->InvalidateSize_();
                    pMaster->InvalidatePage();
                }
            }

            pFrame->SetJustWidow( false );
        }
        return false;
    }

    // Follow to Master II
    // If the Follow *shrinks*, maybe the Master can absorb the whole Orphan.
    // (0W, 2O, 2M, 1F) - 1F = 3M, 0F     -> PREP_ADJUST_FRM
    // (0W, 2O, 3M, 2F) - 1F = 2M, 2F     -> PREP_WIDOWS

    if( 0 > nChg && !pMaster->IsLocked() && pMaster->GetUpper() )
    {
        SwTwips nTmpRstHeight = aRectFnSet.BottomDist( pMaster->getFrameArea(),
            aRectFnSet.GetPrtBottom(*pMaster->GetUpper()) );
        if( nTmpRstHeight >= SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
        {
            pMaster->Prepare( PREP_ADJUST_FRM );
            pMaster->InvalidateSize_();
            pMaster->InvalidatePage();
            pFrame->SetJustWidow( false );
            return false;
        }
    }

    // Master to Follow
    // If the Follow contains fewer lines than Widows after formatting,
    // we still can move over some lines from the Master. If this triggers
    // the Orphans rule of the Master, the Master frame must be Grow()n
    // in its CalcPreps(), such that it won't fit onto its page anymore.
    // But if the Master Frame can still lose a few lines, we need to
    // do a Shrink() in the CalcPreps(); the Follow with the Widows then
    // moves onto the page of the Master, but remains unsplit, so that
    // it (finally) moves onto the next page. So much for the theory!
    //
    // We only request one line at a time for now, because a Master's line
    // could result in multiple lines for us.
    // Therefore, the CalcFollow() remains in control until the Follow got all
    // necessary lines.
    sal_uInt16 nNeed = 1; // was: nWidLines - rLine.GetLineNr();

    // Special case: Master cannot give lines to follow
    // i#91421
    if ( !pMaster->GetIndPrev() )
    {
        pMaster->ChgThisLines();
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
        const SwAttrSet& rSet = m_pFrame->GetTextNodeForParaProps()->GetSwAttrSet();
        nWidLines = rSet.GetWidows().GetValue();
    }

    // After Orphans/Initials, do enough lines remain for Widows?
    // If we are currently doing a test formatting, we may not
    // consider the widows rule for two reasons:
    // 1. The columns may have different widths.
    //    Widow lines would have wrong width.
    // 2. Test formatting is only done up to the given space.
    //    we do not have any lines for widows at all.
    if( bTst || nLineCnt - nMinLines >= nWidLines )
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
