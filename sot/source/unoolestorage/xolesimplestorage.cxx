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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/TempFile.hpp>

#include <comphelper/storagehelper.hxx>
#include <comphelper/componentcontext.hxx>

#include <unotools/ucbstreamhelper.hxx>

#include <cppuhelper/exc_hlp.hxx>

#include <sot/storinfo.hxx>

#include "xolesimplestorage.hxx"


using namespace ::com::sun::star;

const sal_Int32 nBytesCount = 32000;


// --------------------------------------------------------------------------------
OLESimpleStorage::OLESimpleStorage( uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_bDisposed( sal_False )
, m_pStream( NULL )
, m_pStorage( NULL )
, m_pListenersContainer( NULL )
, m_xFactory( xFactory )
, m_bNoTemporaryCopy( sal_False )
{
    OSL_ENSURE( m_xFactory.is(), "No factory is provided on creation!\n" );
    if ( !m_xFactory.is() )
        throw uno::RuntimeException();
}

// --------------------------------------------------------------------------------
OLESimpleStorage::~OLESimpleStorage()
{
    try {
        m_refCount++;
        dispose();
    } catch( uno::Exception& )
    {}

    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OLESimpleStorage::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString("com.sun.star.embed.OLESimpleStorage");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OLESimpleStorage::impl_staticGetImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.embed.OLESimpleStorage");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OLESimpleStorage::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OLESimpleStorage( xServiceManager ) );
}

//-------------------------------------------------------------------------
void OLESimpleStorage::UpdateOriginal_Impl()
{
    if ( !m_bNoTemporaryCopy )
    {
        uno::Reference< io::XSeekable > xSeek( m_xStream, uno::UNO_QUERY_THROW );
        xSeek->seek( 0 );

        uno::Reference< io::XSeekable > xTempSeek( m_xTempStream, uno::UNO_QUERY_THROW );
        sal_Int64 nPos = xTempSeek->getPosition();
        xTempSeek->seek( 0 );

        uno::Reference< io::XInputStream > xTempInp = m_xTempStream->getInputStream();
        uno::Reference< io::XOutputStream > xOutputStream = m_xStream->getOutputStream();
        if ( !xTempInp.is() || !xOutputStream.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XTruncate > xTrunc( xOutputStream, uno::UNO_QUERY_THROW );
        xTrunc->truncate();

        ::comphelper::OStorageHelper::CopyInputToOutput( xTempInp, xOutputStream );
        xOutputStream->flush();
        xTempSeek->seek( nPos );
    }
}

//-------------------------------------------------------------------------
void OLESimpleStorage::InsertInputStreamToStorage_Impl( BaseStorage* pStorage, ::rtl::OUString aName, const uno::Reference< io::XInputStream >& xInputStream )
    throw ( uno::Exception )
{
    if ( !pStorage || aName.isEmpty() || !xInputStream.is() )
        throw uno::RuntimeException();

    if ( pStorage->IsContained( aName ) )
        throw container::ElementExistException(); // TODO:

    BaseStorageStream* pNewStream = pStorage->OpenStream( aName );
    if ( !pNewStream || pNewStream->GetError() || pStorage->GetError() )
    {
        if ( pNewStream )
            DELETEZ( pNewStream );
        pStorage->ResetError();
        throw io::IOException(); // TODO
    }

    try
    {
        uno::Sequence< sal_Int8 > aData( nBytesCount );
        sal_Int32 nRead = 0;
        do
        {
            nRead = xInputStream->readBytes( aData, nBytesCount );
            if ( nRead < nBytesCount )
                aData.realloc( nRead );

            sal_Int32 nWritten = pNewStream->Write( aData.getArray(), nRead );
            if ( nWritten < nRead )
                throw io::IOException();
        } while( nRead == nBytesCount );
    }
    catch( uno::Exception& )
    {
        DELETEZ( pNewStream );
        pStorage->Remove( aName );

        throw;
    }

    DELETEZ( pNewStream );
}

//-------------------------------------------------------------------------
void OLESimpleStorage::InsertNameAccessToStorage_Impl( BaseStorage* pStorage, ::rtl::OUString aName, const uno::Reference< container::XNameAccess >& xNameAccess )
    throw ( uno::Exception )
{
    if ( !pStorage || aName.isEmpty() || !xNameAccess.is() )
        throw uno::RuntimeException();

    if ( pStorage->IsContained( aName ) )
        throw container::ElementExistException(); // TODO:

    BaseStorage* pNewStorage = pStorage->OpenStorage( aName );
    if ( !pNewStorage || pNewStorage->GetError() || pStorage->GetError() )
    {
        if ( pNewStorage )
            DELETEZ( pNewStorage );
        pStorage->ResetError();
        throw io::IOException(); // TODO
    }

    try
    {
        uno::Sequence< ::rtl::OUString > aElements = xNameAccess->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aElements.getLength(); nInd++ )
        {
            uno::Reference< io::XInputStream > xInputStream;
            uno::Reference< container::XNameAccess > xSubNameAccess;
            uno::Any aAny = xNameAccess->getByName( aElements[nInd] );
            if ( aAny >>= xInputStream )
                InsertInputStreamToStorage_Impl( pNewStorage, aElements[nInd], xInputStream );
            else if ( aAny >>= xSubNameAccess )
                InsertNameAccessToStorage_Impl( pNewStorage, aElements[nInd], xSubNameAccess );
        }
    }
    catch( uno::Exception& )
    {
        DELETEZ( pNewStorage );
        pStorage->Remove( aName );

        throw;
    }

    DELETEZ( pNewStorage );
}

//____________________________________________________________________________________________________
//  XInitialization
//____________________________________________________________________________________________________

void SAL_CALL OLESimpleStorage::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    if ( m_pStream || m_pStorage )
        throw io::IOException(); // TODO: already initilized

    sal_Int32 nArgNum = aArguments.getLength();
    OSL_ENSURE( nArgNum >= 1 && nArgNum <= 2, "Wrong parameter number" );

    if ( nArgNum < 1 || nArgNum > 2 )
        throw lang::IllegalArgumentException(); // TODO:

    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XInputStream > xInputStream;
    if ( !( aArguments[0] >>= xStream ) && !( aArguments[0] >>= xInputStream ) )
        throw lang::IllegalArgumentException(); // TODO:

    if ( nArgNum == 2 )
    {
        if ( !( aArguments[1] >>= m_bNoTemporaryCopy ) )
            throw lang::IllegalArgumentException(); // TODO:
    }

    if ( m_bNoTemporaryCopy )
    {
        // TODO: ???
        // If the temporary stream is not created, the original stream must be wrapped
        // since SvStream wrapper closes the stream is owns
        if ( xInputStream.is() )
        {
            // the stream must be seekable for direct access
            uno::Reference< io::XSeekable > xSeek( xInputStream, uno::UNO_QUERY_THROW );
            m_pStream = ::utl::UcbStreamHelper::CreateStream( xInputStream, sal_False );
        }
        else if ( xStream.is() )
        {
            // the stream must be seekable for direct access
            uno::Reference< io::XSeekable > xSeek( xStream, uno::UNO_QUERY_THROW );
            m_pStream = ::utl::UcbStreamHelper::CreateStream( xStream, sal_False );
        }
        else
            throw lang::IllegalArgumentException(); // TODO:
    }
    else
    {
        uno::Reference < io::XStream > xTempFile(
                io::TempFile::create(comphelper::ComponentContext(m_xFactory).getUNOContext()),
                uno::UNO_QUERY_THROW );
        uno::Reference < io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY_THROW );
        uno::Reference< io::XOutputStream > xTempOut = xTempFile->getOutputStream();
        if ( !xTempOut.is() )
            throw uno::RuntimeException();

        if ( xInputStream.is() )
        {
            try
            {
                uno::Reference< io::XSeekable > xSeek( xInputStream, uno::UNO_QUERY_THROW );
                xSeek->seek( 0 );
            }
            catch( uno::Exception& )
            {}

            ::comphelper::OStorageHelper::CopyInputToOutput( xInputStream, xTempOut );
            xTempOut->closeOutput();
            xTempSeek->seek( 0 );
            uno::Reference< io::XInputStream > xTempInput = xTempFile->getInputStream();
            m_pStream = ::utl::UcbStreamHelper::CreateStream( xTempInput, sal_False );
        }
        else if ( xStream.is() )
        {
            // not sure that the storage flashes the stream on commit
            m_xStream = xStream;
            m_xTempStream = xTempFile;

            uno::Reference< io::XSeekable > xSeek( xStream, uno::UNO_QUERY_THROW );
            xSeek->seek( 0 );
            uno::Reference< io::XInputStream > xInpStream = xStream->getInputStream();
            if ( !xInpStream.is() || !xStream->getOutputStream().is() )
                throw uno::RuntimeException();

            ::comphelper::OStorageHelper::CopyInputToOutput( xInpStream, xTempOut );
            xTempOut->flush();
            xTempSeek->seek( 0 );

            m_pStream = ::utl::UcbStreamHelper::CreateStream( xTempFile, sal_False );
        }
        else
            throw lang::IllegalArgumentException(); // TODO:
    }

    if ( !m_pStream || m_pStream->GetError() )
        throw io::IOException(); // TODO

    m_pStorage = new Storage( *m_pStream, sal_False );
}


