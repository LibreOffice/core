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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/file.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/tempfile.hxx>
#include <XTempFile.hxx>

OTempFileService::OTempFileService(css::uno::Reference< css::uno::XComponentContext > const & context)
: ::cppu::PropertySetMixin< css::io::XTempFile >(
    context
    , static_cast< Implements >( IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET | IMPLEMENTS_PROPERTY_ACCESS )
    , com::sun::star::uno::Sequence< OUString >() )
, mpStream( NULL )
, mbRemoveFile( sal_True )
, mbInClosed( sal_False )
, mbOutClosed( sal_False )
, mnCachedPos( 0 )
, mbHasCachedPos( sal_False )

{
    mpTempFile = new ::utl::TempFile;
    mpTempFile->EnableKillingFile ( true );
}

OTempFileService::~OTempFileService ()
{
    if ( mpTempFile )
        delete mpTempFile;
}


// XInterface

css::uno::Any SAL_CALL OTempFileService::queryInterface( css::uno::Type const & aType )
throw ( css::uno::RuntimeException )
{
    css::uno::Any aResult( OTempFileBase::queryInterface( aType ) );
    if (!aResult.hasValue())
        aResult = cppu::PropertySetMixin< css::io::XTempFile >::queryInterface( aType ) ;
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
throw ( css::uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;
    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pTypeCollection == NULL )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                ::getCppuType( ( const css::uno::Reference< css::beans::XPropertySet >*)NULL )
                ,OTempFileBase::getTypes() );
            pTypeCollection = &aTypeCollection;
        }
    }
    return pTypeCollection->getTypes();
};
css::uno::Sequence< sal_Int8 > SAL_CALL OTempFileService::getImplementationId(  )
throw ( css::uno::RuntimeException )
{
    return OTempFileBase::getImplementationId();
}

//  XTempFile

sal_Bool SAL_CALL OTempFileService::getRemoveFile()
throw ( css::uno::RuntimeException )
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
throw ( css::uno::RuntimeException )
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
throw ( css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        throw css::uno::RuntimeException();
    }

    return OUString( mpTempFile->GetURL() );

};
OUString SAL_CALL OTempFileService::getResourceName()
throw ( css::uno::RuntimeException )
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
throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), static_cast< css::uno::XWeak * >(this));

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->Read(static_cast < void* > ( aData.getArray() ), nBytesToRead);
    checkError();

    if (nRead < static_cast < sal_uInt32 > ( nBytesToRead ) )
        aData.realloc( nRead );

    if ( sal::static_int_cast<sal_uInt32>(nBytesToRead) > nRead )
    {
        // usually that means that the stream was read till the end
        // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = sal_True;

        mpStream = NULL;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    return nRead;
}
sal_Int32 SAL_CALL OTempFileService::readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
throw ( css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
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
throw ( css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
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
throw ( css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();

    mpStream->Seek(STREAM_SEEK_TO_END);
    checkError();

    sal_Int32 nAvailable = (sal_Int32)mpStream->Tell() - nPos;
    mpStream->Seek(nPos);
    checkError();

    return nAvailable;
}
void SAL_CALL OTempFileService::closeInput(  )
throw ( css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak  * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    mbInClosed = sal_True;

    if ( mbOutClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = NULL;

        if ( mpTempFile )
        {
            delete mpTempFile;
            mpTempFile = NULL;
        }
    }
}

// XOutputStream

void SAL_CALL OTempFileService::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
throw ( css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    sal_uInt32 nWritten = mpStream->Write(aData.getConstArray(),aData.getLength());
    checkError();
    if  ( nWritten != (sal_uInt32)aData.getLength())
        throw css::io::BufferSizeExceededException( OUString(),static_cast < css::uno::XWeak * > ( this ) );
}
void SAL_CALL OTempFileService::flush(  )
throw ( css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    checkConnected();
    mpStream->Flush();
    checkError();
}
void SAL_CALL OTempFileService::closeOutput(  )
throw ( css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );

    mbOutClosed = sal_True;

    // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
    if ( mpStream )
    {
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = sal_True;

        mpStream = NULL;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    if ( mbInClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = NULL;

        if ( mpTempFile )
        {
            delete mpTempFile;
            mpTempFile = NULL;
        }
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
        mpStream = mpTempFile->GetStream( STREAM_STD_READWRITE );
        if ( mpStream && mbHasCachedPos )
        {
            mpStream->Seek( sal::static_int_cast<sal_Size>(mnCachedPos) );
            if ( mpStream->SvStream::GetError () == ERRCODE_NONE )
            {
                mbHasCachedPos = sal_False;
                mnCachedPos = 0;
            }
            else
            {
                mpStream = NULL;
                mpTempFile->CloseStream();
            }
        }
    }

    if (!mpStream)
        throw css::io::NotConnectedException ( OUString(), const_cast < css::uno::XWeak * > ( static_cast < const css::uno::XWeak * > (this ) ) );
}

// XSeekable

void SAL_CALL OTempFileService::seek( sal_Int64 nLocation )
throw ( css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();
    if ( nLocation < 0 || nLocation > getLength() )
        throw css::lang::IllegalArgumentException();

    mpStream->Seek((sal_uInt32) nLocation );
    checkError();
}
sal_Int64 SAL_CALL OTempFileService::getPosition(  )
throw ( css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}
sal_Int64 SAL_CALL OTempFileService::getLength(  )
throw ( css::io::IOException, css::uno::RuntimeException )
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
throw ( css::uno::RuntimeException )
    {
    return css::uno::Reference< css::io::XInputStream >( *this, css::uno::UNO_QUERY );
}

css::uno::Reference< css::io::XOutputStream > SAL_CALL OTempFileService::getOutputStream()
throw ( css::uno::RuntimeException )
    {
    return css::uno::Reference< css::io::XOutputStream >( *this, css::uno::UNO_QUERY );
    }

// XTruncate

void SAL_CALL OTempFileService::truncate()
throw ( css::io::IOException, css::uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();
    // SetStreamSize() call does not change the position
    mpStream->Seek( 0 );
    mpStream->SetStreamSize( 0 );
    checkError();
}

// XServiceInfo

OUString SAL_CALL OTempFileService::getImplementationName()
throw ( css::uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL OTempFileService::supportsService( OUString const & rServiceName )
throw ( css::uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence < OUString > SAL_CALL OTempFileService::getSupportedServiceNames()
throw ( css::uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}



OUString OTempFileService::getImplementationName_Static ()
{
    return OUString ( "com.sun.star.io.comp.TempFile" );
}
css::uno::Sequence < OUString > OTempFileService::getSupportedServiceNames_Static()
{
    css::uno::Sequence < OUString > aNames ( 1 );
    aNames[0] = "com.sun.star.io.TempFile";
    return aNames;
}
css::uno::Reference < css::uno::XInterface >SAL_CALL XTempFile_createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW( ( css::uno::Exception ) )
{
    return static_cast< ::cppu::OWeakObject * >( new OTempFileService(context) );
}

css::uno::Reference < css::lang::XSingleComponentFactory > OTempFileService::createServiceFactory_Static()
{
    return ::cppu::createSingleComponentFactory( XTempFile_createInstance, getImplementationName_Static(), getSupportedServiceNames_Static() );
}

/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL utl_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMgr(
        reinterpret_cast< css::lang::XMultiServiceFactory * >( pServiceManager ) );
    css::uno::Reference< css::lang::XSingleComponentFactory > xFactory;

    if (OTempFileService::getImplementationName_Static().equalsAscii( pImplName ) )
        xFactory = OTempFileService::createServiceFactory_Static();

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
