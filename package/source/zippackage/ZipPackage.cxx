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

#include <ZipPackage.hxx>
#include "ZipPackageSink.hxx"
#include <ZipEnumeration.hxx>
#include <ZipPackageStream.hxx>
#include <ZipPackageFolder.hxx>
#include <ZipOutputEntry.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageBuffer.hxx>
#include <ZipFile.hxx>
#include <PackageConstants.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/packages/manifest/ManifestWriter.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <officecfg/Office/Common.hxx>
#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/KDFID.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/uri.hxx>
#include <rtl/random.h>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>

#include <string_view>

#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <utility>

using namespace osl;
using namespace cppu;
using namespace ucbhelper;
using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace com::sun::star::packages;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::manifest;
using namespace com::sun::star::packages::zip::ZipConstants;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

namespace {

class ActiveDataStreamer : public ::cppu::WeakImplHelper< XActiveDataStreamer >
{
    uno::Reference< XStream > mStream;
public:

    virtual uno::Reference< XStream > SAL_CALL getStream() override
            { return mStream; }

    virtual void SAL_CALL setStream( const uno::Reference< XStream >& stream ) override
            { mStream = stream; }
};

class DummyInputStream : public ::cppu::WeakImplHelper< XInputStream >
{
    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence< sal_Int8 >&, sal_Int32 ) override
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< sal_Int8 >&, sal_Int32 ) override
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 ) override
        {}

    virtual sal_Int32 SAL_CALL available() override
        { return 0; }

    virtual void SAL_CALL closeInput() override
        {}
};

}

ZipPackage::ZipPackage ( uno::Reference < XComponentContext > xContext )
: m_aMutexHolder( new comphelper::RefCountedMutex )
, m_nStartKeyGenerationID( xml::crypto::DigestID::SHA1 )
, m_oChecksumDigestID( xml::crypto::DigestID::SHA1_1K )
, m_nKeyDerivationFunctionID(xml::crypto::KDFID::PBKDF2)
, m_nCommonEncryptionID( xml::crypto::CipherID::BLOWFISH_CFB_8 )
, m_bHasEncryptedEntries ( false )
, m_bHasNonEncryptedEntries ( false )
, m_bInconsistent ( false )
, m_bForceRecovery ( false )
, m_bMediaTypeFallbackUsed ( false )
, m_nFormat( embed::StorageFormats::PACKAGE ) // package is the default format
, m_bAllowRemoveOnInsert( true )
, m_eMode ( e_IMode_None )
, m_xContext(std::move( xContext ))
{
    m_xRootFolder = new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
}

ZipPackage::~ZipPackage()
{
}

bool ZipPackage::isLocalFile() const
{
    return comphelper::isFileUrl(m_aURL);
}

