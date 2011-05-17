/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>

#include <comphelper/otransactedfilestream.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/implbase1.hxx>

using namespace ::com::sun::star;

namespace comphelper
{

// ========================================================================
class OTransactionHelper : public ::cppu::WeakImplHelper1 < embed::XTransactedObject >
{
    OTruncatedTransactedFileStream* m_pFileStream;
    uno::Reference< io::XStream > m_xStreamHolder;

public:
    OTransactionHelper( OTruncatedTransactedFileStream* pStream )
    : m_pFileStream( pStream )
    {
        m_xStreamHolder = static_cast< io::XStream* >( pStream );
        if ( !m_xStreamHolder.is() )
            throw uno::RuntimeException();
    }

    virtual void SAL_CALL commit(  ) throw (io::IOException, lang::WrappedTargetException, uno::RuntimeException);
    virtual void SAL_CALL revert(  ) throw (io::IOException, lang::WrappedTargetException, uno::RuntimeException);
};

// ------------------------------------------------------------------------
void SAL_CALL OTransactionHelper::commit(  ) throw (io::IOException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_pFileStream->Commit_Impl();
}

// ------------------------------------------------------------------------
void SAL_CALL OTransactionHelper::revert(  ) throw (io::IOException, lang::WrappedTargetException, uno::RuntimeException)
{
    m_pFileStream->Revert_Impl();
}

// ========================================================================
struct TTFileStreamData_Impl
{
    uno::Reference< ucb::XSimpleFileAccess > m_xFileAccess;
    sal_Bool m_bDelete;
    ::rtl::OUString m_aURL;

    // the streams below are not visible from outside so there is no need to remember position

    // original stream related members
    uno::Reference< io::XStream > m_xOrigStream;
    uno::Reference< io::XTruncate > m_xOrigTruncate;
    uno::Reference< io::XSeekable > m_xOrigSeekable;
    uno::Reference< io::XInputStream > m_xOrigInStream;
    uno::Reference< io::XOutputStream > m_xOrigOutStream;

    // temporary stream related members
    uno::Reference< io::XStream > m_xTempStream;
    uno::Reference< io::XTruncate > m_xTempTruncate;
    uno::Reference< io::XSeekable > m_xTempSeekable;
    uno::Reference< io::XInputStream > m_xTempInStream;
    uno::Reference< io::XOutputStream > m_xTempOutStream;

    sal_Bool m_bInOpen;
    sal_Bool m_bOutOpen;

    sal_Bool m_bTransacted;


    TTFileStreamData_Impl(
            const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
            sal_Bool bDelete,
            const ::rtl::OUString& aURL,
            const uno::Reference< io::XStream >& xOrigStream,
            const uno::Reference< io::XTruncate >& xOrigTruncate,
            const uno::Reference< io::XSeekable >& xOrigSeekable,
            const uno::Reference< io::XInputStream >& xOrigInStream,
            const uno::Reference< io::XOutputStream >& xOrigOutStream,
            const uno::Reference< io::XStream >& xTempStream,
            const uno::Reference< io::XTruncate >& xTempTruncate,
            const uno::Reference< io::XSeekable >& xTempSeekable,
            const uno::Reference< io::XInputStream >& xTempInStream,
            const uno::Reference< io::XOutputStream >& xTempOutStream )
    : m_xFileAccess( xFileAccess )
    , m_bDelete( bDelete )
    , m_aURL( aURL )
    , m_xOrigStream( xOrigStream )
    , m_xOrigTruncate( xOrigTruncate )
    , m_xOrigSeekable( xOrigSeekable )
    , m_xOrigInStream( xOrigInStream )
    , m_xOrigOutStream( xOrigOutStream )
    , m_xTempStream( xTempStream )
    , m_xTempTruncate( xTempTruncate )
    , m_xTempSeekable( xTempSeekable )
    , m_xTempInStream( xTempInStream )
    , m_xTempOutStream( xTempOutStream )
    , m_bInOpen( sal_False )
    , m_bOutOpen( sal_False )
    , m_bTransacted( sal_True )
    {}

    void NoTransaction()
    {
        m_bDelete = sal_False;
        m_bTransacted = sal_False;
        m_xTempStream = uno::Reference< io::XStream >();
        m_xTempTruncate = uno::Reference< io::XTruncate >();
        m_xTempSeekable = uno::Reference< io::XSeekable >();
        m_xTempInStream = uno::Reference< io::XInputStream >();
        m_xTempOutStream = uno::Reference< io::XOutputStream >();
    }

