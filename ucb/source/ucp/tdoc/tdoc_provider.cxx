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

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/embed/InvalidStorageException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <ucbhelper/contentidentifier.hxx>
#include <ucbhelper/getcomponentcontext.hxx>
#include <ucbhelper/macros.hxx>

#include "tdoc_provider.hxx"
#include "tdoc_content.hxx"
#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_storage.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;


// ContentProvider Implementation.


ContentProvider::ContentProvider(
            const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext ),
  m_xDocsMgr( new OfficeDocumentsManager( rxContext, this ) ),
  m_xStgElemFac( new StorageElementFactory( rxContext, m_xDocsMgr ) )
{
}


// virtual
ContentProvider::~ContentProvider()
{
    if ( m_xDocsMgr.is() )
        m_xDocsMgr->destroy();
}


// XInterface methods.
void SAL_CALL ContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< lang::XTypeProvider* >(this),
                                               static_cast< lang::XServiceInfo* >(this),
                                               static_cast< ucb::XContentProvider* >(this),
                                               static_cast< frame::XTransientDocumentsDocumentContentIdentifierFactory* >(this),
                                               static_cast< frame::XTransientDocumentsDocumentContentFactory* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_5( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider,
                      frame::XTransientDocumentsDocumentContentIdentifierFactory,
                      frame::XTransientDocumentsDocumentContentFactory );


// XServiceInfo methods.

XSERVICEINFO_COMMOM_IMPL( ContentProvider,
                          OUString( "com.sun.star.comp.ucb.TransientDocumentsContentProvider" ) )
/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface >
ContentProvider_CreateInstance( const css::uno::Reference< css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX =
        static_cast<css::lang::XServiceInfo*>(new ContentProvider( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString >
ContentProvider::getSupportedServiceNames_Static()
{
    css::uno::Sequence< OUString > aSNS { "com.sun.star.ucb.TransientDocumentsContentProvider" };
    return aSNS;
}

// Service factory implementation.


ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );


// XContentProvider methods.


// virtual
uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
{
    Uri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException(
            "Invalid URL!",
            Identifier );

    // Normalize URI.
    uno::Reference< ucb::XContentIdentifier > xCanonicId
        = new ::ucbhelper::ContentIdentifier( aUri.getUri() );

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();

    if ( !xContent.is() )
    {
        // Create a new content.
        xContent = Content::create( m_xContext, this, xCanonicId );
        registerNewContent( xContent );
    }

    return xContent;
}


// XTransientDocumentsDocumentContentIdentifierFactory methods.

uno::Reference<ucb::XContentIdentifier> SAL_CALL
ContentProvider::createDocumentContentIdentifier(
        uno::Reference<frame::XModel> const& xModel)
{
    // model -> id -> content identifier -> queryContent
    if ( !m_xDocsMgr.is() )
     {
        throw lang::IllegalArgumentException(
            "No Document Manager!",
            static_cast< cppu::OWeakObject * >( this ),
            1 );
     }

    OUString aDocId = tdoc_ucp::OfficeDocumentsManager::queryDocumentId(xModel);
    if ( aDocId.isEmpty() )
    {
        throw lang::IllegalArgumentException(
            "Unable to obtain document id from model!",
            static_cast< cppu::OWeakObject * >( this ),
            1 );
    }

    OUStringBuffer aBuffer;
    aBuffer.append( TDOC_URL_SCHEME ":/" );
    aBuffer.append( aDocId );

    uno::Reference< ucb::XContentIdentifier > xId
        = new ::ucbhelper::ContentIdentifier( aBuffer.makeStringAndClear() );
    return xId;
}

// XTransientDocumentsDocumentContentFactory methods.

uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::createDocumentContent(
        uno::Reference<frame::XModel> const& xModel)
{
    uno::Reference<ucb::XContentIdentifier> const xId(
            createDocumentContentIdentifier(xModel));

    osl::MutexGuard aGuard( m_aMutex );

    // Check, if a content with given id already exists...
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xId ).get();

    if ( !xContent.is() )
    {
        // Create a new content.
        xContent = Content::create( m_xContext, this, xId );
    }

    if ( xContent.is() )
        return xContent;

    // no content.
    throw lang::IllegalArgumentException(
        "Illegal Content Identifier!",
        static_cast< cppu::OWeakObject * >( this ),
        1 );
}


// interface OfficeDocumentsEventListener


// virtual
void ContentProvider::notifyDocumentClosed( const OUString & rDocId )
{
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    // Notify all content objects related to the closed doc.

    bool bFoundDocumentContent = false;
    rtl::Reference< Content > xRoot;

    for ( const auto& rContent : aAllContents )
    {
        Uri aUri( rContent->getIdentifier()->getContentIdentifier() );
        OSL_ENSURE( aUri.isValid(),
                    "ContentProvider::notifyDocumentClosed - Invalid URI!" );

        if ( !bFoundDocumentContent )
        {
            if ( aUri.isRoot() )
            {
                xRoot = static_cast< Content * >( rContent.get() );
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
                = static_cast< Content * >( rContent.get() );

            xContent->notifyDocumentClosed();
        }
    }

    if ( xRoot.is() )
    {
        // No document content found for rDocId but root content
        // instantiated. Root content must announce document removal
        // to content event listeners.
        xRoot->notifyChildRemoved( rDocId );
    }
}


// virtual
void ContentProvider::notifyDocumentOpened( const OUString & rDocId )
{
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    // Find root content. If instantiated let it propagate document insertion.

    for ( const auto& rContent : aAllContents )
    {
        Uri aUri( rContent->getIdentifier()->getContentIdentifier() );
        OSL_ENSURE( aUri.isValid(),
                    "ContentProvider::notifyDocumentOpened - Invalid URI!" );

        if ( aUri.isRoot() )
        {
            rtl::Reference< Content > xRoot
                = static_cast< Content * >( rContent.get() );
            xRoot->notifyChildInserted( rDocId );

            // Done.
            break;
        }
    }
}


// Non-UNO


uno::Reference< embed::XStorage >
ContentProvider::queryStorage( const OUString & rUri,
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


uno::Reference< embed::XStorage >
ContentProvider::queryStorageClone( const OUString & rUri ) const
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


uno::Reference< io::XInputStream >
ContentProvider::queryInputStream( const OUString & rUri,
                                   const OUString & rPassword ) const
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


uno::Reference< io::XOutputStream >
ContentProvider::queryOutputStream( const OUString & rUri,
                                    const OUString & rPassword,
                                    bool bTruncate ) const
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


uno::Reference< io::XStream >
ContentProvider::queryStream( const OUString & rUri,
                              const OUString & rPassword,
                              bool bTruncate ) const
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


bool ContentProvider::queryNamesOfChildren(
    const OUString & rUri, uno::Sequence< OUString > & rNames ) const
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


OUString
ContentProvider::queryStorageTitle( const OUString & rUri ) const
{
    OUString aTitle;

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        // always empty.
        aTitle.clear();
    }
    else if ( aUri.isDocument() )
    {
        // for documents, title shall not be derived from URL. It shall
        // be something more 'speaking' than just the document UID.
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


uno::Reference< frame::XModel >
ContentProvider::queryDocumentModel( const OUString & rUri ) const
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
