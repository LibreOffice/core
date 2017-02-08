/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

// ToDo:
//  - Read->RefreshBuffer->Auf Aenderungen von nBufActualLen reagieren

#include <cstddef>

#include <string.h>
#include <stdio.h>
#include <ctype.h>  // isspace
#include <stdlib.h> // strtol, _crotl

#include "boost/static_assert.hpp"

/*
#if defined( DBG_UTIL ) && (OSL_DEBUG_LEVEL > 1)
// prueft Synchronisation des Buffers nach allen Read, Write, Seek
#define OV_DEBUG
#endif
*/

#include <tools/solar.h>

#if defined(BLC)
#define SWAPNIBBLES(c) c=_crotl(c,4);
#else
#define SWAPNIBBLES(c)      \
unsigned char nSwapTmp=c;   \
nSwapTmp <<= 4;             \
c >>= 4;                    \
c |= nSwapTmp;
#endif

#include <tools/debug.hxx>
#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/stream.hxx>
#include <osl/thread.h>
#include <algorithm>

// -----------------------------------------------------------------------

DBG_NAME( Stream )

// -----------------------------------------------------------------------

// sprintf Param-Mode
#define SPECIAL_PARAM_NONE 0        // Format-Str, Number
#define SPECIAL_PARAM_WIDTH 1       // Format-Str, Width, Number
#define SPECIAL_PARAM_PRECISION 2   // Format-Str, Precision, Number
#define SPECIAL_PARAM_BOTH 3        // Format-Str, Width, Precision, Number

// -----------------------------------------------------------------------

// !!! Nicht inline, wenn Operatoren <<,>> inline sind
inline static void SwapUShort( sal_uInt16& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapShort( short& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapLong( long& r )
    {   r = SWAPLONG(r);   }
inline static void SwapULong( sal_uInt32& r )
    {   r = SWAPLONG(r);   }
inline static void SwapLongInt( int& r )
    {   r = SWAPLONG(r);   }
inline static void SwapLongUInt( unsigned int& r )
    {   r = SWAPLONG(r);   }
#ifdef UNX
inline static void SwapFloat( float& r )
    {
          union
          {
              float f;
              sal_uInt32 c;
          } s;

          s.f = r;
          s.c = SWAPLONG( s.c );
          r = s.f;
    }
inline static void SwapDouble( double& r )
    {
        if( sizeof(double) != 8 )
        {
          DBG_ASSERT( sal_False, "Can only swap 8-Byte-doubles\n" );
        }
        else
        {
          union
          {
              double d;
              sal_uInt32 c[2];
          } s;

          s.d = r;
          s.c[0] ^= s.c[1]; // zwei 32-Bit-Werte in situ vertauschen
          s.c[1] ^= s.c[0];
          s.c[0] ^= s.c[1];
          s.c[0] = SWAPLONG(s.c[0]); // und die beiden 32-Bit-Werte selbst in situ drehen
          s.c[1] = SWAPLONG(s.c[1]);
          r = s.d;
        }
    }
#endif

//SDO

#define READNUMBER_WITHOUT_SWAP(datatype,value) \
{\
int tmp = eIOMode; \
if( (tmp == STREAM_IO_READ) && sizeof(datatype)<=nBufFree) \
{\
    for (std::size_t i = 0; i < sizeof(datatype); i++)\
        ((char *)&value)[i] = pBufPos[i];\
    nBufActualPos += sizeof(datatype);\
    pBufPos += sizeof(datatype);\
    nBufFree -= sizeof(datatype);\
}\
else\
    Read( (char*)&value, sizeof(datatype) );\
}

#define WRITENUMBER_WITHOUT_SWAP(datatype,value) \
{\
int tmp = eIOMode; \
if( (tmp==STREAM_IO_WRITE) && sizeof(datatype) <= nBufFree)\
{\
    for (std::size_t i = 0; i < sizeof(datatype); i++)\
        pBufPos[i] = ((char *)&value)[i];\
    nBufFree -= sizeof(datatype);\
    nBufActualPos += sizeof(datatype);\
    if( nBufActualPos > nBufActualLen )\
        nBufActualLen = nBufActualPos;\
    pBufPos += sizeof(datatype);\
    bIsDirty = sal_True;\
}\
else\
    Write( (char*)&value, sizeof(datatype) );\
}

//============================================================================
//
//  class SvLockBytes
//
//============================================================================

void SvLockBytes::close()
{
    if (m_bOwner)
        delete m_pStream;
    m_pStream = 0;
}

//============================================================================
TYPEINIT0(SvLockBytes);

//============================================================================
// virtual
ErrCode SvLockBytes::ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                            sal_Size * pRead) const
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::ReadAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    sal_Size nTheRead = m_pStream->Read(pBuffer, nCount);
    if (pRead)
        *pRead = nTheRead;
    return m_pStream->GetErrorCode();
}

//============================================================================
// virtual
ErrCode SvLockBytes::WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                             sal_Size * pWritten)
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::WriteAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    sal_Size nTheWritten = m_pStream->Write(pBuffer, nCount);
    if (pWritten)
        *pWritten = nTheWritten;
    return m_pStream->GetErrorCode();
}

//============================================================================
// virtual
ErrCode SvLockBytes::Flush() const
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::Flush(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Flush();
    return m_pStream->GetErrorCode();
}

//============================================================================
// virtual
ErrCode SvLockBytes::SetSize(sal_Size nSize)
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::SetSize(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->SetStreamSize(nSize);
    return m_pStream->GetErrorCode();
}

//============================================================================
ErrCode SvLockBytes::LockRegion(sal_Size, sal_Size, LockType)
{
    DBG_ERROR("SvLockBytes::LockRegion(): Not implemented");
    return ERRCODE_NONE;
}

//============================================================================

ErrCode SvLockBytes::UnlockRegion(sal_Size, sal_Size, LockType)
{
    DBG_ERROR("SvLockBytes::UnlockRegion(): Not implemented");
    return ERRCODE_NONE;
}

//============================================================================
ErrCode SvLockBytes::Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::Stat(): Bad stream");
        return ERRCODE_NONE;
    }

    if (pStat)
    {
        sal_Size nPos = m_pStream->Tell();
        pStat->nSize = m_pStream->Seek(STREAM_SEEK_TO_END);
        m_pStream->Seek(nPos);
    }
    return ERRCODE_NONE;
}

//============================================================================
//
//  class SvOpenLockBytes
//
//============================================================================

TYPEINIT1(SvOpenLockBytes, SvLockBytes);

//============================================================================
//
//  class SvAsyncLockBytes
//
//============================================================================

TYPEINIT1(SvAsyncLockBytes, SvOpenLockBytes);

//============================================================================
// virtual
ErrCode SvAsyncLockBytes::ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                                 sal_Size * pRead) const
{
    if (m_bTerminated)
        return SvOpenLockBytes::ReadAt(nPos, pBuffer, nCount, pRead);
    else
    {
        sal_Size nTheCount = std::min(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
        ErrCode nError = SvOpenLockBytes::ReadAt(nPos, pBuffer, nTheCount,
                                                 pRead);
        return !nCount || nTheCount == nCount || nError ? nError :
                                                          ERRCODE_IO_PENDING;
    }
}

//============================================================================
// virtual
ErrCode SvAsyncLockBytes::WriteAt(sal_Size nPos, const void * pBuffer,
                                  sal_Size nCount, sal_Size * pWritten)
{
    if (m_bTerminated)
        return SvOpenLockBytes::WriteAt(nPos, pBuffer, nCount, pWritten);
    else
    {
        sal_Size nTheCount = std::min(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
        ErrCode nError = SvOpenLockBytes::WriteAt(nPos, pBuffer, nTheCount,
                                                  pWritten);
        return !nCount || nTheCount == nCount || nError ? nError :
                                                          ERRCODE_IO_PENDING;
    }
}

//============================================================================
// virtual
ErrCode SvAsyncLockBytes::FillAppend(const void * pBuffer, sal_Size nCount,
                                     sal_Size * pWritten)
{
    sal_Size nTheWritten;
    ErrCode nError = SvOpenLockBytes::WriteAt(m_nSize, pBuffer, nCount,
                                              &nTheWritten);
    if (!nError)
        m_nSize += nTheWritten;
    if (pWritten)
        *pWritten = nTheWritten;
    return nError;
}

//============================================================================
// virtual
sal_Size SvAsyncLockBytes::Seek(sal_Size nPos)
{
    if (nPos != STREAM_SEEK_TO_END)
        m_nSize = nPos;
    return m_nSize;
}

//============================================================================
//
//  class SvStream
//
//============================================================================

sal_Size SvStream::GetData( void* pData, sal_Size nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        sal_Size nRet;
        nError = xLockBytes->ReadAt( nActPos, pData, nSize, &nRet );
        nActPos += nRet;
        return nRet;
    }
    else return 0;
}

ErrCode SvStream::SetLockBytes( SvLockBytesRef& rLB )
{
    xLockBytes = rLB;
    RefreshBuffer();
    return ERRCODE_NONE;
}

//========================================================================

sal_Size SvStream::PutData( const void* pData, sal_Size nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        sal_Size nRet;
        nError = xLockBytes->WriteAt( nActPos, pData, nSize, &nRet );
        nActPos += nRet;
        return nRet;
    }
    else return 0;
}

