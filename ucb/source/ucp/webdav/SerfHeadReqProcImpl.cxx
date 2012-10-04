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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <SerfHeadReqProcImpl.hxx>

using namespace com::sun::star;

namespace http_dav_ucp
{

SerfHeadReqProcImpl::SerfHeadReqProcImpl( const char* inPath,
                                          const DAVRequestHeaders& inRequestHeaders,
                                          const std::vector< ::rtl::OUString > & inHeaderNames,
                                          DAVResource & ioResource )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpHeaderNames( &inHeaderNames )
    , mpResource( &ioResource )
{
}

SerfHeadReqProcImpl::~SerfHeadReqProcImpl()
{
}

serf_bucket_t * SerfHeadReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "HEAD",
                                                                 getPathStr(),
                                                                 0,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    return req_bkt;
}

void SerfHeadReqProcImpl::processChunkOfResponseData( const char* /*data*/,
                                                      apr_size_t /*len*/ )
{
    // nothing to do
}

namespace
{
    apr_status_t Serf_ProcessResponseHeader( void* inUserData,
                                             const char* inHeaderName,
                                             const char* inHeaderValue )
    {
        SerfHeadReqProcImpl* pReqProcImpl = static_cast< SerfHeadReqProcImpl* >( inUserData );
        pReqProcImpl->processSingleResponseHeader( inHeaderName,
                                                   inHeaderValue );

        return APR_SUCCESS;
    }
} // end of anonymous namespace

void SerfHeadReqProcImpl::handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket )
{
    // read response header, if requested
    if ( mpHeaderNames != 0 && mpResource != 0 )
    {
        serf_bucket_t* SerfHeaderBucket = serf_bucket_response_get_headers( inSerfResponseBucket );
        if ( SerfHeaderBucket != 0 )
        {
            serf_bucket_headers_do( SerfHeaderBucket,
                                    Serf_ProcessResponseHeader,
                                    this );
        }
    }
}

void SerfHeadReqProcImpl::processSingleResponseHeader( const char* inHeaderName,
                                                       const char* inHeaderValue )
{
    rtl::OUString aHeaderName( rtl::OUString::createFromAscii( inHeaderName ) );

    bool bStoreHeaderField = false;

    if ( mpHeaderNames->size() == 0 )
    {
        // store all header fields
        bStoreHeaderField = true;
    }
    else
    {
        // store only header fields which are requested
        std::vector< ::rtl::OUString >::const_iterator it( mpHeaderNames->begin() );
        const std::vector< ::rtl::OUString >::const_iterator end( mpHeaderNames->end() );

        while ( it != end )
        {
            // header names are case insensitive
            if ( (*it).equalsIgnoreAsciiCase( aHeaderName ) )
            {
                bStoreHeaderField = true;
                break;
            }
            else
            {
                ++it;
            }
        }
    }

    if ( bStoreHeaderField )
    {
        DAVPropertyValue thePropertyValue;
        thePropertyValue.IsCaseSensitive = false;
        thePropertyValue.Name = aHeaderName;
        thePropertyValue.Value <<= rtl::OUString::createFromAscii( inHeaderValue );
        mpResource->properties.push_back( thePropertyValue );
    }
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
