/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: webdavprovider.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 14:58:43 $
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
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif

#ifndef _WEBDAV_UCP_PROVIDER_HXX
#include "webdavprovider.hxx"
#endif
#ifndef _WEBDAV_UCP_CONTENT_HXX
#include "webdavcontent.hxx"
#endif

#include "osl/mutex.hxx"

using namespace com::sun::star;
using namespace webdav_ucp;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider(
                const uno::Reference< lang::XMultiServiceFactory >& rSMgr )
: ::ucbhelper::ContentProviderImplHelper( rSMgr ),
  m_xDAVSessionFactory( new DAVSessionFactory() ),
  m_pProps( 0 )
{
}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    delete m_pProps;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1( ContentProvider,
                     rtl::OUString::createFromAscii(
                         "com.sun.star.comp.WebDAVContentProvider" ),
                     rtl::OUString::createFromAscii(
                         WEBDAV_CONTENT_PROVIDER_SERVICE_NAME ) );

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
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException,
           uno::RuntimeException )
{
    // Check URL scheme...

    const rtl::OUString aScheme
        = Identifier->getContentProviderScheme().toAsciiLowerCase();
    if ( !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( HTTP_URL_SCHEME ) ) &&
         !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( HTTPS_URL_SCHEME ) ) &&
         !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) )
         &&
         !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( FTP_URL_SCHEME ) )
       )
        throw ucb::IllegalIdentifierException();

    // Normalize URL and create new Id, if nessacary.
    rtl::OUString aURL = Identifier->getContentIdentifier();

    // At least: <scheme> + "://"
    if ( aURL.getLength() < ( aScheme.getLength() + 3 ) )
        throw ucb::IllegalIdentifierException();

    if ( ( aURL.getStr()[ aScheme.getLength() ]     != sal_Unicode( ':' ) ) ||
         ( aURL.getStr()[ aScheme.getLength() + 1 ] != sal_Unicode( '/' ) ) ||
         ( aURL.getStr()[ aScheme.getLength() + 2 ] != sal_Unicode( '/' ) ) )
        throw ucb::IllegalIdentifierException();

    uno::Reference< ucb::XContentIdentifier > xCanonicId;

    bool bNewId = false;
    if ( aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( WEBDAV_URL_SCHEME ) ) )
    {
        aURL = aURL.replaceAt( 0,
                               WEBDAV_URL_SCHEME_LENGTH,
                               rtl::OUString::createFromAscii(
                                                    HTTP_URL_SCHEME ) );
        bNewId = true;
    }

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos != aURL.getLength() - 1 )
    {
        // Find second slash in URL.
        nPos = aURL.indexOf( '/', aURL.indexOf( '/' ) + 1 );
        if ( nPos == -1 )
            throw ucb::IllegalIdentifierException();

        nPos = aURL.indexOf( '/', nPos + 1 );
        if ( nPos == -1 )
        {
            aURL += rtl::OUString::createFromAscii( "/" );
            bNewId = true;
        }
    }

    if ( bNewId )
        xCanonicId = new ::ucbhelper::ContentIdentifier( m_xSMgr, aURL );
    else
        xCanonicId = Identifier;

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();
    if ( xContent.is() )
        return xContent;

    // Create a new content.

    try
    {
        xContent = new ::webdav_ucp::Content(
                        m_xSMgr, this, xCanonicId, m_xDAVSessionFactory );
        registerNewContent( xContent );
    }
    catch ( ucb::ContentCreationException const & )
    {
        throw ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw ucb::IllegalIdentifierException();

    return xContent;
}

