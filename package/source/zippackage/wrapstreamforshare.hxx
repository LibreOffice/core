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

#ifndef INCLUDED_PACKAGE_SOURCE_ZIPPACKAGE_WRAPSTREAMFORSHARE_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPPACKAGE_WRAPSTREAMFORSHARE_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/refcountedmutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

class WrapStreamForShare : public cppu::WeakImplHelper < css::io::XInputStream
                                                        , css::io::XSeekable >
{
protected:
    rtl::Reference< comphelper::RefCountedMutex > m_xMutex;
    css::uno::Reference < css::io::XInputStream > m_xInStream;
    css::uno::Reference < css::io::XSeekable > m_xSeekable;

    sal_Int64 m_nCurPos;

public:
    WrapStreamForShare( const css::uno::Reference< css::io::XInputStream >& xInStream,
                        const rtl::Reference< comphelper::RefCountedMutex >& rMutexRef );
    virtual ~WrapStreamForShare() override;

    // XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override;
    virtual sal_Int32 SAL_CALL available(  ) override;
    virtual void SAL_CALL closeInput(  ) override;

    //XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
