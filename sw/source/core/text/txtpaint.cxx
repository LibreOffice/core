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
#include <txtfrm.hxx>
#include <swrect.hxx>
#include <rootfrm.hxx>

SwSaveClip::~SwSaveClip()
{
    // We recover the old state
    if( !(m_pOut && m_bChg) )
        return;

    if ( m_pOut->GetConnectMetaFile() )
        m_pOut->Pop();
    else
    {
        if( m_bOn )
            m_pOut->SetClipRegion( m_aClip );
        else
            m_pOut->SetClipRegion();
    }
    m_bChg = false;
}

void SwSaveClip::ChgClip_( const SwRect &rRect, const SwTextFrame* pFrame,
                           sal_Int32 nEnlargeTop,
                           sal_Int32 nEnlargeBottom )
{
    SwRect aOldRect( rRect );
    const bool bVertical = pFrame && pFrame->IsVertical();

    if ( pFrame && pFrame->IsRightToLeft() )
        pFrame->SwitchLTRtoRTL( const_cast<SwRect&>(rRect) );

    if ( bVertical )
        pFrame->SwitchHorizontalToVertical( const_cast<SwRect&>(rRect) );

    if ( !m_pOut || (!rRect.HasArea() && !m_pOut->IsClipRegion()) )
    {
        const_cast<SwRect&>(rRect) = aOldRect;
        return;
    }

    if ( !m_bChg )
    {
        if ( m_pOut->GetConnectMetaFile() )
            m_pOut->Push();
        else if ( m_bOn )
            m_aClip = m_pOut->GetClipRegion();
    }

    if ( !rRect.HasArea() )
        m_pOut->SetClipRegion();
    else
    {
        tools::Rectangle aRect( rRect.SVRect() );

        // enlarge clip for paragraph margins at small fixed line height
        if ( nEnlargeTop > 0 )
            aRect.AdjustTop( -nEnlargeTop );

        if ( nEnlargeBottom > 0 )
            aRect.AdjustBottom( nEnlargeBottom );

        // If the ClipRect is identical, nothing will happen
        if( m_pOut->IsClipRegion() ) // no && because of Mac
        {
            if ( aRect == m_pOut->GetClipRegion().GetBoundRect() )
            {
                const_cast<SwRect&>(rRect) = aOldRect;
                return;
            }
        }

        if( SwRootFrame::HasSameRect( rRect ) )
            m_pOut->SetClipRegion();
        else
        {
            const vcl::Region aClipRegion( aRect );
            m_pOut->SetClipRegion( aClipRegion );
        }
    }
    m_bChg = true;

    const_cast<SwRect&>(rRect) = aOldRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
