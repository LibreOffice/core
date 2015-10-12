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

#ifndef INCLUDED_SVL_SOURCE_FSSTOR_OINPUTSTREAMCONTAINER_HXX
#define INCLUDED_SVL_SOURCE_FSSTOR_OINPUTSTREAMCONTAINER_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XExtendedStorageStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>


#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <osl/mutex.hxx>

class OFSInputStreamContainer : public cppu::WeakImplHelper < ::com::sun::star::io::XInputStream
                                                            ,::com::sun::star::embed::XExtendedStorageStream >
                            , public ::com::sun::star::io::XSeekable
{
    ::osl::Mutex m_aMutex;

    ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream > m_xInputStream;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XSeekable > m_xSeekable;

    bool m_bSeekable;

    bool m_bDisposed;

    ::cppu::OInterfaceContainerHelper* m_pListenersContainer; // list of listeners

public:
    explicit OFSInputStreamContainer( const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xStream );

    virtual ~OFSInputStreamContainer();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  )
        throw(::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
