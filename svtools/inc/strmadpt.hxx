/*************************************************************************
 *
 *  $RCSfile: strmadpt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

//============================================================================
class SvOutputStreamOpenLockBytes: public SvOpenLockBytes
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
class SvLockBytesInputStream: public cppu::OWeakObject,
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

    virtual void SAL_CALL acquire()
        throw(com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL release()
        throw(com::sun::star::uno::RuntimeException);

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

#endif // SVTOOLS_STRMADPT_HXX

