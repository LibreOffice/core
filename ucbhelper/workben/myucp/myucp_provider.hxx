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



// @@@ Adjust multi-include-protection-ifdef.
#ifndef _MYUCP_PROVIDER_HXX
#define _MYUCP_PROVIDER_HXX

#include "ucbhelper/providerhelper.hxx"

// @@@ Adjust namespace name.
namespace myucp {

//=========================================================================

// @@@ Adjust defines.

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider. Prefix with reversed company domain name.
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.MyContentProvider"
#define MYUCP_CONTENT_PROVIDER_SERVICE_NAME_LENGTH  34

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define MYUCP_URL_SCHEME \
                "vnd.sun.star.myucp"
#define MYUCP_URL_SCHEME_LENGTH 18

// UCB Content Type.
#define MYUCP_CONTENT_TYPE \
                "application/" MYUCP_URL_SCHEME "-content"

//=========================================================================

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
public:
    ContentProvider( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& rSMgr );
    virtual ~ContentProvider();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Additional interfaces
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////
};

}

#endif
