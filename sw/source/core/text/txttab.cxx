/*************************************************************************
 *
 *  $RCSfile: txttab.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif

#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#include "viewopt.hxx"  // SwViewOptions
#include "txtcfg.hxx"
#include "portab.hxx"
#include "inftxt.hxx"
#include "itrform2.hxx"
#include "txtfrm.hxx"


/*************************************************************************
 *                    SwLineInfo::GetTabStop()
 *************************************************************************/

/* Die Werte in SvxTabStop::nTabPos liegen immer relativ zum linken PrtRand
 * vor. Tabs, die im Bereich des Erstzeileneinzugs liegen, sind also negativ.
 * nLeft  ist der linke PrtRand
 * nRight ist der rechte PrtRand
 * nLinePos die aktuelle Position.
 * Es wird der erste Tabstop returnt, der groesser ist als nLinePos.
 */



const SvxTabStop *SwLineInfo::GetTabStop( const KSHORT nLinePos,
    const KSHORT nLeft, const KSHORT nRight ) const
{
    // Mit den KSHORTs aufpassen, falls nLinePos < nLeft
    SwTwips nPos = nLinePos;
    nPos -= nLeft;
    for( MSHORT i = 0; i < pRuler->Count(); ++i )
    {
        const SvxTabStop &rTabStop = pRuler->operator[](i);
        if( rTabStop.GetTabPos() > SwTwips(nRight) )
        {
            if ( i )
                return 0;
            else
                return &rTabStop;
        }
        if( rTabStop.GetTabPos() > nPos )
            return &rTabStop;
    }
    return 0;
}

/*************************************************************************
 *                      SwTxtFormatter::NewTabPortion()
 *************************************************************************/



