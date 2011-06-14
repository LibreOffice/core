/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#define _UNOTOOLS_STREAMHELPER_HXX_
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
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
    sal_uInt32      m_nActPos;
    sal_Int32       m_nAvailable;   // this is typically the chunk(buffer) size

public:
    OInputStreamHelper(const SvLockBytesRef& _xLockBytes,
                       sal_uInt32 _nAvailable,
                       sal_uInt32 _nPos = 0)
        :m_xLockBytes(_xLockBytes)
        ,m_nActPos(_nPos)
        ,m_nAvailable(_nAvailable){}

// staruno::XInterface
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

// stario::XInputStream
    virtual sal_Int32 SAL_CALL readBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL readSomeBytes( staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual sal_Int32 SAL_CALL available(  ) throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL closeInput(  ) throw (stario::NotConnectedException, stario::IOException, staruno::RuntimeException);

    virtual void SAL_CALL seek( sal_Int64 location ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

/**
 * The helper implementation for a using output streams based on SvLockBytes.
 *
 * @author  Dirk Grobler
 * @since   00/28/03
 */
typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamHelper_Base;
    // needed for some compilers
class UNOTOOLS_DLLPUBLIC OOutputStreamHelper : public OutputStreamHelper_Base
{
    ::osl::Mutex    m_aMutex;
    SvLockBytesRef  m_xLockBytes;
    sal_uInt32      m_nActPos;

public:
    OOutputStreamHelper(const SvLockBytesRef& _xLockBytes, sal_uInt32 _nPos = 0)
        :m_xLockBytes(_xLockBytes)
        ,m_nActPos(_nPos){}

// staruno::XInterface
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

// stario::XOutputStream
    virtual void SAL_CALL writeBytes( const staruno::Sequence< sal_Int8 >& aData ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL flush(  ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput(  ) throw(stario::NotConnectedException, stario::BufferSizeExceededException, stario::IOException, staruno::RuntimeException);
};

}   // namespace utl


#endif // _UNOTOOLS_STREAM_WRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
