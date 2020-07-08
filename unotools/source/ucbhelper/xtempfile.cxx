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
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <o3tl/safeint.hxx>
#include <unotools/tempfile.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/supportsservice.hxx>

OTempFileService::OTempFileService(css::uno::Reference< css::uno::XComponentContext > const &)
: mpStream( nullptr )
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

//  XTypeProvider

css::uno::Sequence< css::uno::Type > SAL_CALL OTempFileService::getTypes(  )
{
    static ::cppu::OTypeCollection ourTypeCollection(
                cppu::UnoType<css::beans::XPropertySet>::get()
                ,OTempFileBase::getTypes() );

    return ourTypeCollection.getTypes();
};

//  XTempFile

sal_Bool SAL_CALL OTempFileService::getRemoveFile()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw css::uno::RuntimeException("Not connected to a file.");
        }

    return mbRemoveFile;
};
void SAL_CALL OTempFileService::setRemoveFile( sal_Bool _removefile )
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw css::uno::RuntimeException("Not connected to a file.");
    }

    mbRemoveFile = _removefile;
    mpTempFile->EnableKillingFile( mbRemoveFile );
};
OUString SAL_CALL OTempFileService::getUri()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        throw css::uno::RuntimeException("Not connected to a file.");
    }

    return mpTempFile->GetURL();

};
OUString SAL_CALL OTempFileService::getResourceName()
{
    ::osl::MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        throw css::uno::RuntimeException("Not connected to a file.");
}

    return mpTempFile->GetFileName();
};

// XInputStream

sal_Int32 SAL_CALL OTempFileService::readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();
    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), static_cast< css::uno::XWeak * >(this));

    if (aData.getLength() < nBytesToRead)
        aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->ReadBytes(static_cast<void*>(aData.getArray()), nBytesToRead);
    checkError();

    if (nRead < o3tl::make_unsigned(aData.getLength()))
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
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();
    checkError();

    if (nMaxBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), static_cast < css::uno::XWeak * >( this ) );

    if (mpStream->eof())
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
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();
    checkError();
    mpStream->SeekRel(nBytesToSkip);
    checkError();
}
sal_Int32 SAL_CALL OTempFileService::available(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();

    sal_Int64 nAvailable = mpStream->remainingSize();
    checkError();

    return std::min<sal_Int64>(SAL_MAX_INT32, nAvailable);
}
void SAL_CALL OTempFileService::closeInput(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    mbInClosed = true;

    if ( mbOutClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = nullptr;
        mpTempFile.reset();
    }
}

// XOutputStream

void SAL_CALL OTempFileService::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();
    sal_uInt32 nWritten = mpStream->WriteBytes(aData.getConstArray(), aData.getLength());
    checkError();
    if  ( nWritten != static_cast<sal_uInt32>(aData.getLength()))
        throw css::io::BufferSizeExceededException( OUString(),static_cast < css::uno::XWeak * > ( this ) );
}
void SAL_CALL OTempFileService::flush(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

    checkConnected();
    mpStream->Flush();
    checkError();
}
void SAL_CALL OTempFileService::closeOutput(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );

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
        mpTempFile.reset();
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
        throw css::io::NotConnectedException ( OUString(), static_cast < css::uno::XWeak * > (this ) );
}

// XSeekable

void SAL_CALL OTempFileService::seek( sal_Int64 nLocation )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();
    if ( nLocation < 0 || nLocation > getLength() )
        throw css::lang::IllegalArgumentException();

    mpStream->Seek(static_cast<sal_uInt32>(nLocation) );
    checkError();
}
sal_Int64 SAL_CALL OTempFileService::getPosition(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();
    return static_cast<sal_Int64>(nPos);
}
sal_Int64 SAL_CALL OTempFileService::getLength(  )
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    checkError();

    sal_Int64 nEndPos = mpStream->TellEnd();

    return nEndPos;
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

#define PROPERTY_HANDLE_URI 1
#define PROPERTY_HANDLE_REMOVE_FILE 2
#define PROPERTY_HANDLE_RESOURCE_NAME 3

