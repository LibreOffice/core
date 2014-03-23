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

#ifndef INCLUDED_SERFREQUESTPROCESSORIMPL_HXX
#define INCLUDED_SERFREQUESTPROCESSORIMPL_HXX

#include <serf/serf.h>

#include <sal/types.h>
#include "DAVRequestEnvironment.hxx"

namespace http_dav_ucp
{

class SerfRequestProcessorImpl
{
public:
    SerfRequestProcessorImpl( const char* inPath,
                              const DAVRequestHeaders& inRequestHeaders );

    virtual ~SerfRequestProcessorImpl();

    /*pure*/ virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest ) = 0;

    bool processSerfResponseBucket( serf_request_t * inSerfRequest,

                                    serf_bucket_t * inSerfResponseBucket,
                                    apr_pool_t * inAprPool,
                                    apr_status_t & outStatus );

    void activateChunkedEncoding();

    /** Turn chunked encoding on or off, dependeing on the result of
        useChunkedEncoding().
    */
    void handleChunkedEncoding (
        serf_bucket_t* pRequestBucket,
        apr_int64_t nLength) const;

protected:
    void setRequestHeaders( serf_bucket_t* inoutSerfHeaderBucket );

    /*pure*/ virtual
    void processChunkOfResponseData( const char* data, apr_size_t len ) = 0;

    /*pure*/ virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket ) = 0;

    const char* getPathStr() const;
    bool useChunkedEncoding() const;

private:
    const char* mPathStr;
    const DAVRequestHeaders& mrRequestHeaders;
    bool mbUseChunkedEncoding;
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFREQUESTPROCESSORIMPL_HXX
