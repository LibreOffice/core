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
#ifndef _ZIP_FILE_HXX
#define _ZIP_FILE_HXX

#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>

#include <rtl/ref.hxx>

#include <ByteGrabber.hxx>
#include <HashMaps.hxx>
#include <Inflater.hxx>
#include <EncryptionData.hxx>

#include <mutexholder.hxx>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb  { class XProgressHandler; }
} } }

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
    Inflater        aInflater;
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

    sal_Bool hasValidPassword ( ZipEntry & rEntry, const ::rtl::Reference < EncryptionData > &rData );

    sal_Bool checkSizeAndCRC( const ZipEntry& aEntry );

    sal_Int32 getCRC( sal_Int32 nOffset, sal_Int32 nSize );

    void getSizeAndCRC( sal_Int32 nOffset, sal_Int32 nCompressedSize, sal_Int32 *nSize, sal_Int32 *nCRC );

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
                                    sal_Int32 nSize,
                                    const ::rtl::OUString& aMediaType,
                                    sal_Int8 * & pHeader );

    static sal_Bool StaticFillData ( ::rtl::Reference < BaseEncryptionData > & rData,
                                     sal_Int32 &rEncAlgorithm,
                                     sal_Int32 &rChecksumAlgorithm,
                                     sal_Int32 &rDerivedKeySize,
                                     sal_Int32 &rStartKeyGenID,
                                     sal_Int32 &rSize,
                                     ::rtl::OUString& aMediaType,
                                     ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream > &rStream );

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
