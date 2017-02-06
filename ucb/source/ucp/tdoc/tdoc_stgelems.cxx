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

 - remove root storage access workaround

 *************************************************************************/

#include <osl/diagnose.h>
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/reflection/ProxyFactory.hpp"

#include "tdoc_uri.hxx"

#include "tdoc_stgelems.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;


// ParentStorageHolder Implementation.


ParentStorageHolder::ParentStorageHolder(
            const uno::Reference< embed::XStorage > & xParentStorage,
            const OUString & rUri )
: m_xParentStorage( xParentStorage ),
  m_bParentIsRootStorage( false )
{
    Uri aUri( rUri );
    if ( aUri.isDocument() )
        m_bParentIsRootStorage = true;
}


// Storage Implementation.


Storage::Storage( const uno::Reference< uno::XComponentContext > & rxContext,
                  const rtl::Reference< StorageElementFactory > & xFactory,
                  const OUString & rUri,
                  const uno::Reference< embed::XStorage > & xParentStorage,
                  const uno::Reference< embed::XStorage > & xStorageToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xFactory( xFactory ),
  m_xWrappedStorage( xStorageToWrap ),
  m_xWrappedTransObj( xStorageToWrap, uno::UNO_QUERY ), // optional interface
  m_xWrappedComponent( xStorageToWrap, uno::UNO_QUERY ),
  m_xWrappedTypeProv( xStorageToWrap, uno::UNO_QUERY ),
  m_bIsDocumentStorage( Uri( rUri ).isDocument() )
{
    OSL_ENSURE( m_xWrappedStorage.is(),
                "Storage::Storage: No storage to wrap!" );

    OSL_ENSURE( m_xWrappedComponent.is(),
                "Storage::Storage: No component to wrap!" );

    OSL_ENSURE( m_xWrappedTypeProv.is(),
                "Storage::Storage: No Type Provider!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac =
            reflection::ProxyFactory::create( rxContext );
        m_xAggProxy = xProxyFac->createProxy( m_xWrappedStorage );
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "Storage::Storage: Caught exception!" );
    }

    OSL_ENSURE( m_xAggProxy.is(),
                "Storage::Storage: Wrapped storage cannot be aggregated!" );

    if ( m_xAggProxy.is() )
    {
        osl_atomic_increment( &m_refCount );
        {
            // Solaris compiler problem:
            // Extra block to enforce destruction of temporary object created
            // in next statement _before_ osl_atomic_decrement is
            // called.  Otherwise 'this' will destroy itself even before ctor
            // is completed (See impl. of XInterface::release())!

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}


// virtual
Storage::~Storage()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );

    // Never dispose a document storage. Not owner!
    if ( !m_bIsDocumentStorage )
    {
        if ( m_xWrappedComponent.is() )
        {
            // "Auto-dispose"...
            try
            {
                m_xWrappedComponent->dispose();
            }
            catch ( lang::DisposedException const & )
            {
                // might happen.
            }
            catch ( ... )
            {
                OSL_FAIL( "Storage::~Storage - Caught exception!" );
            }
        }
    }
}


// uno::XInterface


// virtual
uno::Any SAL_CALL Storage::queryInterface( const uno::Type& aType )
{
    // First, try to use interfaces implemented by myself and base class(es)
    uno::Any aRet = StorageUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    // Try to use requested interface from aggregated storage
    return m_xAggProxy->queryAggregation( aType );
}


// virtual
void SAL_CALL Storage::acquire()
    throw ()
{
    osl_atomic_increment( &m_refCount );
}


// virtual
void SAL_CALL Storage::release()
    throw ()
{
    //#i120738, Storage::release overrides OWeakObject::release(),
    //need call OWeakObject::release() to release OWeakObject::m_pWeakConnectionPoint

    if ( m_refCount == 1 )
        m_xFactory->releaseElement( this );

    //delete this;
    OWeakObject::release();
}


// lang::XTypeProvider


// virtual
uno::Sequence< uno::Type > SAL_CALL Storage::getTypes()
{
    return m_xWrappedTypeProv->getTypes();
}


// virtual
uno::Sequence< sal_Int8 > SAL_CALL Storage::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// lang::XComponent (base of embed::XStorage)


// virtual
void SAL_CALL Storage::dispose()
{
    m_xWrappedStorage->dispose();
}


// virtual
void SAL_CALL Storage::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
{
    m_xWrappedStorage->addEventListener( xListener );
}

// virtual
void SAL_CALL Storage::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
{
    m_xWrappedStorage->removeEventListener( aListener );
}


// container::XElementAccess (base of container::XNameAccess)


// virtual
uno::Type SAL_CALL Storage::getElementType()
{
    return m_xWrappedStorage->getElementType();
}


