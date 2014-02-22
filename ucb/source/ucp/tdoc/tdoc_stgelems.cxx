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

 - remove root storage access workaround

 *************************************************************************/

#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/reflection/ProxyFactory.hpp"

#include "tdoc_uri.hxx"

#include "tdoc_stgelems.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;



//

//



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



//

//



Storage::Storage( const uno::Reference< uno::XComponentContext > & rxContext,
                  const rtl::Reference< StorageElementFactory > & xFactory,
                  const OUString & rUri,
                  const uno::Reference< embed::XStorage > & xParentStorage,
                  const uno::Reference< embed::XStorage > & xStorageToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xFactory( xFactory ),
  m_xWrappedStorage( xStorageToWrap ),
  m_xWrappedTransObj( xStorageToWrap, uno::UNO_QUERY ), 
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
            
            
            
            
            

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}



Storage::~Storage()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );

    
    if ( !isDocumentStorage() )
    {
        if ( m_xWrappedComponent.is() )
        {
            
            try
            {
                m_xWrappedComponent->dispose();
            }
            catch ( lang::DisposedException const & )
            {
                
            }
            catch ( ... )
            {
                OSL_FAIL( "Storage::~Storage - Caught exception!" );
            }
        }
    }
}


//

//



uno::Any SAL_CALL Storage::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException )
{
    
    uno::Any aRet = StorageUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    
    return m_xAggProxy->queryAggregation( aType );
}



void SAL_CALL Storage::acquire()
    throw ()
{
    osl_atomic_increment( &m_refCount );
}



void SAL_CALL Storage::release()
    throw ()
{
    
    

    if ( m_refCount == 1 )
        m_xFactory->releaseElement( this );

    
    OWeakObject::release();
}


//

//



uno::Sequence< uno::Type > SAL_CALL Storage::getTypes()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getTypes();
}



uno::Sequence< sal_Int8 > SAL_CALL Storage::getImplementationId()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getImplementationId();
}


//

//


void SAL_CALL Storage::dispose()
    throw ( uno::RuntimeException )
{
    m_xWrappedStorage->dispose();
}



void SAL_CALL Storage::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    m_xWrappedStorage->addEventListener( xListener );
}


void SAL_CALL Storage::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_xWrappedStorage->removeEventListener( aListener );
}


//

//



uno::Type SAL_CALL Storage::getElementType()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->getElementType();
}



::sal_Bool SAL_CALL Storage::hasElements()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->hasElements();
}


//

//



uno::Any SAL_CALL Storage::getByName( const OUString& aName )
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->getByName( aName );
}



uno::Sequence< OUString > SAL_CALL Storage::getElementNames()
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->getElementNames();
}



::sal_Bool SAL_CALL Storage::hasByName( const OUString& aName )
    throw ( uno::RuntimeException )
{
    return m_xWrappedStorage->hasByName( aName );
}


//

//



void SAL_CALL Storage::copyToStorage(
        const uno::Reference< embed::XStorage >& xDest )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->copyToStorage( xDest );
}



uno::Reference< io::XStream > SAL_CALL Storage::openStreamElement(
        const OUString& aStreamName, sal_Int32 nOpenMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openStreamElement( aStreamName, nOpenMode );
}



uno::Reference< io::XStream > SAL_CALL Storage::openEncryptedStreamElement(
        const OUString& aStreamName,
        sal_Int32 nOpenMode,
        const OUString& aPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::NoEncryptionException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openEncryptedStreamElement(
        aStreamName, nOpenMode, aPassword );
}



uno::Reference< embed::XStorage > SAL_CALL Storage::openStorageElement(
        const OUString& aStorName, sal_Int32 nOpenMode )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->openStorageElement( aStorName, nOpenMode );
}



uno::Reference< io::XStream > SAL_CALL Storage::cloneStreamElement(
        const OUString& aStreamName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->cloneStreamElement( aStreamName );
}



