/*************************************************************************
 *
 *  $RCSfile: stream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2000-11-03 16:55:08 $
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

// ToDo:
//  - Read->RefreshBuffer->Auf Aenderungen von nBufActualLen reagieren

#include <string.h>
#include <stdio.h>
#include <ctype.h>  // isspace
#include <stdlib.h> // strtol, _crotl

/*
#if defined( DBG_UTIL ) && defined( DEBUG )
// prueft Synchronisation des Buffers nach allen Read, Write, Seek
#define OV_DEBUG
#endif
*/

#ifdef WIN
#include <dll.hxx>
#include <svwin.h>
#ifndef SEG
#define  SEG(fp)   (*((size_t*)&(fp) + 1))
#endif
#include <solar.h>
#endif

#if defined(BLC)
#define SWAPNIBBLES(c) c=_crotl(c,4);
#else
#define SWAPNIBBLES(c)      \
unsigned char nSwapTmp=c;   \
nSwapTmp <<= 4;             \
c >>= 4;                    \
c |= nSwapTmp;
#endif

#include <new.hxx>
#include <debug.hxx>
#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <stream.hxx>
#include <osl/thread.h>
#include <algorithm>

// -----------------------------------------------------------------------

DBG_NAME( Stream );

// -----------------------------------------------------------------------

// sprintf Param-Mode
#define SPECIAL_PARAM_NONE 0        // Format-Str, Number
#define SPECIAL_PARAM_WIDTH 1       // Format-Str, Width, Number
#define SPECIAL_PARAM_PRECISION 2   // Format-Str, Precision, Number
#define SPECIAL_PARAM_BOTH 3        // Format-Str, Width, Precision, Number

#if SUPD <= 344
#define _CR '\n'
#define _LF '\r'
#endif

// -----------------------------------------------------------------------