    void FreeOriginal()
    {
        m_bDelete = sal_False;
        m_bTransacted = sal_False;

        m_xOrigStream = m_xTempStream;
        m_xTempStream = uno::Reference< io::XStream >();

        m_xOrigTruncate = m_xTempTruncate;
        m_xTempTruncate = uno::Reference< io::XTruncate >();

        m_xOrigSeekable = m_xTempSeekable;
        m_xTempSeekable = uno::Reference< io::XSeekable >();

        m_xOrigInStream = m_xTempInStream;
        m_xTempInStream = uno::Reference< io::XInputStream >();

        m_xOrigOutStream = m_xTempOutStream;
        m_xTempOutStream = uno::Reference< io::XOutputStream >();
    }
};

// ========================================================================
// ------------------------------------------------------------------------
OTruncatedTransactedFileStream::OTruncatedTransactedFileStream(
        const ::rtl::OUString& aURL,
        const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_pStreamData( NULL )
{
    uno::Reference< ucb::XSimpleFileAccess > xSimpleFileAccess(
        xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" ) ) ),
        uno::UNO_QUERY_THROW );

    CommonInit_Impl( aURL, xSimpleFileAccess, xFactory, sal_False );
}

// ------------------------------------------------------------------------
OTruncatedTransactedFileStream::OTruncatedTransactedFileStream(
        const ::rtl::OUString& aURL,
        const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
        const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_pStreamData( NULL )
{
    CommonInit_Impl( aURL, xFileAccess, xFactory, sal_False );
}

// ------------------------------------------------------------------------
OTruncatedTransactedFileStream::OTruncatedTransactedFileStream(
        const ::rtl::OUString& aURL,
        const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        sal_Bool bDeleteIfNotCommited )
: m_pStreamData( NULL )
{
    CommonInit_Impl( aURL, xFileAccess, xFactory, sal_True );
    if ( m_pStreamData )
        m_pStreamData->m_bDelete = bDeleteIfNotCommited;
}

// ------------------------------------------------------------------------
OTruncatedTransactedFileStream::~OTruncatedTransactedFileStream()
{
    CloseAll_Impl();
}

// ------------------------------------------------------------------------
void OTruncatedTransactedFileStream::CloseAll_Impl()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
    {
        sal_Bool bDelete = m_pStreamData->m_bDelete;
        ::rtl::OUString aURL = m_pStreamData->m_aURL;
        uno::Reference< ucb::XSimpleFileAccess > xFileAccess = m_pStreamData->m_xFileAccess;

        delete m_pStreamData;
        m_pStreamData = NULL;

        if ( bDelete && xFileAccess.is() && aURL.getLength() )
        {
            // delete the file
            try
            {
                xFileAccess->kill( aURL );
            } catch( uno::Exception& )
            {
                OSL_FAIL( "Could not remove the file!" );
            }
        }
    }
}

// ------------------------------------------------------------------------
void OTruncatedTransactedFileStream::CommonInit_Impl(
        const ::rtl::OUString& aURL,
        const uno::Reference< ucb::XSimpleFileAccess >& xFileAccess,
        const uno::Reference< lang::XMultiServiceFactory >& xFactory,
        sal_Bool bDeleteOptionIsProvided )
{
    sal_Bool bDelete = sal_False;
    if ( !bDeleteOptionIsProvided )
        bDelete = !xFileAccess->exists( aURL );

    uno::Reference< io::XStream > xOrigStream = xFileAccess->openFileReadWrite( aURL );
    uno::Reference< io::XTruncate > xOrigTruncate( xOrigStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XSeekable > xOrigSeekable( xOrigStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > xOrigInStream = xOrigStream->getInputStream();
    uno::Reference< io::XOutputStream > xOrigOutStream = xOrigStream->getOutputStream();
    if ( !xOrigInStream.is() || !xOrigOutStream.is() )
        throw uno::RuntimeException();

    // temporary stream related members
    uno::Reference< io::XStream > xTempStream( xFactory->createInstance(
        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TempFile" ) ) ),
        uno::UNO_QUERY_THROW );
    uno::Reference< io::XTruncate > xTempTruncate( xTempStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XSeekable > xTempSeekable( xTempStream, uno::UNO_QUERY_THROW );
    uno::Reference< io::XInputStream > xTempInStream = xTempStream->getInputStream();
    uno::Reference< io::XOutputStream > xTempOutStream = xTempStream->getOutputStream();
    if ( !xTempInStream.is() || !xTempOutStream.is() )
        throw uno::RuntimeException();

    m_pStreamData = new TTFileStreamData_Impl( xFileAccess, bDelete, aURL,
                                            xOrigStream, xOrigTruncate, xOrigSeekable, xOrigInStream, xOrigOutStream,
                                            xTempStream, xTempTruncate, xTempSeekable, xTempInStream, xTempOutStream );
}

// ------------------------------------------------------------------------
void OTruncatedTransactedFileStream::Commit_Impl()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        sal_Int64 nPos = m_pStreamData->m_xTempSeekable->getPosition();
        m_pStreamData->m_xTempSeekable->seek( 0 );

        // after the following step fails the information might be lost, throw an exception with URL of temporary file
        try
        {
            m_pStreamData->m_xOrigTruncate->truncate();
            OStorageHelper::CopyInputToOutput( m_pStreamData->m_xTempInStream, m_pStreamData->m_xOrigOutStream );
            m_pStreamData->m_xOrigOutStream->flush();

            // in case the stream is based on a file it will implement the following interface
            // the call should be used to be sure that the contents are written to the file system
            uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( m_pStreamData->m_xOrigOutStream, uno::UNO_QUERY );
            if ( asyncOutputMonitor.is() )
                asyncOutputMonitor->waitForCompletion();
        }
        catch( uno::Exception& )
        {
            ::rtl::OUString aTempURL;
            try {
                uno::Reference< beans::XPropertySet > xTempFile( m_pStreamData->m_xTempStream, uno::UNO_QUERY_THROW );
                uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Uri")) );
                aUrl >>= aTempURL;
                xTempFile->setPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RemoveFile")),
                                             uno::makeAny( sal_False ) );

                m_pStreamData->m_xTempSeekable->seek( nPos );
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "These calls are pretty simple, they should not fail!\n" );
            }

            m_pStreamData->FreeOriginal();

            ::rtl::OUString aErrTxt( RTL_CONSTASCII_USTRINGPARAM ( "Writing file failed!" ) );
            embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), aTempURL );
            throw lang::WrappedTargetException( aErrTxt,
                                                static_cast < OWeakObject * > ( this ),
                                                uno::makeAny ( aException ) );
        }

        m_pStreamData->m_xOrigSeekable->seek( nPos );
        m_pStreamData->NoTransaction();
    }
    else
        throw io::NotConnectedException();
}

