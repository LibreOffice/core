/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: streamhelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 09:22:15 $
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
#ifndef INCLUDED_UNOTOOLSDLLAPI_H
#include "unotools/unotoolsdllapi.h"
#endif

#ifndef _UNOTOOLS_STREAMHELPER_HXX_
#define _UNOTOOLS_STREAMHELPER_HXX_

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen wg. ::osl::Mutex
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

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

