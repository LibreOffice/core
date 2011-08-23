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

#ifndef SVTOOLS_STRMADPT_HXX
#define SVTOOLS_STRMADPT_HXX

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
namespace binfilter
{


//============================================================================
class  SvOutputStreamOpenLockBytes: public SvOpenLockBytes
{
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        m_xOutputStream;
    sal_uInt32 m_nPosition;

public:
    TYPEINFO();

    SvOutputStreamOpenLockBytes(
            const com::sun::star::uno::Reference<
                      com::sun::star::io::XOutputStream > &
                rTheOutputStream):
        m_xOutputStream(rTheOutputStream), m_nPosition(0) {}

    virtual ErrCode ReadAt(ULONG, void *, ULONG, ULONG *) const;

    virtual ErrCode WriteAt(ULONG nPos, const void * pBuffer, ULONG nCount,
                            ULONG * pWritten);

    virtual ErrCode Flush() const;

    virtual ErrCode SetSize(ULONG);

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const;

    virtual ErrCode FillAppend(const void * pBuffer, ULONG nCount,
                               ULONG * pWritten);

    virtual ULONG Tell() const;

    virtual ULONG Seek(ULONG);

    virtual void Terminate();
};

//============================================================================
class  SvLockBytesInputStream: public cppu::OWeakObject,
                              public com::sun::star::io::XInputStream,
                              public com::sun::star::io::XSeekable
{
    SvLockBytesRef m_xLockBytes;
    sal_Int64 m_nPosition;
    bool m_bDone;

public:
    SvLockBytesInputStream(SvLockBytes * pTheLockBytes):
        m_xLockBytes(pTheLockBytes), m_nPosition(0), m_bDone(false) {}

    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface(const com::sun::star::uno::Type & rType)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw();

    virtual void SAL_CALL release() throw();

    virtual sal_Int32 SAL_CALL
    readBytes(com::sun::star::uno::Sequence< sal_Int8 > & rData,
              sal_Int32 nBytesToRead)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL
    readSomeBytes(com::sun::star::uno::Sequence< sal_Int8 > & rData,
                  sal_Int32 nMaxBytesToRead)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL available()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL closeInput()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL seek(sal_Int64 nLocation)
        throw (com::sun::star::lang::IllegalArgumentException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual sal_Int64 SAL_CALL getPosition()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);

    virtual sal_Int64 SAL_CALL getLength()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException);
};

}

#endif // SVTOOLS_STRMADPT_HXX