//____________________________________________________________________________________________________
//  XNameContainer
//____________________________________________________________________________________________________

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw ( lang::IllegalArgumentException,
                container::ElementExistException,
                lang::WrappedTargetException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

      if ( !m_pStorage )
        throw uno::RuntimeException();

    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XInputStream > xInputStream;
    uno::Reference< container::XNameAccess > xNameAccess;

    try
    {
        if ( !m_bNoTemporaryCopy && !m_xStream.is() )
            throw io::IOException(); // TODO

        if ( aElement >>= xStream )
            xInputStream = xStream->getInputStream();
        else if ( !( aElement >>= xInputStream ) && !( aElement >>= xNameAccess ) )
            throw lang::IllegalArgumentException(); // TODO:

        if ( xInputStream.is() )
            InsertInputStreamToStorage_Impl( m_pStorage, aName, xInputStream );
        else if ( xNameAccess.is() )
            InsertNameAccessToStorage_Impl( m_pStorage, aName, xNameAccess );
        else
            throw uno::RuntimeException();
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( container::ElementExistException& )
    {
        throw;
    }
    catch( const uno::Exception& e )
    {
        throw lang::WrappedTargetException( ::rtl::OUString(  "Insert has failed!"  ),
                                            uno::Reference< uno::XInterface >(),
                                            uno::makeAny( e ) );
    }
}

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::removeByName( const ::rtl::OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

      if ( !m_pStorage )
        throw uno::RuntimeException();

    if ( !m_bNoTemporaryCopy && !m_xStream.is() )
        throw lang::WrappedTargetException(); // io::IOException(); // TODO

    if ( !m_pStorage->IsContained( aName ) )
        throw container::NoSuchElementException(); // TODO:

    m_pStorage->Remove( aName );

    if ( m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw lang::WrappedTargetException(); // io::IOException(); // TODO
    }
}

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw ( lang::IllegalArgumentException,
                container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    removeByName( aName );

    try
    {
        insertByName( aName, aElement );
    }
    catch( container::ElementExistException& )
    {
           uno::Any aCaught( ::cppu::getCaughtException() );

        throw lang::WrappedTargetException( ::rtl::OUString("Can't copy raw stream"),
                                            uno::Reference< uno::XInterface >(),
                                            aCaught );
    }
}

// --------------------------------------------------------------------------------
uno::Any SAL_CALL OLESimpleStorage::getByName( const ::rtl::OUString& aName )
        throw ( container::NoSuchElementException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

      if ( !m_pStorage )
        throw uno::RuntimeException();

    if ( !m_pStorage->IsContained( aName ) )
        throw container::NoSuchElementException(); // TODO:

    uno::Any aResult;

    uno::Reference< io::XStream > xTempFile(
        io::TempFile::create(comphelper::ComponentContext(m_xFactory).getUNOContext()),
        uno::UNO_QUERY );
    uno::Reference< io::XSeekable > xSeekable( xTempFile, uno::UNO_QUERY_THROW );
    uno::Reference< io::XOutputStream > xOutputStream = xTempFile->getOutputStream();
    uno::Reference< io::XInputStream > xInputStream = xTempFile->getInputStream();
    if ( !xOutputStream.is() || !xInputStream.is() )
        throw uno::RuntimeException();

    if ( m_pStorage->IsStorage( aName ) )
    {
        BaseStorage* pStrg = m_pStorage->OpenStorage( aName );
        m_pStorage->ResetError();
        if ( !pStrg )
            throw io::IOException();

        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xTempFile, sal_False ); // do not close the original stream
        if ( !pStream )
            throw uno::RuntimeException();

        BaseStorage* pNewStor = new Storage( *pStream, sal_False );
        sal_Bool bSuccess =
            ( pStrg->CopyTo( pNewStor ) && pNewStor->Commit() && !pNewStor->GetError() && !pStrg->GetError() );

        DELETEZ( pNewStor );
        DELETEZ( pStrg );
        DELETEZ( pStream );

        if ( !bSuccess )
            throw uno::RuntimeException();

        uno::Sequence< uno::Any > aArgs( 2 );
        aArgs[0] <<= xInputStream; // allow readonly access only
        aArgs[1] <<= (sal_Bool)sal_True; // do not create copy

        uno::Reference< container::XNameContainer > xResultNameContainer(
            m_xFactory->createInstanceWithArguments(
                    ::rtl::OUString("com.sun.star.embed.OLESimpleStorage"),
                    aArgs ),
            uno::UNO_QUERY_THROW );

        aResult <<= xResultNameContainer;
    }
    else
    {
        BaseStorageStream* pStream = m_pStorage->OpenStream( aName, STREAM_READ | STREAM_SHARE_DENYALL | STREAM_NOCREATE );
        if ( !pStream || pStream->GetError() || m_pStorage->GetError() )
        {
            m_pStorage->ResetError();
            DELETEZ( pStream );
            throw io::IOException(); // TODO
        }

        try
        {
            uno::Sequence< sal_Int8 > aData( nBytesCount );
            sal_Int32 nSize = nBytesCount;
            sal_Int32 nRead = 0;
            while( 0 != ( nRead = pStream->Read( aData.getArray(), nSize ) ) )
            {
                if ( nRead < nSize )
                {
                    nSize = nRead;
                    aData.realloc( nSize );
                }

                xOutputStream->writeBytes( aData );
            }

            if ( pStream->GetError() )
                throw io::IOException(); // TODO

            xOutputStream->closeOutput();
            xSeekable->seek( 0 );
        }
        catch( uno::RuntimeException& )
        {
            DELETEZ( pStream );
            throw;
        }
        catch( uno::Exception& )
        {
            DELETEZ( pStream );
            throw lang::WrappedTargetException(); // TODO:
        }

        DELETEZ( pStream );

        aResult <<= xInputStream;
    }

    return aResult;
}

