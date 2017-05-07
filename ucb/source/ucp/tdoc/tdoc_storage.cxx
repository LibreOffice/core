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

#include <memory>

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/embed/ElementModes.hpp"
#include "com/sun/star/embed/InvalidStorageException.hpp"
#include "com/sun/star/embed/StorageFactory.hpp"
#include "com/sun/star/embed/StorageWrappedTargetException.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/packages/NoEncryptionException.hpp"
#include <osl/diagnose.h>
#include "comphelper/processfactory.hxx"

#include "tdoc_uri.hxx"
#include "tdoc_docmgr.hxx"
#include "tdoc_stgelems.hxx"

#include "tdoc_storage.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;


// StorageElementFactory Implementation.


StorageElementFactory::StorageElementFactory(
    const uno::Reference< uno::XComponentContext > & rxContext,
    const rtl::Reference< OfficeDocumentsManager > & xDocsMgr )
: m_xDocsMgr( xDocsMgr ),
  m_xContext( rxContext )
{
}


StorageElementFactory::~StorageElementFactory()
{
    OSL_ENSURE( m_aMap.empty(),
        "StorageElementFactory::~StorageElementFactory - Dangling storages!" );
}


uno::Reference< embed::XStorage >
StorageElementFactory::createTemporaryStorage()
{
    uno::Reference< embed::XStorage > xStorage;
    uno::Reference< lang::XSingleServiceFactory > xStorageFac;
    if ( m_xContext.is() )
    {
        xStorageFac = embed::StorageFactory::create( m_xContext );
    }

    OSL_ENSURE( xStorageFac.is(), "Can't create storage factory!" );
    if ( xStorageFac.is() )
        xStorage.set( xStorageFac->createInstance(), uno::UNO_QUERY );

    if ( !xStorage.is() )
        throw uno::RuntimeException();

    return xStorage;
}


uno::Reference< embed::XStorage >
StorageElementFactory::createStorage( const OUString & rUri,
                                      StorageAccessMode eMode )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( ( eMode != READ ) &&
         ( eMode != READ_WRITE_NOCREATE ) &&
         ( eMode != READ_WRITE_CREATE ) )
        throw lang::IllegalArgumentException(
            "Invalid open mode!",
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        throw lang::IllegalArgumentException(
            "Root never has a storage!",
            uno::Reference< uno::XInterface >(),
            sal_Int16( 1 ) );
    }

    OUString aUriKey
        ( rUri.endsWith("/")
          ? rUri.copy( 0, rUri.getLength() - 1 )
          : rUri );

    StorageMap::iterator aIt ( m_aMap.begin() );
    StorageMap::iterator aEnd( m_aMap.end() );

    while ( aIt != aEnd )
    {
        if ( (*aIt).first.first == aUriKey )
        {
            // URI matches. Now, check open mode.
            bool bMatch = true;
            switch ( eMode )
            {
                case READ:
                    // No need to check; storage is at least readable.
                    bMatch = true;
                    break;

                case READ_WRITE_NOCREATE:
                case READ_WRITE_CREATE:
                    // If found storage is writable, it can be used.
                    // If not, a new one must be created.
                    bMatch = (*aIt).first.second;
                    break;
            }

            if ( bMatch )
                break;
        }
        ++aIt;
    }

    if ( aIt == aEnd )
    {
        uno::Reference< embed::XStorage > xParentStorage;

        // documents never have a parent storage.
        if ( !aUri.isDocument() )
        {
            xParentStorage = queryParentStorage( aUriKey, eMode );

            if ( !xParentStorage.is() )
            {
                // requested to create new storage, but failed?
                OSL_ENSURE( eMode != READ_WRITE_CREATE,
                            "Unable to create parent storage!" );
                return xParentStorage;
            }
        }

        uno::Reference< embed::XStorage > xStorage
            = queryStorage( xParentStorage, aUriKey, eMode );

        if ( !xStorage.is() )
        {
            // requested to create new storage, but failed?
            OSL_ENSURE( eMode != READ_WRITE_CREATE,
                        "Unable to create storage!" );
            return xStorage;
        }

        bool bWritable = ( ( eMode == READ_WRITE_NOCREATE )
                            || ( eMode == READ_WRITE_CREATE ) );

        rtl::Reference< Storage > xElement(
            new Storage( m_xContext, this, aUriKey, xParentStorage, xStorage ) );

        aIt = m_aMap.insert(
            StorageMap::value_type(
                std::pair< OUString, bool >( aUriKey, bWritable ),
                xElement.get() ) ).first;

        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
    else if ( osl_atomic_increment( &aIt->second->m_refCount ) > 1 )
    {
        rtl::Reference< Storage > xElement( aIt->second );
        osl_atomic_decrement( &aIt->second->m_refCount );
        return aIt->second;
    }
    else
    {
        osl_atomic_decrement( &aIt->second->m_refCount );
        aIt->second->m_aContainerIt = m_aMap.end();

        uno::Reference< embed::XStorage > xParentStorage;

        // documents never have a parent storage.
        if ( !aUri.isDocument() )
        {
            xParentStorage = queryParentStorage( aUriKey, eMode );

            if ( !xParentStorage.is() )
            {
                // requested to create new storage, but failed?
                OSL_ENSURE( eMode != READ_WRITE_CREATE,
                            "Unable to create parent storage!" );
                return xParentStorage;
            }
        }

        uno::Reference< embed::XStorage > xStorage
            = queryStorage( xParentStorage, aUriKey, eMode );

        if ( !xStorage.is() )
        {
            // requested to create new storage, but failed?
            OSL_ENSURE( eMode != READ_WRITE_CREATE,
                        "Unable to create storage!" );
            return xStorage;
        }

        aIt->second = new Storage( m_xContext, this, aUriKey, xParentStorage, xStorage );
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}


uno::Reference< io::XInputStream >
StorageElementFactory::createInputStream( const OUString & rUri,
                                          const OUString & rPassword )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< embed::XStorage > xParentStorage
        = queryParentStorage( rUri, READ );

    // Each stream must have a parent storage.
    if ( !xParentStorage.is() )
        return uno::Reference< io::XInputStream >();

    uno::Reference< io::XStream > xStream
        = queryStream( xParentStorage, rUri, rPassword, READ, false );

    if ( !xStream.is() )
        return uno::Reference< io::XInputStream >();

    return xStream->getInputStream();
}


