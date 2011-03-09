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
#include "precompiled_package.hxx"
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>


#include <ZipPackageStream.hxx>
#include <ZipPackage.hxx>
#include <ZipFile.hxx>
#include <EncryptedDataHeader.hxx>
#include <osl/diagnose.h>
#include "wrapstreamforshare.hxx"

#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>

#include <rtl/instance.hxx>

#include <PackageConstants.hxx>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using namespace cppu;

using rtl::OUString;
namespace { struct lcl_CachedImplId : public rtl::Static< Sequence < sal_Int8 >, lcl_CachedImplId > {}; }

const ::com::sun::star::uno::Sequence < sal_Int8 >& ZipPackageStream::static_getImplementationId()
{
    return lcl_CachedImplId::get();
}

ZipPackageStream::ZipPackageStream ( ZipPackage & rNewPackage,
                                    const Reference< XMultiServiceFactory >& xFactory,
                                    sal_Bool bAllowRemoveOnInsert )
: m_xFactory( xFactory )
, rZipPackage(rNewPackage)
, bToBeCompressed ( sal_True )
, bToBeEncrypted ( sal_False )
, bHaveOwnKey ( sal_False )
, bIsEncrypted ( sal_False )
, xEncryptionData ( )
, m_nStreamMode( PACKAGE_STREAM_NOTSET )
, m_nMagicalHackPos( 0 )
, m_nMagicalHackSize( 0 )
, m_bHasSeekable( sal_False )
, m_bCompressedIsSetFromOutside( sal_False )
, m_bFromManifest( sal_False )
{
    OSL_ENSURE( m_xFactory.is(), "No factory is provided to ZipPackageStream!\n" );

    this->mbAllowRemoveOnInsert = bAllowRemoveOnInsert;

    SetFolder ( sal_False );
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = -1;
    aEntry.nTime        = -1;
    aEntry.nCrc         = -1;
    aEntry.nCompressedSize  = -1;
    aEntry.nSize        = -1;
    aEntry.nOffset      = -1;
    aEntry.nPathLen     = -1;
    aEntry.nExtraLen    = -1;

    Sequence < sal_Int8 > &rCachedImplId = lcl_CachedImplId::get();
    if ( !rCachedImplId.getLength() )
        rCachedImplId = getImplementationId();
}

ZipPackageStream::~ZipPackageStream( void )
{
}

void ZipPackageStream::setZipEntryOnLoading( const ZipEntry &rInEntry)
{
    aEntry.nVersion = rInEntry.nVersion;
    aEntry.nFlag = rInEntry.nFlag;
    aEntry.nMethod = rInEntry.nMethod;
    aEntry.nTime = rInEntry.nTime;
    aEntry.nCrc = rInEntry.nCrc;
    aEntry.nCompressedSize = rInEntry.nCompressedSize;
    aEntry.nSize = rInEntry.nSize;
    aEntry.nOffset = rInEntry.nOffset;
    aEntry.sPath = rInEntry.sPath;
    aEntry.nPathLen = rInEntry.nPathLen;
    aEntry.nExtraLen = rInEntry.nExtraLen;

    if ( aEntry.nMethod == STORED )
        bToBeCompressed = sal_False;
}

