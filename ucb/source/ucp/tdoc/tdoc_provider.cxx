/*************************************************************************
 *
 *  $RCSfile: tdoc_provider.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:32:36 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/embed/XStorage.hpp"

#include "ucbhelper/contentidentifier.hxx"

#include "tdoc_provider.hxx"
#include "tdoc_content.hxx"
#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx"

using namespace com::sun;
using namespace com::sun::star;

using namespace tdoc_ucp;

//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

ContentProvider::ContentProvider(
            const uno::Reference< lang::XMultiServiceFactory >& xSMgr )
: ::ucb::ContentProviderImplHelper( xSMgr ),
  m_xDocsMgr( new OfficeDocumentsManager( xSMgr, this ) ),
  m_xStgElemFac( new StorageElementFactory( xSMgr, m_xDocsMgr ) )
{
}

//=========================================================================
// virtual
ContentProvider::~ContentProvider()
{
    if ( m_xDocsMgr.is() )
        m_xDocsMgr->destroy();
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   star::ucb::XContentProvider,
                   frame::XTransientDocumentsDocumentContentFactory );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      star::ucb::XContentProvider,
                      frame::XTransientDocumentsDocumentContentFactory );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1(
    ContentProvider,
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.ucb.TransientDocumentsContentProvider" ) ),
    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
        TDOC_CONTENT_PROVIDER_SERVICE_NAME ) ) );

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
uno::Reference< star::ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< star::ucb::XContentIdentifier >& Identifier )
    throw( star::ucb::IllegalIdentifierException, uno::RuntimeException )
{
    Uri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw star::ucb::IllegalIdentifierException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid URL!" ) ),
            Identifier );

    // Normalize URI.
    uno::Reference< star::ucb::XContentIdentifier > xCanonicId
        = new ::ucb::ContentIdentifier( m_xSMgr, aUri.getUri() );

    vos::OGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< star::ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).getBodyPtr();

    if ( !xContent.is() )
    {
        // Create a new content.
        xContent = Content::create( m_xSMgr, this, xCanonicId );
    }

    return xContent;
}

//=========================================================================
//
// XTransientDocumentsDocumentContentFactory methods.
//
//=========================================================================

// virtual
uno::Reference< star::ucb::XContent > SAL_CALL
ContentProvider::createDocumentContent(
        const uno::Reference< frame::XModel >& Model )
    throw ( lang::IllegalArgumentException, uno::RuntimeException )
{
    // model -> id -> content identifier -> queryContent
    if ( m_xDocsMgr.is() )
    {
        rtl::OUString aDocId = m_xDocsMgr->queryDocumentId( Model );
        if ( aDocId.getLength() > 0 )
        {
            rtl::OUStringBuffer aBuffer;
            aBuffer.appendAscii( TDOC_URL_SCHEME ":/" );
            aBuffer.append( aDocId );

            uno::Reference< star::ucb::XContentIdentifier > xId
                = new ::ucb::ContentIdentifier(
                    m_xSMgr, aBuffer.makeStringAndClear() );

            vos::OGuard aGuard( m_aMutex );

            // Check, if a content with given id already exists...
            uno::Reference< star::ucb::XContent > xContent
                = queryExistingContent( xId ).getBodyPtr();

            if ( !xContent.is() )
            {
                // Create a new content.
                xContent = Content::create( m_xSMgr, this, xId );
            }

            if ( xContent.is() )
                return xContent;

            // no content.
            throw lang::IllegalArgumentException(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "Illegal Content Identifier!" ) ),
                static_cast< cppu::OWeakObject * >( this ),
                1 );
        }
        else
        {
            throw lang::IllegalArgumentException(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "Unable to obtain document id from model!" ) ),
                static_cast< cppu::OWeakObject * >( this ),
                1 );
        }
     }
     else
     {
        throw lang::IllegalArgumentException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "No Document Manager!" ) ),
            static_cast< cppu::OWeakObject * >( this ),
            1 );
     }
}

//=========================================================================
//
// interface OfficeDocumentsEventListener
//
//=========================================================================

// virtual
void ContentProvider::notifyDocumentClosed( const rtl::OUString & rDocId )
{
    vos::OGuard aGuard( getContentListMutex() );

    ::ucb::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    // Notify all content objects related to the closed doc.

    bool bFoundDocumentContent = false;
    rtl::Reference< Content > xRoot;

    while ( it != end )
    {
        Uri aUri( (*it)->getIdentifier()->getContentIdentifier() );
        OSL_ENSURE( aUri.isValid(),
                    "ContentProvider::notifyDocumentClosed - Invalid URI!" );

        if ( !bFoundDocumentContent )
        {
            if ( aUri.isRoot() )
            {
                xRoot = static_cast< Content * >( (*it).getBodyPtr() );
            }
            else if ( aUri.isDocument() )
            {
                if ( aUri.getDocumentId() == rDocId )
                {
                    bFoundDocumentContent = true;

                    // document content will notify removal of child itself;
                    // no need for the root to propagate this.
                    xRoot.clear();
                }
            }
        }

        if ( aUri.getDocumentId() == rDocId )
        {
            // Inform content.
            rtl::Reference< Content > xContent
                = static_cast< Content * >( (*it).getBodyPtr() );

            xContent->notifyDocumentClosed();
        }

        ++it;
    }

    if ( xRoot.is() )
    {
        // No document content found for rDocId but root content
        // instanciated. Root content must announce document removal
        // to content event listeners.
        xRoot->notifyChildRemoved( rDocId );
    }
}

//=========================================================================
// virtual
void ContentProvider::notifyDocumentOpened( const rtl::OUString & rDocId )
{
    vos::OGuard aGuard( getContentListMutex() );

    ::ucb::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucb::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucb::ContentRefList::const_iterator end = aAllContents.end();

    // Find root content. If instanciated let it propagate document insertion.

    while ( it != end )
    {
        Uri aUri( (*it)->getIdentifier()->getContentIdentifier() );
        OSL_ENSURE( aUri.isValid(),
                    "ContentProvider::notifyDocumentOpened - Invalid URI!" );

        if ( aUri.isRoot() )
        {
            rtl::Reference< Content > xRoot
                = static_cast< Content * >( (*it).getBodyPtr() );
            xRoot->notifyChildInserted( rDocId );

            // Done.
            break;
        }

        ++it;
    }
}

//=========================================================================
//
// Non-UNO
//
//=========================================================================

uno::Reference< embed::XStorage >
ContentProvider::queryStorage( const rtl::OUString & rUri,
                               StorageAccessMode eMode ) const
{
    if ( m_xStgElemFac.is() )
    {
        try
        {
            return m_xStgElemFac->createStorage( rUri, eMode );
        }
        catch ( embed::InvalidStorageException const & )
        {
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
        }
    }
    return uno::Reference< embed::XStorage >();
}

//=========================================================================
uno::Reference< embed::XStorage >
ContentProvider::queryStorageClone( const rtl::OUString & rUri ) const
{
    if ( m_xStgElemFac.is() )
    {
        try
        {
            Uri aUri( rUri );
            uno::Reference< embed::XStorage > xParentStorage
                = m_xStgElemFac->createStorage( aUri.getParentUri(), READ );
            uno::Reference< embed::XStorage > xStorage
                = m_xStgElemFac->createTemporaryStorage();

            xParentStorage->copyStorageElementLastCommitTo(
                                aUri.getDecodedName(), xStorage );
            return xStorage;
        }
        catch ( embed::InvalidStorageException const & )
        {
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
        }
    }

    return uno::Reference< embed::XStorage >();
}

//=========================================================================
uno::Reference< io::XInputStream >
ContentProvider::queryInputStream( const rtl::OUString & rUri,
                                   const rtl::OUString & rPassword ) const
    throw ( packages::WrongPasswordException )
{
    if ( m_xStgElemFac.is() )
    {
        try
        {
            return m_xStgElemFac->createInputStream( rUri, rPassword );
        }
        catch ( embed::InvalidStorageException const & )
        {
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
        }
/*
        catch ( packages::WrongPasswordException const & )
        {
            // the key provided is wrong; rethrow; to be handled by caller.
            throw;
        }
*/
    }
    return uno::Reference< io::XInputStream >();
}

