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

#include <ZipPackageStream.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/packages/NoRawFormatException.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/KDFID.hpp>

#include <CRC32.hxx>
#include <ZipOutputEntry.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackage.hxx>
#include <ZipFile.hxx>
#include <EncryptedDataHeader.hxx>
#include <osl/diagnose.h>
#include "wrapstreamforshare.hxx"

#include <comphelper/seekableinput.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/tempfile.hxx>

#include <rtl/random.h>
#include <sal/log.hxx>
#include <o3tl/unreachable.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <PackageConstants.hxx>

#include <algorithm>
#include <cstddef>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using namespace cppu;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

ZipPackageStream::ZipPackageStream ( ZipPackage & rNewPackage,
                                    const uno::Reference< XComponentContext >& xContext,
                                    sal_Int32 nFormat,
                                    bool bAllowRemoveOnInsert )
: m_rZipPackage( rNewPackage )
, m_bToBeCompressed ( true )
, m_bToBeEncrypted ( false )
, m_bHaveOwnKey ( false )
, m_bIsEncrypted ( false )
, m_nImportedStartKeyAlgorithm( 0 )
, m_nImportedEncryptionAlgorithm( 0 )
, m_nImportedDerivedKeySize( 0 )
, m_nStreamMode( PACKAGE_STREAM_NOTSET )
, m_nMagicalHackPos( 0 )
, m_nMagicalHackSize( 0 )
, m_nOwnStreamOrigSize( 0 )
, m_bHasSeekable( false )
, m_bCompressedIsSetFromOutside( false )
, m_bFromManifest( false )
, m_bUseWinEncoding( false )
, m_bRawStream( false )
{
    m_xContext = xContext;
    m_nFormat = nFormat;
    mbAllowRemoveOnInsert = bAllowRemoveOnInsert;
    SetFolder ( false );
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
}

ZipPackageStream::~ZipPackageStream()
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
        m_bToBeCompressed = false;
}

uno::Reference< io::XInputStream > const & ZipPackageStream::GetOwnSeekStream()
{
    if ( !m_bHasSeekable && m_xStream.is() )
    {
        // The package component requires that every stream either be FROM a package or it must support XSeekable!
        // The only exception is a nonseekable stream that is provided only for storing, if such a stream
        // is accessed before commit it MUST be wrapped.
        // Wrap the stream in case it is not seekable
        m_xStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( m_xStream, m_xContext );
        uno::Reference< io::XSeekable > xSeek( m_xStream, UNO_QUERY_THROW );

        m_bHasSeekable = true;
    }

    return m_xStream;
}

uno::Reference< io::XInputStream > ZipPackageStream::GetRawEncrStreamNoHeaderCopy()
{
    if ( m_nStreamMode != PACKAGE_STREAM_RAW || !GetOwnSeekStream().is() )
        throw io::IOException(THROW_WHERE );

    if ( m_xBaseEncryptionData.is() )
        throw ZipIOException(THROW_WHERE "Encrypted stream without encryption data!" );

    uno::Reference< io::XSeekable > xSeek( GetOwnSeekStream(), UNO_QUERY );
    if ( !xSeek.is() )
        throw ZipIOException(THROW_WHERE "The stream must be seekable!" );

    // skip header
    xSeek->seek( n_ConstHeaderSize + m_xBaseEncryptionData->m_aInitVector.getLength() +
                    m_xBaseEncryptionData->m_aSalt.getLength() + m_xBaseEncryptionData->m_aDigest.getLength() );

    // create temporary stream
    rtl::Reference < utl::TempFileFastService > xTempFile = new utl::TempFileFastService;
    uno::Reference < io::XInputStream > xTempIn = xTempFile->getInputStream();

    // copy the raw stream to the temporary file starting from the current position
    ::comphelper::OStorageHelper::CopyInputToOutput( GetOwnSeekStream(), xTempFile );
    xTempFile->closeOutput();
    xTempFile->seek( 0 );

    return xTempIn;
}

sal_Int32 ZipPackageStream::GetEncryptionAlgorithm() const
{
    return m_nImportedEncryptionAlgorithm ? m_nImportedEncryptionAlgorithm : m_rZipPackage.GetEncAlgID();
}

sal_Int32 ZipPackageStream::GetIVSize() const
{
    switch (GetEncryptionAlgorithm())
    {
        case css::xml::crypto::CipherID::BLOWFISH_CFB_8:
            return 8;
        case css::xml::crypto::CipherID::AES_CBC_W3C_PADDING:
            return 16;
        case css::xml::crypto::CipherID::AES_GCM_W3C:
            return 12;
        default:
            O3TL_UNREACHABLE;
    }
}

::rtl::Reference<EncryptionData> ZipPackageStream::GetEncryptionData(Bugs const bugs)
{
    ::rtl::Reference< EncryptionData > xResult;
    if ( m_xBaseEncryptionData.is() )
        xResult = new EncryptionData(
            *m_xBaseEncryptionData,
            GetEncryptionKey(bugs),
            GetEncryptionAlgorithm(),
            m_oImportedChecksumAlgorithm ? m_oImportedChecksumAlgorithm : m_rZipPackage.GetChecksumAlgID(),
            m_nImportedDerivedKeySize ? m_nImportedDerivedKeySize : m_rZipPackage.GetDefaultDerivedKeySize(),
            GetStartKeyGenID(),
            bugs != Bugs::None);

    return xResult;
}

