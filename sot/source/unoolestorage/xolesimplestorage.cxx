/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xolesimplestorage.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:45:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif

#include <comphelper/storagehelper.hxx>

#include <unotools/ucbstreamhelper.hxx>

#include <storinfo.hxx>

#include "xolesimplestorage.hxx"


using namespace ::com::sun::star;

const sal_Int32 nBytesCount = 32000;


// --------------------------------------------------------------------------------
OLESimpleStorage::OLESimpleStorage( uno::Reference< lang::XMultiServiceFactory > xFactory )
: m_xFactory( xFactory )
, m_bDisposed( sal_False )
, m_pStream( NULL )
, m_pStorage( NULL )
, m_pListenersContainer( NULL )
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
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.embed.OLESimpleStorage");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OLESimpleStorage");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OLESimpleStorage::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.embed.OLESimpleStorage");
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
    OSL_ENSURE( nArgNum == 1, "Wrong parameter number" );

    if ( nArgNum != 1 )
        throw lang::IllegalArgumentException(); // TODO:

    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XInputStream > xInputStream;
    if ( !( aArguments[0] >>= xStream ) && !( aArguments[0] >>= xInputStream ) )
        throw lang::IllegalArgumentException(); // TODO:

    // The temporary stream has to be created since SvStream wrapper closes the stream is owns
    uno::Reference < io::XStream > xTempFile(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
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
        m_pStream = ::utl::UcbStreamHelper::CreateStream( xTempInput );
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

        m_pStream = ::utl::UcbStreamHelper::CreateStream( xTempFile );
    }
    else
        throw lang::IllegalArgumentException(); // TODO:

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

    BaseStorageStream* pNewStream = NULL;
    try
    {
        if ( !m_xStream.is() )
            throw io::IOException(); // TODO

        if ( aElement >>= xStream )
            xInputStream = xStream->getInputStream();
        else if ( !( aElement >>= xInputStream ) )
            throw lang::IllegalArgumentException(); // TODO:

        if ( !xInputStream.is() )
            throw uno::RuntimeException();

        if ( m_pStorage->IsContained( aName ) )
            throw container::ElementExistException(); // TODO:

        pNewStream = m_pStorage->OpenStream( aName );
        if ( !pNewStream || pNewStream->GetError() || m_pStorage->GetError() )
        {
            m_pStorage->ResetError();
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
            m_pStorage->Remove( aName );

            throw;
        }
    }
    catch( uno::RuntimeException& )
    {
        DELETEZ( pNewStream );
        throw;
    }
    catch( container::ElementExistException& )
    {
        DELETEZ( pNewStream );
        throw;
    }
    catch( uno::Exception& )
    {
        DELETEZ( pNewStream );
        throw lang::WrappedTargetException(); // TODO:
    }

    DELETEZ( pNewStream );
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

    if ( !m_xStream.is() )
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
    insertByName( aName, aElement );
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

    if ( m_pStorage->IsStorage( aName ) )
    {
        OSL_ENSURE( sal_False, "Access to OLE substorages not implemented yet!\n" );
        throw lang::WrappedTargetException(); // TODO
    }

    uno::Reference< io::XOutputStream > xOutputStream(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY );
    uno::Reference< io::XInputStream > xInputStream( xOutputStream, uno::UNO_QUERY );
    uno::Reference< io::XSeekable > xSeekable( xInputStream, uno::UNO_QUERY );

    if ( !xSeekable.is() )
        throw uno::RuntimeException();

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

    return uno::makeAny( xInputStream );
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

    uno::Sequence< ::rtl::OUString > aSeq( aList.Count() );
    for ( sal_uInt32 nInd = 0; nInd < aList.Count(); nInd++ )
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

    return ( aList.Count() != 0 );
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

    if ( !m_xStream.is() )
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

    if ( !m_xStream.is() )
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

void SAL_CALL OLESimpleStorage::setClassInfo( const uno::Sequence< sal_Int8 >& aClassID,
                            const ::rtl::OUString& sClassName )
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


