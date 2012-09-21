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
#ifndef _ZIP_FILE_HXX
#define _ZIP_FILE_HXX

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
    namespace lang { class XMultiServiceFactory; }
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
#define ZIP_MAXEXTRA 256
#define ZIP_MAXENTRIES (0x10000 - 2)

class ZipEnumeration;

class ZipFile
{
protected:
    ::osl::Mutex    m_aMutex;

    ::rtl::OUString sComment;       /* zip file comment */
    EntryHash       aEntries;
    ByteGrabber     aGrabber;
    ZipUtils::Inflater aInflater;
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xStream;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > xSeek;
    const ::com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XProgressHandler > xProgressHandler;

    sal_Bool bRecoveryMode;

    com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createMemoryStream(
            ZipEntry & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Bool bRawStream,
            sal_Bool bDecrypt );

    com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createFileStream(
            ZipEntry & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Bool bRawStream,
            sal_Bool bDecrypt );

    // aMediaType parameter is used only for raw stream header creation
    com::sun::star::uno::Reference < com::sun::star::io::XInputStream >  createUnbufferedStream(
            SotMutexHolderRef aMutexHolder,
            ZipEntry & rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Int8 nStreamMode,
            sal_Bool bDecrypt,
            ::rtl::OUString aMediaType = ::rtl::OUString() );

    sal_Bool hasValidPassword ( ZipEntry & rEntry, const rtl::Reference < EncryptionData > &rData );

    sal_Bool checkSizeAndCRC( const ZipEntry& aEntry );

    sal_Int32 getCRC( sal_Int64 nOffset, sal_Int64 nSize );

    void getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC );

public:

    ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput,
             const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory,
             sal_Bool bInitialise
             )
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

    ZipFile( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &xInput,
             const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > &xNewFactory,
             sal_Bool bInitialise,
             sal_Bool bForceRecover,
             ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XProgressHandler > xProgress
             )
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

    ~ZipFile();

    EntryHash& GetEntryHash() { return aEntries; }

    void setInputStream ( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewStream );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getRawData(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Bool bDecrypt,
            SotMutexHolderRef aMutexHolder )
        throw(::com::sun::star::io::IOException, ::com::sun::star::packages::zip::ZipException, ::com::sun::star::uno::RuntimeException);


    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XDigestContext > StaticGetDigestContextForChecksum(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xArgFactory,
            const ::rtl::Reference< EncryptionData >& xEncryptionData );

    static ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > StaticGetCipher(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xArgFactory,
            const ::rtl::Reference< EncryptionData >& xEncryptionData,
            bool bEncrypt );

    static void StaticFillHeader ( const ::rtl::Reference < EncryptionData > & rData,
                                    sal_Int64 nSize,
                                    const ::rtl::OUString& aMediaType,
                                    sal_Int8 * & pHeader );

    static sal_Bool StaticFillData ( ::rtl::Reference < BaseEncryptionData > & rData,
                                     sal_Int32 &rEncAlgorithm,
                                     sal_Int32 &rChecksumAlgorithm,
                                     sal_Int32 &rDerivedKeySize,
                                     sal_Int32 &rStartKeyGenID,
                                     sal_Int32 &rSize,
                                     ::rtl::OUString& aMediaType,
                                     const ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& rStream );

    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > StaticGetDataFromRawStream(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xStream,
            const ::rtl::Reference < EncryptionData > &rData )
        throw ( ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::packages::zip::ZipIOException,
                ::com::sun::star::uno::RuntimeException );

    static sal_Bool StaticHasValidPassword (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
            const ::com::sun::star::uno::Sequence< sal_Int8 > &aReadBuffer,
            const ::rtl::Reference < EncryptionData > &rData );


    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Bool bDecrypt,
            SotMutexHolderRef aMutexHolder )
        throw(::com::sun::star::io::IOException, ::com::sun::star::packages::zip::ZipException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getDataStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            sal_Bool bDecrypt,
            SotMutexHolderRef aMutexHolder )
        throw ( ::com::sun::star::packages::WrongPasswordException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::packages::zip::ZipException,
                ::com::sun::star::uno::RuntimeException );

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getWrappedRawStream(
            ZipEntry& rEntry,
            const ::rtl::Reference < EncryptionData > &rData,
            const ::rtl::OUString& aMediaType,
            SotMutexHolderRef aMutexHolder )
        throw ( ::com::sun::star::packages::NoEncryptionException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::packages::zip::ZipException,
                ::com::sun::star::uno::RuntimeException );

    ZipEnumeration * SAL_CALL entries(  );
protected:
    sal_Bool        readLOC ( ZipEntry &rEntry)
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
    sal_Int32       readCEN()
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
    sal_Int32       findEND()
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);
    sal_Int32       recover()
        throw(::com::sun::star::io::IOException, com::sun::star::packages::zip::ZipException, com::sun::star::uno::RuntimeException);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
