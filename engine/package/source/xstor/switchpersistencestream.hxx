/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <mutex>
#include <comphelper/bytereader.hxx>
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
                                        css::io::XAsyncOutputMonitor >,
    public comphelper::ByteReader
{
    std::mutex    m_aMutex;

    std::unique_ptr<SPStreamData_Impl> m_pStreamData;

    void CloseAll_Impl();

public:

    SwitchablePersistenceStream(
        const cpo::uno::Reference< css::io::XStream >& xStream );

    SwitchablePersistenceStream(
        const cpo::uno::Reference< css::io::XInputStream >& xInStream );

    virtual ~SwitchablePersistenceStream() override;

    void SwitchPersistenceTo( const cpo::uno::Reference< css::io::XStream >& xStream );

    void SwitchPersistenceTo( const cpo::uno::Reference< css::io::XInputStream >& xInputStream );

    void CopyAndSwitchPersistenceTo( const cpo::uno::Reference< css::io::XStream >& xStream );

// css::io::XStream
    virtual cpo::uno::Reference< css::io::XInputStream > getInputStream(  ) override;
    virtual cpo::uno::Reference< css::io::XOutputStream > getOutputStream(  ) override;

// css::io::XInputStream
    virtual ::sal_Int32 readBytes( cpo::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    virtual ::sal_Int32 readSomeBytes( cpo::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    virtual void skipBytes( ::sal_Int32 nBytesToSkip ) override;
    virtual ::sal_Int32 available(  ) override;
    virtual void closeInput(  ) override;

// css::io::XOutputStream
    virtual void writeBytes( const cpo::uno::Sequence< ::sal_Int8 >& aData ) override;
    virtual void flush(  ) override;
    virtual void closeOutput(  ) override;

// css::io::XTruncate
    virtual void truncate(  ) override;

// css::io::XSeekable
    virtual void seek( ::sal_Int64 location ) override;
    virtual ::sal_Int64 getPosition(  ) override;
    virtual ::sal_Int64 getLength(  ) override;

// css::io::XAsyncOutputMonitor
    virtual void waitForCompletion(  ) override;

    // comphelper::ByteReader
    virtual sal_Int32 readSomeBytes(sal_Int8* aData, sal_Int32 nBytesToRead) override;
};

#endif // INCLUDED_PACKAGE_SOURCE_XSTOR_SWITCHPERSISTENCESTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
