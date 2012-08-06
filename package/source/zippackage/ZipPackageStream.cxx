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

// MARKER( update_precomp.py ): autogen include statement, do not remove
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

#include <string.h>

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
                                    const uno::Reference< XMultiServiceFactory >& xFactory,
                                    sal_Bool bAllowRemoveOnInsert )
: m_xFactory( xFactory )
, rZipPackage( rNewPackage )
, bToBeCompressed ( sal_True )
, bToBeEncrypted ( sal_False )
, bHaveOwnKey ( sal_False )
, bIsEncrypted ( sal_False )
, m_nImportedStartKeyAlgorithm( 0 )
, m_nImportedEncryptionAlgorithm( 0 )
, m_nImportedChecksumAlgorithm( 0 )
, m_nImportedDerivedKeySize( 0 )
, m_nStreamMode( PACKAGE_STREAM_NOTSET )
, m_nMagicalHackPos( 0 )
, m_nMagicalHackSize( 0 )
, m_bHasSeekable( sal_False )
, m_bCompressedIsSetFromOutside( sal_False )
, m_bFromManifest( sal_False )
, m_bUseWinEncoding( false )
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

void ZipPackageStream::setZipEntryOnLoading( const ZipEntry &rInEntry )
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
uno::Reference< io::XInputStream > ZipPackageStream::GetOwnSeekStream()
{
    if ( !m_bHasSeekable && xStream.is() )
    {
        // The package component requires that every stream either be FROM a package or it must support XSeekable!
        // The only exception is a nonseekable stream that is provided only for storing, if such a stream
        // is accessed before commit it MUST be wrapped.
        // Wrap the stream in case it is not seekable
        xStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( xStream, m_xFactory );
        uno::Reference< io::XSeekable > xSeek( xStream, UNO_QUERY );
        if ( !xSeek.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must support XSeekable!" ) ),
                                    uno::Reference< XInterface >() );

        m_bHasSeekable = sal_True;
    }

    return xStream;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > ZipPackageStream::GetRawEncrStreamNoHeaderCopy()
{
    if ( m_nStreamMode != PACKAGE_STREAM_RAW || !GetOwnSeekStream().is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( m_xBaseEncryptionData.is() )
        throw ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Encrypted stream without encryption data!\n" ) ),
                            uno::Reference< XInterface >() );

    uno::Reference< io::XSeekable > xSeek( GetOwnSeekStream(), UNO_QUERY );
    if ( !xSeek.is() )
        throw ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must be seekable!\n" ) ),
                            uno::Reference< XInterface >() );

    // skip header
    xSeek->seek( n_ConstHeaderSize + getInitialisationVector().getLength() +
                    getSalt().getLength() + getDigest().getLength() );

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
sal_Int32 ZipPackageStream::GetEncryptionAlgorithm() const
{
    return m_nImportedEncryptionAlgorithm ? m_nImportedEncryptionAlgorithm : rZipPackage.GetEncAlgID();
}

//--------------------------------------------------------------------------
sal_Int32 ZipPackageStream::GetBlockSize() const
{
    return GetEncryptionAlgorithm() == ::com::sun::star::xml::crypto::CipherID::AES_CBC_W3C_PADDING ? 16 : 8;
}

//--------------------------------------------------------------------------
::rtl::Reference< EncryptionData > ZipPackageStream::GetEncryptionData( bool bUseWinEncoding )
{
    ::rtl::Reference< EncryptionData > xResult;
    if ( m_xBaseEncryptionData.is() )
        xResult = new EncryptionData(
            *m_xBaseEncryptionData,
            GetEncryptionKey( bUseWinEncoding ),
            GetEncryptionAlgorithm(),
            m_nImportedChecksumAlgorithm ? m_nImportedChecksumAlgorithm : rZipPackage.GetChecksumAlgID(),
            m_nImportedDerivedKeySize ? m_nImportedDerivedKeySize : rZipPackage.GetDefaultDerivedKeySize(),
            GetStartKeyGenID() );

    return xResult;
}

//--------------------------------------------------------------------------
uno::Sequence< sal_Int8 > ZipPackageStream::GetEncryptionKey( bool bUseWinEncoding )
{
    uno::Sequence< sal_Int8 > aResult;
    sal_Int32 nKeyGenID = GetStartKeyGenID();
    bUseWinEncoding = ( bUseWinEncoding || m_bUseWinEncoding );

    if ( bHaveOwnKey && m_aStorageEncryptionKeys.getLength() )
    {
        ::rtl::OUString aNameToFind;
        if ( nKeyGenID == xml::crypto::DigestID::SHA256 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
        else if ( nKeyGenID == xml::crypto::DigestID::SHA1 )
        {
            aNameToFind = bUseWinEncoding ? PACKAGE_ENCRYPTIONDATA_SHA1MS1252 : PACKAGE_ENCRYPTIONDATA_SHA1UTF8;
        }
        else
            throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "No expected key is provided!" ) ), uno::Reference< uno::XInterface >() );

        for ( sal_Int32 nInd = 0; nInd < m_aStorageEncryptionKeys.getLength(); nInd++ )
            if ( m_aStorageEncryptionKeys[nInd].Name.equals( aNameToFind ) )
                m_aStorageEncryptionKeys[nInd].Value >>= aResult;

        // empty keys are not allowed here
        // so it is not important whether there is no key, or the key is empty, it is an error
        if ( !aResult.getLength() )
            throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "No expected key is provided!" ) ), uno::Reference< uno::XInterface >() );
    }
    else
        aResult = m_aEncryptionKey;

    if ( !aResult.getLength() || !bHaveOwnKey )
        aResult = rZipPackage.GetEncryptionKey();

    return aResult;
}

