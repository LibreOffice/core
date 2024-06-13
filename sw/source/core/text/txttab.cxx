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
#include <comphelper/string.hxx>
#include <editeng/tstpitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <doc.hxx>
#include <SwPortionHandler.hxx>

#include <viewopt.hxx>
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include <txtfrm.hxx>
#include "porfld.hxx"
#include <memory>

/**
 * #i24363# tab stops relative to indent
 *
 * Return the first tab stop that is > nSearchPos.
 * If the tab stop is outside the print area, we
 * return 0 if it is not the first tab stop.
 */
const SvxTabStop* SwLineInfo::GetTabStop(const SwTwips nSearchPos, SwTwips& nRight) const
{
    for( sal_uInt16 i = 0; i < m_oRuler->Count(); ++i )
    {
        const SvxTabStop &rTabStop = m_oRuler->operator[](i);
        if (nRight && rTabStop.GetTabPos() > nRight)
        {
            // Consider the first tabstop to always be in-bounds.
            if (!i)
                nRight = rTabStop.GetTabPos();
            return i ? nullptr : &rTabStop;
        }
        if( rTabStop.GetTabPos() > nSearchPos )
        {
            if (!i && !nRight)
                nRight = rTabStop.GetTabPos();
            return &rTabStop;
        }
    }
    return nullptr;
}

sal_uInt16 SwLineInfo::NumberOfTabStops() const
{
    return m_oRuler->Count();
}

