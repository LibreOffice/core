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

// TODO: Read->RefreshBuffer-> React to changes from nBufActualLen

#include <cstddef>

#include <string.h>
#include <stdio.h>
#include <ctype.h>  // isspace
#include <stdlib.h> // strtol, _crotl

#include "boost/static_assert.hpp"

#include <osl/endian.h>

#include <comphelper/string.hxx>

#define SWAPNIBBLES(c)      \
unsigned char nSwapTmp=c;   \
nSwapTmp <<= 4;             \
c >>= 4;                    \
c |= nSwapTmp;

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <osl/thread.h>
#include <algorithm>

DBG_NAME( Stream )

// !!! Do not inline if already the operators <<,>> are inline
inline static void SwapUShort( sal_uInt16& r )
    {   r = OSL_SWAPWORD(r);   }
inline static void SwapShort( short& r )
    {   r = OSL_SWAPWORD(r);   }
inline static void SwapLong( long& r )
    {   r = OSL_SWAPDWORD(r);   }
inline static void SwapULong( sal_uInt32& r )
    {   r = OSL_SWAPDWORD(r);   }
inline static void SwapLongInt( sal_Int32& r )
    {   r = OSL_SWAPDWORD(r);   }
inline static void SwapLongUInt( unsigned int& r )
    {   r = OSL_SWAPDWORD(r);   }

inline static void SwapUInt64( sal_uInt64& r )
    {
        union
        {
            sal_uInt64 n;
            sal_uInt32 c[2];
        } s;

        s.n = r;
        s.c[0] ^= s.c[1]; // swap the 32 bit words
        s.c[1] ^= s.c[0];
        s.c[0] ^= s.c[1];
        // swap the bytes in the words
        s.c[0] = OSL_SWAPDWORD(s.c[0]);
        s.c[1] = OSL_SWAPDWORD(s.c[1]);
        r = s.n;
    }
inline static void SwapInt64( sal_Int64& r )
    {
        union
        {
            sal_Int64 n;
            sal_Int32 c[2];
        } s;

        s.n = r;
        s.c[0] ^= s.c[1]; // swap the 32 bit words
        s.c[1] ^= s.c[0];
        s.c[0] ^= s.c[1];
        // swap the bytes in the words
        s.c[0] = OSL_SWAPDWORD(s.c[0]);
        s.c[1] = OSL_SWAPDWORD(s.c[1]);
        r = s.n;
    }

#ifdef UNX
inline static void SwapFloat( float& r )
    {
        union
        {
            float f;
            sal_uInt32 c;
        } s;

        s.f = r;
        s.c = OSL_SWAPDWORD( s.c );
        r = s.f;
    }

inline static void SwapDouble( double& r )
    {
        if( sizeof(double) != 8 )
        {
          DBG_ASSERT( false, "Can only swap 8-Byte-doubles\n" );
        }
        else
        {
            union
            {
                double d;
                sal_uInt32 c[2];
            } s;

            s.d = r;
            s.c[0] ^= s.c[1]; // swap 32-bit values in situ
            s.c[1] ^= s.c[0];
            s.c[0] ^= s.c[1];
            s.c[0] = OSL_SWAPDWORD(s.c[0]); // swap dword itself in situ
            s.c[1] = OSL_SWAPDWORD(s.c[1]);
            r = s.d;
        }
    }
#endif

//SDO

#define READNUMBER_WITHOUT_SWAP(datatype,value) \
    if( bIoRead && sizeof(datatype)<=nBufFree)             \
    {                                                       \
        for (std::size_t i = 0; i < sizeof(datatype); i++)  \
            ((char *)&value)[i] = pBufPos[i];              \
        nBufActualPos += sizeof(datatype);                  \
        pBufPos += sizeof(datatype);                        \
        nBufFree -= sizeof(datatype);                       \
    }                                                       \
    else                                                    \
    {                                                       \
        Read( (char*)&value, sizeof(datatype) );            \
    }                                                       \


#define WRITENUMBER_WITHOUT_SWAP(datatype,value) \
    if( bIoWrite && sizeof(datatype) <= nBufFree)    \
    {                                                   \
        for (std::size_t i = 0; i < sizeof(datatype); i++)  \
            pBufPos[i] = ((char *)&value)[i];               \
        nBufFree -= sizeof(datatype);                       \
        nBufActualPos += sizeof(datatype);                  \
        if( nBufActualPos > nBufActualLen )                 \
            nBufActualLen = nBufActualPos;                  \
        pBufPos += sizeof(datatype);                        \
        bIsDirty = true;                                    \
    }                                                       \
    else                                                    \
    {                                                       \
        Write( (char*)&value, sizeof(datatype) );           \
    }                                                       \

//  class SvLockBytes

void SvLockBytes::close()
{
    if (m_bOwner)
        delete m_pStream;
    m_pStream = 0;
}

TYPEINIT0(SvLockBytes);

// virtual
ErrCode SvLockBytes::ReadAt(sal_Size nPos, void * pBuffer, sal_Size nCount,
                            sal_Size * pRead) const
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::ReadAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    sal_Size nTheRead = m_pStream->Read(pBuffer, nCount);
    if (pRead)
        *pRead = nTheRead;
    return m_pStream->GetErrorCode();
}

