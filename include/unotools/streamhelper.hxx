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
#include <unotools/unotoolsdllapi.h>

#ifndef INCLUDED_UNOTOOLS_STREAMHELPER_HXX
#define INCLUDED_UNOTOOLS_STREAMHELPER_HXX
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/stream.hxx>

namespace utl
{

/**
 * The helper implementation for a using input streams based on SvLockBytes.
 *
 * @author  Dirk Grobler
 * @since   00/28/03
 */
    typedef ::cppu::WeakImplHelper <css::io::XInputStream, css::io::XSeekable> InputStreamHelper_Base;
    // needed for some compilers
class UNOTOOLS_DLLPUBLIC OInputStreamHelper : public InputStreamHelper_Base
{
    ::osl::Mutex    m_aMutex;
    SvLockBytesRef  m_xLockBytes;
    sal_uInt64      m_nActPos;
    sal_Int32       m_nAvailable;   // this is typically the chunk(buffer) size

public:
    OInputStreamHelper(const SvLockBytesRef& _xLockBytes,
                       sal_uInt32 _nAvailable,
                       sal_uInt64 _nPos = 0)
        :m_xLockBytes(_xLockBytes)
        ,m_nActPos(_nPos)
        ,m_nAvailable(_nAvailable){}

// css::uno::XInterface
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

// css::io::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL available(  ) throw(css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeInput(  ) throw (css::io::NotConnectedException, css::io::IOException, css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL seek( sal_Int64 location ) throw(css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition(  ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength(  ) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
};

}   // namespace utl

#endif // _UNOTOOLS_STREAM_WRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
