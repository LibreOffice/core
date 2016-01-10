/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "osl/time.h"

#include "DAVTypes.hxx"
#include "../inc/urihelper.hxx"
#include "NeonUri.hxx"

using namespace webdav_ucp;
using namespace com::sun::star;

// DAVOptions implementation

DAVOptions::DAVOptions() :
    m_isResourceFound( false ),
    m_isClass1( false ),
    m_isClass2( false ),
    m_isClass3( false ),
    m_aAllowedMethods(),
    m_nStaleTime( 0 ),
    m_sURL(),
    m_sRedirectedURL()
{
}


DAVOptions::DAVOptions( const DAVOptions & rOther ) :
    m_isResourceFound( rOther.m_isResourceFound ),
    m_isClass1( rOther.m_isClass1 ),
    m_isClass2( rOther.m_isClass2 ),
    m_isClass3( rOther.m_isClass3 ),
    m_aAllowedMethods( rOther.m_aAllowedMethods ),
    m_nStaleTime( rOther.m_nStaleTime ),
    m_sURL( rOther.m_sURL ),
    m_sRedirectedURL( rOther.m_sRedirectedURL)
{
}


DAVOptions::~DAVOptions()
{
}


bool DAVOptions::operator==( const DAVOptions& rOpts ) const
{
    return
        m_isResourceFound == rOpts.m_isResourceFound &&
        m_isClass1 == rOpts.m_isClass1 &&
        m_isClass2 == rOpts.m_isClass2 &&
        m_isClass3 == rOpts.m_isClass3 &&
        m_aAllowedMethods == rOpts.m_aAllowedMethods &&
        m_nStaleTime == rOpts.m_nStaleTime &&
        m_sURL == rOpts.m_sURL &&
        m_sRedirectedURL == rOpts.m_sRedirectedURL;
}


// DAVOptionsCache implementation

DAVOptionsCache::DAVOptionsCache()
{
}


DAVOptionsCache::~DAVOptionsCache()
{
}


bool DAVOptionsCache::getDAVOptions( const OUString & rURL, DAVOptions & rDAVOptions )
{
    osl::MutexGuard aGuard( m_aMutex );
    OUString aEncodedUrl( ucb_impl::urihelper::encodeURI( NeonUri::unescape( rURL ) ) );
    normalizeURLLastChar( aEncodedUrl );

    // search the URL in the static map
    DAVOptionsMap::iterator it;
    it = m_aTheCache.find( aEncodedUrl );
    if ( it == m_aTheCache.end() )
        return false;
    else
    {
        // check if the capabilities are stale, before restoring
        TimeValue t1;
        osl_getSystemTime( &t1 );
        if ( (*it).second.getStaleTime() < t1.Seconds )
        {
            // if stale, remove from cache, do not restore
            removeDAVOptions( rURL );
            return false;
            // return false instead
        }
        rDAVOptions = (*it).second;
        return true;
    }
}


void DAVOptionsCache::removeDAVOptions( const OUString & rURL )
{
    osl::MutexGuard aGuard( m_aMutex );
    OUString aEncodedUrl( ucb_impl::urihelper::encodeURI( NeonUri::unescape( rURL ) ) );
    normalizeURLLastChar( aEncodedUrl );

    DAVOptionsMap::iterator it;
    it = m_aTheCache.find( aEncodedUrl );
    if ( it != m_aTheCache.end() )
    {
        m_aTheCache.erase( it );
    }
}


void DAVOptionsCache::addDAVOptions( DAVOptions & rDAVOptions, const sal_uInt32 nLifeTime )
{
    osl::MutexGuard aGuard( m_aMutex );

    OUString aURL( rDAVOptions.getURL() );

    OUString aEncodedUrl( ucb_impl::urihelper::encodeURI( NeonUri::unescape( aURL ) ) );
    normalizeURLLastChar( aEncodedUrl );
    rDAVOptions.setURL( aEncodedUrl );

// unchanged, it may be used to access a server
    OUString aRedirURL( rDAVOptions.getRedirectedURL() );
    rDAVOptions.setRedirectedURL( aRedirURL );

    TimeValue t1;
    osl_getSystemTime( &t1 );
    rDAVOptions.setStaleTime( t1.Seconds + nLifeTime );

    m_aTheCache[ aEncodedUrl ] = rDAVOptions;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
