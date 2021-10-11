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

#include <vcl/vclevent.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>

#include <vcleventlisteners.hxx>

void VclEventListeners::Call( VclSimpleEvent& rEvent ) const
{
    if ( m_aListeners.empty() )
        return;

    // Copy the list, because this can be destroyed when calling a Link...
    std::vector<Link<VclSimpleEvent&,void>> aCopy( m_aListeners );
    std::vector<Link<VclSimpleEvent&,void>>::iterator aIter( aCopy.begin() );
    std::vector<Link<VclSimpleEvent&,void>>::const_iterator aEnd( aCopy.end() );
    m_updated = false;
    if (VclWindowEvent* pWindowEvent = dynamic_cast<VclWindowEvent*>(&rEvent))
    {
        VclPtr<vcl::Window> xWin(pWindowEvent->GetWindow());
        // checking mpWindowImpl to see if disposal is complete yet
        while ( aIter != aEnd && (!xWin || xWin->mpWindowImpl) )
        {
            Link<VclSimpleEvent&,void> &rLink = *aIter;
            // check this hasn't been removed in some re-enterancy scenario fdo#47368
            // But only check if the list actually has been changed.
            if( !m_updated || std::find(m_aListeners.begin(), m_aListeners.end(), rLink) != m_aListeners.end() )
                rLink.Call( rEvent );
            ++aIter;
        }
    }
    else
    {
        while ( aIter != aEnd )
        {
            Link<VclSimpleEvent&,void> &rLink = *aIter;
            if( !m_updated || std::find(m_aListeners.begin(), m_aListeners.end(), rLink) != m_aListeners.end() )
                rLink.Call( rEvent );
            ++aIter;
        }
    }
}

void VclEventListeners::addListener( const Link<VclSimpleEvent&,void>& rListener )
{
    m_aListeners.push_back( rListener );
    m_updated = true;
}

void VclEventListeners::removeListener( const Link<VclSimpleEvent&,void>& rListener )
{
    m_aListeners.erase( std::remove(m_aListeners.begin(), m_aListeners.end(), rListener ), m_aListeners.end() );
    m_updated = true;
}

VclWindowEvent::VclWindowEvent( vcl::Window* pWin, VclEventId n, void* pDat ) : VclSimpleEvent(n)
{
    pWindow = pWin; pData = pDat;
}

VclWindowEvent::~VclWindowEvent() {}

VclMenuEvent::VclMenuEvent( Menu* pM, VclEventId n, sal_uInt16 nPos )
    : VclSimpleEvent(n), pMenu(pM), mnPos(nPos)
{}

VclMenuEvent::~VclMenuEvent()
{}

Menu* VclMenuEvent::GetMenu() const
{
    return pMenu;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
