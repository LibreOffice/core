/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "hintids.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <SwPortionHandler.hxx>

#include "viewopt.hxx"  // SwViewOptions
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "txtfrm.hxx"
#include <numrule.hxx>
// #i89179#
#include <porfld.hxx>


/*************************************************************************
 *                    SwLineInfo::GetTabStop()
 *************************************************************************/

//#i24363# tab stops relative to indent
/* Return the first tab stop that is > nSearchPos.
 * If the tab stop is outside the print area, we
 * return 0 if it is not the first tab stop.*/
const SvxTabStop *SwLineInfo::GetTabStop( const SwTwips nSearchPos,
                                         const SwTwips nRight ) const
{
    for( MSHORT i = 0; i < pRuler->Count(); ++i )
    {
        const SvxTabStop &rTabStop = pRuler->operator[](i);
        if( rTabStop.GetTabPos() > SwTwips(nRight) )
            return i ? 0 : &rTabStop;

        if( rTabStop.GetTabPos() > nSearchPos )
            return &rTabStop;
    }
    return 0;
}

/*************************************************************************
 *                    SwLineInfo::NumberOfTabStops()
 *************************************************************************/

sal_uInt16 SwLineInfo::NumberOfTabStops() const
{
    return pRuler->Count();
}

/*************************************************************************
 *                      SwTxtFormatter::NewTabPortion()
 *************************************************************************/
