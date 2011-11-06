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



#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include "gio_provider.hxx"
#include "gio_content.hxx"

#include <stdio.h>

using namespace com::sun::star;

namespace gio
{
uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw( com::sun::star::ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
#ifdef DEBUG
    fprintf(stderr, "QueryContent: '%s'",
       (const sal_Char *)rtl::OUStringToOString
       (Identifier->getContentIdentifier(), RTL_TEXTENCODING_UTF8));
#endif

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent = queryExistingContent( Identifier ).get();
    if ( xContent.is() )
        return xContent;

    try
    {
        xContent = new ::gio::Content(m_xSMgr, this, Identifier);
    }
    catch ( com::sun::star::ucb::ContentCreationException const & )
    {
        throw com::sun::star::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw com::sun::star::ucb::IllegalIdentifierException();

    return xContent;
}

ContentProvider::ContentProvider(
    const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr )
{
}

ContentProvider::~ContentProvider()
{
}

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   com::sun::star::ucb::XContentProvider );

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      com::sun::star::ucb::XContentProvider );

XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString::createFromAscii(
                       "com.sun.star.comp.GIOContentProvider" ),
                     rtl::OUString::createFromAscii(
                       "com.sun.star.ucb.GIOContentProvider" ) );

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char  **ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" void * SAL_CALL component_getFactory( const sal_Char *pImplName,
    void *pServiceManager, void * )
{
    void * pRet = 0;

    uno::Reference< lang::XMultiServiceFactory > xSMgr
        (reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    g_type_init();

    if ( !::gio::ContentProvider::getImplementationName_Static().compareToAscii( pImplName ) )
        xFactory = ::gio::ContentProvider::createServiceFactory( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}
