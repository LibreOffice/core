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

#include <rtl/ustring.hxx>

#include <SerfPutReqProcImpl.hxx>

#include <serf.h>

namespace http_dav_ucp
{

SerfPutReqProcImpl::SerfPutReqProcImpl( const char* inPath,
                                        const DAVRequestHeaders& inRequestHeaders,
                                        const char* inData,
                                        apr_size_t inDataLen )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpData( inData )
    , mnDataLen( inDataLen )
{
}

SerfPutReqProcImpl::~SerfPutReqProcImpl()
{
}

serf_bucket_t * SerfPutReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // create body bucket
    serf_bucket_t* body_bkt = 0;
    if ( mpData != 0 && mnDataLen > 0 )
    {
        body_bkt = SERF_BUCKET_SIMPLE_STRING_LEN( mpData, mnDataLen, pSerfBucketAlloc );
        if ( useChunkedEncoding() )
        {
            body_bkt = serf_bucket_chunk_create( body_bkt, pSerfBucketAlloc );
        }
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PUT",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    // request specific header fields
    if ( body_bkt != 0 )
    {
        if ( useChunkedEncoding() )
        {
            serf_bucket_headers_set( hdrs_bkt, "Transfer-Encoding", "chunked");
        }
        serf_bucket_headers_set( hdrs_bkt, "Content-Length",
                                 rtl::OUStringToOString( rtl::OUString::valueOf( (sal_Int32)mnDataLen ), RTL_TEXTENCODING_UTF8 ) );
    }


    return req_bkt;
}

void SerfPutReqProcImpl::processChunkOfResponseData( const char* /*data*/,
                                                     apr_size_t /*len*/ )
{
    // nothing to do;
}

void SerfPutReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    // nothing to do;
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