SwTabPortion *SwTextFormatter::NewTabPortion( SwTextFormatInfo &rInf, bool bAuto ) const
{
    IDocumentSettingAccess const& rIDSA(rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess());
    const bool bTabOverMargin = rIDSA.get(DocumentSettingId::TAB_OVER_MARGIN);
    const bool bTabOverSpacing = rIDSA.get(DocumentSettingId::TAB_OVER_SPACING);
    const bool bTabsRelativeToIndent = rIDSA.get(DocumentSettingId::TABS_RELATIVE_TO_INDENT);

    // Update search location - since Center/Decimal tabstops' width is dependent on the following text.
    SwTabPortion* pTmpLastTab = rInf.GetLastTab();
    if (pTmpLastTab && (pTmpLastTab->IsTabCenterPortion() || pTmpLastTab->IsTabDecimalPortion()))
        pTmpLastTab->PostFormat(rInf);

    sal_Unicode cFill = 0;
    sal_Unicode cDec = 0;
    SvxTabAdjust eAdj;

    sal_uInt16 nNewTabPos;
    bool bAutoTabStop = true;
    {
        const bool bRTL = m_pFrame->IsRightToLeft();
        // #i24363# tab stops relative to indent
        // nTabLeft: The absolute value, the tab stops are relative to: Tabs origin.

        // #i91133#
        const SwTwips nTabLeft = bRTL
                                 ? m_pFrame->getFrameArea().Right() -
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
                                 : m_pFrame->getFrameArea().Left() +
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 );

        // The absolute position, where we started the line formatting
        SwTwips nLinePos = GetLeftMargin();
        if ( bRTL )
        {
            Point aPoint( nLinePos, 0 );
            m_pFrame->SwitchLTRtoRTL( aPoint );
            nLinePos = aPoint.X();
        }

        // The current position, relative to the line start
        SwTwips nTabPos = rInf.GetLastTab() ? rInf.GetLastTab()->GetTabPos() : 0;
        if( nTabPos < rInf.X() )
        {
            nTabPos = rInf.X();
        }

        // The current position in absolute coordinates
        const SwTwips nCurrentAbsPos = bRTL ?
                                       nLinePos - nTabPos :
                                       nLinePos + nTabPos;

        SwTwips nMyRight;
        if ( m_pFrame->IsVertLR() )
           nMyRight = Left();
        else
           nMyRight = Right();

        if ( m_pFrame->IsVertical() )
        {
            Point aRightTop( nMyRight, m_pFrame->getFrameArea().Top() );
            m_pFrame->SwitchHorizontalToVertical( aRightTop );
            nMyRight = aRightTop.Y();
        }

        SwTwips nNextPos = 0;
        bool bAbsoluteNextPos = false;

        // #i24363# tab stops relative to indent
        // nSearchPos: The current position relative to the tabs origin
        const SwTwips nSearchPos = bRTL ?
                                   nTabLeft - nCurrentAbsPos :
                                   nCurrentAbsPos - nTabLeft;

        // First, we examine the tab stops set at the paragraph style or
        // any hard set tab stops:
        // Note: If there are no user defined tab stops, there is always a
        // default tab stop.
        const SwTwips nOldRight = nMyRight;
        // Accept left-tabstops beyond the paragraph margin for bTabOverSpacing
        if (bTabOverSpacing || bTabOverMargin)
            nMyRight = 0;
        const SvxTabStop* pTabStop = m_aLineInf.GetTabStop( nSearchPos, nMyRight );
        if (!nMyRight)
            nMyRight = nOldRight;
        if (pTabStop)
        {
            cFill = ' ' != pTabStop->GetFill() ? pTabStop->GetFill() : 0;
            cDec = pTabStop->GetDecimal();
            eAdj = pTabStop->GetAdjustment();
            nNextPos = pTabStop->GetTabPos();
            if(!bTabsRelativeToIndent && eAdj == SvxTabAdjust::Default && nSearchPos < 0)
            {
                //calculate default tab position of default tabs in negative indent
                nNextPos = ( nSearchPos / nNextPos ) * nNextPos;
            }
            else if (pTabStop->GetTabPos() > nMyRight
                     && pTabStop->GetAdjustment() != SvxTabAdjust::Left)
            {
                // A rather special situation. The tabstop found is:
                // 1.) in a document compatible with MS formats
                // 2.) not a left tabstop.
                // 3.) not the first tabstop (in that case nMyRight was adjusted to match tabPos).
                // 4.) beyond the end of the text area
                // Therefore, they act like right-tabstops at the edge of the para area.
                // This benefits DOCX 2013+, and doesn't hurt the earlier formats,
                // since up till now these were just treated as automatic tabstops.
                eAdj = SvxTabAdjust::Right;
                bAbsoluteNextPos = true;
                nNextPos = rInf.Width();
            }
            bAutoTabStop = false;
        }
        else
        {
            sal_uInt16 nDefTabDist = m_aLineInf.GetDefTabStop();
            if( USHRT_MAX == nDefTabDist )
            {
                const SvxTabStopItem& rTab =
                    m_pFrame->GetAttrSet()->GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
                if( rTab.Count() )
                    nDefTabDist = o3tl::narrowing<sal_uInt16>(rTab[0].GetTabPos());
                else
                    nDefTabDist = SVX_TAB_DEFDIST;
                m_aLineInf.SetDefTabStop( nDefTabDist );
            }
            SwTwips nCount = nSearchPos;

            // Minimum tab stop width is 1
            if (nDefTabDist <= 0)
                nDefTabDist = 1;

            nCount /= nDefTabDist;
            nNextPos = ( nCount < 0 || ( !nCount && nSearchPos <= 0 ) )
                       ? ( nCount * nDefTabDist )
                       : ( ( nCount + 1 ) * nDefTabDist );

            // --> FME 2004-09-21 #117919 Minimum tab stop width is 1 or 51 twips:
            const SwTwips nMinimumTabWidth = m_pFrame->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::TAB_COMPAT) ? 0 : 50;
            if( (  bRTL && nTabLeft - nNextPos >= nCurrentAbsPos - nMinimumTabWidth ) ||
                 ( !bRTL && nNextPos + nTabLeft <= nCurrentAbsPos + nMinimumTabWidth  ) )
            {
                nNextPos += nDefTabDist;
            }
            cFill = 0;
            eAdj = SvxTabAdjust::Left;
        }

        // #i115705# - correction and refactoring:
        // overrule determined next tab stop position in order to apply
        // a tab stop at the left margin under the following conditions:
        // - the new tab portion is inside the hanging indent
        // - a tab stop at the left margin is allowed
        // - the determined next tab stop is a default tab stop position OR
        //   the determined next tab stop is beyond the left margin
        {
            tools::Long nLeftMarginTabPos = 0;
            {
                if ( !bTabsRelativeToIndent )
                {
                    if ( bRTL )
                    {
                        Point aPoint( Left(), 0 );
                        m_pFrame->SwitchLTRtoRTL( aPoint );
                        nLeftMarginTabPos = m_pFrame->getFrameArea().Right() - aPoint.X();
                    }
                    else
                    {
                        nLeftMarginTabPos = Left() - m_pFrame->getFrameArea().Left();
                    }
                }
                if( m_pCurr->HasForcedLeftMargin() )
                {
                    SwLinePortion* pPor = m_pCurr->GetNextPortion();
                    while( pPor && !pPor->IsFlyPortion() )
                    {
                        pPor = pPor->GetNextPortion();
                    }
                    if ( pPor )
                    {
                        nLeftMarginTabPos += pPor->Width();
                    }
                }
            }
            const bool bNewTabPortionInsideHangingIndent =
                        bRTL ? nCurrentAbsPos > nTabLeft - nLeftMarginTabPos
                             : nCurrentAbsPos < nTabLeft + nLeftMarginTabPos;
            if ( bNewTabPortionInsideHangingIndent )
            {
                // If the paragraph is not inside a list having a list tab stop following
                // the list label or no further tab stop found in such a paragraph or
                // the next tab stop position does not equal the list tab stop,
                // a tab stop at the left margin can be applied. If this condition is
                // not hold, it is overruled by compatibility option TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST.
                const bool bTabAtLeftMarginAllowed =
                    ( !m_aLineInf.IsListTabStopIncluded() ||
                      !pTabStop ||
                      nNextPos != m_aLineInf.GetListTabStopPosition() ) ||
                    // compatibility option TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
                    m_pFrame->GetDoc().getIDocumentSettingAccess().
                        get(DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
                if ( bTabAtLeftMarginAllowed )
                {
                    if ( !pTabStop || eAdj == SvxTabAdjust::Default ||
                         ( nNextPos > nLeftMarginTabPos ) )
                    {
                        eAdj = SvxTabAdjust::Default;
                        cFill = 0;
                        nNextPos = nLeftMarginTabPos;
                    }
                }
            }
        }

        if (!bAbsoluteNextPos)
            nNextPos += bRTL ? nLinePos - nTabLeft : nTabLeft - nLinePos;
        OSL_ENSURE( nNextPos >= 0, "GetTabStop: Don't go back!" );
        nNewTabPos = sal_uInt16(nNextPos);
    }

    SwTabPortion *pTabPor = nullptr;
    if ( bAuto )
    {
        if ( SvxTabAdjust::Decimal == eAdj &&
             1 == m_aLineInf.NumberOfTabStops() )
            pTabPor = new SwAutoTabDecimalPortion( nNewTabPos, cDec, cFill );
    }
    else
    {
        switch( eAdj )
        {
        case SvxTabAdjust::Right :
            {
                pTabPor = new SwTabRightPortion( nNewTabPos, cFill );
                break;
            }
        case SvxTabAdjust::Center :
            {
                pTabPor = new SwTabCenterPortion( nNewTabPos, cFill );
                break;
            }
        case SvxTabAdjust::Decimal :
            {
                pTabPor = new SwTabDecimalPortion( nNewTabPos, cDec, cFill );
                break;
            }
        default:
            {
                OSL_ENSURE( SvxTabAdjust::Left == eAdj || SvxTabAdjust::Default == eAdj,
                    "+SwTextFormatter::NewTabPortion: unknown adjustment" );
                pTabPor = new SwTabLeftPortion( nNewTabPos, cFill, bAutoTabStop );
                break;
            }
        }
    }
    if (pTabPor)
        rInf.UpdateTabSeen(pTabPor->GetWhichPor());

    return pTabPor;
}

