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
#include <comphelper/string.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <rtl/ustrbuf.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <SwPortionHandler.hxx>

#include "viewopt.hxx"
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "txtfrm.hxx"
#include <numrule.hxx>
#include <porfld.hxx>
#include <memory>

/**
 * #i24363# tab stops relative to indent
 *
 * Return the first tab stop that is > nSearchPos.
 * If the tab stop is outside the print area, we
 * return 0 if it is not the first tab stop.
 */
const SvxTabStop *SwLineInfo::GetTabStop( const SwTwips nSearchPos, const SwTwips nRight ) const
{
    for( sal_uInt16 i = 0; i < pRuler->Count(); ++i )
    {
        const SvxTabStop &rTabStop = pRuler->operator[](i);
        if( rTabStop.GetTabPos() > SwTwips(nRight) )
            return i ? nullptr : &rTabStop;

        if( rTabStop.GetTabPos() > nSearchPos )
            return &rTabStop;
    }
    return nullptr;
}

sal_uInt16 SwLineInfo::NumberOfTabStops() const
{
    return pRuler->Count();
}

SwTabPortion *SwTextFormatter::NewTabPortion( SwTextFormatInfo &rInf, bool bAuto ) const
{
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
        const bool bTabsRelativeToIndent =
            m_pFrame->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TABS_RELATIVE_TO_INDENT);
        const SwTwips nTabLeft = bRTL
                                 ? m_pFrame->Frame().Right() -
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
                                 : m_pFrame->Frame().Left() +
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
            Point aRightTop( nMyRight, m_pFrame->Frame().Top() );
            m_pFrame->SwitchHorizontalToVertical( aRightTop );
            nMyRight = aRightTop.Y();
        }

        SwTwips nNextPos = 0;

        // #i24363# tab stops relative to indent
        // nSearchPos: The current position relative to the tabs origin
        const SwTwips nSearchPos = bRTL ?
                                   nTabLeft - nCurrentAbsPos :
                                   nCurrentAbsPos - nTabLeft;

        // First, we examine the tab stops set at the paragraph style or
        // any hard set tab stops:
        // Note: If there are no user defined tab stops, there is always a
        // default tab stop.
        const SvxTabStop* pTabStop = m_aLineInf.GetTabStop( nSearchPos, nMyRight );
        if ( pTabStop )
        {
            cFill = ' ' != pTabStop->GetFill() ? pTabStop->GetFill() : 0;
            cDec = pTabStop->GetDecimal();
            eAdj = pTabStop->GetAdjustment();
            nNextPos = pTabStop->GetTabPos();
            if(!bTabsRelativeToIndent && eAdj == SVX_TAB_ADJUST_DEFAULT && nSearchPos < 0)
            {
                //calculate default tab position of default tabs in negative indent
                nNextPos = ( nSearchPos / nNextPos ) * nNextPos;
            }
            bAutoTabStop = false;
        }
        else
        {
            sal_uInt16 nDefTabDist = m_aLineInf.GetDefTabStop();
            if( USHRT_MAX == nDefTabDist )
            {
                const SvxTabStopItem& rTab =
                    static_cast<const SvxTabStopItem &>(m_pFrame->GetAttrSet()->
                      GetPool()->GetDefaultItem( RES_PARATR_TABSTOP ));
                if( rTab.Count() )
                    nDefTabDist = (sal_uInt16)rTab[0].GetTabPos();
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
            const SwTwips nMinimumTabWidth = m_pFrame->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT) ? 0 : 50;
            if( (  bRTL && nTabLeft - nNextPos >= nCurrentAbsPos - nMinimumTabWidth ) ||
                 ( !bRTL && nNextPos + nTabLeft <= nCurrentAbsPos + nMinimumTabWidth  ) )
            {
                nNextPos += nDefTabDist;
            }
            cFill = 0;
            eAdj = SVX_TAB_ADJUST_LEFT;
        }

        // #i115705# - correction and refactoring:
        // overrule determined next tab stop position in order to apply
        // a tab stop at the left margin under the following conditions:
        // - the new tab portion is inside the hanging indent
        // - a tab stop at the left margin is allowed
        // - the determined next tab stop is a default tab stop position OR
        //   the determined next tab stop is beyond the left margin
        {
            long nLeftMarginTabPos = 0;
            {
                if ( !bTabsRelativeToIndent )
                {
                    if ( bRTL )
                    {
                        Point aPoint( Left(), 0 );
                        m_pFrame->SwitchLTRtoRTL( aPoint );
                        nLeftMarginTabPos = m_pFrame->Frame().Right() - aPoint.X();
                    }
                    else
                    {
                        nLeftMarginTabPos = Left() - m_pFrame->Frame().Left();
                    }
                }
                if( m_pCurr->HasForcedLeftMargin() )
                {
                    SwLinePortion* pPor = m_pCurr->GetPortion();
                    while( pPor && !pPor->IsFlyPortion() )
                    {
                        pPor = pPor->GetPortion();
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
                    m_pFrame->GetTextNode()->getIDocumentSettingAccess()->
                        get(DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
                if ( bTabAtLeftMarginAllowed )
                {
                    if ( !pTabStop || eAdj == SVX_TAB_ADJUST_DEFAULT ||
                         ( nNextPos > nLeftMarginTabPos ) )
                    {
                        eAdj = SVX_TAB_ADJUST_DEFAULT;
                        cFill = 0;
                        nNextPos = nLeftMarginTabPos;
                    }
                }
            }
        }

        nNextPos += bRTL ? nLinePos - nTabLeft : nTabLeft - nLinePos;
        OSL_ENSURE( nNextPos >= 0, "GetTabStop: Don't go back!" );
        nNewTabPos = sal_uInt16(nNextPos);
    }

    SwTabPortion *pTabPor = nullptr;
    if ( bAuto )
    {
        if ( SVX_TAB_ADJUST_DECIMAL == eAdj &&
             1 == m_aLineInf.NumberOfTabStops() )
            pTabPor = new SwAutoTabDecimalPortion( nNewTabPos, cDec, cFill );
    }
    else
    {
        switch( eAdj )
        {
        case SVX_TAB_ADJUST_RIGHT :
            {
                pTabPor = new SwTabRightPortion( nNewTabPos, cFill );
                break;
            }
        case SVX_TAB_ADJUST_CENTER :
            {
                pTabPor = new SwTabCenterPortion( nNewTabPos, cFill );
                break;
            }
        case SVX_TAB_ADJUST_DECIMAL :
            {
                pTabPor = new SwTabDecimalPortion( nNewTabPos, cDec, cFill );
                break;
            }
        default:
            {
                OSL_ENSURE( SVX_TAB_ADJUST_LEFT == eAdj || SVX_TAB_ADJUST_DEFAULT == eAdj,
                    "+SwTextFormatter::NewTabPortion: unknown adjustment" );
                pTabPor = new SwTabLeftPortion( nNewTabPos, cFill, bAutoTabStop );
                break;
            }
        }
    }

    return pTabPor;
}

/**
 * The base class is initialized without setting anything
 */
SwTabPortion::SwTabPortion( const sal_uInt16 nTabPosition, const sal_Unicode cFillChar, const bool bAutoTab )
    : SwFixPortion( 0, 0 ), nTabPos(nTabPosition), cFill(cFillChar), bAutoTabStop( bAutoTab )
{
    nLineLength = 1;
    OSL_ENSURE(!IsFilled() || ' ' != cFill, "SwTabPortion::CTOR: blanks ?!");
    SetWhichPor( POR_TAB );
}

bool SwTabPortion::Format( SwTextFormatInfo &rInf )
{
    SwTabPortion *pLastTab = rInf.GetLastTab();
    if( pLastTab == this )
        return PostFormat( rInf );
    if( pLastTab )
        pLastTab->PostFormat( rInf );
    return PreFormat( rInf );
}

void SwTabPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if( rInf.GetLastTab() == this && !IsTabLeftPortion() )
        PostFormat( rInf );
}