// !!! Nicht inline, wenn Operatoren <<,>> inline sind
inline static void SwapUShort( USHORT& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapShort( short& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapInt( int& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapUInt( unsigned int& r )
    {   r = SWAPSHORT(r);   }
inline static void SwapLong( long& r )
    {   r = SWAPLONG(r);   }
inline static void SwapULong( ULONG& r )
    {   r = SWAPLONG(r);   }
inline static void SwapLongInt( int& r )
    {   r = SWAPLONG(r);   }
inline static void SwapLongUInt( unsigned int& r )
    {   r = SWAPLONG(r);   }
#ifdef UNX
inline static void SwapFloat( float& r )
    {
    DBG_ASSERT( FALSE, "SwapFloat noch nicht implementiert!\n" );
    }
inline static void SwapDouble( double& r )
    {
        if( sizeof(double) != 8 )
        {
          DBG_ASSERT( FALSE, "Can only swap 8-Byte-doubles\n" );
        }
        else
        {
          UINT32* c = (UINT32*)(void*)&r;
          c[0] ^= c[1]; // zwei 32-Bit-Werte in situ vertauschen
          c[1] ^= c[0];
          c[0] ^= c[1];
          c[0] = SWAPLONG(c[0]); // und die beiden 32-Bit-Werte selbst in situ drehen
          c[1] = SWAPLONG(c[1]);
        }
    }

#elif MAC

inline static void SwapFloat( float& r )
    {
    DBG_ASSERT( FALSE, "SwapFloat noch nicht implementiert!\n" );
    }

inline static void SwapDouble( double& r )
    {
#ifdef DBG_UTIL
        if( sizeof(double) != 8 )
          DBG_ASSERT( FALSE, "Can only swap 8-Byte-doubles\n" );
#endif

      UINT32* c = (UINT32*)(void*)&r;
      UINT32  nHelp;
      // zwei 32-Bit-Werte in situ vertauschen
      // und die beiden 32-Bit-Werte selbst in situ drehen
      nHelp = SWAPLONG(c[0]);
      c[0]  = SWAPLONG(c[1]);
      c[1] = nHelp;
    }
#endif // #ifdef UNX / elif MAC

//SDO

#define READNUMBER_WITHOUT_SWAP(datatype,value) \
{\
int tmp = eIOMode; \
if( (tmp == STREAM_IO_READ) && sizeof(datatype)<=nBufFree) \
{\
    for (int i = 0; i < sizeof(datatype); i++)\
        ((char *)&r)[i] = pBufPos[i];\
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
    for (int i = 0; i < sizeof(datatype); i++)\
        pBufPos[i] = ((char *)&value)[i];\
    nBufFree -= sizeof(datatype);\
    nBufActualPos += sizeof(datatype);\
    if( nBufActualPos > nBufActualLen )\
        nBufActualLen = nBufActualPos;\
    pBufPos += sizeof(datatype);\
    bIsDirty = TRUE;\
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
ErrCode SvLockBytes::ReadAt(ULONG nPos, void * pBuffer, ULONG nCount,
                            ULONG * pRead) const
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::ReadAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    ULONG nTheRead = m_pStream->Read(pBuffer, nCount);
    if (pRead)
        *pRead = nTheRead;
    return m_pStream->GetErrorCode();
}

//============================================================================
// virtual
ErrCode SvLockBytes::WriteAt(ULONG nPos, const void * pBuffer, ULONG nCount,
                             ULONG * pWritten)
{
    if (!m_pStream)
    {
        DBG_ERROR("SvLockBytes::WriteAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    ULONG nTheWritten = m_pStream->Write(pBuffer, nCount);
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
ErrCode SvLockBytes::SetSize(ULONG nSize)
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
ErrCode SvLockBytes::LockRegion(ULONG, ULONG, LockType)
{
    DBG_ERROR("SvLockBytes::LockRegion(): Not implemented");
    return ERRCODE_NONE;
}

//============================================================================

ErrCode SvLockBytes::UnlockRegion(ULONG, ULONG, LockType)
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
        ULONG nPos = m_pStream->Tell();
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
ErrCode SvAsyncLockBytes::ReadAt(ULONG nPos, void * pBuffer, ULONG nCount,
                                 ULONG * pRead) const
{
    if (m_bTerminated)
        return SvOpenLockBytes::ReadAt(nPos, pBuffer, nCount, pRead);
    else
    {
        ULONG nTheCount = std::min(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
        ErrCode nError = SvOpenLockBytes::ReadAt(nPos, pBuffer, nTheCount,
                                                 pRead);
        return !nCount || nTheCount == nCount || nError ? nError :
                                                          ERRCODE_IO_PENDING;
    }
}

//============================================================================
// virtual
ErrCode SvAsyncLockBytes::WriteAt(ULONG nPos, const void * pBuffer,
                                  ULONG nCount, ULONG * pWritten)
{
    if (m_bTerminated)
        return SvOpenLockBytes::WriteAt(nPos, pBuffer, nCount, pWritten);
    else
    {
        ULONG nTheCount = std::min(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
        ErrCode nError = SvOpenLockBytes::WriteAt(nPos, pBuffer, nTheCount,
                                                  pWritten);
        return !nCount || nTheCount == nCount || nError ? nError :
                                                          ERRCODE_IO_PENDING;
    }
}

//============================================================================
// virtual
ErrCode SvAsyncLockBytes::FillAppend(const void * pBuffer, ULONG nCount,
                                     ULONG * pWritten)
{
    ULONG nTheWritten;
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
ULONG SvAsyncLockBytes::Seek(ULONG nPos)
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

ULONG SvStream::GetData( void* pData, ULONG nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        ULONG nRet;
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

ULONG SvStream::PutData( const void* pData, ULONG nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        ULONG nRet;
        nError = xLockBytes->WriteAt( nActPos, pData, nSize, &nRet );
        nActPos += nRet;
        return nRet;
    }
    else return 0;
}

//========================================================================

ULONG SvStream::SeekPos( ULONG nPos )
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

void SvStream::SetSize( ULONG nSize )
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
    bIsEof              = FALSE;
#if defined( MAC )
    eLineDelimiter      = LINEEND_CR;   // MAC-Format
#elif defined( UNX )
    eLineDelimiter      = LINEEND_LF;   // UNIX-Format
#else
    eLineDelimiter      = LINEEND_CRLF; // DOS-Format
#endif

    SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    nBufFilePos         = 0;
    nBufActualPos       = 0;
    bIsDirty            = FALSE;
    bIsConsistent       = TRUE;
    bIsWritable         = TRUE;

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
    if( pLockBytesP && (pStrm = pLockBytesP->GetStream() ) )
        SetError( pStrm->GetErrorCode() );
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
        delete pRWBuf;
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

USHORT SvStream::IsA() const
{
    return (USHORT)ID_STREAM;
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
    bIsEof = FALSE;
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

void SvStream::SetError( ULONG nErrorCode )
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

void SvStream::SetNumberFormatInt( USHORT nNewFormat )
{
    nNumberFormatInt = nNewFormat;
    bSwap = FALSE;
#ifdef __BIGENDIAN
    if( nNumberFormatInt == NUMBERFORMAT_INT_LITTLEENDIAN )
        bSwap = TRUE;
#else
    if( nNumberFormatInt == NUMBERFORMAT_INT_BIGENDIAN )
        bSwap = TRUE;
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

void SvStream::SetBufferSize( USHORT nBufferSize )
{
    ULONG nActualFilePos = Tell();
    BOOL bDontSeek = (BOOL)(pRWBuf == 0);

    if( bIsDirty && bIsConsistent && bIsWritable )  // wg. Windows NT: Access denied
        Flush();

    if( nBufSize )
    {
        delete pRWBuf;
        nBufFilePos += nBufActualPos;
    }

    pRWBuf          = 0;
    nBufActualLen   = 0;
    nBufActualPos   = 0;
    nBufSize        = nBufferSize;
    if( nBufSize )
        pRWBuf = new BYTE[ nBufSize ];
    bIsConsistent   = TRUE;
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
    bIsDirty        = FALSE;
    bIsConsistent   = TRUE;
    eIOMode         = STREAM_IO_DONTKNOW;

    bIsEof          = FALSE;
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

BOOL SvStream::ReadByteStringLine( String& rStr, rtl_TextEncoding eSrcCharSet )
{
    BOOL bRet;
    ByteString aStr;

    bRet = ReadLine(aStr);
    rStr = UniString( aStr, eSrcCharSet );
    return bRet;
}

BOOL SvStream::ReadLine( ByteString& rStr )
{
    sal_Char    buf[256+1];
    BOOL        bEnd        = FALSE;
    ULONG       nOldFilePos = Tell();
    sal_Char    c           = 0;

    rStr.Erase();
    while( !bEnd && !GetError() )   // !!! nicht auf EOF testen,
                                    // !!! weil wir blockweise
                                    // !!! lesen
    {
        USHORT nLen = (USHORT)Read( buf, sizeof(buf)-1 );
        if ( !nLen )
        {
            if ( rStr.Len() == 0 )
            {
                // der allererste Blockread hat fehlgeschlagen -> Abflug
                bIsEof = TRUE;
                return FALSE;
            }
            else
                break;
        }

        for( USHORT n = 0; n < nLen ; n++ )
        {
            c = buf[n];
            if ( c != '\n' && c != '\r' )
                rStr += c;
            else
            {
                bEnd = TRUE;
                break;
            }
        }
    }

    if ( !bEnd && !GetError() && rStr.Len() )
        bEnd = TRUE;

    nOldFilePos += rStr.Len();
    if( Tell() > nOldFilePos )
        nOldFilePos++;
    Seek( nOldFilePos );  // seeken wg. obigem BlockRead!

    if ( bEnd && (c=='\r' || c=='\n') )  // Sonderbehandlung DOS-Dateien
    {
        char cTemp;
        Read((char*)&cTemp , sizeof(cTemp) );
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            Seek( nOldFilePos );
    }

    if ( bEnd )
        bIsEof = FALSE;
    return bEnd;
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

BOOL SvStream::WriteByteStringLine( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLine( ByteString( rStr, eDestCharSet ) );
}

BOOL SvStream::WriteLine( const ByteString& rStr )
{
    Write( rStr.GetBuffer(), rStr.Len() );
    endl(*this);
    return nError == SVSTREAM_OK;
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

BOOL SvStream::WriteByteStringLines( const String& rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLines( ByteString( rStr, eDestCharSet ) );
}

BOOL SvStream::WriteLines( const ByteString& rStr )
{
    ByteString aStr( rStr );
    aStr.ConvertLineEnd( eLineDelimiter );
    Write( aStr.GetBuffer(), aStr.Len() );
    endl( *this );
    return (BOOL)(nError == SVSTREAM_OK);
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

ULONG SvStream::SeekRel( long nPos )
{
    ULONG nActualPos = Tell();
    nActualPos += nPos;
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

SvStream& SvStream::operator >> ( USHORT& r )
{
    READNUMBER_WITHOUT_SWAP(USHORT,r)
    if( bSwap )
        SwapUShort(r);
    return *this;
}

SvStream& SvStream::operator>> ( ULONG& r)
{
#if(__SIZEOFLONG != 4)
    unsigned int tmp = r;
    READNUMBER_WITHOUT_SWAP(ULONG,tmp)
    r = tmp;
    if( bSwap )
        SwapULong(r);
#else
    READNUMBER_WITHOUT_SWAP(ULONG,r)
    if( bSwap )
        SwapULong(r);
#endif
    return *this;
}

SvStream& SvStream::operator >> ( long& r )
{
#if(__SIZEOFLONG != 4)
    unsigned int tmp = r;
    READNUMBER_WITHOUT_SWAP(long,tmp)
    r = tmp;
    if( bSwap )
        SwapLong(r);
#else
    READNUMBER_WITHOUT_SWAP(long,r)
    if( bSwap )
        SwapLong(r);
#endif
    return *this;
}

SvStream& SvStream::operator >> ( short& r)
{
    READNUMBER_WITHOUT_SWAP(short,r)
    if( bSwap )
        SwapShort(r);
    return *this;
}

SvStream& SvStream::operator >> ( int& r)
{
    READNUMBER_WITHOUT_SWAP(int,r)
    if( bSwap )
    {
#if(__SIZEOFINT == 2)
        SwapInt(r);
#else
        SwapLongInt(r);
#endif
    }
    return *this;
}

SvStream& SvStream::operator>>( unsigned int& r)
{
    READNUMBER_WITHOUT_SWAP(unsigned int,r)
    if( bSwap )
    {
#if(__SIZEOFINT == 2)
        SwapUInt(r);
#else
        SwapLongUInt(r);
#endif
    }
    return *this;
}

SvStream& SvStream::operator>>( signed char& r)
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

SvStream& SvStream::operator>>( char& r)
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

SvStream& SvStream::operator>>( unsigned char& r)
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

SvStream& SvStream::operator>>( float& r)
{
    // Read( (char*)&r, sizeof(float) );
    READNUMBER_WITHOUT_SWAP(float,r)
#ifdef UNX
    if( bSwap )
      SwapFloat(r);
#endif
    return *this;
}

SvStream& SvStream::operator>>( double& r)
{
    // Read( (char*)&r, sizeof(double) );
    READNUMBER_WITHOUT_SWAP(double,r)
#if defined( UNX ) || defined ( MAC )
    if( bSwap )
      SwapDouble(r);
#endif
    return *this;
}

SvStream& SvStream::operator>> ( SvStream& rStream )
{
    const ULONG cBufLen = 0x8000;
    char* pBuf = new char[ cBufLen ];

    ULONG nCount;
    do {
        nCount = Read( pBuf, cBufLen );
        rStream.Write( pBuf, nCount );
    } while( nCount == cBufLen );

    delete pBuf;
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

SvStream& SvStream::operator<< ( USHORT v )
{
    if( bSwap )
        SwapUShort(v);
    WRITENUMBER_WITHOUT_SWAP(USHORT,v)
    return *this;
}

SvStream& SvStream::operator<<  ( ULONG v)
{
#if(__SIZEOFLONG != 4)
    unsigned int tmp = v;
    if( bSwap )
        SwapUInt(tmp);
    WRITENUMBER_WITHOUT_SWAP(unsigned int,tmp)
#else
    if( bSwap )
        SwapULong(v);
    WRITENUMBER_WITHOUT_SWAP(ULONG,v)
#endif
    return *this;
}

SvStream& SvStream::operator<< ( long v )
{
#if(__SIZEOFLONG != 4)
    int tmp = v;
    if( bSwap )
        SwapInt(tmp);
    WRITENUMBER_WITHOUT_SWAP(int,tmp)
#else
    if( bSwap )
        SwapLong(v);
    WRITENUMBER_WITHOUT_SWAP(long,v)
#endif
    return *this;
}

SvStream& SvStream::operator<<  ( short v)
{
    if( bSwap )
        SwapShort(v);
    WRITENUMBER_WITHOUT_SWAP(short,v)
    return *this;
}

SvStream& SvStream::operator<<( int v)
{
    if( bSwap )
    {
#if( __SIZEOFINT == 2 )
        SwapInt( v );
#else
        SwapLongInt( v );
#endif
    }
    WRITENUMBER_WITHOUT_SWAP(int,v)
    return *this;
}

SvStream& SvStream::operator<<  ( signed char v)
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
        bIsDirty = TRUE;
    }
    else
        Write( (char*)&v, sizeof(signed char) );
    return *this;
}


SvStream& SvStream::operator<<  ( unsigned int v)
{
    if( bSwap )
    {
#if( __SIZEOFINT == 2 )
        SwapUInt( v );
#else
        SwapLongUInt( v );
#endif
    }
    WRITENUMBER_WITHOUT_SWAP(int,v)
    return *this;
}

// Sonderbehandlung fuer chars wegen PutBack

SvStream& SvStream::operator<<  ( char v)
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
        bIsDirty = TRUE;
    }
    else
        Write( (char*)&v, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator<<  ( unsigned char v)
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
        bIsDirty = TRUE;
    }
    else
        Write( (char*)&v, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator<< ( float v)
{
#ifdef UNX
    if( bSwap )
      SwapFloat(v);
#endif
    WRITENUMBER_WITHOUT_SWAP(float,v)
    return *this;
}

SvStream& SvStream::operator<< ( const double& r)
{
//    Write( (char*)&r, sizeof( double ) );
#if defined( UNX ) || defined ( MAC )
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

SvStream& SvStream::operator<< ( SvStream& rStream)
{
    const ULONG cBufLen = 0x8000;
    char* pBuf = new char[ cBufLen ];
    ULONG nCount;
    do {
        nCount = rStream.Read( pBuf, cBufLen );
        Write( pBuf, nCount );
    } while( nCount == cBufLen );

    delete pBuf;
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
            sal_Unicode *pStr = rStr.AllocBuffer(nLen);
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
    USHORT nLen = 0;
    operator>>( nLen );
    if( nLen )
    {
        char* pTmp = rStr.AllocBuffer( nLen );
        nLen = (USHORT)Read( pTmp, nLen );
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
                    WRITENUMBER_WITHOUT_SWAP(USHORT,c)
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
    USHORT nLen = rStr.Len();
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

ULONG SvStream::Read( void* pData, ULONG nCount )
{
    ULONG nSaveCount = nCount;
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
        if( nCount <= (ULONG)(nBufActualLen - nBufActualPos ) )
        {
            // Ja!
#ifdef WIN
            hmemcpy( pData, pBufPos, nCount );
#else
            memcpy(pData, pBufPos, (size_t) nCount);
#endif
            nBufActualPos += (USHORT)nCount;
            pBufPos += nCount;
            nBufFree -= (USHORT)nCount;
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
                bIsDirty = FALSE;
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

                // TODO: Typecast vor GetData, USHORT nCountTmp
                ULONG nCountTmp = GetData( pRWBuf, nBufSize );
                if( nCryptMask )
                    EncryptBuffer(pRWBuf, nCountTmp);
                nBufActualLen = (USHORT)nCountTmp;
                if( nCount > nCountTmp )
                {
                    nCount = nCountTmp;  // zurueckstutzen, Eof siehe unten
                }
#ifdef WIN
                hmemcpy( pData, pRWBuf, nCount );
#else
                memcpy( pData, pRWBuf, (size_t)nCount );
#endif
                nBufActualPos = (USHORT)nCount;
                pBufPos = pRWBuf + nCount;
            }
        }
    }
    bIsEof = FALSE;
    nBufFree = nBufActualLen - nBufActualPos;
    if( nCount != nSaveCount && nError != ERRCODE_IO_PENDING )
        bIsEof = TRUE;
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

ULONG SvStream::Write( const void* pData, ULONG nCount )
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
    if( nCount <= (ULONG)(nBufSize - nBufActualPos) )
    {
#ifdef WIN
        hmemcpy( pBufPos, pData, nCount );
#else
        memcpy( pBufPos, pData, (size_t)nCount );
#endif
        nBufActualPos += (USHORT)nCount;
        // wurde der Puffer erweitert ?
        if( nBufActualPos > nBufActualLen )
            nBufActualLen = nBufActualPos;

        pBufPos += nCount;
        bIsDirty = TRUE;
    }
    else
    {
        // Flushen ?
        if( bIsDirty )
        {
            SeekPos( nBufFilePos );
            if( nCryptMask )
                CryptAndWriteBuffer( pRWBuf, (ULONG)nBufActualLen );
            else
                PutData( pRWBuf, nBufActualLen );
            bIsDirty = FALSE;
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
#ifdef WIN
            hmemcpy( pRWBuf, pData, nCount );
#else
            memcpy( pRWBuf, pData, (size_t)nCount );
#endif
            // Reihenfolge!
            nBufFilePos += nBufActualPos;
            nBufActualPos = (USHORT)nCount;
            pBufPos = pRWBuf + nCount;
            nBufActualLen = (USHORT)nCount;
            bIsDirty = TRUE;
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

ULONG SvStream::Seek( ULONG nFilePos )
{
    eIOMode = STREAM_IO_DONTKNOW;

    bIsEof = FALSE;
    if( !pRWBuf )
    {
        nBufFilePos = SeekPos( nFilePos );
        DBG_ASSERT(Tell()==nBufFilePos,"Out Of Sync!")
        return nBufFilePos;
    }

    // Ist Position im Puffer ?
    if( nFilePos >= nBufFilePos && nFilePos <= (nBufFilePos + nBufActualLen))
    {
        nBufActualPos = (USHORT)(nFilePos - nBufFilePos);
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
            bIsDirty = FALSE;
        }
        nBufActualLen = 0;
        nBufActualPos = 0;
        pBufPos       = pRWBuf;
        nBufFilePos = SeekPos( nFilePos );
    }
#ifdef OV_DEBUG
    {
        ULONG nDebugTemp = nBufFilePos + nBufActualPos;
        DBG_ASSERT(Tell()==nDebugTemp,"Sync?")
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
            CryptAndWriteBuffer( pRWBuf, (ULONG)nBufActualLen );
        else
            if( PutData( pRWBuf, nBufActualLen ) != nBufActualLen )
                SetError( SVSTREAM_WRITE_ERROR );
        bIsDirty = FALSE;
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
        bIsDirty = FALSE;
    }
    bIsConsistent = FALSE;  // Puffer enthaelt jetzt TRASH
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
            memmove( pRWBuf+1, pRWBuf, (USHORT)nBufActualLen );
            nBufActualLen++;
            nBufFree++;
        }
        nBufFilePos--;
        *pRWBuf = aCh;
    }
    eIOMode = STREAM_IO_DONTKNOW;
    bIsEof = FALSE;
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
            CryptAndWriteBuffer( pRWBuf, (ULONG)nBufActualLen );
        else
            PutData( pRWBuf, nBufActualLen );
        bIsDirty = FALSE;
    }
    SeekPos( nBufFilePos );
    nBufActualLen = (USHORT)GetData( pRWBuf, nBufSize );
    if( nBufActualLen && nError == ERRCODE_IO_PENDING )
        nError = ERRCODE_NONE;
    if( nCryptMask )
        EncryptBuffer(pRWBuf, (ULONG)nBufActualLen);
    bIsConsistent = TRUE;
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
    ULONG nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    ULONG nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rLong = strtol( buf, &pEndPtr, (int)nRadix );
    nFPtr += ( (ULONG)pEndPtr - (ULONG)(&(buf[0])) );
    Seek( nFPtr );
    bIsEof = FALSE;
    return *this;
}

SvStream& SvStream::ReadNumber( ULONG& rULong )
{
    EatWhite();
    if( bIsEof || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    ULONG nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    ULONG nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rULong = strtoul( buf, &pEndPtr, (int)nRadix );
    nFPtr += ( (ULONG)pEndPtr - (ULONG)buf );
    Seek( nFPtr );
    bIsEof = FALSE;
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
    ULONG nFPtr = Tell();
    char buf[ BUFSIZE_LONG ];
    memset( buf, 0, BUFSIZE_LONG );
    ULONG nTemp = Read( buf, BUFSIZE_LONG-1 );
    if( !nTemp || nError )
    {
        SetError( SVSTREAM_GENERALERROR );
        return *this;
    }
    char *pEndPtr;
    rDouble = strtod( buf, &pEndPtr );
    nFPtr += ( (ULONG)pEndPtr - (ULONG)buf );
    Seek( nFPtr );
    bIsEof = FALSE;
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

SvStream& SvStream::WriteNumber( ULONG nULong )
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
            nLen = sprintf( buffer, aFStr.GetBuffer(), nULong );
            break;
        case SPECIAL_PARAM_WIDTH :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nWidth, nULong );
            break;
        case SPECIAL_PARAM_PRECISION :
            nLen = sprintf( buffer, aFStr.GetBuffer(), nPrecision,nULong);
            break;
        default:
            nLen=sprintf(buffer,aFStr.GetBuffer(),nWidth,nPrecision,nULong);
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

ULONG SvStream::CryptAndWriteBuffer( const void* pStart, ULONG nLen)
{
    unsigned char* pTemp = new unsigned char[CRYPT_BUFSIZE];
    unsigned char* pDataPtr = (unsigned char*)pStart;
    ULONG nCount = 0;
    ULONG nBufCount;
    unsigned char nMask = nCryptMask;
    do
    {
        if( nLen >= CRYPT_BUFSIZE )
            nBufCount = CRYPT_BUFSIZE;
        else
            nBufCount = nLen;
        nLen -= nBufCount;
        memcpy( pTemp, pDataPtr, (USHORT)nBufCount );
        // **** Verschluesseln *****
        for ( USHORT n=0; n < CRYPT_BUFSIZE; n++ )
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
    delete pTemp;
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

BOOL SvStream::EncryptBuffer(void* pStart, ULONG nLen)
{
    unsigned char* pTemp = (unsigned char*)pStart;
    unsigned char nMask = nCryptMask;

    for ( ULONG n=0; n < nLen; n++, pTemp++ )
    {
        unsigned char aCh = *pTemp;
        SWAPNIBBLES(aCh)
        aCh ^= nMask;
        *pTemp = aCh;
    }
    return TRUE;
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
        for( USHORT i = 0; i < nLen; i++ ) {
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

void SvStream::SyncSvStream( ULONG nNewStreamPos )
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

BOOL SvStream::SetStreamSize( ULONG nSize )
{
#ifdef DBG_UTIL
    ULONG nFPos = Tell();
#endif
    USHORT nBuf = nBufSize;
    SetBufferSize( 0 );
    SetSize( nSize );
    SetBufferSize( nBuf );
    DBG_ASSERT(Tell()==nFPos,"SetStreamSize failed")
    return (BOOL)(nError == 0);
}

//============================================================================

void SvStream::AddMark( ULONG nPos )
{
}

//============================================================================

void SvStream::RemoveMark( ULONG nPos )
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

/*************************************************************************
|*
|*    SvMemoryStream::SvMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 20.06.94
|*    Letzte Aenderung  OV 20.06.94
|*
*************************************************************************/

SvMemoryStream::SvMemoryStream( void* pBuffer, ULONG nBufSize,
                                StreamMode eMode )
{
    if( eMode & STREAM_WRITE )
        bIsWritable = TRUE;
    else
        bIsWritable = FALSE;
    nEndOfData  = nBufSize;
    bOwnsData   = FALSE;
    pBuf        = (BYTE *) pBuffer;
    nResize     = 0L;
    nSize       = nBufSize;
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

SvMemoryStream::SvMemoryStream( ULONG nInitSize, ULONG nResizeOffset )
{
    bIsWritable = TRUE;
    bOwnsData   = TRUE;
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

USHORT SvMemoryStream::IsA() const
{
    return (USHORT)ID_MEMORYSTREAM;
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

void* SvMemoryStream::SetBuffer( void* pNewBuf, ULONG nCount,
                                 BOOL bOwnsDat, ULONG nEOF )
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

    pBuf        = (BYTE *) pNewBuf;
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

ULONG SvMemoryStream::GetData( void* pData, ULONG nCount )
{
    ULONG nMaxCount = nEndOfData-nPos;
    if( nCount > nMaxCount )
        nCount = nMaxCount;
#ifdef WIN
    void _huge* pTmp = (void _huge*)((char _huge*)pBuf + nPos);
    hmemcpy( (void _huge*)pData, pTmp, (long)nCount);
#else
    memcpy( pData, pBuf+nPos, (size_t)nCount );
#endif
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

ULONG SvMemoryStream::PutData( const void* pData, ULONG nCount )
{
    if( GetError() )
        return 0L;

    ULONG nMaxCount = nSize-nPos;

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
#ifdef WIN
    void _huge* pTmp = (void _huge*)((char _huge*)pBuf + nPos);
    hmemcpy( pTmp, (void _huge*)pData,(long)nCount);
#else
    memcpy( pBuf+nPos, pData, (size_t)nCount);
#endif

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

ULONG SvMemoryStream::SeekPos( ULONG nNewPos )
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

BOOL SvMemoryStream::AllocateMemory( ULONG nNewSize )
{
    pBuf = (BYTE*)SvMemAlloc( nNewSize, MEM_NOCALLNEWHDL );
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

BOOL SvMemoryStream::ReAllocateMemory( long nDiff )
{
    BOOL bRetVal    = FALSE;
    long nTemp      = (long)nSize;
    nTemp           += nDiff;
    ULONG nNewSize  = (ULONG)nTemp;

    if( nNewSize )
    {
        BYTE* pNewBuf   = (BYTE *) SvMemAlloc( nNewSize,MEM_NOCALLNEWHDL );

        if( pNewBuf )
        {
            bRetVal = TRUE; // Success!
            if( nNewSize < nSize )      // Verkleinern ?
            {
#ifdef WIN
                hmemcpy((void _huge*)pNewBuf,(void _huge*)pBuf,(long)nNewSize );
#else
                memcpy( pNewBuf, pBuf, (size_t)nNewSize );
#endif
                if( nPos > nNewSize )
                    nPos = 0L;
                if( nEndOfData >= nNewSize )
                    nEndOfData = nNewSize-1L;
            }
            else
            {
#ifdef WIN
                hmemcpy( (void _huge*)pNewBuf, (void _huge*)pBuf, (long)nSize );
#else
                memcpy( pNewBuf, pBuf, (size_t)nSize );
#endif
            }

            FreeMemory();

            pBuf  = pNewBuf;
            nSize = nNewSize;
        }
    }
    else
    {
        bRetVal = TRUE;
        pBuf = 0;
        nSize = 0;
        nEndOfData = 0;
        nPos = 0;
    }

    return bRetVal;
}

void SvMemoryStream::FreeMemory()
{
    SvMemFree( pBuf );
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

void* SvMemoryStream::SwitchBuffer( ULONG nInitSize, ULONG nResizeOffset)
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

void SvMemoryStream::SetSize( ULONG nNewSize )
{
    long nDiff = (long)nNewSize - (long)nSize;
    ReAllocateMemory( nDiff );
}

// *********************************************************************
// Mac implementierung (mit Handles) in StrmMac.cxx
// *********************************************************************

#ifndef MAC

/*************************************************************************
|*
|*    SvSharedMemoryStream::SvSharedMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

SvSharedMemoryStream::SvSharedMemoryStream( void* pBuffer,ULONG nBufSize,
                                            StreamMode eMode ) :
                SvMemoryStream( (char*)pBuffer, nBufSize, eMode )
{
    aHandle = 0;
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::SvSharedMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

SvSharedMemoryStream::SvSharedMemoryStream( ULONG nInitSize,
                                            ULONG nResizeOffset) :
                SvMemoryStream( (void*)NULL )
{
    if( !nInitSize )
        nInitSize = 1024;

    aHandle     = 0;
    bIsWritable = TRUE;
    bOwnsData   = TRUE;
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
|*    SvSharedMemoryStream::~SvSharedMemoryStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

SvSharedMemoryStream::~SvSharedMemoryStream()
{
    if( bOwnsData )
    {
        FreeMemory();
        pBuf = 0; // damit der Dtor von SvMemoryStream nicht mehr zuschlaegt
    }
    else
        Flush();
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::GetData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

const void* SvSharedMemoryStream::GetData()
{
    Flush();
    return pBuf;
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::operator const char*()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 08.06.94
|*    Letzte Aenderung  OV 08.06.94
|*
*************************************************************************/

SvSharedMemoryStream::operator const void*()
{
    Flush();
    return pBuf;
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::IsA()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

USHORT SvSharedMemoryStream::IsA() const
{
    return (USHORT)ID_SHAREDMEMORYSTREAM;
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::SwitchBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    KH 16.06.95
|*    Letzte Aenderung  KH 16.06.95
|*
*************************************************************************/

void* SvSharedMemoryStream::SwitchBuffer( ULONG nInitSize, ULONG nResize )
{
    return (void*)SvMemoryStream::SwitchBuffer(nInitSize, nResize);
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::SetBuffer()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    KH 16.06.95
|*    Letzte Aenderung  KH 16.06.95
|*
*************************************************************************/

void* SvSharedMemoryStream::SetBuffer( void* pBuf, ULONG nSize, BOOL bOwnsData,
                                       ULONG nEof)
{
    return (void*)SvMemoryStream::SetBuffer((char*)pBuf,nSize,bOwnsData,nEof);
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::GetData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    KH 19.06.95
|*    Letzte Aenderung  KH 19.06.95
|*
*************************************************************************/

ULONG SvSharedMemoryStream::GetData( void* pData, ULONG nCount )
{
    return SvMemoryStream::GetData(pData, nCount);
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::PutData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    KH 19.06.95
|*    Letzte Aenderung  KH 19.06.95
|*
*************************************************************************/

ULONG SvSharedMemoryStream::PutData( const void* pData, ULONG nCount )
{
    return SvMemoryStream::PutData(pData, nCount);
}


//
// Speicherverwaltung (Alloc, Free, Realloc, SetHandle)
// Standardimplementation DOS & UNIX
//

#if defined(DOS) || defined(UNX)

/*************************************************************************
|*
|*    SvSharedMemoryStream::SetHandle()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 05.10.95
|*    Letzte Aenderung  OV 05.10.95
|*
*************************************************************************/

void* SvSharedMemoryStream::SetHandle( void* aHandle, ULONG nSize,
    BOOL bOwnsData, ULONG nEOF)
{
    DBG_ERROR("SvSharedMemoryStream::SetHandle not implemented");
    return 0;
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::AllocateMemory()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

BOOL SvSharedMemoryStream::AllocateMemory( ULONG nNewSize )
{
    pBuf = new BYTE[ nNewSize ];
    return( pBuf != 0 );
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::ReAllocateMemory()   (Bozo-Algorithmus)
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

BOOL SvSharedMemoryStream::ReAllocateMemory( long nDiff )
{
    BOOL bRetVal    = FALSE;
    ULONG nNewSize  = nSize + nDiff;
    if( nNewSize )
    {
        BYTE* pNewBuf   = new BYTE[ nNewSize ];
        if( pNewBuf )
        {
            bRetVal = TRUE; // Success!
            if( nNewSize < nSize )      // Verkleinern ?
            {
                memcpy( pNewBuf, pBuf, (size_t)nNewSize );
                if( nPos > nNewSize )
                    nPos = 0L;
                if( nEndOfData >= nNewSize )
                    nEndOfData = nNewSize-1L;
            }
            else
                memcpy( pNewBuf, pBuf, (size_t)nSize );

            FreeMemory();

            pBuf  = pNewBuf;
            nSize = nNewSize;
        }
    }
    else
    {
        FreeMemory();
        bRetVal = TRUE;
        pBuf = 0;
        nSize = 0;
        nPos = 0;
        nEndOfData = 0;
    }
    return bRetVal;
}

void SvSharedMemoryStream::FreeMemory()
{
    delete pBuf;
}


#endif

#endif

TYPEINIT0 ( SvDataCopyStream )

// --------------------

// Diese Methoden muessen fuer die Win16-Version implementiert werden,
// da sonst der Compiler Aerger macht. (?)
/*
TypeId SvDataCopyStream::Type() const
{
    return 0;
}

BOOL SvDataCopyStream::IsA(TypeId aId) const
{
    return FALSE;
}
*/

void SvDataCopyStream::Assign( const SvDataCopyStream& rHack)
{
}
