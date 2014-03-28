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
#include <cppuhelper/implbase2.hxx>
#include <tools/stream.hxx>

namespace utl
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

/**
 * The helper implementation for a using input streams based on SvLockBytes.
 *
 * @author  Dirk Grobler
 * @since   00/28/03
 */
    typedef ::cppu::WeakImplHelper2<stario::XInputStream, stario::XSeekable> InputStreamHelper_Base;
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

// staruno::XInterface
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;
    virtual void SAL_CALL release() throw () SAL_OVERRIDE;

// stario::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL readSomeBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL available(  ) throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL closeInput(  ) throw (stario::NotConnectedException, stario::IOException, staruno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL seek( sal_Int64 location ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int64 SAL_CALL getPosition(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int64 SAL_CALL getLength(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

}   // namespace utl


#endif // _UNOTOOLS_STREAM_WRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
