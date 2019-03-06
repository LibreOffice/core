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
#include <swrect.hxx>
#include <rootfrm.hxx>

SwSaveClip::~SwSaveClip()
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

void SwSaveClip::ChgClip_( const SwRect &rRect, const SwTextFrame* pFrame,
                           bool bEnlargeRect )
{
    SwRect aOldRect( rRect );
    const bool bVertical = pFrame && pFrame->IsVertical();

    if ( pFrame && pFrame->IsRightToLeft() )
        pFrame->SwitchLTRtoRTL( const_cast<SwRect&>(rRect) );

    if ( bVertical )
        pFrame->SwitchHorizontalToVertical( const_cast<SwRect&>(rRect) );

    if ( !pOut || (!rRect.HasArea() && !pOut->IsClipRegion()) )
    {
        const_cast<SwRect&>(rRect) = aOldRect;
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
        tools::Rectangle aRect( rRect.SVRect() );

        // Having underscores in our line, we enlarged the repaint area
        // (see frmform.cxx) because for some fonts it could be too small.
        // Consequently, we have to enlarge the clipping rectangle as well.
        if ( bEnlargeRect && ! bVertical )
            aRect.AdjustBottom(40 );

        // If the ClipRect is identical, nothing will happen
        if( pOut->IsClipRegion() ) // no && because of Mac
        {
            if ( aRect == pOut->GetClipRegion().GetBoundRect() )
            {
                const_cast<SwRect&>(rRect) = aOldRect;
                return;
            }
        }

        if( SwRootFrame::HasSameRect( rRect ) )
            pOut->SetClipRegion();
        else
        {
            const vcl::Region aClipRegion( aRect );
            pOut->SetClipRegion( aClipRegion );
        }
    }
    bChg = true;

    const_cast<SwRect&>(rRect) = aOldRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
