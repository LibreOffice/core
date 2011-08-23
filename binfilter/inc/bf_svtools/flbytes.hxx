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

#ifndef _FLBYTES_HXX
#define _FLBYTES_HXX

#include <tools/debug.hxx>
#include <tools/stream.hxx>

namespace binfilter
{

//============================================================================
class SvSyncLockBytes: public SvOpenLockBytes
{
    SvAsyncLockBytesRef m_xAsyncLockBytes;

public:
    TYPEINFO();

    /// Create a synchronous wrapper around existing asynchronous lock bytes.
    ///
    /// @param pTheAsyncLockBytes  Must not be null.
    inline SvSyncLockBytes(SvAsyncLockBytes * pTheAsyncLockBytes);

    /// Create a synchronous wrapper around an existing stream.
    ///
    /// @descr  This is like first creating asynchronous lock bytes around the
    /// stream and than creating a synchronous wrapper around the asynchronous
    /// lock bytes.
    ///
    /// @param pStream  Must not be null.
    ///
    /// @param bOwner  True if these lock bytes own the stream (delete it on
    /// destruction).
    SvSyncLockBytes(SvStream * pStream, BOOL bOwner):
        m_xAsyncLockBytes(new SvAsyncLockBytes(pStream, bOwner)) {}

    virtual const SvStream * GetStream() const
    { return m_xAsyncLockBytes->GetStream(); }

    virtual void SetSynchronMode(BOOL bSync = TRUE)
    { m_xAsyncLockBytes->SetSynchronMode(bSync); }

    virtual BOOL IsSynchronMode() const
    { return m_xAsyncLockBytes->IsSynchronMode(); }

    virtual ErrCode ReadAt(ULONG nPos, void * pBuffer, ULONG nCount,
                           ULONG * pRead) const;

    virtual ErrCode WriteAt(ULONG nPos, const void * pBuffer, ULONG nCount,
                            ULONG * pWritten);

    virtual ErrCode Flush() const { return m_xAsyncLockBytes->Flush(); }

    virtual ErrCode SetSize(ULONG nSize)
    { return m_xAsyncLockBytes->SetSize(nSize); }

    virtual ErrCode LockRegion(ULONG nPos, ULONG nCount, LockType eType)
    { return m_xAsyncLockBytes->LockRegion(nPos, nCount, eType); }

    virtual ErrCode UnlockRegion(ULONG nPos, ULONG nCount, LockType eType)
    { return m_xAsyncLockBytes->UnlockRegion(nPos, nCount, eType); }

    virtual ErrCode Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag eFlag)
        const
    { return m_xAsyncLockBytes->Stat(pStat, eFlag); }

    virtual ErrCode FillAppend(const void * pBuffer, ULONG nCount,
                               ULONG * pWritten)
    { return m_xAsyncLockBytes->FillAppend(pBuffer, nCount, pWritten); }

    virtual ULONG Tell() const { return m_xAsyncLockBytes->Tell(); }

    virtual ULONG Seek(ULONG nPos)
    { return m_xAsyncLockBytes->Seek(nPos); }

    virtual void Terminate() { m_xAsyncLockBytes->Terminate(); }
};

inline SvSyncLockBytes::SvSyncLockBytes(SvAsyncLockBytes *
                                         pTheAsyncLockBytes):
    m_xAsyncLockBytes(pTheAsyncLockBytes)
{
    DBG_ASSERT(m_xAsyncLockBytes.Is(),
               "SvSyncLockBytes::SvSyncLockBytes(): Null");
}

SV_DECL_IMPL_REF(SvSyncLockBytes);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
