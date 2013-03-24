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

#ifndef INCLUDED_SERFGETREQPROCIMPL_HXX
#define INCLUDED_SERFGETREQPROCIMPL_HXX

#include "SerfRequestProcessorImpl.hxx"

#include <vector>
#include <rtl/ustring.hxx>
#include "DAVResource.hxx"

#include "SerfInputStream.hxx"
#include <com/sun/star/io/XOutputStream.hpp>

namespace http_dav_ucp
{

class SerfGetReqProcImpl : public SerfRequestProcessorImpl
{
public:
    SerfGetReqProcImpl( const char* inPath,
                        const DAVRequestHeaders& inRequestHeaders,
                        const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm );

    SerfGetReqProcImpl( const char* inPath,
                        const DAVRequestHeaders& inRequestHeaders,
                        const com::sun::star::uno::Reference< SerfInputStream > & xioInStrm,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource );

    SerfGetReqProcImpl( const char* inPath,
                        const DAVRequestHeaders& inRequestHeaders,
                        const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm );

    SerfGetReqProcImpl( const char* inPath,
                        const DAVRequestHeaders& inRequestHeaders,
                        const com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > & xioOutStrm,
                        const std::vector< ::rtl::OUString > & inHeaderNames,
                        DAVResource & ioResource );

    virtual ~SerfGetReqProcImpl();

    virtual
    serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest );

    void processSingleResponseHeader( const char* inHeaderName,
                                      const char* inHeaderValue );

protected:
    virtual
    void processChunkOfResponseData( const char* data, apr_size_t len );

    virtual
    void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket );

private:
    com::sun::star::uno::Reference< SerfInputStream > xInputStream;
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > xOutputStream;
    const std::vector< ::rtl::OUString > * mpHeaderNames;
    DAVResource* mpResource;
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFGETREQPROCIMPL_HXX
