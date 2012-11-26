/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/vclevent.hxx"

#include "svdata.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;

VclSimpleEvent::~VclSimpleEvent() {}
VclWindowEvent::~VclWindowEvent() {}
VclMenuEvent::~VclMenuEvent() {}

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
    if( dynamic_cast< VclWindowEvent* >(pEvent) )
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

