/*************************************************************************
 *
 *  $RCSfile: webdavprovider.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sb $ $Date: 2001-08-08 10:04:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef HTTP_REQUEST_H // for HTTP_SESSION_FTP
#include "http_request.h"
#endif

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
: ::ucb::ContentProviderImplHelper( rSMgr ),
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
                   com::sun::star::ucb::XContentProvider );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      com::sun::star::ucb::XContentProvider );

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
uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
            const uno::Reference<
                    com::sun::star::ucb::XContentIdentifier >& Identifier )
    throw( com::sun::star::ucb::IllegalIdentifierException,
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
#if defined HTTP_SESSION_FTP
         &&
         !aScheme.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( FTP_URL_SCHEME ) )
#endif // HTTP_SESSION_FTP
       )
        throw com::sun::star::ucb::IllegalIdentifierException();

    // Normalize URL and create new Id, if nessacary.
    rtl::OUString aURL = Identifier->getContentIdentifier();

    // At least: <scheme> + "://"
    if ( aURL.getLength() < ( aScheme.getLength() + 3 ) )
        throw com::sun::star::ucb::IllegalIdentifierException();

    if ( ( aURL.getStr()[ aScheme.getLength() ]     != sal_Unicode( ':' ) ) ||
         ( aURL.getStr()[ aScheme.getLength() + 1 ] != sal_Unicode( '/' ) ) ||
         ( aURL.getStr()[ aScheme.getLength() + 2 ] != sal_Unicode( '/' ) ) )
        throw com::sun::star::ucb::IllegalIdentifierException();

    uno::Reference< com::sun::star::ucb::XContentIdentifier > xCanonicId;

    sal_Int32 nPos = aURL.lastIndexOf( '/' );
    if ( nPos != aURL.getLength() - 1 )
    {
        // Find second slash in URL.
        nPos = aURL.indexOf( '/', aURL.indexOf( '/' ) + 1 );
        if ( nPos == -1 )
            throw com::sun::star::ucb::IllegalIdentifierException();

        nPos = aURL.indexOf( '/', nPos + 1 );
        if ( nPos == -1 )
        {
            aURL += rtl::OUString::createFromAscii( "/" );
            xCanonicId = new ::ucb::ContentIdentifier( m_xSMgr, aURL );
        }
        else
            xCanonicId = Identifier;
    }
    else
        xCanonicId = Identifier;

    vos::OGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< com::sun::star::ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).getBodyPtr();
    if ( xContent.is() )
        return xContent;

    // Create a new content. Note that the content will insert itself
    // into providers content list by calling addContent(...) from it's ctor.

    try
    {
        xContent = new ::webdav_ucp::Content(
                          m_xSMgr, this, xCanonicId, &m_aDAVSessionFactory );
    }
    catch ( com::sun::star::ucb::ContentCreationException const & )
    {
        throw com::sun::star::ucb::IllegalIdentifierException();
    }

    if ( !xContent->getIdentifier().is() )
        throw com::sun::star::ucb::IllegalIdentifierException();

    return xContent;
}

