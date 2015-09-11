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
    std::list<Link<>> aCopy( m_aListeners );
    std::list<Link<>>::iterator aIter( aCopy.begin() );
    std::list<Link<>>::const_iterator aEnd( aCopy.end() );
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
    m_aListeners.remove( rListener );
}

VclEventListeners2::VclEventListeners2()
{
}

VclEventListeners2::~VclEventListeners2()
{
}

void VclEventListeners2::addListener( const Link<>& i_rLink )
{
    // ensure uniqueness
    for( std::list< Link<> >::const_iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
    {
        if( *it == i_rLink )
            return;
    }
    m_aListeners.push_back( i_rLink );
}

void VclEventListeners2::removeListener( const Link<>& i_rLink )
{
    size_t n = m_aIterators.size();
    for( size_t i = 0; i < n; i++ )
    {
        if( m_aIterators[i].m_aIt != m_aListeners.end() && *m_aIterators[i].m_aIt == i_rLink )
        {
            m_aIterators[i].m_bWasInvalidated = true;
            ++m_aIterators[i].m_aIt;
        }
    }
    m_aListeners.remove( i_rLink );
}

void VclEventListeners2::callListeners( VclSimpleEvent* i_pEvent )
{
    vcl::DeletionListener aDel( this );

    m_aIterators.push_back(ListenerIt(m_aListeners.begin()));
    size_t nIndex = m_aIterators.size() - 1;
    while( ! aDel.isDeleted() && m_aIterators[ nIndex ].m_aIt != m_aListeners.end() )
    {
        m_aIterators[ nIndex ].m_aIt->Call( i_pEvent );
        if( m_aIterators[ nIndex ].m_bWasInvalidated )
            // check if the current element was removed and the iterator increased in the meantime
            m_aIterators[ nIndex ].m_bWasInvalidated = false;
        else
            ++m_aIterators[ nIndex ].m_aIt;
    }
    m_aIterators.pop_back();
}


VclWindowEvent::VclWindowEvent( vcl::Window* pWin, sal_uLong n, void* pDat ) : VclSimpleEvent(n)
{
    pWindow = pWin; pData = pDat;
}

VclWindowEvent::~VclWindowEvent() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
