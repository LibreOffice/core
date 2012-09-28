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

#include <osl/diagnose.h>

#include <com/sun/star/io/TempFile.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <switchpersistencestream.hxx>

using namespace ::com::sun::star;

// ========================================================================
struct SPStreamData_Impl
{
    uno::Reference< lang::XMultiServiceFactory > m_xFactory;

    sal_Bool m_bInStreamBased;

    // the streams below are not visible from outside so there is no need to remember position

    // original stream related members
    uno::Reference< io::XStream > m_xOrigStream;
    uno::Reference< io::XTruncate > m_xOrigTruncate;
    uno::Reference< io::XSeekable > m_xOrigSeekable;
    uno::Reference< io::XInputStream > m_xOrigInStream;
    uno::Reference< io::XOutputStream > m_xOrigOutStream;

    sal_Bool m_bInOpen;
    sal_Bool m_bOutOpen;


    SPStreamData_Impl(
            const uno::Reference< lang::XMultiServiceFactory >& xFactory,
            sal_Bool bInStreamBased,
            const uno::Reference< io::XStream >& xOrigStream,
            const uno::Reference< io::XTruncate >& xOrigTruncate,
            const uno::Reference< io::XSeekable >& xOrigSeekable,
            const uno::Reference< io::XInputStream >& xOrigInStream,
            const uno::Reference< io::XOutputStream >& xOrigOutStream,
            sal_Bool bInOpen,
            sal_Bool bOutOpen )
    : m_xFactory( xFactory )
    , m_bInStreamBased( bInStreamBased )
    , m_xOrigStream( xOrigStream )
    , m_xOrigTruncate( xOrigTruncate )
    , m_xOrigSeekable( xOrigSeekable )
    , m_xOrigInStream( xOrigInStream )
    , m_xOrigOutStream( xOrigOutStream )
    , m_bInOpen( bInOpen )
    , m_bOutOpen( bOutOpen )
    {
    }
};

// ========================================================================
// ------------------------------------------------------------------------
SwitchablePersistenceStream::SwitchablePersistenceStream(
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        const uno::Reference< io::XStream >& xStream )
: m_xFactory( xFactory )
, m_pStreamData( NULL )
{
    SwitchPersistenceTo( xStream );
}

// ------------------------------------------------------------------------
SwitchablePersistenceStream::SwitchablePersistenceStream(
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        const uno::Reference< io::XInputStream >& xInputStream )
: m_xFactory( xFactory )
, m_pStreamData( NULL )
{
    SwitchPersistenceTo( xInputStream );
}

// ------------------------------------------------------------------------
SwitchablePersistenceStream::~SwitchablePersistenceStream()
{
    CloseAll_Impl();
}

