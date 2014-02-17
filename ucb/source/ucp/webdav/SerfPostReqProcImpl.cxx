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

#include "SerfPostReqProcImpl.hxx"

#include <serf/serf.h>

using namespace com::sun::star;

namespace http_dav_ucp
{

SerfPostReqProcImpl::SerfPostReqProcImpl( const char* inPath,
                                          const DAVRequestHeaders& inRequestHeaders,
                                          const char* inData,
                                          apr_size_t inDataLen,
                                          const char* inContentType,
                                          const char* inReferer,
                                          const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpPostData( inData )
    , mnPostDataLen( inDataLen )
    , mpContentType( inContentType )
    , mpReferer( inReferer )
    , xInputStream( xioInStrm )
    , xOutputStream()
{
}

SerfPostReqProcImpl::SerfPostReqProcImpl( const char* inPath,
                                          const DAVRequestHeaders& inRequestHeaders,
                                          const char* inData,
                                          apr_size_t inDataLen,
                                          const char* inContentType,
                                          const char* inReferer,
                                          const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm )
    : SerfRequestProcessorImpl( inPath, inRequestHeaders )
    , mpPostData( inData )
    , mnPostDataLen( inDataLen )
    , mpContentType( inContentType )
    , mpReferer( inReferer )
    , xInputStream()
    , xOutputStream( xioOutStrm )
{
}

SerfPostReqProcImpl::~SerfPostReqProcImpl()
{
}

serf_bucket_t * SerfPostReqProcImpl::createSerfRequestBucket( serf_request_t * inSerfRequest )
{
    serf_bucket_alloc_t* pSerfBucketAlloc = serf_request_get_alloc( inSerfRequest );

    // create body bucket
    serf_bucket_t* body_bkt = 0;
    if ( mpPostData != 0 && mnPostDataLen > 0 )
    {
        body_bkt = SERF_BUCKET_SIMPLE_STRING_LEN( mpPostData, mnPostDataLen, pSerfBucketAlloc );
    }

    // create serf request
    serf_bucket_t *req_bkt = serf_request_bucket_request_create( inSerfRequest,
                                                                 "POST",
                                                                 getPathStr(),
                                                                 body_bkt,
                                                                 serf_request_get_alloc( inSerfRequest ) );
    handleChunkedEncoding(req_bkt, mnPostDataLen);

    // set request header fields
    serf_bucket_t* hdrs_bkt = serf_bucket_request_get_headers( req_bkt );
    // general header fields provided by caller
    setRequestHeaders( hdrs_bkt );

    // request specific header fields
    if ( mpContentType != 0 )
    {
        serf_bucket_headers_set( hdrs_bkt, "Content-Type", mpContentType );
    }
    if ( mpReferer != 0 )
    {
        serf_bucket_headers_set( hdrs_bkt, "Referer", mpReferer );
    }

    return req_bkt;
}

void SerfPostReqProcImpl::processChunkOfResponseData( const char* data,
                                                      apr_size_t len )
{
    if ( xInputStream.is() )
    {
        xInputStream->AddToStream( data, len );
    }
    else if ( xOutputStream.is() )
    {
        const uno::Sequence< sal_Int8 > aDataSeq( (sal_Int8 *)data, len );
        xOutputStream->writeBytes( aDataSeq );
    }
}

void SerfPostReqProcImpl::handleEndOfResponseData( serf_bucket_t * /*inSerfResponseBucket*/ )
{
    // nothing to do;
}

} // namespace http_dav_ucp
