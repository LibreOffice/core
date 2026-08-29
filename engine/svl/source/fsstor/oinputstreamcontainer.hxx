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

#ifndef INCLUDED_SVL_SOURCE_FSSTOR_OINPUTSTREAMCONTAINER_HXX
#define INCLUDED_SVL_SOURCE_FSSTOR_OINPUTSTREAMCONTAINER_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>


#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>

#include <mutex>

class OFSInputStreamContainer : public cppu::WeakImplHelper < css::io::XInputStream
                                                            ,css::embed::XExtendedStorageStream >
                            , public css::io::XSeekable
{
    std::mutex m_aMutex;

    css::uno::Reference < css::io::XInputStream > m_xInputStream;
    css::uno::Reference < css::io::XSeekable > m_xSeekable;

    bool m_bSeekable;

    bool m_bDisposed;

    ::comphelper::OInterfaceContainerHelper4<css::lang::XEventListener> m_aListenersContainer; // list of listeners

public:
    explicit OFSInputStreamContainer( const css::uno::Reference < css::io::XInputStream >& xStream );

    virtual ~OFSInputStreamContainer() override;

    virtual cpo::uno::Sequence< cpo::uno::Type > getTypes() override;
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& rType ) override;
    virtual void acquire() noexcept override;
    virtual void release() noexcept override;

    // XInputStream
    virtual sal_Int32 readBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 readSomeBytes( cpo::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 available(  ) override;
    virtual void closeInput(  ) override;

    //XStream
    virtual css::uno::Reference< css::io::XInputStream > getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > getOutputStream(  ) override;

    //XSeekable
    virtual void seek( sal_Int64 location ) override;
    virtual sal_Int64 getPosition() override;
    virtual sal_Int64 getLength() override;

    //XComponent
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