// XPropertySet
::css::uno::Reference< ::css::beans::XPropertySetInfo > OTempFileService::getPropertySetInfo()
{
    // Create a table that map names to index values.
    // attention: properties need to be sorted by name!
    static cppu::OPropertyArrayHelper ourPropertyInfo(
        {
            css::beans::Property( "Uri", PROPERTY_HANDLE_URI, cppu::UnoType<OUString>::get(),
                css::beans::PropertyAttribute::READONLY ),
            css::beans::Property( "RemoveFile", PROPERTY_HANDLE_REMOVE_FILE, cppu::UnoType<bool>::get(),
                0 ),
            css::beans::Property( "ResourceName", PROPERTY_HANDLE_RESOURCE_NAME, cppu::UnoType<OUString>::get(),
                css::beans::PropertyAttribute::READONLY )
        },
        true );
    static css::uno::Reference< css::beans::XPropertySetInfo > xInfo( 
        ::cppu::OPropertySetHelper::createPropertySetInfo( ourPropertyInfo ) );
    return xInfo;
}
void OTempFileService::setPropertyValue( const ::rtl::OUString& aPropertyName, const ::css::uno::Any& aValue )
{
    if ( aPropertyName == "RemoveFile" )
        setRemoveFile( aValue.get<bool>() );
    else
    {
        assert(false);
        throw css::beans::UnknownPropertyException(aPropertyName);
    }
}
::css::uno::Any OTempFileService::getPropertyValue( const ::rtl::OUString& aPropertyName )
{
    if ( aPropertyName == "RemoveFile" )
        return css::uno::Any(getRemoveFile());
    else if ( aPropertyName == "ResourceName" )
        return css::uno::Any(getResourceName());
    else if ( aPropertyName == "Uri" )
        return css::uno::Any(getUri());
    else
    {
        assert(false);
        throw css::beans::UnknownPropertyException(aPropertyName);
    }
}
void OTempFileService::addPropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& /*xListener*/ )
{
    assert(false);
}
void OTempFileService::removePropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const ::css::uno::Reference< ::css::beans::XPropertyChangeListener >& /*xListener*/ )
{
    assert(false);
}
void OTempFileService::addVetoableChangeListener( const ::rtl::OUString& /*aPropertyName*/, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& /*xListener*/ )
{
    assert(false);
}
void OTempFileService::removeVetoableChangeListener( const ::rtl::OUString& /*aPropertyName*/, const ::css::uno::Reference< ::css::beans::XVetoableChangeListener >& /*xListener*/ )
{
    assert(false);
}
// XFastPropertySet
void OTempFileService::setFastPropertyValue( ::sal_Int32 nHandle, const ::css::uno::Any& aValue )
{
    switch (nHandle)
    {
        case PROPERTY_HANDLE_REMOVE_FILE: setRemoveFile( aValue.get<bool>() ); return;
    }
    assert(false);
    throw css::beans::UnknownPropertyException(OUString::number(nHandle));
}
::css::uno::Any OTempFileService::getFastPropertyValue( ::sal_Int32 nHandle )
{
    switch (nHandle)
    {
        case PROPERTY_HANDLE_REMOVE_FILE: return css::uno::Any(getRemoveFile());
        case PROPERTY_HANDLE_RESOURCE_NAME: return css::uno::Any(getResourceName());
        case PROPERTY_HANDLE_URI: return css::uno::Any(getUri());
    }
    assert(false);
    throw css::beans::UnknownPropertyException(OUString::number(nHandle));
}
// XPropertyAccess
::css::uno::Sequence< ::css::beans::PropertyValue > OTempFileService::getPropertyValues()
{
    return {
        css::beans::PropertyValue("Uri", PROPERTY_HANDLE_URI, css::uno::Any(getUri()), css::beans::PropertyState_DEFAULT_VALUE),
        css::beans::PropertyValue("RemoveFile", PROPERTY_HANDLE_REMOVE_FILE, css::uno::Any(getRemoveFile()), css::beans::PropertyState_DEFAULT_VALUE),
        css::beans::PropertyValue("ResourceName", PROPERTY_HANDLE_RESOURCE_NAME, css::uno::Any(getResourceName()), css::beans::PropertyState_DEFAULT_VALUE)
    };
}
void OTempFileService::setPropertyValues( const ::css::uno::Sequence< ::css::beans::PropertyValue >& aProps )
{
    for ( auto const & rPropVal : aProps )
        setPropertyValue( rPropVal.Name, rPropVal.Value );
}

//  XServiceInfo
sal_Bool OTempFileService::supportsService(const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}
OUString OTempFileService::getImplementationName()
{
    return "com.sun.star.io.comp.TempFile";
}
css::uno::Sequence< OUString > OTempFileService::getSupportedServiceNames()
{
    return { "com.sun.star.io.TempFile" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
unotools_OTempFileService_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new OTempFileService(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