SwTabPortion *SwTxtFormatter::NewTabPortion( SwTxtFormatInfo &rInf ) const
{
    SwTabPortion *pTabPor;
    SwTabPortion  *pLastTab = rInf.GetLastTab();
    if( pLastTab && pLastTab->IsTabCntPortion() )
        if( pLastTab->PostFormat( rInf ) )
            return 0;

    // Wir suchen den naechsten Tab. Wenn gerade ein rechts-Tab unterwegs
    // ist, so koennen wir uns nicht auf rInf.X() beziehen.
    KSHORT nTabPos = rInf.GetLastTab() ? rInf.GetLastTab()->GetTabPos() : 0;
    if( nTabPos < rInf.X() )
        nTabPos = rInf.X();

    xub_Unicode cFill, cDec;
    SvxTabAdjust eAdj;

    KSHORT nNewTabPos;
    {
       /*
        nPos ist der Offset in der Zeile.
        Die Tabulatoren haben ihren 0-Punkt bei Frm().Left().
        Die Zeilen beginnen ab Frm.Left() + Prt.Left().
        In dieser Methode wird zwischen beiden Koordinatensystemen
        konvertiert (vgl. rInf.GetTabPos).
       */
        const KSHORT nTabLeft = KSHORT(pFrm->Frm().Left()) +
                     KSHORT(pFrm->GetAttrSet()->GetLRSpace().GetTxtLeft());
                //    + KSHORT(pFrm->GetTxtNode()->GetLeftMarginWithNum( sal_True ));
        const KSHORT nLinePos = KSHORT(GetLeftMargin());
        const KSHORT nLineTab = nLinePos + nTabPos;
        const KSHORT nRight = KSHORT(Right());
        SwTwips nNextPos;
        const SvxTabStop* pTabStop =
            aLineInf.GetTabStop( nLineTab, nTabLeft, nRight );
        if( pTabStop )
        {
            cFill = ' ' != pTabStop->GetFill() ? pTabStop->GetFill() : 0;
            cDec = pTabStop->GetDecimal();
            eAdj = pTabStop->GetAdjustment();
            nNextPos = pTabStop->GetTabPos();
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
            SwTwips nCount = nLineTab;
            nCount -= nTabLeft;
            // Bei negativen Werten rundet "/" auf, "%" liefert negative Reste,
            // bei positiven Werten rundet "/" ab, "%" liefert positvie Reste!
            KSHORT nPlus = nCount < 0 ? 0 : 1;
            nCount /= nDefTabDist;
            nNextPos = ( nCount + nPlus ) * nDefTabDist ;
            if( nNextPos + nTabLeft <= nLineTab + 50 )
                nNextPos += nDefTabDist;
            cFill = 0;
            eAdj = SVX_TAB_ADJUST_LEFT;
        }
        long nForced = 0;
        if( pCurr->HasForcedLeftMargin() )
        {
            SwLinePortion* pPor = pCurr->GetPortion();
            while( pPor && !pPor->IsFlyPortion() )
                pPor = pPor->GetPortion();
            if( pPor )
                nForced = pPor->Width();
        }
        if( nTabLeft + nForced > nLineTab && nNextPos > 0 )
        {
            eAdj = SVX_TAB_ADJUST_DEFAULT;
            cFill = 0;
            nNextPos = nForced;
        }
        nNextPos += nTabLeft;
        nNextPos -= nLinePos;
        ASSERT( nNextPos >= 0, "GetTabStop: Don't go back!" );
        nNewTabPos = KSHORT(nNextPos);
    }

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
            ASSERT( SVX_TAB_ADJUST_LEFT == eAdj || SVX_TAB_ADJUST_DEFAULT == eAdj,
                    "+SwTxtFormatter::NewTabPortion: unknown adjustment" );
            pTabPor = new SwTabLeftPortion( nNewTabPos, cFill );
            break;
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


SwTabPortion::SwTabPortion( const KSHORT nTabPos, const xub_Unicode cFill )
    : SwFixPortion( 0, 0 ), nTabPos(nTabPos), cFill(cFill)
{
    nLineLength = 1;
#ifndef PRODUCT
    if( IsFilled() )
    {
        ASSERT( ' ' != cFill, "SwTabPortion::CTOR: blanks ?!" );
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
    ASSERT( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );

    // Hier lassen wir uns nieder...
    Fix( rInf.X() );

    // Die Mindestbreite eines Tabs ist immer mindestens ein Blank
    {
        XubString aTmp( ' ' );
        SwTxtSizeInfo aInf( rInf, aTmp );
        PrtWidth( aInf.GetTxtSize().Width() );
    }

    // 8532: CenterTabs, deren Blankbreite nicht mehr in die Zeile passt
    sal_Bool bFull = rInf.Width() <= rInf.X() + PrtWidth();

    if( !bFull )
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
                PrtWidth( GetTabPos() - rInf.X() );
                bFull = rInf.Width() <= rInf.X() + PrtWidth();
                break;
            }
            default: ASSERT( !this, "SwTabPortion::PreFormat: unknown adjustment" );
        }
    }

    if( bFull )
    {
        // Wir muessen aufpassen, dass wir nicht endlos schleifen,
        // wenn die Breite kleiner ist, als ein Blank ...
        if( rInf.GetIdx() == rInf.GetLineStart() )
        {
            PrtWidth( rInf.Width() - rInf.X() );
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
        DBG_LOOP;
        nPorWidth += pPor->Width();
        pPor = pPor->GetPortion();
    }

    const MSHORT nWhich = GetWhichPor();
    ASSERT( POR_TABLEFT != nWhich, "SwTabPortion::PostFormat: already formatted" );
    const KSHORT nDiffWidth = nRight - Fix();

    if( POR_TABCENTER == nWhich )
    {
        // zentrierte Tabs bereiten Probleme:
        // Wir muessen den Anteil herausfinden, der noch auf die Zeile passt.
        KSHORT nNewWidth = nPorWidth /2;
        if( nNewWidth > rInf.Width() - nRight )
            nNewWidth = nPorWidth - (rInf.Width() - nRight);
        nPorWidth = nNewWidth;
    }

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
#ifndef PRODUCT
    // Wir wollen uns die Fixbreite anzeigen
    if( rInf.OnWin() && OPTDBG( rInf ) && rInf.GetOpt().IsField() )
    {
        const KSHORT nWidth = PrtWidth();
        ((SwTabPortion*)this)->PrtWidth( GetFixWidth() );
        rInf.DrawViewOpt( *this, POR_TAB );
        ((SwTabPortion*)this)->PrtWidth( nWidth );
    }
#endif

    rInf.DrawBackBrush( *this );
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
#ifdef DEBUG
        ASSERT( nCharWidth, "!SwTabPortion::Paint: sophisticated tabchar" );
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
 *                class SwTabRightPortion
 *************************************************************************/

#ifdef OLDRECYCLE



sal_Bool SwTabRightPortion::MayRecycle() const { return sal_False; }

#endif

/*************************************************************************
 *                class SwTabCenterPortion
 *************************************************************************/

#ifdef OLDRECYCLE



sal_Bool SwTabCenterPortion::MayRecycle() const { return sal_False; }

#endif

/*************************************************************************
 *                class SwTabDecimalPortion
 *************************************************************************/

#ifdef OLDRECYCLE



sal_Bool SwTabDecimalPortion::MayRecycle() const { return sal_False; }

#endif




