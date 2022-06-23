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
#ifndef INCLUDED_PACKAGE_SOURCE_XSTOR_SWITCHPERSISTENCESTREAM_HXX
#define INCLUDED_PACKAGE_SOURCE_XSTOR_SWITCHPERSISTENCESTREAM_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <mutex>
#include <cppuhelper/implbase.hxx>

// SwitchablePersistenceStream

// Allows to switch the stream persistence on the fly. The target
// stream ( if not filled by the implementation ) MUST have the same
// size as the original one!

struct SPStreamData_Impl;
class SwitchablePersistenceStream
        : public ::cppu::WeakImplHelper <
                                        css::io::XStream,
                                        css::io::XInputStream,
                                        css::io::XOutputStream,
                                        css::io::XTruncate,
                                        css::io::XSeekable,
                                        css::io::XAsyncOutputMonitor >
{
    std::mutex    m_aMutex;

    const css::uno::Reference< css::uno::XComponentContext > m_xContext;

    std::unique_ptr<SPStreamData_Impl> m_pStreamData;

    void CloseAll_Impl();

public:

    SwitchablePersistenceStream(
        css::uno::Reference< css::uno::XComponentContext > xContext,
        const css::uno::Reference< css::io::XStream >& xStream );

    SwitchablePersistenceStream(
        css::uno::Reference< css::uno::XComponentContext > xContext,
        const css::uno::Reference< css::io::XInputStream >& xInStream );

    virtual ~SwitchablePersistenceStream() override;

    void SwitchPersistenceTo( const css::uno::Reference< css::io::XStream >& xStream );

    void SwitchPersistenceTo( const css::uno::Reference< css::io::XInputStream >& xInputStream );

    void CopyAndSwitchPersistenceTo( const css::uno::Reference< css::io::XStream >& xStream );

// css::io::XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) override;

// css::io::XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;

// css::io::XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< ::sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;

// css::io::XTruncate
    virtual void SAL_CALL truncate(  ) override;

// css::io::XSeekable
    virtual void SAL_CALL seek( ::sal_Int64 location ) override;
    virtual ::sal_Int64 SAL_CALL getPosition(  ) override;
    virtual ::sal_Int64 SAL_CALL getLength(  ) override;

// css::io::XAsyncOutputMonitor
    virtual void SAL_CALL waitForCompletion(  ) override;

};

#endif // INCLUDED_PACKAGE_SOURCE_XSTOR_SWITCHPERSISTENCESTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
