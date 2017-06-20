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

#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <comphelper/refcountedmutex.hxx>
#include <package/Inflater.hxx>
#include <ByteGrabber.hxx>
#include <HashMaps.hxx>
#include <EncryptionData.hxx>

#include <memory>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
    namespace ucb  { class XProgressHandler; }
} } }
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
    rtl::Reference<comphelper::RefCountedMutex> m_aMutexHolder;

    EntryHash       aEntries;
    ByteGrabber     aGrabber;
    ZipUtils::Inflater aInflater;
    css::uno::Reference < css::io::XInputStream > xStream;
    const css::uno::Reference < css::uno::XComponentContext > m_xContext;

    bool bRecoveryMode;

    // aMediaType parameter is used only for raw stream header creation
    css::uno::Reference < css::io::XInputStream >  createStreamForZipEntry(
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
            ZipEntry & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Int8 nStreamMode,
            bool bDecrypt,
            const OUString& aMediaType = OUString() );

    bool hasValidPassword ( ZipEntry & rEntry, const rtl::Reference < EncryptionData > &rData );

    bool checkSizeAndCRC( const ZipEntry& aEntry );

    sal_Int32 getCRC( sal_Int64 nOffset, sal_Int64 nSize );

    void getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC );

    bool readLOC( ZipEntry &rEntry );
    sal_Int32 readCEN();
    sal_Int32 findEND();
    void recover();

public:

    ZipFile( const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
             css::uno::Reference < css::io::XInputStream > &xInput,
             const css::uno::Reference < css::uno::XComponentContext > &rxContext,
             bool bInitialise );

    ZipFile( const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
             css::uno::Reference < css::io::XInputStream > &xInput,
             const css::uno::Reference < css::uno::XComponentContext > &rxContext,
             bool bInitialise,
             bool bForceRecover );

    ~ZipFile();

    EntryHash& GetEntryHash() { return aEntries; }

    void setInputStream ( const css::uno::Reference < css::io::XInputStream >& xNewStream );
    css::uno::Reference< css::io::XInputStream > getRawData(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            bool bDecrypt,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

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

    static bool StaticFillData ( ::rtl::Reference < BaseEncryptionData > & rData,
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
            bool bDecrypt,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    css::uno::Reference< css::io::XInputStream > getDataStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            bool bDecrypt,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    css::uno::Reference< css::io::XInputStream > getWrappedRawStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            const OUString& aMediaType,
            const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder );

    std::unique_ptr<ZipEnumeration> entries();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
