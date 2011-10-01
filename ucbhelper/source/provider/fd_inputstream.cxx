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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

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
                                          SAL_STATIC_CAST( XInputStream*,this ),
                                          SAL_STATIC_CAST( XSeekable*,this ) );

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