//========================================================================

sal_Size SvStream::SeekPos( sal_Size nPos )
{
    if( !GetError() && nPos == STREAM_SEEK_TO_END )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        SvLockBytesStat aStat;
        xLockBytes->Stat( &aStat, SVSTATFLAG_DEFAULT );
        nActPos = aStat.nSize;
    }
    else
        nActPos = nPos;
    return nActPos;
}

//========================================================================

void SvStream::FlushData()
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        nError = xLockBytes->Flush();
    }
}

//========================================================================

void SvStream::SetSize( sal_Size nSize )
{
    DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
    nError = xLockBytes->SetSize( nSize );
}

void SvStream::ImpInit()
{
    nActPos             = 0;
    nCompressMode       = COMPRESSMODE_NONE;
    eStreamCharSet      = osl_getThreadTextEncoding();
//  eTargetCharSet      = osl_getThreadTextEncoding();
    nCryptMask          = 0;
    bIsEof              = sal_False;
#if defined UNX
    eLineDelimiter      = LINEEND_LF;   // UNIX-Format
#else
    eLineDelimiter      = LINEEND_CRLF; // DOS-Format
#endif

    SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    nBufFilePos         = 0;
    nBufActualPos       = 0;
    bIsDirty            = sal_False;
    bIsConsistent       = sal_True;
    bIsWritable         = sal_True;

    pRWBuf              = 0;
    pBufPos             = 0;
    nBufSize            = 0;
    nBufActualLen       = 0;
    eIOMode             = STREAM_IO_DONTKNOW;
    nBufFree            = 0;

    nRadix              = 10;
    nPrecision          = 0;  // all significant digits
    nWidth              = 0; // default width
    cFiller             = ' ';
    nJustification      = JUSTIFY_RIGHT;
    eStreamMode         = 0;
    CreateFormatString();

    nVersion           = 0;

    ClearError();
}

/*************************************************************************
|*
|*    Stream::Stream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvStream::SvStream( SvLockBytes* pLockBytesP )
{
    DBG_CTOR( Stream, NULL );

    ImpInit();
    xLockBytes = pLockBytesP;
    const SvStream* pStrm;
    if( pLockBytesP ) {
        pStrm = pLockBytesP->GetStream();
        if( pStrm ) {
            SetError( pStrm->GetErrorCode() );
        }
    }
    SetBufferSize( 256 );
}

SvStream::SvStream()
{
    DBG_CTOR( Stream, NULL );

    ImpInit();
}

/*************************************************************************
|*
|*    Stream::~Stream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvStream::~SvStream()
{
    DBG_DTOR( Stream, NULL );

    if ( xLockBytes.Is() )
        Flush();

    if( pRWBuf )
        delete[] pRWBuf;
}

/*************************************************************************
|*
|*    Stream::IsA()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_uInt16 SvStream::IsA() const
{
    return (sal_uInt16)ID_STREAM;
}

/*************************************************************************
|*
|*    Stream::ClearError()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::ClearError()
{
    bIsEof = sal_False;
    nError = SVSTREAM_OK;
}

/*************************************************************************
|*
|*    Stream::SetError()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::SetError( sal_uInt32 nErrorCode )
{
    if ( nError == SVSTREAM_OK )
        nError = nErrorCode;
}


/*************************************************************************
|*
|*    Stream::SetNumberFormatInt()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::SetNumberFormatInt( sal_uInt16 nNewFormat )
{
    nNumberFormatInt = nNewFormat;
    bSwap = sal_False;
#ifdef OSL_BIGENDIAN
    if( nNumberFormatInt == NUMBERFORMAT_INT_LITTLEENDIAN )
        bSwap = sal_True;
#else
    if( nNumberFormatInt == NUMBERFORMAT_INT_BIGENDIAN )
        bSwap = sal_True;
#endif
}

/*************************************************************************
|*
|*    Stream::SetBufferSize()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::SetBufferSize( sal_uInt16 nBufferSize )
{
    sal_Size nActualFilePos = Tell();
    sal_Bool bDontSeek = (sal_Bool)(pRWBuf == 0);

    if( bIsDirty && bIsConsistent && bIsWritable )  // wg. Windows NT: Access denied
        Flush();

    if( nBufSize )
    {
        delete[] pRWBuf;
        nBufFilePos += nBufActualPos;
    }

    pRWBuf          = 0;
    nBufActualLen   = 0;
    nBufActualPos   = 0;
    nBufSize        = nBufferSize;
    if( nBufSize )
        pRWBuf = new sal_uInt8[ nBufSize ];
    bIsConsistent   = sal_True;
    pBufPos         = pRWBuf;
    eIOMode = STREAM_IO_DONTKNOW;
    if( !bDontSeek )
        SeekPos( nActualFilePos );
}

/*************************************************************************
|*
|*    Stream::ClearBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::ClearBuffer()
{
    nBufActualLen   = 0;
    nBufActualPos   = 0;
    nBufFilePos     = 0;
    pBufPos         = pRWBuf;
    bIsDirty        = sal_False;
    bIsConsistent   = sal_True;
    eIOMode         = STREAM_IO_DONTKNOW;

    bIsEof          = sal_False;
}

/*************************************************************************
|*
|*    Stream::ResetError()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::ResetError()
{
    ClearError();
}

/*************************************************************************
|*
|*    Stream::ReadLine()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Bool SvStream::ReadByteStringLine( String& rStr, rtl_TextEncoding eSrcCharSet )
{
    sal_Bool bRet;
    ByteString aStr;

    bRet = ReadLine(aStr);
    rStr = UniString( aStr, eSrcCharSet );
    return bRet;
}

sal_Bool SvStream::ReadLine( ByteString& rStr )
{
    sal_Char    buf[256+1];
    sal_Bool        bEnd        = sal_False;
    sal_Size       nOldFilePos = Tell();
    sal_Char    c           = 0;
    sal_Size       nTotalLen   = 0;

    rStr.Erase();
    while( !bEnd && !GetError() )   // !!! nicht auf EOF testen,
                                    // !!! weil wir blockweise
                                    // !!! lesen
    {
        sal_uInt16 nLen = (sal_uInt16)Read( buf, sizeof(buf)-1 );
        if ( !nLen )
        {
            if ( rStr.Len() == 0 )
            {
                // der allererste Blockread hat fehlgeschlagen -> Abflug
                bIsEof = sal_True;
                return sal_False;
            }
            else
                break;
        }

        sal_uInt16 j, n;
        for( j = n = 0; j < nLen ; ++j )
        {
            c = buf[j];
            if ( c == '\n' || c == '\r' )
            {
                bEnd = sal_True;
                break;
            }
            // erAck 26.02.01: Old behavior was no special treatment of '\0'
            // character here, but a following rStr+=c did ignore it. Is this
            // really intended? Or should a '\0' better terminate a line?
            // The nOldFilePos stuff wasn't correct then anyways.
            if ( c )
            {
                if ( n < j )
                    buf[n] = c;
                ++n;
            }
        }
        if ( n )
            rStr.Append( buf, n );
        nTotalLen += j;
    }

    if ( !bEnd && !GetError() && rStr.Len() )
        bEnd = sal_True;

    nOldFilePos += nTotalLen;
    if( Tell() > nOldFilePos )
        nOldFilePos++;
    Seek( nOldFilePos );  // seeken wg. obigem BlockRead!

    if ( bEnd && (c=='\r' || c=='\n') )  // Sonderbehandlung DOS-Dateien
    {
        char cTemp;
        sal_Size nLen = Read((char*)&cTemp , sizeof(cTemp) );
        if ( nLen ) {
            if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
                Seek( nOldFilePos );
        }
    }

    if ( bEnd )
        bIsEof = sal_False;
    return bEnd;
}

sal_Bool SvStream::ReadUniStringLine( String& rStr )
{
    sal_Unicode buf[256+1];
    sal_Bool        bEnd        = sal_False;
    sal_Size       nOldFilePos = Tell();
    sal_Unicode c           = 0;
    sal_Size       nTotalLen   = 0;

    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "ReadUniStringLine: swapping sizeof(sal_Unicode) not implemented" );

    rStr.Erase();
    while( !bEnd && !GetError() )   // !!! nicht auf EOF testen,
                                    // !!! weil wir blockweise
                                    // !!! lesen
    {
        sal_uInt16 nLen = (sal_uInt16)Read( (char*)buf, sizeof(buf)-sizeof(sal_Unicode) );
        nLen /= sizeof(sal_Unicode);
        if ( !nLen )
        {
            if ( rStr.Len() == 0 )
            {
                // der allererste Blockread hat fehlgeschlagen -> Abflug
                bIsEof = sal_True;
                return sal_False;
            }
            else
                break;
        }

        sal_uInt16 j, n;
        for( j = n = 0; j < nLen ; ++j )
        {
            if ( bSwap )
                SwapUShort( buf[n] );
            c = buf[j];
            if ( c == '\n' || c == '\r' )
            {
                bEnd = sal_True;
                break;
            }
            // erAck 26.02.01: Old behavior was no special treatment of '\0'
            // character here, but a following rStr+=c did ignore it. Is this
            // really intended? Or should a '\0' better terminate a line?
            // The nOldFilePos stuff wasn't correct then anyways.
            if ( c )
            {
                if ( n < j )
                    buf[n] = c;
                ++n;
            }
        }
        if ( n )
            rStr.Append( buf, n );
        nTotalLen += j;
    }

    if ( !bEnd && !GetError() && rStr.Len() )
        bEnd = sal_True;

    nOldFilePos += nTotalLen * sizeof(sal_Unicode);
    if( Tell() > nOldFilePos )
        nOldFilePos += sizeof(sal_Unicode);
    Seek( nOldFilePos );  // seeken wg. obigem BlockRead!

    if ( bEnd && (c=='\r' || c=='\n') )  // Sonderbehandlung DOS-Dateien
    {
        sal_Unicode cTemp;
        Read( (char*)&cTemp, sizeof(cTemp) );
        if ( bSwap )
            SwapUShort( cTemp );
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            Seek( nOldFilePos );
    }

    if ( bEnd )
        bIsEof = sal_False;
    return bEnd;
}

sal_Bool SvStream::ReadUniOrByteStringLine( String& rStr, rtl_TextEncoding eSrcCharSet )
{
    if ( eSrcCharSet == RTL_TEXTENCODING_UNICODE )
        return ReadUniStringLine( rStr );
    else
        return ReadByteStringLine( rStr, eSrcCharSet );
}

/*************************************************************************
|*
|*    Stream::ReadCString
|*
*************************************************************************/