uno::Reference< io::XStream > SAL_CALL Storage::cloneEncryptedStreamElement(
        const OUString& aStreamName,
        const OUString& aPassword )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            packages::NoEncryptionException,
            packages::WrongPasswordException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->cloneEncryptedStreamElement( aStreamName,
                                                           aPassword );
}



void SAL_CALL Storage::copyLastCommitTo(
        const uno::Reference< embed::XStorage >& xTargetStorage )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException)
{
    m_xWrappedStorage->copyLastCommitTo( xTargetStorage );
}



void SAL_CALL Storage::copyStorageElementLastCommitTo(
        const OUString& aStorName,
        const uno::Reference< embed::XStorage >& xTargetStorage )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException)
{
    m_xWrappedStorage->copyStorageElementLastCommitTo( aStorName, xTargetStorage );
}



sal_Bool SAL_CALL Storage::isStreamElement(
        const OUString& aElementName )
    throw ( container::NoSuchElementException,
            lang::IllegalArgumentException,
            embed::InvalidStorageException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->isStreamElement( aElementName );
}



sal_Bool SAL_CALL Storage::isStorageElement(
        const OUString& aElementName )
    throw ( container::NoSuchElementException,
            lang::IllegalArgumentException,
            embed::InvalidStorageException,
            uno::RuntimeException )
{
    return m_xWrappedStorage->isStorageElement( aElementName );
}



void SAL_CALL Storage::removeElement( const OUString& aElementName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->removeElement( aElementName );
}



void SAL_CALL Storage::renameElement( const OUString& aEleName,
                                      const OUString& aNewName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->renameElement( aEleName, aNewName );
}



void SAL_CALL Storage::copyElementTo(
        const OUString& aElementName,
        const uno::Reference< embed::XStorage >& xDest,
        const OUString& aNewName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->copyElementTo( aElementName, xDest, aNewName );
}



void SAL_CALL Storage::moveElementTo(
        const OUString& aElementName,
        const uno::Reference< embed::XStorage >& xDest,
        const OUString& rNewName )
    throw ( embed::InvalidStorageException,
            lang::IllegalArgumentException,
            container::NoSuchElementException,
            container::ElementExistException,
            io::IOException,
            embed::StorageWrappedTargetException,
            uno::RuntimeException )
{
    m_xWrappedStorage->moveElementTo( aElementName, xDest, rNewName );
}


//

//



void SAL_CALL Storage::commit()
    throw ( io::IOException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    
    

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



void SAL_CALL Storage::revert()
    throw ( io::IOException,
            lang::WrappedTargetException,
            uno::RuntimeException )
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



//

//



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
            
            
            
            
            

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}



OutputStream::~OutputStream()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
}


//

//



uno::Any SAL_CALL OutputStream::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet = OutputStreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xAggProxy.is() )
        return m_xAggProxy->queryAggregation( aType );
    else
        return uno::Any();
}


//

//



uno::Sequence< uno::Type > SAL_CALL OutputStream::getTypes()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getTypes();
}



uno::Sequence< sal_Int8 > SAL_CALL OutputStream::getImplementationId()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getImplementationId();
}


//

//



void SAL_CALL
OutputStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->writeBytes( aData );
}



void SAL_CALL
OutputStream::flush()
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->flush();
}



void SAL_CALL
OutputStream::closeOutput(  )
    throw ( io::NotConnectedException,
            io::BufferSizeExceededException,
            io::IOException,
            uno::RuntimeException )
{
    m_xWrappedStream->closeOutput();

    
    
    setParentStorage( uno::Reference< embed::XStorage >() );
}


//

//



void SAL_CALL
OutputStream::dispose()
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->dispose();

    
    
    setParentStorage( uno::Reference< embed::XStorage >() );
}



void SAL_CALL
OutputStream::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->addEventListener( xListener );
}



void SAL_CALL
OutputStream::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->removeEventListener( aListener );
}



