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

#include "oox/ole/vbainputstream.hxx"
#include <osl/diagnose.h>

namespace oox {
namespace ole {

namespace {

const sal_uInt8 VBASTREAM_SIGNATURE         = 1;

const sal_uInt16 VBACHUNK_SIGMASK           = 0x7000;
const sal_uInt16 VBACHUNK_SIG               = 0x3000;
const sal_uInt16 VBACHUNK_COMPRESSED        = 0x8000;
const sal_uInt16 VBACHUNK_LENMASK           = 0x0FFF;

} // namespace

VbaInputStream::VbaInputStream( BinaryInputStream& rInStrm ) :
    BinaryStreamBase( false ),
    mpInStrm( &rInStrm ),
    mnChunkPos( 0 )
{
    maChunk.reserve( 4096 );

    sal_uInt8 nSig = rInStrm.readuInt8();
    OSL_ENSURE( nSig == VBASTREAM_SIGNATURE, "VbaInputStream::VbaInputStream - wrong signature" );
    mbEof = mbEof || rInStrm.isEof() || (nSig != VBASTREAM_SIGNATURE);
}

sal_Int64 VbaInputStream::size() const
{
    return -1;
}

sal_Int64 VbaInputStream::tell() const
{
    return -1;
}

void VbaInputStream::seek( sal_Int64 )
{
}

void VbaInputStream::close()
{
    mpInStrm = nullptr;
    mbEof = true;
}

sal_Int32 VbaInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nRet = 0;
    if( !mbEof )
    {
        orData.realloc( ::std::max< sal_Int32 >( nBytes, 0 ) );
        if( nBytes > 0 )
        {
            nRet = readMemory( orData.getArray(), nBytes, nAtomSize );
            if( nRet < nBytes )
                orData.realloc( nRet );
        }
    }
    return nRet;
}

sal_Int32 VbaInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    sal_Int32 nRet = 0;
    sal_uInt8* opnMem = static_cast< sal_uInt8* >( opMem );
    while( (nBytes > 0) && updateChunk() )
    {
        sal_Int32 nChunkLeft = static_cast< sal_Int32 >( maChunk.size() - mnChunkPos );
        sal_Int32 nReadBytes = ::std::min( nBytes, nChunkLeft );
        memcpy( opnMem, &*(maChunk.begin() + mnChunkPos), nReadBytes );
        opnMem += nReadBytes;
        mnChunkPos += static_cast< size_t >( nReadBytes );
        nBytes -= nReadBytes;
        nRet += nReadBytes;
    }
    return nRet;
}

void VbaInputStream::skip( sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    while( (nBytes > 0) && updateChunk() )
    {
        sal_Int32 nChunkLeft = static_cast< sal_Int32 >( maChunk.size() - mnChunkPos );
        sal_Int32 nSkipBytes = ::std::min( nBytes, nChunkLeft );
        mnChunkPos += static_cast< size_t >( nSkipBytes );
        nBytes -= nSkipBytes;
    }
}

// private --------------------------------------------------------------------

bool VbaInputStream::updateChunk()
{
    if( mbEof || (mnChunkPos < maChunk.size()) ) return !mbEof;
    // try to read next chunk header, this may trigger EOF
    sal_uInt16 nHeader = mpInStrm->readuInt16();

    mbEof = mpInStrm->isEof();
    if( mbEof ) return false;

    // check header signature
    bool bIgnoreBrokenSig = !( (nHeader & VBACHUNK_SIGMASK) == VBACHUNK_SIG );

    // decode length of chunk data and compression flag
    bool bCompressed = getFlag( nHeader, VBACHUNK_COMPRESSED );
    sal_uInt16 nChunkLen = (nHeader & VBACHUNK_LENMASK) + 1;
    OSL_ENSURE( bCompressed || (nChunkLen == 4096), "VbaInputStream::updateChunk - invalid uncompressed chunk size" );

    // From the amazing bit detective work of Valek Filippov<frob@gnome.org>
    // this tweak and the one at the bottom of the method to seek to the
    // start of the next chunk we can read those strange broken
    // ( I guess from a MSO bug ) compressed streams > 4k

    if ( bIgnoreBrokenSig )
    {
        bCompressed = true;
        nChunkLen = 4094;
    }

    sal_Int64 target = mpInStrm->tell() + nChunkLen;
    if( bCompressed )
    {
        maChunk.clear();
        sal_uInt8 nBitCount = 4;
        sal_uInt16 nChunkPos = 0;
        while( !mbEof && !mpInStrm->isEof() && (nChunkPos < nChunkLen) )
        {
            sal_uInt8 nTokenFlags = mpInStrm->readuInt8();
            ++nChunkPos;
            for( int nBit = 0; !mbEof && !mpInStrm->isEof() && (nBit < 8) && (nChunkPos < nChunkLen); ++nBit, nTokenFlags >>= 1 )
            {
                if( nTokenFlags & 1 )
                {
                    sal_uInt16 nCopyToken = mpInStrm->readuInt16();
                    nChunkPos = nChunkPos + 2;
                    // update bit count used for offset/length in the token
                    while( ((static_cast<size_t>(1)) << nBitCount ) < maChunk.size() ) ++nBitCount;
                    // extract length from lower (16-nBitCount) bits, plus 3
                    sal_uInt16 nLength = extractValue< sal_uInt16 >( nCopyToken, 0, 16 - nBitCount ) + 3;
                    // extract offset from high nBitCount bits, plus 1
                    sal_uInt16 nOffset = extractValue< sal_uInt16 >( nCopyToken, 16 - nBitCount, nBitCount ) + 1;
                    mbEof = (nOffset > maChunk.size()) || (maChunk.size() + nLength > 4096);
                    OSL_ENSURE( !mbEof, "VbaInputStream::updateChunk - invalid offset or size in copy token" );
                    if( !mbEof )
                    {
                        // append data to buffer
                        maChunk.resize( maChunk.size() + nLength );
                        sal_uInt8* pnTo = &*(maChunk.end() - nLength);
                        const sal_uInt8* pnEnd = pnTo + nLength;
                        const sal_uInt8* pnFrom = pnTo - nOffset;
                        // offset may be less than length, effectively duplicating source data several times
                        size_t nRunLen = ::std::min< size_t >( nLength, nOffset );
                        while( pnTo < pnEnd )
                        {
                            size_t nStepLen = ::std::min< size_t >( nRunLen, pnEnd - pnTo );
                            memcpy( pnTo, pnFrom, nStepLen );
                            pnTo += nStepLen;
                        }
                    }
                }
                // we suspect this will never be called
                else
                {
                    maChunk.emplace_back();
                    maChunk.back() = mpInStrm->readuChar();
                    ++nChunkPos;
                }
            }
        }
    }
    else
    {
        maChunk.resize( nChunkLen );
        mpInStrm->readMemory(maChunk.data(), nChunkLen);
    }
    // decompression sometimes leaves the stream pos offset 1 place ( at
    // least ) past or before the expected stream pos.
    // here we make sure we are on the chunk boundary
    mpInStrm->seek( target );
    mnChunkPos = 0;
    return !mbEof;
}

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
