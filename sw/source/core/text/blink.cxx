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
 * pBlink points to the instance where blinking portions need to register.
 * If necessary, it needs to be created by SwBlink.
 * They are then triggered rhythimcally for a repaint. They can query
 * for being visible or invisible with IsVisible().
 *************************************************************************/
SwBlink *pBlink = NULL;


SwBlink::SwBlink()
{
    bVisible = sal_True;
    // Prepare the timer
    aTimer.SetTimeout( BLINK_ON_TIME );
    aTimer.SetTimeoutHdl( LINK(this, SwBlink, Blinker) );
}

SwBlink::~SwBlink( )
{
    aTimer.Stop();
}

/*************************************************************************
 * SwBlink::Blinker (timer):
 * Toggle visibility flag
 * Determine the repaint rectangle and invalidate them in their OleShells.
 *************************************************************************/

IMPL_LINK_NOARG(SwBlink, Blinker)
{
    bVisible = !bVisible;
    if( bVisible )
        aTimer.SetTimeout( BLINK_ON_TIME );
    else
        aTimer.SetTimeout( BLINK_OFF_TIME );
    if( !aList.empty() )
    {

        for( SwBlinkList::iterator it = aList.begin(); it != aList.end(); )
        {
            const SwBlinkPortion* pTmp = &*it;
            if( pTmp->GetRootFrm() &&
                ((SwRootFrm*)pTmp->GetRootFrm())->GetCurrShell() )
            {
                ++it;

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
            else // Portions without a shell can be removed from the list
                aList.erase( it );
        }
    }
    else // If the list is empty, the timer can be stopped
        aTimer.Stop();
    return sal_True;
}

void SwBlink::Insert( const Point& rPoint, const SwLinePortion* pPor,
                      const SwTxtFrm *pTxtFrm, sal_uInt16 nDir )
{
    SwBlinkPortion *pBlinkPor = new SwBlinkPortion( pPor, nDir );

    SwBlinkList::iterator it = aList.find( *pBlinkPor );
    if( it != aList.end()  )
    {
        (*it).SetPos( rPoint );
        delete pBlinkPor;
    }
    else
    {
        pBlinkPor->SetPos( rPoint );
        pBlinkPor->SetRootFrm( pTxtFrm->getRootFrm() );
        aList.insert( pBlinkPor );
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
    SwBlinkList::iterator it = aList.find( aBlink );
    if( it != aList.end()  )
    {
        SwBlinkPortion* aTmp = new SwBlinkPortion( &*it, pNew );
        aList.erase( it );
        aList.insert( aTmp );
    }
}

void SwBlink::Delete( const SwLinePortion* pPor )
{
    // setting direction to 0 because direction does not matter
    // for this operation
    SwBlinkPortion aBlink( pPor, 0 );
    aList.erase( aBlink );
}

void SwBlink::FrmDelete( const SwRootFrm* pRoot )
{
    for( SwBlinkList::iterator it = aList.begin(); it != aList.end(); )
    {
        if( pRoot == (*it).GetRootFrm() )
            aList.erase( it++ );
        else
            ++it;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