// --------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OLESimpleStorage::getElementNames()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

      if ( !m_pStorage )
        throw uno::RuntimeException();

    SvStorageInfoList aList;
    m_pStorage->FillInfoList( &aList );

    if ( m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw uno::RuntimeException(); // TODO:
    }

    uno::Sequence< ::rtl::OUString > aSeq( aList.size() );
    for ( sal_uInt32 nInd = 0; nInd < aList.size(); nInd++ )
        aSeq[nInd] = aList[nInd].GetName();

    return aSeq;
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OLESimpleStorage::hasByName( const ::rtl::OUString& aName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

     if ( !m_pStorage )
        throw uno::RuntimeException();

    sal_Bool bResult = m_pStorage->IsContained( aName );

    if ( m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw uno::RuntimeException(); // TODO:
    }

    return bResult;
}

// --------------------------------------------------------------------------------
uno::Type SAL_CALL OLESimpleStorage::getElementType()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    return getCppuType( (const uno::Reference< io::XInputStream >*)NULL );
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OLESimpleStorage::hasElements()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

      if ( !m_pStorage )
        throw uno::RuntimeException();

    SvStorageInfoList aList;
    m_pStorage->FillInfoList( &aList );

    if ( m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw uno::RuntimeException(); // TODO:
    }

    return ( aList.size() != 0 );
}