// ------------------------------------------------------------------------
void OTruncatedTransactedFileStream::Revert_Impl()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
        m_pStreamData->m_xTempTruncate->truncate();
    else
        throw io::NotConnectedException();
}

// com::sun::star::io::XStream
// ------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OTruncatedTransactedFileStream::getInputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bInOpen = sal_True;
    return static_cast< io::XInputStream* >( this );
}


// ------------------------------------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OTruncatedTransactedFileStream::getOutputStream(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_pStreamData )
        m_pStreamData->m_bOutOpen = sal_True;
    return static_cast< io::XOutputStream* >( this );
}



// com::sun::star::io::XInputStream
// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OTruncatedTransactedFileStream::readBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xTempInStream->readBytes( aData, nBytesToRead );
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->readBytes( aData, nBytesToRead );
    }
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OTruncatedTransactedFileStream::readSomeBytes( uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xTempInStream->readSomeBytes( aData, nMaxBytesToRead );
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->readSomeBytes( aData, nMaxBytesToRead );
    }
}

// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::skipBytes( ::sal_Int32 nBytesToSkip )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempInStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xTempInStream->skipBytes( nBytesToSkip );
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigInStream->skipBytes( nBytesToSkip );
    }
}


// ------------------------------------------------------------------------
::sal_Int32 SAL_CALL OTruncatedTransactedFileStream::available(  )
    throw (io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xTempInStream->available();
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigInStream.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigInStream->available();
    }
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::closeInput()
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
void SAL_CALL OTruncatedTransactedFileStream::writeBytes( const uno::Sequence< ::sal_Int8 >& aData )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempOutStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xTempOutStream->writeBytes( aData );
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigOutStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigOutStream->writeBytes( aData );
    }
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::flush(  )
    throw (io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
    {
        OSL_FAIL( "flush() call on closed stream!\n" );
        return;
        // in future throw exception, for now some code might call flush() on closed stream
        // since file ucp implementation allows it
        // throw io::NotConnectedException();
    }

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempOutStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xTempOutStream->flush();
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigOutStream.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigOutStream->flush();
    }
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::closeOutput(  )
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
void SAL_CALL OTruncatedTransactedFileStream::truncate(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempTruncate.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xTempTruncate->truncate();
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigTruncate.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigTruncate->truncate();
    }
}