// virtual
ErrCode SvLockBytes::WriteAt(sal_Size nPos, const void * pBuffer, sal_Size nCount,
                             sal_Size * pWritten)
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::WriteAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    sal_Size nTheWritten = m_pStream->Write(pBuffer, nCount);
    if (pWritten)
        *pWritten = nTheWritten;
    return m_pStream->GetErrorCode();
}

// virtual
ErrCode SvLockBytes::Flush() const
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::Flush(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Flush();
    return m_pStream->GetErrorCode();
}

// virtual
ErrCode SvLockBytes::SetSize(sal_Size nSize)
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::SetSize(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->SetStreamSize(nSize);
    return m_pStream->GetErrorCode();
}

ErrCode SvLockBytes::Stat(SvLockBytesStat * pStat, SvLockBytesStatFlag) const
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::Stat(): Bad stream");
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

//  class SvOpenLockBytes

TYPEINIT1(SvOpenLockBytes, SvLockBytes);

//  class SvAsyncLockBytes

TYPEINIT1(SvAsyncLockBytes, SvOpenLockBytes);

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

// virtual
sal_Size SvAsyncLockBytes::Seek(sal_Size nPos)
{
    if (nPos != STREAM_SEEK_TO_END)
        m_nSize = nPos;
    return m_nSize;
}

//  class SvStream

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

void SvStream::FlushData()
{
    if( !GetError() )
    {
        DBG_ASSERT( xLockBytes.Is(), "pure virtual function" );
        nError = xLockBytes->Flush();
    }
}

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
    nCryptMask          = 0;
    bIsEof              = false;
#if defined UNX
    eLineDelimiter      = LINEEND_LF;   // UNIX-Format
#else
    eLineDelimiter      = LINEEND_CRLF; // DOS-Format
#endif

    SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

    nBufFilePos         = 0;
    nBufActualPos       = 0;
    bIsDirty            = false;
    bIsConsistent       = true;
    bIsWritable         = true;

    pRWBuf              = 0;
    pBufPos             = 0;
    nBufSize            = 0;
    nBufActualLen       = 0;
    bIoRead             = false;
    bIoWrite            = false;
    nBufFree            = 0;

    eStreamMode         = 0;

    nVersion           = 0;

    ClearError();
}

