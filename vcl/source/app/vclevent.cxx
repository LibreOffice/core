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

#include "vcl/vclevent.hxx"
#include "vcl/window.hxx"

#include "svdata.hxx"
#include "vcleventlisteners.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;

TYPEINIT0(VclSimpleEvent);
TYPEINIT1(VclWindowEvent, VclSimpleEvent);
TYPEINIT1(VclMenuEvent, VclSimpleEvent);

VclAccessibleEvent::VclAccessibleEvent( sal_uLong n, const Reference<XAccessible>& rxAccessible ) :
    VclSimpleEvent(n),
    mxAccessible(rxAccessible)
{
}

VclAccessibleEvent::~VclAccessibleEvent()
{
}


void VclEventListeners::Call( VclSimpleEvent* pEvent ) const
{
    if ( m_aListeners.empty() )
        return;

    // Copy the list, because this can be destroyed when calling a Link...
    std::vector<Link<>> aCopy( m_aListeners );
    std::vector<Link<>>::iterator aIter( aCopy.begin() );
    std::vector<Link<>>::const_iterator aEnd( aCopy.end() );
    if( pEvent->IsA( VclWindowEvent::StaticType() ) )
    {
        VclWindowEvent* pWinEvent = static_cast<VclWindowEvent*>(pEvent);
        ImplDelData aDel( pWinEvent->GetWindow() );
        while ( aIter != aEnd && ! aDel.IsDead() )
        {
            Link<> &rLink = *aIter;
            // check this hasn't been removed in some re-enterancy scenario fdo#47368
            if( std::find(m_aListeners.begin(), m_aListeners.end(), rLink) != m_aListeners.end() )
                rLink.Call( pEvent );
            ++aIter;
        }
    }
    else
    {
        while ( aIter != aEnd )
        {
            Link<> &rLink = *aIter;
            if( std::find(m_aListeners.begin(), m_aListeners.end(), rLink) != m_aListeners.end() )
                rLink.Call( pEvent );
            ++aIter;
        }
    }
}

void VclEventListeners::addListener( const Link<>& rListener )
{
    m_aListeners.push_back( rListener );
}

void VclEventListeners::removeListener( const Link<>& rListener )
{
    m_aListeners.erase( std::remove(m_aListeners.begin(), m_aListeners.end(), rListener ), m_aListeners.end() );
}

VclEventListeners2::VclEventListeners2()
{
}

VclEventListeners2::~VclEventListeners2()
{
}

void VclEventListeners2::addListener( const Link<>& rListener )
{
    // ensure uniqueness
    if (std::find(m_aListeners.begin(), m_aListeners.end(), rListener) == m_aListeners.end())
       m_aListeners.push_back( rListener );
}

void VclEventListeners2::removeListener( const Link<>& rListener )
{
    m_aListeners.erase( std::remove(m_aListeners.begin(), m_aListeners.end(), rListener ), m_aListeners.end() );
}

void VclEventListeners2::callListeners( VclSimpleEvent* pEvent )
{
    vcl::DeletionListener aDel( this );

    // Copy the list, because this can be destroyed when calling a Link...
    std::vector<Link<>> aCopy( m_aListeners );
    std::vector<Link<>>::iterator aIter( aCopy.begin() );
    std::vector<Link<>>::const_iterator aEnd( aCopy.end() );

    while ( aIter != aEnd && ! aDel.isDeleted() )
    {
        Link<> &rLink = *aIter;
        // check this hasn't been removed in some re-enterancy scenario fdo#47368
        if( std::find(m_aListeners.begin(), m_aListeners.end(), rLink) != m_aListeners.end() )
            rLink.Call( pEvent );
        ++aIter;
    }
}


VclWindowEvent::VclWindowEvent( vcl::Window* pWin, sal_uLong n, void* pDat ) : VclSimpleEvent(n)
{
    pWindow = pWin; pData = pDat;
}

VclWindowEvent::~VclWindowEvent() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