SwTabPortion *SwTxtFormatter::NewTabPortion( SwTxtFormatInfo &rInf, bool bAuto ) const
{
    SwTabPortion *pTabPor = 0;

    sal_Unicode cFill = 0;
    sal_Unicode cDec = 0;
    SvxTabAdjust eAdj;

    KSHORT nNewTabPos;
    bool bAutoTabStop = true;
    {
        const bool bRTL = pFrm->IsRightToLeft();
        // #i24363# tab stops relative to indent
        // nTabLeft: The absolute value, the tab stops are relative to: Tabs origin.
        //
        // #i91133#
        const bool bTabsRelativeToIndent =
            pFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT);
        const SwTwips nTabLeft = bRTL
                                 ? pFrm->Frm().Right() -
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
                                 : pFrm->Frm().Left() +
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 );

        //
        // nLinePos: The absolute position, where we started the line formatting.
        //
        SwTwips nLinePos = GetLeftMargin();
        if ( bRTL )
        {
            Point aPoint( nLinePos, 0 );
            pFrm->SwitchLTRtoRTL( aPoint );
            nLinePos = aPoint.X();
        }

        //
        // nTabPos: The current position, relative to the line start.
        //
        SwTwips nTabPos = rInf.GetLastTab() ? rInf.GetLastTab()->GetTabPos() : 0;
        if( nTabPos < rInf.X() )
        {
            nTabPos = rInf.X();
        }

        //
        // nCurrentAbsPos: The current position in absolute coordinates.
        //
        const SwTwips nCurrentAbsPos = bRTL ?
                                       nLinePos - nTabPos :
                                       nLinePos + nTabPos;

       //Badaa: 2008-04-18 * Support for Classical Mongolian Script (SCMS) joint with Jiayanmin
        SwTwips nMyRight;
        if ( pFrm->IsVertLR() )
           nMyRight = Left();
        else
           nMyRight = Right();

        if ( pFrm->IsVertical() )
        {
            Point aRightTop( nMyRight, pFrm->Frm().Top() );
            pFrm->SwitchHorizontalToVertical( aRightTop );
            nMyRight = aRightTop.Y();
        }

        SwTwips nNextPos;

        // #i24363# tab stops relative to indent
        // nSearchPos: The current position relative to the tabs origin.
        //
        const SwTwips nSearchPos = bRTL ?
                                   nTabLeft - nCurrentAbsPos :
                                   nCurrentAbsPos - nTabLeft;

        //
        // First, we examine the tab stops set at the paragraph style or
        // any hard set tab stops:
        // Note: If there are no user defined tab stops, there is always a
        // default tab stop.
        //
        const SvxTabStop* pTabStop =
            aLineInf.GetTabStop( nSearchPos, nMyRight );
        if( pTabStop )
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
            KSHORT nDefTabDist = aLineInf.GetDefTabStop();
            if( KSHRT_MAX == nDefTabDist )
            {
                const SvxTabStopItem& rTab =
                    (const SvxTabStopItem &)pFrm->GetAttrSet()->
                    GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
                if( rTab.Count() )
                    nDefTabDist = (KSHORT)rTab[0].GetTabPos();
                else
                    nDefTabDist = SVX_TAB_DEFDIST;
                aLineInf.SetDefTabStop( nDefTabDist );
            }
            SwTwips nCount = nSearchPos;

            //Minimum tab stop width is 1
            if (nDefTabDist <= 0)
                nDefTabDist = 1;

            nCount /= nDefTabDist;
            nNextPos = nCount < 0 || (!nCount && nSearchPos <= 0)? nCount * nDefTabDist :( nCount + 1 ) * nDefTabDist ;
            // --> FME 2004-09-21 #117919 Minimum tab stop width is 1 or 51 twips:
            const SwTwips nMinimumTabWidth = pFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT) ? 0 : 50;
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
                        pFrm->SwitchLTRtoRTL( aPoint );
                        nLeftMarginTabPos = pFrm->Frm().Right() - aPoint.X();
                    }
                    else
                    {
                        nLeftMarginTabPos = Left() - pFrm->Frm().Left();
                    }
                }
                if( pCurr->HasForcedLeftMargin() )
                {
                    SwLinePortion* pPor = pCurr->GetPortion();
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
                    ( !aLineInf.IsListTabStopIncluded() ||
                      !pTabStop ||
                      nNextPos != aLineInf.GetListTabStopPosition() ) ||
                    // compatibility option TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
                    pFrm->GetTxtNode()->getIDocumentSettingAccess()->
                        get(IDocumentSettingAccess::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
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
        nNewTabPos = KSHORT(nNextPos);
    }

    if ( bAuto )
    {
        if ( SVX_TAB_ADJUST_DECIMAL == eAdj &&
             // #127428#
             1 == aLineInf.NumberOfTabStops() )
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
                        "+SwTxtFormatter::NewTabPortion: unknown adjustment" );
                pTabPor = new SwTabLeftPortion( nNewTabPos, cFill, bAutoTabStop );
                break;
            }
        }
    }

    // Show existence of tabs "..." is not necessary anymore
    // pCurr->SetTabulation();
    // We calculate the data as a precaution
    // pTabPor->Height( pLast->Height() );
    // pTabPor->SetAscent( pLast->GetAscent() );
    return pTabPor;
}

/*************************************************************************
 *                SwTabPortion::SwTabPortion()
 *************************************************************************/

// Die Basisklasse wird erstmal ohne alles initialisiert.


SwTabPortion::SwTabPortion( const KSHORT nTabPosition, const sal_Unicode cFillChar, const bool bAutoTab )
    : SwFixPortion( 0, 0 ), nTabPos(nTabPosition), cFill(cFillChar), bAutoTabStop( bAutoTab )
{
    nLineLength = 1;
    OSL_ENSURE(!IsFilled() || ' ' != cFill, "SwTabPortion::CTOR: blanks ?!");
    SetWhichPor( POR_TAB );
}

/*************************************************************************
 *                 virtual SwTabPortion::Format()
 *************************************************************************/



sal_Bool SwTabPortion::Format( SwTxtFormatInfo &rInf )
{
    SwTabPortion *pLastTab = rInf.GetLastTab();
    if( pLastTab == this )
        return PostFormat( rInf );
    if( pLastTab )
        pLastTab->PostFormat( rInf );
    return PreFormat( rInf );
}

/*************************************************************************
 *                 virtual SwTabPortion::FormatEOL()
 *************************************************************************/



void SwTabPortion::FormatEOL( SwTxtFormatInfo &rInf )
{
    if( rInf.GetLastTab() == this && !IsTabLeftPortion() )
        PostFormat( rInf );
}