uno::Reference< io::XOutputStream >
StorageElementFactory::createOutputStream( const OUString & rUri,
                                           const OUString & rPassword,
                                           bool bTruncate )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< embed::XStorage > xParentStorage
        = queryParentStorage( rUri, READ_WRITE_CREATE );

    // Each stream must have a parent storage.
    if ( !xParentStorage.is() )
    {
        OSL_FAIL( "StorageElementFactory::createOutputStream - "
                    "Unable to create parent storage!" );
        return uno::Reference< io::XOutputStream >();
    }

    uno::Reference< io::XStream > xStream
        = queryStream(
            xParentStorage, rUri, rPassword, READ_WRITE_CREATE, bTruncate );

    if ( !xStream.is() )
    {
        OSL_FAIL( "StorageElementFactory::createOutputStream - "
                    "Unable to create stream!" );
        return uno::Reference< io::XOutputStream >();
    }

    // Note: We need a wrapper to hold a reference to the parent storage to
    //       ensure that nobody else owns it at the moment we want to commit
    //       our changes. (There can be only one writable instance at a time
    //       and even no writable instance if there is  already another
    //       read-only instance!)
    return uno::Reference< io::XOutputStream >(
        new OutputStream( m_xContext, rUri, xParentStorage, xStream->getOutputStream() ) );
}


uno::Reference< io::XStream >
StorageElementFactory::createStream( const OUString & rUri,
                                     const OUString & rPassword,
                                     bool bTruncate )
{
    osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< embed::XStorage > xParentStorage
        = queryParentStorage( rUri, READ_WRITE_CREATE );

    // Each stream must have a parent storage.
    if ( !xParentStorage.is() )
    {
        OSL_FAIL( "StorageElementFactory::createStream - "
                    "Unable to create parent storage!" );
        return uno::Reference< io::XStream >();
    }

    uno::Reference< io::XStream > xStream
        = queryStream(
            xParentStorage, rUri, rPassword, READ_WRITE_NOCREATE, bTruncate );

    if ( !xStream.is() )
    {
        OSL_FAIL( "StorageElementFactory::createStream - "
                    "Unable to create stream!" );
        return uno::Reference< io::XStream >();
    }

    return uno::Reference< io::XStream >(
        new Stream( m_xContext, rUri, xParentStorage, xStream ) );
}


