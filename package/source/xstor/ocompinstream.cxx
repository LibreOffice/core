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


#include "ocompinstream.hxx"
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <osl/diagnose.h>

#include "owriteablestream.hxx"
#include "xstorage.hxx"

using namespace ::com::sun::star;

//-----------------------------------------------
OInputCompStream::OInputCompStream( OWriteStream_Impl& aImpl,
                                    uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: m_pImpl( &aImpl )
, m_rMutexRef( m_pImpl->m_rMutexRef )
, m_xStream( xStream )
, m_pInterfaceContainer( NULL )
, m_aProperties( aProps )
, m_bDisposed( sal_False )
, m_nStorageType( nStorageType )
{
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex is provided!\n" );
    if ( !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );
}

//-----------------------------------------------
OInputCompStream::OInputCompStream( uno::Reference < io::XInputStream > xStream,
                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                    sal_Int32 nStorageType )
: m_pImpl( NULL )
, m_rMutexRef( new SotMutexHolder )
, m_xStream( xStream )
, m_pInterfaceContainer( NULL )
, m_aProperties( aProps )
, m_bDisposed( sal_False )
, m_nStorageType( nStorageType )
{
    OSL_ENSURE( xStream.is(), "No stream is provided!\n" );
}

//-----------------------------------------------
OInputCompStream::~OInputCompStream()
{
    {
        ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

        if ( !m_bDisposed )
        {
            m_refCount++;
            dispose();
        }

        if ( m_pInterfaceContainer )
            delete m_pInterfaceContainer;
    }
}

//-----------------------------------------------
uno::Any SAL_CALL OInputCompStream::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    // common interfaces
    aReturn <<= ::cppu::queryInterface
                (   rType
                    ,   static_cast<io::XInputStream*> ( this )
                    ,   static_cast<io::XStream*> ( this )
                    ,   static_cast<lang::XComponent*> ( this )
                    ,   static_cast<beans::XPropertySet*> ( this )
                    ,   static_cast<embed::XExtendedStorageStream*> ( this ) );

    if ( aReturn.hasValue() == sal_True )
        return aReturn ;

    if ( m_nStorageType == embed::StorageFormats::OFOPXML )
    {
        aReturn <<= ::cppu::queryInterface
                    (   rType
                        ,   static_cast<embed::XRelationshipAccess*> ( this ) );

        if ( aReturn.hasValue() == sal_True )
            return aReturn ;
    }

    return OWeakObject::queryInterface( rType );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xStream.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No stream!" );
        throw uno::RuntimeException();
    }

    return m_xStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xStream.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No stream!" );
        throw uno::RuntimeException();
    }

    return m_xStream->readSomeBytes( aData, nMaxBytesToRead );

}

//-----------------------------------------------
void SAL_CALL OInputCompStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xStream.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No stream!" );
        throw uno::RuntimeException();
    }

    m_xStream->skipBytes( nBytesToSkip );

}

//-----------------------------------------------
sal_Int32 SAL_CALL OInputCompStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xStream.is() )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "No stream!" );
        throw uno::RuntimeException();
    }

    return m_xStream->available();

}

//-----------------------------------------------
void SAL_CALL OInputCompStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    dispose();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OInputCompStream::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_xStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OInputCompStream::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    return uno::Reference< io::XOutputStream >();
}

//-----------------------------------------------
void OInputCompStream::InternalDispose()
{
    // can be called only by OWriteStream_Impl
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    // the source object is also a kind of locker for the current object
    // since the listeners could dispose the object while being notified
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->disposeAndClear( aSource );

    try
    {
        m_xStream->closeInput();
    }
    catch( uno::Exception& )
    {}

    m_pImpl = NULL;
    m_bDisposed = sal_True;
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::dispose(  )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aSource( static_cast< ::cppu::OWeakObject*>( this ) );
        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_xStream->closeInput();

    if ( m_pImpl )
    {
        m_pImpl->InputStreamDisposed( this );
        m_pImpl = NULL;
    }

    m_bDisposed = sal_True;
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OInterfaceContainerHelper( m_rMutexRef->GetMutex() );

    m_pInterfaceContainer->addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( xListener );
}

