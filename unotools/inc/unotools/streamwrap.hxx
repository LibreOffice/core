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

#ifndef _UTL_STREAM_WRAPPER_HXX_
#define _UTL_STREAM_WRAPPER_HXX_
#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>

class SvStream;

namespace utl
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

//==================================================================
//= OInputStreamWrapper
//==================================================================
typedef ::cppu::WeakImplHelper1 <   stario::XInputStream
                                > InputStreamWrapper_Base;
    // needed for some compilers
/// helper class for wrapping an SvStream into an <type scope="com.sun.star.io">XInputStream</type>
class UNOTOOLS_DLLPUBLIC OInputStreamWrapper : public InputStreamWrapper_Base
{
protected:
    ::osl::Mutex    m_aMutex;
    SvStream*       m_pSvStream;
    sal_Bool        m_bSvStreamOwner : 1;
    OInputStreamWrapper()
                    { m_pSvStream = 0; m_bSvStreamOwner = sal_False; }
    void            SetStream(SvStream* _pStream, sal_Bool bOwner )
                    { m_pSvStream = _pStream; m_bSvStreamOwner = bOwner; }

public:
    OInputStreamWrapper(SvStream& _rStream);
    OInputStreamWrapper(SvStream* pStream, sal_Bool bOwner=sal_False);
    virtual ~OInputStreamWrapper();

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OInputStreamWrapper, InputStreamWrapper_Base);

// stario::XInputStream
    virtual sal_Int32   SAL_CALL    readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);

protected:
    /// throws a NotConnectedException if the object is not connected anymore
    void checkConnected() const;
    /// throws an exception according to the error flag of m_pSvStream
    void checkError() const;
};

//==================================================================
//= OSeekableInputStreamWrapper
//==================================================================
typedef ::cppu::ImplHelper1 <   ::com::sun::star::io::XSeekable
                            >   OSeekableInputStreamWrapper_Base;
/** helper class for wrapping an SvStream into an <type scope="com.sun.star.io">XInputStream</type>
    which is seekable (i.e. supports the <type scope="com.sun.star.io">XSeekable</type> interface).
*/
class UNOTOOLS_DLLPUBLIC OSeekableInputStreamWrapper : public ::cppu::ImplInheritanceHelper1 < OInputStreamWrapper, com::sun::star::io::XSeekable >
{
protected:
    OSeekableInputStreamWrapper() {}
public:
    OSeekableInputStreamWrapper(SvStream& _rStream);
    OSeekableInputStreamWrapper(SvStream* _pStream, sal_Bool _bOwner = sal_False);

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

//==================================================================
//= OOutputStreamWrapper
//==================================================================
typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamWrapper_Base;
    // needed for some compilers
class UNOTOOLS_DLLPUBLIC OOutputStreamWrapper : public OutputStreamWrapper_Base
{
protected:
    // TODO: thread safety!
    SvStream&       rStream;

public:
    OOutputStreamWrapper(SvStream& _rStream) :rStream(_rStream) { }

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OOutputStreamWrapper, OutputStreamWrapper_Base);

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

protected:
    /// throws an exception according to the error flag of m_pSvStream
    void checkError() const;
};

//==================================================================
//= OSeekableOutputStreamWrapper
//==================================================================
typedef ::cppu::ImplHelper1 <   ::com::sun::star::io::XSeekable
                            >   OSeekableOutputStreamWrapper_Base;
/** helper class for wrapping an SvStream into an <type scope="com.sun.star.io">XOutputStream</type>
    which is seekable (i.e. supports the <type scope="com.sun.star.io">XSeekable</type> interface).
*/
class UNOTOOLS_DLLPUBLIC OSeekableOutputStreamWrapper
                :public OOutputStreamWrapper
                ,public OSeekableOutputStreamWrapper_Base
{
public:
    OSeekableOutputStreamWrapper(SvStream& _rStream);

    // disambiguate XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getPosition(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int64 SAL_CALL getLength(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

class UNOTOOLS_DLLPUBLIC OStreamWrapper : public ::cppu::ImplInheritanceHelper3 < OSeekableInputStreamWrapper, com::sun::star::io::XStream, com::sun::star::io::XOutputStream, com::sun::star::io::XTruncate >
{
public:
    OStreamWrapper(SvStream& _rStream);

// stario::XStream
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL truncate() throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
};

}
// namespace utl

#endif // _UTL_STREAM_WRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