bool SwTabPortion::PreFormat( SwTextFormatInfo &rInf )
{
    OSL_ENSURE( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );

    // Here we settle down ...
    Fix( static_cast<sal_uInt16>(rInf.X()) );

    const bool bTabCompat = rInf.GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT);
    const bool bTabOverflow = rInf.GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_OVERFLOW);

    // The minimal width of a tab is one blank at least.
    // #i37686# In compatibility mode, the minimum width
    // should be 1, even for non-left tab stops.
    sal_uInt16 nMinimumTabWidth = 1;
    if ( !bTabCompat )
    {
        // #i89179#
        // tab portion representing the list tab of a list label gets the
        // same font as the corresponding number portion
        std::unique_ptr< SwFontSave > pSave;
        if ( GetLen() == 0 &&
             rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
             static_cast<SwNumberPortion*>(rInf.GetLast())->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<SwNumberPortion*>(rInf.GetLast())->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
        }
        OUString aTmp( ' ' );
        SwTextSizeInfo aInf( rInf, &aTmp );
        nMinimumTabWidth = aInf.GetTextSize().Width();
    }
    PrtWidth( nMinimumTabWidth );

    // Break tab stop to next line if:
    // 1. Minmal width does not fit to line anymore.
    // 2. An underflow event was called for the tab portion.
    bool bFull = ( bTabCompat && rInf.IsUnderflow() ) ||
                     ( rInf.Width() <= rInf.X() + PrtWidth() && rInf.X() <= rInf.Width() ) ;

    // #95477# Rotated tab stops get the width of one blank
    const sal_uInt16 nDir = rInf.GetFont()->GetOrientation( rInf.GetTextFrame()->IsVertical() );

    if( ! bFull && 0 == nDir )
    {
        const sal_uInt16 nWhich = GetWhichPor();
        switch( nWhich )
        {
            case POR_TABRIGHT:
            case POR_TABDECIMAL:
            case POR_TABCENTER:
            {
                if( POR_TABDECIMAL == nWhich )
                    rInf.SetTabDecimal(
                        static_cast<SwTabDecimalPortion*>(this)->GetTabDecimal());
                rInf.SetLastTab( this );
                break;
            }
            case POR_TABLEFT:
            {
                PrtWidth( static_cast<sal_uInt16>(GetTabPos() - rInf.X()) );
                bFull = rInf.Width() <= rInf.X() + PrtWidth();

                // In tabulator compatibility mode, we reset the bFull flag
                // if the tabulator is at the end of the paragraph and the
                // tab stop position is outside the frame:
                bool bAtParaEnd = rInf.GetIdx() + GetLen() == rInf.GetText().getLength();
                if ( bFull && bTabCompat &&
                     ( ( bTabOverflow && ( rInf.IsTabOverflow() || !IsAutoTabStop() ) ) || bAtParaEnd ) &&
                     GetTabPos() >= rInf.GetTextFrame()->Frame().Width() )
                {
                    bFull = false;
                    if ( bTabOverflow && !IsAutoTabStop() )
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
            PrtWidth( static_cast<sal_uInt16>(rInf.Width() - rInf.X()) );
            SetFixWidth( PrtWidth() );
        }
        else
        {
            Height( 0 );
            Width( 0 );
            SetLen( 0 );
            SetAscent( 0 );
            SetPortion( nullptr ); //?????
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
    const bool bTabOverMargin = rInf.GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_OVER_MARGIN);
    // If the tab position is larger than the right margin, it gets scaled down by default.
    // However, if compat mode enabled, we allow tabs to go over the margin: the rest of the paragraph is not broken into lines.
    const sal_uInt16 nRight = bTabOverMargin ? GetTabPos() : std::min(GetTabPos(), rInf.Width());
    const SwLinePortion *pPor = GetPortion();

    sal_uInt16 nPorWidth = 0;
    while( pPor )
    {
        nPorWidth = nPorWidth + pPor->Width();
        pPor = pPor->GetPortion();
    }

    const sal_uInt16 nWhich = GetWhichPor();
    OSL_ENSURE( POR_TABLEFT != nWhich, "SwTabPortion::PostFormat: already formatted" );
    const bool bTabCompat = rInf.GetTextFrame()->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::TAB_COMPAT);

    // #127428# Abandon dec. tab position if line is full
    if ( bTabCompat && POR_TABDECIMAL == nWhich )
    {
        sal_uInt16 nPrePorWidth = static_cast<const SwTabDecimalPortion*>(this)->GetWidthOfPortionsUpToDecimalPosition();

        // no value was set => no decimal character was found
        if ( USHRT_MAX != nPrePorWidth )
        {
            if ( nPrePorWidth && nPorWidth - nPrePorWidth > rInf.Width() - nRight )
            {
                nPrePorWidth += nPorWidth - nPrePorWidth - ( rInf.Width() - nRight );
            }

            nPorWidth = nPrePorWidth - 1;
        }
    }

    if( POR_TABCENTER == nWhich )
    {
        // centered tabs are problematic:
        // We have to detect how much fits into the line.
        sal_uInt16 nNewWidth = nPorWidth /2;
        if( nNewWidth > rInf.Width() - nRight )
            nNewWidth = nPorWidth - (rInf.Width() - nRight);
        nPorWidth = nNewWidth;
    }

    const sal_uInt16 nDiffWidth = nRight - Fix();

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
    if( POR_TABDECIMAL == nWhich )
        rInf.SetTabDecimal(0);

    return rInf.Width() <= rInf.X();
}

/**
 * Ex: LineIter::DrawTab()
 */
void SwTabPortion::Paint( const SwTextPaintInfo &rInf ) const
{
#if OSL_DEBUG_LEVEL > 1
    // We want to view the fixed width
    if( rInf.OnWin() && OPTDBG( rInf ) &&
        !rInf.GetOpt().IsPagePreview() && \
        !rInf.GetOpt().IsReadonly() && \
        SwViewOption::IsFieldShadings()    )
    {
        const sal_uInt16 nTmpWidth = PrtWidth();
        const_cast<SwTabPortion*>(this)->PrtWidth( GetFixWidth() );
        rInf.DrawViewOpt( *this, POR_TAB );
        const_cast<SwTabPortion*>(this)->PrtWidth( nTmpWidth );
    }
#endif

    // #i89179#
    // tab portion representing the list tab of a list label gets the
    // same font as the corresponding number portion
    std::unique_ptr< SwFontSave > pSave;
    bool bAfterNumbering = false;
    if ( GetLen() == 0 )
    {
        const SwLinePortion* pPrevPortion =
            const_cast<SwTabPortion*>(this)->FindPrevPortion( rInf.GetParaPortion() );
        if ( pPrevPortion &&
             pPrevPortion->InNumberGrp() &&
             static_cast<const SwNumberPortion*>(pPrevPortion)->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<const SwNumberPortion*>(pPrevPortion)->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
            bAfterNumbering = true;
        }
    }
    rInf.DrawBackBrush( *this );
    if( !bAfterNumbering )
        rInf.DrawBorder( *this );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    // display special characters
    if( rInf.OnWin() && rInf.GetOpt().IsTab() )
    {
        // filled tabs are shaded in gray
        if( IsFilled() )
            rInf.DrawViewOpt( *this, POR_TAB );
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
            OUStringBuffer aBuf;
            comphelper::string::padToLength(aBuf, nChar, ' ');
            rInf.DrawText(aBuf.makeStringAndClear(), *this, 0, nChar, true);
        }
    }

    // Display fill characters
    if( IsFilled() )
    {
        // Tabs with filling/filled tabs
        const sal_uInt16 nCharWidth = rInf.GetTextSize(OUString(cFill)).Width();
        OSL_ENSURE( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );

        // Robust:
        if( nCharWidth )
        {
            // Always with kerning, also on printer!
            sal_uInt16 nChar = Width() / nCharWidth;
            if ( cFill == '_' )
                ++nChar; // to avoid gaps
            OUStringBuffer aBuf;
            comphelper::string::padToLength(aBuf, nChar, cFill);
            rInf.DrawText(aBuf.makeStringAndClear(), *this, 0, nChar, true);
        }
    }
}

void SwAutoTabDecimalPortion::Paint( const SwTextPaintInfo & ) const
{
}

void SwTabPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor(), Height(), Width() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
