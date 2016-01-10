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

// TODO: Read->RefreshBuffer-> React to changes from m_nBufActualLen

#include <cstddef>

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <osl/endian.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>

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

// !!! Do not inline if already the operators <<,>> are inline
inline static void SwapUShort( sal_uInt16& r )
    {   r = OSL_SWAPWORD(r);   }
inline static void SwapShort( short& r )
    {   r = OSL_SWAPWORD(r);   }
inline static void SwapULong( sal_uInt32& r )
    {   r = OSL_SWAPDWORD(r);   }
inline static void SwapLongInt( sal_Int32& r )
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

static void SwapUnicode(sal_Unicode & r) { r = OSL_SWAPWORD(r); }

//SDO

#define READNUMBER_WITHOUT_SWAP(datatype,value) \
    if (m_isIoRead && sizeof(datatype) <= m_nBufFree)       \
    {                                                       \
        for (std::size_t i = 0; i < sizeof(datatype); i++)  \
            reinterpret_cast<char *>(&value)[i] = m_pBufPos[i]; \
        m_nBufActualPos += sizeof(datatype);                \
        m_pBufPos += sizeof(datatype);                      \
        m_nBufFree -= sizeof(datatype);                     \
    }                                                       \
    else                                                    \
    {                                                       \
        Read( &value, sizeof(datatype) );                   \
    }                                                       \


#define WRITENUMBER_WITHOUT_SWAP(datatype,value) \
    if (m_isIoWrite && sizeof(datatype) <= m_nBufFree)      \
    {                                                   \
        for (std::size_t i = 0; i < sizeof(datatype); i++)  \
            m_pBufPos[i] = reinterpret_cast<char const *>(&value)[i]; \
        m_nBufFree -= sizeof(datatype);                     \
        m_nBufActualPos += sizeof(datatype);                \
        if (m_nBufActualPos > m_nBufActualLen)              \
            m_nBufActualLen = m_nBufActualPos;              \
        m_pBufPos += sizeof(datatype);                      \
        m_isDirty = true;                                   \
    }                                                       \
    else                                                    \
    {                                                       \
        Write( &value, sizeof(datatype) );                  \
    }                                                       \

//  class SvLockBytes

void SvLockBytes::close()
{
    if (m_bOwner)
        delete m_pStream;
    m_pStream = nullptr;
}


// virtual
ErrCode SvLockBytes::ReadAt(sal_uInt64 const nPos, void * pBuffer, sal_Size nCount,
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
ErrCode SvLockBytes::WriteAt(sal_uInt64 const nPos, const void * pBuffer, sal_Size nCount,
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
ErrCode SvLockBytes::SetSize(sal_uInt64 const nSize)
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
        sal_uInt64 const nPos = m_pStream->Tell();
        pStat->nSize = m_pStream->Seek(STREAM_SEEK_TO_END);
        m_pStream->Seek(nPos);
    }
    return ERRCODE_NONE;
}

//  class SvOpenLockBytes


//  class SvAsyncLockBytes


// virtual
ErrCode SvAsyncLockBytes::ReadAt(sal_uInt64 const nPos, void * pBuffer, sal_Size nCount,
                                 sal_Size * pRead) const
{
    if (m_bTerminated)
        return SvOpenLockBytes::ReadAt(nPos, pBuffer, nCount, pRead);
    else
    {
        sal_Size nTheCount =
            std::min<sal_Size>(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
        ErrCode nError = SvOpenLockBytes::ReadAt(nPos, pBuffer, nTheCount,
                                                 pRead);
        return !nCount || nTheCount == nCount || nError ? nError :
                                                          ERRCODE_IO_PENDING;
    }
}

// virtual
ErrCode SvAsyncLockBytes::WriteAt(sal_uInt64 const nPos, const void * pBuffer,
                                  sal_Size nCount, sal_Size * pWritten)
{
    if (m_bTerminated)
        return SvOpenLockBytes::WriteAt(nPos, pBuffer, nCount, pWritten);
    else
    {
        sal_Size nTheCount =
            std::min<sal_Size>(nPos < m_nSize ? m_nSize - nPos : 0, nCount);
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
    sal_Size nTheWritten(0);
    ErrCode nError = SvOpenLockBytes::WriteAt(m_nSize, pBuffer, nCount,
                                              &nTheWritten);
    if (!nError)
        m_nSize += nTheWritten;
    if (pWritten)
        *pWritten = nTheWritten;
    return nError;
}

//  class SvStream

sal_Size SvStream::GetData( void* pData, sal_Size nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( m_xLockBytes.Is(), "pure virtual function" );
        sal_Size nRet(0);
        m_nError = m_xLockBytes->ReadAt(m_nActPos, pData, nSize, &nRet);
        m_nActPos += nRet;
        return nRet;
    }
    else return 0;
}

sal_Size SvStream::PutData( const void* pData, sal_Size nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( m_xLockBytes.Is(), "pure virtual function" );
        sal_Size nRet(0);
        m_nError = m_xLockBytes->WriteAt(m_nActPos, pData, nSize, &nRet);
        m_nActPos += nRet;
        return nRet;
    }
    else return 0;
}

