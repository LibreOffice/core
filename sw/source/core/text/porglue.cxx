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

#include "swrect.hxx"
#include "paratr.hxx"   // pTabStop, ADJ*
#include "viewopt.hxx"  // SwViewOptions
#include <SwPortionHandler.hxx>

#include "porglue.hxx"
#include "inftxt.hxx"
#include "porlay.hxx"   // SwParaPortion, SetFull
#include "porfly.hxx"   // SwParaPortion, SetFull

/*************************************************************************
 *                      class SwGluePortion
 *************************************************************************/

SwGluePortion::SwGluePortion( const KSHORT nInitFixWidth )
    : nFixWidth( nInitFixWidth )
{
    PrtWidth( nFixWidth );
    SetWhichPor( POR_GLUE );
}

/*************************************************************************
 *                virtual SwGluePortion::GetCrsrOfst()
 *************************************************************************/

xub_StrLen SwGluePortion::GetCrsrOfst( const KSHORT nOfst ) const
{
    if( !GetLen() || nOfst > GetLen() || !Width() )
        return SwLinePortion::GetCrsrOfst( nOfst );
    else
        return nOfst / (Width() / GetLen());
}

/*************************************************************************
 *                virtual SwGluePortion::GetTxtSize()
 *************************************************************************/

SwPosSize SwGluePortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
{
    if( 1 >= GetLen() || rInf.GetLen() > GetLen() || !Width() || !GetLen() )
        return SwPosSize(*this);
    else
        return SwPosSize( (Width() / GetLen()) * rInf.GetLen(), Height() );
}

/*************************************************************************
 *              virtual SwGluePortion::GetExpTxt()
 *************************************************************************/

