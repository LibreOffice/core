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

#ifndef INCLUDED_SVL_SOURCE_FSSTOR_OSTREAMCONTAINER_HXX
#define INCLUDED_SVL_SOURCE_FSSTOR_OSTREAMCONTAINER_HXX

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <osl/mutex.hxx>
#include <memory>

class OFSStreamContainer : public cppu::OWeakObject,
                     public css::lang::XTypeProvider,
                     public css::embed::XExtendedStorageStream,
                     public css::io::XSeekable,
                     public css::io::XInputStream,
                     public css::io::XOutputStream,
                     public css::io::XTruncate,
                     public css::io::XAsyncOutputMonitor
{
    ::osl::Mutex m_aMutex;

    css::uno::Reference< css::io::XStream >               m_xStream;
    css::uno::Reference< css::io::XSeekable >             m_xSeekable;
    css::uno::Reference< css::io::XInputStream >          m_xInputStream;
    css::uno::Reference< css::io::XOutputStream >         m_xOutputStream;
    css::uno::Reference< css::io::XTruncate >             m_xTruncate;
    css::uno::Reference< css::io::XAsyncOutputMonitor >   m_xAsyncOutputMonitor;

    bool m_bDisposed;
    bool m_bInputClosed;
    bool m_bOutputClosed;

    std::unique_ptr<::comphelper::OInterfaceContainerHelper2> m_pListenersContainer; // list of listeners
    css::uno::Sequence<css::uno::Type> m_aTypes;

public:
    explicit OFSStreamContainer( const css::uno::Reference < css::io::XStream >& xStream );
    virtual ~OFSStreamContainer() override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& rType ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    //  XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;

    // XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< sal_Int8 >& aData ) override;
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;

    // XTruncate
    virtual void SAL_CALL truncate() override;

    // XAsyncOutputMonitor
    virtual void SAL_CALL waitForCompletion(  ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
