/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/time.h>
#include "PropfindCache.hxx"

namespace webdav_ucp
{

    // PropertyNames implementation

    PropertyNames::PropertyNames() :
        m_nStaleTime( 0 ),
        m_sURL(),
        m_aPropertiesNames()
    {
    }

    PropertyNames::PropertyNames( const OUString& rURL ) :
        m_nStaleTime( 0 ),
        m_sURL( rURL ),
        m_aPropertiesNames()
    {
    }

    //PropertyNamesCache implementation

    PropertyNamesCache::PropertyNamesCache()
    {
    }

    PropertyNamesCache::~PropertyNamesCache()
    {
    }

    bool PropertyNamesCache::getCachedPropertyNames( const OUString& rURL, PropertyNames& rCacheElement )
    {
        // search the URL in the static map
        osl::MutexGuard aGuard( m_aMutex );
        PropNameCache::const_iterator it;
        it = m_aTheCache.find( rURL );
        if ( it == m_aTheCache.end() )
            return false;
        else
        {
            // check if the element is stale, before restoring
            TimeValue t1;
            osl_getSystemTime( &t1 );
            if ( (*it).second.getStaleTime() < t1.Seconds )
            {
                // if stale, remove from cache, do not restore
                m_aTheCache.erase( it );
                return false;
                // return false instead
            }
            rCacheElement = (*it).second;
            return true;
        }
    }

    void PropertyNamesCache::removeCachedPropertyNames( const OUString& rURL )
    {
        osl::MutexGuard aGuard( m_aMutex );
        PropNameCache::const_iterator it;
        it = m_aTheCache.find( rURL );
        if ( it != m_aTheCache.end() )
        {
            m_aTheCache.erase( it );
        }
    }

    void PropertyNamesCache::addCachePropertyNames( PropertyNames& rCacheElement )
    {
        osl::MutexGuard aGuard( m_aMutex );
        OUString aURL( rCacheElement.getURL() );
        TimeValue t1;
        osl_getSystemTime( &t1 );
        rCacheElement.setStaleTime( t1.Seconds + 10 );

        m_aTheCache[ aURL ] = rCacheElement;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
