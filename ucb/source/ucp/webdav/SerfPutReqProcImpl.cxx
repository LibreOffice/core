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

#include <rtl/ustring.hxx>

#include <SerfPutReqProcImpl.hxx>

#include <serf.h>

namespace http_dav_ucp
{

SerfPutReqProcImpl::SerfPutReqProcImpl( const char* inPath,
                                        const char* inData,
                                        apr_size_t inDataLen )
    : SerfRequestProcessorImpl( inPath )
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
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "PUT",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );

    // TODO - correct headers
    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    serf_bucket_headers_setn( hdrs_bkt, "User-Agent", "www.openoffice.org/ucb/" );
    serf_bucket_headers_setn( hdrs_bkt, "Accept-Encoding", "gzip");

    // request specific header fields
    if ( body_bkt != 0 )
    {
        serf_bucket_headers_setn( hdrs_bkt, "Content-Length",
                                  rtl::OUStringToOString( rtl::OUString::valueOf( (sal_Int32)mnDataLen ), RTL_TEXTENCODING_UTF8 ) );
    }


    return req_bkt;
}


bool SerfPutReqProcImpl::processSerfResponseBucket( serf_request_t * /*inSerfRequest*/,
                                                    serf_bucket_t * inSerfResponseBucket,
                                                    apr_pool_t * /*inAprPool*/,
                                                    apr_status_t & outStatus )
{
    const char* data;
    apr_size_t len;

    while (1) {
        outStatus = serf_bucket_read(inSerfResponseBucket, 8096, &data, &len);
        if (SERF_BUCKET_READ_ERROR(outStatus))
        {
            return true;
        }

        /* are we done yet? */
        if (APR_STATUS_IS_EOF(outStatus))
        {
            outStatus = APR_EOF;
            return true;
        }

        /* have we drained the response so far? */
        if ( APR_STATUS_IS_EAGAIN( outStatus ) )
        {
            return false;
        }
    }

    /* NOTREACHED */
    return true;
}

} // namespace http_dav_ucp
