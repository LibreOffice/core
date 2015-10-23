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
#ifndef INCLUDED_PACKAGE_SOURCE_ZIPAPI_XUNBUFFEREDSTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPAPI_XUNBUFFEREDSTREAM_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>
#include <package/Inflater.hxx>
#include <ZipEntry.hxx>
#include <CRC32.hxx>
#include <mutexholder.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

#define UNBUFF_STREAM_DATA          0
#define UNBUFF_STREAM_RAW           1
#define UNBUFF_STREAM_WRAPPEDRAW    2

class EncryptionData;
class XUnbufferedStream : public cppu::WeakImplHelper
<
    css::io::XInputStream
>
{
protected:
    rtl::Reference<SotMutexHolder> maMutexHolder;

    css::uno::Reference < css::io::XInputStream > mxZipStream;
    css::uno::Reference < css::io::XSeekable > mxZipSeek;
    css::uno::Sequence < sal_Int8 > maCompBuffer, maHeader;
    ZipEntry maEntry;
    ::rtl::Reference< EncryptionData > mxData;
    sal_Int32 mnBlockSize;
    css::uno::Reference< css::xml::crypto::XCipherContext > m_xCipherContext;
    ZipUtils::Inflater maInflater;
    bool mbRawStream, mbWrappedRaw, mbFinished;
    sal_Int16 mnHeaderToRead;
    sal_Int64 mnZipCurrent, mnZipEnd, mnZipSize, mnMyCurrent;
    CRC32 maCRC;
    bool mbCheckCRC;

public:
    XUnbufferedStream(
                 const css::uno::Reference< css::uno::XComponentContext >& xContext,
                 const rtl::Reference<SotMutexHolder>& aMutexHolder,
                 ZipEntry & rEntry,
                 css::uno::Reference < css::io::XInputStream > xNewZipStream,
                 const ::rtl::Reference< EncryptionData >& rData,
                 sal_Int8 nStreamMode,
                 bool bIsEncrypted,
                 const OUString& aMediaType,
                 bool bRecoveryMode );

    // allows to read package raw stream
    XUnbufferedStream(
                 const css::uno::Reference< css::uno::XComponentContext >& xContext,
                 const css::uno::Reference < css::io::XInputStream >& xRawStream,
                 const ::rtl::Reference< EncryptionData >& rData );

    virtual ~XUnbufferedStream();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available(  )
        throw(css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  )
        throw(css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