// virtual
sal_Bool SAL_CALL Storage::hasElements()
{
    return m_xWrappedStorage->hasElements();
}


// container::XNameAccess (base of embed::XStorage)


// virtual
uno::Any SAL_CALL Storage::getByName( const OUString& aName )
{
    return m_xWrappedStorage->getByName( aName );
}


// virtual
uno::Sequence< OUString > SAL_CALL Storage::getElementNames()
{
    return m_xWrappedStorage->getElementNames();
}


// virtual
sal_Bool SAL_CALL Storage::hasByName( const OUString& aName )
{
    return m_xWrappedStorage->hasByName( aName );
}


// embed::XStorage


// virtual
void SAL_CALL Storage::copyToStorage(
        const uno::Reference< embed::XStorage >& xDest )
{
    m_xWrappedStorage->copyToStorage( xDest );
}


// virtual
uno::Reference< io::XStream > SAL_CALL Storage::openStreamElement(
        const OUString& aStreamName, sal_Int32 nOpenMode )
{
    return m_xWrappedStorage->openStreamElement( aStreamName, nOpenMode );
}


// virtual
uno::Reference< io::XStream > SAL_CALL Storage::openEncryptedStreamElement(
        const OUString& aStreamName,
        sal_Int32 nOpenMode,
        const OUString& aPassword )
{
    return m_xWrappedStorage->openEncryptedStreamElement(
        aStreamName, nOpenMode, aPassword );
}


// virtual
uno::Reference< embed::XStorage > SAL_CALL Storage::openStorageElement(
        const OUString& aStorName, sal_Int32 nOpenMode )
{
    return m_xWrappedStorage->openStorageElement( aStorName, nOpenMode );
}


// virtual
uno::Reference< io::XStream > SAL_CALL Storage::cloneStreamElement(
        const OUString& aStreamName )
{
    return m_xWrappedStorage->cloneStreamElement( aStreamName );
}


// virtual
uno::Reference< io::XStream > SAL_CALL Storage::cloneEncryptedStreamElement(
        const OUString& aStreamName,
        const OUString& aPassword )
{
    return m_xWrappedStorage->cloneEncryptedStreamElement( aStreamName,
                                                           aPassword );
}


// virtual
void SAL_CALL Storage::copyLastCommitTo(
        const uno::Reference< embed::XStorage >& xTargetStorage )
{
    m_xWrappedStorage->copyLastCommitTo( xTargetStorage );
}


// virtual
void SAL_CALL Storage::copyStorageElementLastCommitTo(
        const OUString& aStorName,
        const uno::Reference< embed::XStorage >& xTargetStorage )
{
    m_xWrappedStorage->copyStorageElementLastCommitTo( aStorName, xTargetStorage );
}


// virtual
sal_Bool SAL_CALL Storage::isStreamElement(
        const OUString& aElementName )
{
    return m_xWrappedStorage->isStreamElement( aElementName );
}


// virtual
sal_Bool SAL_CALL Storage::isStorageElement(
        const OUString& aElementName )
{
    return m_xWrappedStorage->isStorageElement( aElementName );
}


// virtual
void SAL_CALL Storage::removeElement( const OUString& aElementName )
{
    m_xWrappedStorage->removeElement( aElementName );
}


// virtual
void SAL_CALL Storage::renameElement( const OUString& aEleName,
                                      const OUString& aNewName )
{
    m_xWrappedStorage->renameElement( aEleName, aNewName );
}


// virtual
void SAL_CALL Storage::copyElementTo(
        const OUString& aElementName,
        const uno::Reference< embed::XStorage >& xDest,
        const OUString& aNewName )
{
    m_xWrappedStorage->copyElementTo( aElementName, xDest, aNewName );
}


// virtual
void SAL_CALL Storage::moveElementTo(
        const OUString& aElementName,
        const uno::Reference< embed::XStorage >& xDest,
        const OUString& rNewName )
{
    m_xWrappedStorage->moveElementTo( aElementName, xDest, rNewName );
}


// embed::XTransactedObject


// virtual
void SAL_CALL Storage::commit()
{
    // Never commit a root storage (-> has no parent)!
    // Would lead in writing the whole document to disk.

    uno::Reference< embed::XStorage > xParentStorage = getParentStorage();
    if ( xParentStorage.is() )
    {
        OSL_ENSURE( m_xWrappedTransObj.is(), "No XTransactedObject interface!" );

        if ( m_xWrappedTransObj.is() )
        {
            m_xWrappedTransObj->commit();

            if ( !isParentARootStorage() )
            {
                uno::Reference< embed::XTransactedObject > xParentTA(
                    xParentStorage, uno::UNO_QUERY );
                OSL_ENSURE( xParentTA.is(), "No XTransactedObject interface!" );

                if ( xParentTA.is() )
                    xParentTA->commit();
            }
        }
    }
}