//-----------------------------------------------
sal_Bool SAL_CALL OInputCompStream::hasByID(  const ::rtl::OUString& sID )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    try
    {
        getRelationshipByID( sID );
        return sal_True;
    }
    catch( container::NoSuchElementException& )
    {}

    return sal_False;
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OInputCompStream::getTargetByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Target" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
::rtl::OUString SAL_CALL OInputCompStream::getTypeByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< beans::StringPair > aSeq = getRelationshipByID( sID );
    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( aSeq[nInd].First == "Type" )
            return aSeq[nInd].Second;

    return ::rtl::OUString();
}

//-----------------------------------------------
uno::Sequence< beans::StringPair > SAL_CALL OInputCompStream::getRelationshipByID(  const ::rtl::OUString& sID  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Id" )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sID ) )
                    return aSeq[nInd1];
                break;
            }

    throw container::NoSuchElementException();
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OInputCompStream::getRelationshipsByType(  const ::rtl::OUString& sType  )
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    uno::Sequence< uno::Sequence< beans::StringPair > > aResult;
    sal_Int32 nEntriesNum = 0;

    // TODO/LATER: in future the unification of the ID could be checked
    uno::Sequence< uno::Sequence< beans::StringPair > > aSeq = getAllRelationships();
    for ( sal_Int32 nInd1 = 0; nInd1 < aSeq.getLength(); nInd1++ )
        for ( sal_Int32 nInd2 = 0; nInd2 < aSeq[nInd1].getLength(); nInd2++ )
            if ( aSeq[nInd1][nInd2].First == "Type" )
            {
                if ( aSeq[nInd1][nInd2].Second.equals( sType ) )
                {
                    aResult.realloc( nEntriesNum );
                    aResult[nEntriesNum-1] = aSeq[nInd1];
                }
                break;
            }

    return aResult;
}

//-----------------------------------------------
uno::Sequence< uno::Sequence< beans::StringPair > > SAL_CALL OInputCompStream::getAllRelationships()
        throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    // TODO/LATER: in future the information could be taken directly from m_pImpl when possible
    uno::Sequence< uno::Sequence< beans::StringPair > > aResult;
    for ( sal_Int32 aInd = 0; aInd < m_aProperties.getLength(); aInd++ )
        if ( m_aProperties[aInd].Name == "RelationsInfo" )
        {
            if ( m_aProperties[aInd].Value >>= aResult )
                return aResult;

            break;
        }

    throw io::IOException(); // the relations info could not be read
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::insertRelationshipByID(  const ::rtl::OUString& /*sID*/, const uno::Sequence< beans::StringPair >& /*aEntry*/, ::sal_Bool /*bReplace*/  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::removeRelationshipByID(  const ::rtl::OUString& /*sID*/  )
        throw ( container::NoSuchElementException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::insertRelationships(  const uno::Sequence< uno::Sequence< beans::StringPair > >& /*aEntries*/, ::sal_Bool /*bReplace*/  )
        throw ( container::ElementExistException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::clearRelationships()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    if ( m_nStorageType != embed::StorageFormats::OFOPXML )
        throw uno::RuntimeException();

    throw io::IOException(); // TODO: Access denied
}

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OInputCompStream::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

//-----------------------------------------------
void SAL_CALL OInputCompStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& /*aValue*/ )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    // all the provided properties are accessible
    for ( sal_Int32 aInd = 0; aInd < m_aProperties.getLength(); aInd++ )
    {
        if ( m_aProperties[aInd].Name.equals( aPropertyName ) )
        {
            throw beans::PropertyVetoException(); // TODO
        }
    }

    throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
uno::Any SAL_CALL OInputCompStream::getPropertyValue( const ::rtl::OUString& aProp )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    ::rtl::OUString aPropertyName;
    if ( aProp == "IsEncrypted" )
        aPropertyName = "Encrypted";
    else
        aPropertyName = aProp;

    if ( aPropertyName == "RelationsInfo" )
        throw beans::UnknownPropertyException(); // TODO

    // all the provided properties are accessible
    for ( sal_Int32 aInd = 0; aInd < m_aProperties.getLength(); aInd++ )
    {
        if ( m_aProperties[aInd].Name.equals( aPropertyName ) )
        {
            return m_aProperties[aInd].Value;
        }
    }

    throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::addPropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::removePropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::addVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OInputCompStream::removeVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    if ( m_bDisposed )
    {
        ::package::StaticAddLog( OSL_LOG_PREFIX "Disposed!" );
        throw lang::DisposedException();
    }

    //TODO:
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