sal_uInt64 SvStream::SeekPos(sal_uInt64 const nPos)
{
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nPos != SAL_MAX_UINT32);
    if( !GetError() && nPos == STREAM_SEEK_TO_END )
    {
        DBG_ASSERT( m_xLockBytes.Is(), "pure virtual function" );
        SvLockBytesStat aStat;
        m_xLockBytes->Stat( &aStat, SVSTATFLAG_DEFAULT );
        m_nActPos = aStat.nSize;
    }
    else
        m_nActPos = nPos;
    return m_nActPos;
}

void SvStream::FlushData()
{
    if( !GetError() )
    {
        DBG_ASSERT( m_xLockBytes.Is(), "pure virtual function" );
        m_nError = m_xLockBytes->Flush();
    }
}

void SvStream::SetSize(sal_uInt64 const nSize)
{
    DBG_ASSERT( m_xLockBytes.Is(), "pure virtual function" );
    m_nError = m_xLockBytes->SetSize( nSize );
}

void SvStream::ImpInit()
{
    m_nActPos           = 0;
    m_nCompressMode     = SvStreamCompressFlags::NONE;
    m_eStreamCharSet    = osl_getThreadTextEncoding();
    m_nCryptMask        = 0;
    m_isEof             = false;
#if defined UNX
    m_eLineDelimiter    = LINEEND_LF;   // UNIX-Format
#else
    m_eLineDelimiter    = LINEEND_CRLF; // DOS-Format
#endif

    SetEndian( SvStreamEndian::LITTLE );

    m_nBufFilePos       = 0;
    m_nBufActualPos     = 0;
    m_isDirty           = false;
    m_isConsistent      = true;
    m_isWritable        = true;

    m_pRWBuf            = nullptr;
    m_pBufPos           = nullptr;
    m_nBufSize          = 0;
    m_nBufActualLen     = 0;
    m_isIoRead          = false;
    m_isIoWrite         = false;
    m_nBufFree          = 0;

    m_eStreamMode       = StreamMode::NONE;

    m_nVersion          = 0;

    ClearError();
}

SvStream::SvStream( SvLockBytes* pLockBytesP )
{
    ImpInit();
    m_xLockBytes = pLockBytesP;
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
    ImpInit();
}

SvStream::~SvStream()
{
    if (m_xLockBytes.Is())
        Flush();

    delete[] m_pRWBuf;
}

void SvStream::ClearError()
{
    m_isEof = false;
    m_nError = SVSTREAM_OK;
}

void SvStream::SetError( sal_uInt32 nErrorCode )
{
    if (m_nError == SVSTREAM_OK)
        m_nError = nErrorCode;
}

void SvStream::SetEndian( SvStreamEndian nNewFormat )
{
    m_nEndian = nNewFormat;
    m_isSwap = false;
#ifdef OSL_BIGENDIAN
    if (m_nEndian == SvStreamEndian::LITTLE)
        m_isSwap = true;
#else
    if (m_nEndian == SvStreamEndian::BIG)
        m_isSwap = true;
#endif
}

void SvStream::SetBufferSize( sal_uInt16 nBufferSize )
{
    sal_uInt64 const nActualFilePos = Tell();
    bool bDontSeek = (m_pRWBuf == nullptr);

    if (m_isDirty && m_isConsistent && m_isWritable)  // due to Windows NT: Access denied
        Flush();

    if (m_nBufSize)
    {
        delete[] m_pRWBuf;
        m_nBufFilePos += m_nBufActualPos;
    }

    m_pRWBuf        = nullptr;
    m_nBufActualLen = 0;
    m_nBufActualPos = 0;
    m_nBufSize      = nBufferSize;
    if (m_nBufSize)
        m_pRWBuf = new sal_uInt8[ m_nBufSize ];
    m_isConsistent  = true;
    m_pBufPos       = m_pRWBuf;
    m_isIoRead = m_isIoWrite = false;
    if( !bDontSeek )
        SeekPos( nActualFilePos );
}

void SvStream::ClearBuffer()
{
    m_nBufActualLen = 0;
    m_nBufActualPos = 0;
    m_nBufFilePos   = 0;
    m_pBufPos       = m_pRWBuf;
    m_isDirty       = false;
    m_isConsistent  = true;
    m_isIoRead = m_isIoWrite = false;

    m_isEof         = false;
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
    sal_uInt64  nOldFilePos = Tell();
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
                m_isEof = true;
                rStr.clear();
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
        sal_Size nLen = Read(&cTemp , sizeof(cTemp) );
        if ( nLen ) {
            if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
                Seek( nOldFilePos );
        }
    }

    if ( bEnd )
        m_isEof = false;
    rStr = aBuf.makeStringAndClear();
    return bEnd;
}