//--------------------------------------------------------------------------
void ZipPackageStream::CloseOwnStreamIfAny()
{
    if ( xStream.is() )
    {
        xStream->closeInput();
        xStream = uno::Reference< io::XInputStream >();
        m_bHasSeekable = sal_False;
    }
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream >& ZipPackageStream::GetOwnSeekStream()
{
    if ( !m_bHasSeekable && xStream.is() )
    {
        // The package component requires that every stream either be FROM a package or it must support XSeekable!
        // The only exception is a nonseekable stream that is provided only for storing, if such a stream
        // is accessed before commit it MUST be wrapped.
        // Wrap the stream in case it is not seekable
        xStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( xStream, m_xFactory );
        Reference< io::XSeekable > xSeek( xStream, UNO_QUERY );
        if ( !xSeek.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must support XSeekable!" ) ),
                                    Reference< XInterface >() );

        m_bHasSeekable = sal_True;
    }

    return xStream;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > ZipPackageStream::GetRawEncrStreamNoHeaderCopy()
{
    if ( m_nStreamMode != PACKAGE_STREAM_RAW || !GetOwnSeekStream().is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !xEncryptionData.is() )
        throw ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Encrypted stream without encryption data!\n" ) ),
                            Reference< XInterface >() );

    uno::Reference< io::XSeekable > xSeek( GetOwnSeekStream(), UNO_QUERY );
    if ( !xSeek.is() )
        throw ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must be seekable!\n" ) ),
                            Reference< XInterface >() );

    // skip header
    xSeek->seek( n_ConstHeaderSize + xEncryptionData->aInitVector.getLength() +
                    xEncryptionData->aSalt.getLength() + xEncryptionData->aDigest.getLength() );

    // create temporary stream
    uno::Reference < io::XOutputStream > xTempOut(
                        m_xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TempFile" ) ) ),
                        uno::UNO_QUERY );
    uno::Reference < io::XInputStream > xTempIn( xTempOut, UNO_QUERY );
    uno::Reference < io::XSeekable > xTempSeek( xTempOut, UNO_QUERY );
    if ( !xTempOut.is() || !xTempIn.is() || !xTempSeek.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // copy the raw stream to the temporary file starting from the current position
    ::comphelper::OStorageHelper::CopyInputToOutput( GetOwnSeekStream(), xTempOut );
    xTempOut->closeOutput();
    xTempSeek->seek( 0 );

    return xTempIn;
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > ZipPackageStream::TryToGetRawFromDataStream( sal_Bool bAddHeaderForEncr )
{
    if ( m_nStreamMode != PACKAGE_STREAM_DATA || !GetOwnSeekStream().is() || (bAddHeaderForEncr && !bToBeEncrypted) )
        throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    Sequence< sal_Int8 > aKey;

    if ( bToBeEncrypted )
    {
        aKey = ( !xEncryptionData.is() || !bHaveOwnKey ) ? rZipPackage.getEncryptionKey() :
                                                                                xEncryptionData->aKey;
        if ( !aKey.getLength() )
            throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    }

    try
    {
        // create temporary file
        uno::Reference < io::XStream > xTempStream(
                            m_xFactory->createInstance ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TempFile" ) ) ),
                            uno::UNO_QUERY );
        if ( !xTempStream.is() )
            throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        // create a package based on it
        ZipPackage* pPackage = new ZipPackage( m_xFactory );
        Reference< XSingleServiceFactory > xPackageAsFactory( static_cast< XSingleServiceFactory* >( pPackage ) );
        if ( !xPackageAsFactory.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        Sequence< Any > aArgs( 1 );
        aArgs[0] <<= xTempStream;
        pPackage->initialize( aArgs );

        // create a new package stream
        Reference< XDataSinkEncrSupport > xNewPackStream( xPackageAsFactory->createInstance(), UNO_QUERY );
        if ( !xNewPackStream.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        xNewPackStream->setDataStream( static_cast< io::XInputStream* >(
                                                    new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() ) ) );

        Reference< XPropertySet > xNewPSProps( xNewPackStream, UNO_QUERY );
        if ( !xNewPSProps.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        // copy all the properties of this stream to the new stream
        xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), makeAny( sMediaType ) );
        xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ), makeAny( bToBeCompressed ) );
        if ( bToBeEncrypted )
        {
            xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EncryptionKey" ) ), makeAny( aKey ) );
            xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" ) ), makeAny( sal_True ) );
        }

        // insert a new stream in the package
        Reference< XUnoTunnel > xTunnel;
        Any aRoot = pPackage->getByHierarchicalName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) );
        aRoot >>= xTunnel;
        Reference< container::XNameContainer > xRootNameContainer( xTunnel, UNO_QUERY );
        if ( !xRootNameContainer.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        Reference< XUnoTunnel > xNPSTunnel( xNewPackStream, UNO_QUERY );
        xRootNameContainer->insertByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "dummy" ) ), makeAny( xNPSTunnel ) );

        // commit the temporary package
        pPackage->commitChanges();

        // get raw stream from the temporary package
        Reference< io::XInputStream > xInRaw;
        if ( bAddHeaderForEncr )
            xInRaw = xNewPackStream->getRawStream();
        else
            xInRaw = xNewPackStream->getPlainRawStream();

        // create another temporary file
        uno::Reference < io::XOutputStream > xTempOut(
                            m_xFactory->createInstance ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TempFile" ) ) ),
                            uno::UNO_QUERY );
        uno::Reference < io::XInputStream > xTempIn( xTempOut, UNO_QUERY );
        uno::Reference < io::XSeekable > xTempSeek( xTempOut, UNO_QUERY );
        if ( !xTempOut.is() || !xTempIn.is() || !xTempSeek.is() )
            throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        // copy the raw stream to the temporary file
        ::comphelper::OStorageHelper::CopyInputToOutput( xInRaw, xTempOut );
        xTempOut->closeOutput();
        xTempSeek->seek( 0 );

        // close raw stream, package stream and folder
        xInRaw = Reference< io::XInputStream >();
        xNewPSProps = Reference< XPropertySet >();
        xNPSTunnel = Reference< XUnoTunnel >();
        xNewPackStream = Reference< XDataSinkEncrSupport >();
        xTunnel = Reference< XUnoTunnel >();
        xRootNameContainer = Reference< container::XNameContainer >();

        // return the stream representing the first temporary file
        return xTempIn;
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
    }

    throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------------------------