uno::Sequence<sal_Int8> ZipPackageStream::GetEncryptionKey(Bugs const bugs)
{
    uno::Sequence< sal_Int8 > aResult;
    sal_Int32 nKeyGenID = GetStartKeyGenID();
    bool const bUseWinEncoding = (bugs == Bugs::WinEncodingWrongSHA1 || m_bUseWinEncoding);

    if ( m_bHaveOwnKey && m_aStorageEncryptionKeys.hasElements() )
    {
        OUString aNameToFind;
        if ( nKeyGenID == xml::crypto::DigestID::SHA256 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
        else if ( nKeyGenID == xml::crypto::DigestID::SHA1 )
        {
            aNameToFind = bUseWinEncoding
                ? PACKAGE_ENCRYPTIONDATA_SHA1MS1252
                : (bugs == Bugs::WrongSHA1)
                    ? PACKAGE_ENCRYPTIONDATA_SHA1UTF8
                    : PACKAGE_ENCRYPTIONDATA_SHA1CORRECT;
        }
        else
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );

        for ( const auto& rKey : std::as_const(m_aStorageEncryptionKeys) )
            if ( rKey.Name == aNameToFind )
                rKey.Value >>= aResult;

        // empty keys are not allowed here
        // so it is not important whether there is no key, or the key is empty, it is an error
        if ( !aResult.hasElements() )
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );
    }
    else
        aResult = m_aEncryptionKey;

    if ( !aResult.hasElements() || !m_bHaveOwnKey )
        aResult = m_rZipPackage.GetEncryptionKey();

    return aResult;
}

sal_Int32 ZipPackageStream::GetStartKeyGenID() const
{
    // generally should all the streams use the same Start Key
    // but if raw copy without password takes place, we should preserve the imported algorithm
    return m_nImportedStartKeyAlgorithm ? m_nImportedStartKeyAlgorithm : m_rZipPackage.GetStartKeyGenID();
}