void ZipPackage::parseManifest()
{
    if ( m_nFormat != embed::StorageFormats::PACKAGE )
        return;

    bool bManifestParsed = false;
    ::std::optional<OUString> oFirstVersion;
    static constexpr OUString sMeta (u"META-INF"_ustr);
    if ( m_xRootFolder->hasByName( sMeta ) )
    {
        try {
            static constexpr OUString sManifest (u"manifest.xml"_ustr);
            Any aAny = m_xRootFolder->getByName( sMeta );
            uno::Reference< XNameContainer > xMetaInfFolder;
            aAny >>= xMetaInfFolder;
            if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
            {
                uno::Reference < XActiveDataSink > xSink;
                aAny = xMetaInfFolder->getByName( sManifest );
                aAny >>= xSink;
                if ( xSink.is() )
                {
                    uno::Reference < XManifestReader > xReader = ManifestReader::create( m_xContext );

                    static constexpr OUStringLiteral sPropFullPath (u"FullPath");
                    static constexpr OUStringLiteral sPropVersion (u"Version");
                    static constexpr OUStringLiteral sPropMediaType (u"MediaType");
                    static constexpr OUStringLiteral sPropInitialisationVector (u"InitialisationVector");
                    static constexpr OUStringLiteral sPropSalt (u"Salt");
                    static constexpr OUStringLiteral sPropIterationCount (u"IterationCount");
                    static constexpr OUStringLiteral sPropSize (u"Size");
                    static constexpr OUStringLiteral sPropDigest (u"Digest");
                    static constexpr OUStringLiteral sPropDerivedKeySize (u"DerivedKeySize");
                    static constexpr OUStringLiteral sPropDigestAlgorithm (u"DigestAlgorithm");
                    static constexpr OUStringLiteral sPropEncryptionAlgorithm (u"EncryptionAlgorithm");
                    static constexpr OUStringLiteral sPropStartKeyAlgorithm (u"StartKeyAlgorithm");
                    static constexpr OUStringLiteral sKeyInfo (u"KeyInfo");

                    const uno::Sequence < uno::Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                    const Any *pKeyInfo = nullptr;

                    for ( const uno::Sequence<PropertyValue>& rSequence : aManifestSequence )
                    {
                        OUString sPath, sMediaType, sVersion;
                        const Any *pSalt = nullptr, *pVector = nullptr, *pCount = nullptr, *pSize = nullptr, *pDigest = nullptr, *pDigestAlg = nullptr, *pEncryptionAlg = nullptr, *pStartKeyAlg = nullptr, *pDerivedKeySize = nullptr;
                        uno::Any const* pKDF = nullptr;
                        uno::Any const* pArgon2Args = nullptr;
                        for ( const PropertyValue& rValue : rSequence )
                        {
                            if ( rValue.Name == sPropFullPath )
                                rValue.Value >>= sPath;
                            else if ( rValue.Name == sPropVersion )
                            {
                                rValue.Value >>= sVersion;
                                if (!oFirstVersion)
                                {
                                    oFirstVersion.emplace(sVersion);
                                }
                            }
                            else if ( rValue.Name == sPropMediaType )
                                rValue.Value >>= sMediaType;
                            else if ( rValue.Name == sPropSalt )
                                pSalt = &( rValue.Value );
                            else if ( rValue.Name == sPropInitialisationVector )
                                pVector = &( rValue.Value );
                            else if ( rValue.Name == sPropIterationCount )
                                pCount = &( rValue.Value );
                            else if ( rValue.Name == sPropSize )
                                pSize = &( rValue.Value );
                            else if ( rValue.Name == sPropDigest )
                                pDigest = &( rValue.Value );
                            else if ( rValue.Name == sPropDigestAlgorithm )
                                pDigestAlg = &( rValue.Value );
                            else if ( rValue.Name == sPropEncryptionAlgorithm )
                                pEncryptionAlg = &( rValue.Value );
                            else if ( rValue.Name == sPropStartKeyAlgorithm )
                                pStartKeyAlg = &( rValue.Value );
                            else if ( rValue.Name == sPropDerivedKeySize )
                                pDerivedKeySize = &( rValue.Value );
                            else if ( rValue.Name == sKeyInfo )
                                pKeyInfo = &( rValue.Value );
                            else if (rValue.Name == "KeyDerivationFunction") {
                                pKDF = &rValue.Value;
                            } else if (rValue.Name == "Argon2Args") {
                                pArgon2Args = &rValue.Value;
                            }
                        }

                        if ( !sPath.isEmpty() && hasByHierarchicalName ( sPath ) )
                        {
                            aAny = getByHierarchicalName( sPath );
                            uno::Reference < XInterface > xTmp;
                            aAny >>= xTmp;
                            if (auto pFolder = dynamic_cast<ZipPackageFolder*>(xTmp.get()))
                            {
                                pFolder->SetMediaType ( sMediaType );
                                pFolder->SetVersion ( sVersion );
                            }
                            else if (auto pStream = dynamic_cast<ZipPackageStream*>(xTmp.get()))
                            {
                                pStream->SetMediaType ( sMediaType );
                                pStream->SetFromManifest( true );

                                if (pKeyInfo
                                    && pVector && pSize && pEncryptionAlg
                                    && pKDF && pKDF->has<sal_Int32>() && pKDF->get<sal_Int32>() == xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P
                                    && ((pEncryptionAlg->has<sal_Int32>()
                                            && pEncryptionAlg->get<sal_Int32>() == xml::crypto::CipherID::AES_GCM_W3C)
                                        || (pDigest && pDigestAlg)))
                                {
                                    uno::Sequence < sal_Int8 > aSequence;
                                    sal_Int64 nSize = 0;
                                    ::std::optional<sal_Int32> oDigestAlg;
                                    sal_Int32 nEncryptionAlg = 0;

                                    pStream->SetToBeEncrypted ( true );

                                    *pVector >>= aSequence;
                                    pStream->setInitialisationVector ( aSequence );

                                    *pSize >>= nSize;
                                    pStream->setSize ( nSize );

                                    if (pDigest && pDigestAlg)
                                    {
                                        *pDigest >>= aSequence;
                                        pStream->setDigest(aSequence);

                                        assert(pDigestAlg->has<sal_Int32>());
                                        oDigestAlg.emplace(pDigestAlg->get<sal_Int32>());
                                        pStream->SetImportedChecksumAlgorithm(oDigestAlg);
                                    }

                                    *pEncryptionAlg >>= nEncryptionAlg;
                                    pStream->SetImportedEncryptionAlgorithm( nEncryptionAlg );

                                    *pKeyInfo >>= m_aGpgProps;

                                    pStream->SetToBeCompressed ( true );
                                    pStream->SetToBeEncrypted ( true );
                                    pStream->SetIsEncrypted ( true );
                                    pStream->setIterationCount(::std::optional<sal_Int32>());
                                    pStream->setArgon2Args(::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>>());

                                    // clamp to default SHA256 start key magic value,
                                    // c.f. ZipPackageStream::GetEncryptionKey()
                                    // trying to get key value from properties
                                    const sal_Int32 nStartKeyAlg = xml::crypto::DigestID::SHA256;
                                    pStream->SetImportedStartKeyAlgorithm( nStartKeyAlg );

                                    if (!m_bHasEncryptedEntries
                                        && (pStream->getName() == "content.xml"
                                            || pStream->getName() == "encrypted-package"))
                                    {
                                        m_bHasEncryptedEntries = true;
                                        m_oChecksumDigestID = oDigestAlg;
                                        m_nKeyDerivationFunctionID = xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P;
                                        m_nCommonEncryptionID = nEncryptionAlg;
                                        m_nStartKeyGenerationID = nStartKeyAlg;
                                    }
                                }
                                else if (pSalt
                                    && pVector && pSize && pEncryptionAlg
                                    && pKDF && pKDF->has<sal_Int32>()
                                    && ((pKDF->get<sal_Int32>() == xml::crypto::KDFID::PBKDF2 && pCount)
                                        || (pKDF->get<sal_Int32>() == xml::crypto::KDFID::Argon2id && pArgon2Args))
                                    && ((pEncryptionAlg->has<sal_Int32>()
                                            && pEncryptionAlg->get<sal_Int32>() == xml::crypto::CipherID::AES_GCM_W3C)
                                        || (pDigest && pDigestAlg)))

                                {
                                    uno::Sequence < sal_Int8 > aSequence;
                                    sal_Int64 nSize = 0;
                                    ::std::optional<sal_Int32> oDigestAlg;
                                    sal_Int32 nKDF = 0;
                                    sal_Int32 nEncryptionAlg = 0;
                                    sal_Int32 nCount = 0;
                                    sal_Int32 nDerivedKeySize = 16, nStartKeyAlg = xml::crypto::DigestID::SHA1;

                                    pStream->SetToBeEncrypted ( true );

                                    *pSalt >>= aSequence;
                                    pStream->setSalt ( aSequence );

                                    *pVector >>= aSequence;
                                    pStream->setInitialisationVector ( aSequence );

                                    *pKDF >>= nKDF;

                                    if (pCount)
                                    {
                                        *pCount >>= nCount;
                                        pStream->setIterationCount(::std::optional<sal_Int32>(nCount));
                                    }

                                    if (pArgon2Args)
                                    {
                                        uno::Sequence<sal_Int32> args;
                                        *pArgon2Args >>= args;
                                        assert(args.getLength() == 3);
                                        ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgs;
                                        oArgs.emplace(args[0], args[1], args[2]);
                                        pStream->setArgon2Args(oArgs);
                                    }

                                    *pSize >>= nSize;
                                    pStream->setSize ( nSize );

                                    if (pDigest && pDigestAlg)
                                    {
                                        *pDigest >>= aSequence;
                                        pStream->setDigest(aSequence);

                                        assert(pDigestAlg->has<sal_Int32>());
                                        oDigestAlg.emplace(pDigestAlg->get<sal_Int32>());
                                        pStream->SetImportedChecksumAlgorithm(oDigestAlg);
                                    }

                                    *pEncryptionAlg >>= nEncryptionAlg;
                                    pStream->SetImportedEncryptionAlgorithm( nEncryptionAlg );

                                    if ( pDerivedKeySize )
                                        *pDerivedKeySize >>= nDerivedKeySize;
                                    pStream->SetImportedDerivedKeySize( nDerivedKeySize );

                                    if ( pStartKeyAlg )
                                        *pStartKeyAlg >>= nStartKeyAlg;
                                    pStream->SetImportedStartKeyAlgorithm( nStartKeyAlg );

                                    pStream->SetToBeCompressed ( true );
                                    pStream->SetToBeEncrypted ( true );
                                    pStream->SetIsEncrypted ( true );
                                    if (!m_bHasEncryptedEntries
                                        && (pStream->getName() == "content.xml"
                                            || pStream->getName() == "encrypted-package"))
                                    {
                                        m_bHasEncryptedEntries = true;
                                        m_nStartKeyGenerationID = nStartKeyAlg;
                                        m_nKeyDerivationFunctionID = nKDF;
                                        m_oChecksumDigestID = oDigestAlg;
                                        m_nCommonEncryptionID = nEncryptionAlg;
                                    }
                                }
                                else
                                    m_bHasNonEncryptedEntries = true;
                            }
                            else
                                throw ZipIOException(THROW_WHERE "Wrong content");
                        }
                    }

                    bManifestParsed = true;
                }

                // now hide the manifest.xml file from user
                xMetaInfFolder->removeByName( sManifest );
            }
        }
        catch( Exception& )
        {
            if ( !m_bForceRecovery )
                throw;
        }
    }

    if ( !bManifestParsed && !m_bForceRecovery )
        throw ZipIOException(
            THROW_WHERE "Could not parse manifest.xml" );

    static constexpr OUString sMimetype (u"mimetype"_ustr);
    if ( m_xRootFolder->hasByName( sMimetype ) )
    {
        // get mediatype from the "mimetype" stream
        OUString aPackageMediatype;
        uno::Reference < io::XActiveDataSink > xMimeSink;
        m_xRootFolder->getByName( sMimetype ) >>= xMimeSink;
        if ( xMimeSink.is() )
        {
            uno::Reference< io::XInputStream > xMimeInStream = xMimeSink->getInputStream();
            if ( xMimeInStream.is() )
            {
                // Mediatypes longer than 1024 symbols should not appear here
                uno::Sequence< sal_Int8 > aData( 1024 );
                sal_Int32 nRead = xMimeInStream->readBytes( aData, 1024 );
                if ( nRead > aData.getLength() )
                    nRead = aData.getLength();

                if ( nRead )
                    aPackageMediatype = OUString( reinterpret_cast<char const *>(aData.getConstArray()), nRead, RTL_TEXTENCODING_ASCII_US );
            }
        }

        if (!bManifestParsed || m_xRootFolder->GetMediaType().isEmpty())
        {
            // the manifest.xml could not be successfully parsed, this is an inconsistent package
            if ( aPackageMediatype.startsWith("application/vnd.") )
            {
                // accept only types that look similar to own mediatypes
                m_xRootFolder->SetMediaType( aPackageMediatype );
                // also set version explicitly
                if (oFirstVersion && m_xRootFolder->GetVersion().isEmpty())
                {
                    m_xRootFolder->SetVersion(*oFirstVersion);
                }
                // if there is an encrypted inner package, there is no root
                // document, because instead there is a package, and it is not
                // an error
                if (!m_xRootFolder->hasByName("encrypted-package"))
                {
                    m_bMediaTypeFallbackUsed = true;
                }
            }
        }
        else if ( !m_bForceRecovery )
        {
            // the mimetype stream should contain the same information as manifest.xml
            OUString const mediaTypeXML(m_xRootFolder->hasByName("encrypted-package")
                ? m_xRootFolder->doGetByName("encrypted-package").xPackageEntry->GetMediaType()
                : m_xRootFolder->GetMediaType());
            if (mediaTypeXML != aPackageMediatype)
            {
                throw ZipIOException(
                    THROW_WHERE
                    "mimetype conflicts with manifest.xml, \""
                    + mediaTypeXML + "\" vs. \""
                    + aPackageMediatype + "\"" );
            }
        }

        m_xRootFolder->removeByName( sMimetype );
    }

    m_bInconsistent = m_xRootFolder->LookForUnexpectedODF12Streams(
        std::u16string_view(), m_xRootFolder->hasByName("encrypted-package"));

    bool bODF12AndNewer = ( m_xRootFolder->GetVersion().compareTo( ODFVER_012_TEXT ) >= 0 );
    if ( !m_bForceRecovery && bODF12AndNewer )
    {
        if ( m_bInconsistent )
        {
            // this is an ODF1.2 document that contains streams not referred in the manifest.xml;
            // in case of ODF1.2 documents without version in manifest.xml the property IsInconsistent
            // should be checked later
            throw ZipIOException(
                THROW_WHERE "there are streams not referred in manifest.xml" );
        }
        // all the streams should be encrypted with the same StartKey in ODF1.2
        // TODO/LATER: in future the exception should be thrown
        // throw ZipIOException( THROW_WHERE "More than one Start Key Generation algorithm is specified!" );
    }

    // in case it is a correct ODF1.2 document, the version must be set
    // and the META-INF folder is reserved for package format
    if ( bODF12AndNewer )
        m_xRootFolder->removeByName( sMeta );
}

