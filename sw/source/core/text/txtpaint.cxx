/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtpaint.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 16:45:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma hdrstop


#include "txtpaint.hxx"
#include "swrect.hxx"
#include "rootfrm.hxx"

/*************************************************************************
 *                      SwSaveClip::Reset()
 *************************************************************************/

void SwSaveClip::Reset()
{
    // Der alte Zustand wird wieder hergestellt.
    if( pOut && bChg )
    {
        if ( pOut->GetConnectMetaFile() )
            pOut->Pop();
        else
        {
            if( bOn )
                pOut->SetClipRegion( aClip );
            else
                pOut->SetClipRegion();
        }
        bChg = sal_False;
    }
}

/*************************************************************************
 *                      SwSaveClip::_ChgClip()
 *************************************************************************/

void SwSaveClip::_ChgClip( const SwRect &rRect, const SwTxtFrm* pFrm,
                           sal_Bool bEnlargeRect )
{
    SwRect aOldRect( rRect );
    const sal_Bool bVertical = pFrm && pFrm->IsVertical();

    if ( pFrm && pFrm->IsRightToLeft() )
        pFrm->SwitchLTRtoRTL( (SwRect&)rRect );

    if ( bVertical )
        pFrm->SwitchHorizontalToVertical( (SwRect&)rRect );

    if ( !pOut || (!rRect.HasArea() && !pOut->IsClipRegion()) )
    {
        (SwRect&)rRect = aOldRect;
        return;
    }

    if ( !bChg )
    {
        if ( pOut->GetConnectMetaFile() )
            pOut->Push();
        else if ( bOn )
            aClip = pOut->GetClipRegion();
    }

    if ( !rRect.HasArea() )
        pOut->SetClipRegion();
    else
    {
        Rectangle aRect( rRect.SVRect() );

        // Having underscores in our line, we enlarged the repaint area
        // (see frmform.cxx) because for some fonts it could be too small.
        // Consequently, we have to enlarge the clipping rectangle as well.
        if ( bEnlargeRect && ! bVertical )
            aRect.Bottom() += 40;

        // Wenn das ClipRect identisch ist, passiert nix.
        if( pOut->IsClipRegion() ) // kein && wg Mac
        {
            if ( aRect == pOut->GetClipRegion().GetBoundRect() )
            {
                (SwRect&)rRect = aOldRect;
                return;
            }
        }

        if( SwRootFrm::HasSameRect( rRect ) )
            pOut->SetClipRegion();
        else
        {
            const Region aClipRegion( aRect );
            pOut->SetClipRegion( aClipRegion );
#if OSL_DEBUG_LEVEL > 1
            Rectangle aDbgRect = pOut->GetClipRegion().GetBoundRect();
#endif
        }
#if OSL_DEBUG_LEVEL > 1
#ifndef PRODUCT
        static sal_Bool bDbg = sal_False;
        if( bDbg )
        {
            DbgBackColor aDbg( pOut, bDbg, COL_RED );
            pOut->DrawRect( aRect );
        }
#endif
#endif
    }
    bChg = sal_True;

    (SwRect&)rRect = aOldRect;
}


