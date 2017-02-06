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

#include <com/sun/star/io/IOException.hpp>
#include <rtl/alloc.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <algorithm>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;

namespace ucbhelper
{
    FdInputStream::FdInputStream( oslFileHandle tmpfl )
        : m_tmpfl(tmpfl)
        , m_nLength( 0 )
    {
        if ( !m_tmpfl )
            osl_createTempFile( nullptr, &m_tmpfl, nullptr );
        OSL_ENSURE( m_tmpfl, "input stream without tempfile!" );

        if ( osl_setFilePos( m_tmpfl, osl_Pos_End, 0 ) == osl_File_E_None )
        {
            sal_uInt64 nFileSize = 0;
            if ( osl_getFilePos( m_tmpfl, &nFileSize ) == osl_File_E_None )
                m_nLength = nFileSize;
            oslFileError rc = osl_setFilePos( m_tmpfl, osl_Pos_Absolut, 0 );
            SAL_WARN_IF(rc != osl_File_E_None, "ucbhelper", "osl_setFilePos failed");
        }
    }

    FdInputStream::~FdInputStream()
    {
        if ( nullptr != m_tmpfl)
            osl_closeFile(m_tmpfl);
    }

    sal_Int32 SAL_CALL FdInputStream::readBytes(Sequence< sal_Int8 >& aData,
                                                 sal_Int32 nBytesToRead)
    {
        osl::MutexGuard aGuard(m_aMutex);

        sal_uInt64 nBeforePos( 0 );
        sal_uInt64 nBytesRequested( nBytesToRead );
        sal_uInt64 nBytesRead( 0 );

        osl_getFilePos( m_tmpfl, &nBeforePos );

        if ( 0 == ( nBytesRequested = std::min< sal_uInt64 >( m_nLength - nBeforePos, nBytesRequested ) ) )
            return 0;

        if ( 0 <= nBytesToRead && aData.getLength() < nBytesToRead )
            aData.realloc( nBytesToRead );

        if ( osl_readFile( m_tmpfl, aData.getArray(), nBytesRequested, &nBytesRead ) != osl_File_E_None )
            throw IOException();

        return sal_Int32( nBytesRead );
    }


    sal_Int32 SAL_CALL FdInputStream::readSomeBytes( Sequence< sal_Int8 >& aData,
                                                      sal_Int32 nMaxBytesToRead )
    {
        return readBytes(aData,nMaxBytesToRead);
    }


    void SAL_CALL FdInputStream::skipBytes(sal_Int32 nBytesToSkip)
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

        oslFileError rc = osl_setFilePos( m_tmpfl, osl_Pos_Current, nBytesToSkip );
        SAL_WARN_IF(rc != osl_File_E_None, "ucbhelper", "osl_setFilePos failed");
    }


    sal_Int32 SAL_CALL FdInputStream::available()
    {
        return sal::static_int_cast<sal_Int32>(m_nLength - getPosition());
    }


    void SAL_CALL FdInputStream::closeInput()
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(m_tmpfl)
        {
            osl_closeFile(m_tmpfl);
            m_tmpfl = nullptr;
        }
    }


    void SAL_CALL FdInputStream::seek(sal_Int64 location)
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

        oslFileError rc = osl_setFilePos( m_tmpfl, osl_Pos_Absolut, location );
        SAL_WARN_IF(rc != osl_File_E_None, "ucbhelper", "osl_setFilePos failed");
    }


    sal_Int64 SAL_CALL
    FdInputStream::getPosition()
    {
        osl::MutexGuard aGuard(m_aMutex);
        if(!m_tmpfl)
            throw IOException();

        sal_uInt64 nFilePos = 0;
        osl_getFilePos( m_tmpfl, &nFilePos );
        return nFilePos;
    }


    sal_Int64 SAL_CALL FdInputStream::getLength()
    {
        return m_nLength;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
