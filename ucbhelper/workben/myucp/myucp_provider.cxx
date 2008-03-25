/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: myucp_provider.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:27:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
