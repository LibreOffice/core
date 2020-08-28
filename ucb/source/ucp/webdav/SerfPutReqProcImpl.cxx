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

#include <rtl/ustring.hxx>

#include "SerfPutReqProcImpl.hxx"

#include <serf.h>

namespace http_dav_ucp
{

SerfPutReqProcImpl::SerfPutReqProcImpl( const char* inPath,
                                        const DAVRequestHeaders& inRequestHeaders,
                                        const char* inData,
                                        apr_size_t inDataLen,
                                        const OUString& sToken )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpData( inData )
    , mnDataLen( inDataLen )
    , msToken( sToken )
{
}

SerfPutReqProcImpl::~SerfPutReqProcImpl()
{
}

serf_bucket_t * SerfPutReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // create body bucket
    serf_bucket_t* body_bkt = nullptr;
    if ( mpData != nullptr && mnDataLen > 0 )
    {
        body_bkt = SERF_BUCKET_SIMPLE_STRING_LEN( mpData, mnDataLen, pSerfBucketAlloc );
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PUT",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );
    handleChunkedEncoding(req_bkt, mnDataLen);

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    // 'If' header with token, so that we can save document locked by us
    const OString sIfHeader( OString::Concat("<") + getPathStr() + "> (<" + OUStringToOString(
                msToken, RTL_TEXTENCODING_UTF8) + ">)" );
    serf_bucket_headers_set( hdrs_bkt, "If", sIfHeader.getStr() );

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
