/*************************************************************************
 *
 *  $RCSfile: porglue.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:25 $
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

#include "swrect.hxx"
#include "paratr.hxx"   // pTabStop, ADJ*
#include "viewopt.hxx"  // SwViewOptions
#include "errhdl.hxx"   // ASSERT
#include "segmentc.hxx"

#include "txtcfg.hxx"
#include "porglue.hxx"
#include "inftxt.hxx"
#include "porlay.hxx"   // SwParaPortion, SetFull
#include "porfly.hxx"   // SwParaPortion, SetFull

/*************************************************************************
 *                      class SwGluePortion
 *************************************************************************/

#ifdef OLDRECYCLE

sal_Bool SwGluePortion::MayRecycle() const { return sal_False; }

#endif

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
#ifndef PRODUCT
        const xub_Unicode cChar = rInf.GetChar( rInf.GetIdx() );
        ASSERT( CH_BLANK  == cChar || CH_BULLET == cChar,
                "SwGluePortion::Paint: blank expected" );
#endif
        if( 1 == GetLen() )
        {
            String aBullet( CH_BULLET, RTL_TEXTENCODING_MS_1252 );
            SwPosSize aBulletSize( rInf.GetTxtSize( aBullet ) );
            Point aPos( rInf.GetPos() );
            aPos.X() += (Width()/2) - (aBulletSize.Width()/2);
            SwTxtPaintInfo aInf( rInf, aBullet );
            aInf.SetPos( aPos );
            SwTxtPortion aBulletPor;
            aBulletPor.Width( aBulletSize.Width() );
            aBulletPor.Height( aBulletSize.Height() );
            aBulletPor.SetAscent( GetAscent() );
            aInf.DrawText( aBulletPor, aBullet.Len(), sal_True );
        }
        else
        {
            SwTxtSlotLen aSlot( &rInf, this );
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

SwFixPortion::SwFixPortion(const KSHORT nFixWidth, const KSHORT nFixPos)
       : SwGluePortion(nFixWidth), nFix(nFixPos)
{
    SetWhichPor( POR_FIX );
}

/*************************************************************************
 *                class SwMarginPortion
 *************************************************************************/

SwMarginPortion::SwMarginPortion( const KSHORT nFixWidth )
    :SwGluePortion( nFixWidth )
{
    SetWhichPor( POR_MARGIN );
}

#ifdef OLDRECYCLE

sal_Bool SwMarginPortion::MayRecycle() const { return sal_False; }

#endif

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

void SwMarginPortion::AdjustRight()
{
    SwGluePortion *pRight = 0;
    while( pRight != this )
    {

        // 1) Wir suchen den linken Glue
        SwLinePortion *pPos = (SwLinePortion*)this;
        SwGluePortion *pLeft = 0;
        while( pPos )
        {
            DBG_LOOP;
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
                    nRightGlue -= pFly->GetBlankWidth();
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
                DBG_LOOP;

                if( pPrev->PrtWidth() >= nRightGlue || pPrev->InHyphGrp() )
                {
                    // Die Portion, die vor pRight liegt kann nicht
                    // verschoben werden, weil kein Glue mehr vorhanden ist.
                    // Wir fuehren die Abbruchbedingung herbei:
                    pPrev = pLeft;
                }
                else
                {
                    nRightGlue -= pPrev->PrtWidth();
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



