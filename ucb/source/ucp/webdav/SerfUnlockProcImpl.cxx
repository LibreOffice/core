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
#include "precompiled_ucb.hxx"

#include "SerfTypes.hxx"
#include "SerfUnlockProcImpl.hxx"
#include "DAVProperties.hxx"

#include "webdavresponseparser.hxx"
#include <serf.h>
#include <rtl/ustrbuf.hxx>
#include <apr_strings.h>

namespace http_dav_ucp
{

SerfUnlockProcImpl::SerfUnlockProcImpl( const char* inSourcePath,
                                        const DAVRequestHeaders& inRequestHeaders,
                                        const ucb::Lock& inLock,
                                        const char* inLockToken )
    : SerfRequestProcessorImpl( inSourcePath, inRequestHeaders )
    , mLock( inLock )
    , mpLockToken( inLockToken )
    , xInputStream( new SerfInputStream() )
{

}

SerfUnlockProcImpl::~SerfUnlockProcImpl()
{
}

serf_bucket_t * SerfUnlockProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "UNLOCK",
                                                                 getPathStr(),
                                                                 0,
                                                                 serf_request_get_alloc( inSerfRequest ) );
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    if (hdrs_bkt != NULL)
    {
        // general header fields provided by caller
        setRequestHeaders( hdrs_bkt );

        // request specific header fields
        serf_bucket_headers_set( hdrs_bkt, "Lock-Token", mpLockToken );
    }
    else
    {
        OSL_ASSERT("Headers Bucket missing");
    }

    return req_bkt;
}

void SerfUnlockProcImpl::processChunkOfResponseData( const char* data,
                                                      apr_size_t len )
{
    if ( xInputStream.is() )
    {
        xInputStream->AddToStream( data, len );
    }
}

void SerfUnlockProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
}

} // namespace http_dav_ucp