void ZipPackage::parseContentType()
{
    if ( m_nFormat != embed::StorageFormats::OFOPXML )
        return;

    try {
        static constexpr OUString aContentTypes(u"[Content_Types].xml"_ustr);
        // the content type must exist in OFOPXML format!
        if ( !m_xRootFolder->hasByName( aContentTypes ) )
            throw io::IOException(THROW_WHERE "Wrong format!" );

        uno::Reference < io::XActiveDataSink > xSink;
        uno::Any aAny = m_xRootFolder->getByName( aContentTypes );
        aAny >>= xSink;
        if ( xSink.is() )
        {
            uno::Reference< io::XInputStream > xInStream = xSink->getInputStream();
            if ( xInStream.is() )
            {
                // here aContentTypeInfo[0] - Defaults, and aContentTypeInfo[1] - Overrides
                const uno::Sequence< uno::Sequence< beans::StringPair > > aContentTypeInfo =
                    ::comphelper::OFOPXMLHelper::ReadContentTypeSequence( xInStream, m_xContext );

                if ( aContentTypeInfo.getLength() != 2 )
                    throw io::IOException(THROW_WHERE );

                // set the implicit types first
                for ( const auto& rPair : aContentTypeInfo[0] )
                    m_xRootFolder->setChildStreamsTypeByExtension( rPair );

                // now set the explicit types
                for ( const auto& rPair : aContentTypeInfo[1] )
                {
                    OUString aPath;
                    if ( rPair.First.toChar() == '/' )
                        aPath = rPair.First.copy( 1 );
                    else
                        aPath = rPair.First;

                    if ( !aPath.isEmpty() && hasByHierarchicalName( aPath ) )
                    {
                        uno::Any aIterAny = getByHierarchicalName( aPath );
                        uno::Reference < XInterface > xIterTmp;
                        aIterAny >>= xIterTmp;
                        if (auto pStream = dynamic_cast<ZipPackageStream*>(xIterTmp.get()))
                        {
                            // this is a package stream, in OFOPXML format only streams can have mediatype
                            pStream->SetMediaType( rPair.Second );
                        }
                    }
                }
            }
        }

        m_xRootFolder->removeByName( aContentTypes );
    }
    catch( uno::Exception& )
    {
        if ( !m_bForceRecovery )
            throw;
    }
}

void ZipPackage::getZipFileContents()
{
    ZipEnumeration aEnum = m_pZipFile->entries();
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    while (aEnum.hasMoreElements())
    {
        nOldIndex = 0;
        ZipPackageFolder* pCurrent = m_xRootFolder.get();
        const ZipEntry & rEntry = *aEnum.nextElement();
        OUString rName = rEntry.sPath;

        if ( m_bForceRecovery )
        {
            // the PKZIP Application note version 6.2 does not allows to use '\' as separator
            // unfortunately it is used by some implementations, so we have to support it in recovery mode
            rName = rName.replace( '\\', '/' );
        }

        nStreamIndex = rName.lastIndexOf ( '/' );
        if ( nStreamIndex != -1 )
        {
            sDirName = rName.copy ( 0, nStreamIndex );
            aIter = m_aRecent.find ( sDirName );
            if ( aIter != m_aRecent.end() )
                pCurrent = ( *aIter ).second;
        }

        if ( pCurrent == m_xRootFolder.get() )
        {
            sal_Int32 nIndex;
            while ( ( nIndex = rName.indexOf( '/', nOldIndex ) ) != -1 )
            {
                sTemp = rName.copy ( nOldIndex, nIndex - nOldIndex );
                if ( nIndex == nOldIndex )
                    break;
                if ( !pCurrent->hasByName( sTemp ) )
                {
                    rtl::Reference<ZipPackageFolder> pPkgFolder = new ZipPackageFolder(m_xContext, m_nFormat, m_bAllowRemoveOnInsert);
                    pPkgFolder->setName( sTemp );
                    pPkgFolder->doSetParent( pCurrent );
                    pCurrent = pPkgFolder.get();
                }
                else
                {
                    ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
                    if (!rInfo.bFolder)
                        throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
                    pCurrent = rInfo.pFolder;
                }
                nOldIndex = nIndex+1;
            }
            if ( nStreamIndex != -1 && !sDirName.isEmpty() )
                m_aRecent [ sDirName ] = pCurrent;
        }
        if ( rName.getLength() -1 != nStreamIndex )
        {
            nStreamIndex++;
            sTemp = rName.copy( nStreamIndex );

            if (!pCurrent->hasByName(sTemp))
            {
                rtl::Reference<ZipPackageStream> pPkgStream = new ZipPackageStream(*this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert);
                pPkgStream->SetPackageMember(true);
                pPkgStream->setZipEntryOnLoading(rEntry);
                pPkgStream->setName(sTemp);
                pPkgStream->doSetParent(pCurrent);
            }
        }
    }

    if ( m_nFormat == embed::StorageFormats::PACKAGE )
        parseManifest();
    else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        parseContentType();
}

