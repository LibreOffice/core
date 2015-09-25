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

#ifndef INCLUDED_SERFUNLOCKPROCIMPL_HXX
#define INCLUDED_SERFUNLOCKPROCIMPL_HXX

#include "SerfTypes.hxx"
#include "SerfRequestProcessorImpl.hxx"

#include "SerfInputStream.hxx"

#include <vector>
#include <rtl/ustring.hxx>
#include "SerfTypes.hxx"
#include "DAVTypes.hxx"
#include "DAVResource.hxx"

namespace http_dav_ucp
{

    class SerfUnlockProcImpl : public SerfRequestProcessorImpl
    {

    protected:
        const ucb::Lock     mLock;
        const char*         mpLockToken;
        com::sun::star::uno::Reference< SerfInputStream > xInputStream;

    public:
        SerfUnlockProcImpl( const char* inSourcePath,
                            const DAVRequestHeaders& inRequestHeaders,
                            const ucb::Lock& inLock,
                            const char* inLockToken );

        virtual ~SerfUnlockProcImpl();

        virtual
        serf_bucket_t * createSerfRequestBucket( serf_request_t * inSerfRequest );

    protected:
        virtual
        void processChunkOfResponseData( const char* data, apr_size_t len );

        virtual
        void handleEndOfResponseData( serf_bucket_t * inSerfResponseBucket );

    };

} // namespace http_dav_ucp

#endif // INCLUDED_SERFUNLOCKPROCIMPL_HXX
