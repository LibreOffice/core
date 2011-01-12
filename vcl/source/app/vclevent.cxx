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
#include "precompiled_vcl.hxx"

#include "vcl/vclevent.hxx"
#include "vcl/svdata.hxx"

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

Reference<XAccessible> VclAccessibleEvent::GetAccessible() const
{
    return mxAccessible;
}

void VclEventListeners::Call( VclSimpleEvent* pEvent ) const
{
    // Copy the list, because this can be destroyed when calling a Link...
    std::list<Link> aCopy( *this );
    std::list<Link>::iterator aIter( aCopy.begin() );
    if( pEvent->IsA( VclWindowEvent::StaticType() ) )
    {
        VclWindowEvent* pWinEvent = static_cast<VclWindowEvent*>(pEvent);
        ImplDelData aDel( pWinEvent->GetWindow() );
        while ( aIter != aCopy.end() && ! aDel.IsDead() )
        {
            (*aIter).Call( pEvent );
            aIter++;
        }
    }
    else
    {
        while ( aIter != aCopy.end() )
        {
            (*aIter).Call( pEvent );
            aIter++;
        }
    }
}

sal_Bool VclEventListeners::Process( VclSimpleEvent* pEvent ) const
{
    sal_Bool bProcessed = sal_False;
    // Copy the list, because this can be destroyed when calling a Link...
    std::list<Link> aCopy( *this );
    std::list<Link>::iterator aIter( aCopy.begin() );
    while ( aIter != aCopy.end() )
    {
        if( (*aIter).Call( pEvent ) != 0 )
        {
            bProcessed = sal_True;
            break;
        }
        aIter++;
    }
    return bProcessed;
}

VclEventListeners2::VclEventListeners2()
{
}

VclEventListeners2::~VclEventListeners2()
{
}

void VclEventListeners2::addListener( const Link& i_rLink )
{
    // ensure uniqueness
    for( std::list< Link >::const_iterator it = m_aListeners.begin(); it != m_aListeners.end(); ++it )
    {
        if( *it == i_rLink )
            return;
    }
    m_aListeners.push_back( i_rLink );
}

void VclEventListeners2::removeListener( const Link& i_rLink )
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

