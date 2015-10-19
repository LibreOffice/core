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
#ifndef INCLUDED_COMPHELPER_SEEKABLEINPUT_HXX
#define INCLUDED_COMPHELPER_SEEKABLEINPUT_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>

namespace comphelper
{

class SAL_DLLPUBLIC_TEMPLATE OSeekableInputWrapper_BASE
    : public ::cppu::WeakImplHelper< css::io::XInputStream,
                                     css::io::XSeekable >
{};

class COMPHELPER_DLLPUBLIC OSeekableInputWrapper
    : public OSeekableInputWrapper_BASE
{
    ::osl::Mutex    m_aMutex;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::io::XInputStream > m_xOriginalStream;

    css::uno::Reference< css::io::XInputStream > m_xCopyInput;
    css::uno::Reference< css::io::XSeekable > m_xCopySeek;

private:
    COMPHELPER_DLLPRIVATE void PrepareCopy_Impl();

public:
    OSeekableInputWrapper(
                const css::uno::Reference< css::io::XInputStream >& xInStream,
                const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~OSeekableInputWrapper();

    static css::uno::Reference< css::io::XInputStream > CheckSeekableCanWrap(
                        const css::uno::Reference< css::io::XInputStream >& xInStream,
                        const css::uno::Reference< css::uno::XComponentContext >& rxContext );

// XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw (css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available() throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput() throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;

// XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition() throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength() throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

};

}   // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
