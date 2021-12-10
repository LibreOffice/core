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

#include <sal/config.h>

#include <cassert>
#include <cstddef>
#include <memory>

#include <string.h>
#include <stdio.h>

#include <o3tl/safeint.hxx>
#include <osl/endian.h>
#include <osl/diagnose.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/long.hxx>

#include <comphelper/fileformat.h>
#include <comphelper/fileurl.hxx>

static void swapNibbles(unsigned char &c)
{
    unsigned char nSwapTmp=c;
    nSwapTmp <<= 4;
    c >>= 4;
    c |= nSwapTmp;
}

#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <osl/thread.h>
#include <algorithm>

// !!! Do not inline if already the operators <<,>> are inline
template <typename T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 2, int> = 0>
static void SwapNumber(T& r)
    {   r = OSL_SWAPWORD(r);   }
template <typename T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4, int> = 0>
static void SwapNumber(T& r)
    {   r = OSL_SWAPDWORD(r);   }
template <typename T, std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8, int> = 0>
static void SwapNumber(T& r)
    {
        union
        {
            T n;
            sal_uInt32 c[2];
        } s;

        s.n = r;
        std::swap(s.c[0], s.c[1]); // swap the 32 bit words
        // swap the bytes in the words
        s.c[0] = OSL_SWAPDWORD(s.c[0]);
        s.c[1] = OSL_SWAPDWORD(s.c[1]);
        r = s.n;
    }

#ifdef UNX
static void SwapFloat( float& r )
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

static void SwapDouble( double& r )
    {
        if( sizeof(double) != 8 )
        {
          SAL_WARN( "tools.stream", "Can only swap 8-Byte-doubles" );
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

void SvStream::readNumberWithoutSwap_(void * pDataDest, int nDataSize)
{
    if (m_isIoRead && nDataSize <= m_nBufFree)
    {
        for (int i = 0; i < nDataSize; i++)
            static_cast<char*>(pDataDest)[i] = m_pBufPos[i];
        m_nBufActualPos += nDataSize;
        m_pBufPos += nDataSize;
        m_nBufFree -= nDataSize;
    }
    else
    {
        ReadBytes( pDataDest, nDataSize );
    }
}


void SvStream::writeNumberWithoutSwap_(const void * pDataSrc, int nDataSize)
{
    if (m_isIoWrite && nDataSize <= m_nBufFree)
    {
        for (int i = 0; i < nDataSize; i++)
            m_pBufPos[i] = static_cast<const char*>(pDataSrc)[i];
        m_nBufFree -= nDataSize;
        m_nBufActualPos += nDataSize;
        if (m_nBufActualPos > m_nBufActualLen)
            m_nBufActualLen = m_nBufActualPos;
        m_pBufPos += nDataSize;
        m_isDirty = true;
    }
    else
    {
        WriteBytes( pDataSrc, nDataSize );
    }
}


void SvLockBytes::close()
{
    if (m_bOwner)
        delete m_pStream;
    m_pStream = nullptr;
}


// virtual
ErrCode SvLockBytes::ReadAt(sal_uInt64 const nPos, void * pBuffer, std::size_t nCount,
                            std::size_t * pRead) const
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::ReadAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    std::size_t nTheRead = m_pStream->ReadBytes(pBuffer, nCount);
    if (pRead)
        *pRead = nTheRead;
    return m_pStream->GetErrorCode();
}

// virtual
ErrCode SvLockBytes::WriteAt(sal_uInt64 const nPos, const void * pBuffer, std::size_t nCount,
                             std::size_t * pWritten)
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::WriteAt(): Bad stream");
        return ERRCODE_NONE;
    }

    m_pStream->Seek(nPos);
    std::size_t nTheWritten = m_pStream->WriteBytes(pBuffer, nCount);
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

ErrCode SvLockBytes::Stat(SvLockBytesStat * pStat) const
{
    if (!m_pStream)
    {
        OSL_FAIL("SvLockBytes::Stat(): Bad stream");
        return ERRCODE_NONE;
    }

    if (pStat)
        pStat->nSize = m_pStream->TellEnd();
    return ERRCODE_NONE;
}


std::size_t SvStream::GetData( void* pData, std::size_t nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( m_xLockBytes.is(), "pure virtual function" );
        std::size_t nRet(0);
        m_nError = m_xLockBytes->ReadAt(m_nActPos, pData, nSize, &nRet);
        m_nActPos += nRet;
        return nRet;
    }
    else return 0;
}