sal_Bool ZipPackageStream::ParsePackageRawStream()
{
    OSL_ENSURE( GetOwnSeekStream().is(), "A stream must be provided!\n" );

    if ( !GetOwnSeekStream().is() )
        return sal_False;

    sal_Bool bOk = sal_False;

    rtl::Reference < EncryptionData > xTempEncrData;
    sal_Int32 nMagHackSize = 0;
    Sequence < sal_Int8 > aHeader ( 4 );

    try
    {
        if ( GetOwnSeekStream()->readBytes ( aHeader, 4 ) == 4 )
        {
            const sal_Int8 *pHeader = aHeader.getConstArray();
            sal_uInt32 nHeader = ( pHeader [0] & 0xFF )       |
                                 ( pHeader [1] & 0xFF ) << 8  |
                                 ( pHeader [2] & 0xFF ) << 16 |
                                 ( pHeader [3] & 0xFF ) << 24;
            if ( nHeader == n_ConstHeader )
            {
                // this is one of our god-awful, but extremely devious hacks, everyone cheer
                xTempEncrData = new EncryptionData;

                ::rtl::OUString aMediaType;
                if ( ZipFile::StaticFillData ( xTempEncrData, nMagHackSize, aMediaType, GetOwnSeekStream() ) )
                {
                    // We'll want to skip the data we've just read, so calculate how much we just read
                    // and remember it
                    m_nMagicalHackPos = n_ConstHeaderSize + xTempEncrData->aSalt.getLength()
                                                        + xTempEncrData->aInitVector.getLength()
                                                        + xTempEncrData->aDigest.getLength()
                                                        + aMediaType.getLength() * sizeof( sal_Unicode );
                    m_nMagicalHackSize = nMagHackSize;
                    sMediaType = aMediaType;

                    bOk = sal_True;
                }
            }
        }
    }
    catch( Exception& )
    {
    }

    if ( !bOk )
    {
        // the provided stream is not a raw stream
        return sal_False;
    }

    xEncryptionData = xTempEncrData;
    SetIsEncrypted ( sal_True );
    // it's already compressed and encrypted
    bToBeEncrypted = bToBeCompressed = sal_False;

    return sal_True;
}

void ZipPackageStream::SetPackageMember( sal_Bool bNewValue )
{
    if ( bNewValue )
    {
        m_nStreamMode = PACKAGE_STREAM_PACKAGEMEMBER;
        m_nMagicalHackPos = 0;
        m_nMagicalHackSize = 0;
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_PACKAGEMEMBER )
        m_nStreamMode = PACKAGE_STREAM_NOTSET; // must be reset
}