// ------------------------------------------------------------------------
void SwitchablePersistenceStream::SwitchPersistenceTo( const uno::Reference< io::XStream >& xStream )
{
    uno::Reference< io::XTruncate > xNewTruncate( xStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XSeekable > xNewSeekable( xStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > xNewInStream = xStream->getInputStream();
    uno::Reference< io::XOutputStream > xNewOutStream = xStream->getOutputStream();
    if ( !xNewInStream.is() || !xNewOutStream.is() )
        throw uno::RuntimeException();

    sal_Int64 nPos = 0;
    sal_Bool bInOpen = sal_False;
    sal_Bool bOutOpen = sal_False;

    if ( m_pStreamData && m_pStreamData->m_xOrigSeekable.is() )
    {
        // check that the length is the same
        if ( m_pStreamData->m_xOrigSeekable->getLength() != xNewSeekable->getLength() )
            throw uno::RuntimeException();

        // get the current position
        nPos = m_pStreamData->m_xOrigSeekable->getPosition();
        bInOpen = m_pStreamData->m_bInOpen;
        bOutOpen = m_pStreamData->m_bOutOpen;
    }

    xNewSeekable->seek( nPos );

    CloseAll_Impl();

    m_pStreamData = new SPStreamData_Impl( m_xFactory, sal_False,
                                            xStream, xNewTruncate, xNewSeekable, xNewInStream, xNewOutStream,
                                            bInOpen, bOutOpen );
}

// ------------------------------------------------------------------------
void SwitchablePersistenceStream::SwitchPersistenceTo( const uno::Reference< io::XInputStream >& xInputStream )
{
    uno::Reference< io::XStream > xNewStream;
    uno::Reference< io::XTruncate > xNewTruncate;
    uno::Reference< io::XSeekable > xNewSeekable( xInputStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XOutputStream > xNewOutStream;
    if ( !xInputStream.is() )
        throw uno::RuntimeException();

    sal_Int64 nPos = 0;
    sal_Bool bInOpen = sal_False;
    sal_Bool bOutOpen = sal_False;

    if ( m_pStreamData && m_pStreamData->m_xOrigSeekable.is() )
    {
        // check that the length is the same
        if ( m_pStreamData->m_xOrigSeekable->getLength() != xNewSeekable->getLength() )
            throw uno::RuntimeException();

        // get the current position
        nPos = m_pStreamData->m_xOrigSeekable->getPosition();
        bInOpen = m_pStreamData->m_bInOpen;
        bOutOpen = m_pStreamData->m_bOutOpen;
    }

    xNewSeekable->seek( nPos );

    CloseAll_Impl();

    m_pStreamData = new SPStreamData_Impl( m_xFactory, sal_True,
                                            xNewStream, xNewTruncate, xNewSeekable, xInputStream, xNewOutStream,
                                            bInOpen, bOutOpen );

}

// ------------------------------------------------------------------------
void SwitchablePersistenceStream::CopyAndSwitchPersistenceTo( const uno::Reference< io::XStream >& xStream )
{
    uno::Reference< io::XStream > xTargetStream = xStream;
    uno::Reference< io::XSeekable > xTargetSeek;

    if ( !xTargetStream.is() )
    {
        xTargetStream = uno::Reference < io::XStream >(
            io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
            uno::UNO_QUERY_THROW );

        xTargetSeek = uno::Reference< io::XSeekable >( xTargetStream, uno::UNO_QUERY_THROW );
    }
    else
    {
        // the provided stream must be empty
        xTargetSeek = uno::Reference< io::XSeekable >( xTargetStream, uno::UNO_QUERY_THROW );
        if ( xTargetSeek->getLength() )
            throw io::IOException();
    }

    uno::Reference< io::XTruncate > xTargetTruncate( xTargetStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > xTargetInStream = xTargetStream->getInputStream();
    uno::Reference< io::XOutputStream > xTargetOutStream = xTargetStream->getOutputStream();
    if ( !xTargetInStream.is() || !xTargetOutStream.is() )
        throw uno::RuntimeException();

    if ( !m_pStreamData->m_xOrigInStream.is() || !m_pStreamData->m_xOrigSeekable.is() )
        throw uno::RuntimeException();

    sal_Int64 nPos = m_pStreamData->m_xOrigSeekable->getPosition();
    m_pStreamData->m_xOrigSeekable->seek( 0 );
    ::comphelper::OStorageHelper::CopyInputToOutput( m_pStreamData->m_xOrigInStream, xTargetOutStream );
    xTargetOutStream->flush();
    xTargetSeek->seek( nPos );

    sal_Bool bInOpen = m_pStreamData->m_bInOpen;
    sal_Bool bOutOpen = m_pStreamData->m_bOutOpen;

    CloseAll_Impl();

    m_pStreamData = new SPStreamData_Impl( m_xFactory, sal_False,
                                        xTargetStream, xTargetTruncate, xTargetSeek, xTargetInStream, xTargetOutStream,
                                        bInOpen, bOutOpen );
}

// ------------------------------------------------------------------------
void SwitchablePersistenceStream::CloseAll_Impl()
{
    if ( m_pStreamData )
    {
        delete m_pStreamData;
        m_pStreamData = NULL;
    }
}

// com::sun::star::io::XStream
// ------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL SwitchablePersistenceStream::getInputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bInOpen = sal_True;
    return static_cast< io::XInputStream* >( this );
}


// ------------------------------------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL SwitchablePersistenceStream::getOutputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bOutOpen = sal_True;
    return static_cast< io::XOutputStream* >( this );
}



// com::sun::star::io::XInputStream
// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL SwitchablePersistenceStream::readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigInStream.is() )
        throw uno::RuntimeException();

    return m_pStreamData->m_xOrigInStream->readBytes( aData, nBytesToRead );
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL SwitchablePersistenceStream::readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigInStream.is() )
        throw uno::RuntimeException();

    return m_pStreamData->m_xOrigInStream->readBytes( aData, nMaxBytesToRead );
}

// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::skipBytes( ::sal_Int32 nBytesToSkip )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigInStream.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigInStream->skipBytes( nBytesToSkip );
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL SwitchablePersistenceStream::available(  )
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigInStream.is() )
        throw uno::RuntimeException();

    return m_pStreamData->m_xOrigInStream->available();
}


// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::closeInput()
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    m_pStreamData->m_bInOpen = sal_False;
    if ( !m_pStreamData->m_bOutOpen )
        CloseAll_Impl();
}



// com::sun::star::io::XOutputStream
// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::writeBytes( const uno::Sequence< ::sal_Int8 >& aData )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bInStreamBased )
        throw io::IOException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigOutStream.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigOutStream->writeBytes( aData );
}


// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::flush(  )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData || m_pStreamData->m_bInStreamBased )
    {
        OSL_FAIL( "flush() is not acceptable!\n" );
        return;
        // in future throw exception, for now some code might call flush() on closed stream
        // since file ucp implementation allows it
        // throw io::NotConnectedException();
    }

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigOutStream.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigOutStream->flush();
}


// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::closeOutput(  )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    m_pStreamData->m_bOutOpen = sal_False;
    if ( !m_pStreamData->m_bInOpen )
        CloseAll_Impl();
}



// com::sun::star::io::XTruncate
// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::truncate(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bInStreamBased )
        throw io::IOException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigTruncate.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigTruncate->truncate();
}


// com::sun::star::io::XSeekable
// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::seek( ::sal_Int64 location )
    throw (lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigSeekable.is() )
        throw uno::RuntimeException();

    m_pStreamData->m_xOrigSeekable->seek( location );
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL SwitchablePersistenceStream::getPosition(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigSeekable.is() )
        throw uno::RuntimeException();

    return m_pStreamData->m_xOrigSeekable->getPosition();
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL SwitchablePersistenceStream::getLength(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    // the original stream data should be provided
    if ( !m_pStreamData->m_xOrigSeekable.is() )
        throw uno::RuntimeException();

    return m_pStreamData->m_xOrigSeekable->getLength();
}

// ------------------------------------------------------------------------
void SAL_CALL SwitchablePersistenceStream::waitForCompletion()
    throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    if ( !m_pStreamData )
        throw io::NotConnectedException();

    uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( m_pStreamData->m_xOrigOutStream, uno::UNO_QUERY );
    if ( asyncOutputMonitor.is() )
        asyncOutputMonitor->waitForCompletion();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
