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
#ifndef _COMPHELPER_STREAM_SEEKABLEINPUT_HXX_
#define _COMPHELPER_STREAM_SEEKABLEINPUT_HXX_

#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/implbase2.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{

class COMPHELPER_DLLPUBLIC OSeekableInputWrapper : public ::cppu::WeakImplHelper2< ::com::sun::star::io::XInputStream,
                                                                ::com::sun::star::io::XSeekable >
{
    ::osl::Mutex    m_aMutex;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xOriginalStream;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xCopyInput;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XSeekable > m_xCopySeek;

private:
    COMPHELPER_DLLPRIVATE void PrepareCopy_Impl();

public:
    OSeekableInputWrapper(
                const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

    virtual ~OSeekableInputWrapper();

    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > CheckSeekableCanWrap(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInStream,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

// XInputStream
    virtual sal_Int32 SAL_CALL readBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( ::com::sun::star::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::BufferSizeExceededException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL available() throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL closeInput() throw (::com::sun::star::io::NotConnectedException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

};

}   // namespace comphelper

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