bool SvStream::ReadUniStringLine( OUString& rStr, sal_Int32 nMaxCodepointsToRead )
{
    sal_Unicode buf[256+1];
    bool        bEnd        = false;
    sal_uInt64  nOldFilePos = Tell();
    sal_Unicode c           = 0;
    sal_Size       nTotalLen   = 0;

    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "ReadUniStringLine: swapping sizeof(sal_Unicode) not implemented" );

    OUStringBuffer aBuf(4096);
    while( !bEnd && !GetError() )   // Don't test for EOF as we
                                    // are reading block-wise!
    {
        sal_uInt16 nLen = (sal_uInt16)Read( buf, sizeof(buf)-sizeof(sal_Unicode) );
        nLen /= sizeof(sal_Unicode);
        if ( !nLen )
        {
            if ( aBuf.isEmpty() )
            {
                // exit on first BlockRead error
                m_isEof = true;
                rStr.clear();
                return false;
            }
            else
                break;
        }

        sal_uInt16 j, n;
        for( j = n = 0; j < nLen ; ++j )
        {
            if (m_isSwap)
                SwapUnicode( buf[n] );
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
        Read( &cTemp, sizeof(cTemp) );
        if (m_isSwap)
            SwapUnicode( cTemp );
        if( cTemp == c || (cTemp != '\n' && cTemp != '\r') )
            Seek( nOldFilePos );
    }

    if ( bEnd )
        m_isEof = false;
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
    sal_uInt64 nFilePos = rStream.Tell();

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
        nWritten = rStrm.Write( rStr.getStr(), nUnits * sizeof(sal_Unicode) );
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
            SwapUnicode( *p );
            p++;
        }
        nWritten = rStrm.Write( pTmp, nLen * sizeof(sal_Unicode) );
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
        return m_nError == SVSTREAM_OK;
    }
    else
    {
        OString aStr(OUStringToOString(rStr, eDestCharSet));
        write_uInt8s_FromOString(*this, aStr, aStr.getLength());
        return m_nError == SVSTREAM_OK;
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
    return m_nError == SVSTREAM_OK;
}

bool SvStream::WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        WriteUnicode(ch);
    else
    {
        OString aStr(&ch, 1, eDestCharSet);
        Write(aStr.getStr(), aStr.getLength());
    }
    return m_nError == SVSTREAM_OK;
}

bool SvStream::StartWritingUnicodeText()
{
    // BOM, Byte Order Mark, U+FEFF, see
    // http://www.unicode.org/faq/utf_bom.html#BOM
    // Upon read: 0xfeff(-257) => no swap; 0xfffe(-2) => swap
    sal_uInt16 v = 0xfeff;
    WRITENUMBER_WITHOUT_SWAP(sal_uInt16, v); // write native format
    return m_nError == SVSTREAM_OK;
}

bool SvStream::StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet )
{
    if (!(  eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
            eReadBomCharSet == RTL_TEXTENCODING_UNICODE ||
            eReadBomCharSet == RTL_TEXTENCODING_UTF8))
        return true;    // nothing to read

    bool bTryUtf8 = false;
    sal_uInt16 nFlag(0);
    sal_sSize nBack = sizeof(nFlag);
    this->ReadUInt16( nFlag );
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
                SetEndian( m_nEndian == SvStreamEndian::BIG ? SvStreamEndian::LITTLE : SvStreamEndian::BIG );
                nBack = 0;
            }
        break;
        case 0xefbb :
            if (m_nEndian == SvStreamEndian::BIG &&
                    (eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                     eReadBomCharSet == RTL_TEXTENCODING_UTF8))
                bTryUtf8 = true;
        break;
        case 0xbbef :
            if (m_nEndian == SvStreamEndian::LITTLE &&
                    (eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                     eReadBomCharSet == RTL_TEXTENCODING_UTF8))
                bTryUtf8 = true;
        break;
        default:
            ;   // nothing
    }
    if (bTryUtf8)
    {
        unsigned char nChar(0);
        nBack += sizeof(nChar);
        this->ReadUChar( nChar );
        if (nChar == 0xbf)
            nBack = 0;      // it is UTF-8
    }
    if (nBack)
        SeekRel( -nBack );      // no BOM, pure data
    return m_nError == SVSTREAM_OK;
}

sal_uInt64 SvStream::SeekRel(sal_Int64 const nPos)
{
    sal_uInt64 nActualPos = Tell();

    if ( nPos >= 0 )
    {
        if (SAL_MAX_UINT64 - nActualPos > static_cast<sal_uInt64>(nPos))
            nActualPos += nPos;
    }
    else
    {
        sal_uInt64 const nAbsPos = static_cast<sal_uInt64>(-nPos);
        if ( nActualPos >= nAbsPos )
            nActualPos -= nAbsPos;
    }

    m_pBufPos = m_pRWBuf + nActualPos;
    return Seek( nActualPos );
}