// com::sun::star::io::XSeekable
// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::seek( ::sal_Int64 location )
    throw (lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempSeekable.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xTempSeekable->seek( location );
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        m_pStreamData->m_xOrigSeekable->seek( location );
    }
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL OTruncatedTransactedFileStream::getPosition(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xTempSeekable->getPosition();
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigSeekable->getPosition();
    }
}


// ------------------------------------------------------------------------
::sal_Int64 SAL_CALL OTruncatedTransactedFileStream::getLength(  )
    throw (io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    if ( m_pStreamData->m_bTransacted )
    {
        // temporary stream data should be provided
        if ( !m_pStreamData->m_xTempSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xTempSeekable->getLength();
    }
    else
    {
        // the original stream data should be provided
        if ( !m_pStreamData->m_xOrigSeekable.is() )
            throw uno::RuntimeException();

        return m_pStreamData->m_xOrigSeekable->getLength();
    }
}

// com::sun::star::beans::XPropertySetInfo
// ------------------------------------------------------------------------
uno::Sequence< beans::Property > SAL_CALL OTruncatedTransactedFileStream::getProperties()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Sequence< beans::Property > aProps( 1 );
    aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );
    aProps[0].Type = getCppuType( static_cast< uno::Reference< beans::XPropertySet >* >( NULL ) );
    aProps[0].Attributes = beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY;

    return aProps;
}


// ------------------------------------------------------------------------
beans::Property SAL_CALL OTruncatedTransactedFileStream::getPropertyByName( const ::rtl::OUString& aName )
    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aTransactionPropName( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );

    if ( !aName.equals( aTransactionPropName ) )
        throw beans::UnknownPropertyException();

    beans::Property aProp;
    aProp.Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );
    aProp.Type = getCppuType( static_cast< uno::Reference< beans::XPropertySet >* >( NULL ) );
    aProp.Attributes = beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY;

    return aProp;
}


// ------------------------------------------------------------------------
::sal_Bool SAL_CALL OTruncatedTransactedFileStream::hasPropertyByName( const ::rtl::OUString& Name )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aTransactionPropName( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );
    return ( Name.equals( aTransactionPropName ) );
}



// com::sun::star::beans::XPropertySet
// ------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OTruncatedTransactedFileStream::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return static_cast< beans::XPropertySetInfo* >( this );
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aTransactionPropName( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );
    if ( aPropertyName.equals( aTransactionPropName ) )
        throw beans::PropertyVetoException();

    throw beans::UnknownPropertyException();
}


// ------------------------------------------------------------------------
uno::Any SAL_CALL OTruncatedTransactedFileStream::getPropertyValue( const ::rtl::OUString& PropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pStreamData )
        throw io::NotConnectedException();

    ::rtl::OUString aTransactionPropName( RTL_CONSTASCII_USTRINGPARAM( "TransactionSupport" ) );
    if ( PropertyName.equals( aTransactionPropName ) )
    {
        uno::Reference< embed::XTransactedObject > xObj;
        if ( m_pStreamData->m_bTransacted )
            xObj = static_cast< embed::XTransactedObject* >( new OTransactionHelper( this ) );

        return uno::makeAny( xObj );
    }

    throw beans::UnknownPropertyException();
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::addPropertyChangeListener( const ::rtl::OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    // not implemented
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::removePropertyChangeListener( const ::rtl::OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    // not implemented
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::addVetoableChangeListener( const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    // not implemented
}


// ------------------------------------------------------------------------
void SAL_CALL OTruncatedTransactedFileStream::removeVetoableChangeListener( const ::rtl::OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    // not implemented
}


} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
