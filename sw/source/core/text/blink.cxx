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


#include <tools/debug.hxx>
#include "viewsh.hxx"
#include "rootfrm.hxx"  // GetOleShell()
#include "txtfrm.hxx"   // FindRootFrm()
#include "blink.hxx"
#include "porlin.hxx"
#include "porlay.hxx"   // SwLineLayout

// Sichtbare Zeit:
#define BLINK_ON_TIME       2400L
// Nihct sichtbare Zeit:
#define BLINK_OFF_TIME      800L

/*************************************************************************
 * pBlink zeigt auf die Instanz, bei der sich blinkende Portions anmelden
 * muessen, ggf. muss pBlink erst per new SwBlink angelegt werden.
 * Diese werden dann rhythmisch zum Repaint angeregt und koennen abfragen,
 * ob sie zur Zeit sichtbar oder unsichtbar sein sollen ( IsVisible() ).
 *************************************************************************/
SwBlink *pBlink = NULL;


// Liste von blinkenden Portions
SV_IMPL_OP_PTRARR_SORT( SwBlinkList, SwBlinkPortionPtr )

SwBlink::SwBlink()
{
    bVisible = sal_True;
    // Den Timer vorbereiten
    aTimer.SetTimeout( BLINK_ON_TIME );
    aTimer.SetTimeoutHdl( LINK(this, SwBlink, Blinker) );
}

SwBlink::~SwBlink( )
{
    aTimer.Stop();
}

/*************************************************************************
 * SwBlink::Blinker (Timerablauf):
 * Sichtbar/unsichtbar-Flag toggeln.
 * Repaint-Rechtecke der Blinkportions ermitteln und an ihren OleShells
 * invalidieren.
 *************************************************************************/

IMPL_LINK( SwBlink, Blinker, Timer *, EMPTYARG )
{
    bVisible = !bVisible;
    if( bVisible )
        aTimer.SetTimeout( BLINK_ON_TIME );
    else
        aTimer.SetTimeout( BLINK_OFF_TIME );
    if( aList.Count() )
    {

        for( MSHORT nPos = 0; nPos < aList.Count(); )
        {
            const SwBlinkPortion* pTmp = aList[ nPos ];
            if( pTmp->GetRootFrm() &&
                ((SwRootFrm*)pTmp->GetRootFrm())->GetCurrShell() )
            {
                ++nPos;

                Point aPos = pTmp->GetPos();
                long nWidth, nHeight;
                switch ( pTmp->GetDirection() )
                {
                    case 900:
                        aPos.X() -= pTmp->GetPortion()->GetAscent();
                        aPos.Y() -= pTmp->GetPortion()->Width();
                        nWidth = pTmp->GetPortion()->SvLSize().Height();
                        nHeight = pTmp->GetPortion()->SvLSize().Width();
                        break;
                    case 1800:
                        aPos.Y() -= pTmp->GetPortion()->Height() -
                                    pTmp->GetPortion()->GetAscent();
                        aPos.X() -= pTmp->GetPortion()->Width();
                        nWidth = pTmp->GetPortion()->SvLSize().Width();
                        nHeight = pTmp->GetPortion()->SvLSize().Height();
                        break;
                    case 2700:
                        aPos.X() -= pTmp->GetPortion()->Height() -
                                    pTmp->GetPortion()->GetAscent();
                        nWidth = pTmp->GetPortion()->SvLSize().Height();
                        nHeight = pTmp->GetPortion()->SvLSize().Width();
                        break;
                    default:
                        aPos.Y() -= pTmp->GetPortion()->GetAscent();
                        nWidth = pTmp->GetPortion()->SvLSize().Width();
                        nHeight = pTmp->GetPortion()->SvLSize().Height();
                }

                Rectangle aRefresh( aPos, Size( nWidth, nHeight ) );
                aRefresh.Right() += ( aRefresh.Bottom()- aRefresh.Top() ) / 8;
                ((SwRootFrm*)pTmp->GetRootFrm())
                    ->GetCurrShell()->InvalidateWindows( aRefresh );
            }
            else // Portions ohne Shell koennen aus der Liste entfernt werden.
                aList.Remove( nPos );
        }
    }
    else // Wenn die Liste leer ist, kann der Timer gestoppt werden.
        aTimer.Stop();
    return sal_True;
}

void SwBlink::Insert( const Point& rPoint, const SwLinePortion* pPor,
                      const SwTxtFrm *pTxtFrm, sal_uInt16 nDir )
{
    SwBlinkPortion *pBlinkPor = new SwBlinkPortion( pPor, nDir );

    MSHORT nPos;
    if( aList.Seek_Entry( pBlinkPor, &nPos ) )
    {
        aList[ nPos ]->SetPos( rPoint );
        delete pBlinkPor;
    }
    else
    {
        pBlinkPor->SetPos( rPoint );
        pBlinkPor->SetRootFrm( pTxtFrm->FindRootFrm() );
        aList.Insert( pBlinkPor );
        pTxtFrm->SetBlinkPor();
        if( pPor->IsLayPortion() || pPor->IsParaPortion() )
            ((SwLineLayout*)pPor)->SetBlinking( sal_True );

        if( !aTimer.IsActive() )
            aTimer.Start();
    }
}

void SwBlink::Replace( const SwLinePortion* pOld, const SwLinePortion* pNew )
{
    // setting direction to 0 because direction does not matter
    // for this operation
    SwBlinkPortion aBlink( pOld, 0 );
    MSHORT nPos;
    if( aList.Seek_Entry( &aBlink, &nPos ) )
    {
        SwBlinkPortion* pTmp = new SwBlinkPortion( aList[ nPos ], pNew );
        aList.Remove( nPos );
        aList.Insert( pTmp );
    }
}

void SwBlink::Delete( const SwLinePortion* pPor )
{
    // setting direction to 0 because direction does not matter
    // for this operation
    SwBlinkPortion aBlink( pPor, 0 );
    MSHORT nPos;
    if( aList.Seek_Entry( &aBlink, &nPos ) )
        aList.Remove( nPos );
}

void SwBlink::FrmDelete( const SwRootFrm* pRoot )
{
    for( MSHORT nPos = 0; nPos < aList.Count(); )
    {
        if( pRoot == aList[ nPos ]->GetRootFrm() )
            aList.Remove( nPos );
        else
            ++nPos;
    }
}