// XActiveDataSink
//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setInputStream( const Reference< io::XInputStream >& aStream )
        throw(RuntimeException)
{
    // if seekable access is required the wrapping will be done on demand
    xStream = aStream;
    m_bHasSeekable = sal_False;
    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_DETECT;
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawData()
        throw(RuntimeException)
{
    try
    {
        if (IsPackageMember())
        {
            if ( xEncryptionData.is() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getRawData( aEntry, xEncryptionData, bIsEncrypted, rZipPackage.GetSharedMutexRef() );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
        }
        else
            return Reference < io::XInputStream > ();
    }
    catch (ZipException &)//rException)
    {
        OSL_ENSURE( 0, "ZipException thrown");//rException.Message);
        return Reference < io::XInputStream > ();
    }
    catch (Exception &)
    {
        OSL_ENSURE( 0, "Exception is thrown during stream wrapping!\n");
        return Reference < io::XInputStream > ();
    }
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream(  )
        throw(RuntimeException)
{
    try
    {
        if (IsPackageMember())
        {
            if ( xEncryptionData.is() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getInputStream( aEntry, xEncryptionData, bIsEncrypted, rZipPackage.GetSharedMutexRef() );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
        }
        else
            return Reference < io::XInputStream > ();
    }
    catch (ZipException &)//rException)
    {
        OSL_ENSURE( 0,"ZipException thrown");//rException.Message);
        return Reference < io::XInputStream > ();
    }
    catch (Exception &)
    {
        OSL_ENSURE( 0, "Exception is thrown during stream wrapping!\n");
        return Reference < io::XInputStream > ();
    }
}

// XDataSinkEncrSupport
//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getDataStream()
        throw ( packages::WrongPasswordException,
                io::IOException,
                RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( xEncryptionData.is() && !bHaveOwnKey )
        xEncryptionData->aKey = rZipPackage.getEncryptionKey();

    if (IsPackageMember())
    {
        if ( xEncryptionData.is() && !bHaveOwnKey )
            xEncryptionData->aKey = rZipPackage.getEncryptionKey();

        return rZipPackage.getZipFile().getDataStream( aEntry, xEncryptionData, bIsEncrypted, rZipPackage.GetSharedMutexRef() );
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        return ZipFile::StaticGetDataFromRawStream( GetOwnSeekStream(), xEncryptionData );
    else if ( GetOwnSeekStream().is() )
    {
        return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
    }
    else
        return uno::Reference< io::XInputStream >();
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawStream()
        throw ( packages::NoEncryptionException,
                io::IOException,
                uno::RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if (IsPackageMember())
    {
        if ( !bIsEncrypted || !xEncryptionData.is() )
            throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        return rZipPackage.getZipFile().getWrappedRawStream( aEntry, xEncryptionData, sMediaType, rZipPackage.GetSharedMutexRef() );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA && bToBeEncrypted )
            return TryToGetRawFromDataStream( sal_True );
    }

    throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}


//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setDataStream( const Reference< io::XInputStream >& aStream )
        throw ( io::IOException,
                RuntimeException )
{
    setInputStream( aStream );
    m_nStreamMode = PACKAGE_STREAM_DATA;
}

//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setRawStream( const Reference< io::XInputStream >& aStream )
        throw ( packages::EncryptionNotAllowedException,
                packages::NoRawFormatException,
                io::IOException,
                RuntimeException)
{
    // wrap the stream in case it is not seekable
    Reference< io::XInputStream > xNewStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( aStream, m_xFactory );
    Reference< io::XSeekable > xSeek( xNewStream, UNO_QUERY );
    if ( !xSeek.is() )
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must support XSeekable!" ) ),
                                    Reference< XInterface >() );

    xSeek->seek( 0 );
    Reference< io::XInputStream > xOldStream = xStream;
    xStream = xNewStream;
    if ( !ParsePackageRawStream() )
    {
        xStream = xOldStream;
        throw packages::NoRawFormatException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    }

    // the raw stream MUST have seekable access
    m_bHasSeekable = sal_True;

    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_RAW;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getPlainRawStream()
        throw ( io::IOException,
                uno::RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if (IsPackageMember())
    {
        return rZipPackage.getZipFile().getRawData( aEntry, xEncryptionData, bIsEncrypted, rZipPackage.GetSharedMutexRef() );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            // the header should not be returned here
            return GetRawEncrStreamNoHeaderCopy();
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA )
            return TryToGetRawFromDataStream( sal_False );
    }

    return Reference< io::XInputStream >();
}

// XUnoTunnel

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL ZipPackageStream::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == rtl_compareMemory( static_getImplementationId().getConstArray(), aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}

// XPropertySet
//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE && rZipPackage.getFormat() != embed::StorageFormats::OFOPXML )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        if ( aValue >>= sMediaType )
        {
            if (sMediaType.getLength() > 0)
            {
                if ( sMediaType.indexOf (OUString( RTL_CONSTASCII_USTRINGPARAM ( "text" ) ) ) != -1
                 || sMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM ( "application/vnd.sun.star.oleobject" ) ) ) )
                    bToBeCompressed = sal_True;
                else if ( !m_bCompressedIsSetFromOutside )
                    bToBeCompressed = sal_False;
            }
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "MediaType must be a string!\n" ) ),
                                            Reference< XInterface >(),
                                            2 );

    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
    {
        if ( !( aValue >>= aEntry.nSize ) )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Size property!\n" ) ),
                                            Reference< XInterface >(),
                                            2 );
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Encrypted") ) )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        sal_Bool bEnc = sal_False;
        if ( aValue >>= bEnc )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bEnc && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Raw stream can not be encrypted on storing" ) ),
                                                Reference< XInterface >(),
                                                2 );

            bToBeEncrypted = bEnc;
            if ( bToBeEncrypted && !xEncryptionData.is())
                xEncryptionData = new EncryptionData;
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Encrypted property!\n" ) ),
                                            Reference< XInterface >(),
                                            2 );

    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        Sequence < sal_Int8 > aNewKey;

        if ( !( aValue >>= aNewKey ) )
        {
            OUString sTempString;
            if ( ( aValue >>= sTempString ) )
            {
                sal_Int32 nPathLength = sTempString.getLength();
                Sequence < sal_Int8 > aSequence ( nPathLength );
                sal_Int8 *pArray = aSequence.getArray();
                const sal_Unicode *pChar = sTempString.getStr();
                for ( sal_Int16 i = 0; i < nPathLength; i++)
                    pArray[i] = static_cast < const sal_Int8 > (pChar[i]);
                aNewKey = aSequence;
            }
            else
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for EncryptionKey property!\n" ) ),
                                                Reference< XInterface >(),
                                                2 );
        }

        if ( aNewKey.getLength() )
        {
            if ( !xEncryptionData.is())
                xEncryptionData = new EncryptionData;

            xEncryptionData->aKey = aNewKey;
            // In case of new raw stream, the stream must not be encrypted on storing
            bHaveOwnKey = sal_True;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                bToBeEncrypted = sal_True;
        }
        else
            bHaveOwnKey = sal_False;
    }
    else if (aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
    {
        sal_Bool bCompr = sal_False;

        if ( aValue >>= bCompr )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bCompr && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Raw stream can not be encrypted on storing" ) ),
                                                Reference< XInterface >(),
                                                2 );

            bToBeCompressed = bCompr;
            m_bCompressedIsSetFromOutside = sal_True;
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Compressed property!\n" ) ),
                                            Reference< XInterface >(),
                                            2 );
    }
    else
        throw beans::UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------------------------
Any SAL_CALL ZipPackageStream::getPropertyValue( const OUString& PropertyName )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Encrypted" ) ) )
    {
        aAny <<= ( m_nStreamMode == PACKAGE_STREAM_RAW ) ? sal_True : bToBeEncrypted;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WasEncrypted" ) ) )
    {
        aAny <<= bIsEncrypted;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
    {
        aAny <<= bToBeCompressed;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EncryptionKey" ) ) )
    {
        aAny <<= !xEncryptionData.is() ? Sequence < sal_Int8 > () : xEncryptionData->aKey;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------------------------
void ZipPackageStream::setSize (const sal_Int32 nNewSize)
{
    if (aEntry.nCompressedSize != nNewSize )
        aEntry.nMethod = DEFLATED;
    aEntry.nSize = nNewSize;
}
//--------------------------------------------------------------------------
OUString ZipPackageStream::getImplementationName()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageStream" ) );
}

//--------------------------------------------------------------------------
Sequence< OUString > ZipPackageStream::getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageStream" ) );
    return aNames;
}
//--------------------------------------------------------------------------
sal_Bool SAL_CALL ZipPackageStream::supportsService( OUString const & rServiceName )
    throw (RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