SvStream::SvStream( SvLockBytes* pLockBytesP )
{
    DBG_CTOR( Stream, NULL );

    ImpInit();
    xLockBytes = pLockBytesP;
    if( pLockBytesP ) {
        const SvStream* pStrm = pLockBytesP->GetStream();
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

SvStream::~SvStream()
{
    DBG_DTOR( Stream, NULL );

    if ( xLockBytes.Is() )
        Flush();

    if( pRWBuf )
        delete[] pRWBuf;
}

sal_uInt16 SvStream::IsA() const
{
    return (sal_uInt16)ID_STREAM;
}

void SvStream::ClearError()
{
    bIsEof = false;
    nError = SVSTREAM_OK;
}

void SvStream::SetError( sal_uInt32 nErrorCode )
{
    if ( nError == SVSTREAM_OK )
        nError = nErrorCode;
}

void SvStream::SetNumberFormatInt( sal_uInt16 nNewFormat )
{
    nNumberFormatInt = nNewFormat;
    bSwap = false;
#ifdef OSL_BIGENDIAN
    if( nNumberFormatInt == NUMBERFORMAT_INT_LITTLEENDIAN )
        bSwap = true;
#else
    if( nNumberFormatInt == NUMBERFORMAT_INT_BIGENDIAN )
        bSwap = true;
#endif
}

void SvStream::SetBufferSize( sal_uInt16 nBufferSize )
{
    sal_Size nActualFilePos = Tell();
    bool bDontSeek = (pRWBuf == 0);

    if( bIsDirty && bIsConsistent && bIsWritable )  // due to Windows NT: Access denied
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
    bIsConsistent   = true;
    pBufPos         = pRWBuf;
    bIoRead = bIoWrite = false;
    if( !bDontSeek )
        SeekPos( nActualFilePos );
}

void SvStream::ClearBuffer()
{
    nBufActualLen   = 0;
    nBufActualPos   = 0;
    nBufFilePos     = 0;
    pBufPos         = pRWBuf;
    bIsDirty        = false;
    bIsConsistent   = true;
    bIoRead = bIoWrite = false;

    bIsEof          = false;
}

void SvStream::ResetError()
{
    ClearError();
}

bool SvStream::ReadByteStringLine( OUString& rStr, rtl_TextEncoding eSrcCharSet,
                                       sal_Int32 nMaxBytesToRead )
{
    OString aStr;
    bool bRet = ReadLine( aStr, nMaxBytesToRead);
    rStr = OStringToOUString(aStr, eSrcCharSet);
    return bRet;
}

bool SvStream::ReadLine( OString& rStr, sal_Int32 nMaxBytesToRead )
{
    sal_Char    buf[256+1];
    bool        bEnd        = false;
    sal_Size       nOldFilePos = Tell();
    sal_Char    c           = 0;
    sal_Size       nTotalLen   = 0;

    OStringBuffer aBuf(4096);
    while( !bEnd && !GetError() )   // Don't test for EOF as we
                                    // are reading block-wise!
    {
        sal_uInt16 nLen = (sal_uInt16)Read( buf, sizeof(buf)-1 );
        if ( !nLen )
        {
            if ( aBuf.isEmpty() )
            {
                // Exit on first block-read error
                bIsEof = true;
                rStr = OString();
                return false;
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
                bEnd = true;
                break;
            }
            if ( n < j )
                buf[n] = c;
            ++n;
        }
        nTotalLen += j;
        if (nTotalLen > static_cast<sal_Size>(nMaxBytesToRead))
        {
            n -= nTotalLen - nMaxBytesToRead;
            nTotalLen = nMaxBytesToRead;
            bEnd = true;
        }
        if ( n )
            aBuf.append(buf, n);
    }

    if ( !bEnd && !GetError() && !aBuf.isEmpty() )
        bEnd = true;

    nOldFilePos += nTotalLen;
    if( Tell() > nOldFilePos )
        nOldFilePos++;
    Seek( nOldFilePos );  // Seek pointer due to BlockRead above

    if ( bEnd && (c=='\r' || c=='\n') )  // Special treatment for DOS files
    {
        char cTemp;
        sal_Size nLen = Read((char*)&cTemp , sizeof(cTemp) );
        if ( nLen ) {
            if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
                Seek( nOldFilePos );
        }
    }

    if ( bEnd )
        bIsEof = false;
    rStr = aBuf.makeStringAndClear();
    return bEnd;
}

bool SvStream::ReadUniStringLine( OUString& rStr, sal_Int32 nMaxCodepointsToRead )
{
    sal_Unicode buf[256+1];
    bool        bEnd        = false;
    sal_Size       nOldFilePos = Tell();
    sal_Unicode c           = 0;
    sal_Size       nTotalLen   = 0;

    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "ReadUniStringLine: swapping sizeof(sal_Unicode) not implemented" );

    OUStringBuffer aBuf(4096);
    while( !bEnd && !GetError() )   // Don't test for EOF as we
                                    // are reading block-wise!
    {
        sal_uInt16 nLen = (sal_uInt16)Read( (char*)buf, sizeof(buf)-sizeof(sal_Unicode) );
        nLen /= sizeof(sal_Unicode);
        if ( !nLen )
        {
            if ( aBuf.isEmpty() )
            {
                // exit on first BlockRead error
                bIsEof = true;
                rStr = OUString();
                return false;
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
                bEnd = true;
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
        nTotalLen += j;
        if (nTotalLen > static_cast<sal_Size>(nMaxCodepointsToRead))
        {
            n -= nTotalLen - nMaxCodepointsToRead;
            nTotalLen = nMaxCodepointsToRead;
            bEnd = true;
        }
        if ( n )
            aBuf.append( buf, n );
    }

    if ( !bEnd && !GetError() && !aBuf.isEmpty() )
        bEnd = true;

    nOldFilePos += nTotalLen * sizeof(sal_Unicode);
    if( Tell() > nOldFilePos )
        nOldFilePos += sizeof(sal_Unicode);
    Seek( nOldFilePos );  // seek due to BlockRead above

    if ( bEnd && (c=='\r' || c=='\n') )  // special treatment for DOS files
    {
        sal_Unicode cTemp;
        Read( (char*)&cTemp, sizeof(cTemp) );
        if ( bSwap )
            SwapUShort( cTemp );
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            Seek( nOldFilePos );
    }

    if ( bEnd )
        bIsEof = false;
    rStr = aBuf.makeStringAndClear();
    return bEnd;
}

bool SvStream::ReadUniOrByteStringLine( OUString& rStr, rtl_TextEncoding eSrcCharSet,
                                            sal_Int32 nMaxCodepointsToRead )
{
    if ( eSrcCharSet == RTL_TEXTENCODING_UNICODE )
        return ReadUniStringLine( rStr, nMaxCodepointsToRead );
    else
        return ReadByteStringLine( rStr, eSrcCharSet, nMaxCodepointsToRead );
}

OString read_zeroTerminated_uInt8s_ToOString(SvStream& rStream)
{
    OStringBuffer aOutput(256);

    sal_Char buf[ 256 + 1 ];
    bool bEnd = false;
    sal_Size nFilePos = rStream.Tell();

    while( !bEnd && !rStream.GetError() )
    {
        sal_Size nLen = rStream.Read(buf, sizeof(buf)-1);
        if (!nLen)
            break;

        sal_Size nReallyRead = nLen;
        const sal_Char* pPtr = buf;
        while (nLen && *pPtr)
            ++pPtr, --nLen;

        bEnd =  ( nReallyRead < sizeof(buf)-1 )         // read less than attempted to read
                ||  (  ( nLen > 0 )                    // OR it is inside the block we read
                    &&  ( 0 == *pPtr )                  //    AND found a string terminator
                    );

        aOutput.append(buf, pPtr - buf);
    }

    nFilePos += aOutput.getLength();
    if (rStream.Tell() > nFilePos)
        rStream.Seek(nFilePos+1);  // seek due to FileRead above
    return aOutput.makeStringAndClear();
}

OUString read_zeroTerminated_uInt8s_ToOUString(SvStream& rStream, rtl_TextEncoding eEnc)
{
    return OStringToOUString(
        read_zeroTerminated_uInt8s_ToOString(rStream), eEnc);
}

/** Attempt to write a prefixed sequence of nUnits 16bit units from an OUString,
    returned value is number of bytes written */
sal_Size write_uInt16s_FromOUString(SvStream& rStrm, const OUString& rStr,
    sal_Size nUnits)
{
    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "write_uInt16s_FromOUString: swapping sizeof(sal_Unicode) not implemented" );
    sal_Size nWritten;
    if (!rStrm.IsEndianSwap())
        nWritten = rStrm.Write( (char*)rStr.getStr(), nUnits * sizeof(sal_Unicode) );
    else
    {
        sal_Size nLen = nUnits;
        sal_Unicode aBuf[384];
        sal_Unicode* const pTmp = ( nLen > 384 ? new sal_Unicode[nLen] : aBuf);
        memcpy( pTmp, rStr.getStr(), nLen * sizeof(sal_Unicode) );
        sal_Unicode* p = pTmp;
        const sal_Unicode* const pStop = pTmp + nLen;
        while ( p < pStop )
        {
            SwapUShort( *p );
            p++;
        }
        nWritten = rStrm.Write( (char*)pTmp, nLen * sizeof(sal_Unicode) );
        if ( pTmp != aBuf )
            delete [] pTmp;
    }
    return nWritten;
}

bool SvStream::WriteUnicodeOrByteText( const OUString& rStr, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
    {
        write_uInt16s_FromOUString(*this, rStr, rStr.getLength());
        return nError == SVSTREAM_OK;
    }
    else
    {
        OString aStr(OUStringToOString(rStr, eDestCharSet));
        write_uInt8s_FromOString(*this, aStr, aStr.getLength());
        return nError == SVSTREAM_OK;
    }
}

bool SvStream::WriteByteStringLine( const OUString& rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLine(OUStringToOString(rStr, eDestCharSet));
}

bool SvStream::WriteLine(const OString& rStr)
{
    Write(rStr.getStr(), rStr.getLength());
    endl(*this);
    return nError == SVSTREAM_OK;
}

bool SvStream::WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        *this << ch;
    else
    {
        OString aStr(&ch, 1, eDestCharSet);
        Write(aStr.getStr(), aStr.getLength());
    }
    return nError == SVSTREAM_OK;
}

bool SvStream::StartWritingUnicodeText()
{
    SetEndianSwap( false );     // write native format
    // BOM, Byte Order Mark, U+FEFF, see
    // http://www.unicode.org/faq/utf_bom.html#BOM
    // Upon read: 0xfeff(-257) => no swap; 0xfffe(-2) => swap
    *this << sal_uInt16( 0xfeff );
    return nError == SVSTREAM_OK;
}

bool SvStream::StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet )
{
    if (!(  eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
            eReadBomCharSet == RTL_TEXTENCODING_UNICODE ||
            eReadBomCharSet == RTL_TEXTENCODING_UTF8))
        return true;    // nothing to read

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

SvStream& SvStream::operator>>(sal_uInt16& r)
{
    sal_uInt16 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt16, n)
    if (good())
    {
        if (bSwap)
            SwapUShort(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>(sal_uInt32& r)
{
    sal_uInt32 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt32, n)
    if (good())
    {
        if (bSwap)
            SwapULong(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>(sal_uInt64& r)
{
    sal_uInt64 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt64, n)
    if (good())
    {
        if (bSwap)
            SwapUInt64(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>(sal_Int16& r)
{
    sal_Int16 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_Int16, n)
    if (good())
    {
        if (bSwap)
            SwapShort(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>(sal_Int32& r)
{
    sal_Int32 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_Int32, n)
    if (good())
    {
        if (bSwap)
            SwapLongInt(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadInt64(sal_Int64& r)
{
    sal_Int64 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_Int64, n)
    if (good())
    {
        if (bSwap)
            SwapInt64(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>( signed char& r )
{
    if( (bIoRead || !bIsConsistent) &&
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

// Special treatment for Chars due to PutBack

SvStream& SvStream::operator>>( char& r )
{
    if( (bIoRead || !bIsConsistent) &&
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
    if( (bIoRead || !bIsConsistent) &&
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

SvStream& SvStream::operator>>(float& r)
{
    float n = 0;
    READNUMBER_WITHOUT_SWAP(float, n)
    if (good())
    {
#if defined UNX
        if (bSwap)
          SwapFloat(n);
#endif
        r = n;
    }
    return *this;
}

SvStream& SvStream::operator>>(double& r)
{
    double n = 0;
    READNUMBER_WITHOUT_SWAP(double, n)
    if (good())
    {
#if defined UNX
        if (bSwap)
          SwapDouble(n);
#endif
        r = n;
    }
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

SvStream& SvStream::operator<<  ( sal_uInt64 v )
{
    if( bSwap )
        SwapUInt64(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt64,v)
    return *this;
}

SvStream& SvStream::operator<< ( sal_Int16 v )
{
    if( bSwap )
        SwapShort(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int16,v)
    return *this;
}

SvStream& SvStream::operator<<  ( sal_Int32 v )
{
    if( bSwap )
        SwapLongInt(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int32,v)
    return *this;
}

SvStream& SvStream::WriteInt64  (sal_Int64 v)
{
    if( bSwap )
        SwapInt64(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int64,v)
    return *this;
}

SvStream& SvStream::operator<<  ( signed char v )
{
    //SDO
    if(bIoWrite && sizeof(signed char) <= nBufFree )
    {
        *pBufPos = v;
        pBufPos++; // sizeof(char);
        nBufActualPos++;
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--; // = sizeof(char);
        bIsDirty = true;
    }
    else
        Write( (char*)&v, sizeof(signed char) );
    return *this;
}

// Special treatment for Chars due to PutBack

SvStream& SvStream::operator<<  ( char v )
{
    //SDO
    if(bIoWrite && sizeof(char) <= nBufFree )
    {
        *pBufPos = v;
        pBufPos++; // sizeof(char);
        nBufActualPos++;
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--; // = sizeof(char);
        bIsDirty = true;
    }
    else
        Write( (char*)&v, sizeof(char) );
    return *this;
}

SvStream& SvStream::operator<<  ( unsigned char v )
{
//SDO
    if(bIoWrite && sizeof(char) <= nBufFree )
    {
        *(unsigned char*)pBufPos = v;
        pBufPos++; // = sizeof(char);
        nBufActualPos++; // = sizeof(char);
        if( nBufActualPos > nBufActualLen )  // Append ?
            nBufActualLen = nBufActualPos;
        nBufFree--;
        bIsDirty = true;
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
    {
        WRITENUMBER_WITHOUT_SWAP(double,r);
    }
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

OUString SvStream::ReadUniOrByteString( rtl_TextEncoding eSrcCharSet )
{
    // read UTF-16 string directly from stream ?
    if (eSrcCharSet == RTL_TEXTENCODING_UNICODE)
        return read_lenPrefixed_uInt16s_ToOUString<sal_uInt32>(*this);
    return read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(*this, eSrcCharSet);
}

SvStream& SvStream::WriteUniOrByteString( const OUString& rStr, rtl_TextEncoding eDestCharSet )
{
    // write UTF-16 string directly into stream ?
    if (eDestCharSet == RTL_TEXTENCODING_UNICODE)
        write_lenPrefixed_uInt16s_FromOUString<sal_uInt32>(*this, rStr);
    else
        write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(*this, rStr, eDestCharSet);
    return *this;
}

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
        // check if block is completely within buffer
        bIoRead = true;
        bIoWrite = false;
        if( nCount <= (sal_Size)(nBufActualLen - nBufActualPos ) )
        {
            // => yes
            memcpy(pData, pBufPos, (size_t) nCount);
            nBufActualPos = nBufActualPos + (sal_uInt16)nCount;
            pBufPos += nCount;
            nBufFree = nBufFree - (sal_uInt16)nCount;
        }
        else
        {
            if( bIsDirty ) // Does stream require a flush?
            {
                SeekPos( nBufFilePos );
                if( nCryptMask )
                    CryptAndWriteBuffer(pRWBuf, nBufActualLen);
                else
                    PutData( pRWBuf, nBufActualLen );
                bIsDirty = false;
            }

            // Does data block fit into buffer?
            if( nCount > nBufSize )
            {
                // => No! Thus read directly
                // into target area without using the buffer

                bIoRead = false;

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
                // => Yes. Fill buffer first, then copy to target area

                nBufFilePos += nBufActualPos;
                SeekPos( nBufFilePos );

                // TODO: Typecast before GetData, sal_uInt16 nCountTmp
                sal_Size nCountTmp = GetData( pRWBuf, nBufSize );
                if( nCryptMask )
                    EncryptBuffer(pRWBuf, nCountTmp);
                nBufActualLen = (sal_uInt16)nCountTmp;
                if( nCount > nCountTmp )
                {
                    nCount = nCountTmp;  // trim count back, EOF see below
                }
                memcpy( pData, pRWBuf, (size_t)nCount );
                nBufActualPos = (sal_uInt16)nCount;
                pBufPos = pRWBuf + nCount;
            }
        }
    }
    bIsEof = false;
    nBufFree = nBufActualLen - nBufActualPos;
    if( nCount != nSaveCount && nError != ERRCODE_IO_PENDING )
        bIsEof = true;
    if( nCount == nSaveCount && nError == ERRCODE_IO_PENDING )
        nError = ERRCODE_NONE;
    return nCount;
}

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
        RefreshBuffer();   // Remove changes in buffer through PutBack

    if( !pRWBuf )
    {
        if( nCryptMask )
            nCount = CryptAndWriteBuffer( pData, nCount );
        else
            nCount = PutData( (char*)pData, nCount );
        nBufFilePos += nCount;
        return nCount;
    }

    bIoRead = false;
    bIoWrite = true;
    if( nCount <= (sal_Size)(nBufSize - nBufActualPos) )
    {
        memcpy( pBufPos, pData, (size_t)nCount );
        nBufActualPos = nBufActualPos + (sal_uInt16)nCount;
        // Update length if buffer was updated
        if( nBufActualPos > nBufActualLen )
            nBufActualLen = nBufActualPos;

        pBufPos += nCount;
        bIsDirty = true;
    }
    else
    {
        // Does stream require flushing?
        if( bIsDirty )
        {
            SeekPos( nBufFilePos );
            if( nCryptMask )
                CryptAndWriteBuffer( pRWBuf, (sal_Size)nBufActualLen );
            else
                PutData( pRWBuf, nBufActualLen );
            bIsDirty = false;
        }

        // Does data block fit into buffer?
        if( nCount > nBufSize )
        {
            bIoWrite = false;
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
            // Copy block to buffer
            memcpy( pRWBuf, pData, (size_t)nCount );

            // Mind the order!
            nBufFilePos += nBufActualPos;
            nBufActualPos = (sal_uInt16)nCount;
            pBufPos = pRWBuf + nCount;
            nBufActualLen = (sal_uInt16)nCount;
            bIsDirty = true;
        }
    }
    nBufFree = nBufSize - nBufActualPos;
    return nCount;
}

sal_Size SvStream::Seek( sal_Size nFilePos )
{
    bIoRead = bIoWrite = false;
    bIsEof = false;
    if( !pRWBuf )
    {
        nBufFilePos = SeekPos( nFilePos );
        DBG_ASSERT(Tell()==nBufFilePos,"Out Of Sync!");
        return nBufFilePos;
    }

    // Is seek position within buffer?
    if( nFilePos >= nBufFilePos && nFilePos <= (nBufFilePos + nBufActualLen))
    {
        nBufActualPos = (sal_uInt16)(nFilePos - nBufFilePos);
        pBufPos = pRWBuf + nBufActualPos;
        // Update nBufFree to avoid crash upon PutBack
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
            bIsDirty = false;
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

//STREAM_SEEK_TO_END in the some of the Seek backends is special cased to be
//efficient, in others e.g. SotStorageStream it's really horribly slow, and in
//those this should be overridden
sal_Size SvStream::remainingSize()
{
    sal_Size nCurr = Tell();
    sal_Size nEnd = Seek(STREAM_SEEK_TO_END);
    sal_Size nMaxAvailable = nEnd-nCurr;
    Seek(nCurr);
    return nMaxAvailable;
}

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
        bIsDirty = false;
    }
    if( bIsWritable )
        FlushData();
}

void SvStream::RefreshBuffer()
{
    if( bIsDirty && bIsConsistent )
    {
        SeekPos( nBufFilePos );
        if( nCryptMask )
            CryptAndWriteBuffer( pRWBuf, (sal_Size)nBufActualLen );
        else
            PutData( pRWBuf, nBufActualLen );
        bIsDirty = false;
    }
    SeekPos( nBufFilePos );
    nBufActualLen = (sal_uInt16)GetData( pRWBuf, nBufSize );
    if( nBufActualLen && nError == ERRCODE_IO_PENDING )
        nError = ERRCODE_NONE;
    if( nCryptMask )
        EncryptBuffer(pRWBuf, (sal_Size)nBufActualLen);
    bIsConsistent = true;
    bIoRead = bIoWrite = false;
}

SvStream& SvStream::WriteNumber(sal_Int32 nInt32)
{
    char buffer[12];
    sal_Size nLen = sprintf(buffer, "%" SAL_PRIdINT32, nInt32);
    Write(buffer, nLen);
    return *this;
}

SvStream& SvStream::WriteNumber(sal_uInt32 nUInt32)
{
    char buffer[11];
    sal_Size nLen = sprintf(buffer, "%" SAL_PRIuUINT32, nUInt32);
    Write(buffer, nLen);
    return *this;
}

#define CRYPT_BUFSIZE 1024

/// Encrypt and write
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

bool SvStream::EncryptBuffer(void* pStart, sal_Size nLen)
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
    return true;
}

static unsigned char implGetCryptMask(const sal_Char* pStr, sal_Int32 nLen, long nVersion)
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

void SvStream::SetCryptMaskKey(const OString& rCryptMaskKey)
{
    m_aCryptMaskKey = rCryptMaskKey;
    nCryptMask = implGetCryptMask(m_aCryptMaskKey.getStr(),
        m_aCryptMaskKey.getLength(), GetVersion());
}

void SvStream::SyncSvStream( sal_Size nNewStreamPos )
{
    ClearBuffer();
    SvStream::nBufFilePos = nNewStreamPos;
}

void SvStream::SyncSysStream()
{
    Flush();
    SeekPos( Tell() );
}

bool SvStream::SetStreamSize( sal_Size nSize )
{
#ifdef DBG_UTIL
    sal_Size nFPos = Tell();
#endif
    sal_uInt16 nBuf = nBufSize;
    SetBufferSize( 0 );
    SetSize( nSize );
    SetBufferSize( nBuf );
    DBG_ASSERT(Tell()==nFPos,"SetStreamSize failed");
    return (nError == 0);
}

SvStream& endl( SvStream& rStr )
{
    LineEnd eDelim = rStr.GetLineDelimiter();
    if ( eDelim == LINEEND_CR )
        rStr << '\r';
    else if( eDelim == LINEEND_LF )
        rStr << '\n';
    else
        rStr << '\r' << '\n';
    return rStr;
}

SvStream& endlu( SvStream& rStrm )
{
    switch ( rStrm.GetLineDelimiter() )
    {
        case LINEEND_CR :
            rStrm << sal_Unicode('\r');
        break;
        case LINEEND_LF :
            rStrm << sal_Unicode('\n');
        break;
        default:
            rStrm << sal_Unicode('\r') << sal_Unicode('\n');
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

SvMemoryStream::SvMemoryStream( void* pBuffer, sal_Size bufSize,
                                StreamMode eMode )
{
    if( eMode & STREAM_WRITE )
        bIsWritable = true;
    else
        bIsWritable = false;
    nEndOfData  = bufSize;
    bOwnsData   = false;
    pBuf        = (sal_uInt8 *) pBuffer;
    nResize     = 0L;
    nSize       = bufSize;
    nPos        = 0L;
    SetBufferSize( 0 );
}

SvMemoryStream::SvMemoryStream( sal_Size nInitSize, sal_Size nResizeOffset )
{
    bIsWritable = true;
    bOwnsData   = true;
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

sal_uInt16 SvMemoryStream::IsA() const
{
    return (sal_uInt16)ID_MEMORYSTREAM;
}

const void* SvMemoryStream::GetBuffer()
{
    Flush();
    return (const void*)GetData();
}

sal_uIntPtr SvMemoryStream::GetSize()
{
    Flush();
    sal_uIntPtr nTemp = Tell();
    sal_uIntPtr nLength = Seek( STREAM_SEEK_TO_END );
    Seek( nTemp );
    return nLength;
}

void* SvMemoryStream::SetBuffer( void* pNewBuf, sal_Size nCount,
                                 bool bOwnsDat, sal_Size nEOF )
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

sal_Size SvMemoryStream::GetData( void* pData, sal_Size nCount )
{
    sal_Size nMaxCount = nEndOfData-nPos;
    if( nCount > nMaxCount )
        nCount = nMaxCount;
    memcpy( pData, pBuf+nPos, (size_t)nCount );
    nPos += nCount;
    return nCount;
}

sal_Size SvMemoryStream::PutData( const void* pData, sal_Size nCount )
{
    if( GetError() )
        return 0L;

    sal_Size nMaxCount = nSize-nPos;

    // check for overflow
    if( nCount > nMaxCount )
    {
        if( nResize == 0 )
        {
            // copy as much as possible
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
                // lacking memory is smaller than nResize,
                // resize accordingly
                if( !ReAllocateMemory( nNewResize) )
                {
                    nCount = 0;
                    SetError( SVSTREAM_WRITE_ERROR );
                }
            }
            else
            {
                // lacking memory is larger than nResize,
                // resize by (nCoount-nMaxCount) + resize offset
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

sal_Size SvMemoryStream::SeekPos( sal_Size nNewPos )
{
    // nEndOfData: First position in stream not allowed to read from
    // nSize: Size of allocated buffer

    if( nNewPos < nEndOfData )
        nPos = nNewPos;
    else if( nNewPos == STREAM_SEEK_TO_END )
        nPos = nEndOfData;
    else
    {
        if( nNewPos >= nSize ) // Does buffer need extension?
        {
            if( nResize )  // Is extension possible?
            {
                long nDiff = (long)(nNewPos - nSize + 1);
                nDiff += (long)nResize;
                ReAllocateMemory( nDiff );
                nPos = nNewPos;
                nEndOfData = nNewPos;
            }
            else  // Extension not possible, set pos to end of data
            {
                // SetError( SVSTREAM_OUTOFMEMORY );
                nPos = nEndOfData;
            }
        }
        else  // Expand buffer size
        {
            nPos = nNewPos;
            nEndOfData = nNewPos;
        }
    }
    return nPos;
}

void SvMemoryStream::FlushData()
{
}

void SvMemoryStream::ResetError()
{
    SvStream::ClearError();
}

bool SvMemoryStream::AllocateMemory( sal_Size nNewSize )
{
    pBuf = new sal_uInt8[nNewSize];
    return( pBuf != 0 );
}

// (using Bozo algorithm)
bool SvMemoryStream::ReAllocateMemory( long nDiff )
{
    bool bRetVal    = false;
    long nTemp      = (long)nSize;
    nTemp           += nDiff;
    sal_Size nNewSize  = (sal_Size)nTemp;

    if( nNewSize )
    {
        sal_uInt8* pNewBuf   = new sal_uInt8[nNewSize];

        if( pNewBuf )
        {
            bRetVal = true; // Success!
            if( nNewSize < nSize )      // Are we shrinking?
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
        bRetVal = true;
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

//Create a OString of nLen bytes from rStream
OString read_uInt8s_ToOString(SvStream& rStrm, sal_Size nLen)
{
    rtl_String *pStr = NULL;
    if (nLen)
    {
        nLen = std::min(nLen, static_cast<sal_Size>(SAL_MAX_INT32));
        //alloc a (ref-count 1) rtl_String of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_string_alloc(sal::static_int_cast<sal_Int32>(nLen));
        sal_Size nWasRead = rStrm.Read(pStr->buffer, nLen);
        if (nWasRead != nLen)
        {
            //on (typically unlikely) short read set length to what we could
            //read, and null terminate. Excess buffer capacity remains of
            //course, could create a (true) replacement OString if it matters.
            pStr->length = sal::static_int_cast<sal_Int32>(nWasRead);
            pStr->buffer[pStr->length] = 0;
        }
    }

    //take ownership of buffer and return, otherwise return empty string
    return pStr ? OString(pStr, SAL_NO_ACQUIRE) : OString();
}

//Create a OUString of nLen sal_Unicodes from rStream
OUString read_uInt16s_ToOUString(SvStream& rStrm, sal_Size nLen)
{
    rtl_uString *pStr = NULL;
    if (nLen)
    {
        nLen = std::min(nLen, static_cast<sal_Size>(SAL_MAX_INT32));
        //alloc a (ref-count 1) rtl_uString of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_uString_alloc(sal::static_int_cast<sal_Int32>(nLen));
        sal_Size nWasRead = rStrm.Read(pStr->buffer, nLen*2)/2;
        if (nWasRead != nLen)
        {
            //on (typically unlikely) short read set length to what we could
            //read, and null terminate. Excess buffer capacity remains of
            //course, could create a (true) replacement OUString if it matters.
            pStr->length = sal::static_int_cast<sal_Int32>(nWasRead);
            pStr->buffer[pStr->length] = 0;
        }
        if (rStrm.IsEndianSwap())
        {
            for (sal_Int32 i = 0; i < pStr->length; ++i)
                pStr->buffer[i] = OSL_SWAPWORD(pStr->buffer[i]);
        }
    }

    //take ownership of buffer and return, otherwise return empty string
    return pStr ? OUString(pStr, SAL_NO_ACQUIRE) : OUString();
}

namespace
{
    template <typename T, typename O> T tmpl_convertLineEnd(const T &rIn, LineEnd eLineEnd)
    {
        // Determine linebreaks and compute length
        bool            bConvert    = false;    // Needs conversion
        sal_Int32       nStrLen     = rIn.getLength();
        sal_Int32       nLineEndLen = (eLineEnd == LINEEND_CRLF) ? 2 : 1;
        sal_Int32       nLen        = 0;        // Target length
        sal_Int32       i           = 0;        // Source counter

        while (i < nStrLen)
        {
            // \r or \n causes linebreak
            if ( (rIn[i] == '\r') || (rIn[i] == '\n') )
            {
                nLen = nLen + nLineEndLen;

                // If set already, skip expensive test
                if ( !bConvert )
                {
                    // Muessen wir Konvertieren
                    if ( ((eLineEnd != LINEEND_LF) && (rIn[i] == '\n')) ||
                         ((eLineEnd == LINEEND_CRLF) && (rIn[i+1] != '\n')) ||
                         ((eLineEnd == LINEEND_LF) &&
                          ((rIn[i] == '\r') || (rIn[i+1] == '\r'))) ||
                         ((eLineEnd == LINEEND_CR) &&
                          ((rIn[i] == '\n') || (rIn[i+1] == '\n'))) )
                        bConvert = true;
                }

                // skip char if \r\n oder \n\r
                if ( ((rIn[i+1] == '\r') || (rIn[i+1] == '\n')) &&
                     (rIn[i] != rIn[i+1]) )
                    ++i;
            }
            else
                ++nLen;
            ++i;
        }

        if (!bConvert)
            return rIn;

        // convert linebreaks, insert string
        O aNewData(nLen);
        i = 0;
        while (i < nStrLen)
        {
            // \r or \n causes linebreak
            if ( (rIn[i] == '\r') || (rIn[i] == '\n') )
            {
                if ( eLineEnd == LINEEND_CRLF )
                {
                    aNewData.append('\r');
                    aNewData.append('\n');
                }
                else
                {
                    if ( eLineEnd == LINEEND_CR )
                        aNewData.append('\r');
                    else
                        aNewData.append('\n');
                }

                if ( ((rIn[i+1] == '\r') || (rIn[i+1] == '\n')) &&
                     (rIn[i] != rIn[i+1]) )
                    ++i;
            }
            else
            {
                aNewData.append(rIn[i]);
            }

            ++i;
        }

        return aNewData.makeStringAndClear();
    }
}

OString convertLineEnd(const OString &rIn, LineEnd eLineEnd)
{
    return tmpl_convertLineEnd<OString, OStringBuffer>(rIn, eLineEnd);
}

OUString convertLineEnd(const OUString &rIn, LineEnd eLineEnd)
{
    return tmpl_convertLineEnd<OUString, OUStringBuffer>(rIn, eLineEnd);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
