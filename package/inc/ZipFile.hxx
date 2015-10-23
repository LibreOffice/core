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

#include <package/Inflater.hxx>
#include <ByteGrabber.hxx>
#include <HashMaps.hxx>
#include <EncryptionData.hxx>

#include <mutexholder.hxx>

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
protected:
    ::osl::Mutex    m_aMutex;

    OUString sComment;       /* zip file comment */
    EntryHash       aEntries;
    ByteGrabber     aGrabber;
    ZipUtils::Inflater aInflater;
    css::uno::Reference < css::io::XInputStream > xStream;
    css::uno::Reference < css::io::XSeekable > xSeek;
    const css::uno::Reference < css::uno::XComponentContext > m_xContext;
    css::uno::Reference < css::ucb::XProgressHandler > xProgressHandler;

    bool bRecoveryMode;

    // aMediaType parameter is used only for raw stream header creation
    css::uno::Reference < css::io::XInputStream >  createUnbufferedStream(
            const rtl::Reference<SotMutexHolder>& aMutexHolder,
            ZipEntry & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Int8 nStreamMode,
            bool bDecrypt,
            const OUString& aMediaType = OUString() );

    bool hasValidPassword ( ZipEntry & rEntry, const rtl::Reference < EncryptionData > &rData );

    bool checkSizeAndCRC( const ZipEntry& aEntry );

    sal_Int32 getCRC( sal_Int64 nOffset, sal_Int64 nSize );

    void getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC );

public:

    ZipFile( css::uno::Reference < css::io::XInputStream > &xInput,
             const css::uno::Reference < css::uno::XComponentContext > &rxContext,
             bool bInitialise
             )
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);

    ZipFile( css::uno::Reference < css::io::XInputStream > &xInput,
             const css::uno::Reference < css::uno::XComponentContext > &rxContext,
             bool bInitialise,
             bool bForceRecover,
             css::uno::Reference < css::ucb::XProgressHandler > xProgress
             )
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);

    ~ZipFile();

    EntryHash& GetEntryHash() { return aEntries; }

    void setInputStream ( css::uno::Reference < css::io::XInputStream > xNewStream );
    css::uno::Reference< css::io::XInputStream > SAL_CALL getRawData(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            bool bDecrypt,
            const rtl::Reference<SotMutexHolder>& aMutexHolder )
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);

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
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Reference< css::io::XInputStream >& xStream,
            const ::rtl::Reference < EncryptionData > &rData )
        throw ( css::packages::WrongPasswordException,
                css::packages::zip::ZipIOException,
                css::uno::RuntimeException );

    static bool StaticHasValidPassword (
            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const css::uno::Sequence< sal_Int8 > &aReadBuffer,
            const ::rtl::Reference < EncryptionData > &rData );

    css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            bool bDecrypt,
            const rtl::Reference<SotMutexHolder>& aMutexHolder )
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);

    css::uno::Reference< css::io::XInputStream > SAL_CALL getDataStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            bool bDecrypt,
            const rtl::Reference<SotMutexHolder>& aMutexHolder )
        throw ( css::packages::WrongPasswordException,
                css::io::IOException,
                css::packages::zip::ZipException,
                css::uno::RuntimeException );

    css::uno::Reference< css::io::XInputStream > SAL_CALL getWrappedRawStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            const OUString& aMediaType,
            const rtl::Reference<SotMutexHolder>& aMutexHolder )
        throw ( css::packages::NoEncryptionException,
                css::io::IOException,
                css::packages::zip::ZipException,
                css::uno::RuntimeException );

    ZipEnumeration * SAL_CALL entries(  );
protected:
    bool        readLOC ( ZipEntry &rEntry)
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);
    sal_Int32       readCEN()
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);
    sal_Int32       findEND()
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);
    sal_Int32       recover()
        throw(css::io::IOException, css::packages::zip::ZipException, css::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
