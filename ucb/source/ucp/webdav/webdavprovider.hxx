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



#ifndef _WEBDAV_UCP_PROVIDER_HXX
#define _WEBDAV_UCP_PROVIDER_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/beans/Property.hpp>
#include "DAVSessionFactory.hxx"
#include <ucbhelper/providerhelper.hxx>
#include "PropertyMap.hxx"

namespace webdav_ucp {

//=========================================================================

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.
#define WEBDAV_CONTENT_PROVIDER_SERVICE_NAME \
                "com.sun.star.ucb.WebDAVContentProvider"
#define WEBDAV_CONTENT_PROVIDER_SERVICE_NAME_LENGTH 38

// URL scheme. This is the scheme the provider will be able to create
// contents for. The UCB will select the provider ( i.e. in order to create
// contents ) according to this scheme.
#define WEBDAV_URL_SCHEME \
                "vnd.sun.star.webdav"
#define WEBDAV_URL_SCHEME_LENGTH    19

#define HTTP_URL_SCHEME         "http"
#define HTTP_URL_SCHEME_LENGTH  4

#define HTTPS_URL_SCHEME        "https"
#define HTTPS_URL_SCHEME_LENGTH 5

#define DAV_URL_SCHEME          "dav"
#define DAV_URL_SCHEME_LENGTH   3

#define DAVS_URL_SCHEME     "davs"
#define DAVS_URL_SCHEME_LENGTH  4



#define FTP_URL_SCHEME "ftp"

#define HTTP_CONTENT_TYPE \
                "application/" HTTP_URL_SCHEME "-content"

#define WEBDAV_CONTENT_TYPE    HTTP_CONTENT_TYPE
#define WEBDAV_COLLECTION_TYPE \
                "application/" WEBDAV_URL_SCHEME "-collection"

//=========================================================================

class ContentProvider : public ::ucbhelper::ContentProviderImplHelper
{
    rtl::Reference< DAVSessionFactory > m_xDAVSessionFactory;
    PropertyMap * m_pProps;

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

    rtl::Reference< DAVSessionFactory > getDAVSessionFactory()
    { return m_xDAVSessionFactory; }

    bool getProperty( const ::rtl::OUString & rPropName,
                      ::com::sun::star::beans::Property & rProp,
                      bool bStrict = false );
};

}

#endif