sal_Bool SvStream::ReadCString( ByteString& rStr )
{
    if( rStr.Len() )
        rStr.Erase();

    sal_Char buf[ 256 + 1 ];
    sal_Bool bEnd = sal_False;
    sal_Size nFilePos = Tell();

    while( !bEnd && !GetError() )
    {
        sal_uInt16 nLen = (sal_uInt16)Read( buf, sizeof(buf)-1 );
        sal_uInt16 nReallyRead = nLen;
        if( !nLen )
            break;

        const sal_Char* pPtr = buf;
        while( *pPtr && nLen )
            ++pPtr, --nLen;

        bEnd =  ( nReallyRead < sizeof(buf)-1 )         // read less than attempted to read
                ||  (  ( nLen > 0 )                    // OR it is inside the block we read
                    &&  ( 0 == *pPtr )                  //    AND found a string terminator
                    );

        rStr.Append( buf, ::sal::static_int_cast< xub_StrLen >( pPtr - buf ) );
    }

    nFilePos += rStr.Len();
    if( Tell() > nFilePos )
        nFilePos++;
    Seek( nFilePos );  // seeken wg. obigem BlockRead!
    return bEnd;
}

sal_Bool SvStream::ReadCString( String& rStr, rtl_TextEncoding eToEncode )
{
    ByteString sStr;
    sal_Bool bRet = ReadCString( sStr );
    rStr = String( sStr, eToEncode );
    return bRet;
}


/*************************************************************************
|*
|*    Stream::WriteUnicodeText()
|*
*************************************************************************/

sal_Bool SvStream::WriteUnicodeText( const String& rStr )
{
    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "WriteUnicodeText: swapping sizeof(sal_Unicode) not implemented" );
    if ( bSwap )
    {
        xub_StrLen nLen = rStr.Len();
        sal_Unicode aBuf[384];
        sal_Unicode* const pTmp = ( nLen > 384 ? new sal_Unicode[nLen] : aBuf);
        memcpy( pTmp, rStr.GetBuffer(), nLen * sizeof(sal_Unicode) );
        sal_Unicode* p = pTmp;
        const sal_Unicode* const pStop = pTmp + nLen;
        while ( p < pStop )
        {
            SwapUShort( *p );
            p++;
        }
        Write( (char*)pTmp, nLen * sizeof(sal_Unicode) );
        if ( pTmp != aBuf )
            delete [] pTmp;
    }
    else
        Write( (char*)rStr.GetBuffer(), rStr.Len() * sizeof(sal_Unicode) );
    return nError == SVSTREAM_OK;
}

sal_Bool SvStream::WriteUnicodeOrByteText( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        return WriteUnicodeText( rStr );
    else
    {
        ByteString aStr( rStr, eDestCharSet );
        Write( aStr.GetBuffer(), aStr.Len() );
        return nError == SVSTREAM_OK;
    }
}

/*************************************************************************
|*
|*    Stream::WriteLine()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Bool SvStream::WriteByteStringLine( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLine( ByteString( rStr, eDestCharSet ) );
}

sal_Bool SvStream::WriteLine( const ByteString& rStr )
{
    Write( rStr.GetBuffer(), rStr.Len() );
    endl(*this);
    return nError == SVSTREAM_OK;
}

sal_Bool SvStream::WriteUniStringLine( const String& rStr )
{
    WriteUnicodeText( rStr );
    endlu(*this);
    return nError == SVSTREAM_OK;
}

sal_Bool SvStream::WriteUniOrByteStringLine( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        return WriteUniStringLine( rStr );
    else
        return WriteByteStringLine( rStr, eDestCharSet );
}

/*************************************************************************
|*
|*    Stream::WriteLines()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 17.07.95
|*    Letzte Aenderung  OV 17.07.95
|*
*************************************************************************/

sal_Bool SvStream::WriteByteStringLines( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLines( ByteString( rStr, eDestCharSet ) );
}

sal_Bool SvStream::WriteLines( const ByteString& rStr )
{
    ByteString aStr( rStr );
    aStr.ConvertLineEnd( eLineDelimiter );
    Write( aStr.GetBuffer(), aStr.Len() );
    endl( *this );
    return (sal_Bool)(nError == SVSTREAM_OK);
}

sal_Bool SvStream::WriteUniStringLines( const String& rStr )
{
    String aStr( rStr );
    aStr.ConvertLineEnd( eLineDelimiter );
    WriteUniStringLine( aStr );
    return nError == SVSTREAM_OK;
}

sal_Bool SvStream::WriteUniOrByteStringLines( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        return WriteUniStringLines( rStr );
    else
        return WriteByteStringLines( rStr, eDestCharSet );
}

/*************************************************************************
|*
|*    Stream::WriteUniOrByteChar()
|*
*************************************************************************/

sal_Bool SvStream::WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        *this << ch;
    else
    {
        ByteString aStr( ch, eDestCharSet );
        Write( aStr.GetBuffer(), aStr.Len() );
    }
    return nError == SVSTREAM_OK;
}

/*************************************************************************
|*
|*    Stream::StartWritingUnicodeText()
|*
*************************************************************************/

sal_Bool SvStream::StartWritingUnicodeText()
{
    SetEndianSwap( sal_False );     // write native format
    // BOM, Byte Order Mark, U+FEFF, see
    // http://www.unicode.org/faq/utf_bom.html#BOM
    // Upon read: 0xfeff(-257) => no swap; 0xfffe(-2) => swap
    *this << sal_uInt16( 0xfeff );
    return nError == SVSTREAM_OK;
}

/*************************************************************************
|*
|*    Stream::StartReadingUnicodeText()
|*
*************************************************************************/

sal_Bool SvStream::StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet )
{
    if (!(  eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
            eReadBomCharSet == RTL_TEXTENCODING_UNICODE ||
            eReadBomCharSet == RTL_TEXTENCODING_UTF8))
        return sal_True;    // nothing to read

    bool bTryUtf8 = false;
    sal_uInt16 nFlag;
    sal_sSize nBack = sizeof(nFlag);
    *this >> nFlag;
    switch ( nFlag )
    {
        case 0xfeff :
            // native UTF-16
            if (    eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                    eReadBomCharSet == RTL_TEXTENCODING_UNICODE)
                nBack = 0;
        break;
        case 0xfffe :
            // swapped UTF-16
            if (    eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                    eReadBomCharSet == RTL_TEXTENCODING_UNICODE)
            {
                SetEndianSwap( !bSwap );
                nBack = 0;
            }
        break;
        case 0xefbb :
            if (nNumberFormatInt == NUMBERFORMAT_INT_BIGENDIAN &&
                    (eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                     eReadBomCharSet == RTL_TEXTENCODING_UTF8))
                bTryUtf8 = true;
        break;
        case 0xbbef :
            if (nNumberFormatInt == NUMBERFORMAT_INT_LITTLEENDIAN &&
                    (eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                     eReadBomCharSet == RTL_TEXTENCODING_UTF8))
                bTryUtf8 = true;
        break;
        default:
            ;   // nothing
    }
    if (bTryUtf8)
    {
        sal_uChar nChar;
        nBack += sizeof(nChar);
        *this >> nChar;
        if (nChar == 0xbf)
            nBack = 0;      // it is UTF-8
    }
    if (nBack)
        SeekRel( -nBack );      // no BOM, pure data
    return nError == SVSTREAM_OK;
}