//=========================================================================
uno::Reference< io::XOutputStream >
ContentProvider::queryOutputStream( const rtl::OUString & rUri,
                                    const rtl::OUString & rPassword,
                                    bool bTruncate ) const
    throw ( packages::WrongPasswordException )
{
    if ( m_xStgElemFac.is() )
    {
        try
        {
            return
                m_xStgElemFac->createOutputStream( rUri, rPassword, bTruncate );
        }
        catch ( embed::InvalidStorageException const & )
        {
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
        }
/*
        catch ( packages::WrongPasswordException const & )
        {
            // the key provided is wrong; rethrow; to be handled by caller.
            throw;
        }
*/
    }
    return uno::Reference< io::XOutputStream >();
}

//=========================================================================
uno::Reference< io::XStream >
ContentProvider::queryStream( const rtl::OUString & rUri,
                              const rtl::OUString & rPassword,
                              bool bTruncate ) const
    throw ( packages::WrongPasswordException )
{
    if ( m_xStgElemFac.is() )
    {
        try
        {
            return m_xStgElemFac->createStream( rUri, rPassword, bTruncate );
        }
        catch ( embed::InvalidStorageException const & )
        {
            OSL_ENSURE( false, "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_ENSURE( false, "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
        }
/*
        catch ( packages::WrongPasswordException const & )
        {
            // the key provided is wrong; rethrow; to be handled by caller.
            throw;
        }
*/
    }
    return uno::Reference< io::XStream >();
}

//=========================================================================
bool ContentProvider::queryNamesOfChildren(
    const rtl::OUString & rUri, uno::Sequence< rtl::OUString > & rNames ) const
{
    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        // special handling for root, which has no storage, but children.
        if ( m_xDocsMgr.is() )
        {
            rNames = m_xDocsMgr->queryDocuments();
            return true;
        }
    }
    else
    {
        if ( m_xStgElemFac.is() )
        {
            try
            {
                uno::Reference< embed::XStorage > xStorage
                    = m_xStgElemFac->createStorage( rUri, READ );

                OSL_ENSURE( xStorage.is(), "Got no Storage!" );

                if ( xStorage.is() )
                {
                    uno::Reference< container::XNameAccess > xNA(
                        xStorage, uno::UNO_QUERY );

                    OSL_ENSURE( xNA.is(), "Got no css.container.XNameAccess!" );
                    if ( xNA.is() )
                    {
                        rNames = xNA->getElementNames();
#if 1
                        // Special folder that is always kept open by SFX
                        // in read-write mode => never accessible
                        for ( sal_Int32 n = 0; n < rNames.getLength(); ++n )
                        {
                            if ( rNames[ n ].equalsAscii( "Configurations2" ) )
                            {
                                for ( sal_Int32 nn = n;
                                      nn < rNames.getLength() - 1;
                                      ++nn )
                                {
                                    rNames[ nn ] = rNames[ nn + 1 ];
                                }
                                rNames.realloc( rNames.getLength() - 1 );
                                break;
                            }
                        }
#endif
                        return true;
                    }
                }
            }
            catch ( embed::InvalidStorageException const & )
            {
                OSL_ENSURE( false, "Caught InvalidStorageException!" );
            }
            catch ( lang::IllegalArgumentException const & )
            {
                OSL_ENSURE( false, "Caught IllegalArgumentException!" );
            }
            catch ( io::IOException const & )
            {
                // Okay to happen, for instance when the storage does not exist.
                //OSL_ENSURE( false, "Caught IOException!" );
            }
            catch ( embed::StorageWrappedTargetException const & )
            {
                OSL_ENSURE( false, "Caught embed::StorageWrappedTargetException!" );
            }
        }
    }
    return false;
}

