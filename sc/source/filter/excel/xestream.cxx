/*************************************************************************
 *
 *  $RCSfile: xestream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-24 11:54:45 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// ============================================================================

#ifndef SC_XESTREAM_HXX
#include "xestream.hxx"
#endif

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif


// ============================================================================

XclExpStream::XclExpStream( SvStream& rOutStrm, const XclExpRoot& rRoot, sal_uInt32 nMaxRecSize ) :
    mrStrm( rOutStrm ),
    mrRoot( rRoot ),
    mnMaxRecSize( nMaxRecSize ),
    mnCurrMaxSize( 0 ),
    mnMaxSliceSize( 0 ),
    mnCalcSize( 0 ),
    mnHeaderSize( 0 ),
    mnCurrSize( 0 ),
    mnSliceSize( 0 ),
    mnLastSizePos( 0 ),
    mbInRec( false )
{
    if( !mnMaxRecSize )
        mnMaxRecSize = (mrRoot.GetBiff() < xlBiff8) ? EXC_MAXRECSIZE_BIFF5 : EXC_MAXRECSIZE_BIFF8;
    mnMaxContSize = nMaxRecSize;
}

XclExpStream::~XclExpStream()
{
    mrStrm.Flush();
}

void XclExpStream::InitRecord( sal_uInt16 nRecId )
{
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mrStrm << nRecId;

    mnLastSizePos = mrStrm.Tell();
    mnHeaderSize = ::std::min( mnCalcSize, mnCurrMaxSize );
    mrStrm << static_cast< sal_uInt16 >( mnHeaderSize );
    mnCurrSize = mnSliceSize = 0;
}

void XclExpStream::UpdateRecSize()
{
    if( mnCurrSize != mnHeaderSize )
    {
        mrStrm.Seek( mnLastSizePos );
        mrStrm << static_cast< sal_uInt16 >( mnCurrSize );
    }
}

void XclExpStream::UpdateSizeVars( sal_uInt32 nSize )
{
    mnCurrSize += nSize;

    if( mnMaxSliceSize )
    {
        mnSliceSize += nSize;
        DBG_ASSERT( mnSliceSize <= mnMaxSliceSize, "XclExpStream::UpdateSizeVars - slice overwritten" );
        if( mnSliceSize >= mnMaxSliceSize )
            mnSliceSize = 0;
    }
}

void XclExpStream::StartContinue()
{
    UpdateRecSize();
    mnCurrMaxSize = mnMaxContSize;
    mnCalcSize -= mnCurrSize;
    InitRecord( EXC_ID_CONT );
}

void XclExpStream::PrepareWrite( sal_uInt32 nSize )
{
    if( mbInRec )
    {
        if( (mnCurrSize + nSize > mnCurrMaxSize) ||
            (mnMaxSliceSize && !mnSliceSize && (mnCurrSize + mnMaxSliceSize > mnCurrMaxSize)) )
            StartContinue();
        UpdateSizeVars( nSize );
    }
}

sal_uInt32 XclExpStream::PrepareWrite()
{
    sal_uInt32 nRet = 0;
    if( mbInRec )
    {
        if( (mnCurrSize >= mnCurrMaxSize) ||
            (mnMaxSliceSize && !mnSliceSize && (mnCurrSize + mnMaxSliceSize > mnCurrMaxSize)) )
            StartContinue();
        UpdateSizeVars( 0 );

        nRet = mnMaxSliceSize ? mnMaxSliceSize - mnSliceSize : mnCurrMaxSize - mnCurrSize;
    }
    return nRet;
}

void XclExpStream::StartRecord( sal_uInt16 nRecId, sal_uInt32 nRecSize )
{
    DBG_ASSERT( !mbInRec, "XclExpStream::StartRecord - another record still open" );
    mnMaxContSize = mnCurrMaxSize = mnMaxRecSize;
    mnCalcSize = nRecSize;
    mbInRec = true;
    InitRecord( nRecId );
    SetSliceSize( 0 );
}

void XclExpStream::EndRecord()
{
    DBG_ASSERT( mbInRec, "XclExpStream::EndRecord - no record open" );
    UpdateRecSize();
    mrStrm.Seek( STREAM_SEEK_TO_END );
    mbInRec = false;
}

void XclExpStream::SetSliceSize( sal_uInt32 nSize )
{
    mnMaxSliceSize = nSize;
    mnSliceSize = 0;
}

sal_uInt32 XclExpStream::Write( const void* pData, sal_uInt32 nBytes )
{
    sal_uInt32 nRet = 0;
    if( pData && nBytes )
    {
        if( mbInRec )
        {
            const sal_uInt8* pBuffer = reinterpret_cast< const sal_uInt8* >( pData );
            sal_uInt32 nBytesLeft = nBytes;
            bool bValid = true;

            while( bValid && nBytesLeft )
            {
                sal_uInt32 nWriteLen = ::std::min( PrepareWrite(), nBytesLeft );
                sal_uInt32 nWriteRet = mrStrm.Write( pBuffer, nWriteLen );
                bValid = (nWriteLen == nWriteRet);
                DBG_ASSERT( bValid, "XclExpStream::Write - stream write error" );
                pBuffer += nWriteRet;
                nRet += nWriteRet;
                nBytesLeft -= nWriteRet;
                UpdateSizeVars( nWriteRet );
            }
        }
        else
            nRet = mrStrm.Write( pData, nBytes );
    }
    return nRet;
}

void XclExpStream::WriteRawZeroBytes( sal_uInt32 nBytes )
{
    const sal_uInt32 nData = 0;
    sal_uInt32 nBytesLeft = nBytes;
    while( nBytesLeft >= sizeof( sal_uInt32 ) )
    {
        mrStrm << nData;
        nBytesLeft -= sizeof( sal_uInt32 );
    }
    if( nBytesLeft )
        mrStrm.Write( &nData, nBytesLeft );
}

void XclExpStream::WriteZeroBytes( sal_uInt32 nBytes )
{
    if( mbInRec )
    {
        sal_uInt32 nBytesLeft = nBytes;
        while( nBytesLeft )
        {
            sal_uInt32 nWriteLen = ::std::min( PrepareWrite(), nBytesLeft );
            WriteRawZeroBytes( nWriteLen );
            nBytesLeft -= nWriteLen;
            UpdateSizeVars( nWriteLen );
        }
    }
    else
        WriteRawZeroBytes( nBytes );
}

sal_uInt32 XclExpStream::CopyFromStream( SvStream& rInStrm, sal_uInt32 nBytes )
{
    sal_uInt32 nStreamPos = rInStrm.Tell();
    sal_uInt32 nStreamSize = rInStrm.Seek( STREAM_SEEK_TO_END );
    rInStrm.Seek( nStreamPos );

    sal_uInt32 nBytesLeft = ::std::min( nBytes, nStreamSize - nStreamPos );
    sal_uInt32 nRet = 0;
    if( nBytesLeft )
    {
        const sal_uInt32 nMaxBuffer = 0x00001000;
        sal_uInt8* pBuffer = new sal_uInt8[ ::std::min( nBytesLeft, nMaxBuffer ) ];
        bool bValid = true;

        while( bValid && nBytesLeft )
        {
            sal_uInt32 nWriteLen = ::std::min( nBytesLeft, nMaxBuffer );
            rInStrm.Read( pBuffer, nWriteLen );
            sal_uInt32 nWriteRet = Write( pBuffer, nWriteLen );
            bValid = (nWriteLen == nWriteRet);
            nRet += nWriteRet;
            nBytesLeft -= nWriteRet;
        }
        delete[] pBuffer;
    }
    return nRet;
}

void XclExpStream::WriteUnicodeBuffer( const sal_uInt16* pBuffer, sal_uInt32 nChars, sal_uInt8 nFlags )
{
    SetSliceSize( 0 );
    if( pBuffer && nChars )
    {
        sal_uInt32 nCharLen = (nFlags & EXC_STRF_16BIT) ? 2 : 1;
        for( sal_uInt32 nIndex = 0; nIndex < nChars; ++nIndex )
        {
            if( mbInRec && (mnCurrSize + nCharLen > mnCurrMaxSize) )
            {
                StartContinue();
                // repeat only 16bit flag
                operator<<( static_cast< sal_uInt8 >( nFlags & EXC_STRF_16BIT ) );
            }
            if( nCharLen == 2 )
                operator<<( pBuffer[ nIndex ] );
            else
                operator<<( static_cast< sal_uInt8 >( pBuffer[ nIndex ] ) );
        }
    }
}

void XclExpStream::WriteUnicodeBuffer( const ScfUInt16Vec& rBuffer, sal_uInt8 nFlags )
{
    SetSliceSize( 0 );
    nFlags &= EXC_STRF_16BIT;   // repeat only 16bit flag
    sal_uInt32 nCharLen = nFlags ? 2 : 1;

    ScfUInt16Vec::const_iterator aEnd = rBuffer.end();
    for( ScfUInt16Vec::const_iterator aIter = rBuffer.begin(); aIter != aEnd; ++aIter )
    {
        if( mbInRec && (mnCurrSize + nCharLen > mnCurrMaxSize) )
        {
            StartContinue();
            operator<<( nFlags );
        }
        if( nCharLen == 2 )
            operator<<( *aIter );
        else
            operator<<( static_cast< sal_uInt8 >( *aIter ) );
    }
}

void XclExpStream::WriteByteStringBuffer( const ByteString& rString, sal_uInt16 nMaxLen )
{
    SetSliceSize( 0 );
    Write( rString.GetBuffer(), ::std::min< xub_StrLen >( rString.Len(), nMaxLen ) );
}

// ER: #71367# Xcl has an obscure sense of whether starting a new record or not,
// and crashes if it encounters the string header at the very end of a record.
// Thus we add 1 to give some room, seems like they do it that way but with another count (10?)
void XclExpStream::WriteByteString( const ByteString& rString, sal_uInt16 nMaxLen, bool b16BitCount )
{
    SetSliceSize( 0 );
    sal_uInt16 nLen = static_cast< sal_uInt16 >( ::std::min< xub_StrLen >( rString.Len(), nMaxLen ) );
    if( !b16BitCount )
        nLen = ::std::min< sal_uInt16 >( nLen, 0xFF );

    sal_uInt32 nLeft = PrepareWrite();
    if( mbInRec && (nLeft <= (b16BitCount ? 2UL : 1UL)) )
        StartContinue();

    if( b16BitCount )
        operator<<( nLen );
    else
        operator<<( static_cast< sal_uInt8 >( nLen ) );
    Write( rString.GetBuffer(), nLen );
}

void XclExpStream::WriteCharBuffer( const ScfUInt8Vec& rBuffer )
{
    SetSliceSize( 0 );
    Write( &rBuffer[ 0 ], rBuffer.size() );
}

sal_uInt32 XclExpStream::SetStreamPos( sal_uInt32 nPos )
{
    DBG_ASSERT( !mbInRec, "XclExpStream::SetStreamPos - not allowed inside of a record" );
    return mbInRec ? 0 : mrStrm.Seek( nPos );
}


// ============================================================================