// virtual
void SAL_CALL Storage::revert()
{
    uno::Reference< embed::XStorage > xParentStorage = getParentStorage();
    if ( xParentStorage.is() )
    {
        OSL_ENSURE( m_xWrappedTransObj.is(), "No XTransactedObject interface!" );

        if ( m_xWrappedTransObj.is() )
        {
            m_xWrappedTransObj->revert();

            if ( !isParentARootStorage() )
            {
                uno::Reference< embed::XTransactedObject > xParentTA(
                    xParentStorage, uno::UNO_QUERY );
                OSL_ENSURE( xParentTA.is(), "No XTransactedObject interface!" );

                if ( xParentTA.is() )
                    xParentTA->revert();
            }
        }
    }
}


// OutputStream Implementation.


OutputStream::OutputStream(
            const uno::Reference< uno::XComponentContext > & rxContext,
            const OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XOutputStream > & xStreamToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xWrappedStream( xStreamToWrap ),
  m_xWrappedComponent( xStreamToWrap, uno::UNO_QUERY ),
  m_xWrappedTypeProv( xStreamToWrap, uno::UNO_QUERY )
{
    OSL_ENSURE( m_xWrappedStream.is(),
                "OutputStream::OutputStream: No stream to wrap!" );

    OSL_ENSURE( m_xWrappedComponent.is(),
                "OutputStream::OutputStream: No component to wrap!" );

    OSL_ENSURE( m_xWrappedTypeProv.is(),
                "OutputStream::OutputStream: No Type Provider!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac =
            reflection::ProxyFactory::create( rxContext );
        m_xAggProxy = xProxyFac->createProxy( m_xWrappedStream );
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "OutputStream::OutputStream: Caught exception!" );
    }

    OSL_ENSURE( m_xAggProxy.is(),
            "OutputStream::OutputStream: Wrapped stream cannot be aggregated!" );

    if ( m_xAggProxy.is() )
    {
        osl_atomic_increment( &m_refCount );
        {
            // Solaris compiler problem:
            // Extra block to enforce destruction of temporary object created
            // in next statement _before_ osl_atomic_decrement is
            // called.  Otherwise 'this' will destroy itself even before ctor
            // is completed (See impl. of XInterface::release())!

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}


// virtual
OutputStream::~OutputStream()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
}


// uno::XInterface


// virtual
uno::Any SAL_CALL OutputStream::queryInterface( const uno::Type& aType )
{
    uno::Any aRet = OutputStreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xAggProxy.is() )
        return m_xAggProxy->queryAggregation( aType );
    else
        return uno::Any();
}


// lang::XTypeProvider


// virtual
uno::Sequence< uno::Type > SAL_CALL OutputStream::getTypes()
{
    return m_xWrappedTypeProv->getTypes();
}


// virtual
uno::Sequence< sal_Int8 > SAL_CALL OutputStream::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// io::XOutputStream


// virtual
void SAL_CALL
OutputStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
{
    m_xWrappedStream->writeBytes( aData );
}


// virtual
void SAL_CALL
OutputStream::flush()
{
    m_xWrappedStream->flush();
}


