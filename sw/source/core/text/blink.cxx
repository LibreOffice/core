/*************************************************************************
 *
 *  $RCSfile: blink.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:25:29 $
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


#pragma hdrstop

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
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

IMPL_LINK( SwBlink, Blinker, Timer *, pTimer )
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
                      const SwTxtFrm *pTxtFrm, USHORT nDir )
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