/*************************************************************************
 *                    SwTabPortion::PreFormat()
 *************************************************************************/



sal_Bool SwTabPortion::PreFormat( SwTxtFormatInfo &rInf )
{
    OSL_ENSURE( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );

    // Here we settle down ...
    Fix( static_cast<sal_uInt16>(rInf.X()) );

    const bool bTabCompat = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);
    const bool bTabOverflow = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_OVERFLOW);

    // The minimal width of a tab is one blank at least.
    // #i37686# In compatibility mode, the minimum width
    // should be 1, even for non-left tab stops.
    sal_uInt16 nMinimumTabWidth = 1;
    if ( !bTabCompat )
    {
        // #i89179#
        // tab portion representing the list tab of a list label gets the
        // same font as the corresponding number portion
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        std::auto_ptr< SwFontSave > pSave( 0 );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if ( GetLen() == 0 &&
             rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
             static_cast<SwNumberPortion*>(rInf.GetLast())->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<SwNumberPortion*>(rInf.GetLast())->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
        }
        rtl::OUString aTmp( ' ' );
        SwTxtSizeInfo aInf( rInf, aTmp );
        nMinimumTabWidth = aInf.GetTxtSize().Width();
    }
    PrtWidth( nMinimumTabWidth );

    // Break tab stop to next line if:
    // 1. Minmal width does not fit to line anymore.
    // 2. An underflow event was called for the tab portion.
    sal_Bool bFull = ( bTabCompat && rInf.IsUnderFlow() ) ||
                     ( rInf.Width() <= rInf.X() + PrtWidth() && rInf.X() <= rInf.Width() ) ;

    // #95477# Rotated tab stops get the width of one blank
    const sal_uInt16 nDir = rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );

    if( ! bFull && 0 == nDir )
    {
        const MSHORT nWhich = GetWhichPor();
        switch( nWhich )
        {
            case POR_TABRIGHT:
            case POR_TABDECIMAL:
            case POR_TABCENTER:
            {
                if( POR_TABDECIMAL == nWhich )
                    rInf.SetTabDecimal(
                        ((SwTabDecimalPortion*)this)->GetTabDecimal());
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
                bool bAtParaEnd = rInf.GetIdx() + GetLen() == rInf.GetTxt().Len();
                if ( bFull && bTabCompat &&
                     ( ( bTabOverflow && ( rInf.IsTabOverflow() || !IsAutoTabStop() ) ) || bAtParaEnd ) &&
                     GetTabPos() >= rInf.GetTxtFrm()->Frm().Width() )
                {
                    bFull = sal_False;
                    if ( bTabOverflow && !IsAutoTabStop() )
                        rInf.SetTabOverflow( sal_True );
                }

                break;
            }
            default: OSL_ENSURE( !this, "SwTabPortion::PreFormat: unknown adjustment" );
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
            SetPortion( NULL ); //?????
        }
        return sal_True;
    }
    else
    {
        // A trick with impact: The new Tabportions now behave like
        // FlyFrms, located in the line - including adjustment !
        SetFixWidth( PrtWidth() );
        return sal_False;
    }
}

/*************************************************************************
 *                      SwTabPortion::PostFormat()
 *************************************************************************/