/**
 * The base class is initialized without setting anything
 */
SwTabPortion::SwTabPortion( const sal_uInt16 nTabPosition, const sal_Unicode cFillChar, const bool bAutoTab )
    : m_nTabPos(nTabPosition), m_cFill(cFillChar), m_bAutoTabStop( bAutoTab )
{
    mnLineLength = TextFrameIndex(1);
    OSL_ENSURE(!IsFilled() || ' ' != m_cFill, "SwTabPortion::CTOR: blanks ?!");
    SetWhichPor( PortionType::Tab );
}

bool SwTabPortion::Format( SwTextFormatInfo &rInf )
{
    SwTabPortion *pLastTab = rInf.GetLastTab();
    if( pLastTab == this )
        return PostFormat( rInf );
    if( pLastTab )
        pLastTab->PostFormat( rInf );
    return PreFormat(rInf, pLastTab);
}

void SwTabPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if( rInf.GetLastTab() == this )
        PostFormat( rInf );
}

bool SwTabPortion::PreFormat(SwTextFormatInfo &rInf, SwTabPortion const*const pLastTab)
{
    OSL_ENSURE( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );

    // Here we settle down ...
    SetFix( o3tl::narrowing<sal_uInt16>(rInf.X()) );

    IDocumentSettingAccess const& rIDSA(rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess());
    const bool bTabCompat = rIDSA.get(DocumentSettingId::TAB_COMPAT);
    const bool bTabOverflow = rIDSA.get(DocumentSettingId::TAB_OVERFLOW);
    const bool bTabOverMargin = rIDSA.get(DocumentSettingId::TAB_OVER_MARGIN);
    const bool bTabOverSpacing = rIDSA.get(DocumentSettingId::TAB_OVER_SPACING);
    const sal_Int32 nTextFrameWidth = rInf.GetTextFrame()->getFrameArea().Width();

    // The minimal width of a tab is one blank at least.
    // #i37686# In compatibility mode, the minimum width
    // should be 1, even for non-left tab stops.
    sal_uInt16 nMinimumTabWidth = 1;
    if ( !bTabCompat )
    {
        // #i89179#
        // tab portion representing the list tab of a list label gets the
        // same font as the corresponding number portion
        std::optional< SwFontSave > oSave;
        if ( GetLen() == TextFrameIndex(0) &&
             rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
             static_cast<SwNumberPortion*>(rInf.GetLast())->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<SwNumberPortion*>(rInf.GetLast())->GetFont();
            oSave.emplace( rInf, const_cast<SwFont*>(pNumberPortionFont) );
        }
        OUString aTmp( ' ' );
        SwTextSizeInfo aInf( rInf, &aTmp );
        nMinimumTabWidth = aInf.GetTextSize().Width();
    }
    PrtWidth( nMinimumTabWidth );

    // Break tab stop to next line if:
    // 1. Minimal width does not fit to line anymore.
    // 2. An underflow event was called for the tab portion.
    bool bFull = ( bTabCompat && rInf.IsUnderflow() ) ||
             (rInf.Width() <= rInf.X() + PrtWidth() && rInf.X() <= rInf.Width()
              && (!bTabOverMargin || !pLastTab));

    // #95477# Rotated tab stops get the width of one blank
    const Degree10 nDir = rInf.GetFont()->GetOrientation( rInf.GetTextFrame()->IsVertical() );

    if( ! bFull && 0_deg10 == nDir )
    {
        const PortionType nWhich = GetWhichPor();
        switch( nWhich )
        {
            case PortionType::TabRight:
            case PortionType::TabDecimal:
            case PortionType::TabCenter:
            {
                if( PortionType::TabDecimal == nWhich )
                    rInf.SetTabDecimal(
                        static_cast<SwTabDecimalPortion*>(this)->GetTabDecimal());
                rInf.SetLastTab( this );
                break;
            }
            case PortionType::TabLeft:
            {
                // handle this case in PostFormat
                if ((bTabOverMargin || bTabOverSpacing) && GetTabPos() > rInf.Width()
                    && (!m_bAutoTabStop || rInf.Width() <= rInf.X()))
                {
                    if (bTabOverMargin || GetTabPos() < nTextFrameWidth)
                    {
                        rInf.SetLastTab(this);
                        break;
                    }
                    else
                    {
                        assert(!bTabOverMargin && bTabOverSpacing && GetTabPos() >= nTextFrameWidth);
                        bFull = true;
                        break;
                    }
                }

                PrtWidth( o3tl::narrowing<sal_uInt16>(GetTabPos() - rInf.X()) );
                bFull = rInf.Width() <= rInf.X() + PrtWidth();

                // In tabulator compatibility mode, we reset the bFull flag
                // if the tabulator is at the end of the paragraph and the
                // tab stop position is outside the frame:
                bool bAtParaEnd = rInf.GetIdx() + GetLen() == TextFrameIndex(rInf.GetText().getLength());
                if ( bFull && bTabCompat &&
                     ( ( bTabOverflow && ( rInf.IsTabOverflow() || !m_bAutoTabStop ) ) || bAtParaEnd ) &&
                     GetTabPos() >= nTextFrameWidth)
                {
                    bFull = false;
                    if ( bTabOverflow && !m_bAutoTabStop )
                        rInf.SetTabOverflow( true );
                }

                break;
            }
            default: OSL_ENSURE( false, "SwTabPortion::PreFormat: unknown adjustment" );
        }
    }

    if( bFull )
    {
        // We have to look for endless loops, if the width is smaller than one blank
        if( rInf.GetIdx() == rInf.GetLineStart() &&
            // #119175# TabStop should be forced to current
            // line if there is a fly reducing the line width:
            !rInf.GetFly() )
        {
            PrtWidth( o3tl::narrowing<sal_uInt16>(rInf.Width() - rInf.X()) );
            SetFixWidth( PrtWidth() );
        }
        else
        {
            Height( 0 );
            Width( 0 );
            SetLen( TextFrameIndex(0) );
            SetAscent( 0 );
            SetNextPortion( nullptr ); //?????
        }
        return true;
    }
    else
    {
        // A trick with impact: The new Tabportions now behave like
        // FlyFrames, located in the line - including adjustment !
        SetFixWidth( PrtWidth() );
        return false;
    }
}

