/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oslfile2streamwrap.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#define _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

#include <osl/mutex.hxx>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>
#include "comphelper/comphelperdllapi.h"

namespace comphelper
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

//==================================================================
// FmUnoIOStream,
// stream zum schreiben un lesen von Daten, basieren  auf File
//==================================================================
struct InputStreamWrapper_Base : public ::cppu::WeakImplHelper1<stario::XInputStream>
{};

class COMPHELPER_DLLPUBLIC OSLInputStreamWrapper : public InputStreamWrapper_Base
{
    ::osl::Mutex    m_aMutex;
    ::osl::File*    m_pFile;
    sal_Bool        m_bFileOwner : 1;

public:
    OSLInputStreamWrapper(::osl::File& _rStream);
    OSLInputStreamWrapper(::osl::File* pStream, sal_Bool bOwner=sal_False);
    virtual ~OSLInputStreamWrapper();

// UNO Anbindung
    virtual void            SAL_CALL acquire() throw()
        { InputStreamWrapper_Base::acquire(); }
    virtual void            SAL_CALL release() throw()
        { InputStreamWrapper_Base::release(); }
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
        { return InputStreamWrapper_Base::queryInterface(_rType); }

// stario::XInputStream
    virtual sal_Int32   SAL_CALL    readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
};

//==================================================================
// FmUnoOutStream,
// Datensenke fuer Files
//==================================================================
struct OutputStreamWrapper_Base : public ::cppu::WeakImplHelper1<stario::XOutputStream>
{};

class COMPHELPER_DLLPUBLIC OSLOutputStreamWrapper : public OutputStreamWrapper_Base
{
    ::osl::File&        rFile;

public:
    OSLOutputStreamWrapper(::osl::File& _rFile) :rFile(_rFile) { }

// UNO Anbindung
    virtual void            SAL_CALL acquire() throw()
        { OutputStreamWrapper_Base::acquire(); }
    virtual void            SAL_CALL release() throw()
        { OutputStreamWrapper_Base::release(); }
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException)
        { return OutputStreamWrapper_Base::queryInterface(_rType); }

// stario::XOutputStream
    virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    virtual void SAL_CALL closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
};

}   // namespace comphelper


#endif // _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

