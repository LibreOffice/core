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
#pragma once

#include <comphelper/comphelperdllapi.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekableInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/bytereader.hxx>
#include <boost/core/noinit_adaptor.hpp>
#include <vector>

namespace comphelper
{
class COMPHELPER_DLLPUBLIC UNOMemoryStream
    : public cppu::WeakImplHelper<css::lang::XServiceInfo, css::io::XStream,
                                  css::io::XSeekableInputStream, css::io::XOutputStream,
                                  css::io::XTruncate>,
      public comphelper::ByteWriter,
      public comphelper::ByteReader
{
public:
    UNOMemoryStream();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XStream
    virtual css::uno::Reference<XInputStream> SAL_CALL getInputStream() override;
    virtual css::uno::Reference<XOutputStream> SAL_CALL getOutputStream() override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes(css::uno::Sequence<sal_Int8>& aData,
                                         sal_Int32 nBytesToRead) override;
    virtual sal_Int32 SAL_CALL readSomeBytes(css::uno::Sequence<sal_Int8>& aData,
                                             sal_Int32 nMaxBytesToRead) override;
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

    // XSeekable
    virtual void SAL_CALL seek(sal_Int64 location) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

    // XOutputStream
    virtual void SAL_CALL writeBytes(const css::uno::Sequence<sal_Int8>& aData) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL closeOutput() override;

    // XTruncate
    virtual void SAL_CALL truncate() override;

    // comphelper::ByteWriter
    virtual void writeBytes(const sal_Int8* aData, sal_Int32 nBytesToWrite) override;

    // comphelper::ByteReader
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;

private:
    std::vector<sal_Int8, boost::noinit_adaptor<std::allocator<sal_Int8>>> maData;
    sal_Int32 mnCursor;
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
