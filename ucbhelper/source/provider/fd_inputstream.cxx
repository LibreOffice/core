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

#include "ucbhelper/fd_inputstream.hxx"

#include <rtl/alloc.h>
#include <algorithm>
#include <stdio.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;

namespace ucbhelper
{
    FdInputStream::FdInputStream(FILE* tmpfl)
        : m_tmpfl(tmpfl ? tmpfl : tmpfile())
    {
        fseek(m_tmpfl,0,SEEK_END);
        long pos = ftell(m_tmpfl);
        rewind(m_tmpfl);
        m_nLength = sal_Int64(pos);
    }



    FdInputStream::~FdInputStream()
    {
        if ( 0 != m_tmpfl)
            fclose(m_tmpfl);
    }


    Any SAL_CALL FdInputStream::queryInterface(
        const Type& rType
    )
        throw(
            RuntimeException
        )
    {
        Any aRet = ::cppu::queryInterface(rType,
                                          (static_cast< XInputStream* >(this)),
                                          (static_cast< XSeekable* >(this)) );

        return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
    }


    void SAL_CALL FdInputStream::acquire( void ) throw() {
        OWeakObject::acquire();
    }

    void SAL_CALL FdInputStream::release( void ) throw() {
        OWeakObject::release();
    }


    sal_Int32 SAL_CALL FdInputStream::readBytes(Sequence< sal_Int8 >& aData,
                                                 sal_Int32 nBytesToRead)
        throw(NotConnectedException,
              BufferSizeExceededException,
              IOException,
              RuntimeException)
    {
        osl::MutexGuard aGuard(m_aMutex);

        if(0 <= nBytesToRead && aData.getLength() < nBytesToRead)
            aData.realloc(nBytesToRead);

        size_t nWanted = static_cast<size_t>(nBytesToRead);
        size_t nRead = fread(aData.getArray(), 1, nWanted, m_tmpfl);
        if (nRead != nWanted && ferror(m_tmpfl))
            throw IOException();

        return static_cast<sal_Int32>(nRead);
    }


    sal_Int32 SAL_CALL FdInputStream::readSomeBytes( Sequence< sal_Int8 >& aData,
                                                      sal_Int32 nMaxBytesToRead )
        throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException)
    {
        return readBytes(aData,nMaxBytesToRead);
    }



    void SAL_CALL FdInputStream::skipBytes(sal_Int32 nBytesToSkip)
        throw(NotConnectedException,
              BufferSizeExceededException,
              IOException,
              RuntimeException)
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

        fseek(m_tmpfl,long(nBytesToSkip),SEEK_CUR);
    }



    sal_Int32 SAL_CALL FdInputStream::available(void)
        throw(NotConnectedException,
              IOException,
              RuntimeException)
    {
        return sal::static_int_cast<sal_Int32>(m_nLength - getPosition());
    }



    void SAL_CALL FdInputStream::closeInput(void)
        throw(NotConnectedException,
              IOException,
              RuntimeException)
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(m_tmpfl)
            fclose(m_tmpfl),m_tmpfl = 0;
    }



    void SAL_CALL FdInputStream::seek(sal_Int64 location)
        throw( IllegalArgumentException,
               IOException,
               RuntimeException )
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

        fseek(m_tmpfl,long(location),SEEK_SET);
    }



    sal_Int64 SAL_CALL
    FdInputStream::getPosition(
        void )
        throw( IOException,
               RuntimeException )
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

    //     fpos_t pos;
    //     fgetpos(m_tmpfl,&pos);
        long pos;
        pos = ftell(m_tmpfl);
        return sal_Int64(pos);
    }



    sal_Int64 SAL_CALL FdInputStream::getLength(
        void
    ) throw(
        IOException,RuntimeException
    )
    {
        return m_nLength;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
