/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oslfile2streamwrap.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:34:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#define _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_

#ifndef _OSL_MUTEX_HXX_ //autogen wg. ::osl::Mutex
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef INCLUDED_COMPHELPERDLLAPI_H
#include "comphelper/comphelperdllapi.h"
#endif

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

