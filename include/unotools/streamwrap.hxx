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
class OOutputStreamWrapper : public OutputStreamWrapper_Base
{
public:
    UNOTOOLS_DLLPUBLIC OOutputStreamWrapper(SvStream& _rStream);

protected:
    virtual ~OOutputStreamWrapper();

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

    /// throws an exception according to the error flag of m_pSvStream
    void checkError() const;

    // TODO: thread safety!
    SvStream&       rStream;
};

//==================================================================
//= OSeekableOutputStreamWrapper
//==================================================================
typedef ::cppu::ImplHelper1 <   ::com::sun::star::io::XSeekable
                            >   OSeekableOutputStreamWrapper_Base;
/** helper class for wrapping an SvStream into an <type scope="com.sun.star.io">XOutputStream</type>
    which is seekable (i.e. supports the <type scope="com.sun.star.io">XSeekable</type> interface).
*/
class OSeekableOutputStreamWrapper
                :public OOutputStreamWrapper
                ,public OSeekableOutputStreamWrapper_Base
{
public:
    UNOTOOLS_DLLPUBLIC OSeekableOutputStreamWrapper(SvStream& _rStream);

private:
    virtual ~OSeekableOutputStreamWrapper();

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