sal_Bool SwTabPortion::PostFormat( SwTxtFormatInfo &rInf )
{
    const KSHORT nRight = Min( GetTabPos(), rInf.Width() );
    const SwLinePortion *pPor = GetPortion();

    KSHORT nPorWidth = 0;
    while( pPor )
    {
        nPorWidth = nPorWidth + pPor->Width();
        pPor = pPor->GetPortion();
    }

    const MSHORT nWhich = GetWhichPor();
    OSL_ENSURE( POR_TABLEFT != nWhich, "SwTabPortion::PostFormat: already formatted" );
    const bool bTabCompat = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);

    // #127428# Abandon dec. tab position if line is full
    if ( bTabCompat && POR_TABDECIMAL == nWhich )
    {
        KSHORT nPrePorWidth = static_cast<const SwTabDecimalPortion*>(this)->GetWidthOfPortionsUpToDecimalPosition();

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
        KSHORT nNewWidth = nPorWidth /2;
        if( nNewWidth > rInf.Width() - nRight )
            nNewWidth = nPorWidth - (rInf.Width() - nRight);
        nPorWidth = nNewWidth;
    }

    const KSHORT nDiffWidth = nRight - Fix();

    if( nDiffWidth > nPorWidth )
    {
        const KSHORT nOldWidth = GetFixWidth();
        const KSHORT nAdjDiff = nDiffWidth - nPorWidth;
        if( nAdjDiff > GetFixWidth() )
            PrtWidth( nAdjDiff );
        // Dont be afraid: we have to move rInf further.
        // The right-tab till now only had the width of one blank.
        // Now that we stretched, the difference had to be added to rInf.X() !
        rInf.X( rInf.X() + PrtWidth() - nOldWidth );
    }
    SetFixWidth( PrtWidth() );
    // reset last values
    rInf.SetLastTab(0);
    if( POR_TABDECIMAL == nWhich )
        rInf.SetTabDecimal(0);

    return rInf.Width() <= rInf.X();
}

/*************************************************************************
 *                virtual SwTabPortion::Paint()
 *
 * Ex: LineIter::DrawTab()
 *************************************************************************/

void SwTabPortion::Paint( const SwTxtPaintInfo &rInf ) const
{
#if OSL_DEBUG_LEVEL > 1
    // We want to view the fixed width
    if( rInf.OnWin() && OPTDBG( rInf ) &&
        !rInf.GetOpt().IsPagePreview() && \
        !rInf.GetOpt().IsReadonly() && \
        SwViewOption::IsFieldShadings()    )
    {
        const KSHORT nTmpWidth = PrtWidth();
        ((SwTabPortion*)this)->PrtWidth( GetFixWidth() );
        rInf.DrawViewOpt( *this, POR_TAB );
        ((SwTabPortion*)this)->PrtWidth( nTmpWidth );
    }
#endif

    // #i89179#
    // tab portion representing the list tab of a list label gets the
    // same font as the corresponding number portion
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    std::auto_ptr< SwFontSave > pSave( 0 );
    SAL_WNODEPRECATED_DECLARATIONS_POP
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
        }
    }
    rInf.DrawBackBrush( *this );

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

    // 6842: Tabs should be underlined at once.
    if( rInf.GetFont()->IsPaintBlank() )
    {
        // tabs with filling / filled tabs
        UniString aTxt = rtl::OUString(' ');
        const KSHORT nCharWidth = rInf.GetTxtSize( aTxt ).Width();
        // robust:
        if( nCharWidth )
        {
            // 6864: always with kerning, also on printer!
            KSHORT nChar = Width() / nCharWidth;
            rInf.DrawText( aTxt.Fill( nChar, ' ' ), *this, 0, nChar, sal_True );
        }
    }

    // Display fill characters
    if( IsFilled() )
    {
        // tabs with filling / filled tabs
        UniString aTxt = rtl::OUString(cFill);
        const KSHORT nCharWidth = rInf.GetTxtSize( aTxt ).Width();
        OSL_ENSURE( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );
        // robust:
        if( nCharWidth )
        {
            // 6864: always with kerning, also on printer!
            KSHORT nChar = Width() / nCharWidth;
            if ( cFill == '_' )
                ++nChar; // to avoid gaps (Bug 13430)
            rInf.DrawText( aTxt.Fill( nChar, cFill ), *this, 0, nChar, sal_True );
        }
    }
}

/*************************************************************************
 *                virtual SwAutoTabDecimalPortion::Paint()
 *************************************************************************/

void SwAutoTabDecimalPortion::Paint( const SwTxtPaintInfo & ) const
{
}

/*************************************************************************
 *              virtual SwTabPortion::HandlePortion()
 *************************************************************************/

void SwTabPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
