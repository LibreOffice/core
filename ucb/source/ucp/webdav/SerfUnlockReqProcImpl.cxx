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

#include "SerfUnlockReqProcImpl.hxx"

namespace http_dav_ucp
{

SerfUnlockReqProcImpl::SerfUnlockReqProcImpl( const char* inPath,
                                              const DAVRequestHeaders& inRequestHeaders,
                                              const OUString& sToken)
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , m_sToken( sToken )
{
}

SerfUnlockReqProcImpl::~SerfUnlockReqProcImpl()
{
}

serf_bucket_t * SerfUnlockReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "UNLOCK",
                                                                 getPathStr(),
                                                                 0,
                                                                 serf_request_get_alloc( inSerfRequest ) );
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );

    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    // token header field
    serf_bucket_headers_set( hdrs_bkt, "Lock-Token",
            OUStringToOString(m_sToken, RTL_TEXTENCODING_UTF8).getStr() );

    return req_bkt;
}

void SerfUnlockReqProcImpl::processChunkOfResponseData( const char* , apr_size_t )
{
}

void SerfUnlockReqProcImpl::handleEndOfResponseData( serf_bucket_t * )
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