void SAL_CALL ZipPackage::initialize( const uno::Sequence< Any >& aArguments )
{
    beans::NamedValue aNamedValue;

    if ( !aArguments.hasElements() )
        return;

    bool bHaveZipFile = true;

    for( const auto& rArgument : aArguments )
    {
        OUString aParamUrl;
        if ( rArgument >>= aParamUrl )
        {
            m_eMode = e_IMode_URL;
            try
            {
                sal_Int32 nParam = aParamUrl.indexOf( '?' );
                if ( nParam >= 0 )
                {
                    m_aURL = aParamUrl.copy( 0, nParam );
                    std::u16string_view aParam = aParamUrl.subView( nParam + 1 );

                    sal_Int32 nIndex = 0;
                    do
                    {
                        std::u16string_view aCommand = o3tl::getToken(aParam, 0, '&', nIndex );
                        if ( aCommand == u"repairpackage" )
                        {
                            m_bForceRecovery = true;
                            break;
                        }
                        else if ( aCommand == u"purezip" )
                        {
                            m_nFormat = embed::StorageFormats::ZIP;
                            m_xRootFolder->setPackageFormat_Impl( m_nFormat );
                            break;
                        }
                        else if ( aCommand == u"ofopxml" )
                        {
                            m_nFormat = embed::StorageFormats::OFOPXML;
                            m_xRootFolder->setPackageFormat_Impl( m_nFormat );
                            break;
                        }
                    }
                    while ( nIndex >= 0 );
                }
                else
                    m_aURL = aParamUrl;

                Content aContent(
                    m_aURL, uno::Reference< XCommandEnvironment >(),
                    m_xContext );
                Any aAny = aContent.getPropertyValue("Size");
                sal_uInt64 aSize = 0;
                // kind of optimization: treat empty files as nonexistent files
                // and write to such files directly. Note that "Size" property is optional.
                bool bHasSizeProperty = aAny >>= aSize;
                if( !bHasSizeProperty || aSize )
                {
                    uno::Reference < XActiveDataSink > xSink = new ZipPackageSink;
                    if ( aContent.openStream ( xSink ) )
                        m_xContentStream = xSink->getInputStream();
                }
                else
                    bHaveZipFile = false;
            }
            catch ( css::uno::Exception& )
            {
                // Exception derived from uno::Exception thrown. This probably
                // means the file doesn't exist...we'll create it at
                // commitChanges time
                bHaveZipFile = false;
            }
        }
        else if ( rArgument >>= m_xStream )
        {
            // a writable stream can implement both XStream & XInputStream
            m_eMode = e_IMode_XStream;
            m_xContentStream = m_xStream->getInputStream();
        }
        else if ( rArgument >>= m_xContentStream )
        {
            m_eMode = e_IMode_XInputStream;
        }
        else if ( rArgument >>= aNamedValue )
        {
            if ( aNamedValue.Name == "RepairPackage" )
                aNamedValue.Value >>= m_bForceRecovery;
            else if ( aNamedValue.Name == "PackageFormat" )
            {
                // setting this argument to true means Package format
                // setting it to false means plain Zip format

                bool bPackFormat = true;
                aNamedValue.Value >>= bPackFormat;
                if ( !bPackFormat )
                    m_nFormat = embed::StorageFormats::ZIP;

                m_xRootFolder->setPackageFormat_Impl( m_nFormat );
            }
            else if ( aNamedValue.Name == "StorageFormat" )
            {
                OUString aFormatName;
                sal_Int32 nFormatID = 0;
                if ( aNamedValue.Value >>= aFormatName )
                {
                    if ( aFormatName == PACKAGE_STORAGE_FORMAT_STRING )
                        m_nFormat = embed::StorageFormats::PACKAGE;
                    else if ( aFormatName == ZIP_STORAGE_FORMAT_STRING )
                        m_nFormat = embed::StorageFormats::ZIP;
                    else if ( aFormatName == OFOPXML_STORAGE_FORMAT_STRING )
                        m_nFormat = embed::StorageFormats::OFOPXML;
                    else
                        throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );
                }
                else if ( aNamedValue.Value >>= nFormatID )
                {
                    if (nFormatID != embed::StorageFormats::PACKAGE
                        && nFormatID != embed::StorageFormats::ZIP
                        && nFormatID != embed::StorageFormats::OFOPXML)
                        throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

                    m_nFormat = nFormatID;
                }
                else
                    throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

                m_xRootFolder->setPackageFormat_Impl( m_nFormat );
            }
            else if ( aNamedValue.Name == "AllowRemoveOnInsert" )
            {
                aNamedValue.Value >>= m_bAllowRemoveOnInsert;
                m_xRootFolder->setRemoveOnInsertMode_Impl( m_bAllowRemoveOnInsert );
            }
            else if (aNamedValue.Name == "NoFileSync")
                aNamedValue.Value >>= m_bDisableFileSync;

            // for now the progress handler is not used, probably it will never be
            // if ( aNamedValue.Name == "ProgressHandler" )
        }
        else
        {
            // The URL is not acceptable
            throw css::uno::Exception (THROW_WHERE "Bad arguments.",
                getXWeak() );
        }
    }

    try
    {
        if ( m_xContentStream.is() )
        {
            // the stream must be seekable, if it is not it will be wrapped
            m_xContentStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( m_xContentStream, m_xContext );
            m_xContentSeek.set( m_xContentStream, UNO_QUERY_THROW );
            if ( !m_xContentSeek->getLength() )
                bHaveZipFile = false;
        }
        else
            bHaveZipFile = false;
    }
    catch ( css::uno::Exception& )
    {
        // Exception derived from uno::Exception thrown. This probably
        // means the file doesn't exist...we'll create it at
        // commitChanges time
        bHaveZipFile = false;
    }
    if ( !bHaveZipFile )
        return;

    bool bBadZipFile = false;
    OUString message;
    try
    {
        m_pZipFile.emplace(m_aMutexHolder, m_xContentStream, m_xContext, true, m_bForceRecovery);
        getZipFileContents();
    }
    catch ( IOException & e )
    {
        bBadZipFile = true;
        message = "IOException: " + e.Message;
    }
    catch ( ZipException & e )
    {
        bBadZipFile = true;
        message = "ZipException: " + e.Message;
    }
    catch ( Exception & )
    {
        m_pZipFile.reset();
        throw;
    }

    if ( bBadZipFile )
    {
        // clean up the memory, and tell the UCB about the error
        m_pZipFile.reset();

        throw css::packages::zip::ZipIOException (
            THROW_WHERE "Bad Zip File, " + message,
            getXWeak() );
    }
}

Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    sal_Int32 nIndex = aName.getLength();

    if (aName == "/")
        // root directory.
        return Any ( uno::Reference( cppu::getXWeak(m_xRootFolder.get()) ) );

    nStreamIndex = aName.lastIndexOf ( '/' );
    bool bFolder = nStreamIndex == nIndex-1; // last character is '/'.

    if ( nStreamIndex != -1 )
    {
        // The name contains '/'.
        sDirName = aName.copy ( 0, nStreamIndex );
        aIter = m_aRecent.find ( sDirName );
        if ( aIter != m_aRecent.end() )
        {
            // There is a cached entry for this name.

            ZipPackageFolder* pFolder = aIter->second;

            if ( bFolder )
            {
                // Determine the directory name.
                sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );

                if (pFolder && sTemp == pFolder->getName())
                    return Any(uno::Reference(cppu::getXWeak(pFolder)));
            }
            else
            {
                // Determine the file name.
                sTemp = aName.copy ( nStreamIndex + 1 );

                if (pFolder && pFolder->hasByName(sTemp))
                    return pFolder->getByName(sTemp);
            }

            m_aRecent.erase( aIter );
        }
    }
    else if ( m_xRootFolder->hasByName ( aName ) )
        // top-level element.
        return m_xRootFolder->getByName ( aName );

    // Not in the cache. Search normally.

    nOldIndex = 0;
    ZipPackageFolder * pCurrent = m_xRootFolder.get();
    ZipPackageFolder * pPrevious = nullptr;

    // Find the right directory for the given path.

    while ( ( nIndex = aName.indexOf( '/', nOldIndex )) != -1 )
    {
        sTemp = aName.copy ( nOldIndex, nIndex - nOldIndex );
        if ( nIndex == nOldIndex )
            break;
        if ( !pCurrent->hasByName( sTemp ) )
            throw NoSuchElementException(THROW_WHERE );

        pPrevious = pCurrent;
        ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
        if (!rInfo.bFolder)
            throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
        pCurrent = rInfo.pFolder;
        nOldIndex = nIndex+1;
    }

    if ( bFolder )
    {
        if ( nStreamIndex != -1 )
            m_aRecent[sDirName] = pPrevious; // cache it.
        return Any ( uno::Reference( cppu::getXWeak(pCurrent) ) );
    }

    sTemp = aName.copy( nOldIndex );

    if ( pCurrent->hasByName ( sTemp ) )
    {
        if ( nStreamIndex != -1 )
            m_aRecent[sDirName] = pCurrent; // cache it.
        return pCurrent->getByName( sTemp );
    }

    throw NoSuchElementException(THROW_WHERE);
}

sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
{
    OUString sTemp;
    sal_Int32 nOldIndex;
    FolderHash::iterator aIter;

    sal_Int32 nIndex = aName.getLength();

    if (aName == "/")
        // root directory
        return true;

    try
    {
        OUString sDirName;
        sal_Int32 nStreamIndex;
        nStreamIndex = aName.lastIndexOf ( '/' );
        bool bFolder = nStreamIndex == nIndex-1;
        if ( nStreamIndex != -1 )
        {
            sDirName = aName.copy ( 0, nStreamIndex );
            aIter = m_aRecent.find ( sDirName );
            if ( aIter != m_aRecent.end() )
            {
                if ( bFolder )
                {
                    sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                    sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );
                    if ( sTemp == ( *aIter ).second->getName() )
                        return true;
                    else
                        m_aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( ( *aIter ).second->hasByName( sTemp ) )
                        return true;
                    else
                        m_aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( m_xRootFolder->hasByName ( aName ) )
                return true;
        }
        ZipPackageFolder * pCurrent = m_xRootFolder.get();
        ZipPackageFolder * pPrevious = nullptr;
        nOldIndex = 0;
        while ( ( nIndex = aName.indexOf( '/', nOldIndex )) != -1 )
        {
            sTemp = aName.copy ( nOldIndex, nIndex - nOldIndex );
            if ( nIndex == nOldIndex )
                break;
            if ( pCurrent->hasByName( sTemp ) )
            {
                pPrevious = pCurrent;
                ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
                if (!rInfo.bFolder)
                    throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
                pCurrent = rInfo.pFolder;
            }
            else
                return false;
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            m_aRecent[sDirName] = pPrevious;
            return true;
        }
        else
        {
            sTemp = aName.copy( nOldIndex );

            if ( pCurrent->hasByName( sTemp ) )
            {
                m_aRecent[sDirName] = pCurrent;
                return true;
            }
        }
    }
    catch (const uno::RuntimeException &)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        uno::Any e(::cppu::getCaughtException());
        throw lang::WrappedTargetRuntimeException("ZipPackage::hasByHierarchicalName", nullptr, e);
    }
    return false;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstance()
{
    uno::Reference < XInterface > xRef = *( new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert ) );
    return xRef;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const uno::Sequence< Any >& aArguments )
{
    bool bArg = false;
    uno::Reference < XInterface > xRef;
    if ( aArguments.hasElements() )
        aArguments[0] >>= bArg;
    if ( bArg )
        xRef = *new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
    else
        xRef = *new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert );

    return xRef;
}

void ZipPackage::WriteMimetypeMagicFile( ZipOutputStream& aZipOut )
{
    static constexpr OUString sMime (u"mimetype"_ustr);
    if ( m_xRootFolder->hasByName( sMime ) )
        m_xRootFolder->removeByName( sMime );

    ZipEntry * pEntry = new ZipEntry;
    sal_Int32 nBufferLength = m_xRootFolder->GetMediaType().getLength();
    OString sMediaType = OUStringToOString( m_xRootFolder->GetMediaType(), RTL_TEXTENCODING_ASCII_US );
    const uno::Sequence< sal_Int8 > aType( reinterpret_cast<sal_Int8 const *>(sMediaType.getStr()),
                                     nBufferLength );

    pEntry->sPath = sMime;
    pEntry->nMethod = STORED;
    pEntry->nSize = pEntry->nCompressedSize = nBufferLength;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    CRC32 aCRC32;
    aCRC32.update( aType );
    pEntry->nCrc = aCRC32.getValue();

    try
    {
        ZipOutputStream::setEntry(pEntry);
        aZipOut.writeLOC(pEntry);
        aZipOut.rawWrite(aType);
        aZipOut.rawCloseEntry();
    }
    catch ( const css::io::IOException & )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException(
                THROW_WHERE "Error adding mimetype to the ZipOutputStream!",
                getXWeak(),
                anyEx );
    }
}

void ZipPackage::WriteManifest( ZipOutputStream& aZipOut, const std::vector< uno::Sequence < PropertyValue > >& aManList )
{
    // Write the manifest
    uno::Reference < XManifestWriter > xWriter = ManifestWriter::create( m_xContext );
    ZipEntry * pEntry = new ZipEntry;
    rtl::Reference<ZipPackageBuffer> pBuffer = new ZipPackageBuffer;

    pEntry->sPath = "META-INF/manifest.xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    xWriter->writeManifestSequence ( pBuffer,  comphelper::containerToSequence(aManList) );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // the manifest.xml is never encrypted - so pass an empty reference
    ZipOutputStream::setEntry(pEntry);
    aZipOut.writeLOC(pEntry);
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr, /*bEncrypt*/false);
    aZipEntry.write(pBuffer->getSequence());
    aZipEntry.closeEntry();
    aZipOut.rawCloseEntry();
}

void ZipPackage::WriteContentTypes( ZipOutputStream& aZipOut, const std::vector< uno::Sequence < PropertyValue > >& aManList )
{
    ZipEntry* pEntry = new ZipEntry;
    rtl::Reference<ZipPackageBuffer> pBuffer = new ZipPackageBuffer;

    pEntry->sPath = "[Content_Types].xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    // Add default entries, the count must be updated manually when appending.
    // Add at least the standard default entries.
    uno::Sequence< beans::StringPair > aDefaultsSequence
    {
        { "xml", "application/xml" },
        { "rels", "application/vnd.openxmlformats-package.relationships+xml" },
        { "png", "image/png" },
        { "jpeg", "image/jpeg" }
    };

    uno::Sequence< beans::StringPair > aOverridesSequence(aManList.size());
    auto aOverridesSequenceRange = asNonConstRange(aOverridesSequence);
    sal_Int32 nOverSeqLength = 0;
    for (const auto& rMan : aManList)
    {
        OUString aType;
        OSL_ENSURE( rMan[PKG_MNFST_MEDIATYPE].Name == "MediaType" && rMan[PKG_MNFST_FULLPATH].Name == "FullPath",
                    "The mediatype sequence format is wrong!" );
        rMan[PKG_MNFST_MEDIATYPE].Value >>= aType;
        if ( !aType.isEmpty() )
        {
            OUString aPath;
            // only nonempty type makes sense here
            rMan[PKG_MNFST_FULLPATH].Value >>= aPath;
            //FIXME: For now we have no way of differentiating defaults from others.
            aOverridesSequenceRange[nOverSeqLength].First = "/" + aPath;
            aOverridesSequenceRange[nOverSeqLength].Second = aType;
            ++nOverSeqLength;
        }
    }
    aOverridesSequence.realloc(nOverSeqLength);

    ::comphelper::OFOPXMLHelper::WriteContentSequence(
            pBuffer, aDefaultsSequence, aOverridesSequence, m_xContext );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // there is no encryption in this format currently
    ZipOutputStream::setEntry(pEntry);
    aZipOut.writeLOC(pEntry);
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr, /*bEncrypt*/false);
    aZipEntry.write(pBuffer->getSequence());
    aZipEntry.closeEntry();
    aZipOut.rawCloseEntry();
}

void ZipPackage::ConnectTo( const uno::Reference< io::XInputStream >& xInStream )
{
    m_xContentSeek.set( xInStream, uno::UNO_QUERY_THROW );
    m_xContentStream = xInStream;

    // seek back to the beginning of the temp file so we can read segments from it
    m_xContentSeek->seek( 0 );
    if ( m_pZipFile )
        m_pZipFile->setInputStream( m_xContentStream );
    else
        m_pZipFile.emplace(m_aMutexHolder, m_xContentStream, m_xContext, false);
}

