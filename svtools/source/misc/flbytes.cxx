/*************************************************************************
 *
 *  $RCSfile: flbytes.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:02 $
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _FLBYTES_HXX
#include <flbytes.hxx>
#endif

#ifndef _SVSTDARR_ULONGS_DECL
#define _SVSTDARR_ULONGS
#include <svstdarr.hxx>
#undef _SVSTDARR_ULONGS
#endif

namespace unnamed_svtools_flbytes {} using namespace unnamed_svtools_flbytes;
    // unnamed namespaces don't work well yet

//============================================================================
namespace unnamed_svtools_flbytes {

inline ULONG MyMin( long a, long b )
{
    return Max( long( Min(  a ,  b ) ), 0L );
}

}

//============================================================================
//
//  SvFillLockBytes
//
//============================================================================

TYPEINIT1(SvFillLockBytes, SvLockBytes);

//============================================================================
SvFillLockBytes::SvFillLockBytes( SvLockBytes* pLockBytes )
    : xLockBytes( pLockBytes ),
      nFilledSize( 0 ),
      bTerminated( FALSE )
{
}

//============================================================================
ErrCode SvFillLockBytes::ReadAt( ULONG nPos, void* pBuffer, ULONG nCount,
                             ULONG *pRead ) const
{
    if( bTerminated )
        return xLockBytes->ReadAt( nPos, pBuffer, nCount, pRead );
    else
    {
        ULONG nWanted = nPos + nCount;
        if( IsSynchronMode() )
        {
            while( nWanted > nFilledSize && !bTerminated )
                Application::Yield();
            return xLockBytes->ReadAt( nPos, pBuffer, nCount, pRead );
        }
        else
        {
            ULONG nRead = MyMin( nCount, long( nFilledSize ) - nPos );
            ULONG nErr = xLockBytes->ReadAt( nPos, pBuffer, nRead, pRead );
            return ( !nCount || nRead == nCount || nErr ) ?
                nErr : ERRCODE_IO_PENDING;
        }
    }
}

//============================================================================
ErrCode SvFillLockBytes::WriteAt( ULONG nPos, const void* pBuffer,
                                  ULONG nCount, ULONG *pWritten )
{
    if( bTerminated )
        return xLockBytes->WriteAt( nPos, pBuffer, nCount, pWritten );
    else
    {
        ULONG nWanted = nPos + nCount;
        if( IsSynchronMode() )
        {
            while( nWanted > nFilledSize && !bTerminated )
                Application::Yield();
            return xLockBytes->WriteAt( nPos, pBuffer, nCount, pWritten );
        }
        else
        {
            ULONG nRead = MyMin( nCount, long( nFilledSize ) - nPos );
            ULONG nErr = xLockBytes->WriteAt( nPos, pBuffer, nRead, pWritten );
            return ( !nCount || nRead == nCount || nErr ) ?
                nErr : ERRCODE_IO_PENDING;
        }
    }
}

//============================================================================
ErrCode SvFillLockBytes::Flush() const
{
    return xLockBytes->Flush( );
}

//============================================================================
ErrCode SvFillLockBytes::SetSize( ULONG nSize )
{
    return xLockBytes->SetSize( nSize );
}

//============================================================================
ErrCode SvFillLockBytes::LockRegion( ULONG nPos, ULONG nCount, LockType eType)
{
    return xLockBytes->LockRegion( nPos, nCount, eType );
}

//============================================================================
ErrCode SvFillLockBytes::UnlockRegion(
    ULONG nPos, ULONG nCount, LockType eType)
{
    return xLockBytes->UnlockRegion( nPos, nCount, eType );
}

//============================================================================
ErrCode SvFillLockBytes::Stat(
    SvLockBytesStat* pStat, SvLockBytesStatFlag eFlag) const
{
    return xLockBytes->Stat( pStat, eFlag );
}

//============================================================================
ErrCode SvFillLockBytes::FillAppend( const void* pBuffer, ULONG nCount, ULONG *pWritten )
{
    ErrCode nRet = xLockBytes->WriteAt(
        nFilledSize, pBuffer, nCount, pWritten );
    nFilledSize += *pWritten;
    return nRet;
}

//============================================================================
void SvFillLockBytes::Terminate()
{
    bTerminated = TRUE;
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

//============================================================================
//
//  SvCompositeLockBytes
//
//============================================================================

struct SvCompositeLockBytes_Impl
{
    SvLockBytesMemberList aLockBytes;
    SvULongs              aPositions;
    SvULongs              aOffsets;
    BOOL                  bPending;
    ULONG RelativeOffset( ULONG nPos ) const;
    ErrCode ReadWrite_Impl(
        ULONG nPos, void* pBuffer, ULONG nCount, ULONG* pProcessed,
        BOOL bRead );
    SvCompositeLockBytes_Impl() : bPending( FALSE ){}
};

//============================================================================
ULONG SvCompositeLockBytes_Impl::RelativeOffset( ULONG nPos ) const
{
    const SvULongs& rPositions = aPositions;
    const SvULongs& rOffsets = aOffsets;

    USHORT nMinPos = 0;
    USHORT nListCount = rPositions.Count();

    // Erster Lockbytes, der bearbeitet werden muss
    while( nMinPos + 1 < nListCount && rPositions[ nMinPos + 1 ] <= nPos )
        nMinPos ++;
    ULONG nSectionStart = rPositions[ nMinPos ];
    if( nSectionStart > nPos )
        return ULONG_MAX;
    return rOffsets[ nMinPos ] + nPos - nSectionStart;
}

//============================================================================
ErrCode SvCompositeLockBytes_Impl::ReadWrite_Impl(
    ULONG nPos, void* pBuffer, ULONG nCount, ULONG* pProcessed,
    BOOL bRead )
{
    ErrCode nErr = ERRCODE_NONE;
    SvULongs& rPositions = aPositions;
    SvULongs& rOffsets = aOffsets;
    SvLockBytesMemberList& rLockBytes = aLockBytes;

    ULONG nBytes = nCount;
    USHORT nListCount = rPositions.Count();
    USHORT nMinPos = 0;

    // Erster Lockbytes, der bearbeitet werden muss
    while( nMinPos + 1 < nListCount && rPositions[ nMinPos + 1 ] <= nPos )
        nMinPos ++;
    ULONG nSectionStart = rPositions[ nMinPos ];

    if( nSectionStart > nPos )
    {
        // Es wird aus fuehrendem Leerbereich gearbeitet
        *pProcessed = 0;
        return ERRCODE_IO_CANTREAD;
    }

    ULONG nDone;
    while( nMinPos < nListCount )
    {
        ULONG nToProcess;
        ULONG nSectionStop;
        if( nMinPos + 1 < nListCount )
        {
            nSectionStop = rPositions[ nMinPos + 1 ];
            nToProcess = MyMin( long( nSectionStop ) - nPos, nBytes );
        }
        else
        {
            nToProcess = nBytes;
            nSectionStop = 0;
        }
        ULONG nAbsPos = nPos - nSectionStart + rOffsets[ nMinPos ];
        SvLockBytes* pLB = rLockBytes.GetObject( nMinPos );
        if( bRead )
            nErr = pLB->ReadAt( nAbsPos, pBuffer, nToProcess, &nDone );
        else
            nErr = pLB->WriteAt( nAbsPos, pBuffer, nToProcess, &nDone );
        nBytes -= nDone;
        if( nErr || nDone < nToProcess || !nBytes )
        {
            *pProcessed = nCount - nBytes;
            // Wenn aus dem letzten LockBytes nichts mehr gelesen wurde und
            // bPending gesetzt ist, Pending zurueck
            if( !nDone && nMinPos == nListCount - 1 )
                return bPending ? ERRCODE_IO_PENDING : nErr;
            else return nErr;
        }
        pBuffer = static_cast< sal_Char * >(pBuffer) + nDone;
        nPos += nDone;
        nSectionStart = nSectionStop;
        nMinPos++;
    }
    return nErr;
}

//============================================================================
TYPEINIT1(SvCompositeLockBytes, SvLockBytes);

//============================================================================
SvCompositeLockBytes::SvCompositeLockBytes()
    : pImpl( new SvCompositeLockBytes_Impl )
{
}

//============================================================================
SvCompositeLockBytes::~SvCompositeLockBytes()
{
    delete pImpl;
}

//============================================================================
void  SvCompositeLockBytes::SetIsPending( BOOL bSet )
{
    pImpl->bPending = bSet;
}

//============================================================================
ULONG SvCompositeLockBytes::RelativeOffset( ULONG nPos ) const
{
    return  pImpl->RelativeOffset( nPos );
}

//============================================================================
ErrCode SvCompositeLockBytes::ReadAt(
    ULONG nPos, void* pBuffer, ULONG nCount, ULONG* pRead ) const
{
    return pImpl->ReadWrite_Impl( nPos, pBuffer, nCount, pRead, TRUE );
}

//============================================================================
ErrCode SvCompositeLockBytes::WriteAt(
    ULONG nPos, const void* pBuffer, ULONG nCount, ULONG* pWritten )
{
    return pImpl->ReadWrite_Impl(
        nPos, const_cast< void * >(pBuffer), nCount, pWritten, FALSE );
}

//============================================================================
ErrCode SvCompositeLockBytes::Flush() const
{
    SvLockBytesMemberList& rLockBytes = pImpl->aLockBytes;
    ErrCode nErr = ERRCODE_NONE;
    for( USHORT nCount = (USHORT)rLockBytes.Count(); !nErr && nCount--; )
        nErr = rLockBytes.GetObject( nCount )->Flush();
    return nErr;
}

//============================================================================
ErrCode SvCompositeLockBytes::SetSize( ULONG nSize )
{
    DBG_ERROR( "not implemented" );
    return ERRCODE_IO_NOTSUPPORTED;
}

//============================================================================
ErrCode SvCompositeLockBytes::LockRegion( ULONG nPos, ULONG nCount, LockType )
{
    DBG_ERROR( "not implemented" );
    return ERRCODE_IO_NOTSUPPORTED;
}

//============================================================================
ErrCode SvCompositeLockBytes::UnlockRegion(
    ULONG nPos, ULONG nCount, LockType )
{
    DBG_ERROR( "not implemented" );
    return ERRCODE_IO_NOTSUPPORTED;
}

//============================================================================
ErrCode SvCompositeLockBytes::Stat(
    SvLockBytesStat* pStat, SvLockBytesStatFlag eFlag) const
{
    USHORT nMax = pImpl->aPositions.Count() - 1;

    SvLockBytesStat aStat;
    ErrCode nErr = pImpl->aLockBytes.GetObject( nMax )->Stat( &aStat, eFlag );
    pStat->nSize = pImpl->aPositions[ nMax ] + aStat.nSize;

    return nErr;
}

//============================================================================
void SvCompositeLockBytes::Append(
    SvLockBytes* pLockBytes, ULONG nPos, ULONG nOffset )
{
    USHORT nCount = pImpl->aOffsets.Count();
    pImpl->aLockBytes.Insert( pLockBytes, nCount );
    pImpl->aPositions.Insert( nPos, nCount );
    pImpl->aOffsets.Insert( nOffset, nCount );
}

//============================================================================
SvLockBytes* SvCompositeLockBytes::GetLastLockBytes() const
{
    return pImpl->aLockBytes.Count() ?
        pImpl->aLockBytes.GetObject( pImpl->aLockBytes.Count() - 1 ) : 0;
}