// virtual
void SAL_CALL
OutputStream::closeOutput(  )
{
    m_xWrappedStream->closeOutput();

    // Release parent storage.
    // Now, that the stream is closed/disposed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}


// lang::XComponent


// virtual
void SAL_CALL
OutputStream::dispose()
{
    m_xWrappedComponent->dispose();

    // Release parent storage.
    // Now, that the stream is closed/disposed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}


// virtual
void SAL_CALL
OutputStream::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
{
    m_xWrappedComponent->addEventListener( xListener );
}


// virtual
void SAL_CALL
OutputStream::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
{
    m_xWrappedComponent->removeEventListener( aListener );
}


// Stream Implementation.


Stream::Stream(
            const uno::Reference< uno::XComponentContext > & rxContext,
            const OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XStream > & xStreamToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xWrappedStream( xStreamToWrap ),
  m_xWrappedOutputStream( xStreamToWrap->getOutputStream() ), // might be empty
  m_xWrappedTruncate( m_xWrappedOutputStream, uno::UNO_QUERY ), // might be empty
  m_xWrappedInputStream( xStreamToWrap->getInputStream(), uno::UNO_QUERY ),
  m_xWrappedComponent( xStreamToWrap, uno::UNO_QUERY ),
  m_xWrappedTypeProv( xStreamToWrap, uno::UNO_QUERY )
{
    OSL_ENSURE( m_xWrappedStream.is(),
                "OutputStream::OutputStream: No stream to wrap!" );

    OSL_ENSURE( m_xWrappedComponent.is(),
                "OutputStream::OutputStream: No component to wrap!" );

    OSL_ENSURE( m_xWrappedTypeProv.is(),
                "OutputStream::OutputStream: No Type Provider!" );

    // Use proxy factory service to create aggregatable proxy.
    try
    {
        uno::Reference< reflection::XProxyFactory > xProxyFac =
            reflection::ProxyFactory::create( rxContext );
        m_xAggProxy = xProxyFac->createProxy( m_xWrappedStream );
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "OutputStream::OutputStream: Caught exception!" );
    }

    OSL_ENSURE( m_xAggProxy.is(),
            "OutputStream::OutputStream: Wrapped stream cannot be aggregated!" );

    if ( m_xAggProxy.is() )
    {
        osl_atomic_increment( &m_refCount );
        {
            // Solaris compiler problem:
            // Extra block to enforce destruction of temporary object created
            // in next statement _before_ osl_atomic_decrement is
            // called.  Otherwise 'this' will destroy itself even before ctor
            // is completed (See impl. of XInterface::release())!

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}


// virtual
Stream::~Stream()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
}


// uno::XInterface


// virtual
uno::Any SAL_CALL Stream::queryInterface( const uno::Type& aType )
{
    uno::Any aRet = StreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xAggProxy.is() )
        return m_xAggProxy->queryAggregation( aType );
    else
        return uno::Any();
}


// lang::XTypeProvider


// virtual
uno::Sequence< uno::Type > SAL_CALL Stream::getTypes()
{
    return m_xWrappedTypeProv->getTypes();
}


// virtual
uno::Sequence< sal_Int8 > SAL_CALL Stream::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


// io::XStream.


// virtual
uno::Reference< io::XInputStream > SAL_CALL Stream::getInputStream()
{
    return uno::Reference< io::XInputStream >( this );
}


// virtual
uno::Reference< io::XOutputStream > SAL_CALL Stream::getOutputStream()
{
    return uno::Reference< io::XOutputStream >( this );
}


// io::XOutputStream.


// virtual
void SAL_CALL Stream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->writeBytes( aData );
        commitChanges();
    }
}


// virtual
void SAL_CALL Stream::flush()
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->flush();
        commitChanges();
    }
}


// virtual
void SAL_CALL Stream::closeOutput()
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->closeOutput();
        commitChanges();
    }

    // Release parent storage.
    // Now, that the stream is closed/disposed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}


// io::XTruncate.


// virtual
void SAL_CALL Stream::truncate()
{
    if ( m_xWrappedTruncate.is() )
    {
        m_xWrappedTruncate->truncate();
        commitChanges();
    }
}


// io::XInputStream.


// virtual
sal_Int32 SAL_CALL Stream::readBytes( uno::Sequence< sal_Int8 >& aData,
                                      sal_Int32 nBytesToRead )
{
    return m_xWrappedInputStream->readBytes( aData, nBytesToRead );
}


// virtual
sal_Int32 SAL_CALL Stream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                                          sal_Int32 nMaxBytesToRead )
{
    return m_xWrappedInputStream->readSomeBytes( aData, nMaxBytesToRead );
}


// virtual
void SAL_CALL Stream::skipBytes( sal_Int32 nBytesToSkip )
{
    m_xWrappedInputStream->skipBytes( nBytesToSkip );
}


// virtual
sal_Int32 SAL_CALL Stream::available()
{
    return m_xWrappedInputStream->available();
}


// virtual
void SAL_CALL Stream::closeInput()
{
    m_xWrappedInputStream->closeInput();
}


// lang::XComponent


// virtual
void SAL_CALL Stream::dispose()
{
    m_xWrappedComponent->dispose();

    // Release parent storage.
    // Now, that the stream is closed/disposed it is not needed any longer.
    setParentStorage( uno::Reference< embed::XStorage >() );
}


// virtual
void SAL_CALL Stream::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
{
    m_xWrappedComponent->addEventListener( xListener );
}


// virtual
void SAL_CALL Stream::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
{
    m_xWrappedComponent->removeEventListener( aListener );
}


// Non-UNO


void Stream::commitChanges()
{
    uno::Reference< embed::XTransactedObject >
        xParentTA( getParentStorage(), uno::UNO_QUERY );
    OSL_ENSURE( xParentTA.is(), "No XTransactedObject interface!" );

    if ( xParentTA.is() )
    {
        try
        {
            xParentTA->commit();
        }
        catch ( lang::WrappedTargetException const & )
        {
            throw io::IOException(); // @@@
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