namespace
{
    class RandomPool
    {
    private:
        rtlRandomPool m_aRandomPool;
    public:
        RandomPool() : m_aRandomPool(rtl_random_createPool ())
        {
        }
        rtlRandomPool get()
        {
            return m_aRandomPool;
        }
        ~RandomPool()
        {
            // Clean up random pool memory
            rtl_random_destroyPool(m_aRandomPool);
        }
    };
}

uno::Reference< io::XInputStream > ZipPackage::writeTempFile()
{
    // In case the target local file does not exist or empty
    // write directly to it otherwise create a temporary file to write to.
    // If a temporary file is created it is returned back by the method.
    // If the data written directly, xComponentStream will be switched here

    bool bUseTemp = true;
    uno::Reference < io::XInputStream > xResult;
    uno::Reference < io::XInputStream > xTempIn;

    uno::Reference < io::XOutputStream > xTempOut;
    uno::Reference< io::XActiveDataStreamer > xSink;

    if ( m_eMode == e_IMode_URL && !m_pZipFile && isLocalFile() )
    {
        xSink = openOriginalForOutput();
        if( xSink.is() )
        {
            uno::Reference< io::XStream > xStr = xSink->getStream();
            if( xStr.is() )
            {
                xTempOut = xStr->getOutputStream();
                if( xTempOut.is() )
                    bUseTemp = false;
            }
        }
    }
    else if ( m_eMode == e_IMode_XStream && !m_pZipFile )
    {
        // write directly to an empty stream
        xTempOut = m_xStream->getOutputStream();
        if( xTempOut.is() )
            bUseTemp = false;
    }

    if( bUseTemp )
    {
        // create temporary file
        rtl::Reference < utl::TempFileFastService > xTempFile( new utl::TempFileFastService );
        xTempOut.set( xTempFile );
        xTempIn.set( xTempFile );
    }

    // Hand it to the ZipOutputStream:
    ZipOutputStream aZipOut( xTempOut );
    try
    {
        if ( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            // Remove the old manifest.xml file as the
            // manifest will be re-generated and the
            // META-INF directory implicitly created if does not exist
            static constexpr OUString sMeta (u"META-INF"_ustr);

            if ( m_xRootFolder->hasByName( sMeta ) )
            {
                static constexpr OUString sManifest (u"manifest.xml"_ustr);

                uno::Reference< XNameContainer > xMetaInfFolder;
                Any aAny = m_xRootFolder->getByName( sMeta );
                aAny >>= xMetaInfFolder;
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                    xMetaInfFolder->removeByName( sManifest );
            }

            // Write a magic file with mimetype
            WriteMimetypeMagicFile( aZipOut );
        }
        else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        {
            // Remove the old [Content_Types].xml file as the
            // file will be re-generated

            static constexpr OUString aContentTypes(u"[Content_Types].xml"_ustr);

            if ( m_xRootFolder->hasByName( aContentTypes ) )
                m_xRootFolder->removeByName( aContentTypes );
        }

        // Create a vector to store data for the manifest.xml file
        std::vector < uno::Sequence < PropertyValue > > aManList;

        static constexpr OUStringLiteral sMediaType(u"MediaType");
        static constexpr OUStringLiteral sVersion(u"Version");
        static constexpr OUStringLiteral sFullPath(u"FullPath");
        const bool bIsGpgEncrypt = m_aGpgProps.hasElements();

        // note: this is always created here (needed for GPG), possibly
        // filtered out later in ManifestExport
        if ( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            uno::Sequence < PropertyValue > aPropSeq(
                bIsGpgEncrypt ? PKG_SIZE_NOENCR_MNFST+1 : PKG_SIZE_NOENCR_MNFST );
            auto pPropSeq = aPropSeq.getArray();
            pPropSeq [PKG_MNFST_MEDIATYPE].Name = sMediaType;
            pPropSeq [PKG_MNFST_MEDIATYPE].Value <<= m_xRootFolder->GetMediaType();
            pPropSeq [PKG_MNFST_VERSION].Name = sVersion;
            pPropSeq [PKG_MNFST_VERSION].Value <<= m_xRootFolder->GetVersion();
            pPropSeq [PKG_MNFST_FULLPATH].Name = sFullPath;
            pPropSeq [PKG_MNFST_FULLPATH].Value <<= OUString("/");

            if( bIsGpgEncrypt )
            {
                pPropSeq[PKG_SIZE_NOENCR_MNFST].Name = "KeyInfo";
                pPropSeq[PKG_SIZE_NOENCR_MNFST].Value <<= m_aGpgProps;
            }
            aManList.push_back( aPropSeq );
        }

        {
            // This will be used to generate random salt and initialisation vectors
            // for encrypted streams
            RandomPool aRandomPool;

            ::std::optional<sal_Int32> oPBKDF2IterationCount;
            ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> oArgon2Args;

            if (!bIsGpgEncrypt)
            {
                if (m_nKeyDerivationFunctionID == xml::crypto::KDFID::PBKDF2)
                {   // if there is only one KDF invocation, increase the safety margin
                    oPBKDF2IterationCount.emplace(officecfg::Office::Common::Misc::ExperimentalMode::get() ? 600000 : 100000);
                }
                else
                {
                    assert(m_nKeyDerivationFunctionID == xml::crypto::KDFID::Argon2id);
                    oArgon2Args.emplace(3, (1<<16), 4);
                }
            }

            // call saveContents - it will recursively save sub-directories
            m_xRootFolder->saveContents("", aManList, aZipOut, GetEncryptionKey(),
                oPBKDF2IterationCount, oArgon2Args, aRandomPool.get());
        }

        if( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            WriteManifest( aZipOut, aManList );
        }
        else if( m_nFormat == embed::StorageFormats::OFOPXML )
        {
            WriteContentTypes( aZipOut, aManList );
        }

        aZipOut.finish();

        if( bUseTemp )
            xResult = xTempIn;

        // Update our References to point to the new temp file
        if( !bUseTemp )
        {
            // the case when the original contents were written directly
            xTempOut->flush();

            // in case the stream is based on a file it will implement the following interface
            // the call should be used to be sure that the contents are written to the file system
            uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( xTempOut, uno::UNO_QUERY );
            if (asyncOutputMonitor.is() && !m_bDisableFileSync)
                asyncOutputMonitor->waitForCompletion();

            // no need to postpone switching to the new stream since the target was written directly
            uno::Reference< io::XInputStream > xNewStream;
            if ( m_eMode == e_IMode_URL )
                xNewStream = xSink->getStream()->getInputStream();
            else if ( m_eMode == e_IMode_XStream && m_xStream.is() )
                xNewStream = m_xStream->getInputStream();

            if ( xNewStream.is() )
                ConnectTo( xNewStream );
        }
    }
    catch ( uno::Exception& )
    {
        if( bUseTemp )
        {
            // no information loss appears, thus no special handling is required
            uno::Any aCaught( ::cppu::getCaughtException() );

            // it is allowed to throw WrappedTargetException
            WrappedTargetException aException;
            if ( aCaught >>= aException )
                throw aException;

            throw WrappedTargetException(
                    THROW_WHERE "Problem writing the original content!",
                    getXWeak(),
                    aCaught );
        }
        else
        {
            // the document is written directly, although it was empty it is important to notify that the writing has failed
            // TODO/LATER: let the package be able to recover in this situation
            OUString aErrTxt(THROW_WHERE "This package is unusable!");
            embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), OUString() );
            throw WrappedTargetException( aErrTxt,
                                            getXWeak(),
                                            Any ( aException ) );
        }
    }

    return xResult;
}

