/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "txtpaint.hxx"
#include "swrect.hxx"
#include "rootfrm.hxx"

/*************************************************************************
 *                      SwSaveClip::Reset()
 *************************************************************************/

void SwSaveClip::Reset()
{
    // We recover the old state
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
        bChg = false;
    }
}

/*************************************************************************
 *                      SwSaveClip::_ChgClip()
 *************************************************************************/

void SwSaveClip::_ChgClip( const SwRect &rRect, const SwTxtFrm* pFrm,
                           bool bEnlargeRect )
{
    SwRect aOldRect( rRect );
    const bool bVertical = pFrm && pFrm->IsVertical();

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

        // If the ClipRect is identical, nothing will happen
        if( pOut->IsClipRegion() ) // no && because of Mac
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
        }
#ifdef DBG_UTIL
        static bool bDbg = false;
        if( bDbg )
        {
            DbgBackColor aDbg( pOut, bDbg, COL_RED );
            pOut->DrawRect( aRect );
        }
#endif
    }
    bChg = true;

    (SwRect&)rRect = aOldRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