/*************************************************************************
|*
|*    Stream::ReadCsvLine()
|*
*************************************************************************/

// Precondition: pStr is guaranteed to be non-NULL and points to a 0-terminated
// array.
inline const sal_Unicode* lcl_UnicodeStrChr( const sal_Unicode* pStr,
        sal_Unicode c )
{
    while (*pStr)
    {
        if (*pStr == c)
            return pStr;
        ++pStr;
    }
    return 0;
}

sal_Bool SvStream::ReadCsvLine( String& rStr, sal_Bool bEmbeddedLineBreak,
        const String& rFieldSeparators, sal_Unicode cFieldQuote,
        sal_Bool bAllowBackslashEscape)
{
    ReadUniOrByteStringLine( rStr);

    if (bEmbeddedLineBreak)
    {
        const sal_Unicode* pSeps = rFieldSeparators.GetBuffer();
        xub_StrLen nLastOffset = 0;
        bool isQuoted = false;
        bool isFieldStarting = true;
        while (!IsEof() && rStr.Len() < STRING_MAXLEN)
        {
            bool wasQuote = false;
            bool bBackslashEscaped = false;
            const sal_Unicode *p;
            p = rStr.GetBuffer();
            p += nLastOffset;
            while (*p)
            {
                if (isQuoted)
                {
                    if (*p == cFieldQuote && !bBackslashEscaped)
                        wasQuote = !wasQuote;
                    else
                    {
                        if (bAllowBackslashEscape)
                        {
                            if (*p == '\\')
                                bBackslashEscaped = !bBackslashEscaped;
                            else
                                bBackslashEscaped = false;
                        }
                        if (wasQuote)
                        {
                            wasQuote = false;
                            isQuoted = false;
                            if (lcl_UnicodeStrChr( pSeps, *p ))
                                isFieldStarting = true;
                        }
                    }
                }
                else
                {
                    if (isFieldStarting)
                    {
                        isFieldStarting = false;
                        if (*p == cFieldQuote)
                            isQuoted = true;
                        else if (lcl_UnicodeStrChr( pSeps, *p ))
                            isFieldStarting = true;
                    }
                    else if (lcl_UnicodeStrChr( pSeps, *p ))
                        isFieldStarting = true;
                }
                ++p;
            }

            if (wasQuote)
                isQuoted = false;

            if (isQuoted)
            {
                nLastOffset = rStr.Len();
                String aNext;
                ReadUniOrByteStringLine( aNext);
                rStr += sal_Unicode(_LF);
                rStr += aNext;
            }
            else
                break;
        }
    }
    return nError == SVSTREAM_OK;
}