uno::Reference< XActiveDataStreamer > ZipPackage::openOriginalForOutput()
{
    // open and truncate the original file
    Content aOriginalContent(
        m_aURL, uno::Reference< XCommandEnvironment >(),
        m_xContext );
    uno::Reference< XActiveDataStreamer > xSink = new ActiveDataStreamer;

    if ( m_eMode == e_IMode_URL )
    {
        try
        {
            bool bTruncSuccess = false;

            try
            {
                Exception aDetect;
                Any aAny = aOriginalContent.setPropertyValue("Size", Any( sal_Int64(0) ) );
                if( !( aAny >>= aDetect ) )
                    bTruncSuccess = true;
            }
            catch( Exception& )
            {
            }

            if( !bTruncSuccess )
            {
                // the file is not accessible
                // just try to write an empty stream to it

                uno::Reference< XInputStream > xTempIn = new DummyInputStream; //uno::Reference< XInputStream >( xTempOut, UNO_QUERY );
                aOriginalContent.writeStream( xTempIn , true );
            }

            OpenCommandArgument2 aArg;
            aArg.Mode        = OpenMode::DOCUMENT;
            aArg.Priority    = 0; // unused
            aArg.Sink       = xSink;
            aArg.Properties = uno::Sequence< Property >( 0 ); // unused

            aOriginalContent.executeCommand("open", Any( aArg ) );
        }
        catch( Exception& )
        {
            // seems to be nonlocal file
            // temporary file mechanics should be used
        }
    }

    return xSink;
}

void SAL_CALL ZipPackage::commitChanges()
{
    // lock the component for the time of committing
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_eMode == e_IMode_XInputStream )
    {
        IOException aException;
        throw WrappedTargetException(THROW_WHERE "This package is read only!",
                getXWeak(), Any ( aException ) );
    }
    // first the writeTempFile is called, if it returns a stream the stream should be written to the target
    // if no stream was returned, the file was written directly, nothing should be done
    uno::Reference< io::XInputStream > xTempInStream;
    try
    {
        xTempInStream = writeTempFile();
    }
    catch (const ucb::ContentCreationException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException(THROW_WHERE "Temporary file should be creatable!",
                    getXWeak(), anyEx );
    }
    if ( xTempInStream.is() )
    {
        uno::Reference< io::XSeekable > xTempSeek( xTempInStream, uno::UNO_QUERY_THROW );

        try
        {
            xTempSeek->seek( 0 );
        }
        catch( const uno::Exception& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw WrappedTargetException(THROW_WHERE "Temporary file should be seekable!",
                    getXWeak(), anyEx );
        }

        try
        {
            // connect to the temporary stream
            ConnectTo( xTempInStream );
        }
        catch( const io::IOException& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw WrappedTargetException(THROW_WHERE "Temporary file should be connectable!",
                    getXWeak(), anyEx );
        }

        if ( m_eMode == e_IMode_XStream )
        {
            // First truncate our output stream
            uno::Reference < XOutputStream > xOutputStream;

            // preparation for copy step
            try
            {
                xOutputStream = m_xStream->getOutputStream();

                // Make sure we avoid a situation where the current position is
                // not zero, but the underlying file is truncated in the
                // meantime.
                uno::Reference<io::XSeekable> xSeekable(xOutputStream, uno::UNO_QUERY);
                if (xSeekable.is())
                    xSeekable->seek(0);

                uno::Reference < XTruncate > xTruncate ( xOutputStream, UNO_QUERY_THROW );

                // after successful truncation the original file contents are already lost
                xTruncate->truncate();
            }
            catch( const uno::Exception& )
            {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw WrappedTargetException(THROW_WHERE "This package is read only!",
                        getXWeak(), anyEx );
            }

            try
            {
                // then copy the contents of the tempfile to our output stream
                ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, xOutputStream );
                xOutputStream->flush();
                uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor(
                    xOutputStream, uno::UNO_QUERY );
                if ( asyncOutputMonitor.is() ) {
                    asyncOutputMonitor->waitForCompletion();
                }
            }
            catch( uno::Exception& )
            {
                // if anything goes wrong in this block the target file becomes corrupted
                // so an exception should be thrown as a notification about it
                // and the package must disconnect from the stream
                DisconnectFromTargetAndThrowException_Impl( xTempInStream );
            }
        }
        else if ( m_eMode == e_IMode_URL )
        {
            uno::Reference< XOutputStream > aOrigFileStream;
            bool bCanBeCorrupted = false;

            if( isLocalFile() )
            {
                // write directly in case of local file
                uno::Reference< css::ucb::XSimpleFileAccess3 > xSimpleAccess(
                    SimpleFileAccess::create( m_xContext ) );
                OSL_ENSURE( xSimpleAccess.is(), "Can't instantiate SimpleFileAccess service!" );
                uno::Reference< io::XTruncate > xOrigTruncate;
                if ( xSimpleAccess.is() )
                {
                    try
                    {
                        aOrigFileStream = xSimpleAccess->openFileWrite( m_aURL );
                        xOrigTruncate.set( aOrigFileStream, uno::UNO_QUERY_THROW );
                        // after successful truncation the file is already corrupted
                        xOrigTruncate->truncate();
                    }
                    catch( uno::Exception& )
                    {}
                }

                if( xOrigTruncate.is() )
                {
                    try
                    {
                        ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, aOrigFileStream );
                        aOrigFileStream->closeOutput();
                    }
                    catch( uno::Exception& )
                    {
                        try {
                            aOrigFileStream->closeOutput();
                        } catch ( uno::Exception& ) {}

                        aOrigFileStream.clear();
                        // the original file can already be corrupted
                        bCanBeCorrupted = true;
                    }
                }
            }

            if( !aOrigFileStream.is() )
            {
                try
                {
                    uno::Reference < XPropertySet > xPropSet ( xTempInStream, UNO_QUERY_THROW );

                    OUString sTargetFolder = m_aURL.copy ( 0, m_aURL.lastIndexOf ( u'/' ) );
                    Content aContent(
                        sTargetFolder, uno::Reference< XCommandEnvironment >(),
                        m_xContext );

                    OUString sTempURL;
                    Any aAny = xPropSet->getPropertyValue ("Uri");
                    aAny >>= sTempURL;

                    TransferInfo aInfo;
                    aInfo.NameClash = NameClash::OVERWRITE;
                    aInfo.MoveData = false;
                    aInfo.SourceURL = sTempURL;
                    aInfo.NewTitle = rtl::Uri::decode ( m_aURL.copy ( 1 + m_aURL.lastIndexOf ( u'/' ) ),
                                                        rtl_UriDecodeWithCharset,
                                                        RTL_TEXTENCODING_UTF8 );
                    // if the file is still not corrupted, it can become after the next step
                    aContent.executeCommand ("transfer", Any(aInfo) );
                }
                catch ( const css::uno::Exception& )
                {
                    if ( bCanBeCorrupted )
                        DisconnectFromTargetAndThrowException_Impl( xTempInStream );

                    css::uno::Any anyEx = cppu::getCaughtException();
                    throw WrappedTargetException(
                                                THROW_WHERE "This package may be read only!",
                                                getXWeak(),
                                                anyEx );
                }
            }
        }
    }

    // after successful storing it can be set to false
    m_bMediaTypeFallbackUsed = false;
}

void ZipPackage::DisconnectFromTargetAndThrowException_Impl( const uno::Reference< io::XInputStream >& xTempStream )
{
    m_xStream.set( xTempStream, uno::UNO_QUERY );
    if ( m_xStream.is() )
        m_eMode = e_IMode_XStream;
    else
        m_eMode = e_IMode_XInputStream;

    OUString aTempURL;
    try {
        uno::Reference< beans::XPropertySet > xTempFile( xTempStream, uno::UNO_QUERY_THROW );
        uno::Any aUrl = xTempFile->getPropertyValue("Uri");
        aUrl >>= aTempURL;
        xTempFile->setPropertyValue("RemoveFile",
                                     uno::Any( false ) );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "These calls are pretty simple, they should not fail!" );
    }

    OUString aErrTxt(THROW_WHERE "This package is read only!");
    embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), aTempURL );
    throw WrappedTargetException( aErrTxt,
                                    getXWeak(),
                                    Any ( aException ) );
}

