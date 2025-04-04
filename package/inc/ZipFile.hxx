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
#ifndef INCLUDED_PACKAGE_INC_ZIPFILE_HXX
#define INCLUDED_PACKAGE_INC_ZIPFILE_HXX

#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <comphelper/refcountedmutex.hxx>
#include <package/Inflater.hxx>
#include <rtl/ref.hxx>
#include "ByteGrabber.hxx"
#include "HashMaps.hxx"
#include "EncryptionData.hxx"

#include <optional>
#include <span>
#include <unordered_set>

class MemoryByteGrabber;
namespace com::sun::star {
    namespace uno { class XComponentContext; }
    namespace ucb  { class XProgressHandler; }
}
namespace rtl
{
    template < class T > class Reference;
}

/*
 * We impose arbitrary but reasonable limit on ZIP files.
 */

#define ZIP_MAXNAMELEN 512
#define ZIP_MAXENTRIES (0x10000 - 2)

class ZipEnumeration;


class ZipFile
{
public:
    enum class Checks { Default, CheckInsensitive, TryCheckInsensitive };

private:
    rtl::Reference<comphelper::RefCountedMutex> m_aMutexHolder;

    std::unordered_set<OUString> m_EntriesInsensitive;
    Checks m_Checks;

    EntryHash       aEntries;
    ByteGrabber     aGrabber;
    ZipUtils::Inflater aInflater;
    css::uno::Reference < css::io::XInputStream > xStream;
    const css::uno::Reference < css::uno::XComponentContext > m_xContext;

    bool bRecoveryMode;

    // aMediaType parameter is used only for raw stream header creation
    css::uno::Reference < css::io::XInputStream >  createStreamForZipEntry(
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
            ZipEntry const & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Int8 nStreamMode,
            ::std::optional<sal_Int64> oDecryptedSize,
            const bool bUseBufferedStream = true,
            const OUString& aMediaType = OUString() );

    css::uno::Reference<css::io::XInputStream> checkValidPassword(
            ZipEntry const& rEntry, rtl::Reference<EncryptionData> const& rData,
            sal_Int64 nDecryptedSize,
            rtl::Reference<comphelper::RefCountedMutex> const& rMutexHolder);

    bool checkSizeAndCRC( const ZipEntry& aEntry );

    sal_Int32 getCRC( sal_Int64 nOffset, sal_Int64 nSize );

    void getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC );

    sal_uInt64 readLOC(ZipEntry &rEntry);
    sal_uInt64 readLOC_Impl(ZipEntry &rEntry, std::vector<sal_Int8>& rNameBuffer, std::vector<sal_Int8>& rExtraBuffer);
    sal_Int32 readCEN();
    std::tuple<sal_Int64, sal_Int64, sal_Int64> findCentralDirectory();
    bool TryDDImpl(sal_Int64 dataOffset, sal_Int32 nCRC32,
        sal_Int64 nCompressedSize, sal_Int64 nSize);
    bool TryDDEndAt(std::span<const sal_Int8> data, sal_Int64 dataOffset);
    bool HandlePK34(std::span<const sal_Int8> data, sal_Int64 dataOffset, sal_Int64 totalSize);
    void HandlePK78(std::span<const sal_Int8> data, sal_Int64 dataOffset);
    void recover();
    static bool readExtraFields(MemoryByteGrabber& aMemGrabber, sal_Int16 nExtraLen,
                                sal_uInt64& nSize, sal_uInt64& nCompressedSize,
                                ::std::optional<sal_uInt64> & roOffset,
                                std::string_view const * pCENFilenameToCheck);

public:

    ZipFile( rtl::Reference<comphelper::RefCountedMutex> aMutexHolder,
             css::uno::Reference < css::io::XInputStream > const &xInput,
             css::uno::Reference < css::uno::XComponentContext > xContext,
             bool bInitialise,
             bool bForceRecover,
             Checks checks);

    ~ZipFile();

    EntryHash& GetEntryHash() { return aEntries; }

    void setInputStream ( const css::uno::Reference < css::io::XInputStream >& xNewStream );
    css::uno::Reference< css::io::XInputStream > getRawData(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            ::std::optional<sal_Int64> oDecryptedSize,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
            const bool bUseBufferedStream = true );

    static css::uno::Reference< css::xml::crypto::XDigestContext > StaticGetDigestContextForChecksum(
            const css::uno::Reference< css::uno::XComponentContext >& xArgContext,
            const ::rtl::Reference< EncryptionData >& xEncryptionData );

    static css::uno::Reference< css::xml::crypto::XCipherContext > StaticGetCipher(
            const css::uno::Reference< css::uno::XComponentContext >& xArgContext,
            const ::rtl::Reference< EncryptionData >& xEncryptionData,
            bool bEncrypt );

    static void StaticFillHeader ( const ::rtl::Reference < EncryptionData > & rData,
                                    sal_Int64 nSize,
                                    const OUString& aMediaType,
                                    sal_Int8 * & pHeader );

    static bool StaticFillData ( ::rtl::Reference < BaseEncryptionData > const & rData,
                                     sal_Int32 &rEncAlgorithm,
                                     sal_Int32 &rChecksumAlgorithm,
                                     sal_Int32 &rDerivedKeySize,
                                     sal_Int32 &rStartKeyGenID,
                                     sal_Int32 &rSize,
                                     OUString& aMediaType,
                                     const css::uno::Reference < css::io::XInputStream >& rStream );

    static css::uno::Reference< css::io::XInputStream > StaticGetDataFromRawStream(
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Reference< css::io::XInputStream >& xStream,
            const ::rtl::Reference < EncryptionData > &rData );

    static bool StaticHasValidPassword (
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Sequence< sal_Int8 > &aReadBuffer,
            const ::rtl::Reference < EncryptionData > &rData );

    css::uno::Reference< css::io::XInputStream > getInputStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            ::std::optional<sal_Int64> oDecryptedSize,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    css::uno::Reference< css::io::XInputStream > getDataStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            ::std::optional<sal_Int64> oEncryptedSize,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    css::uno::Reference< css::io::XInputStream > getWrappedRawStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Int64 nDecryptedSize,
            const OUString& aMediaType,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    ZipEnumeration entries();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