/*************************************************************************
|*
|*    Stream::SeekRel()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Size SvStream::SeekRel( sal_sSize nPos )
{
    sal_Size nActualPos = Tell();

    if ( nPos >= 0 )
    {
        if ( SAL_MAX_SIZE - nActualPos > (sal_Size)nPos )
            nActualPos += nPos;
    }
    else
    {
        sal_Size nAbsPos = (sal_Size)-nPos;
        if ( nActualPos >= nAbsPos )
            nActualPos -= nAbsPos;
    }

    pBufPos = pRWBuf + nActualPos;
    return Seek( nActualPos );
}

/*************************************************************************
|*
|*    Stream::operator>>()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvStream& SvStream::operator >> ( sal_uInt16& r )
{
    READNUMBER_WITHOUT_SWAP(sal_uInt16,r)
    if( bSwap )
        SwapUShort(r);
    return *this;
}

SvStream& SvStream::operator>> ( sal_uInt32& r )
{
    READNUMBER_WITHOUT_SWAP(sal_uInt32,r)
    if( bSwap )
        SwapULong(r);
    return *this;
}

SvStream& SvStream::operator >> ( long& r )
{
#if(SAL_TYPES_SIZEOFLONG != 4)
    int tmp = r;
    *this >> tmp;
    r = tmp;
#else
    READNUMBER_WITHOUT_SWAP(long,r)
    if( bSwap )
        SwapLong(r);
#endif
    return *this;
}

SvStream& SvStream::operator >> ( short& r )
{
    READNUMBER_WITHOUT_SWAP(short,r)
    if( bSwap )
        SwapShort(r);
    return *this;
}

SvStream& SvStream::operator >> ( int& r )
{
    READNUMBER_WITHOUT_SWAP(int,r)
    if( bSwap )
        SwapLongInt(r);
    return *this;
}

SvStream& SvStream::operator>>( signed char& r )
{
    if( (eIOMode == STREAM_IO_READ || !bIsConsistent) &&
        sizeof(signed char) <= nBufFree )
    {
        r = *pBufPos;
        nBufActualPos += sizeof(signed char);
        pBufPos += sizeof(signed char);
        nBufFree -= sizeof(signed char);
    }
    else
        Read( (char*)&r, sizeof(signed char) );
    return *this;
}

// Sonderbehandlung fuer Chars wegen PutBack

SvStream& SvStream::operator>>( char& r )
{
    if( (eIOMode == STREAM_IO_READ || !bIsConsistent) &&
        sizeof(char) <= nBufFree )
    {
        r = *pBufPos;
        nBufActualPos += sizeof(char);
        pBufPos += sizeof(char);
        nBufFree -= sizeof(char);
    }
    else
        Read( (char*)&r, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator>>( unsigned char& r )
{
    if( (eIOMode == STREAM_IO_READ || !bIsConsistent) &&
        sizeof(char) <= nBufFree )
    {
        r = *pBufPos;
        nBufActualPos += sizeof(char);
        pBufPos += sizeof(char);
        nBufFree -= sizeof(char);
    }
    else
        Read( (char*)&r, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator>>( float& r )
{
    // Read( (char*)&r, sizeof(float) );
    READNUMBER_WITHOUT_SWAP(float,r)
#if defined UNX
    if( bSwap )
      SwapFloat(r);
#endif
    return *this;
}

SvStream& SvStream::operator>>( double& r )
{
    // Read( (char*)&r, sizeof(double) );
    READNUMBER_WITHOUT_SWAP(double,r)
#if defined UNX
    if( bSwap )
      SwapDouble(r);
#endif
    return *this;
}

SvStream& SvStream::operator>> ( SvStream& rStream )
{
    const sal_uInt32 cBufLen = 0x8000;
    char* pBuf = new char[ cBufLen ];

    sal_uInt32 nCount;
    do {
        nCount = Read( pBuf, cBufLen );
        rStream.Write( pBuf, nCount );
    } while( nCount == cBufLen );

    delete[] pBuf;
    return *this;
}

/*************************************************************************
|*
|*    Stream::operator<<()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvStream& SvStream::operator<< ( sal_uInt16 v )
{
    if( bSwap )
        SwapUShort(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt16,v)
    return *this;
}

SvStream& SvStream::operator<<  ( sal_uInt32 v )
{
    if( bSwap )
        SwapULong(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt32,v)
    return *this;
}

SvStream& SvStream::operator<< ( long v )
{
#if(SAL_TYPES_SIZEOFLONG != 4)
    int tmp = v;
    *this << tmp;
#else
    if( bSwap )
        SwapLong(v);
    WRITENUMBER_WITHOUT_SWAP(long,v)
#endif
    return *this;
}

SvStream& SvStream::operator<<  ( short v )
{
    if( bSwap )
        SwapShort(v);
    WRITENUMBER_WITHOUT_SWAP(short,v)
    return *this;
}

SvStream& SvStream::operator<<( int v )
{
    if( bSwap )
        SwapLongInt( v );
    WRITENUMBER_WITHOUT_SWAP(int,v)
    return *this;
}

SvStream& SvStream::operator<<  ( signed char v )
{
    //SDO
    int tmp = eIOMode;
    if(tmp == STREAM_IO_WRITE && sizeof(signed char) <= nBufFree )
    {
        *pBufPos = v;
        pBufPos++; // sizeof(char);
        nBufActualPos++;
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--; // = sizeof(char);
        bIsDirty = sal_True;
    }
    else
        Write( (char*)&v, sizeof(signed char) );
    return *this;
}

// Sonderbehandlung fuer chars wegen PutBack

SvStream& SvStream::operator<<  ( char v )
{
    //SDO
    int tmp = eIOMode;
    if(tmp == STREAM_IO_WRITE && sizeof(char) <= nBufFree )
    {
        *pBufPos = v;
        pBufPos++; // sizeof(char);
        nBufActualPos++;
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--; // = sizeof(char);
        bIsDirty = sal_True;
    }
    else
        Write( (char*)&v, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator<<  ( unsigned char v )
{
//SDO
    int tmp = eIOMode;
    if(tmp == STREAM_IO_WRITE && sizeof(char) <= nBufFree )
    {
        *(unsigned char*)pBufPos = v;
        pBufPos++; // = sizeof(char);
        nBufActualPos++; // = sizeof(char);
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--;
        bIsDirty = sal_True;
    }
    else
        Write( (char*)&v, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator<< ( float v )
{
#ifdef UNX
    if( bSwap )
      SwapFloat(v);
#endif
    WRITENUMBER_WITHOUT_SWAP(float,v)
    return *this;
}

SvStream& SvStream::operator<< ( const double& r )
{
//    Write( (char*)&r, sizeof( double ) );
#if defined UNX
    if( bSwap )
    {
      double nHelp = r;
      SwapDouble(nHelp);
      WRITENUMBER_WITHOUT_SWAP(double,nHelp)
      return *this;
    }
    else
#endif
    WRITENUMBER_WITHOUT_SWAP(double,r)

    return *this;
}

SvStream& SvStream::operator<<  ( const char* pBuf )
{
    Write( pBuf, strlen( pBuf ) );
    return *this;
}

SvStream& SvStream::operator<<  ( const unsigned char* pBuf )
{
    Write( (char*)pBuf, strlen( (char*)pBuf ) );
    return *this;
}

SvStream& SvStream::operator<< ( SvStream& rStream )
{
    const sal_uInt32 cBufLen = 0x8000;
    char* pBuf = new char[ cBufLen ];
    sal_uInt32 nCount;
    do {
        nCount = rStream.Read( pBuf, cBufLen );
        Write( pBuf, nCount );
    } while( nCount == cBufLen );

    delete[] pBuf;
    return *this;
}

// -----------------------------------------------------------------------

SvStream& SvStream::ReadByteString( UniString& rStr, rtl_TextEncoding eSrcCharSet )
{
    // read UTF-16 string directly from stream ?
    if (eSrcCharSet == RTL_TEXTENCODING_UNICODE)
    {
        sal_uInt32 nLen;
        operator>> (nLen);
        if (nLen)
        {
            if (nLen > STRING_MAXLEN) {
                SetError(SVSTREAM_GENERALERROR);
                return *this;
            }
            sal_Unicode *pStr = rStr.AllocBuffer(
                static_cast< xub_StrLen >(nLen));
            BOOST_STATIC_ASSERT(STRING_MAXLEN <= SAL_MAX_SIZE / 2);
            Read( pStr, nLen << 1 );

            if (bSwap)
                for (sal_Unicode *pEnd = pStr + nLen; pStr < pEnd; pStr++)
                    SwapUShort(*pStr);
        }
        else
            rStr.Erase();

        return *this;
    }

    ByteString aStr;
    ReadByteString( aStr );
    rStr = UniString( aStr, eSrcCharSet );
    return *this;
}

// -----------------------------------------------------------------------

SvStream& SvStream::ReadByteString( ByteString& rStr )
{
    sal_uInt16 nLen = 0;
    operator>>( nLen );
    if( nLen )
    {
        char* pTmp = rStr.AllocBuffer( nLen );
        nLen = (sal_uInt16)Read( pTmp, nLen );
    }
    else
        rStr.Erase();
    return *this;
}

// -----------------------------------------------------------------------

SvStream& SvStream::WriteByteString( const UniString& rStr, rtl_TextEncoding eDestCharSet )
{
    // write UTF-16 string directly into stream ?
    if (eDestCharSet == RTL_TEXTENCODING_UNICODE)
    {
        sal_uInt32 nLen = rStr.Len();
        operator<< (nLen);
        if (nLen)
        {
            if (bSwap)
            {
                const sal_Unicode *pStr = rStr.GetBuffer();
                const sal_Unicode *pEnd = pStr + nLen;

                for (; pStr < pEnd; pStr++)
                {
                    sal_Unicode c = *pStr;
                    SwapUShort(c);
                    WRITENUMBER_WITHOUT_SWAP(sal_uInt16,c)
                }
            }
            else
                Write( rStr.GetBuffer(), nLen << 1 );
        }

        return *this;
    }

    return WriteByteString(ByteString( rStr, eDestCharSet ));
}

// -----------------------------------------------------------------------

SvStream& SvStream::WriteByteString( const ByteString& rStr)
{
    sal_uInt16 nLen = rStr.Len();
    operator<< ( nLen );
    if( nLen != 0 )
        Write( rStr.GetBuffer(), nLen );
    return *this;
}

/*************************************************************************
|*
|*    Stream::Read()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Size SvStream::Read( void* pData, sal_Size nCount )
{
    sal_Size nSaveCount = nCount;
    if( !bIsConsistent )
        RefreshBuffer();

    if( !pRWBuf )
    {
        nCount = GetData( (char*)pData,nCount);
        if( nCryptMask )
            EncryptBuffer(pData, nCount);
        nBufFilePos += nCount;
    }
    else
    {
        // ist Block komplett im Puffer
        eIOMode = STREAM_IO_READ;
        if( nCount <= (sal_Size)(nBufActualLen - nBufActualPos ) )
        {
            // Ja!
            memcpy(pData, pBufPos, (size_t) nCount);
            nBufActualPos = nBufActualPos + (sal_uInt16)nCount;
            pBufPos += nCount;
            nBufFree = nBufFree - (sal_uInt16)nCount;
        }
        else
        {
            if( bIsDirty ) // Flushen ?
            {
                SeekPos( nBufFilePos );
                if( nCryptMask )
                    CryptAndWriteBuffer(pRWBuf, nBufActualLen);
                else
                    PutData( pRWBuf, nBufActualLen );
                bIsDirty = sal_False;
            }

            // passt der Datenblock in den Puffer ?
            if( nCount > nBufSize )
            {
                // Nein! Deshalb ohne Umweg ueber den Puffer direkt
                // in den Zielbereich einlesen

                eIOMode = STREAM_IO_DONTKNOW;

                SeekPos( nBufFilePos + nBufActualPos );
                nBufActualLen = 0;
                pBufPos       = pRWBuf;
                nCount = GetData( (char*)pData, nCount );
                if( nCryptMask )
                    EncryptBuffer(pData, nCount);
                nBufFilePos += nCount;
                nBufFilePos += nBufActualPos;
                nBufActualPos = 0;
            }
            else
            {
                // Der Datenblock passt komplett in den Puffer. Deshalb
                // Puffer fuellen und dann die angeforderten Daten in den
                // Zielbereich kopieren.

                nBufFilePos += nBufActualPos;
                SeekPos( nBufFilePos );

                // TODO: Typecast vor GetData, sal_uInt16 nCountTmp
                sal_Size nCountTmp = GetData( pRWBuf, nBufSize );
                if( nCryptMask )
                    EncryptBuffer(pRWBuf, nCountTmp);
                nBufActualLen = (sal_uInt16)nCountTmp;
                if( nCount > nCountTmp )
                {
                    nCount = nCountTmp;  // zurueckstutzen, Eof siehe unten
                }
                memcpy( pData, pRWBuf, (size_t)nCount );
                nBufActualPos = (sal_uInt16)nCount;
                pBufPos = pRWBuf + nCount;
            }
        }
    }
    bIsEof = sal_False;
    nBufFree = nBufActualLen - nBufActualPos;
    if( nCount != nSaveCount && nError != ERRCODE_IO_PENDING )
        bIsEof = sal_True;
    if( nCount == nSaveCount && nError == ERRCODE_IO_PENDING )
        nError = ERRCODE_NONE;
    return nCount;
}

/*************************************************************************
|*
|*    Stream::Write()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Size SvStream::Write( const void* pData, sal_Size nCount )
{
    if( !nCount )
        return 0;
    if( !bIsWritable )
    {
        SetError( ERRCODE_IO_CANTWRITE );
        return 0;
    }
    if( !bIsConsistent )
        RefreshBuffer();   // Aenderungen des Puffers durch PutBack loeschen

    if( !pRWBuf )
    {
        if( nCryptMask )
            nCount = CryptAndWriteBuffer( pData, nCount );
        else
            nCount = PutData( (char*)pData, nCount );
        nBufFilePos += nCount;
        return nCount;
    }

    eIOMode = STREAM_IO_WRITE;
    if( nCount <= (sal_Size)(nBufSize - nBufActualPos) )
    {
        memcpy( pBufPos, pData, (size_t)nCount );
        nBufActualPos = nBufActualPos + (sal_uInt16)nCount;
        // wurde der Puffer erweitert ?
        if( nBufActualPos > nBufActualLen )
            nBufActualLen = nBufActualPos;

        pBufPos += nCount;
        bIsDirty = sal_True;
    }
    else
    {
        // Flushen ?
        if( bIsDirty )
        {
            SeekPos( nBufFilePos );
            if( nCryptMask )
                CryptAndWriteBuffer( pRWBuf, (sal_Size)nBufActualLen );
            else
                PutData( pRWBuf, nBufActualLen );
            bIsDirty = sal_False;
        }

        // passt der Block in den Puffer ?
        if( nCount > nBufSize )
        {
            eIOMode = STREAM_IO_DONTKNOW;
            nBufFilePos += nBufActualPos;
            nBufActualLen = 0;
            nBufActualPos = 0;
            pBufPos       = pRWBuf;
            SeekPos( nBufFilePos );
            if( nCryptMask )
                nCount = CryptAndWriteBuffer( pData, nCount );
            else
                nCount = PutData( (char*)pData, nCount );
            nBufFilePos += nCount;
        }
        else
        {
            // Block in Puffer stellen
            memcpy( pRWBuf, pData, (size_t)nCount );

            // Reihenfolge!
            nBufFilePos += nBufActualPos;
            nBufActualPos = (sal_uInt16)nCount;
            pBufPos = pRWBuf + nCount;
            nBufActualLen = (sal_uInt16)nCount;
            bIsDirty = sal_True;
        }
    }
    nBufFree = nBufSize - nBufActualPos;
    return nCount;
}


/*************************************************************************
|*
|*    Stream::Seek()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Size SvStream::Seek( sal_Size nFilePos )
{
    eIOMode = STREAM_IO_DONTKNOW;

    bIsEof = sal_False;
    if( !pRWBuf )
    {
        nBufFilePos = SeekPos( nFilePos );
        DBG_ASSERT(Tell()==nBufFilePos,"Out Of Sync!");
        return nBufFilePos;
    }

    // Ist Position im Puffer ?
    if( nFilePos >= nBufFilePos && nFilePos <= (nBufFilePos + nBufActualLen))
    {
        nBufActualPos = (sal_uInt16)(nFilePos - nBufFilePos);
        pBufPos = pRWBuf + nBufActualPos;
        // nBufFree korrigieren, damit wir nicht von einem
        // PutBack (ignoriert den StreamMode) getoetet werden
        nBufFree = nBufActualLen - nBufActualPos;
    }
    else
    {
        if( bIsDirty && bIsConsistent)
        {
            SeekPos( nBufFilePos );
            if( nCryptMask )
                CryptAndWriteBuffer( pRWBuf, nBufActualLen );
            else
                PutData( pRWBuf, nBufActualLen );
            bIsDirty = sal_False;
        }
        nBufActualLen = 0;
        nBufActualPos = 0;
        pBufPos       = pRWBuf;
        nBufFilePos = SeekPos( nFilePos );
    }
#ifdef OV_DEBUG
    {
        sal_Size nDebugTemp = nBufFilePos + nBufActualPos;
        DBG_ASSERT(Tell()==nDebugTemp,"Sync?");
    }
#endif
    return nBufFilePos + nBufActualPos;
}

/*************************************************************************
|*
|*    Stream::Flush()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::Flush()
{
    if( bIsDirty && bIsConsistent )
    {
        SeekPos( nBufFilePos );
        if( nCryptMask )
            CryptAndWriteBuffer( pRWBuf, (sal_Size)nBufActualLen );
        else
            if( PutData( pRWBuf, nBufActualLen ) != nBufActualLen )
                SetError( SVSTREAM_WRITE_ERROR );
        bIsDirty = sal_False;
    }
    if( bIsWritable )
        FlushData();
}


/*************************************************************************
|*
|*    Stream::PutBack()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 01.08.94
|*    Letzte Aenderung  OV 01.08.94
|*
*************************************************************************/

