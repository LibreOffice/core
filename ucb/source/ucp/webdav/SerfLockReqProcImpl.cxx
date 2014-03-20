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

#include "SerfLockReqProcImpl.hxx"
#include "DAVException.hxx"

//#include "webdavresponseparser.hxx"
#include <rtl/strbuf.hxx>

namespace http_dav_ucp
{

SerfLockReqProcImpl::SerfLockReqProcImpl( const char* inPath,
                                          const DAVRequestHeaders& inRequestHeaders,
                                          const css::ucb::Lock & rLock )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , m_aLock( rLock )
    , m_xInputStream( new SerfInputStream() )
{
}

SerfLockReqProcImpl::~SerfLockReqProcImpl()
{
}

serf_bucket_t * SerfLockReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    OStringBuffer aBody("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                        "<lockinfo xmlns='DAV:'>\n <lockscope>");

    // Set the lock scope
    switch ( m_aLock.Scope )
    {
        case css::ucb::LockScope_EXCLUSIVE:
            aBody.append("<exclusive/>");
            break;
        case css::ucb::LockScope_SHARED:
            aBody.append("<shared/>");
            break;
        default:
            throw DAVException( DAVException::DAV_INVALID_ARG );
    }
    aBody.append("</lockscope>\n <locktype><write/></locktype>\n");

    // Set the lock owner
    OUString aValue;
    if ((m_aLock.Owner >>= aValue) && !aValue.isEmpty())
    {
        aBody.append(" <owner>");
        aBody.append(OUStringToOString(aValue, RTL_TEXTENCODING_UTF8));
        aBody.append("</owner>\n");
    }
    aBody.append("</lockinfo>\n");

    const OString aBodyText(aBody.makeStringAndClear());
    serf_bucket_t* body_bkt = serf_bucket_simple_copy_create( aBodyText.getStr(),
                                                              aBodyText.getLength(),
                                                              pSerfBucketAlloc );

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "LOCK",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 pSerfBucketAlloc );
    handleChunkedEncoding(req_bkt, aBodyText.getLength());

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );

    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    // request specific header fields
    const char * depth = 0;
    switch( m_aLock.Depth )
    {
        case css::ucb::LockDepth_ZERO:
            depth = "0";
            break;
        case css::ucb::LockDepth_ONE:
            depth = "1";
            break;
        case css::ucb::LockDepth_INFINITY:
            depth = "infinity";
            break;
        default:
            throw DAVException( DAVException::DAV_INVALID_ARG );
    }
    serf_bucket_headers_set( hdrs_bkt, "Depth", depth );
    serf_bucket_headers_set( hdrs_bkt, "Content-Type", "application/xml" );

    // Set the lock timeout
    if (m_aLock.Timeout == -1)
        serf_bucket_headers_set( hdrs_bkt, "Timeout", "Infinite" );
    else if (m_aLock.Timeout > 0)
    {
        const OString aTimeValue("Second-" + OString::number(m_aLock.Timeout));
        serf_bucket_headers_set( hdrs_bkt, "Timeout", aTimeValue.getStr() );
    }

    osl_getSystemTime( &m_aStartCall );

    return req_bkt;
}

void SerfLockReqProcImpl::processChunkOfResponseData( const char* data,
                                                          apr_size_t len )
{
    if ( m_xInputStream.is() )
    {
        m_xInputStream->AddToStream( data, len );
    }
}

void SerfLockReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    //const std::vector< DAVResourceInfo > rResInfo( parseWebDAVPropNameResponse( m_xInputStream.get() ) );

    /*
    if ( theRetVal == NE_OK )
    {
        m_aSerfLockStore.addLock( theLock,
                                  this,
                                  lastChanceToSendRefreshRequest(
                                      startCall, theLock->timeout ) );

        uno::Sequence< OUString > aTokens( 1 );
        aTokens[ 0 ] = OUString::createFromAscii( theLock->token );
        m_xLock.LockTokens = aTokens;

        SAL_INFO("ucb.ucp.webdav",  "SerfSession::LOCK: created lock for "
                    << makeAbsoluteURL( inPath ) << ". token: " << theLock->token );
    }
    else
    {
        ne_lock_destroy( theLock );

        SAL_INFO("ucb.ucp.webdav",  "SerfSession::LOCK: obtaining lock for "
                    << makeAbsoluteURL( inPath ) << " failed!");
    }
    */
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