uno::Reference< io::XInputStream > ZipPackageStream::TryToGetRawFromDataStream( bool bAddHeaderForEncr )
{
    if ( m_nStreamMode != PACKAGE_STREAM_DATA || !GetOwnSeekStream().is() || ( bAddHeaderForEncr && !m_bToBeEncrypted ) )
        throw packages::NoEncryptionException(THROW_WHERE );

    Sequence< sal_Int8 > aKey;

    if ( m_bToBeEncrypted )
    {
        aKey = GetEncryptionKey();
        if ( !aKey.hasElements() )
            throw packages::NoEncryptionException(THROW_WHERE );
    }

    try
    {
        // create temporary file
        uno::Reference < io::XStream > xTempStream(new utl::TempFileFastService);

        // create a package based on it
        rtl::Reference<ZipPackage> pPackage = new ZipPackage( m_xContext );

        Sequence< Any > aArgs{ Any(xTempStream) };
        pPackage->initialize( aArgs );

        // create a new package stream
        uno::Reference< XDataSinkEncrSupport > xNewPackStream( pPackage->createInstance(), UNO_QUERY_THROW );
        xNewPackStream->setDataStream(
            new WrapStreamForShare(GetOwnSeekStream(), m_rZipPackage.GetSharedMutexRef()));

        uno::Reference< XPropertySet > xNewPSProps( xNewPackStream, UNO_QUERY_THROW );

        // copy all the properties of this stream to the new stream
        xNewPSProps->setPropertyValue("MediaType", Any( msMediaType ) );
        xNewPSProps->setPropertyValue("Compressed", Any( m_bToBeCompressed ) );
        if ( m_bToBeEncrypted )
        {
            xNewPSProps->setPropertyValue(ENCRYPTION_KEY_PROPERTY, Any( aKey ) );
            xNewPSProps->setPropertyValue("Encrypted", Any( true ) );
        }

        // insert a new stream in the package
        uno::Reference< XInterface > xTmp;
        Any aRoot = pPackage->getByHierarchicalName("/");
        aRoot >>= xTmp;
        uno::Reference< container::XNameContainer > xRootNameContainer( xTmp, UNO_QUERY_THROW );

        uno::Reference< XInterface > xNPSDummy( xNewPackStream, UNO_QUERY );
        xRootNameContainer->insertByName("dummy", Any( xNPSDummy ) );

        // commit the temporary package
        pPackage->commitChanges();

        // get raw stream from the temporary package
        uno::Reference< io::XInputStream > xInRaw;
        if ( bAddHeaderForEncr )
            xInRaw = xNewPackStream->getRawStream();
        else
            xInRaw = xNewPackStream->getPlainRawStream();

        // create another temporary file
        rtl::Reference < utl::TempFileFastService > xTempOut = new utl::TempFileFastService;
        uno::Reference < io::XInputStream > xTempIn( xTempOut );

        // copy the raw stream to the temporary file
        ::comphelper::OStorageHelper::CopyInputToOutput( xInRaw, xTempOut );
        xTempOut->closeOutput();
        xTempOut->seek( 0 );

        // close raw stream, package stream and folder
        xInRaw.clear();
        xNewPSProps.clear();
        xNPSDummy.clear();
        xNewPackStream.clear();
        xTmp.clear();
        xRootNameContainer.clear();

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

    throw io::IOException(THROW_WHERE );
}

// presumably the purpose of this is to transfer encrypted streams between
// storages, needed for password-protected macros in documents, which is
// tragically a feature that exists
bool ZipPackageStream::ParsePackageRawStream()
{
    OSL_ENSURE( GetOwnSeekStream().is(), "A stream must be provided!" );

    if ( !GetOwnSeekStream().is() )
        return false;

    bool bOk = false;

    ::rtl::Reference< BaseEncryptionData > xTempEncrData;
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

                OUString aMediaType;
                sal_Int32 nEncAlgorithm = 0;
                sal_Int32 nChecksumAlgorithm = 0;
                sal_Int32 nDerivedKeySize = 0;
                sal_Int32 nStartKeyGenID = 0;
                sal_Int32 nMagHackSize = 0;
                if ( ZipFile::StaticFillData( xTempEncrData, nEncAlgorithm, nChecksumAlgorithm, nDerivedKeySize, nStartKeyGenID, nMagHackSize, aMediaType, GetOwnSeekStream() ) )
                {
                    // We'll want to skip the data we've just read, so calculate how much we just read
                    // and remember it
                    m_nMagicalHackPos = n_ConstHeaderSize + xTempEncrData->m_aSalt.getLength()
                                                        + xTempEncrData->m_aInitVector.getLength()
                                                        + xTempEncrData->m_aDigest.getLength()
                                                        + aMediaType.getLength() * sizeof( sal_Unicode );
                    m_nImportedEncryptionAlgorithm = nEncAlgorithm;
                    if (nChecksumAlgorithm == 0)
                    {
                        m_oImportedChecksumAlgorithm.reset();
                    }
                    else
                    {
                        m_oImportedChecksumAlgorithm.emplace(nChecksumAlgorithm);
                    }
                    m_nImportedDerivedKeySize = nDerivedKeySize;
                    m_nImportedStartKeyAlgorithm = nStartKeyGenID;
                    m_nMagicalHackSize = nMagHackSize;
                    msMediaType = aMediaType;

                    bOk = true;
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
        return false;
    }

    m_xBaseEncryptionData = xTempEncrData;
    SetIsEncrypted ( true );
    // it's already compressed and encrypted
    m_bToBeEncrypted = m_bToBeCompressed = false;

    return true;
}

static void ImplSetStoredData( ZipEntry & rEntry, uno::Reference< io::XInputStream> const & rStream )
{
    // It's very annoying that we have to do this, but lots of zip packages
    // don't allow data descriptors for STORED streams, meaning we have to
    // know the size and CRC32 of uncompressed streams before we actually
    // write them !
    CRC32 aCRC32;
    rEntry.nMethod = STORED;
    rEntry.nCompressedSize = rEntry.nSize = aCRC32.updateStream ( rStream );
    rEntry.nCrc = aCRC32.getValue();
}

bool ZipPackageStream::saveChild(
        const OUString &rPath,
        std::vector < uno::Sequence < beans::PropertyValue > > &rManList,
        ZipOutputStream & rZipOut,
        const uno::Sequence < sal_Int8 >& rEncryptionKey,
        ::std::optional<sal_Int32> const oPBKDF2IterationCount,
        ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> const oArgon2Args)
{
    bool bSuccess = true;

    static constexpr OUString sDigestProperty (u"Digest"_ustr);
    static constexpr OUString sEncryptionAlgProperty    (u"EncryptionAlgorithm"_ustr);
    static constexpr OUString sStartKeyAlgProperty  (u"StartKeyAlgorithm"_ustr);
    static constexpr OUString sDigestAlgProperty    (u"DigestAlgorithm"_ustr);
    static constexpr OUString sDerivedKeySizeProperty  (u"DerivedKeySize"_ustr);

    uno::Sequence < beans::PropertyValue > aPropSet (PKG_SIZE_NOENCR_MNFST);

    // In case the entry we are reading is also the entry we are writing, we will
    // store the ZipEntry data in pTempEntry

    // if pTempEntry is necessary, it will be released and passed to the ZipOutputStream
    // and be deleted in the ZipOutputStream destructor
    std::unique_ptr < ZipEntry > pAutoTempEntry ( new ZipEntry(aEntry) );
    ZipEntry* pTempEntry = pAutoTempEntry.get();

    pTempEntry->sPath = rPath;
    pTempEntry->nPathLen = static_cast<sal_Int16>( OUStringToOString( pTempEntry->sPath, RTL_TEXTENCODING_UTF8 ).getLength() );

    const bool bToBeEncrypted = m_bToBeEncrypted && (rEncryptionKey.hasElements() || m_bHaveOwnKey);
    const bool bToBeCompressed = bToBeEncrypted || m_bToBeCompressed;

    auto pPropSet = aPropSet.getArray();
    pPropSet[PKG_MNFST_MEDIATYPE].Name = "MediaType";
    pPropSet[PKG_MNFST_MEDIATYPE].Value <<= GetMediaType( );
    pPropSet[PKG_MNFST_VERSION].Name = "Version";
    pPropSet[PKG_MNFST_VERSION].Value <<= OUString(); // no version is stored for streams currently
    pPropSet[PKG_MNFST_FULLPATH].Name = "FullPath";
    pPropSet[PKG_MNFST_FULLPATH].Value <<= pTempEntry->sPath;

    OSL_ENSURE( m_nStreamMode != PACKAGE_STREAM_NOTSET, "Unacceptable ZipPackageStream mode!" );

    m_bRawStream = false;
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        m_bRawStream = ParsePackageRawStream();
    else if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        m_bRawStream = true;

    bool bBackgroundThreadDeflate = false;
    bool bTransportOwnEncrStreamAsRaw = false;
    // During the storing the original size of the stream can be changed
    // TODO/LATER: get rid of this hack
    m_nOwnStreamOrigSize = m_bRawStream ? m_nMagicalHackSize : aEntry.nSize;

    bool bUseNonSeekableAccess = false;
    uno::Reference < io::XInputStream > xStream;
    if ( !IsPackageMember() && !m_bRawStream && !bToBeEncrypted && bToBeCompressed )
    {
        // the stream is not a package member, not a raw stream,
        // it should not be encrypted and it should be compressed,
        // in this case nonseekable access can be used

        xStream = m_xStream;
        uno::Reference < io::XSeekable > xSeek ( xStream, uno::UNO_QUERY );

        bUseNonSeekableAccess = ( xStream.is() && !xSeek.is() );
    }

    if ( !bUseNonSeekableAccess )
    {
        xStream = getRawData();

        if ( !xStream.is() )
        {
            OSL_FAIL( "ZipPackageStream didn't have a stream associated with it, skipping!" );
            bSuccess = false;
            return bSuccess;
        }

        uno::Reference < io::XSeekable > xSeek ( xStream, uno::UNO_QUERY );
        try
        {
            if ( xSeek.is() )
            {
                // If the stream is a raw one, then we should be positioned
                // at the beginning of the actual data
                if ( !bToBeCompressed || m_bRawStream )
                {
                    // The raw stream can neither be encrypted nor connected
                    OSL_ENSURE( !m_bRawStream || !(bToBeCompressed || bToBeEncrypted), "The stream is already encrypted!" );
                    xSeek->seek ( m_bRawStream ? m_nMagicalHackPos : 0 );
                    ImplSetStoredData ( *pTempEntry, xStream );

                    // TODO/LATER: Get rid of hacks related to switching of Flag Method and Size properties!
                }
                else if ( bToBeEncrypted )
                {
                    // this is the correct original size
                    pTempEntry->nSize = xSeek->getLength();
                    m_nOwnStreamOrigSize = pTempEntry->nSize;
                }

                xSeek->seek ( 0 );
            }
            else
            {
                // Okay, we don't have an xSeekable stream. This is possibly bad.
                // check if it's one of our own streams, if it is then we know that
                // each time we ask for it we'll get a new stream that will be
                // at position zero...otherwise, assert and skip this stream...
                if ( IsPackageMember() )
                {
                    // if the password has been changed then the stream should not be package member any more
                    if ( m_bIsEncrypted && m_bToBeEncrypted )
                    {
                        // Should be handled close to the raw stream handling
                        bTransportOwnEncrStreamAsRaw = true;
                        pTempEntry->nMethod = STORED;

                        // TODO/LATER: get rid of this situation
                        // this size should be different from the one that will be stored in manifest.xml
                        // it is used in storing algorithms and after storing the correct size will be set
                        pTempEntry->nSize = pTempEntry->nCompressedSize;
                    }
                }
                else
                {
                    bSuccess = false;
                    return bSuccess;
                }
            }
        }
        catch ( uno::Exception& )
        {
            bSuccess = false;
            return bSuccess;
        }

        if ( bToBeEncrypted || m_bRawStream || bTransportOwnEncrStreamAsRaw )
        {
            if ( bToBeEncrypted && !bTransportOwnEncrStreamAsRaw )
            {
                uno::Sequence<sal_Int8> aSalt(16);
                // note: for GCM it's particularly important that IV is unique
                uno::Sequence<sal_Int8> aVector(GetIVSize());
                if (rtl_random_getBytes(nullptr, aSalt.getArray(), 16) != rtl_Random_E_None)
                {
                    throw uno::RuntimeException("rtl_random_getBytes failed");
                }
                if (rtl_random_getBytes(nullptr, aVector.getArray(), aVector.getLength()) != rtl_Random_E_None)
                {
                    throw uno::RuntimeException("rtl_random_getBytes failed");
                }
                if ( !m_bHaveOwnKey )
                {
                    m_aEncryptionKey = rEncryptionKey;
                    m_aStorageEncryptionKeys.realloc( 0 );
                }

                setInitialisationVector ( aVector );
                setSalt ( aSalt );
                setIterationCount(oPBKDF2IterationCount);
                setArgon2Args(oArgon2Args);
            }

            // last property is digest, which is inserted later if we didn't have
            // a magic header
            aPropSet.realloc(PKG_SIZE_ENCR_MNFST);
            pPropSet = aPropSet.getArray();
            pPropSet[PKG_MNFST_INIVECTOR].Name = "InitialisationVector";
            pPropSet[PKG_MNFST_INIVECTOR].Value <<= m_xBaseEncryptionData->m_aInitVector;
            pPropSet[PKG_MNFST_SALT].Name = "Salt";
            pPropSet[PKG_MNFST_SALT].Value <<= m_xBaseEncryptionData->m_aSalt;
            if (m_xBaseEncryptionData->m_oArgon2Args)
            {
                pPropSet[PKG_MNFST_KDF].Name = "KeyDerivationFunction";
                pPropSet[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::Argon2id;
                pPropSet[PKG_MNFST_ARGON2ARGS].Name = "Argon2Args";
                uno::Sequence<sal_Int32> const args{
                    ::std::get<0>(*m_xBaseEncryptionData->m_oArgon2Args),
                    ::std::get<1>(*m_xBaseEncryptionData->m_oArgon2Args),
                    ::std::get<2>(*m_xBaseEncryptionData->m_oArgon2Args) };
                pPropSet[PKG_MNFST_ARGON2ARGS].Value <<= args;
            }
            else if (m_xBaseEncryptionData->m_oPBKDFIterationCount)
            {
                pPropSet[PKG_MNFST_KDF].Name = "KeyDerivationFunction";
                pPropSet[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::PBKDF2;
                pPropSet[PKG_MNFST_ITERATION].Name = "IterationCount";
                pPropSet[PKG_MNFST_ITERATION].Value <<= *m_xBaseEncryptionData->m_oPBKDFIterationCount;
            }
            else
            {
                pPropSet[PKG_MNFST_KDF].Name = "KeyDerivationFunction";
                pPropSet[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P;
            }

            // Need to store the uncompressed size in the manifest
            OSL_ENSURE( m_nOwnStreamOrigSize >= 0, "The stream size was not correctly initialized!" );
            pPropSet[PKG_MNFST_UCOMPSIZE].Name = "Size";
            pPropSet[PKG_MNFST_UCOMPSIZE].Value <<= m_nOwnStreamOrigSize;

            if ( m_bRawStream || bTransportOwnEncrStreamAsRaw )
            {
                ::rtl::Reference< EncryptionData > xEncData = GetEncryptionData();
                if ( !xEncData.is() )
                    throw uno::RuntimeException();

                pPropSet[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
                pPropSet[PKG_MNFST_ENCALG].Value <<= xEncData->m_nEncAlg;
                pPropSet[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
                pPropSet[PKG_MNFST_STARTALG].Value <<= xEncData->m_nStartKeyGenID;
                if (xEncData->m_oCheckAlg)
                {
                    assert(xEncData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C);
                    pPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                    pPropSet[PKG_MNFST_DIGEST].Value <<= m_xBaseEncryptionData->m_aDigest;
                    pPropSet[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
                    pPropSet[PKG_MNFST_DIGESTALG].Value <<= *xEncData->m_oCheckAlg;
                }
                pPropSet[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
                pPropSet[PKG_MNFST_DERKEYSIZE].Value <<= xEncData->m_nDerivedKeySize;
            }
        }
    }

    // If the entry is already stored in the zip file in the format we
    // want for this write...copy it raw
    if ( !bUseNonSeekableAccess
      && ( m_bRawStream || bTransportOwnEncrStreamAsRaw
        || ( IsPackageMember() && !bToBeEncrypted
          && ( ( aEntry.nMethod == DEFLATED && bToBeCompressed )
            || ( aEntry.nMethod == STORED && !bToBeCompressed ) ) ) ) )
    {
        // If it's a PackageMember, then it's an unbuffered stream and we need
        // to get a new version of it as we can't seek backwards.
        if ( IsPackageMember() )
        {
            xStream = getRawData();
            if ( !xStream.is() )
            {
                // Make sure that we actually _got_ a new one !
                bSuccess = false;
                return bSuccess;
            }
        }

        try
        {
            if ( m_bRawStream )
                xStream->skipBytes( m_nMagicalHackPos );

            ZipOutputStream::setEntry(pTempEntry);
            rZipOut.writeLOC(pTempEntry);
            // coverity[leaked_storage] - the entry is provided to the ZipOutputStream that will delete it
            pAutoTempEntry.release();

            uno::Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
            sal_Int32 nLength;

            do
            {
                nLength = xStream->readBytes( aSeq, n_ConstBufferSize );
                if (nLength != n_ConstBufferSize)
                    aSeq.realloc(nLength);

                rZipOut.rawWrite(aSeq);
            }
            while ( nLength == n_ConstBufferSize );

            rZipOut.rawCloseEntry();
        }
        catch ( ZipException& )
        {
            bSuccess = false;
        }
        catch ( io::IOException& )
        {
            bSuccess = false;
        }
    }
    else
    {
        // This stream is definitely not a raw stream

        // If nonseekable access is used the stream should be at the beginning and
        // is useless after the storing. Thus if the storing fails the package should
        // be thrown away ( as actually it is done currently )!
        // To allow to reuse the package after the error, the optimization must be removed!

        // If it's a PackageMember, then our previous reference held a 'raw' stream
        // so we need to re-get it, unencrypted, uncompressed and positioned at the
        // beginning of the stream
        if ( IsPackageMember() )
        {
            xStream = getInputStream();
            if ( !xStream.is() )
            {
                // Make sure that we actually _got_ a new one !
                bSuccess = false;
                return bSuccess;
            }
        }

        if ( bToBeCompressed )
        {
            pTempEntry->nMethod = DEFLATED;
            pTempEntry->nCrc = -1;
            pTempEntry->nCompressedSize = pTempEntry->nSize = -1;
        }

        uno::Reference< io::XSeekable > xSeek(xStream, uno::UNO_QUERY);
        // It's not worth to deflate jpegs to save ~1% in a slow process
        // Unfortunately, does not work for streams protected by password
        if (xSeek.is() && msMediaType.endsWith("/jpeg") && !m_bToBeEncrypted && !m_bToBeCompressed)
        {
            ImplSetStoredData(*pTempEntry, xStream);
            xSeek->seek(0);
        }

        try
        {
            ZipOutputStream::setEntry(pTempEntry);
            // the entry is provided to the ZipOutputStream that will delete it
            pAutoTempEntry.release();

            if (pTempEntry->nMethod == STORED)
            {
                sal_Int32 nLength;
                uno::Sequence< sal_Int8 > aSeq(n_ConstBufferSize);
                rZipOut.writeLOC(pTempEntry, bToBeEncrypted);
                do
                {
                    nLength = xStream->readBytes(aSeq, n_ConstBufferSize);
                    if (nLength != n_ConstBufferSize)
                        aSeq.realloc(nLength);

                    rZipOut.rawWrite(aSeq);
                }
                while ( nLength == n_ConstBufferSize );
                rZipOut.rawCloseEntry(bToBeEncrypted);
            }
            else
            {
                // tdf#89236 Encrypting in a background thread does not work
                bBackgroundThreadDeflate = !bToBeEncrypted;
                // Do not deflate small streams using threads. XSeekable's getLength()
                // gives the full size, XInputStream's available() may not be
                // the full size, but it appears that at this point it usually is.
                sal_Int64 estimatedSize = xSeek.is() ? xSeek->getLength() : xStream->available();

                if (estimatedSize > 1000000)
                {
                    // Use ThreadDeflater which will split the stream into blocks and compress
                    // them in threads, but not in background (i.e. writeStream() will block).
                    // This is suitable for large data.
                    bBackgroundThreadDeflate = false;
                    rZipOut.writeLOC(pTempEntry, bToBeEncrypted);
                    ZipOutputEntryParallel aZipEntry(rZipOut.getStream(), m_xContext, *pTempEntry, this, bToBeEncrypted);
                    aZipEntry.writeStream(xStream);
                    rZipOut.rawCloseEntry(bToBeEncrypted);
                }
                else if (bBackgroundThreadDeflate && estimatedSize > 100000)
                {
                    // tdf#93553 limit to a useful amount of pending tasks. Having way too many
                    // tasks pending may use a lot of memory. Take number of available
                    // cores and allow 4-times the amount for having the queue well filled. The
                    // 2nd parameter is the time to wait between cleanups in 10th of a second.
                    // Both values may be added to the configuration settings if needed.
                    static std::size_t nAllowedTasks(comphelper::ThreadPool::getPreferredConcurrency() * 4); //TODO: overflow
                    rZipOut.reduceScheduledThreadTasksToGivenNumberOrLess(nAllowedTasks);

                    // Start a new thread task deflating this zip entry
                    ZipOutputEntryInThread *pZipEntry = new ZipOutputEntryInThread(
                            m_xContext, *pTempEntry, this, bToBeEncrypted);
                    rZipOut.addDeflatingThreadTask( pZipEntry,
                            pZipEntry->createTask( rZipOut.getThreadTaskTag(), xStream) );
                }
                else
                {
                    bBackgroundThreadDeflate = false;
                    rZipOut.writeLOC(pTempEntry, bToBeEncrypted);
                    ZipOutputEntry aZipEntry(rZipOut.getStream(), m_xContext, *pTempEntry, this, bToBeEncrypted);
                    aZipEntry.writeStream(xStream);
                    rZipOut.rawCloseEntry(bToBeEncrypted);
                }
            }
        }
        catch ( ZipException& )
        {
            bSuccess = false;
        }
        catch ( io::IOException& )
        {
            bSuccess = false;
        }

        if ( bToBeEncrypted )
        {
            ::rtl::Reference< EncryptionData > xEncData = GetEncryptionData();
            if ( !xEncData.is() )
                throw uno::RuntimeException();

            // very confusing: half the encryption properties are
            // unconditionally added above and the other half conditionally;
            // assert that we have the expected group and not duplicates
            assert(std::any_of(aPropSet.begin(), aPropSet.end(), [](auto const& it){ return it.Name == "Salt"; }));
            assert(!std::any_of(aPropSet.begin(), aPropSet.end(), [](auto const& it){ return it.Name == sEncryptionAlgProperty; }));

            pPropSet[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            pPropSet[PKG_MNFST_ENCALG].Value <<= xEncData->m_nEncAlg;
            pPropSet[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
            pPropSet[PKG_MNFST_STARTALG].Value <<= xEncData->m_nStartKeyGenID;
            if (xEncData->m_oCheckAlg)
            {
                assert(xEncData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C);
                pPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                pPropSet[PKG_MNFST_DIGEST].Value <<= m_xBaseEncryptionData->m_aDigest;
                pPropSet[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
                pPropSet[PKG_MNFST_DIGESTALG].Value <<= *xEncData->m_oCheckAlg;
            }
            pPropSet[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
            pPropSet[PKG_MNFST_DERKEYSIZE].Value <<= xEncData->m_nDerivedKeySize;

            SetIsEncrypted ( true );
        }
    }

    if (bSuccess && !bBackgroundThreadDeflate)
        successfullyWritten(pTempEntry);

    if ( aPropSet.hasElements()
      && ( m_nFormat == embed::StorageFormats::PACKAGE || m_nFormat == embed::StorageFormats::OFOPXML ) )
        rManList.push_back( aPropSet );

    return bSuccess;
}

void ZipPackageStream::successfullyWritten( ZipEntry const *pEntry )
{
    if ( !IsPackageMember() )
    {
        if ( m_xStream.is() )
        {
            m_xStream->closeInput();
            m_xStream.clear();
            m_bHasSeekable = false;
        }
        SetPackageMember ( true );
    }

    if ( m_bRawStream )
    {
        // the raw stream was integrated and now behaves
        // as usual encrypted stream
        SetToBeEncrypted( true );
    }

    // Then copy it back afterwards...
    aEntry = *pEntry;

    // TODO/LATER: get rid of this hack ( the encrypted stream size property is changed during saving )
    if ( m_bIsEncrypted )
        setSize( m_nOwnStreamOrigSize );

    aEntry.nOffset *= -1;
}

void ZipPackageStream::SetPackageMember( bool bNewValue )
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
void SAL_CALL ZipPackageStream::setInputStream( const uno::Reference< io::XInputStream >& aStream )
{
    // if seekable access is required the wrapping will be done on demand
    m_xStream = aStream;
    m_nImportedEncryptionAlgorithm = 0;
    m_bHasSeekable = false;
    SetPackageMember ( false );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_DETECT;
}

uno::Reference< io::XInputStream > ZipPackageStream::getRawData()
{
    try
    {
        if ( IsPackageMember() )
        {
            return m_rZipPackage.getZipFile().getRawData( aEntry, GetEncryptionData(), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef(), false/*bUseBufferedStream*/ );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), m_rZipPackage.GetSharedMutexRef() );
        }
        else
            return uno::Reference < io::XInputStream > ();
    }
    catch ( ZipException & )//rException )
    {
        TOOLS_WARN_EXCEPTION( "package", "" );
        return uno::Reference < io::XInputStream > ();
    }
    catch ( Exception & )
    {
        TOOLS_WARN_EXCEPTION( "package", "Exception is thrown during stream wrapping!" );
        return uno::Reference < io::XInputStream > ();
    }
}

uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream()
{
    try
    {
        if ( IsPackageMember() )
        {
            return m_rZipPackage.getZipFile().getInputStream( aEntry, GetEncryptionData(), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
        }
        else if ( GetOwnSeekStream().is() )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), m_rZipPackage.GetSharedMutexRef() );
        }
        else
            return uno::Reference < io::XInputStream > ();
    }
    catch ( ZipException & )//rException )
    {
        TOOLS_WARN_EXCEPTION( "package", "" );
        return uno::Reference < io::XInputStream > ();
    }
    catch ( const Exception & )
    {
        TOOLS_WARN_EXCEPTION( "package", "Exception is thrown during stream wrapping!");
        return uno::Reference < io::XInputStream > ();
    }
}

// XDataSinkEncrSupport
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getDataStream()
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(THROW_WHERE );

    if ( IsPackageMember() )
    {
        uno::Reference< io::XInputStream > xResult;
        try
        {
            xResult = m_rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData(Bugs::None), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
        }
        catch( const packages::WrongPasswordException& )
        {
            // note: due to SHA1 check this fallback is only done for
            // * ODF 1.2 files written by OOo < 3.4beta / LO < 3.5
            // * ODF 1.1/OOoXML files written by any version
            if ( m_rZipPackage.GetStartKeyGenID() == xml::crypto::DigestID::SHA1 )
            {
                SAL_WARN("package", "ZipPackageStream::getDataStream(): SHA1 mismatch, trying fallbacks...");
                try
                {   // tdf#114939 try with legacy StarOffice SHA1 bug
                    xResult = m_rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData(Bugs::WrongSHA1), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
                    return xResult;
                }
                catch (const packages::WrongPasswordException&)
                {
                    /* ignore and try next... */
                }

                try
                {
                    // rhbz#1013844 / fdo#47482 workaround for the encrypted
                    // OpenOffice.org 1.0 documents generated by Libreoffice <=
                    // 3.6 with the new encryption format and using SHA256, but
                    // missing a specified startkey of SHA256

                    // force SHA256 and see if that works
                    m_nImportedStartKeyAlgorithm = xml::crypto::DigestID::SHA256;
                    xResult = m_rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData(), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
                    return xResult;
                }
                catch (const packages::WrongPasswordException&)
                {
                    // if that didn't work, restore to SHA1 and trundle through the *other* earlier
                    // bug fix
                    m_nImportedStartKeyAlgorithm = xml::crypto::DigestID::SHA1;
                }

                // workaround for the encrypted documents generated with the old OOo1.x bug.
                if ( !m_bUseWinEncoding )
                {
                    xResult = m_rZipPackage.getZipFile().getDataStream( aEntry, GetEncryptionData(Bugs::WinEncodingWrongSHA1), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
                    m_bUseWinEncoding = true;
                }
                else
                    throw;
            }
            else
                throw;
        }
        return xResult;
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        return ZipFile::StaticGetDataFromRawStream( m_rZipPackage.GetSharedMutexRef(), m_xContext, GetOwnSeekStream(), GetEncryptionData() );
    else if ( GetOwnSeekStream().is() )
    {
        return new WrapStreamForShare( GetOwnSeekStream(), m_rZipPackage.GetSharedMutexRef() );
    }
    else
        return uno::Reference< io::XInputStream >();
}

uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawStream()
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(THROW_WHERE );

    if ( IsPackageMember() )
    {
        if ( !m_bIsEncrypted || !GetEncryptionData().is() )
            throw packages::NoEncryptionException(THROW_WHERE );

        return m_rZipPackage.getZipFile().getWrappedRawStream( aEntry, GetEncryptionData(), msMediaType, m_rZipPackage.GetSharedMutexRef() );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            return new WrapStreamForShare( GetOwnSeekStream(), m_rZipPackage.GetSharedMutexRef() );
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA && m_bToBeEncrypted )
            return TryToGetRawFromDataStream( true );
    }

    throw packages::NoEncryptionException(THROW_WHERE );
}

void SAL_CALL ZipPackageStream::setDataStream( const uno::Reference< io::XInputStream >& aStream )
{
    setInputStream( aStream );
    m_nStreamMode = PACKAGE_STREAM_DATA;
}

void SAL_CALL ZipPackageStream::setRawStream( const uno::Reference< io::XInputStream >& aStream )
{
    // wrap the stream in case it is not seekable
    uno::Reference< io::XInputStream > xNewStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( aStream, m_xContext );
    uno::Reference< io::XSeekable > xSeek( xNewStream, UNO_QUERY_THROW );
    xSeek->seek( 0 );
    uno::Reference< io::XInputStream > xOldStream = m_xStream;
    m_xStream = xNewStream;
    if ( !ParsePackageRawStream() )
    {
        m_xStream = xOldStream;
        throw packages::NoRawFormatException(THROW_WHERE );
    }

    // the raw stream MUST have seekable access
    m_bHasSeekable = true;

    SetPackageMember ( false );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_RAW;
}

uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getPlainRawStream()
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return uno::Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(THROW_WHERE );

    if ( IsPackageMember() )
    {
        return m_rZipPackage.getZipFile().getRawData( aEntry, GetEncryptionData(), m_bIsEncrypted, m_rZipPackage.GetSharedMutexRef() );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            // the header should not be returned here
            return GetRawEncrStreamNoHeaderCopy();
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA )
            return TryToGetRawFromDataStream( false );
    }

    return uno::Reference< io::XInputStream >();
}

// XPropertySet
void SAL_CALL ZipPackageStream::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    if ( aPropertyName == "MediaType" )
    {
        if ( m_rZipPackage.getFormat() != embed::StorageFormats::PACKAGE && m_rZipPackage.getFormat() != embed::StorageFormats::OFOPXML )
            throw beans::PropertyVetoException(THROW_WHERE );

        if ( !(aValue >>= msMediaType) )
            throw IllegalArgumentException(THROW_WHERE "MediaType must be a string!",
                                            uno::Reference< XInterface >(),
                                            2 );

        if ( !msMediaType.isEmpty() )
        {
            if ( msMediaType.indexOf ( "text" ) != -1
             || msMediaType == "application/vnd.sun.star.oleobject" )
                m_bToBeCompressed = true;
            else if ( !m_bCompressedIsSetFromOutside )
                m_bToBeCompressed = false;
        }
    }
    else if ( aPropertyName == "Size" )
    {
        if ( !( aValue >>= aEntry.nSize ) )
            throw IllegalArgumentException(THROW_WHERE "Wrong type for Size property!",
                                            uno::Reference< XInterface >(),
                                            2 );
    }
    else if ( aPropertyName == "Encrypted" )
    {
        if ( m_rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException(THROW_WHERE );

        bool bEnc = false;
        if ( !(aValue >>= bEnc) )
            throw IllegalArgumentException(THROW_WHERE "Wrong type for Encrypted property!",
                                            uno::Reference< XInterface >(),
                                            2 );

        // In case of new raw stream, the stream must not be encrypted on storing
        if ( bEnc && m_nStreamMode == PACKAGE_STREAM_RAW )
            throw IllegalArgumentException(THROW_WHERE "Raw stream can not be encrypted on storing",
                                            uno::Reference< XInterface >(),
                                            2 );

        m_bToBeEncrypted = bEnc;
        if ( m_bToBeEncrypted && !m_xBaseEncryptionData.is() )
            m_xBaseEncryptionData = new BaseEncryptionData;

    }
    else if ( aPropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        if ( m_rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException(THROW_WHERE );

        uno::Sequence< sal_Int8 > aNewKey;

        if ( !( aValue >>= aNewKey ) )
        {
            OUString sTempString;
            if ( !(aValue >>= sTempString) )
                throw IllegalArgumentException(THROW_WHERE "Wrong type for EncryptionKey property!",
                                                uno::Reference< XInterface >(),
                                                2 );

            sal_Int32 nPathLength = sTempString.getLength();
            Sequence < sal_Int8 > aSequence ( nPathLength );
            sal_Int8 *pArray = aSequence.getArray();
            const sal_Unicode *pChar = sTempString.getStr();
            for ( sal_Int32 i = 0; i < nPathLength; i++ )
                pArray[i] = static_cast < sal_Int8 > ( pChar[i] );
            aNewKey = aSequence;
        }

        if ( aNewKey.hasElements() )
        {
            if ( !m_xBaseEncryptionData.is() )
                m_xBaseEncryptionData = new BaseEncryptionData;

            m_aEncryptionKey = aNewKey;
            // In case of new raw stream, the stream must not be encrypted on storing
            m_bHaveOwnKey = true;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                m_bToBeEncrypted = true;
        }
        else
        {
            m_bHaveOwnKey = false;
            m_aEncryptionKey.realloc( 0 );
        }

        m_aStorageEncryptionKeys.realloc( 0 );
    }
    else if ( aPropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        if ( m_rZipPackage.getFormat() != embed::StorageFormats::PACKAGE )
            throw beans::PropertyVetoException(THROW_WHERE );

        uno::Sequence< beans::NamedValue > aKeys;
        if ( !( aValue >>= aKeys ) )
        {
                throw IllegalArgumentException(THROW_WHERE "Wrong type for StorageEncryptionKeys property!",
                                                uno::Reference< XInterface >(),
                                                2 );
        }

        if ( aKeys.hasElements() )
        {
            if ( !m_xBaseEncryptionData.is() )
                m_xBaseEncryptionData = new BaseEncryptionData;

            m_aStorageEncryptionKeys = aKeys;

            // In case of new raw stream, the stream must not be encrypted on storing
            m_bHaveOwnKey = true;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                m_bToBeEncrypted = true;
        }
        else
        {
            m_bHaveOwnKey = false;
            m_aStorageEncryptionKeys.realloc( 0 );
        }

        m_aEncryptionKey.realloc( 0 );
    }
    else if ( aPropertyName == "Compressed" )
    {
        bool bCompr = false;

        if ( !(aValue >>= bCompr) )
            throw IllegalArgumentException(THROW_WHERE "Wrong type for Compressed property!",
                                            uno::Reference< XInterface >(),
                                            2 );

        // In case of new raw stream, the stream must not be encrypted on storing
        if ( bCompr && m_nStreamMode == PACKAGE_STREAM_RAW )
            throw IllegalArgumentException(THROW_WHERE "Raw stream can not be encrypted on storing",
                                            uno::Reference< XInterface >(),
                                            2 );

        m_bToBeCompressed = bCompr;
        m_bCompressedIsSetFromOutside = true;
    }
    else
        throw beans::UnknownPropertyException(aPropertyName);
}

Any SAL_CALL ZipPackageStream::getPropertyValue( const OUString& PropertyName )
{
    if ( PropertyName == "MediaType" )
    {
        return Any(msMediaType);
    }
    else if ( PropertyName == "Size" )
    {
        return Any(aEntry.nSize);
    }
    else if ( PropertyName == "Encrypted" )
    {
        return Any((m_nStreamMode == PACKAGE_STREAM_RAW) || m_bToBeEncrypted);
    }
    else if ( PropertyName == "WasEncrypted" )
    {
        return Any(m_bIsEncrypted);
    }
    else if ( PropertyName == "Compressed" )
    {
        return Any(m_bToBeCompressed);
    }
    else if ( PropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        return Any(m_aEncryptionKey);
    }
    else if ( PropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        return Any(m_aStorageEncryptionKeys);
    }
    else
        throw beans::UnknownPropertyException(PropertyName);
}

void ZipPackageStream::setSize ( const sal_Int64 nNewSize )
{
    if ( aEntry.nCompressedSize != nNewSize )
        aEntry.nMethod = DEFLATED;
    aEntry.nSize = nNewSize;
}
OUString ZipPackageStream::getImplementationName()
{
    return "ZipPackageStream";
}

Sequence< OUString > ZipPackageStream::getSupportedServiceNames()
{
    return { "com.sun.star.packages.PackageStream" };
}

sal_Bool SAL_CALL ZipPackageStream::supportsService( OUString const & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