void StorageElementFactory::releaseElement( Storage * pElement )
{
    OSL_ASSERT( pElement );
    osl::MutexGuard aGuard( m_aMutex );
    if ( pElement->m_aContainerIt != m_aMap.end() )
        m_aMap.erase( pElement->m_aContainerIt );
}


// Non-UNO interface


uno::Reference< embed::XStorage > StorageElementFactory::queryParentStorage(
        const OUString & rUri, StorageAccessMode eMode )
{
    uno::Reference< embed::XStorage > xParentStorage;

    Uri aUri( rUri );
    Uri aParentUri( aUri.getParentUri() );
    if ( !aParentUri.isRoot() )
    {
        xParentStorage = createStorage( aUri.getParentUri(), eMode );
        OSL_ENSURE( xParentStorage.is()
                    // requested to create new storage, but failed?
                    || ( eMode != READ_WRITE_CREATE ),
                    "StorageElementFactory::queryParentStorage - No storage!" );
    }
    return xParentStorage;
}


uno::Reference< embed::XStorage > StorageElementFactory::queryStorage(
        const uno::Reference< embed::XStorage > & xParentStorage,
        const OUString & rUri,
        StorageAccessMode eMode )
{
    uno::Reference< embed::XStorage > xStorage;

    Uri aUri( rUri );

    if ( !xParentStorage.is() )
    {
        // document storage

        xStorage = m_xDocsMgr->queryStorage( aUri.getDocumentId() );

        if ( !xStorage.is() )
        {
            if ( eMode == READ_WRITE_CREATE )
                throw lang::IllegalArgumentException(
                    "Invalid open mode: document storages cannot be created!",
                    uno::Reference< uno::XInterface >(),
                    sal_Int16( 2 ) );
            else
                throw embed::InvalidStorageException(
                    "Invalid document id!",
                    uno::Reference< uno::XInterface >() );
        }

        // match xStorage's open mode against requested open mode

        uno::Reference< beans::XPropertySet > xPropSet(
            xStorage, uno::UNO_QUERY );
        OSL_ENSURE( xPropSet.is(),
                    "StorageElementFactory::queryStorage - "
                    "No XPropertySet interface!" );
        try
        {
            uno::Any aPropValue = xPropSet->getPropertyValue("OpenMode");

            sal_Int32 nOpenMode = 0;
            if ( aPropValue >>= nOpenMode )
            {
                switch ( eMode )
                {
                    case READ:
                        if ( !( nOpenMode & embed::ElementModes::READ ) )
                        {
                            // document opened, but not readable.
                            throw embed::InvalidStorageException(
                                    "Storage is open, but not readable!" );
                        }
                        // storage okay
                        break;

                    case READ_WRITE_NOCREATE:
                    case READ_WRITE_CREATE:
                        if ( !( nOpenMode & embed::ElementModes::WRITE ) )
                        {
                            // document opened, but not writable.
                            throw embed::InvalidStorageException(
                                    "Storage is open, but not writable!" );
                        }
                        // storage okay
                        break;
                }
            }
            else
            {
                OSL_FAIL(
                    "Bug! Value of property OpenMode has wrong type!" );

                throw uno::RuntimeException(
                        "Bug! Value of property OpenMode has wrong type!" );
            }
        }
        catch ( beans::UnknownPropertyException const & e )
        {
            OSL_FAIL( "Property OpenMode not supported!" );

            throw embed::StorageWrappedTargetException(
                    "Bug! Value of property OpenMode has wrong type!",
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
        }
        catch ( lang::WrappedTargetException const & e )
        {
            OSL_FAIL( "Caught WrappedTargetException!" );

            throw embed::StorageWrappedTargetException(
                    "WrappedTargetException during getPropertyValue!",
                    uno::Reference< uno::XInterface >(),
                    uno::makeAny( e ) );
        }
    }
    else
    {
        // sub storage

        const OUString & rName = aUri.getDecodedName();

        if ( eMode == READ )
        {
            try
            {
                sal_Int32 nOpenMode = embed::ElementModes::READ
                                      | embed::ElementModes::NOCREATE;
                xStorage
                    = xParentStorage->openStorageElement( rName, nOpenMode );
            }
            catch ( io::IOException const & )
            {
                // Another chance: Try to clone storage.
                xStorage = createTemporaryStorage();
                xParentStorage->copyStorageElementLastCommitTo( rName,
                                                                xStorage );
            }
        }
        else
        {
            sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
            if ( eMode == READ_WRITE_NOCREATE )
                nOpenMode |= embed::ElementModes::NOCREATE;

            xStorage = xParentStorage->openStorageElement( rName, nOpenMode );
        }
    }

    OSL_ENSURE( xStorage.is() || ( eMode != READ_WRITE_CREATE ),
                "StorageElementFactory::queryStorage - No storage!" );
    return xStorage;
}


uno::Reference< io::XStream >
StorageElementFactory::queryStream(
                const uno::Reference< embed::XStorage > & xParentStorage,
                const OUString & rUri,
                const OUString & rPassword,
                StorageAccessMode eMode,
                bool bTruncate )
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !xParentStorage.is() )
    {
        throw lang::IllegalArgumentException(
            "No parent storage!",
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }

    Uri aUri( rUri );
    if ( aUri.isRoot() )
    {
        throw lang::IllegalArgumentException(
            "Root never is a stream!",
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }
    else if ( aUri.isDocument() )
    {
        throw lang::IllegalArgumentException(
            "A document never is a stream!",
            uno::Reference< uno::XInterface >(),
            sal_Int16( 2 ) );
    }

    sal_Int32 nOpenMode;
    switch ( eMode )
    {
        case READ:
            nOpenMode = embed::ElementModes::READ
                        | embed::ElementModes::NOCREATE
                        | embed::ElementModes::SEEKABLE;
            break;

        case READ_WRITE_NOCREATE:
            nOpenMode = embed::ElementModes::READWRITE
                        | embed::ElementModes::NOCREATE
                        | embed::ElementModes::SEEKABLE;

            if ( bTruncate )
                nOpenMode |= embed::ElementModes::TRUNCATE;

            break;

        case READ_WRITE_CREATE:
            nOpenMode = embed::ElementModes::READWRITE
                        | embed::ElementModes::SEEKABLE;

            if ( bTruncate )
                nOpenMode |= embed::ElementModes::TRUNCATE;

            break;

        default:
            OSL_FAIL( "StorageElementFactory::queryStream : Unknown open mode!" );

            throw embed::InvalidStorageException(
                "Unknown open mode!",
                uno::Reference< uno::XInterface >() );
    }

    // No object re-usage mechanism; streams are seekable => not stateless.

    uno::Reference< io::XStream > xStream;
    if ( !rPassword.isEmpty() )
    {
        if ( eMode == READ )
        {
            try
            {
                xStream = xParentStorage->cloneEncryptedStreamElement(
                                                         aUri.getDecodedName(),
                                                         rPassword );
            }
            catch ( packages::NoEncryptionException const & )
            {
                xStream
                    = xParentStorage->cloneStreamElement( aUri.getDecodedName() );
            }
        }
        else
        {
            try
            {
                xStream = xParentStorage->openEncryptedStreamElement(
                                                         aUri.getDecodedName(),
                                                         nOpenMode,
                                                         rPassword );
            }
            catch ( packages::NoEncryptionException const & )
            {
                xStream
                    = xParentStorage->openStreamElement( aUri.getDecodedName(),
                                                         nOpenMode );
            }
        }
    }
    else
    {
        if ( eMode == READ )
        {
            xStream = xParentStorage->cloneStreamElement( aUri.getDecodedName() );
        }
        else
        {
            xStream = xParentStorage->openStreamElement( aUri.getDecodedName(),
                                                         nOpenMode );
        }
    }

    if ( !xStream.is() )
    {
        throw embed::InvalidStorageException(
            "No stream!",
            uno::Reference< uno::XInterface >() );
    }

    return xStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