//=========================================================================
rtl::OUString
ContentProvider::queryStorageTitle( const rtl::OUString & rUri ) const
{
    rtl::OUString aTitle;

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        // always empty.
        aTitle = rtl::OUString();
    }
    else if ( aUri.isDocument() )
    {
        // for documents, title shall not be derived from URL. It shall
        // be somethimg more 'speaking' than just the document UID.
        if ( m_xDocsMgr.is() )
            aTitle = m_xDocsMgr->queryStorageTitle( aUri.getDocumentId() );
    }
    else
    {
        // derive title from URL
        aTitle = aUri.getDecodedName();
    }

    OSL_ENSURE( ( aTitle.getLength() > 0 ) || aUri.isRoot(),
                "ContentProvider::queryStorageTitle - empty title!" );
    return aTitle;
}

//=========================================================================
uno::Reference< frame::XModel >
ContentProvider::queryDocumentModel( const rtl::OUString & rUri ) const
{
    uno::Reference< frame::XModel > xModel;

    if ( m_xDocsMgr.is() )
    {
        Uri aUri( rUri );
        xModel = m_xDocsMgr->queryDocumentModel( aUri.getDocumentId() );
    }

    OSL_ENSURE( xModel.is(),
                "ContentProvider::queryDocumentModel - no model!" );
    return xModel;
}