bool SwTabPortion::PostFormat( SwTextFormatInfo &rInf )
{
    bool bTabOverMargin = rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
        DocumentSettingId::TAB_OVER_MARGIN);
    bool bTabOverSpacing = rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
        DocumentSettingId::TAB_OVER_SPACING);
    if (rInf.GetTextFrame()->IsInSct())
        bTabOverMargin = false;

    // If the tab position is larger than the right margin, it gets scaled down by default.
    // However, if compat mode enabled, we allow tabs to go over the margin: the rest of the paragraph is not broken into lines.
    const sal_uInt16 nRight
        = bTabOverMargin
              ? GetTabPos()
              : bTabOverSpacing
                    ? std::min<long>(GetTabPos(), rInf.GetTextFrame()->getFrameArea().Right())
                    : std::min(GetTabPos(), rInf.Width());
    const SwLinePortion *pPor = GetNextPortion();

    sal_uInt16 nPorWidth = 0;
    while( pPor )
    {
        nPorWidth = nPorWidth + pPor->Width();
        pPor = pPor->GetNextPortion();
    }

    const PortionType nWhich = GetWhichPor();
    const bool bTabCompat = rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::TAB_COMPAT);

    if ((bTabOverMargin || bTabOverSpacing) && PortionType::TabLeft == nWhich)
    {
        nPorWidth = 0;
    }

    // #127428# Abandon dec. tab position if line is full
    if ( bTabCompat && PortionType::TabDecimal == nWhich )
    {
        sal_uInt16 nPrePorWidth = static_cast<const SwTabDecimalPortion*>(this)->GetWidthOfPortionsUpToDecimalPosition();

        // no value was set => no decimal character was found
        if ( USHRT_MAX != nPrePorWidth )
        {
            if ( !bTabOverMargin && nPrePorWidth && nPorWidth - nPrePorWidth > rInf.Width() - nRight )
            {
                nPrePorWidth += nPorWidth - nPrePorWidth - ( rInf.Width() - nRight );
            }

            nPorWidth = nPrePorWidth - 1;
        }
    }

    if( PortionType::TabCenter == nWhich )
    {
        // centered tabs are problematic:
        // We have to detect how much fits into the line.
        sal_uInt16 nNewWidth = nPorWidth /2;
        if (!bTabOverMargin && !bTabOverSpacing && nNewWidth > rInf.Width() - nRight)
            nNewWidth = nPorWidth - (rInf.Width() - nRight);
        nPorWidth = nNewWidth;
    }

    const sal_uInt16 nDiffWidth = nRight - GetFix();

    if( nDiffWidth > nPorWidth )
    {
        const sal_uInt16 nOldWidth = GetFixWidth();
        const sal_uInt16 nAdjDiff = nDiffWidth - nPorWidth;
        if( nAdjDiff > GetFixWidth() )
            PrtWidth( nAdjDiff );
        // Don't be afraid: we have to move rInf further.
        // The right-tab till now only had the width of one blank.
        // Now that we stretched, the difference had to be added to rInf.X() !
        rInf.X( rInf.X() + PrtWidth() - nOldWidth );
    }
    SetFixWidth( PrtWidth() );
    // reset last values
    rInf.SetLastTab(nullptr);
    if( PortionType::TabDecimal == nWhich )
        rInf.SetTabDecimal(0);

    return rInf.Width() <= rInf.X();
}