//--------------------------------------------------------------------------
sal_Int32 ZipPackageStream::GetStartKeyGenID()
{
    // generally should all the streams use the same Start Key
    // but if raw copy without password takes place, we should preserve the imported algorithm
    return m_nImportedStartKeyAlgorithm ? m_nImportedStartKeyAlgorithm : rZipPackage.GetStartKeyGenID();
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > ZipPackageStream::TryToGetRawFromDataStream( sal_Bool bAddHeaderForEncr )
{
    if ( m_nStreamMode != PACKAGE_STREAM_DATA || !GetOwnSeekStream().is() || ( bAddHeaderForEncr && !bToBeEncrypted ) )
        throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    Sequence< sal_Int8 > aKey;

    if ( bToBeEncrypted )
    {
        aKey = GetEncryptionKey();
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
        uno::Reference< XSingleServiceFactory > xPackageAsFactory( static_cast< XSingleServiceFactory* >( pPackage ) );
        if ( !xPackageAsFactory.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        Sequence< Any > aArgs( 1 );
        aArgs[0] <<= xTempStream;
        pPackage->initialize( aArgs );

        // create a new package stream
        uno::Reference< XDataSinkEncrSupport > xNewPackStream( xPackageAsFactory->createInstance(), UNO_QUERY );
        if ( !xNewPackStream.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        xNewPackStream->setDataStream( static_cast< io::XInputStream* >(
                                                    new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() ) ) );

        uno::Reference< XPropertySet > xNewPSProps( xNewPackStream, UNO_QUERY );
        if ( !xNewPSProps.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        // copy all the properties of this stream to the new stream
        xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), makeAny( sMediaType ) );
        xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ), makeAny( bToBeCompressed ) );
        if ( bToBeEncrypted )
        {
            xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ENCRYPTION_KEY_PROPERTY ) ), makeAny( aKey ) );
            xNewPSProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" ) ), makeAny( sal_True ) );
        }

        // insert a new stream in the package
        uno::Reference< XUnoTunnel > xTunnel;
        Any aRoot = pPackage->getByHierarchicalName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) );
        aRoot >>= xTunnel;
        uno::Reference< container::XNameContainer > xRootNameContainer( xTunnel, UNO_QUERY );
        if ( !xRootNameContainer.is() )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        uno::Reference< XUnoTunnel > xNPSTunnel( xNewPackStream, UNO_QUERY );
        xRootNameContainer->insertByName( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "dummy" ) ), makeAny( xNPSTunnel ) );

        // commit the temporary package
        pPackage->commitChanges();

        // get raw stream from the temporary package
        uno::Reference< io::XInputStream > xInRaw;
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
        xInRaw = uno::Reference< io::XInputStream >();
        xNewPSProps = uno::Reference< XPropertySet >();
        xNPSTunnel = uno::Reference< XUnoTunnel >();
        xNewPackStream = uno::Reference< XDataSinkEncrSupport >();
        xTunnel = uno::Reference< XUnoTunnel >();
        xRootNameContainer = uno::Reference< container::XNameContainer >();

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

    ::rtl::Reference< BaseEncryptionData > xTempEncrData;
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
                xTempEncrData = new BaseEncryptionData;

                ::rtl::OUString aMediaType;
                sal_Int32 nEncAlgorithm = 0;
                sal_Int32 nChecksumAlgorithm = 0;
                sal_Int32 nDerivedKeySize = 0;
                sal_Int32 nStartKeyGenID = 0;
                if ( ZipFile::StaticFillData( xTempEncrData, nEncAlgorithm, nChecksumAlgorithm, nDerivedKeySize, nStartKeyGenID, nMagHackSize, aMediaType, GetOwnSeekStream() ) )
                {
                    // We'll want to skip the data we've just read, so calculate how much we just read
                    // and remember it
                    m_nMagicalHackPos = n_ConstHeaderSize + xTempEncrData->m_aSalt.getLength()
                                                        + xTempEncrData->m_aInitVector.getLength()
                                                        + xTempEncrData->m_aDigest.getLength()
                                                        + aMediaType.getLength() * sizeof( sal_Unicode );
                    m_nImportedEncryptionAlgorithm = nEncAlgorithm;
                    m_nImportedChecksumAlgorithm = nChecksumAlgorithm;
                    m_nImportedDerivedKeySize = nDerivedKeySize;
                    m_nImportedStartKeyAlgorithm = nStartKeyGenID;
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

    m_xBaseEncryptionData = xTempEncrData;
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
void SAL_CALL ZipPackageStream::setInputStream( const uno::Reference< io::XInputStream >& aStream )
        throw( RuntimeException )
{
    // if seekable access is required the wrapping will be done on demand
    xStream = aStream;
    m_nImportedEncryptionAlgorithm = 0;
    m_bHasSeekable = sal_False;
    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_DETECT;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawData()
        throw( RuntimeException )
{
    try
    {
        if ( IsPackageMember() )
        {
            return rZipPackage.getZipFile().getRawData( aEntry, GetEncryptionData(), bIsEncrypted, rZipPackage.GetSharedMutexRef() );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
        }
        else
            return uno::Reference < io::XInputStream > ();
    }
    catch ( ZipException & )//rException )
    {
        OSL_FAIL( "ZipException thrown" );//rException.Message);
        return uno::Reference < io::XInputStream > ();
    }
    catch ( Exception & )
    {
        OSL_FAIL( "Exception is thrown during stream wrapping!\n" );
        return uno::Reference < io::XInputStream > ();
    }
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream()
        throw( RuntimeException )
{
    try
    {
        if ( IsPackageMember() )
        {
            return rZipPackage.getZipFile().getInputStream( aEntry, GetEncryptionData(), bIsEncrypted, rZipPackage.GetSharedMutexRef() );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
        }
        else
            return uno::Reference < io::XInputStream > ();
    }
    catch ( ZipException & )//rException )
    {
        OSL_FAIL( "ZipException thrown" );//rException.Message);
        return uno::Reference < io::XInputStream > ();
    }
    catch ( Exception &ex )
    {
        OSL_FAIL( "Exception is thrown during stream wrapping!\n" );
        OSL_FAIL(OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
        (void)ex;
        return uno::Reference < io::XInputStream > ();
    }
}

// XDataSinkEncrSupport
//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getDataStream()
        throw ( packages::WrongPasswordException,
                io::IOException,
                RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( IsPackageMember() )
    {
        uno::Reference< io::XInputStream > xResult;
        try
        {
            xResult = rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData(), bIsEncrypted, rZipPackage.GetSharedMutexRef() );
        }
        catch( packages::WrongPasswordException& )
        {
            // workaround for the encrypted documents generated with the old OOo1.x bug.
            if ( rZipPackage.GetStartKeyGenID() == xml::crypto::DigestID::SHA1 && !m_bUseWinEncoding )
            {
                xResult = rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData( true ), bIsEncrypted, rZipPackage.GetSharedMutexRef() );
                m_bUseWinEncoding = true;
            }
            else
                throw;
        }
        return xResult;
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        return ZipFile::StaticGetDataFromRawStream( m_xFactory, GetOwnSeekStream(), GetEncryptionData() );
    else if ( GetOwnSeekStream().is() )
    {
        return new WrapStreamForShare( GetOwnSeekStream(), rZipPackage.GetSharedMutexRef() );
    }
    else
        return uno::Reference< io::XInputStream >();
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawStream()
        throw ( packages::NoEncryptionException,
                io::IOException,
                uno::RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( IsPackageMember() )
    {
        if ( !bIsEncrypted || !GetEncryptionData().is() )
            throw packages::NoEncryptionException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        return rZipPackage.getZipFile().getWrappedRawStream( aEntry, GetEncryptionData(), sMediaType, rZipPackage.GetSharedMutexRef() );
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
void SAL_CALL ZipPackageStream::setDataStream( const uno::Reference< io::XInputStream >& aStream )
        throw ( io::IOException,
                RuntimeException )
{
    setInputStream( aStream );
    m_nStreamMode = PACKAGE_STREAM_DATA;
}

//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setRawStream( const uno::Reference< io::XInputStream >& aStream )
        throw ( packages::EncryptionNotAllowedException,
                packages::NoRawFormatException,
                io::IOException,
                RuntimeException )
{
    // wrap the stream in case it is not seekable
    uno::Reference< io::XInputStream > xNewStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( aStream, m_xFactory );
    uno::Reference< io::XSeekable > xSeek( xNewStream, UNO_QUERY );
    if ( !xSeek.is() )
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "The stream must support XSeekable!" ) ),
                                    uno::Reference< XInterface >() );

    xSeek->seek( 0 );
    uno::Reference< io::XInputStream > xOldStream = xStream;
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
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( IsPackageMember() )
    {
        return rZipPackage.getZipFile().getRawData( aEntry, GetEncryptionData(), bIsEncrypted, rZipPackage.GetSharedMutexRef() );
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

    return uno::Reference< io::XInputStream >();
}

// XUnoTunnel

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL ZipPackageStream::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException )
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == memcmp( static_getImplementationId().getConstArray(), aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}

// XPropertySet
//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw( beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    if ( aPropertyName == "MediaType" )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE && rZipPackage.getFormat() != embed::StorageFormats::OFOPXML )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        if ( aValue >>= sMediaType )
        {
            if ( !sMediaType.isEmpty() )
            {
                if ( sMediaType.indexOf ( "text" ) != -1
                 || sMediaType == "application/vnd.sun.star.oleobject" )
                    bToBeCompressed = sal_True;
                else if ( !m_bCompressedIsSetFromOutside )
                    bToBeCompressed = sal_False;
            }
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "MediaType must be a string!\n" ) ),
                                            uno::Reference< XInterface >(),
                                            2 );

    }
    else if ( aPropertyName == "Size" )
    {
        if ( !( aValue >>= aEntry.nSize ) )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Size property!\n" ) ),
                                            uno::Reference< XInterface >(),
                                            2 );
    }
    else if ( aPropertyName == "Encrypted" )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        sal_Bool bEnc = sal_False;
        if ( aValue >>= bEnc )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bEnc && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Raw stream can not be encrypted on storing" ) ),
                                                uno::Reference< XInterface >(),
                                                2 );

            bToBeEncrypted = bEnc;
            if ( bToBeEncrypted && !m_xBaseEncryptionData.is() )
                m_xBaseEncryptionData = new BaseEncryptionData;
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Encrypted property!\n" ) ),
                                            uno::Reference< XInterface >(),
                                            2 );

    }
    else if ( aPropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        uno::Sequence< sal_Int8 > aNewKey;

        if ( !( aValue >>= aNewKey ) )
        {
            OUString sTempString;
            if ( ( aValue >>= sTempString ) )
            {
                sal_Int32 nPathLength = sTempString.getLength();
                Sequence < sal_Int8 > aSequence ( nPathLength );
                sal_Int8 *pArray = aSequence.getArray();
                const sal_Unicode *pChar = sTempString.getStr();
                for ( sal_Int16 i = 0; i < nPathLength; i++ )
                    pArray[i] = static_cast < const sal_Int8 > ( pChar[i] );
                aNewKey = aSequence;
            }
            else
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for EncryptionKey property!\n" ) ),
                                                uno::Reference< XInterface >(),
                                                2 );
        }

        if ( aNewKey.getLength() )
        {
            if ( !m_xBaseEncryptionData.is() )
                m_xBaseEncryptionData = new BaseEncryptionData;

            m_aEncryptionKey = aNewKey;
            // In case of new raw stream, the stream must not be encrypted on storing
            bHaveOwnKey = sal_True;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                bToBeEncrypted = sal_True;
        }
        else
        {
            bHaveOwnKey = sal_False;
            m_aEncryptionKey.realloc( 0 );
        }

        m_aStorageEncryptionKeys.realloc( 0 );
    }
    else if ( aPropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        if ( rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        uno::Sequence< beans::NamedValue > aKeys;
        if ( !( aValue >>= aKeys ) )
        {
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for StorageEncryptionKeys property!\n" ) ),
                                                uno::Reference< XInterface >(),
                                                2 );
        }

        if ( aKeys.getLength() )
        {
            if ( !m_xBaseEncryptionData.is() )
                m_xBaseEncryptionData = new BaseEncryptionData;

            m_aStorageEncryptionKeys = aKeys;

            // In case of new raw stream, the stream must not be encrypted on storing
            bHaveOwnKey = sal_True;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                bToBeEncrypted = sal_True;
        }
        else
        {
            bHaveOwnKey = sal_False;
            m_aStorageEncryptionKeys.realloc( 0 );
        }

        m_aEncryptionKey.realloc( 0 );
    }
    else if ( aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
    {
        sal_Bool bCompr = sal_False;

        if ( aValue >>= bCompr )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bCompr && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Raw stream can not be encrypted on storing" ) ),
                                                uno::Reference< XInterface >(),
                                                2 );

            bToBeCompressed = bCompr;
            m_bCompressedIsSetFromOutside = sal_True;
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong type for Compressed property!\n" ) ),
                                            uno::Reference< XInterface >(),
                                            2 );
    }
    else
        throw beans::UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------------------------