std::size_t SvStream::PutData( const void* pData, std::size_t nSize )
{
    if( !GetError() )
    {
        DBG_ASSERT( m_xLockBytes.is(), "pure virtual function" );
        std::size_t nRet(0);
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
        DBG_ASSERT( m_xLockBytes.is(), "pure virtual function" );
        SvLockBytesStat aStat;
        m_xLockBytes->Stat( &aStat );
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
        DBG_ASSERT( m_xLockBytes.is(), "pure virtual function" );
        m_nError = m_xLockBytes->Flush();
    }
}

void SvStream::SetSize(sal_uInt64 const nSize)
{
    DBG_ASSERT( m_xLockBytes.is(), "pure virtual function" );
    m_nError = m_xLockBytes->SetSize( nSize );
}

SvStream::SvStream() :
     m_nActPos(0)

   , m_pBufPos(nullptr)
   , m_nBufSize(0)
   , m_nBufActualLen(0)
   , m_nBufActualPos(0)
   , m_nBufFree(0)
   , m_isIoRead(false)
   , m_isIoWrite(false)

   , m_isDirty(false)
   , m_isEof(false)

   , m_nCompressMode(SvStreamCompressFlags::NONE)
#if defined UNX
   , m_eLineDelimiter(LINEEND_LF)   // UNIX-Format
#else
   , m_eLineDelimiter(LINEEND_CRLF) // DOS-Format
#endif
   , m_eStreamCharSet(osl_getThreadTextEncoding())

   , m_nCryptMask(0)

   , m_nVersion(0)

   , m_nBufFilePos(0)
   , m_eStreamMode(StreamMode::NONE)
   , m_isWritable(true)

{
    SetEndian( SvStreamEndian::LITTLE );

    ClearError();
}

SvStream::SvStream( SvLockBytes* pLockBytesP ) : SvStream()
{
    m_xLockBytes = pLockBytesP;
    if( pLockBytesP ) {
        const SvStream* pStrm = pLockBytesP->GetStream();
        if( pStrm ) {
            SetError( pStrm->GetErrorCode() );
        }
    }
    SetBufferSize( 256 );
}

SvStream::~SvStream()
{
    if (m_xLockBytes.is())
        Flush();
}

void SvStream::ClearError()
{
    m_isEof = false;
    m_nError = ERRCODE_NONE;
}

void SvStream::SetError( ErrCode nErrorCode )
{
    if (m_nError == ERRCODE_NONE)
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

    if (m_isDirty && m_isWritable)  // due to Windows NT: Access denied
        Flush();

    if (m_nBufSize)
    {
        m_pRWBuf.reset();
        m_nBufFilePos += m_nBufActualPos;
    }

    m_pRWBuf        = nullptr;
    m_nBufActualLen = 0;
    m_nBufActualPos = 0;
    m_nBufSize      = nBufferSize;
    if (m_nBufSize)
        m_pRWBuf.reset(new sal_uInt8[ m_nBufSize ]);
    m_pBufPos       = m_pRWBuf.get();
    m_isIoRead = m_isIoWrite = false;
    if( !bDontSeek )
        SeekPos( nActualFilePos );
}