/*
    4 Faelle :

    1. Datenzeiger steht mitten im Puffer (nBufActualPos >= 1)
    2. Datenzeiger auf Position 0, Puffer ist voll
    3. Datenzeiger auf Position 0, Puffer ist teilweise gefuellt
    4. Datenzeiger auf Position 0, Puffer ist leer -> Fehler!
*/

SvStream& SvStream::PutBack( char aCh )
{
    // wenn kein Buffer oder Zurueckscrollen nicht moeglich -> Fehler
    if( !pRWBuf || !nBufActualLen || ( !nBufActualPos && !nBufFilePos ) )
    {
        // 4. Fall
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }

    // Flush() (Phys. Flushen aber nicht notwendig, deshalb selbst schreiben)
    if( bIsConsistent && bIsDirty  )
    {
        SeekPos( nBufFilePos );
        if( nCryptMask )
            CryptAndWriteBuffer( pRWBuf, nBufActualLen );
        else
            PutData( pRWBuf, nBufActualLen );
        bIsDirty = sal_False;
    }
    bIsConsistent = sal_False;  // Puffer enthaelt jetzt TRASH
    if( nBufActualPos )
    {
        // 1. Fall
        nBufActualPos--;
        pBufPos--;
        *pBufPos = aCh;
        nBufFree++;
    }
    else  // Puffer muss verschoben werden
    {
        // Ist Puffer am Anschlag ?
        if( nBufSize == nBufActualLen )
        {
            // 2. Fall
            memmove( pRWBuf+1, pRWBuf, nBufSize-1 );
            // nBufFree behaelt den Wert!
        }
        else
        {
            // 3. Fall -> Puffer vergroessern
            memmove( pRWBuf+1, pRWBuf, (sal_uInt16)nBufActualLen );
            nBufActualLen++;
            nBufFree++;
        }
        nBufFilePos--;
        *pRWBuf = aCh;
    }
    eIOMode = STREAM_IO_DONTKNOW;
    bIsEof = sal_False;
    return *this;
}

/*************************************************************************
|*
|*    Stream::EatWhite()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 01.08.94
|*    Letzte Aenderung  OV 01.08.94
|*
*************************************************************************/

void SvStream::EatWhite()
{
    char aCh;
    Read(&aCh, sizeof(char) );
    while( !bIsEof && isspace((int)aCh) )  //( aCh == ' ' || aCh == '\t' ) )
        Read(&aCh, sizeof(char) );
    if( !bIsEof ) // konnte das letzte Char gelesen werden ?
        SeekRel( -1L );
}

/*************************************************************************
|*
|*    Stream::RefreshBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 01.08.94
|*    Letzte Aenderung  OV 01.08.94
|*
*************************************************************************/

void SvStream::RefreshBuffer()
{
    if( bIsDirty && bIsConsistent )
    {
        SeekPos( nBufFilePos );
        if( nCryptMask )
            CryptAndWriteBuffer( pRWBuf, (sal_Size)nBufActualLen );
        else
            PutData( pRWBuf, nBufActualLen );
        bIsDirty = sal_False;
    }
    SeekPos( nBufFilePos );
    nBufActualLen = (sal_uInt16)GetData( pRWBuf, nBufSize );
    if( nBufActualLen && nError == ERRCODE_IO_PENDING )
        nError = ERRCODE_NONE;
    if( nCryptMask )
        EncryptBuffer(pRWBuf, (sal_Size)nBufActualLen);
    bIsConsistent = sal_True;
    eIOMode = STREAM_IO_DONTKNOW;
}


