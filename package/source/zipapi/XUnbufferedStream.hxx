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
#ifndef _XUNBUFFERED_STREAM_HXX
#define _XUNBUFFERED_STREAM_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <package/Inflater.hxx>
#include <ZipEntry.hxx>
#include <CRC32.hxx>
#include <mutexholder.hxx>

#define UNBUFF_STREAM_DATA          0
#define UNBUFF_STREAM_RAW           1
#define UNBUFF_STREAM_WRAPPEDRAW    2

class EncryptionData;
class XUnbufferedStream : public cppu::WeakImplHelper1
<
    com::sun::star::io::XInputStream
>
{
protected:
    SotMutexHolderRef maMutexHolder;

    com::sun::star::uno::Reference < com::sun::star::io::XInputStream > mxZipStream;
    com::sun::star::uno::Reference < com::sun::star::io::XSeekable > mxZipSeek;
    com::sun::star::uno::Sequence < sal_Int8 > maCompBuffer, maHeader;
    ZipEntry maEntry;
    ::rtl::Reference< EncryptionData > mxData;
    sal_Int32 mnBlockSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XCipherContext > m_xCipherContext;
    ZipUtils::Inflater maInflater;
    sal_Bool mbRawStream, mbWrappedRaw, mbFinished;
    sal_Int16 mnHeaderToRead;
    sal_Int64 mnZipCurrent, mnZipEnd, mnZipSize, mnMyCurrent;
    CRC32 maCRC;
    sal_Bool mbCheckCRC;

public:
    XUnbufferedStream(
                 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                 SotMutexHolderRef aMutexHolder,
                 ZipEntry & rEntry,
                 com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewZipStream,
                 const ::rtl::Reference< EncryptionData >& rData,
                 sal_Int8 nStreamMode,
                 sal_Bool bIsEncrypted,
                 const OUString& aMediaType,
                 sal_Bool bRecoveryMode );

    // allows to read package raw stream
    XUnbufferedStream(
                 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                 const com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& xRawStream,
                 const ::rtl::Reference< EncryptionData >& rData );

    virtual ~XUnbufferedStream();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    // XSeekable
    /*
    virtual void SAL_CALL seek( sal_Int64 location )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  )
        throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    */
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
