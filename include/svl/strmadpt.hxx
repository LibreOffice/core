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

#ifndef INCLUDED_SVL_STRMADPT_HXX
#define INCLUDED_SVL_STRMADPT_HXX

#include <svl/svldllapi.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <cppuhelper/weak.hxx>
#include <tools/stream.hxx>


class SVL_DLLPUBLIC SvOutputStreamOpenLockBytes: public SvOpenLockBytes
{
    com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        m_xOutputStream;
    sal_uInt64 m_nPosition;

public:
    TYPEINFO_OVERRIDE();

    SvOutputStreamOpenLockBytes(
            const com::sun::star::uno::Reference<
                      com::sun::star::io::XOutputStream > &
                rTheOutputStream):
        m_xOutputStream(rTheOutputStream), m_nPosition(0) {}

    virtual ErrCode ReadAt(sal_uInt64, void *, sal_uLong, sal_uInt64 *) const SAL_OVERRIDE;

    virtual ErrCode WriteAt(sal_uInt64 nPos, const void * pBuffer, sal_uLong nCount,
                            sal_uInt64 * pWritten) SAL_OVERRIDE;

    virtual ErrCode Flush() const SAL_OVERRIDE;

    virtual ErrCode SetSize(sal_uInt64) SAL_OVERRIDE;

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const SAL_OVERRIDE;

    virtual ErrCode FillAppend(const void * pBuffer, sal_uLong nCount,
                               sal_uLong * pWritten) SAL_OVERRIDE;

    virtual sal_uInt64 Tell() const SAL_OVERRIDE;

    virtual sal_uInt64 Seek(sal_uInt64) SAL_OVERRIDE;

    virtual void Terminate() SAL_OVERRIDE;
};


class SVL_DLLPUBLIC SvLockBytesInputStream: public cppu::OWeakObject,
                              public com::sun::star::io::XInputStream,
                              public com::sun::star::io::XSeekable
{
    SvLockBytesRef m_xLockBytes;
    sal_Int64 m_nPosition;

public:
    SvLockBytesInputStream(SvLockBytes * pTheLockBytes):
        m_xLockBytes(pTheLockBytes), m_nPosition(0) {}

    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface(const com::sun::star::uno::Type & rType)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;

    virtual void SAL_CALL release() throw() SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL
    readBytes(com::sun::star::uno::Sequence< sal_Int8 > & rData,
              sal_Int32 nBytesToRead)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL
    readSomeBytes(com::sun::star::uno::Sequence< sal_Int8 > & rData,
                  sal_Int32 nMaxBytesToRead)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip)
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL available()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL closeInput()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL seek(sal_Int64 nLocation)
        throw (com::sun::star::lang::IllegalArgumentException,
               com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL getPosition()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL getLength()
        throw (com::sun::star::io::IOException,
               com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // INCLUDED_SVL_STRMADPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
