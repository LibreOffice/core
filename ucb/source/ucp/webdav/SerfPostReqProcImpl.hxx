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

#ifndef INCLUDED_SERFPOSTREQPROCIMPL_HXX
#define INCLUDED_SERFPOSTREQPROCIMPL_HXX

#include "SerfRequestProcessorImpl.hxx"

#include "SerfInputStream.hxx"
#include <com/sun/star/io/XOutputStream.hpp>

namespace http_dav_ucp
{

class SerfPostReqProcImpl : public SerfRequestProcessorImpl
{
public:
    SerfPostReqProcImpl( const char* inPath,
                         const DAVRequestHeaders& inRequestHeaders,
                         const char* inData,
                         apr_size_t inDataLen,
                         const char* inLockToken,
                         const char* inContentType,
                         const char* inReferer,
                         const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm );

    SerfPostReqProcImpl( const char* inPath,
                         const DAVRequestHeaders& inRequestHeaders,
                         const char* inData,
                         apr_size_t inDataLen,
                         const char* inLockToken,
                         const char* inContentType,
                         const char* inReferer,
                         const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm );

    virtual ~SerfPostReqProcImpl();

    virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest );

protected:
    virtual
    void processChunkOfResponseData( const char* data, apr_size_t len );

    virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket );

private:
    const char* mpPostData;
    apr_size_t mnPostDataLen;
    const char* mpLockToken;
    const char* mpContentType;
    const char* mpReferer;
    com::sun::star::uno::Reference< SerfInputStream > xInputStream;
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xOutputStream;

};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFPOSTREQPROCIMPL_HXX
