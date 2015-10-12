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

#ifndef INCLUDED_UNOTOOLS_STREAMWRAP_HXX
#define INCLUDED_UNOTOOLS_STREAMWRAP_HXX

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

//= OInputStreamWrapper

typedef ::cppu::WeakImplHelper1 <   css::io::XInputStream
                                > InputStreamWrapper_Base;
// needed for some compilers
/// helper class for wrapping an SvStream into an com.sun.star.io::XInputStream
class UNOTOOLS_DLLPUBLIC OInputStreamWrapper : public InputStreamWrapper_Base
{
protected:
    ::osl::Mutex    m_aMutex;
    SvStream*       m_pSvStream;
    bool        m_bSvStreamOwner : 1;
    OInputStreamWrapper()
                    { m_pSvStream = 0; m_bSvStreamOwner = false; }
    void            SetStream(SvStream* _pStream, bool bOwner )
                    { m_pSvStream = _pStream; m_bSvStreamOwner = bOwner; }

public:
    OInputStreamWrapper(SvStream& _rStream);
    OInputStreamWrapper(SvStream* pStream, bool bOwner=false);
    virtual ~OInputStreamWrapper();

// css::io::XInputStream
    virtual sal_Int32   SAL_CALL    readBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32   SAL_CALL    readSomeBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32   SAL_CALL    available() throw(css::io::NotConnectedException, css::uno::RuntimeException, std::exception) override;
    virtual void        SAL_CALL    closeInput() throw(css::io::NotConnectedException, css::uno::RuntimeException, std::exception) override;

protected:
    /// throws a NotConnectedException if the object is not connected anymore
    void checkConnected() const;
    /// throws an exception according to the error flag of m_pSvStream
    void checkError() const;
};

//= OSeekableInputStreamWrapper

typedef ::cppu::ImplHelper1 <   css::io::XSeekable
                            >   OSeekableInputStreamWrapper_Base;
/** helper class for wrapping an SvStream into an com.sun.star.io::XInputStream
    which is seekable (i.e. supports the com.sun.star.io::XSeekable interface).
*/
class UNOTOOLS_DLLPUBLIC OSeekableInputStreamWrapper : public ::cppu::ImplInheritanceHelper1 < OInputStreamWrapper, css::io::XSeekable >
{
protected:
    OSeekableInputStreamWrapper() {}
public:
    OSeekableInputStreamWrapper(SvStream& _rStream);
    OSeekableInputStreamWrapper(SvStream* _pStream, bool _bOwner = false);

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
};

//= OOutputStreamWrapper

typedef ::cppu::WeakImplHelper1<css::io::XOutputStream> OutputStreamWrapper_Base;
    // needed for some compilers
class OOutputStreamWrapper : public OutputStreamWrapper_Base
{
public:
    UNOTOOLS_DLLPUBLIC OOutputStreamWrapper(SvStream& _rStream);

protected:
    virtual ~OOutputStreamWrapper();

// css::io::XOutputStream
    virtual void SAL_CALL writeBytes(const css::uno::Sequence< sal_Int8 >& aData) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL flush() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeOutput() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;

    /// throws an exception according to the error flag of m_pSvStream
    void checkError() const;

    // TODO: thread safety!
    SvStream&       rStream;
};

//= OSeekableOutputStreamWrapper

typedef ::cppu::ImplHelper1 <   css::io::XSeekable
                            >   OSeekableOutputStreamWrapper_Base;
/** helper class for wrapping an SvStream into an com.sun.star.io::XOutputStream
    which is seekable (i.e. supports the com.sun.star.io::XSeekable interface).
*/
class UNOTOOLS_DLLPUBLIC OSeekableOutputStreamWrapper
                :public OOutputStreamWrapper
                ,public OSeekableOutputStreamWrapper_Base
{
public:
    OSeekableOutputStreamWrapper(SvStream& _rStream);

private:
    virtual ~OSeekableOutputStreamWrapper();

    // disambiguate XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 _nLocation ) throw (css::lang::IllegalArgumentException, css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getPosition(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLength(  ) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
};

class UNOTOOLS_DLLPUBLIC OStreamWrapper : public ::cppu::ImplInheritanceHelper3 < OSeekableInputStreamWrapper, css::io::XStream, css::io::XOutputStream, css::io::XTruncate >
{
public:
    OStreamWrapper(SvStream& _rStream);

// css::io::XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) throw (css::uno::RuntimeException, std::exception) override;

// css::io::XOutputStream
    virtual void SAL_CALL writeBytes(const css::uno::Sequence< sal_Int8 >& aData) throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL flush() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL closeOutput() throw(css::io::NotConnectedException, css::io::BufferSizeExceededException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL truncate() throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
};

}
// namespace utl

#endif // INCLUDED_UNOTOOLS_STREAMWRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
