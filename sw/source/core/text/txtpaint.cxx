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
        static sal_Bool bDbg = sal_False;
        if( bDbg )
        {
            DbgBackColor aDbg( pOut, bDbg, COL_RED );
            pOut->DrawRect( aRect );
        }
#endif
    }
    bChg = sal_True;

    (SwRect&)rRect = aOldRect;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
