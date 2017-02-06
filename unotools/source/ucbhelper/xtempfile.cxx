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

#include "XTempFile.hxx"
#include <unotoolsservices.hxx>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/servicedecl.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/tempfile.hxx>

OTempFileService::OTempFileService(css::uno::Reference< css::uno::XComponentContext > const & context)
: ::cppu::PropertySetMixin< css::io::XTempFile >(
    context
    , static_cast< Implements >( IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET | IMPLEMENTS_PROPERTY_ACCESS )
    , css::uno::Sequence< OUString >() )
, mpStream( nullptr )
, mbRemoveFile( true )
, mbInClosed( false )
, mbOutClosed( false )
, mnCachedPos( 0 )
, mbHasCachedPos( false )

{
    mpTempFile.reset(new utl::TempFile());
    mpTempFile->EnableKillingFile();
}

OTempFileService::~OTempFileService ()
{
}

// XInterface

css::uno::Any SAL_CALL OTempFileService::queryInterface( css::uno::Type const & aType )
{
    css::uno::Any aResult( OTempFileBase::queryInterface( aType ) );
    if (!aResult.hasValue())
        aResult = cppu::PropertySetMixin< css::io::XTempFile >::queryInterface( aType );
    return aResult;
};
void SAL_CALL OTempFileService::acquire(  )
throw ()
{
    OTempFileBase::acquire();
}
void SAL_CALL OTempFileService::release(  )
throw ()
{
    OTempFileBase::release();
}

//  XTypeProvider

css::uno::Sequence< css::uno::Type > SAL_CALL OTempFileService::getTypes(  )
{
    static ::cppu::OTypeCollection* pTypeCollection = nullptr;
    if ( pTypeCollection == nullptr )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        if ( pTypeCollection == nullptr )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                cppu::UnoType<css::beans::XPropertySet>::get()
                ,OTempFileBase::getTypes() );
            pTypeCollection = &aTypeCollection;
        }
    }
    return pTypeCollection->getTypes();
};

//  XTempFile

sal_Bool SAL_CALL OTempFileService::getRemoveFile()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw css::uno::RuntimeException();
        }

    return mbRemoveFile;
};
void SAL_CALL OTempFileService::setRemoveFile( sal_Bool _removefile )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw css::uno::RuntimeException();
    }

    mbRemoveFile = _removefile;
    mpTempFile->EnableKillingFile( mbRemoveFile );
};
OUString SAL_CALL OTempFileService::getUri()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        throw css::uno::RuntimeException();
    }

    return OUString( mpTempFile->GetURL() );

};
OUString SAL_CALL OTempFileService::getResourceName()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        throw css::uno::RuntimeException();
}

    return OUString( mpTempFile->GetFileName() );
};

// XInputStream

sal_Int32 SAL_CALL OTempFileService::readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), static_cast< css::uno::XWeak * >(this));

    if (aData.getLength() < nBytesToRead)
        aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->ReadBytes(static_cast<void*>(aData.getArray()), nBytesToRead);
    checkError();

    if (nRead < (std::size_t)aData.getLength())
        aData.realloc( nRead );

    if ( sal::static_int_cast<sal_uInt32>(nBytesToRead) > nRead )
    {
        // usually that means that the stream was read till the end
        // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = true;

        mpStream = nullptr;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    return nRead;
}
sal_Int32 SAL_CALL OTempFileService::readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    checkError();

    if (nMaxBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), static_cast < css::uno::XWeak * >( this ) );

    if (mpStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL OTempFileService::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    checkError();
    mpStream->SeekRel(nBytesToSkip);
    checkError();
}
sal_Int32 SAL_CALL OTempFileService::available(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();

    sal_uInt32 const nAvailable =
        static_cast<sal_uInt32>(mpStream->remainingSize());
    checkError();

    return nAvailable;
}
void SAL_CALL OTempFileService::closeInput(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak  * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    mbInClosed = true;

    if ( mbOutClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = nullptr;

        if ( mpTempFile )
            mpTempFile.reset(nullptr);
    }
}

// XOutputStream

void SAL_CALL OTempFileService::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    sal_uInt32 nWritten = mpStream->WriteBytes(aData.getConstArray(), aData.getLength());
    checkError();
    if  ( nWritten != (sal_uInt32)aData.getLength())
        throw css::io::BufferSizeExceededException( OUString(),static_cast < css::uno::XWeak * > ( this ) );
}
void SAL_CALL OTempFileService::flush(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    mpStream->Flush();
    checkError();
}
void SAL_CALL OTempFileService::closeOutput(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    mbOutClosed = true;

    // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
    if ( mpStream )
    {
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = true;

        mpStream = nullptr;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    if ( mbInClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = nullptr;

        if ( mpTempFile )
            mpTempFile.reset(nullptr);
    }
}

void OTempFileService::checkError () const
{
    if (!mpStream || mpStream->SvStream::GetError () != ERRCODE_NONE )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );
}
void OTempFileService::checkConnected ()
{
    if (!mpStream && mpTempFile)
    {
        mpStream = mpTempFile->GetStream( StreamMode::STD_READWRITE );
        if ( mpStream && mbHasCachedPos )
        {
            mpStream->Seek( sal::static_int_cast<std::size_t>(mnCachedPos) );
            if ( mpStream->SvStream::GetError () == ERRCODE_NONE )
            {
                mbHasCachedPos = false;
                mnCachedPos = 0;
            }
            else
            {
                mpStream = nullptr;
                mpTempFile->CloseStream();
            }
        }
    }

    if (!mpStream)
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );
}

// XSeekable

void SAL_CALL OTempFileService::seek( sal_Int64 nLocation )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();
    if ( nLocation < 0 || nLocation > getLength() )
        throw css::lang::IllegalArgumentException();

    mpStream->Seek((sal_uInt32) nLocation );
    checkError();
}
sal_Int64 SAL_CALL OTempFileService::getPosition(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}
sal_Int64 SAL_CALL OTempFileService::getLength(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nCurrentPos = mpStream->Tell();
    checkError();

    mpStream->Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = mpStream->Tell();
    mpStream->Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

// XStream

css::uno::Reference< css::io::XInputStream > SAL_CALL OTempFileService::getInputStream()
    {
    return css::uno::Reference< css::io::XInputStream >( *this, css::uno::UNO_QUERY );
}

css::uno::Reference< css::io::XOutputStream > SAL_CALL OTempFileService::getOutputStream()
    {
    return css::uno::Reference< css::io::XOutputStream >( *this, css::uno::UNO_QUERY );
    }

// XTruncate

void SAL_CALL OTempFileService::truncate()
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();
    // SetStreamSize() call does not change the position
    mpStream->Seek( 0 );
    mpStream->SetStreamSize( 0 );
    checkError();
}

namespace sdecl = ::comphelper::service_decl;
sdecl::class_< OTempFileService> const OTempFileServiceImpl;
const sdecl::ServiceDecl OTempFileServiceDecl(
    OTempFileServiceImpl,
    "com.sun.star.io.comp.TempFile",
    "com.sun.star.io.TempFile");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
