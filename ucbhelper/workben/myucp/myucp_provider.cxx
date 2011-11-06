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
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "osl/diagnose.h"
#include "osl/mutex.hxx"

#include "ucbhelper/contentidentifier.hxx"

#include "myucp_provider.hxx"
#include "myucp_content.hxx"

using namespace com::sun::star;

// @@@ Adjust namespace name.
namespace myucp {

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider(
                const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr )
{
}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

// @@@ Add own interfaces.
XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

// @@@ Add own interfaces.
XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

// @@@ Adjust implementation name. Keep the prefix "com.sun.star.comp."!
// @@@ Adjust service name.
XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString::createFromAscii(
                            "com.sun.star.comp.myucp.ContentProvider" ),
                     rtl::OUString::createFromAscii(
                             MYUCP_CONTENT_PROVIDER_SERVICE_NAME ) );

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );

//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
uno::Reference< ucb::XContent > SAL_CALL ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    // Check URL scheme...

    rtl::OUString aScheme( rtl::OUString::createFromAscii( MYUCP_URL_SCHEME ) );
    if ( !Identifier->getContentProviderScheme().equalsIgnoreAsciiCase( aScheme ) )
        throw ucb::IllegalIdentifierException();

    // @@@ Further id checks may go here...
#if 0
    if ( id-check-failes )
        throw ucb::IllegalIdentifierException();
#endif

    // @@@ Id normalization may go here...
#if 0
    // Normalize URL and create new Id.
    rtl::OUString aCanonicURL = xxxxx( Identifier->getContentIdentifier() );
    uno::Reference< ucb::XContentIdentifier > xCanonicId
        = new ::ucbhelper::ContentIdentifier( m_xSMgr, aCanonicURL );
#else
    uno::Reference< ucb::XContentIdentifier > xCanonicId = Identifier;
#endif

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    // @@@ Decision, which content implementation to instanciate may be
    //     made here ( in case you have different content classes ).

    // Create a new content.

    xContent = new Content( m_xSMgr, this, xCanonicId );
    registerNewContent( xContent );

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

} // namespace
