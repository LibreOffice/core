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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "hintids.hxx"
#include <editeng/lrspitem.hxx>
#include <editeng/tstpitem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <frmatr.hxx>
#include <SwPortionHandler.hxx>

#include "viewopt.hxx"  // SwViewOptions
#include "txtcfg.hxx"
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "txtfrm.hxx"
#include <numrule.hxx>
// --> OD 2008-06-05 #i89179#
#include <porfld.hxx>
// <--


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

USHORT SwLineInfo::NumberOfTabStops() const
{
    return pRuler->Count();
}

/*************************************************************************
 *                      SwTxtFormatter::NewTabPortion()
 *************************************************************************/

SwTabPortion *SwTxtFormatter::NewTabPortion( SwTxtFormatInfo &rInf, bool bAuto ) const
{
    SwTabPortion *pTabPor = 0;
    SwTabPortion  *pLastTab = rInf.GetLastTab();
    if( pLastTab && ( pLastTab->IsTabCntPortion() || pLastTab->IsTabDecimalPortion() ) )
        if( pLastTab->PostFormat( rInf ) )
            return 0;

    xub_Unicode cFill = 0;
    xub_Unicode cDec = 0;
    SvxTabAdjust eAdj;

    KSHORT nNewTabPos;
    {
        const bool bRTL = pFrm->IsRightToLeft();
        // #i24363# tab stops relative to indent
        // nTabLeft: The absolute value, the tab stops are relative to: Tabs origin.
        //
        // --> OD 2008-07-01 #i91133#
        const bool bTabsRelativeToIndent =
            pFrm->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TABS_RELATIVE_TO_INDENT);
        const SwTwips nTabLeft = bRTL
                                 ? pFrm->Frm().Right() -
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 )
                                 : pFrm->Frm().Left() +
                                   ( bTabsRelativeToIndent ? GetTabLeft() : 0 );
        // <--

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

        SwTwips nMyRight = Right();

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
                    nDefTabDist = (KSHORT)rTab.GetStart()->GetTabPos();
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
            // <--
            if( (  bRTL && nTabLeft - nNextPos >= nCurrentAbsPos - nMinimumTabWidth ) ||
                 ( !bRTL && nNextPos + nTabLeft <= nCurrentAbsPos + nMinimumTabWidth  ) )
            {
                nNextPos += nDefTabDist;
            }
            cFill = 0;
            eAdj = SVX_TAB_ADJUST_LEFT;
        }
        long nForced = 0;
        if ( !bTabsRelativeToIndent )
        {
            if ( bRTL )
            {
                Point aPoint( Left(), 0 );
                pFrm->SwitchLTRtoRTL( aPoint );
                nForced = pFrm->Frm().Right() - aPoint.X();
            }
            else
            {
                nForced = Left() - pFrm->Frm().Left();
            }
        }
        if( pCurr->HasForcedLeftMargin() )
        {
            SwLinePortion* pPor = pCurr->GetPortion();
            while( pPor && !pPor->IsFlyPortion() )
                pPor = pPor->GetPortion();
            if( pPor )
                nForced += pPor->Width();
        }

        // --> OD 2009-04-03 #i100732#
        // correction of condition, when a tab stop at the left margin can
        // be applied:
        // If the paragraph is not inside a list having a list tab stop following
        // the list label or no further tab stop found in such a paragraph or
        // the next tab stop position does not equal the list tab stop,
        // a tab stop at the left margin can be applied. If this condition is
        // not hold, it is overruled by compatibility option TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST.
        const bool bTabAtLeftMargin =
            ( !aLineInf.IsListTabStopIncluded() ||
              !pTabStop ||
              nNextPos != aLineInf.GetListTabStopPosition() ) ||
            // compatibility option TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
            pFrm->GetTxtNode()->getIDocumentSettingAccess()->
                get(IDocumentSettingAccess::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST);
        if ( bTabAtLeftMargin &&
        // <--
             ( ( bRTL && nCurrentAbsPos > nTabLeft - nForced ) ||
               ( !bRTL && nCurrentAbsPos < nTabLeft + nForced ) ) &&
               // --> OD 2009-07-21 #i103685#
               //  adjust condition:
               // - back to pre OOo 3.0 condition, if tab stops are relative to indent
               // - further checks needed, if tab stops are not relative to indent
               ( nNextPos > 0 &&
               ( bTabsRelativeToIndent ||
                 ( !pTabStop || nNextPos > nForced ) ) ) )
               // <--
        {
            eAdj = SVX_TAB_ADJUST_DEFAULT;
            cFill = 0;
            nNextPos = nForced;
        }
        nNextPos += bRTL ? nLinePos - nTabLeft : nTabLeft - nLinePos;
        OSL_ENSURE( nNextPos >= 0, "GetTabStop: Don't go back!" );
        nNewTabPos = KSHORT(nNextPos);
    }

    if ( bAuto )
    {
        if ( SVX_TAB_ADJUST_DECIMAL == eAdj &&
             // --> FME 2005-12-19 #127428#
             1 == aLineInf.NumberOfTabStops() )
             // <--
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
                pTabPor = new SwTabLeftPortion( nNewTabPos, cFill );
                break;
            }
        }
    }

    // Vorhandensein von Tabulatoren anzeigen ... ist nicht mehr noetig
    // pCurr->SetTabulation();
    // Aus Sicherheitsgruenden lassen wir uns die Daten errechnen
    // pTabPor->Height( pLast->Height() );
    // pTabPor->SetAscent( pLast->GetAscent() );
    return pTabPor;
}

