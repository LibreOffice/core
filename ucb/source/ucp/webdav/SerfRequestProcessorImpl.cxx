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

#include <SerfRequestProcessorImpl.hxx>

namespace http_dav_ucp
{

SerfRequestProcessorImpl::SerfRequestProcessorImpl( const char* inPath )
    : mPathStr( inPath )
    , mbUseChunkedEncoding( false )
{
}

SerfRequestProcessorImpl::~SerfRequestProcessorImpl()
{
}

const char* SerfRequestProcessorImpl::getPathStr() const
{
    return mPathStr;
}

void SerfRequestProcessorImpl::activateChunkedEncoding()
{
    mbUseChunkedEncoding = true;
}

const bool SerfRequestProcessorImpl::useChunkedEncoding() const
{
    return mbUseChunkedEncoding;
}

bool SerfRequestProcessorImpl::processSerfResponseBucket( serf_request_t * /*inSerfRequest*/,
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

        if ( len > 0 )
        {
            processChunkOfResponseData( data, len );
        }

        /* are we done yet? */
        if (APR_STATUS_IS_EOF(outStatus))
        {
            handleEndOfResponseData( inSerfResponseBucket );

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