uno::Sequence< sal_Int8 > ZipPackage::GetEncryptionKey()
{
    uno::Sequence< sal_Int8 > aResult;

    if ( m_aStorageEncryptionKeys.hasElements() )
    {
        OUString aNameToFind;
        if ( m_nStartKeyGenerationID == xml::crypto::DigestID::SHA256 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
        else if ( m_nStartKeyGenerationID == xml::crypto::DigestID::SHA1 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA1CORRECT;
        else
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );

        for ( const auto& rKey : std::as_const(m_aStorageEncryptionKeys) )
            if ( rKey.Name == aNameToFind )
                rKey.Value >>= aResult;

        if (!aResult.hasElements() && m_aStorageEncryptionKeys.hasElements())
        {   // tdf#159519 sanity check
            throw uno::RuntimeException(THROW_WHERE "Expected key is missing!");
        }
    }
    else
        aResult = m_aEncryptionKey;

    return aResult;
}

sal_Bool SAL_CALL ZipPackage::hasPendingChanges()
{
    return false;
}
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges()
{
    return uno::Sequence < ElementChange > ();
}


OUString ZipPackage::getImplementationName()
{
    return "com.sun.star.packages.comp.ZipPackage";
}

Sequence< OUString > ZipPackage::getSupportedServiceNames()
{
    return { "com.sun.star.packages.Package" };
}

sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo()
{
    return uno::Reference < XPropertySetInfo > ();
}

void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
{
    if ( m_nFormat != embed::StorageFormats::PACKAGE )
        throw UnknownPropertyException(aPropertyName);

    if (aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == IS_INCONSISTENT_PROPERTY
      ||aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY)
        throw PropertyVetoException(THROW_WHERE );
    else if ( aPropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        if ( !( aValue >>= m_aEncryptionKey ) )
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

        m_aStorageEncryptionKeys.realloc( 0 );
    }
    else if ( aPropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        // this property is only necessary to support raw passwords in storage API;
        // because of this support the storage has to operate with more than one key dependent on storage generation algorithm;
        // when this support is removed, the storage will get only one key from outside
        if ( !( aValue >>= m_aStorageEncryptionKeys ) )
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

        m_aEncryptionKey.realloc( 0 );
    }
    else if ( aPropertyName == ENCRYPTION_ALGORITHMS_PROPERTY )
    {
        uno::Sequence< beans::NamedValue > aAlgorithms;
        if ( m_pZipFile || !( aValue >>= aAlgorithms ) || !aAlgorithms.hasElements() )
        {
            // the algorithms can not be changed if the file has a persistence based on the algorithms ( m_pZipFile )
            throw IllegalArgumentException(THROW_WHERE "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
        }

        for ( const auto& rAlgorithm : std::as_const(aAlgorithms) )
        {
            if ( rAlgorithm.Name == "StartKeyGenerationAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( rAlgorithm.Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA256 && nID != xml::crypto::DigestID::SHA1 ) )
                {
                    throw IllegalArgumentException(THROW_WHERE "Unexpected start key generation algorithm is provided!", uno::Reference<uno::XInterface>(), 2);
                }

                m_nStartKeyGenerationID = nID;
            }
            else if (rAlgorithm.Name == "KeyDerivationFunction")
            {
                sal_Int32 nID = 0;
                if (!(rAlgorithm.Value >>= nID)
                  || (nID != xml::crypto::KDFID::PBKDF2
                      && nID != xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P
                      && nID != xml::crypto::KDFID::Argon2id))
                {
                    throw IllegalArgumentException(THROW_WHERE "Unexpected key derivation function provided!", uno::Reference<uno::XInterface>(), 2);
                }
                m_nKeyDerivationFunctionID = nID;
            }
            else if ( rAlgorithm.Name == "EncryptionAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( rAlgorithm.Value >>= nID )
                  || (nID != xml::crypto::CipherID::AES_GCM_W3C
                      && nID != xml::crypto::CipherID::AES_CBC_W3C_PADDING
                      && nID != xml::crypto::CipherID::BLOWFISH_CFB_8))
                {
                    throw IllegalArgumentException(THROW_WHERE "Unexpected encryption algorithm is provided!", uno::Reference<uno::XInterface>(), 2);
                }

                m_nCommonEncryptionID = nID;
            }
            else if ( rAlgorithm.Name == "ChecksumAlgorithm" )
            {
                sal_Int32 nID = 0;
                if (!rAlgorithm.Value.hasValue())
                {
                    m_oChecksumDigestID.reset();
                    continue;
                }
                if ( !( rAlgorithm.Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA1_1K && nID != xml::crypto::DigestID::SHA256_1K ) )
                {
                    throw IllegalArgumentException(THROW_WHERE "Unexpected checksum algorithm is provided!", uno::Reference<uno::XInterface>(), 2);
                }

                m_oChecksumDigestID.emplace(nID);
            }
            else
            {
                OSL_ENSURE( false, "Unexpected encryption algorithm is provided!" );
                throw IllegalArgumentException(THROW_WHERE "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
            }
        }
    }
    else if ( aPropertyName == ENCRYPTION_GPG_PROPERTIES )
    {
        uno::Sequence< uno::Sequence< beans::NamedValue > > aGpgProps;
        if ( !( aValue >>= aGpgProps ) || !aGpgProps.hasElements() )
        {
            throw IllegalArgumentException(THROW_WHERE "unexpected Gpg properties are provided.", uno::Reference< uno::XInterface >(), 2 );
        }

        m_aGpgProps = aGpgProps;

        // override algorithm defaults (which are some legacy ODF
        // defaults) with reasonable values
        // note: these should be overridden by SfxObjectShell::SetupStorage()
        m_nStartKeyGenerationID = 0; // this is unused for PGP
        m_nKeyDerivationFunctionID = xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P;
        m_nCommonEncryptionID = xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        m_oChecksumDigestID.emplace(xml::crypto::DigestID::SHA512_1K);
    }
    else
        throw UnknownPropertyException(aPropertyName);
}

Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
{
    // TODO/LATER: Activate the check when zip-ucp is ready
    // if ( m_nFormat != embed::StorageFormats::PACKAGE )
    //  throw UnknownPropertyException(THROW_WHERE );

    if ( PropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        return Any(m_aEncryptionKey);
    }
    else if ( PropertyName == ENCRYPTION_ALGORITHMS_PROPERTY )
    {
        ::comphelper::SequenceAsHashMap aAlgorithms;
        aAlgorithms["StartKeyGenerationAlgorithm"] <<= m_nStartKeyGenerationID;
        aAlgorithms["KeyDerivationFunction"] <<= m_nKeyDerivationFunctionID;
        aAlgorithms["EncryptionAlgorithm"] <<= m_nCommonEncryptionID;
        if (m_oChecksumDigestID)
        {
            aAlgorithms["ChecksumAlgorithm"] <<= *m_oChecksumDigestID;
        }
        else
        {
            aAlgorithms["ChecksumAlgorithm"];
        }
        return Any(aAlgorithms.getAsConstNamedValueList());
    }
    if ( PropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        return Any(m_aStorageEncryptionKeys);
    }
    else if ( PropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY )
    {
        return Any(m_bHasEncryptedEntries);
    }
    else if ( PropertyName == ENCRYPTION_GPG_PROPERTIES )
    {
        return Any(m_aGpgProps);
    }
    else if ( PropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY )
    {
        return Any(m_bHasNonEncryptedEntries);
    }
    else if ( PropertyName == IS_INCONSISTENT_PROPERTY )
    {
        return Any(m_bInconsistent);
    }
    else if ( PropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY )
    {
        return Any(m_bMediaTypeFallbackUsed);
    }
    else if (PropertyName == "HasElements")
    {
        return Any(m_pZipFile && m_pZipFile->entries().hasMoreElements());
    }
    throw UnknownPropertyException(PropertyName);
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
{
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
package_ZipPackage_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ZipPackage(context));
}

extern "C" bool TestImportZip(SvStream& rStream)
{
    // explicitly tests the "RepairPackage" recovery mode
    rtl::Reference<ZipPackage> xPackage(new ZipPackage(comphelper::getProcessComponentContext()));
    css::uno::Reference<css::io::XInputStream> xStream(new utl::OInputStreamWrapper(rStream));
    css::uno::Sequence<Any> aArgs{ Any(xStream), Any(NamedValue("RepairPackage", Any(true))) };
    xPackage->initialize(aArgs);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