/*************************************************************************
 *                SwTabPortion::SwTabPortion()
 *************************************************************************/

// Die Basisklasse wird erstmal ohne alles initialisiert.


SwTabPortion::SwTabPortion( const KSHORT nTabPosition, const xub_Unicode cFillChar )
    : SwFixPortion( 0, 0 ), nTabPos(nTabPosition), cFill(cFillChar)
{
    nLineLength = 1;
#if OSL_DEBUG_LEVEL > 1
    if( IsFilled() )
    {
        OSL_ENSURE( ' ' != cFill, "SwTabPortion::CTOR: blanks ?!" );
    }
#endif
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

    // Hier lassen wir uns nieder...
    Fix( static_cast<USHORT>(rInf.X()) );

    const bool bTabCompat = rInf.GetTxtFrm()->GetTxtNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);

    // Die Mindestbreite eines Tabs ist immer mindestens ein Blank
    // --> FME 2004-11-25 #i37686# In compatibility mode, the minimum width
    // should be 1, even for non-left tab stops.
    USHORT nMinimumTabWidth = 1;
    // <--
    if ( !bTabCompat )
    {
        // --> OD 2008-06-05 #i89179#
        // tab portion representing the list tab of a list label gets the
        // same font as the corresponding number portion
        std::auto_ptr< SwFontSave > pSave( 0 );
        if ( GetLen() == 0 &&
             rInf.GetLast() && rInf.GetLast()->InNumberGrp() &&
             static_cast<SwNumberPortion*>(rInf.GetLast())->HasFont() )
        {
            const SwFont* pNumberPortionFont =
                    static_cast<SwNumberPortion*>(rInf.GetLast())->GetFont();
            pSave.reset( new SwFontSave( rInf, const_cast<SwFont*>(pNumberPortionFont) ) );
        }
        // <--
        XubString aTmp( ' ' );
        SwTxtSizeInfo aInf( rInf, aTmp );
        nMinimumTabWidth = aInf.GetTxtSize().Width();
    }
    PrtWidth( nMinimumTabWidth );

    // Break tab stop to next line if:
    // 1. Minmal width does not fit to line anymore.
    // 2. An underflow event was called for the tab portion.
    sal_Bool bFull = ( bTabCompat && rInf.IsUnderFlow() ) ||
                       rInf.Width() <= rInf.X() + PrtWidth();

    // #95477# Rotated tab stops get the width of one blank
    const USHORT nDir = rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );

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
                PrtWidth( static_cast<USHORT>(GetTabPos() - rInf.X()) );
                bFull = rInf.Width() <= rInf.X() + PrtWidth();

                // In tabulator compatibility mode, we reset the bFull flag
                // if the tabulator is at the end of the paragraph and the
                // tab stop position is outside the frame:
                if ( bFull && bTabCompat &&
                     rInf.GetIdx() + GetLen() == rInf.GetTxt().Len() &&
                     GetTabPos() >= rInf.GetTxtFrm()->Frm().Width() )
                    bFull = sal_False;

                break;
            }
            default: OSL_ENSURE( !this, "SwTabPortion::PreFormat: unknown adjustment" );
        }
    }

    if( bFull )
    {
        // Wir muessen aufpassen, dass wir nicht endlos schleifen,
        // wenn die Breite kleiner ist, als ein Blank ...
        if( rInf.GetIdx() == rInf.GetLineStart() &&
            // --> FME 2005-01-19 #119175# TabStop should be forced to current
            // line if there is a fly reducing the line width:
            !rInf.GetFly() )
            // <--
        {
            PrtWidth( static_cast<USHORT>(rInf.Width() - rInf.X()) );
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
        // Ein Kunstgriff mit Effekt: Die neuen Tabportions verhalten sich nun
        // so, wie FlyFrms, die in der Zeile stehen - inklusive Adjustment !
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

    // --> FME 2005-12-19 #127428# Abandon dec. tab position if line is full:
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
    // <--

    if( POR_TABCENTER == nWhich )
    {
        // zentrierte Tabs bereiten Probleme:
        // Wir muessen den Anteil herausfinden, der noch auf die Zeile passt.
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
        // Nicht erschrecken: wir muessen rInf weiterschieben.
        // Immerhin waren wir als Rechtstab bislang nur ein Blank breit.
        // Da wir uns jetzt aufgespannt haben, muss der Differenzbetrag
        // auf rInf.X() addiert werden !
        rInf.X( rInf.X() + PrtWidth() - nOldWidth );
    }
    SetFixWidth( PrtWidth() );
    // letzte Werte zuruecksetzen
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
    // Wir wollen uns die Fixbreite anzeigen
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

    // --> OD 2008-06-05 #i89179#
    // tab portion representing the list tab of a list label gets the
    // same font as the corresponding number portion
    std::auto_ptr< SwFontSave > pSave( 0 );
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
    // <--
    rInf.DrawBackBrush( *this );

    // do we have to repaint a post it portion?
    if( rInf.OnWin() && pPortion && !pPortion->Width() )
        pPortion->PrePaint( rInf, this );

    // Darstellung von Sonderzeichen
    if( rInf.OnWin() && rInf.GetOpt().IsTab() )
    {
        // gefuellte Tabs werden grau hinterlegt.
        if( IsFilled() )
            rInf.DrawViewOpt( *this, POR_TAB );
        else
            rInf.DrawTab( *this );
    }

    // 6842: Tabs sollen auf einmal wieder unterstrichen werden.
    if( rInf.GetFont()->IsPaintBlank() )
    {
        // Tabs mit Fuellung
        XubString aTxt( ' ' );
        const KSHORT nCharWidth = rInf.GetTxtSize( aTxt ).Width();
        // robust:
        if( nCharWidth )
        {
            // 6864: immer mit Kerning, auch auf dem Drucker!
            KSHORT nChar = Width() / nCharWidth;
            rInf.DrawText( aTxt.Fill( nChar, ' ' ), *this, 0, nChar, sal_True );
        }
    }

    // Ausgabe von Fuellzeichen
    if( IsFilled() )
    {
        // Tabs mit Fuellung
        XubString aTxt( cFill );
        const KSHORT nCharWidth = rInf.GetTxtSize( aTxt ).Width();
#if OSL_DEBUG_LEVEL > 1
        OSL_ENSURE( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );
#endif
        // robust:
        if( nCharWidth )
        {
            // 6864: immer mit Kerning, auch auf dem Drucker!
            KSHORT nChar = Width() / nCharWidth;
            if ( cFill == '_' )
                ++nChar; // damit keine Luecken entstehen (Bug 13430)
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
