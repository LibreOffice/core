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


#include <vcl/svapp.hxx>

#include <flbytes.hxx>

#ifndef _SVSTDARR_ULONGS_DECL
#define _SVSTDARR_ULONGS
#include <bf_svtools/svstdarr.hxx>
#undef _SVSTDARR_ULONGS
#endif

//============================================================================
namespace binfilter
{

inline ULONG MyMin( long a, long b )
{
    return Max( long( Min(  a ,  b ) ), 0L );
}

//============================================================================
SV_DECL_IMPL_REF_LIST( SvLockBytes, SvLockBytes* )

//============================================================================
//
//  SvSyncLockBytes
//
//============================================================================

TYPEINIT1(SvSyncLockBytes, SvOpenLockBytes);

//============================================================================
// virtual
ErrCode SvSyncLockBytes::ReadAt(ULONG nPos, void * pBuffer, ULONG nCount,
                                ULONG * pRead) const
{
    for (ULONG nReadTotal = 0;;)
    {
        ULONG nReadCount = 0;
        ErrCode nError = m_xAsyncLockBytes->ReadAt(nPos, pBuffer, nCount,
                                                   &nReadCount);
        nReadTotal += nReadCount;
        if (nError != ERRCODE_IO_PENDING || !IsSynchronMode())
        {
            if (pRead)
                *pRead = nReadTotal;
            return nError;
        }
        nPos += nReadCount;
        pBuffer = static_cast< sal_Char * >(pBuffer) + nReadCount;
        nCount -= nReadCount;
        Application::Yield();
    }
}

//============================================================================
// virtual
ErrCode SvSyncLockBytes::WriteAt(ULONG nPos, const void * pBuffer,
                                 ULONG nCount, ULONG * pWritten)
{
    for (ULONG nWrittenTotal = 0;;)
    {
        ULONG nWrittenCount = 0;
        ErrCode nError = m_xAsyncLockBytes->WriteAt(nPos, pBuffer, nCount,
                                                    &nWrittenCount);
        nWrittenTotal += nWrittenCount;
        if (nError != ERRCODE_IO_PENDING || !IsSynchronMode())
        {
            if (pWritten)
                *pWritten = nWrittenTotal;
            return nError;
        }
        nPos += nWrittenCount;
        pBuffer = static_cast< sal_Char const * >(pBuffer) + nWrittenCount;
        nCount -= nWrittenCount;
        Application::Yield();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