//____________________________________________________________________________________________________
//  XComponent
//____________________________________________________________________________________________________

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::dispose()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_pListenersContainer )
    {
           lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
        m_pListenersContainer->disposeAndClear( aSource );
    }

    DELETEZ( m_pStorage );
    DELETEZ( m_pStream );

    m_xStream = uno::Reference< io::XStream >();
    m_xTempStream = uno::Reference< io::XStream >();

    m_bDisposed = sal_True;
}

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

//____________________________________________________________________________________________________
//  XTransactedObject
//____________________________________________________________________________________________________

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::commit()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

     if ( !m_pStorage )
        throw uno::RuntimeException();

    if ( !m_bNoTemporaryCopy && !m_xStream.is() )
        throw io::IOException(); // TODO

    if ( !m_pStorage->Commit() || m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw io::IOException(); // TODO
    }

    UpdateOriginal_Impl();
}

// --------------------------------------------------------------------------------
void SAL_CALL OLESimpleStorage::revert()
        throw ( ::com::sun::star::io::IOException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

     if ( !m_pStorage )
        throw uno::RuntimeException();

    if ( !m_bNoTemporaryCopy && !m_xStream.is() )
        throw io::IOException(); // TODO

    if ( !m_pStorage->Revert() || m_pStorage->GetError() )
    {
        m_pStorage->ResetError();
        throw io::IOException(); // TODO
    }

    UpdateOriginal_Impl();
}

//____________________________________________________________________________________________________
//  XClassifiedObject
//____________________________________________________________________________________________________

uno::Sequence< sal_Int8 > SAL_CALL OLESimpleStorage::getClassID()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pStorage )
        throw uno::RuntimeException();

    return m_pStorage->GetClassName().GetByteSequence();
}

::rtl::OUString SAL_CALL OLESimpleStorage::getClassName()
    throw ( uno::RuntimeException )
{
    return ::rtl::OUString();
}

void SAL_CALL OLESimpleStorage::setClassInfo( const uno::Sequence< sal_Int8 >& /*aClassID*/,
                            const ::rtl::OUString& /*sClassName*/ )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    throw lang::NoSupportException();
}

//____________________________________________________________________________________________________
//  XServiceInfo
//____________________________________________________________________________________________________

// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OLESimpleStorage::getImplementationName()
        throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

// --------------------------------------------------------------------------------
::sal_Bool SAL_CALL OLESimpleStorage::supportsService( const ::rtl::OUString& ServiceName )
        throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

// --------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OLESimpleStorage::getSupportedServiceNames()
        throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