SvStream& SvStream::ReadUInt16(sal_uInt16& r)
{
    sal_uInt16 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt16, n)
    if (good())
    {
        if (m_isSwap)
            SwapUShort(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadUInt32(sal_uInt32& r)
{
    sal_uInt32 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt32, n)
    if (good())
    {
        if (m_isSwap)
            SwapULong(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadUInt64(sal_uInt64& r)
{
    sal_uInt64 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt64, n)
    if (good())
    {
        if (m_isSwap)
            SwapUInt64(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadInt16(sal_Int16& r)
{
    sal_Int16 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_Int16, n)
    if (good())
    {
        if (m_isSwap)
            SwapShort(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadInt32(sal_Int32& r)
{
    sal_Int32 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_Int32, n)
    if (good())
    {
        if (m_isSwap)
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
        if (m_isSwap)
            SwapInt64(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadSChar( signed char& r )
{
    if ((m_isIoRead || !m_isConsistent) &&
        sizeof(signed char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(signed char);
        m_pBufPos += sizeof(signed char);
        m_nBufFree -= sizeof(signed char);
    }
    else
        Read( &r, sizeof(signed char) );
    return *this;
}

// Special treatment for Chars due to PutBack

SvStream& SvStream::ReadChar( char& r )
{
    if ((m_isIoRead || !m_isConsistent) &&
        sizeof(char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(char);
        m_pBufPos += sizeof(char);
        m_nBufFree -= sizeof(char);
    }
    else
        Read( &r, sizeof(char) );
    return *this;
}

SvStream& SvStream::ReadUChar( unsigned char& r )
{
    if ((m_isIoRead || !m_isConsistent) &&
        sizeof(char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(char);
        m_pBufPos += sizeof(char);
        m_nBufFree -= sizeof(char);
    }
    else
        Read( &r, sizeof(char) );
    return *this;
}

SvStream& SvStream::ReadUtf16(sal_Unicode& r)
{
    sal_uInt16 n = 0;
    READNUMBER_WITHOUT_SWAP(sal_uInt16, n)
    if (good())
    {
        if (m_isSwap)
            SwapUShort(n);
        r = sal_Unicode(n);
    }
    return *this;
}

SvStream& SvStream::ReadCharAsBool( bool& r )
{
    if ((m_isIoRead || !m_isConsistent) &&
        sizeof(char) <= m_nBufFree)
    {
        SAL_WARN_IF(
            *m_pBufPos > 1, "tools.stream", unsigned(*m_pBufPos) << " not 0/1");
        r = *m_pBufPos != 0;
        m_nBufActualPos += sizeof(char);
        m_pBufPos += sizeof(char);
        m_nBufFree -= sizeof(char);
    }
    else
    {
        unsigned char c;
        if (Read(&c, 1) == 1)
        {
            SAL_WARN_IF(c > 1, "tools.stream", unsigned(c) << " not 0/1");
            r = c != 0;
        }
    }
    return *this;
}

SvStream& SvStream::ReadFloat(float& r)
{
    float n = 0;
    READNUMBER_WITHOUT_SWAP(float, n)
    if (good())
    {
#if defined UNX
        if (m_isSwap)
          SwapFloat(n);
#endif
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadDouble(double& r)
{
    double n = 0;
    READNUMBER_WITHOUT_SWAP(double, n)
    if (good())
    {
#if defined UNX
        if (m_isSwap)
          SwapDouble(n);
#endif
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadStream( SvStream& rStream )
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

SvStream& SvStream::WriteUInt16( sal_uInt16 v )
{
    if (m_isSwap)
        SwapUShort(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt16,v)
    return *this;
}

SvStream& SvStream::WriteUInt32( sal_uInt32 v )
{
    if (m_isSwap)
        SwapULong(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt32,v)
    return *this;
}

SvStream& SvStream::WriteUInt64( sal_uInt64 v )
{
    if (m_isSwap)
        SwapUInt64(v);
    WRITENUMBER_WITHOUT_SWAP(sal_uInt64,v)
    return *this;
}

SvStream& SvStream::WriteInt16( sal_Int16 v )
{
    if (m_isSwap)
        SwapShort(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int16,v)
    return *this;
}

SvStream& SvStream::WriteInt32( sal_Int32 v )
{
    if (m_isSwap)
        SwapLongInt(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int32,v)
    return *this;
}

SvStream& SvStream::WriteInt64  (sal_Int64 v)
{
    if (m_isSwap)
        SwapInt64(v);
    WRITENUMBER_WITHOUT_SWAP(sal_Int64,v)
    return *this;
}

SvStream& SvStream::WriteSChar( signed char v )
{
    //SDO
    if (m_isIoWrite && sizeof(signed char) <= m_nBufFree)
    {
        *m_pBufPos = v;
        m_pBufPos++; // sizeof(char);
        m_nBufActualPos++;
        if (m_nBufActualPos > m_nBufActualLen)  // Append ?
            m_nBufActualLen = m_nBufActualPos;
        m_nBufFree--; // = sizeof(char);
        m_isDirty = true;
    }
    else
        Write( &v, sizeof(signed char) );
    return *this;
}

// Special treatment for Chars due to PutBack

SvStream& SvStream::WriteChar( char v )
{
    //SDO
    if (m_isIoWrite && sizeof(char) <= m_nBufFree)
    {
        *m_pBufPos = v;
        m_pBufPos++; // sizeof(char);
        m_nBufActualPos++;
        if (m_nBufActualPos > m_nBufActualLen)  // Append ?
            m_nBufActualLen = m_nBufActualPos;
        m_nBufFree--; // = sizeof(char);
        m_isDirty = true;
    }
    else
        Write( &v, sizeof(char) );
    return *this;
}

SvStream& SvStream::WriteUChar( unsigned char v )
{
//SDO
    if (m_isIoWrite && sizeof(char) <= m_nBufFree)
    {
        *reinterpret_cast<unsigned char*>(m_pBufPos) = v;
        m_pBufPos++; // = sizeof(char);
        m_nBufActualPos++; // = sizeof(char);
        if (m_nBufActualPos > m_nBufActualLen)  // Append ?
            m_nBufActualLen = m_nBufActualPos;
        m_nBufFree--;
        m_isDirty = true;
    }
    else
        Write( &v, sizeof(char) );
    return *this;
}

SvStream& SvStream::WriteUInt8( sal_uInt8 v )
{
    return WriteUChar(v);
}

SvStream& SvStream::WriteUnicode( sal_Unicode v )
{
    return WriteUInt16(v);
}

SvStream& SvStream::WriteFloat( float v )
{
#ifdef UNX
    if (m_isSwap)
      SwapFloat(v);
#endif
    WRITENUMBER_WITHOUT_SWAP(float,v)
    return *this;
}

SvStream& SvStream::WriteDouble ( const double& r )
{
#if defined UNX
    if (m_isSwap)
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

SvStream& SvStream::WriteCharPtr( const char* pBuf )
{
    Write( pBuf, strlen( pBuf ) );
    return *this;
}

SvStream& SvStream::WriteStream( SvStream& rStream )
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
        return read_uInt32_lenPrefixed_uInt16s_ToOUString(*this);
    return read_uInt16_lenPrefixed_uInt8s_ToOUString(*this, eSrcCharSet);
}

SvStream& SvStream::WriteUniOrByteString( const OUString& rStr, rtl_TextEncoding eDestCharSet )
{
    // write UTF-16 string directly into stream ?
    if (eDestCharSet == RTL_TEXTENCODING_UNICODE)
        write_uInt32_lenPrefixed_uInt16s_FromOUString(*this, rStr);
    else
        write_uInt16_lenPrefixed_uInt8s_FromOUString(*this, rStr, eDestCharSet);
    return *this;
}

sal_Size SvStream::Read( void* pData, sal_Size nCount )
{
    sal_Size nSaveCount = nCount;
    if (!m_isConsistent)
        RefreshBuffer();

    if (!m_pRWBuf)
    {
        nCount = GetData( pData,nCount);
        if (m_nCryptMask)
            EncryptBuffer(pData, nCount);
        m_nBufFilePos += nCount;
    }
    else
    {
        // check if block is completely within buffer
        m_isIoRead = true;
        m_isIoWrite = false;
        if (nCount <= static_cast<sal_Size>(m_nBufActualLen - m_nBufActualPos))
        {
            // => yes
            memcpy(pData, m_pBufPos, (size_t) nCount);
            m_nBufActualPos = m_nBufActualPos + (sal_uInt16)nCount;
            m_pBufPos += nCount;
            m_nBufFree = m_nBufFree - (sal_uInt16)nCount;
        }
        else
        {
            if (m_isDirty) // Does stream require a flush?
            {
                SeekPos(m_nBufFilePos);
                if (m_nCryptMask)
                    CryptAndWriteBuffer(m_pRWBuf, m_nBufActualLen);
                else
                    PutData(m_pRWBuf, m_nBufActualLen);
                m_isDirty = false;
            }

            // Does data block fit into buffer?
            if (nCount > m_nBufSize)
            {
                // => No! Thus read directly
                // into target area without using the buffer

                m_isIoRead = false;

                SeekPos(m_nBufFilePos + m_nBufActualPos);
                m_nBufActualLen = 0;
                m_pBufPos     = m_pRWBuf;
                nCount = GetData( pData, nCount );
                if (m_nCryptMask)
                    EncryptBuffer(pData, nCount);
                m_nBufFilePos += nCount;
                m_nBufFilePos += m_nBufActualPos;
                m_nBufActualPos = 0;
            }
            else
            {
                // => Yes. Fill buffer first, then copy to target area

                m_nBufFilePos += m_nBufActualPos;
                SeekPos(m_nBufFilePos);

                // TODO: Typecast before GetData, sal_uInt16 nCountTmp
                sal_Size nCountTmp = GetData( m_pRWBuf, m_nBufSize );
                if (m_nCryptMask)
                    EncryptBuffer(m_pRWBuf, nCountTmp);
                m_nBufActualLen = (sal_uInt16)nCountTmp;
                if( nCount > nCountTmp )
                {
                    nCount = nCountTmp;  // trim count back, EOF see below
                }
                memcpy( pData, m_pRWBuf, (size_t)nCount );
                m_nBufActualPos = (sal_uInt16)nCount;
                m_pBufPos = m_pRWBuf + nCount;
            }
        }
    }
    m_isEof = false;
    m_nBufFree = m_nBufActualLen - m_nBufActualPos;
    if (nCount != nSaveCount && m_nError != ERRCODE_IO_PENDING)
        m_isEof = true;
    if (nCount == nSaveCount && m_nError == ERRCODE_IO_PENDING)
        m_nError = ERRCODE_NONE;
    return nCount;
}

sal_Size SvStream::Write( const void* pData, sal_Size nCount )
{
    if( !nCount )
        return 0;
    if (!m_isWritable)
    {
        SetError( ERRCODE_IO_CANTWRITE );
        return 0;
    }
    if (!m_isConsistent)
        RefreshBuffer();   // Remove changes in buffer through PutBack

    if (!m_pRWBuf)
    {
        if (m_nCryptMask)
            nCount = CryptAndWriteBuffer( pData, nCount );
        else
            nCount = PutData( pData, nCount );
        m_nBufFilePos += nCount;
        return nCount;
    }

    m_isIoRead = false;
    m_isIoWrite = true;
    if (nCount <= static_cast<sal_Size>(m_nBufSize - m_nBufActualPos))
    {
        memcpy( m_pBufPos, pData, (size_t)nCount );
        m_nBufActualPos = m_nBufActualPos + (sal_uInt16)nCount;
        // Update length if buffer was updated
        if (m_nBufActualPos > m_nBufActualLen)
            m_nBufActualLen = m_nBufActualPos;

        m_pBufPos += nCount;
        m_isDirty = true;
    }
    else
    {
        // Does stream require flushing?
        if (m_isDirty)
        {
            SeekPos(m_nBufFilePos);
            if (m_nCryptMask)
                CryptAndWriteBuffer( m_pRWBuf, (sal_Size)m_nBufActualLen );
            else
                PutData( m_pRWBuf, m_nBufActualLen );
            m_isDirty = false;
        }

        // Does data block fit into buffer?
        if (nCount > m_nBufSize)
        {
            m_isIoWrite = false;
            m_nBufFilePos += m_nBufActualPos;
            m_nBufActualLen = 0;
            m_nBufActualPos = 0;
            m_pBufPos     = m_pRWBuf;
            SeekPos(m_nBufFilePos);
            if (m_nCryptMask)
                nCount = CryptAndWriteBuffer( pData, nCount );
            else
                nCount = PutData( pData, nCount );
            m_nBufFilePos += nCount;
        }
        else
        {
            // Copy block to buffer
            memcpy( m_pRWBuf, pData, (size_t)nCount );

            // Mind the order!
            m_nBufFilePos += m_nBufActualPos;
            m_nBufActualPos = (sal_uInt16)nCount;
            m_pBufPos = m_pRWBuf + nCount;
            m_nBufActualLen = (sal_uInt16)nCount;
            m_isDirty = true;
        }
    }
    m_nBufFree = m_nBufSize - m_nBufActualPos;
    return nCount;
}

sal_uInt64 SvStream::Seek(sal_uInt64 const nFilePos)
{
    m_isIoRead = m_isIoWrite = false;
    m_isEof = false;
    if (!m_pRWBuf)
    {
        m_nBufFilePos = SeekPos( nFilePos );
        DBG_ASSERT(Tell() == m_nBufFilePos,"Out Of Sync!");
        return m_nBufFilePos;
    }

    // Is seek position within buffer?
    if (nFilePos >= m_nBufFilePos && nFilePos <= (m_nBufFilePos + m_nBufActualLen))
    {
        m_nBufActualPos = (sal_uInt16)(nFilePos - m_nBufFilePos);
        m_pBufPos = m_pRWBuf + m_nBufActualPos;
        // Update m_nBufFree to avoid crash upon PutBack
        m_nBufFree = m_nBufActualLen - m_nBufActualPos;
    }
    else
    {
        if (m_isDirty && m_isConsistent)
        {
            SeekPos(m_nBufFilePos);
            if (m_nCryptMask)
                CryptAndWriteBuffer( m_pRWBuf, m_nBufActualLen );
            else
                PutData( m_pRWBuf, m_nBufActualLen );
            m_isDirty = false;
        }
        m_nBufActualLen = 0;
        m_nBufActualPos = 0;
        m_pBufPos     = m_pRWBuf;
        m_nBufFilePos = SeekPos( nFilePos );
    }
    return m_nBufFilePos + m_nBufActualPos;
}

//STREAM_SEEK_TO_END in the some of the Seek backends is special cased to be
//efficient, in others e.g. SotStorageStream it's really horribly slow, and in
//those this should be overridden
sal_uInt64 SvStream::remainingSize()
{
    sal_uInt64 const nCurr = Tell();
    sal_uInt64 const nEnd = Seek(STREAM_SEEK_TO_END);
    sal_uInt64 nMaxAvailable = nEnd > nCurr ? (nEnd-nCurr) : 0;
    Seek(nCurr);
    return nMaxAvailable;
}

void SvStream::Flush()
{
    if (m_isDirty && m_isConsistent)
    {
        SeekPos(m_nBufFilePos);
        if (m_nCryptMask)
            CryptAndWriteBuffer( m_pRWBuf, (sal_Size)m_nBufActualLen );
        else
            if (PutData( m_pRWBuf, m_nBufActualLen ) != m_nBufActualLen)
                SetError( SVSTREAM_WRITE_ERROR );
        m_isDirty = false;
    }
    if (m_isWritable)
        FlushData();
}

void SvStream::RefreshBuffer()
{
    if (m_isDirty && m_isConsistent)
    {
        SeekPos(m_nBufFilePos);
        if (m_nCryptMask)
            CryptAndWriteBuffer( m_pRWBuf, (sal_Size)m_nBufActualLen );
        else
            PutData( m_pRWBuf, m_nBufActualLen );
        m_isDirty = false;
    }
    SeekPos(m_nBufFilePos);
    m_nBufActualLen = (sal_uInt16)GetData( m_pRWBuf, m_nBufSize );
    if (m_nBufActualLen && m_nError == ERRCODE_IO_PENDING)
        m_nError = ERRCODE_NONE;
    if (m_nCryptMask)
        EncryptBuffer(m_pRWBuf, (sal_Size)m_nBufActualLen);
    m_isConsistent = true;
    m_isIoRead = m_isIoWrite = false;
}

SvStream& SvStream::WriteInt32AsString(sal_Int32 nInt32)
{
    char buffer[12];
    sal_Size nLen = sprintf(buffer, "%" SAL_PRIdINT32, nInt32);
    Write(buffer, nLen);
    return *this;
}

SvStream& SvStream::WriteUInt32AsString(sal_uInt32 nUInt32)
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
    unsigned char const * pDataPtr = static_cast<unsigned char const *>(pStart);
    sal_Size nCount = 0;
    sal_Size nBufCount;
    unsigned char nMask = m_nCryptMask;
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
        nCount += PutData( pTemp, nBufCount );
        pDataPtr += nBufCount;
    }
    while ( nLen );
    return nCount;
}

bool SvStream::EncryptBuffer(void* pStart, sal_Size nLen)
{
    unsigned char* pTemp = static_cast<unsigned char*>(pStart);
    unsigned char nMask = m_nCryptMask;

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
        for( sal_Int32 i = 0; i < nLen; i++ ) {
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
    m_nCryptMask = implGetCryptMask(m_aCryptMaskKey.getStr(),
        m_aCryptMaskKey.getLength(), GetVersion());
}

void SvStream::SyncSvStream( sal_Size nNewStreamPos )
{
    ClearBuffer();
    SvStream::m_nBufFilePos = nNewStreamPos;
}

void SvStream::SyncSysStream()
{
    Flush();
    SeekPos( Tell() );
}

bool SvStream::SetStreamSize(sal_uInt64 const nSize)
{
#ifdef DBG_UTIL
    sal_uInt64 nFPos = Tell();
#endif
    sal_uInt16 nBuf = m_nBufSize;
    SetBufferSize( 0 );
    SetSize( nSize );
    SetBufferSize( nBuf );
#ifdef DBG_UTIL
    DBG_ASSERT(Tell()==nFPos,"SetStreamSize failed");
#endif
    return (m_nError == 0);
}

SvStream& endl( SvStream& rStr )
{
    LineEnd eDelim = rStr.GetLineDelimiter();
    if ( eDelim == LINEEND_CR )
        rStr.WriteChar('\r');
    else if( eDelim == LINEEND_LF )
        rStr.WriteChar('\n');
    else
        rStr.WriteChar('\r').WriteChar('\n');
    return rStr;
}

SvStream& endlu( SvStream& rStrm )
{
    switch ( rStrm.GetLineDelimiter() )
    {
        case LINEEND_CR :
            rStrm.WriteUnicode('\r');
        break;
        case LINEEND_LF :
            rStrm.WriteUnicode('\n');
        break;
        default:
            rStrm.WriteUnicode('\r').WriteUnicode('\n');
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
    if( eMode & StreamMode::WRITE )
        m_isWritable = true;
    else
        m_isWritable = false;
    nEndOfData  = bufSize;
    bOwnsData   = false;
    pBuf        = static_cast<sal_uInt8 *>(pBuffer);
    nResize     = 0L;
    nSize       = bufSize;
    nPos        = 0L;
    SetBufferSize( 0 );
}

SvMemoryStream::SvMemoryStream( sal_Size nInitSize, sal_Size nResizeOffset )
{
    m_isWritable = true;
    bOwnsData   = true;
    nEndOfData  = 0L;
    nResize     = nResizeOffset;
    nPos        = 0;
    pBuf        = nullptr;
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

const void* SvMemoryStream::GetBuffer()
{
    return GetData();
}

sal_uInt64 SvMemoryStream::GetSize()
{
    Flush();
    sal_uInt64 const nTemp = Tell();
    sal_uInt64 const nLength = Seek( STREAM_SEEK_TO_END );
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
        pResult = nullptr;
        if( pNewBuf != pBuf )
            FreeMemory();
    }
    else
        pResult = pBuf;

    pBuf        = static_cast<sal_uInt8 *>(pNewBuf);
    nPos        = 0;
    nSize       = nCount;
    nResize     = 0;
    bOwnsData   = bOwnsDat;

    if( nEOF > nCount )
        nEOF = nCount;
    nEndOfData = nEOF;

    ResetError();

    return pResult;
}

sal_Size SvMemoryStream::GetData( void* pData, sal_Size nCount )
{
    sal_Size nMaxCount = nEndOfData-nPos;
    if( nCount > nMaxCount )
        nCount = nMaxCount;
    if (nCount != 0)
    {
        memcpy( pData, pBuf+nPos, (size_t)nCount );
    }
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
                // resize by (nCount-nMaxCount) + resize offset
                if( !ReAllocateMemory( nCount-nMaxCount+nNewResize ) )
                {
                    nCount = 0;
                    SetError( SVSTREAM_WRITE_ERROR );
                }
            }
        }
    }
    assert(pBuf && "Possibly Reallocate failed");
    memcpy( pBuf+nPos, pData, (size_t)nCount);

    nPos += nCount;
    if( nPos > nEndOfData )
        nEndOfData = nPos;
    return nCount;
}

sal_uInt64 SvMemoryStream::SeekPos(sal_uInt64 const nNewPos)
{
    // nEndOfData: First position in stream not allowed to read from
    // nSize: Size of allocated buffer

    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nNewPos != SAL_MAX_UINT32);
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
    return( pBuf != nullptr );
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
            if (nSize != 0)
            {
                memcpy( pNewBuf, pBuf, (size_t)nSize );
            }
            memset(pNewBuf + nSize, 0x00, nNewSize - nSize);
        }

        FreeMemory();

        pBuf  = pNewBuf;
        nSize = nNewSize;
    }
    else
    {
        bRetVal = true;
        FreeMemory();
        pBuf = nullptr;
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
        return nullptr;
    Seek( STREAM_SEEK_TO_BEGIN );

    void* pRetVal = pBuf;
    pBuf          = nullptr;
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

void SvMemoryStream::SetSize(sal_uInt64 const nNewSize)
{
    long nDiff = (long)nNewSize - (long)nSize;
    ReAllocateMemory( nDiff );
}

SvScriptStream::SvScriptStream(const OUString& rUrl):
    mpProcess(nullptr), mpHandle(nullptr)
{
    oslProcessError rc;
    rc = osl_executeProcess_WithRedirectedIO(
        rUrl.pData,
        nullptr, 0,
        osl_Process_HIDDEN,
        nullptr,
        nullptr,
        nullptr, 0,
        &mpProcess,
        nullptr, &mpHandle, nullptr);
    if (osl_Process_E_None != rc)
    {
        mpProcess = nullptr;
        mpHandle = nullptr;
    }
}

SvScriptStream::~SvScriptStream()
{
    if (mpProcess)
    {
        osl_terminateProcess(mpProcess);
        osl_freeProcessHandle(mpProcess);
    }
    if (mpHandle)
        osl_closeFile(mpHandle);
}

bool SvScriptStream::ReadLine(OString &rStr, sal_Int32)
{
    rStr.clear();
    if (!good())
        return false;

    OStringBuffer sBuf;
    sal_Char aChar('\n');
    sal_uInt64 nBytesRead;
    while (osl_File_E_None == osl_readFile(mpHandle, &aChar, 1, &nBytesRead)
            && nBytesRead == 1 && aChar != '\n')
    {
        sBuf.append( aChar );
    }
    rStr = sBuf.makeStringAndClear();
    if (!rStr.isEmpty())
        return true;

    return false;
}

bool SvScriptStream::good() const
{
    return mpHandle != nullptr;
}

//Create a OString of nLen bytes from rStream
OString read_uInt8s_ToOString(SvStream& rStrm, sal_Size nLen)
{
    rtl_String *pStr = nullptr;
    if (nLen)
    {
        nLen = std::min(nLen, static_cast<sal_Size>(SAL_MAX_INT32));
        //alloc a (ref-count 1) rtl_String of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_string_alloc(sal::static_int_cast<sal_Int32>(nLen));
        SAL_WARN_IF(!pStr, "tools", "allocation failed");
        if (pStr)
        {
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
    }

    //take ownership of buffer and return, otherwise return empty string
    return pStr ? OString(pStr, SAL_NO_ACQUIRE) : OString();
}

//Create a OUString of nLen sal_Unicodes from rStream
OUString read_uInt16s_ToOUString(SvStream& rStrm, sal_Size nLen)
{
    rtl_uString *pStr = nullptr;
    if (nLen)
    {
        nLen = std::min(nLen, static_cast<sal_Size>(SAL_MAX_INT32));
        //alloc a (ref-count 1) rtl_uString of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_uString_alloc(sal::static_int_cast<sal_Int32>(nLen));
        SAL_WARN_IF(!pStr, "tools", "allocation failed");
        if (pStr)
        {
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
                         ((eLineEnd == LINEEND_CRLF) && (i+1) < nStrLen && (rIn[i+1] != '\n')) ||
                         ((eLineEnd == LINEEND_LF) &&
                          ((rIn[i] == '\r') || ((i+1) < nStrLen && rIn[i+1] == '\r'))) ||
                         ((eLineEnd == LINEEND_CR) &&
                          ((rIn[i] == '\n') || ((i+1) < nStrLen && rIn[i+1] == '\n'))) )
                        bConvert = true;
                }

                // skip char if \r\n or \n\r
                if ( (i+1) < nStrLen && ((rIn[i+1] == '\r') || (rIn[i+1] == '\n')) &&
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

                if ( (i+1) < nStrLen && ((rIn[i+1] == '\r') || (rIn[i+1] == '\n')) &&
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

sal_Size write_uInt32_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                const OUString &rStr)
{
    sal_Size nWritten = 0;
    sal_uInt32 nUnits = std::min<sal_Size>(rStr.getLength(), std::numeric_limits<sal_uInt32>::max());
    SAL_WARN_IF(static_cast<sal_Size>(nUnits) != static_cast<sal_Size>(rStr.getLength()),
        "tools.stream",
        "string too long for prefix count to fit in output type");
    rStrm.WriteUInt32(nUnits);
    if (rStrm.good())
    {
        nWritten += sizeof(sal_uInt32);
        nWritten += write_uInt16s_FromOUString(rStrm, rStr, nUnits);
    }
    return nWritten;
}

sal_Size write_uInt16_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                const OUString &rStr)
{
    sal_Size nWritten = 0;
    sal_uInt16 nUnits = std::min<sal_Size>(rStr.getLength(), std::numeric_limits<sal_uInt16>::max());
    SAL_WARN_IF(nUnits != rStr.getLength(),
        "tools.stream",
        "string too long for prefix count to fit in output type");
    rStrm.WriteUInt16(nUnits);
    if (rStrm.good())
    {
        nWritten += sizeof(nUnits);
        nWritten += write_uInt16s_FromOUString(rStrm, rStr, nUnits);
    }
    return nWritten;
}

sal_Size write_uInt16_lenPrefixed_uInt8s_FromOString(SvStream& rStrm,
                                              const OString &rStr)
{
    sal_Size nWritten = 0;
    sal_uInt16 nUnits = std::min<sal_Size>(rStr.getLength(), std::numeric_limits<sal_uInt16>::max());
    SAL_WARN_IF(static_cast<sal_Size>(nUnits) != static_cast<sal_Size>(rStr.getLength()),
        "tools.stream",
        "string too long for sal_uInt16 count to fit in output type");
    rStrm.WriteUInt16( nUnits );
    if (rStrm.good())
    {
        nWritten += sizeof(sal_uInt16);
        nWritten += write_uInt8s_FromOString(rStrm, rStr, nUnits);
    }
    return nWritten;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
