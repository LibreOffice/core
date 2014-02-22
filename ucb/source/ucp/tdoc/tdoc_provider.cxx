/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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



//

//



ContentProvider::ContentProvider(
            const uno::Reference< uno::XComponentContext >& rxContext )
: ::ucbhelper::ContentProviderImplHelper( rxContext ),
  m_xDocsMgr( new OfficeDocumentsManager( rxContext, this ) ),
  m_xStgElemFac( new StorageElementFactory( rxContext, m_xDocsMgr ) )
{
}



ContentProvider::~ContentProvider()
{
    if ( m_xDocsMgr.is() )
        m_xDocsMgr->destroy();
}


//

//


XINTERFACE_IMPL_4( ContentProvider,
                   lang::XTypeProvider,
                   lang::XServiceInfo,
                   ucb::XContentProvider,
                   frame::XTransientDocumentsDocumentContentFactory );


//

//


XTYPEPROVIDER_IMPL_4( ContentProvider,
                      lang::XTypeProvider,
                      lang::XServiceInfo,
                      ucb::XContentProvider,
                      frame::XTransientDocumentsDocumentContentFactory );


//

//


XSERVICEINFO_IMPL_1_CTX(
    ContentProvider,
    OUString( "com.sun.star.comp.ucb.TransientDocumentsContentProvider" ),
    OUString( TDOC_CONTENT_PROVIDER_SERVICE_NAME ) );


//

//


ONE_INSTANCE_SERVICE_FACTORY_IMPL( ContentProvider );


//

//



uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::queryContent(
        const uno::Reference< ucb::XContentIdentifier >& Identifier )
    throw( ucb::IllegalIdentifierException, uno::RuntimeException )
{
    Uri aUri( Identifier->getContentIdentifier() );
    if ( !aUri.isValid() )
        throw ucb::IllegalIdentifierException(
            OUString( "Invalid URL!" ),
            Identifier );

    
    uno::Reference< ucb::XContentIdentifier > xCanonicId
        = new ::ucbhelper::ContentIdentifier( aUri.getUri() );

    osl::MutexGuard aGuard( m_aMutex );

    
    uno::Reference< ucb::XContent > xContent
        = queryExistingContent( xCanonicId ).get();

    if ( !xContent.is() )
    {
        
        xContent = Content::create( m_xContext, this, xCanonicId );
        registerNewContent( xContent );
    }

    return xContent;
}


//

//



uno::Reference< ucb::XContent > SAL_CALL
ContentProvider::createDocumentContent(
        const uno::Reference< frame::XModel >& Model )
    throw ( lang::IllegalArgumentException, uno::RuntimeException )
{
    
    if ( m_xDocsMgr.is() )
    {
        OUString aDocId = m_xDocsMgr->queryDocumentId( Model );
        if ( !aDocId.isEmpty() )
        {
            OUStringBuffer aBuffer;
            aBuffer.appendAscii( TDOC_URL_SCHEME ":/" );
            aBuffer.append( aDocId );

            uno::Reference< ucb::XContentIdentifier > xId
                = new ::ucbhelper::ContentIdentifier( aBuffer.makeStringAndClear() );

            osl::MutexGuard aGuard( m_aMutex );

            
            uno::Reference< ucb::XContent > xContent
                = queryExistingContent( xId ).get();

            if ( !xContent.is() )
            {
                
                xContent = Content::create( m_xContext, this, xId );
            }

            if ( xContent.is() )
                return xContent;

            
            throw lang::IllegalArgumentException(
                OUString(
                    "Illegal Content Identifier!" ),
                static_cast< cppu::OWeakObject * >( this ),
                1 );
        }
        else
        {
            throw lang::IllegalArgumentException(
                OUString(
                    "Unable to obtain document id from model!" ),
                static_cast< cppu::OWeakObject * >( this ),
                1 );
        }
     }
     else
     {
        throw lang::IllegalArgumentException(
            OUString(
                "No Document Manager!" ),
            static_cast< cppu::OWeakObject * >( this ),
            1 );
     }
}


//

//



void ContentProvider::notifyDocumentClosed( const OUString & rDocId )
{
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    

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

                    
                    
                    xRoot.clear();
                }
            }
        }

        if ( aUri.getDocumentId() == rDocId )
        {
            
            rtl::Reference< Content > xContent
                = static_cast< Content * >( (*it).get() );

            xContent->notifyDocumentClosed();
        }

        ++it;
    }

    if ( xRoot.is() )
    {
        
        
        
        xRoot->notifyChildRemoved( rDocId );
    }
}



void ContentProvider::notifyDocumentOpened( const OUString & rDocId )
{
    osl::MutexGuard aGuard( getContentListMutex() );

    ::ucbhelper::ContentRefList aAllContents;
    queryExistingContents( aAllContents );

    ::ucbhelper::ContentRefList::const_iterator it  = aAllContents.begin();
    ::ucbhelper::ContentRefList::const_iterator end = aAllContents.end();

    

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

            
            break;
        }

        ++it;
    }
}


//

//


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





    }
    return uno::Reference< io::XInputStream >();
}


uno::Reference< io::XOutputStream >
ContentProvider::queryOutputStream( const OUString & rUri,
                                    const OUString & rPassword,
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
            
            
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
        }





    }
    return uno::Reference< io::XOutputStream >();
}


uno::Reference< io::XStream >
ContentProvider::queryStream( const OUString & rUri,
                              const OUString & rPassword,
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
            
            
        }
        catch ( embed::StorageWrappedTargetException const & )
        {
            OSL_FAIL( "Caught embed::StorageWrappedTargetException!" );
        }





    }
    return uno::Reference< io::XStream >();
}


bool ContentProvider::queryNamesOfChildren(
    const OUString & rUri, uno::Sequence< OUString > & rNames ) const
{
    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        
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
        
        aTitle = OUString();
    }
    else if ( aUri.isDocument() )
    {
        
        
        if ( m_xDocsMgr.is() )
            aTitle = m_xDocsMgr->queryStorageTitle( aUri.getDocumentId() );
    }
    else
    {
        
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