Any SAL_CALL ZipPackageStream::getPropertyValue( const OUString& PropertyName )
        throw( beans::UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    Any aAny;
    if ( PropertyName == "MediaType" )
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if ( PropertyName == "Size" )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else if ( PropertyName == "Encrypted" )
    {
        aAny <<= ((m_nStreamMode == PACKAGE_STREAM_RAW) ? sal_True : bToBeEncrypted);
        return aAny;
    }
    else if ( PropertyName == "WasEncrypted" )
    {
        aAny <<= bIsEncrypted;
        return aAny;
    }
    else if ( PropertyName == "Compressed" )
    {
        aAny <<= bToBeCompressed;
        return aAny;
    }
    else if ( PropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        aAny <<= m_aEncryptionKey;
        return aAny;
    }
    else if ( PropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        aAny <<= m_aStorageEncryptionKeys;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------------------------
void ZipPackageStream::setSize ( const sal_Int32 nNewSize )
{
    if ( aEntry.nCompressedSize != nNewSize )
        aEntry.nMethod = DEFLATED;
    aEntry.nSize = nNewSize;
}
//--------------------------------------------------------------------------
OUString ZipPackageStream::getImplementationName()
    throw ( RuntimeException )
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageStream" ) );
}

//--------------------------------------------------------------------------
Sequence< OUString > ZipPackageStream::getSupportedServiceNames()
    throw ( RuntimeException )
{
    Sequence< OUString > aNames( 1 );
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageStream" ) );
    return aNames;
}
//--------------------------------------------------------------------------
sal_Bool SAL_CALL ZipPackageStream::supportsService( OUString const & rServiceName )
    throw ( RuntimeException )
{
    return rServiceName == getSupportedServiceNames()[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