/*************************************************************************
|*
|*    Stream::CreateFormatString()
|*
|*    Beschreibung      Baut Formatstring zusammen
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::CreateFormatString()
{
    aFormatString = '%';
    nPrintfParams = SPECIAL_PARAM_NONE;

    if( nJustification )
    {
        aFormatString += '-';
    }

    if( nWidth )
    {
        if( cFiller != ' ' )
            aFormatString += '0';
        aFormatString += '*';
        nPrintfParams = SPECIAL_PARAM_WIDTH;
    }

    if( nPrecision )
    {
        aFormatString += ".*";
        if( nWidth )
            nPrintfParams = SPECIAL_PARAM_BOTH;
        else
            nPrintfParams = SPECIAL_PARAM_PRECISION;
    }
}

/*************************************************************************
|*
|*    Stream::ReadNumber()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

#define BUFSIZE_LONG 21  // log( 2 hoch 64 ) + 1

SvStream& SvStream::ReadNumber( long& rLong )
{
    EatWhite();
    if( bIsEof || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    sal_Size nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    sal_Size nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rLong = strtol( buf, &pEndPtr, (int)nRadix );
    nFPtr += ( (sal_Size)pEndPtr - (sal_Size)(&(buf[0])) );
    Seek( nFPtr );
    bIsEof = sal_False;
    return *this;
}

SvStream& SvStream::ReadNumber( sal_uInt32& rUInt32 )
{
    EatWhite();
    if( bIsEof || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    sal_Size nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    sal_Size nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rUInt32 = strtoul( buf, &pEndPtr, (int)nRadix );
    nFPtr += ( (sal_uIntPtr)pEndPtr - (sal_uIntPtr)buf );
    Seek( nFPtr );
    bIsEof = sal_False;
    return *this;
}

SvStream& SvStream::ReadNumber( double& rDouble )
{
    EatWhite();
    if( bIsEof || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    sal_Size nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    sal_Size nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rDouble = strtod( buf, &pEndPtr );
    nFPtr += ( (sal_Size)pEndPtr - (sal_Size)buf );
    Seek( nFPtr );
    bIsEof = sal_False;
    return *this;
}


/*************************************************************************
|*
|*    Stream::WriteNumber()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvStream& SvStream::WriteNumber( long nLong )
{
    char buffer[256+12];
    char pType[] = "ld"; // Nicht static!
    if( nRadix == 16 )
        pType[1] = 'x';
    else if( nRadix == 8 )
        pType[1] = 'o';
    ByteString aFStr( aFormatString);
    aFStr += pType;
    int nLen;
    switch ( nPrintfParams )
    {
        case SPECIAL_PARAM_NONE :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nLong );
            break;
        case SPECIAL_PARAM_WIDTH :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nWidth, nLong );
            break;
        case SPECIAL_PARAM_PRECISION :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nPrecision,nLong);
            break;
        default:
            nLen=sprintf(buffer, aFStr.GetBuffer(),nWidth,nPrecision,nLong);
    }
    Write( buffer, (long)nLen );
    return *this;
}

SvStream& SvStream::WriteNumber( sal_uInt32 nUInt32 )
{
    char buffer[256+12];
    char pType[] = "lu"; // Nicht static!
    if( nRadix == 16 )
        pType[1] = 'x';
    else if( nRadix == 8 )
        pType[1] = 'o';
    ByteString aFStr( aFormatString);
    aFStr += pType;
    int nLen;
    switch ( nPrintfParams )
    {
        case SPECIAL_PARAM_NONE :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nUInt32 );
            break;
        case SPECIAL_PARAM_WIDTH :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nWidth, nUInt32 );
            break;
        case SPECIAL_PARAM_PRECISION :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nPrecision, nUInt32 );
            break;
        default:
            nLen=sprintf(buffer,aFStr.GetBuffer(),nWidth,nPrecision,nUInt32 );
    }
    Write( buffer, (long)nLen );
    return *this;
}


SvStream& SvStream::WriteNumber( const double& rDouble )
{
    char buffer[256+24];
    ByteString aFStr( aFormatString);
    aFStr += "lf";
    int nLen;
    switch ( nPrintfParams )
    {
        case SPECIAL_PARAM_NONE :
            nLen = sprintf( buffer, aFStr.GetBuffer(), rDouble );
            break;
        case SPECIAL_PARAM_WIDTH :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nWidth, rDouble );
            break;
        case SPECIAL_PARAM_PRECISION :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nPrecision, rDouble);
            break;
        default:
            nLen=sprintf(buffer, aFStr.GetBuffer(),nWidth,nPrecision,rDouble);
    }
    Write( buffer, (long)nLen );
    return *this;
}

/*************************************************************************
|*
|*    Stream::CryptAndWriteBuffer()
|*
|*    Beschreibung      Verschluesseln und Schreiben
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

#define CRYPT_BUFSIZE 1024

sal_Size SvStream::CryptAndWriteBuffer( const void* pStart, sal_Size nLen)
{
    unsigned char  pTemp[CRYPT_BUFSIZE];
    unsigned char* pDataPtr = (unsigned char*)pStart;
    sal_Size nCount = 0;
    sal_Size nBufCount;
    unsigned char nMask = nCryptMask;
    do
    {
        if( nLen >= CRYPT_BUFSIZE )
            nBufCount = CRYPT_BUFSIZE;
        else
            nBufCount = nLen;
        nLen -= nBufCount;
        memcpy( pTemp, pDataPtr, (sal_uInt16)nBufCount );
        // **** Verschluesseln *****
        for ( sal_uInt16 n=0; n < CRYPT_BUFSIZE; n++ )
        {
            unsigned char aCh = pTemp[n];
            aCh ^= nMask;
            SWAPNIBBLES(aCh)
            pTemp[n] = aCh;
        }
        // *************************
        nCount += PutData( (char*)pTemp, nBufCount );
        pDataPtr += nBufCount;
    }
    while ( nLen );
    return nCount;
}

/*************************************************************************
|*
|*    Stream::EncryptBuffer()
|*
|*    Beschreibung      Buffer entschluesseln
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Bool SvStream::EncryptBuffer(void* pStart, sal_Size nLen)
{
    unsigned char* pTemp = (unsigned char*)pStart;
    unsigned char nMask = nCryptMask;

    for ( sal_Size n=0; n < nLen; n++, pTemp++ )
    {
        unsigned char aCh = *pTemp;
        SWAPNIBBLES(aCh)
        aCh ^= nMask;
        *pTemp = aCh;
    }
    return sal_True;
}

/*************************************************************************
|*
|*    Stream::SetKey()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

unsigned char implGetCryptMask(const sal_Char* pStr, sal_Int32 nLen, long nVersion)
{
    unsigned char nCryptMask = 0;

    if (!nLen)
        return nCryptMask;

    if( nVersion <= SOFFICE_FILEFORMAT_31 )
    {
        while( nLen )
        {
            nCryptMask ^= *pStr;
            pStr++;
            nLen--;
        }
    }
    else // BugFix #25888#
    {
        for( sal_uInt16 i = 0; i < nLen; i++ ) {
            nCryptMask ^= pStr[i];
            if( nCryptMask & 0x80 ) {
                nCryptMask <<= 1;
                nCryptMask++;
            }
            else
                nCryptMask <<= 1;
        }
    }

    if( !nCryptMask )
        nCryptMask = 67;

    return nCryptMask;
}

void SvStream::SetKey( const ByteString& rKey )
{
    aKey = rKey;
    nCryptMask = implGetCryptMask( aKey.GetBuffer(), aKey.Len(), GetVersion() );
}

/*************************************************************************
|*
|*    Stream::SyncSvStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::SyncSvStream( sal_Size nNewStreamPos )
{
    ClearBuffer();
    SvStream::nBufFilePos = nNewStreamPos;
}

/*************************************************************************
|*
|*    Stream::SyncSysStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

void SvStream::SyncSysStream()
{
    Flush();
    SeekPos( Tell() );
}

/*************************************************************************
|*
|*    Stream::SetStreamSize()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

sal_Bool SvStream::SetStreamSize( sal_Size nSize )
{
#ifdef DBG_UTIL
    sal_Size nFPos = Tell();
#endif
    sal_uInt16 nBuf = nBufSize;
    SetBufferSize( 0 );
    SetSize( nSize );
    SetBufferSize( nBuf );
    DBG_ASSERT(Tell()==nFPos,"SetStreamSize failed");
    return (sal_Bool)(nError == 0);
}

//============================================================================

void SvStream::AddMark( sal_Size )
{
}

//============================================================================

void SvStream::RemoveMark( sal_Size )
{
}

/*************************************************************************
|*
|*    endl()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  TH 13.11.96
|*
*************************************************************************/

SvStream& endl( SvStream& rStr )
{
    LineEnd eDelim = rStr.GetLineDelimiter();
    if ( eDelim == LINEEND_CR )
        rStr << _CR;
    else if( eDelim == LINEEND_LF )
        rStr << _LF;
    else
        rStr << _CR << _LF;
    return rStr;
}

SvStream& endlu( SvStream& rStrm )
{
    switch ( rStrm.GetLineDelimiter() )
    {
        case LINEEND_CR :
            rStrm << sal_Unicode(_CR);
        break;
        case LINEEND_LF :
            rStrm << sal_Unicode(_LF);
        break;
        default:
            rStrm << sal_Unicode(_CR) << sal_Unicode(_LF);
    }
    return rStrm;
}

SvStream& endlub( SvStream& rStrm )
{
    if ( rStrm.GetStreamCharSet() == RTL_TEXTENCODING_UNICODE )
        return endlu( rStrm );
    else
        return endl( rStrm );
}