void SvStream::ClearBuffer()
{
    m_nBufActualLen = 0;
    m_nBufActualPos = 0;
    m_nBufFilePos   = 0;
    m_pBufPos       = m_pRWBuf.get();
    m_isDirty       = false;
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
    char    buf[256+1];
    bool        bEnd        = false;
    sal_uInt64  nOldFilePos = Tell();
    char    c           = 0;
    std::size_t nTotalLen   = 0;

    OStringBuffer aBuf(4096);
    while( !bEnd && !GetError() )   // Don't test for EOF as we
                                    // are reading block-wise!
    {
        sal_uInt16 nLen = static_cast<sal_uInt16>(ReadBytes(buf, sizeof(buf)-1));
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
        if (nTotalLen > o3tl::make_unsigned(nMaxBytesToRead))
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
        std::size_t nLen = ReadBytes(&cTemp, sizeof(cTemp));
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
    std::size_t nTotalLen   = 0;

    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "ReadUniStringLine: swapping sizeof(sal_Unicode) not implemented" );

    OUStringBuffer aBuf(4096);
    while( !bEnd && !GetError() )   // Don't test for EOF as we
                                    // are reading block-wise!
    {
        sal_uInt16 nLen = static_cast<sal_uInt16>(ReadBytes( buf, sizeof(buf)-sizeof(sal_Unicode)));
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
                SwapNumber( buf[n] );
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
        if (nTotalLen > o3tl::make_unsigned(nMaxCodepointsToRead))
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
        ReadBytes( &cTemp, sizeof(cTemp) );
        if (m_isSwap)
            SwapNumber( cTemp );
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

    char buf[ 256 + 1 ];
    bool bEnd = false;
    sal_uInt64 nFilePos = rStream.Tell();

    while( !bEnd && !rStream.GetError() )
    {
        std::size_t nLen = rStream.ReadBytes(buf, sizeof(buf)-1);
        if (!nLen)
            break;

        std::size_t nReallyRead = nLen;
        const char* pPtr = buf;
        while (nLen && *pPtr)
        {
            ++pPtr;
            --nLen;
        }

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
std::size_t write_uInt16s_FromOUString(SvStream& rStrm, std::u16string_view rStr,
    std::size_t nUnits)
{
    DBG_ASSERT( sizeof(sal_Unicode) == sizeof(sal_uInt16), "write_uInt16s_FromOUString: swapping sizeof(sal_Unicode) not implemented" );
    std::size_t nWritten;
    if (!rStrm.IsEndianSwap())
        nWritten = rStrm.WriteBytes(rStr.data(), nUnits * sizeof(sal_Unicode));
    else
    {
        std::size_t nLen = nUnits;
        sal_Unicode aBuf[384];
        sal_Unicode* const pTmp = ( nLen > 384 ? new sal_Unicode[nLen] : aBuf);
        memcpy( pTmp, rStr.data(), nLen * sizeof(sal_Unicode) );
        sal_Unicode* p = pTmp;
        const sal_Unicode* const pStop = pTmp + nLen;
        while ( p < pStop )
        {
            SwapNumber( *p );
            p++;
        }
        nWritten = rStrm.WriteBytes( pTmp, nLen * sizeof(sal_Unicode) );
        if ( pTmp != aBuf )
            delete [] pTmp;
    }
    return nWritten;
}

bool SvStream::WriteUnicodeOrByteText( std::u16string_view rStr, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
    {
        write_uInt16s_FromOUString(*this, rStr, rStr.size());
        return m_nError == ERRCODE_NONE;
    }
    else
    {
        OString aStr(OUStringToOString(rStr, eDestCharSet));
        write_uInt8s_FromOString(*this, aStr, aStr.getLength());
        return m_nError == ERRCODE_NONE;
    }
}

bool SvStream::WriteByteStringLine( std::u16string_view rStr, rtl_TextEncoding eDestCharSet )
{
    return WriteLine(OUStringToOString(rStr, eDestCharSet));
}

bool SvStream::WriteLine(std::string_view rStr)
{
    WriteBytes(rStr.data(), rStr.size());
    endl(*this);
    return m_nError == ERRCODE_NONE;
}

bool SvStream::WriteUniOrByteChar( sal_Unicode ch, rtl_TextEncoding eDestCharSet )
{
    if ( eDestCharSet == RTL_TEXTENCODING_UNICODE )
        WriteUnicode(ch);
    else
    {
        OString aStr(&ch, 1, eDestCharSet);
        WriteBytes(aStr.getStr(), aStr.getLength());
    }
    return m_nError == ERRCODE_NONE;
}

void SvStream::StartWritingUnicodeText()
{
    // BOM, Byte Order Mark, U+FEFF, see
    // http://www.unicode.org/faq/utf_bom.html#BOM
    // Upon read: 0xfeff(-257) => no swap; 0xfffe(-2) => swap
    writeNumberWithoutSwap(sal_uInt16(0xfeff)); // write native format
}

void SvStream::StartReadingUnicodeText( rtl_TextEncoding eReadBomCharSet )
{
    if (!(  eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
            eReadBomCharSet == RTL_TEXTENCODING_UNICODE ||
            eReadBomCharSet == RTL_TEXTENCODING_UTF8))
        return;    // nothing to read

    const sal_uInt64 nOldPos = Tell();
    bool bGetBack = true;
    unsigned char nFlag(0);
    ReadUChar( nFlag );
    switch ( nFlag )
    {
        case 0xfe: // UTF-16BE?
            if (    eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                    eReadBomCharSet == RTL_TEXTENCODING_UNICODE)
            {
                ReadUChar(nFlag);
                if (nFlag == 0xff)
                {
                    SetEndian(SvStreamEndian::BIG);
                    bGetBack = false;
                }
            }

        case 0xff: // UTF-16LE?
            if (    eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                    eReadBomCharSet == RTL_TEXTENCODING_UNICODE)
            {
                ReadUChar(nFlag);
                if (nFlag == 0xfe)
                {
                    SetEndian(SvStreamEndian::LITTLE);
                    bGetBack = false;
                }
            }

        case 0xef: // UTF-8?
            if (    eReadBomCharSet == RTL_TEXTENCODING_DONTKNOW ||
                    eReadBomCharSet == RTL_TEXTENCODING_UTF8)
            {
                ReadUChar(nFlag);
                if (nFlag == 0xbb)
                {
                    ReadUChar(nFlag);
                    if (nFlag == 0xbf)
                    {
                        bGetBack = false; // it is UTF-8
                    }
                }
            }

        default:
            ;   // nothing
    }
    if (bGetBack)
        Seek(nOldPos);      // no BOM, pure data
}

sal_uInt64 SvStream::SeekRel(sal_Int64 const nPos)
{
    sal_uInt64 nActualPos = Tell();

    if ( nPos >= 0 )
    {
        if (SAL_MAX_UINT64 - nActualPos > o3tl::make_unsigned(nPos))
            nActualPos += nPos;
    }
    else
    {
        sal_uInt64 const nAbsPos = static_cast<sal_uInt64>(-nPos);
        if ( nActualPos >= nAbsPos )
            nActualPos -= nAbsPos;
    }

    assert((m_pBufPos != nullptr) == bool(m_pRWBuf));
    if (m_pRWBuf)
    {
        m_pBufPos = m_pRWBuf.get() + nActualPos;
    }
    return Seek( nActualPos );
}

template <typename T> SvStream& SvStream::ReadNumber(T& r)
{
    T n = 0;
    readNumberWithoutSwap(n);
    if (good())
    {
        if (m_isSwap)
            SwapNumber(n);
        r = n;
    }
    return *this;
}

SvStream& SvStream::ReadUInt16(sal_uInt16& r) { return ReadNumber(r); }
SvStream& SvStream::ReadUInt32(sal_uInt32& r) { return ReadNumber(r); }
SvStream& SvStream::ReadUInt64(sal_uInt64& r) { return ReadNumber(r); }
SvStream& SvStream::ReadInt16(sal_Int16& r) { return ReadNumber(r); }
SvStream& SvStream::ReadInt32(sal_Int32& r) { return ReadNumber(r); }
SvStream& SvStream::ReadInt64(sal_Int64& r) { return ReadNumber(r); }

SvStream& SvStream::ReadSChar( signed char& r )
{
    if (m_isIoRead && sizeof(signed char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(signed char);
        m_pBufPos += sizeof(signed char);
        m_nBufFree -= sizeof(signed char);
    }
    else
        ReadBytes( &r, sizeof(signed char) );
    return *this;
}

// Special treatment for Chars due to PutBack

SvStream& SvStream::ReadChar( char& r )
{
    if (m_isIoRead && sizeof(char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(char);
        m_pBufPos += sizeof(char);
        m_nBufFree -= sizeof(char);
    }
    else
        ReadBytes( &r, sizeof(char) );
    return *this;
}

SvStream& SvStream::ReadUChar( unsigned char& r )
{
    if (m_isIoRead && sizeof(char) <= m_nBufFree)
    {
        r = *m_pBufPos;
        m_nBufActualPos += sizeof(char);
        m_pBufPos += sizeof(char);
        m_nBufFree -= sizeof(char);
    }
    else
        ReadBytes( &r, sizeof(char) );
    return *this;
}

SvStream& SvStream::ReadUtf16(sal_Unicode& r) { return ReadNumber(r); }

SvStream& SvStream::ReadCharAsBool( bool& r )
{
    if (m_isIoRead && sizeof(char) <= m_nBufFree)
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
        if (ReadBytes(&c, 1) == 1)
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
    readNumberWithoutSwap(n);
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
    readNumberWithoutSwap(n);
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
    std::unique_ptr<char[]> pBuf( new char[ cBufLen ] );

    sal_uInt32 nCount;
    do {
        nCount = ReadBytes( pBuf.get(), cBufLen );
        rStream.WriteBytes( pBuf.get(), nCount );
    } while( nCount == cBufLen );

    return *this;
}

template <typename T> SvStream& SvStream::WriteNumber(T n)
{
    if (m_isSwap)
        SwapNumber(n);
    writeNumberWithoutSwap(n);
    return *this;
}

SvStream& SvStream::WriteUInt16(sal_uInt16 v) { return WriteNumber(v); }
SvStream& SvStream::WriteUInt32(sal_uInt32 v) { return WriteNumber(v); }
SvStream& SvStream::WriteUInt64(sal_uInt64 v) { return WriteNumber(v); }
SvStream& SvStream::WriteInt16(sal_Int16 v) { return WriteNumber(v); }
SvStream& SvStream::WriteInt32(sal_Int32 v) { return WriteNumber(v); }
SvStream& SvStream::WriteInt64(sal_Int64 v) { return WriteNumber(v); }

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
        WriteBytes( &v, sizeof(signed char) );
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
        WriteBytes( &v, sizeof(char) );
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
        WriteBytes( &v, sizeof(char) );
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
    writeNumberWithoutSwap(v);
    return *this;
}

SvStream& SvStream::WriteDouble ( const double& r )
{
#if defined UNX
    if (m_isSwap)
    {
      double nHelp = r;
      SwapDouble(nHelp);
      writeNumberWithoutSwap(nHelp);
      return *this;
    }
    else
#endif
    {
        writeNumberWithoutSwap(r);
    }
    return *this;
}

SvStream& SvStream::WriteCharPtr( const char* pBuf )
{
    WriteBytes( pBuf, strlen(pBuf) );
    return *this;
}

SvStream& SvStream::WriteStream( SvStream& rStream )
{
    const sal_uInt32 cBufLen = 0x8000;
    std::unique_ptr<char[]> pBuf( new char[ cBufLen ] );
    sal_uInt32 nCount;
    do {
        nCount = rStream.ReadBytes( pBuf.get(), cBufLen );
        WriteBytes( pBuf.get(), nCount );
    } while( nCount == cBufLen );

    return *this;
}

sal_uInt64 SvStream::WriteStream( SvStream& rStream, sal_uInt64 nSize )
{
    const sal_uInt32 cBufLen = 0x8000;
    std::unique_ptr<char[]> pBuf( new char[ cBufLen ] );
    sal_uInt32 nCurBufLen = cBufLen;
    sal_uInt32 nCount;
    sal_uInt64 nWriteSize = nSize;

    do
    {
        nCurBufLen = std::min<sal_uInt64>(nCurBufLen, nWriteSize);
        nCount = rStream.ReadBytes(pBuf.get(), nCurBufLen);
        WriteBytes( pBuf.get(), nCount );
        nWriteSize -= nCount;
    }
    while( nWriteSize && nCount == nCurBufLen );

    return nSize - nWriteSize;
}

OUString SvStream::ReadUniOrByteString( rtl_TextEncoding eSrcCharSet )
{
    // read UTF-16 string directly from stream ?
    if (eSrcCharSet == RTL_TEXTENCODING_UNICODE)
        return read_uInt32_lenPrefixed_uInt16s_ToOUString(*this);
    return read_uInt16_lenPrefixed_uInt8s_ToOUString(*this, eSrcCharSet);
}

SvStream& SvStream::WriteUniOrByteString( std::u16string_view rStr, rtl_TextEncoding eDestCharSet )
{
    // write UTF-16 string directly into stream ?
    if (eDestCharSet == RTL_TEXTENCODING_UNICODE)
        write_uInt32_lenPrefixed_uInt16s_FromOUString(*this, rStr);
    else
        write_uInt16_lenPrefixed_uInt8s_FromOUString(*this, rStr, eDestCharSet);
    return *this;
}

void SvStream::FlushBuffer()
{
    if (m_isDirty) // Does stream require a flush?
    {
        SeekPos(m_nBufFilePos);
        if (m_nCryptMask)
            CryptAndWriteBuffer(m_pRWBuf.get(), m_nBufActualLen);
        else if (PutData(m_pRWBuf.get(), m_nBufActualLen) != m_nBufActualLen)
            SetError(SVSTREAM_WRITE_ERROR);
        m_isDirty = false;
    }
}

std::size_t SvStream::ReadBytes( void* pData, std::size_t nCount )
{
    std::size_t nSaveCount = nCount;

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
        if (nCount <= o3tl::make_unsigned(m_nBufActualLen - m_nBufActualPos))
        {
            // => yes
            if (nCount != 0)
                memcpy(pData, m_pBufPos, nCount);
            m_nBufActualPos = m_nBufActualPos + static_cast<sal_uInt16>(nCount);
            m_pBufPos += nCount;
            m_nBufFree = m_nBufFree - static_cast<sal_uInt16>(nCount);
        }
        else
        {
            FlushBuffer();

            // Does data block fit into buffer?
            if (nCount > m_nBufSize)
            {
                // => No! Thus read directly
                // into target area without using the buffer

                m_isIoRead = false;

                SeekPos(m_nBufFilePos + m_nBufActualPos);
                m_nBufActualLen = 0;
                m_pBufPos     = m_pRWBuf.get();
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
                std::size_t nCountTmp = GetData( m_pRWBuf.get(), m_nBufSize );
                if (m_nCryptMask)
                    EncryptBuffer(m_pRWBuf.get(), nCountTmp);
                m_nBufActualLen = static_cast<sal_uInt16>(nCountTmp);
                if( nCount > nCountTmp )
                {
                    nCount = nCountTmp;  // trim count back, EOF see below
                }
                memcpy( pData, m_pRWBuf.get(), nCount );
                m_nBufActualPos = static_cast<sal_uInt16>(nCount);
                m_pBufPos = m_pRWBuf.get() + nCount;
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

std::size_t SvStream::WriteBytes( const void* pData, std::size_t nCount )
{
    if( !nCount )
        return 0;

    if (!m_isWritable)
    {
        SetError( ERRCODE_IO_CANTWRITE );
        return 0;
    }

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
    if (nCount <= o3tl::make_unsigned(m_nBufSize - m_nBufActualPos))
    {
        memcpy( m_pBufPos, pData, nCount );
        m_nBufActualPos = m_nBufActualPos + static_cast<sal_uInt16>(nCount);
        // Update length if buffer was updated
        if (m_nBufActualPos > m_nBufActualLen)
            m_nBufActualLen = m_nBufActualPos;

        m_pBufPos += nCount;
        m_isDirty = true;
    }
    else
    {
        FlushBuffer();

        // Does data block fit into buffer?
        if (nCount > m_nBufSize)
        {
            m_isIoWrite = false;
            m_nBufFilePos += m_nBufActualPos;
            m_nBufActualLen = 0;
            m_nBufActualPos = 0;
            m_pBufPos     = m_pRWBuf.get();
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
            memcpy( m_pRWBuf.get(), pData, nCount );

            // Mind the order!
            m_nBufFilePos += m_nBufActualPos;
            m_nBufActualPos = static_cast<sal_uInt16>(nCount);
            m_pBufPos = m_pRWBuf.get() + nCount;
            m_nBufActualLen = static_cast<sal_uInt16>(nCount);
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
        m_nBufActualPos = static_cast<sal_uInt16>(nFilePos - m_nBufFilePos);
        m_pBufPos = m_pRWBuf.get() + m_nBufActualPos;
        // Update m_nBufFree to avoid crash upon PutBack
        m_nBufFree = m_nBufActualLen - m_nBufActualPos;
    }
    else
    {
        FlushBuffer();
        m_nBufActualLen = 0;
        m_nBufActualPos = 0;
        m_pBufPos     = m_pRWBuf.get();
        m_nBufFilePos = SeekPos( nFilePos );
    }
    return m_nBufFilePos + m_nBufActualPos;
}

bool checkSeek(SvStream &rSt, sal_uInt64 nOffset)
{
    const sal_uInt64 nMaxSeek = rSt.TellEnd();
    return (nOffset <= nMaxSeek && rSt.Seek(nOffset) == nOffset);
}

namespace tools
{
bool isEmptyFileUrl(const OUString& rUrl)
{
    if (!comphelper::isFileUrl(rUrl))
    {
        return false;
    }

    SvFileStream aStream(rUrl, StreamMode::READ);
    if (!aStream.IsOpen())
    {
        return false;
    }

    return aStream.remainingSize() == 0;
}
}

//STREAM_SEEK_TO_END in some of the Seek backends is special cased to be
//efficient, in others e.g. SotStorageStream it's really horribly slow, and in
//those this should be overridden
sal_uInt64 SvStream::remainingSize()
{
    sal_uInt64 const nCurr = Tell();
    sal_uInt64 const nEnd = TellEnd();
    sal_uInt64 nMaxAvailable = nEnd > nCurr ? (nEnd-nCurr) : 0;
    return nMaxAvailable;
}

sal_uInt64 SvStream::TellEnd()
{
    FlushBuffer();
    sal_uInt64 const nCurr = Tell();
    sal_uInt64 const nEnd = Seek(STREAM_SEEK_TO_END);
    Seek(nCurr);
    return nEnd;
}

void SvStream::Flush()
{
    FlushBuffer();
    if (m_isWritable)
        FlushData();
}

void SvStream::RefreshBuffer()
{
    FlushBuffer();
    SeekPos(m_nBufFilePos);
    m_nBufActualLen = static_cast<sal_uInt16>(GetData( m_pRWBuf.get(), m_nBufSize ));
    if (m_nBufActualLen && m_nError == ERRCODE_IO_PENDING)
        m_nError = ERRCODE_NONE;
    if (m_nCryptMask)
        EncryptBuffer(m_pRWBuf.get(), static_cast<std::size_t>(m_nBufActualLen));
    m_isIoRead = m_isIoWrite = false;
}

SvStream& SvStream::WriteInt32AsString(sal_Int32 nInt32)
{
    char buffer[12];
    std::size_t nLen = sprintf(buffer, "%" SAL_PRIdINT32, nInt32);
    WriteBytes(buffer, nLen);
    return *this;
}

SvStream& SvStream::WriteUInt32AsString(sal_uInt32 nUInt32)
{
    char buffer[11];
    std::size_t nLen = sprintf(buffer, "%" SAL_PRIuUINT32, nUInt32);
    WriteBytes(buffer, nLen);
    return *this;
}

#define CRYPT_BUFSIZE 1024

/// Encrypt and write
std::size_t SvStream::CryptAndWriteBuffer( const void* pStart, std::size_t nLen)
{
    unsigned char  pTemp[CRYPT_BUFSIZE];
    unsigned char const * pDataPtr = static_cast<unsigned char const *>(pStart);
    std::size_t nCount = 0;
    std::size_t nBufCount;
    unsigned char nMask = m_nCryptMask;
    do
    {
        if( nLen >= CRYPT_BUFSIZE )
            nBufCount = CRYPT_BUFSIZE;
        else
            nBufCount = nLen;
        nLen -= nBufCount;
        memcpy( pTemp, pDataPtr, static_cast<sal_uInt16>(nBufCount) );
        // ******** Encrypt ********
        for (unsigned char & rn : pTemp)
        {
            unsigned char aCh = rn;
            aCh ^= nMask;
            swapNibbles(aCh);
            rn = aCh;
        }
        // *************************
        nCount += PutData( pTemp, nBufCount );
        pDataPtr += nBufCount;
    }
    while ( nLen );
    return nCount;
}

void SvStream::EncryptBuffer(void* pStart, std::size_t nLen) const
{
    unsigned char* pTemp = static_cast<unsigned char*>(pStart);
    unsigned char nMask = m_nCryptMask;

    for ( std::size_t n=0; n < nLen; n++, pTemp++ )
    {
        unsigned char aCh = *pTemp;
        swapNibbles(aCh);
        aCh ^= nMask;
        *pTemp = aCh;
    }
}

static unsigned char implGetCryptMask(const char* pStr, sal_Int32 nLen, tools::Long nVersion)
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

bool SvStream::SetStreamSize(sal_uInt64 const nSize)
{
#ifdef DBG_UTIL
    sal_uInt64 nFPos = Tell();
#endif
    sal_uInt16 nBuf = m_nBufSize;
    SetBufferSize( 0 );
    SetSize( nSize );
    if (nSize < m_nBufFilePos)
    {
        m_nBufFilePos = nSize;
    }
    SetBufferSize( nBuf );
#ifdef DBG_UTIL
    DBG_ASSERT(Tell()==nFPos,"SetStreamSize failed");
#endif
    return (m_nError == ERRCODE_NONE);
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

SvMemoryStream::SvMemoryStream( void* pBuffer, std::size_t bufSize,
                                StreamMode eMode )
{
    if( eMode & StreamMode::WRITE )
        m_isWritable = true;
    else
        m_isWritable = false;
    nEndOfData  = bufSize;
    bOwnsData   = false;
    pBuf        = static_cast<sal_uInt8 *>(pBuffer);
    nResize     = 0;
    nSize       = bufSize;
    nPos        = 0;
    SetBufferSize( 0 );
}

SvMemoryStream::SvMemoryStream( std::size_t nInitSize, std::size_t nResizeOffset )
{
    m_isWritable = true;
    bOwnsData   = true;
    nEndOfData  = 0;
    nResize     = nResizeOffset;
    nPos        = 0;
    pBuf        = nullptr;
    if( nResize != 0 && nResize < 16 )
        nResize = 16;
    if( nInitSize )
        AllocateMemory( nInitSize );
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

void SvMemoryStream::SetBuffer( void* pNewBuf, std::size_t nCount,
                                 std::size_t nEOF )
{
    SetBufferSize( 0 ); // Init buffering in the base class
    Seek( 0 );
    if( bOwnsData && pNewBuf != pBuf )
        FreeMemory();

    pBuf        = static_cast<sal_uInt8 *>(pNewBuf);
    nPos        = 0;
    nSize       = nCount;
    nResize     = 0;
    bOwnsData   = false;

    if( nEOF > nCount )
        nEOF = nCount;
    nEndOfData = nEOF;

    ResetError();
}

std::size_t SvMemoryStream::GetData( void* pData, std::size_t nCount )
{
    std::size_t nMaxCount = nEndOfData-nPos;
    if( nCount > nMaxCount )
        nCount = nMaxCount;
    if (nCount != 0)
    {
        memcpy( pData, pBuf+nPos, nCount );
    }
    nPos += nCount;
    return nCount;
}

std::size_t SvMemoryStream::PutData( const void* pData, std::size_t nCount )
{
    if( GetError() )
        return 0;

    std::size_t nMaxCount = nSize-nPos;

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
            tools::Long nNewResize;
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
    memcpy( pBuf+nPos, pData, nCount);

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
                tools::Long nDiff = static_cast<tools::Long>(nNewPos - nSize + 1);
                nDiff += static_cast<tools::Long>(nResize);
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

void SvMemoryStream::AllocateMemory( std::size_t nNewSize )
{
    pBuf = new sal_uInt8[nNewSize];
}

// (using Bozo algorithm)
bool SvMemoryStream::ReAllocateMemory( tools::Long nDiff )
{
    if (!m_isWritable || !bOwnsData)
        return false;

    bool bRetVal    = false;
    tools::Long nTemp      = static_cast<tools::Long>(nSize);
    nTemp           += nDiff;
    std::size_t nNewSize  = static_cast<std::size_t>(nTemp);

    if( nNewSize )
    {
        sal_uInt8* pNewBuf   = new sal_uInt8[nNewSize];

        bRetVal = true; // Success!
        if( nNewSize < nSize )      // Are we shrinking?
        {
            memcpy( pNewBuf, pBuf, nNewSize );
            if( nPos > nNewSize )
                nPos = 0;
            if( nEndOfData >= nNewSize )
                nEndOfData = nNewSize-1;
        }
        else
        {
            if (nSize != 0)
            {
                memcpy( pNewBuf, pBuf, nSize );
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
    assert(bOwnsData);
    if (bOwnsData)
    {
        delete[] pBuf;
        pBuf = nullptr;
    }
}

void* SvMemoryStream::SwitchBuffer()
{
    Flush();
    if( !bOwnsData )
        return nullptr;
    Seek( STREAM_SEEK_TO_BEGIN );

    void* pRetVal = pBuf;
    pBuf          = nullptr;
    nEndOfData    = 0;
    nResize       = 64;
    nPos          = 0;

    ResetError();

    std::size_t nInitSize = 512;
    AllocateMemory(nInitSize);
    nSize = nInitSize;

    SetBufferSize( 64 );
    return pRetVal;
}

void SvMemoryStream::SetSize(sal_uInt64 const nNewSize)
{
    if (!m_isWritable)
    {
        SetError(SVSTREAM_INVALID_HANDLE);
        return;
    }

    tools::Long nDiff = static_cast<tools::Long>(nNewSize) - static_cast<tools::Long>(nSize);
    ReAllocateMemory( nDiff );
}

//Create an OString of nLen bytes from rStream
OString read_uInt8s_ToOString(SvStream& rStrm, std::size_t nLen)
{
    rtl_String *pStr = nullptr;
    if (nLen)
    {
        nLen = std::min<std::size_t>(nLen, SAL_MAX_INT32);
        //limit allocation to size of file, but + 1 to set eof state
        nLen = std::min<sal_uInt64>(nLen, rStrm.remainingSize() + 1);
        //alloc a (ref-count 1) rtl_String of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_string_alloc(sal::static_int_cast<sal_Int32>(nLen));
        SAL_WARN_IF(!pStr, "tools.stream", "allocation failed");
        if (pStr)
        {
            std::size_t nWasRead = rStrm.ReadBytes(pStr->buffer, nLen);
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

//Create an OUString of nLen sal_Unicode code units from rStream
OUString read_uInt16s_ToOUString(SvStream& rStrm, std::size_t nLen)
{
    rtl_uString *pStr = nullptr;
    if (nLen)
    {
        nLen = std::min<std::size_t>(nLen, SAL_MAX_INT32);
        //limit allocation to size of file, but + 1 to set eof state
        nLen = o3tl::sanitizing_min<sal_uInt64>(nLen, (rStrm.remainingSize() + 2) / 2);
        //alloc a (ref-count 1) rtl_uString of the desired length.
        //rtl_String's buffer is uninitialized, except for null termination
        pStr = rtl_uString_alloc(sal::static_int_cast<sal_Int32>(nLen));
        SAL_WARN_IF(!pStr, "tools.stream", "allocation failed");
        if (pStr)
        {
            std::size_t nWasRead = rStrm.ReadBytes(pStr->buffer, nLen*2)/2;
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
                    // Do we need to convert?
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

std::size_t write_uInt32_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                std::u16string_view rStr)
{
    std::size_t nWritten = 0;
    sal_uInt32 nUnits = std::min<std::size_t>(rStr.size(), std::numeric_limits<sal_uInt32>::max());
    SAL_WARN_IF(static_cast<std::size_t>(nUnits) != static_cast<std::size_t>(rStr.size()),
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

std::size_t write_uInt16_lenPrefixed_uInt16s_FromOUString(SvStream& rStrm,
                                                std::u16string_view rStr)
{
    std::size_t nWritten = 0;
    sal_uInt16 nUnits = std::min<std::size_t>(rStr.size(), std::numeric_limits<sal_uInt16>::max());
    SAL_WARN_IF(nUnits != rStr.size(),
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

std::size_t write_uInt16_lenPrefixed_uInt8s_FromOString(SvStream& rStrm,
                                              std::string_view rStr)
{
    std::size_t nWritten = 0;
    sal_uInt16 nUnits = std::min<std::size_t>(rStr.size(), std::numeric_limits<sal_uInt16>::max());
    SAL_WARN_IF(static_cast<std::size_t>(nUnits) != static_cast<std::size_t>(rStr.size()),
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
