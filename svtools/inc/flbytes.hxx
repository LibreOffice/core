/*************************************************************************
 *
 *  $RCSfile: flbytes.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:50 $
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

#ifndef _FLBYTES_HXX
#define _FLBYTES_HXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

//============================================================================
class SvFillLockBytes : public SvLockBytes
{
    SvLockBytesRef xLockBytes;
    ULONG nFilledSize;
    BOOL bTerminated;
    BOOL bSync;
public:
    TYPEINFO();

    SvFillLockBytes( SvLockBytes* pLockBytes );
    virtual ErrCode ReadAt(
        ULONG nPos, void* pBuffer, ULONG nCount, ULONG* pRead ) const;
    virtual ErrCode WriteAt(
        ULONG nPos, const void* pBuffer, ULONG nCount, ULONG* pWritten );
    virtual ErrCode Flush() const;
    virtual ErrCode SetSize( ULONG nSize );
    virtual ErrCode LockRegion( ULONG nPos, ULONG nCount, LockType );
    virtual ErrCode UnlockRegion( ULONG nPos, ULONG nCount, LockType );
    virtual ErrCode Stat( SvLockBytesStat*, SvLockBytesStatFlag ) const;
    ErrCode FillAppend( const void* pBuffer, ULONG nCount, ULONG *pWritten );
    ULONG   Tell() const { return nFilledSize; }
    void    Seek( ULONG nPos ) { nFilledSize = nPos; }

    void Terminate();
};

SV_DECL_IMPL_REF( SvFillLockBytes )

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

//============================================================================
struct SvCompositeLockBytes_Impl;
class SvCompositeLockBytes : public SvLockBytes
{
    SvCompositeLockBytes_Impl* pImpl;
public:
    TYPEINFO();

    SvCompositeLockBytes( );
    ~SvCompositeLockBytes();

    void Append( SvLockBytes* pLockBytes, ULONG nPos, ULONG nOffset );
    ULONG RelativeOffset( ULONG nPos ) const;
    void  SetIsPending( BOOL bSet );
    SvLockBytes*    GetLastLockBytes() const;

    virtual ErrCode ReadAt(
        ULONG nPos, void* pBuffer, ULONG nCount, ULONG* pRead ) const;
    virtual ErrCode WriteAt(
        ULONG nPos, const void* pBuffer, ULONG nCount, ULONG* pWritten );
    virtual ErrCode Flush() const;
    virtual ErrCode SetSize( ULONG nSize );
    virtual ErrCode LockRegion( ULONG nPos, ULONG nCount, LockType );
    virtual ErrCode UnlockRegion( ULONG nPos, ULONG nCount, LockType );
    virtual ErrCode Stat( SvLockBytesStat*, SvLockBytesStatFlag ) const;
};

SV_DECL_IMPL_REF( SvCompositeLockBytes )


#endif