/*************************************************************************
|*
|*    SvMemoryStream::SvMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

SvMemoryStream::SvMemoryStream( void* pBuffer, sal_Size bufSize,
                                StreamMode eMode )
{
    if( eMode & STREAM_WRITE )
        bIsWritable = sal_True;
    else
        bIsWritable = sal_False;
    nEndOfData  = bufSize;
    bOwnsData   = sal_False;
    pBuf        = (sal_uInt8 *) pBuffer;
    nResize     = 0L;
    nSize       = bufSize;
    nPos        = 0L;
    SetBufferSize( 0 );
}

/*************************************************************************
|*
|*    SvMemoryStream::SvMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

SvMemoryStream::SvMemoryStream( sal_Size nInitSize, sal_Size nResizeOffset )
{
    bIsWritable = sal_True;
    bOwnsData   = sal_True;
    nEndOfData  = 0L;
    nResize     = nResizeOffset;
    nPos        = 0;
    pBuf        = 0;
    if( nResize != 0 && nResize < 16 )
        nResize = 16;
    if( nInitSize && !AllocateMemory( nInitSize ) )
    {
        SetError( SVSTREAM_OUTOFMEMORY );
        nSize = 0;
    }
    else
        nSize = nInitSize;
    SetBufferSize( 64 );
}

/*************************************************************************
|*
|*    SvMemoryStream::~SvMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

SvMemoryStream::~SvMemoryStream()
{
    if( pBuf )
    {
        if( bOwnsData )
            FreeMemory();
        else
            Flush();
    }
}

/*************************************************************************
|*
|*    SvMemoryStream::IsA()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

sal_uInt16 SvMemoryStream::IsA() const
{
    return (sal_uInt16)ID_MEMORYSTREAM;
}

/*************************************************************************
|*
|*    SvMemoryStream::SetBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

void* SvMemoryStream::SetBuffer( void* pNewBuf, sal_Size nCount,
                                 sal_Bool bOwnsDat, sal_Size nEOF )
{
    void* pResult;
    SetBufferSize( 0 ); // Buffering in der Basisklasse initialisieren
    Seek( 0 );
    if( bOwnsData )
    {
        pResult = 0;
        if( pNewBuf != pBuf )
            FreeMemory();
    }
    else
        pResult = pBuf;

    pBuf        = (sal_uInt8 *) pNewBuf;
    nPos        = 0;
    nSize       = nCount;
    nResize     = 0;
    bOwnsData   = bOwnsDat;

    if( nEOF > nCount )
        nEOF = nCount;
    nEndOfData = nEOF;

    ResetError();

    DBG_ASSERT( nEndOfData<STREAM_SEEK_TO_END,"Invalid EOF");
    return pResult;
}

/*************************************************************************
|*
|*    SvMemoryStream::GetData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

sal_Size SvMemoryStream::GetData( void* pData, sal_Size nCount )
{
    sal_Size nMaxCount = nEndOfData-nPos;
    if( nCount > nMaxCount )
        nCount = nMaxCount;
    memcpy( pData, pBuf+nPos, (size_t)nCount );
    nPos += nCount;
    return nCount;
}

/*************************************************************************
|*
|*    SvMemoryStream::PutData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

sal_Size SvMemoryStream::PutData( const void* pData, sal_Size nCount )
{
    if( GetError() )
        return 0L;

    sal_Size nMaxCount = nSize-nPos;

    // auf Ueberlauf testen
    if( nCount > nMaxCount )
    {
        if( nResize == 0 )
        {
            // soviel wie moeglich rueberschaufeln
            nCount = nMaxCount;
            SetError( SVSTREAM_OUTOFMEMORY );
        }
        else
        {
            long nNewResize;
            if( nSize && nSize > nResize )
                nNewResize = nSize;
            else
                nNewResize = nResize;

            if( (nCount-nMaxCount) < nResize )
            {
                // fehlender Speicher ist kleiner als Resize-Offset,
                // deshalb um Resize-Offset vergroessern
                if( !ReAllocateMemory( nNewResize) )
                {
                    nCount = 0;
                    SetError( SVSTREAM_WRITE_ERROR );
                }
            }
            else
            {
                // fehlender Speicher ist groesser als Resize-Offset
                // deshalb um Differenz+ResizeOffset vergroessern
                if( !ReAllocateMemory( nCount-nMaxCount+nNewResize ) )
                {
                    nCount = 0;
                    SetError( SVSTREAM_WRITE_ERROR );
                }
            }
        }
    }
    DBG_ASSERT(pBuf,"Possibly Reallocate failed");
    memcpy( pBuf+nPos, pData, (size_t)nCount);

    nPos += nCount;
    if( nPos > nEndOfData )
        nEndOfData = nPos;
    return nCount;
}

/*************************************************************************
|*
|*    SvMemoryStream::SeekPos()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

// nEndOfData: Erste Position im Stream, die nicht gelesen werden darf
// nSize: Groesse des allozierten Speichers

sal_Size SvMemoryStream::SeekPos( sal_Size nNewPos )
{
    if( nNewPos < nEndOfData )
        nPos = nNewPos;
    else if( nNewPos == STREAM_SEEK_TO_END )
        nPos = nEndOfData;
    else
    {
        if( nNewPos >= nSize ) // muss Buffer vergroessert werden ?
        {
            if( nResize )  // ist vergroeseern erlaubt ?
            {
                long nDiff = (long)(nNewPos - nSize + 1);
                nDiff += (long)nResize;
                ReAllocateMemory( nDiff );
                nPos = nNewPos;
                nEndOfData = nNewPos;
            }
            else  // vergroessern ist nicht erlaubt -> ans Ende setzen
            {
                // SetError( SVSTREAM_OUTOFMEMORY );
                nPos = nEndOfData;
            }
        }
        else  // gueltigen Bereich innerhalb des Buffers vergroessern
        {
            nPos = nNewPos;
            nEndOfData = nNewPos;
        }
    }
    return nPos;
}

/*************************************************************************
|*
|*    SvMemoryStream::FlushData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

void SvMemoryStream::FlushData()
{
}

/*************************************************************************
|*
|*    SvMemoryStream::ResetError()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

void SvMemoryStream::ResetError()
{
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvMemoryStream::AllocateMemory()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

sal_Bool SvMemoryStream::AllocateMemory( sal_Size nNewSize )
{
    pBuf = new sal_uInt8[nNewSize];
    return( pBuf != 0 );
}

/*************************************************************************
|*
|*    SvMemoryStream::ReAllocateMemory()   (Bozo-Algorithmus)
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

sal_Bool SvMemoryStream::ReAllocateMemory( long nDiff )
{
    sal_Bool bRetVal    = sal_False;
    long nTemp      = (long)nSize;
    nTemp           += nDiff;
    sal_Size nNewSize  = (sal_Size)nTemp;

    if( nNewSize )
    {
        sal_uInt8* pNewBuf   = new sal_uInt8[nNewSize];

        if( pNewBuf )
        {
            bRetVal = sal_True; // Success!
            if( nNewSize < nSize )      // Verkleinern ?
            {
                memcpy( pNewBuf, pBuf, (size_t)nNewSize );
                if( nPos > nNewSize )
                    nPos = 0L;
                if( nEndOfData >= nNewSize )
                    nEndOfData = nNewSize-1L;
            }
            else
            {
                memcpy( pNewBuf, pBuf, (size_t)nSize );
            }

            FreeMemory();

            pBuf  = pNewBuf;
            nSize = nNewSize;
        }
    }
    else
    {
        bRetVal = sal_True;
        FreeMemory();
        pBuf = 0;
        nSize = 0;
        nEndOfData = 0;
        nPos = 0;
    }

    return bRetVal;
}

void SvMemoryStream::FreeMemory()
{
    delete[] pBuf;
}

/*************************************************************************
|*
|*    SvMemoryStream::SwitchBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 26.07.94
|*    Letzte Aenderung  OV 26.07.94
|*
*************************************************************************/

void* SvMemoryStream::SwitchBuffer( sal_Size nInitSize, sal_Size nResizeOffset)
{
    Flush();
    if( !bOwnsData )
        return 0;
    Seek( STREAM_SEEK_TO_BEGIN );

    void* pRetVal = pBuf;
    pBuf          = 0;
    nEndOfData    = 0L;
    nResize       = nResizeOffset;
    nPos          = 0;

    if( nResize != 0 && nResize < 16 )
        nResize = 16;

    ResetError();

    if( nInitSize && !AllocateMemory(nInitSize) )
    {
        SetError( SVSTREAM_OUTOFMEMORY );
        nSize = 0;
    }
    else
        nSize = nInitSize;

    SetBufferSize( 64 );
    return pRetVal;
}

void SvMemoryStream::SetSize( sal_Size nNewSize )
{
    long nDiff = (long)nNewSize - (long)nSize;
    ReAllocateMemory( nDiff );
}

TYPEINIT0 ( SvDataCopyStream )

void SvDataCopyStream::Assign( const SvDataCopyStream& )
{
}