/**
 * Ex: LineIter::DrawTab()
 */
void SwTabPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    // #i89179#
    // tab portion representing the list tab of a list label gets the
    // same font as the corresponding number portion
    std::optional< SwFontSave > oSave;
    bool bAfterNumbering = false;
    if (GetLen() == TextFrameIndex(0))
    {
        const SwLinePortion* pPrevPortion =
            const_cast<SwTabPortion*>(this)->FindPrevPortion( rInf.GetParaPortion() );
        if ( pPrevPortion &&
             pPrevPortion->InNumberGrp() &&
             static_cast<const SwNumberPortion*>(pPrevPortion)->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<const SwNumberPortion*>(pPrevPortion)->GetFont();
            oSave.emplace( rInf, const_cast<SwFont*>(pNumberPortionFont) );
            bAfterNumbering = true;
        }
    }
    rInf.DrawBackBrush( *this );
    if( !bAfterNumbering )
        rInf.DrawBorder( *this );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && mpNextPortion && !mpNextPortion->Width() )
        mpNextPortion->PrePaint( rInf, this );

    // display special characters
    if( rInf.OnWin() && rInf.GetOpt().IsTab() )
    {
        // filled tabs are shaded in gray
        if( IsFilled() )
            rInf.DrawViewOpt( *this, PortionType::Tab );
        else
            rInf.DrawTab( *this );
    }

    // Tabs should be underlined at once
    if( rInf.GetFont()->IsPaintBlank() )
    {
        // Tabs with filling/filled tabs
        const sal_uInt16 nCharWidth = rInf.GetTextSize(OUString(' ')).Width();

        // Robust:
        if( nCharWidth )
        {
            // Always with kerning, also on printer!
            sal_uInt16 nChar = Width() / nCharWidth;
            OUStringBuffer aBuf(nChar);
            comphelper::string::padToLength(aBuf, nChar, ' ');
            rInf.DrawText(aBuf.makeStringAndClear(), *this, TextFrameIndex(0),
                            TextFrameIndex(nChar), true);
        }
    }

    // Display fill characters
    if( !IsFilled() )
        return;

    // Tabs with filling/filled tabs
    const sal_uInt16 nCharWidth = rInf.GetTextSize(OUString(m_cFill)).Width();
    OSL_ENSURE( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );

    // Robust:
    if( nCharWidth )
    {
        // Always with kerning, also on printer!
        sal_uInt16 nChar = Width() / nCharWidth;
        if ( m_cFill == '_' )
            ++nChar; // to avoid gaps
        OUStringBuffer aBuf(nChar);
        comphelper::string::padToLength(aBuf, nChar, m_cFill);
        rInf.DrawText(aBuf.makeStringAndClear(), *this, TextFrameIndex(0),
                        TextFrameIndex(nChar), true);
    }
}

void SwAutoTabDecimalPortion::Paint( const SwTextPaintInfo & ) const
{
}

void SwTabPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
