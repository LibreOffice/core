/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "rtl/ustrbuf.hxx"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/embed/XStorage.hpp"

#include "comphelper/processfactory.hxx"
#include "ucbhelper/contentidentifier.hxx"

#include "tdoc_provider.hxx"
#include "tdoc_content.hxx"
#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx"

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
: ::ucbhelper::ContentProviderImplHelper( xSMgr ),
  m_xDocsMgr( new OfficeDocumentsManager( comphelper::getComponentContext(xSMgr), this ) ),
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
                   ucb::XContentProvider,
                   frame::XTransientDocumentsDocumentContentFactory );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_4( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider,
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
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    Uri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid URL!" ) ),
            Identifier );

    // Normalize URI.
    uno::Reference< ucb::XContentIdentifier > xCanonicId
        = new ::ucbhelper::ContentIdentifier( m_xSMgr, aUri.getUri() );

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();

    if ( !xContent.is() )
    {
        // Create a new content.
        xContent = Content::create( m_xSMgr, this, xCanonicId );
        registerNewContent( xContent );
    }

    return xContent;
}

//=========================================================================
//
// XTransientDocumentsDocumentContentFactory methods.
//
//=========================================================================

// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::createDocumentContent(
        const uno::Reference< frame::XModel >& Model )
    throw ( lang::IllegalArgumentException, uno::RuntimeException )
{
    // model -> id -> content identifier -> queryContent
    if ( m_xDocsMgr.is() )
    {
        rtl::OUString aDocId = m_xDocsMgr->queryDocumentId( Model );
        if ( !aDocId.isEmpty() )
        {
            rtl::OUStringBuffer aBuffer;
            aBuffer.appendAscii( TDOC_URL_SCHEME ":/" );
            aBuffer.append( aDocId );

            uno::Reference< ucb::XContentIdentifier > xId
                = new ::ucbhelper::ContentIdentifier(
                    m_xSMgr, aBuffer.makeStringAndClear() );

            osl::MutexGuard aGuard( m_aMutex );

            // Check, if a content with given id already exists...
            uno::Reference< ucb::XContent > xContent
                = queryExistingContent( xId ).get();

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
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

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
                xRoot = static_cast< Content * >( (*it).get() );
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
                = static_cast< Content * >( (*it).get() );

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
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    // Find root content. If instanciated let it propagate document insertion.

    while ( it != end )
    {
        Uri aUri( (*it)->getIdentifier()->getContentIdentifier() );
        OSL_ENSURE( aUri.isValid(),
                    "ContentProvider::notifyDocumentOpened - Invalid URI!" );

        if ( aUri.isRoot() )
        {
            rtl::Reference< Content > xRoot
                = static_cast< Content * >( (*it).get() );
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
            OSL_FAIL( "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
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
            OSL_FAIL( "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
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
            OSL_FAIL( "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            OSL_FAIL( "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
        }
//        catch ( packages::WrongPasswordException const & )
//        {
//            // the key provided is wrong; rethrow; to be handled by caller.
//            throw;
//        }
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
            OSL_FAIL( "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
        }
//        catch ( packages::WrongPasswordException const & )
//        {
//            // the key provided is wrong; rethrow; to be handled by caller.
//            throw;
//        }
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
            OSL_FAIL( "Caught InvalidStorageException!" );
        }
        catch ( lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Caught IllegalArgumentException!" );
        }
        catch ( io::IOException const & )
        {
            // Okay to happen, for instance when the storage does not exist.
            //OSL_ENSURE( false, "Caught IOException!" );
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
        }
//        catch ( packages::WrongPasswordException const & )
//        {
//            // the key provided is wrong; rethrow; to be handled by caller.
//            throw;
//        }
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
                        return true;
                    }
                }
            }
            catch ( embed::InvalidStorageException const & )
            {
                OSL_FAIL( "Caught InvalidStorageException!" );
            }
            catch ( lang::IllegalArgumentException const & )
            {
                OSL_FAIL( "Caught IllegalArgumentException!" );
            }
            catch ( io::IOException const & )
            {
                // Okay to happen, for instance if the storage does not exist.
                //OSL_ENSURE( false, "Caught IOException!" );
            }
            catch ( embed::StorageWrappedTargetException const & )
            {
                OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
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

    OSL_ENSURE( !aTitle.isEmpty() || aUri.isRoot(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
