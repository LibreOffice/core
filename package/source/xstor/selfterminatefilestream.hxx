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

#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_SELFTERMINATEFILESTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_SELFTERMINATEFILESTREAM_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/bytereader.hxx>
#include <cppuhelper/implbase.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <rtl/ref.hxx>
#include <optional>

class OSelfTerminateFileStream final : public cppu::WeakImplHelper< css::io::XInputStream,
                                                               css::io::XSeekable >,
                                        public comphelper::ByteReader
{
    std::optional<utl::TempFileFast> m_oTempFile;
    rtl::Reference< utl::OSeekableInputStreamWrapper > m_xStreamWrapper;

public:
    OSelfTerminateFileStream( const css::uno::Reference< css::uno::XComponentContext >& xContext, utl::TempFileFast aTempFile );

    virtual ~OSelfTerminateFileStream() override;

    void CloseStreamDeleteFile();

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

    // comphelper::ByteReader
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