sal_Bool SwGluePortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
{
    if( GetLen() && rInf.OnWin() &&
        rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
        rTxt.Fill( GetLen(), CH_BULLET );
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
 *                virtual SwGluePortion::Paint()
 *************************************************************************/

void SwGluePortion::Paint( const SwTxtPaintInfo &rInf ) const
{
    if( !GetLen() )
        return;

    if( rInf.GetFont()->IsPaintBlank() )
    {
        XubString aTxt;
        aTxt.Fill( GetFixWidth() / GetLen(), ' ' );
        SwTxtPaintInfo aInf( rInf, aTxt );
        aInf.DrawText( *this, aTxt.Len(), sal_True );
    }

    if( rInf.OnWin() && rInf.GetOpt().IsBlank() && rInf.IsNoSymbol() )
    {
#if OSL_DEBUG_LEVEL > 0
        const sal_Unicode cChar = rInf.GetChar( rInf.GetIdx() );
        OSL_ENSURE( CH_BLANK  == cChar || CH_BULLET == cChar,
                "SwGluePortion::Paint: blank expected" );
#endif
        if( 1 == GetLen() )
        {
            rtl::OUString aBullet( CH_BULLET );
            SwPosSize aBulletSize( rInf.GetTxtSize( aBullet ) );
            Point aPos( rInf.GetPos() );
            aPos.X() += (Width()/2) - (aBulletSize.Width()/2);
            SwTxtPaintInfo aInf( rInf, aBullet );
            aInf.SetPos( aPos );
            SwTxtPortion aBulletPor;
            aBulletPor.Width( aBulletSize.Width() );
            aBulletPor.Height( aBulletSize.Height() );
            aBulletPor.SetAscent( GetAscent() );
            aInf.DrawText( aBulletPor, aBullet.getLength(), sal_True );
        }
        else
        {
            SwTxtSlot aSlot( &rInf, this, true, false );
            rInf.DrawText( *this, rInf.GetLen(), sal_True );
        }
    }
}

/*************************************************************************
 *                      SwGluePortion::MoveGlue()
 *************************************************************************/

void SwGluePortion::MoveGlue( SwGluePortion *pTarget, const short nPrtGlue )
{
    short nPrt = Min( nPrtGlue, GetPrtGlue() );
    if( 0 < nPrt )
    {
        pTarget->AddPrtWidth( nPrt );
        SubPrtWidth( nPrt );
    }
}

/*************************************************************************
 *                void SwGluePortion::Join()
 *************************************************************************/

void SwGluePortion::Join( SwGluePortion *pVictim )
{
    // Die GluePortion wird ausgesogen und weggespuelt ...
    AddPrtWidth( pVictim->PrtWidth() );
    SetLen( pVictim->GetLen() + GetLen() );
    if( Height() < pVictim->Height() )
        Height( pVictim->Height() );

    AdjFixWidth();
    Cut( pVictim );
    delete pVictim;
}

/*************************************************************************
 *                class SwFixPortion
 *************************************************************************/

// Wir erwarten ein framelokales SwRect !
SwFixPortion::SwFixPortion( const SwRect &rRect )
       :SwGluePortion( KSHORT(rRect.Width()) ), nFix( KSHORT(rRect.Left()) )
{
    Height( KSHORT(rRect.Height()) );
    SetWhichPor( POR_FIX );
}

SwFixPortion::SwFixPortion(const KSHORT nFixedWidth, const KSHORT nFixedPos)
       : SwGluePortion(nFixedWidth), nFix(nFixedPos)
{
    SetWhichPor( POR_FIX );
}

/*************************************************************************
 *                class SwMarginPortion
 *************************************************************************/

SwMarginPortion::SwMarginPortion( const KSHORT nFixedWidth )
    :SwGluePortion( nFixedWidth )
{
    SetWhichPor( POR_MARGIN );
}

/*************************************************************************
 *                SwMarginPortion::AdjustRight()
 *
 * In der umschliessenden Schleife werden alle Portions durchsucht,
 * dabei werden erst die am Ende liegenden GluePortions verarbeitet.
 * Das Ende wird nach jeder Schleife nach vorne verlegt, bis keine
 * GluePortions mehr vorhanden sind.
 * Es werden immer GluePortion-Paare betrachtet (pLeft und pRight),
 * wobei Textportions zwischen pLeft und pRight hinter pRight verschoben
 * werden, wenn pRight genuegend Glue besitzt. Bei jeder Verschiebung
 * wandert ein Teil des Glues von pRight nach pLeft.
 * Im naechsten Schleifendurchlauf ist pLeft das pRight und das Spiel
 * beginnt von vorne.
 *************************************************************************/

void SwMarginPortion::AdjustRight( const SwLineLayout *pCurr )
{
    SwGluePortion *pRight = 0;
    sal_Bool bNoMove = 0 != pCurr->GetpKanaComp();
    while( pRight != this )
    {

        // 1) Wir suchen den linken Glue
        SwLinePortion *pPos = (SwLinePortion*)this;
        SwGluePortion *pLeft = 0;
        while( pPos )
        {
            if( pPos->InFixMargGrp() )
                pLeft = (SwGluePortion*)pPos;
            pPos = pPos->GetPortion();
            if( pPos == pRight)
                pPos = 0;
        }

        // Zwei nebeneinander liegende FlyPortions verschmelzen
        if( pRight && pLeft->GetPortion() == pRight )
        {
            pRight->MoveAllGlue( pLeft );
            pRight = 0;
        }
        KSHORT nRightGlue = pRight && 0 < pRight->GetPrtGlue()
                          ? KSHORT(pRight->GetPrtGlue()) : 0;
        // 2) linken und rechten Glue ausgleichen
        //    Bei Tabs haengen wir nix um ...
        if( pLeft && nRightGlue && !pRight->InTabGrp() )
        {
            // pPrev ist die Portion, die unmittelbar vor pRight liegt.
            SwLinePortion *pPrev = pRight->FindPrevPortion( pLeft );

            if ( pRight->IsFlyPortion() && pRight->GetLen() )
            {
                SwFlyPortion *pFly = (SwFlyPortion *)pRight;
                if ( pFly->GetBlankWidth() < nRightGlue )
                {
                    // Hier entsteht eine neue TxtPortion, die dass zuvor
                    // vom Fly verschluckte Blank reaktiviert.
                    nRightGlue = nRightGlue - pFly->GetBlankWidth();
                    pFly->SubPrtWidth( pFly->GetBlankWidth() );
                    pFly->SetLen( 0 );
                    SwTxtPortion *pNewPor = new SwTxtPortion;
                    pNewPor->SetLen( 1 );
                    pNewPor->Height( pFly->Height() );
                    pNewPor->Width( pFly->GetBlankWidth() );
                    pFly->Insert( pNewPor );
                }
                else
                    pPrev = pLeft;
            }
            while( pPrev != pLeft )
            {
                if( bNoMove || pPrev->PrtWidth() >= nRightGlue ||
                    pPrev->InHyphGrp() || pPrev->IsKernPortion() )
                {
                    // Die Portion, die vor pRight liegt kann nicht
                    // verschoben werden, weil kein Glue mehr vorhanden ist.
                    // Wir fuehren die Abbruchbedingung herbei:
                    pPrev = pLeft;
                }
                else
                {
                    nRightGlue = nRightGlue - pPrev->PrtWidth();
                    // pPrev wird hinter pRight verschoben.
                    // Dazu wird der Gluewert zwischen pRight und pLeft
                    // ausgeglichen.
                    pRight->MoveGlue( pLeft, short( pPrev->PrtWidth() ) );
                    // Jetzt wird die Verkettung gerichtet.
                    SwLinePortion *pPrevPrev = pPrev->FindPrevPortion( pLeft );
                    pPrevPrev->SetPortion( pRight );
                    pPrev->SetPortion( pRight->GetPortion() );
                    pRight->SetPortion( pPrev );
                    if ( pPrev->GetPortion() && pPrev->InTxtGrp()
                         && pPrev->GetPortion()->IsHolePortion() )
                    {
                        SwHolePortion *pHolePor =
                            (SwHolePortion*)pPrev->GetPortion();
                        if ( !pHolePor->GetPortion() ||
                             !pHolePor->GetPortion()->InFixMargGrp() )
                        {
                            pPrev->AddPrtWidth( pHolePor->GetBlankWidth() );
                            pPrev->SetLen( pPrev->GetLen() + 1 );
                            pPrev->SetPortion( pHolePor->GetPortion() );
                            delete pHolePor;
                        }
                    }
                    pPrev = pPrevPrev;
                }
            }
        }
        // Wenn es keinen linken Glue mehr gibt, wird die Abbruchbedingung
        // herbeigefuehrt.
        pRight = pLeft ? pLeft : (SwGluePortion*)this;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