//

//



Stream::Stream(
            const uno::Reference< uno::XComponentContext > & rxContext,
            const OUString & rUri,
            const uno::Reference< embed::XStorage >  & xParentStorage,
            const uno::Reference< io::XStream > & xStreamToWrap )
: ParentStorageHolder( xParentStorage, Uri( rUri ).getParentUri() ),
  m_xWrappedStream( xStreamToWrap ),
  m_xWrappedOutputStream( xStreamToWrap->getOutputStream() ), 
  m_xWrappedTruncate( m_xWrappedOutputStream, uno::UNO_QUERY ), 
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
            
            
            
            
            

            m_xAggProxy->setDelegator(
                static_cast< cppu::OWeakObject * >( this ) );
        }
        osl_atomic_decrement( &m_refCount );
    }
}



Stream::~Stream()
{
    if ( m_xAggProxy.is() )
        m_xAggProxy->setDelegator( uno::Reference< uno::XInterface >() );
}


//

//



uno::Any SAL_CALL Stream::queryInterface( const uno::Type& aType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet = StreamUNOBase::queryInterface( aType );

    if ( aRet.hasValue() )
        return aRet;

    if ( m_xAggProxy.is() )
        return m_xAggProxy->queryAggregation( aType );
    else
        return uno::Any();
}


//

//



uno::Sequence< uno::Type > SAL_CALL Stream::getTypes()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getTypes();
}



uno::Sequence< sal_Int8 > SAL_CALL Stream::getImplementationId()
    throw ( uno::RuntimeException )
{
    return m_xWrappedTypeProv->getImplementationId();
}


//

//



uno::Reference< io::XInputStream > SAL_CALL Stream::getInputStream()
    throw( uno::RuntimeException )
{
    return uno::Reference< io::XInputStream >( this );
}



uno::Reference< io::XOutputStream > SAL_CALL Stream::getOutputStream()
    throw( uno::RuntimeException )
{
    return uno::Reference< io::XOutputStream >( this );
}


//

//



void SAL_CALL Stream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->writeBytes( aData );
        commitChanges();
    }
}



void SAL_CALL Stream::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->flush();
        commitChanges();
    }
}



void SAL_CALL Stream::closeOutput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if ( m_xWrappedOutputStream.is() )
    {
        m_xWrappedOutputStream->closeOutput();
        commitChanges();
    }

    
    
    setParentStorage( uno::Reference< embed::XStorage >() );
}


//

//



void SAL_CALL Stream::truncate()
    throw( io::IOException,
           uno::RuntimeException )
{
    if ( m_xWrappedTruncate.is() )
    {
        m_xWrappedTruncate->truncate();
        commitChanges();
    }
}


//

//



sal_Int32 SAL_CALL Stream::readBytes( uno::Sequence< sal_Int8 >& aData,
                                      sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    return m_xWrappedInputStream->readBytes( aData, nBytesToRead );
}



sal_Int32 SAL_CALL Stream::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                                          sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    return m_xWrappedInputStream->readSomeBytes( aData, nMaxBytesToRead );
}



void SAL_CALL Stream::skipBytes( sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    m_xWrappedInputStream->skipBytes( nBytesToSkip );
}



sal_Int32 SAL_CALL Stream::available()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    return m_xWrappedInputStream->available();
}



void SAL_CALL Stream::closeInput()
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    m_xWrappedInputStream->closeInput();
}


//

//



void SAL_CALL Stream::dispose()
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->dispose();

    
    
    setParentStorage( uno::Reference< embed::XStorage >() );
}



void SAL_CALL Stream::addEventListener(
        const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->addEventListener( xListener );
}



void SAL_CALL Stream::removeEventListener(
        const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
    m_xWrappedComponent->removeEventListener( aListener );
}


//

//


void Stream::commitChanges()
    throw( io::IOException )
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
            throw io::IOException(); 
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
