/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: biffcodec.cxx,v $
 * $Revision: 1.4 $
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

#include "oox/xls/biffcodec.hxx"
#include <osl/thread.h>
#include <string.h>

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OStringToOUString;

namespace oox {
namespace xls {

// ============================================================================

const OString& BiffCodecHelper::getBiff5WbProtPassword()
{
    static const OString saPass( "VelvetSweatshop" );
    return saPass;
}

const OUString& BiffCodecHelper::getBiff8WbProtPassword()
{
    static const OUString saPass = OStringToOUString( getBiff5WbProtPassword(), RTL_TEXTENCODING_ASCII_US );
    return saPass;
}

// ============================================================================

BiffDecoderBase::BiffDecoderBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnError( CODEC_ERROR_UNSUPP_CRYPT )
{
}

BiffDecoderBase::~BiffDecoderBase()
{
}

void BiffDecoderBase::decode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int64 nStreamPos, sal_uInt16 nBytes )
{
    if( pnDestData && pnSrcData && (nBytes > 0) )
    {
        if( isValid() )
            implDecode( pnDestData, pnSrcData, nStreamPos, nBytes );
        else
            memcpy( pnDestData, pnSrcData, nBytes );
    }
}

void BiffDecoderBase::setHasValidPassword( bool bValid )
{
    mnError = bValid ? CODEC_OK : CODEC_ERROR_WRONG_PASS;
}

// ============================================================================

BiffDecoder_XOR::BiffDecoder_XOR( const WorkbookHelper& rHelper, sal_uInt16 nKey, sal_uInt16 nHash ) :
    BiffDecoderBase( rHelper ),
    maCodec( ::oox::core::BinaryCodec_XOR::CODEC_EXCEL )
{
    init( BiffCodecHelper::getBiff5WbProtPassword(), nKey, nHash );
    if( !isValid() )
    {
        OString aPass = OUStringToOString( queryPassword(), osl_getThreadTextEncoding() );
        init( aPass, nKey, nHash );
    }
}

void BiffDecoder_XOR::init( const OString& rPass, sal_uInt16 nKey, sal_uInt16 nHash )
{
    sal_Int32 nLen = rPass.getLength();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // copy byte string to sal_uInt8 array
        sal_uInt8 pnPassw[ 16 ];
        memset( pnPassw, 0, sizeof( pnPassw ) );
        memcpy( pnPassw, rPass.getStr(), static_cast< size_t >( nLen ) );

        // init codec
        maCodec.initKey( pnPassw );
        bValid = maCodec.verifyKey( nKey, nHash );
    }

    setHasValidPassword( bValid );
}

void BiffDecoder_XOR::implDecode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int64 nStreamPos, sal_uInt16 nBytes )
{
    maCodec.startBlock();
    maCodec.skip( static_cast< sal_Int32 >( (nStreamPos + nBytes) & 0x0F ) );
    maCodec.decode( pnDestData, pnSrcData, nBytes );
}

// ============================================================================

namespace {

/** Returns the block index of the passed stream position for RCF decryption. */
sal_Int32 lclGetRcfBlock( sal_Int64 nStreamPos )
{
    return static_cast< sal_Int32 >( nStreamPos / BIFF_RCF_BLOCKSIZE );
}

/** Returns the offset of the passed stream position in a block for RCF decryption. */
sal_Int32 lclGetRcfOffset( sal_Int64 nStreamPos )
{
    return static_cast< sal_Int32 >( nStreamPos % BIFF_RCF_BLOCKSIZE );
}

} // namespace

// ----------------------------------------------------------------------------

BiffDecoder_RCF::BiffDecoder_RCF( const WorkbookHelper& rHelper,
        sal_uInt8 pnDocId[ 16 ], sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] ) :
    BiffDecoderBase( rHelper )
{
    init( BiffCodecHelper::getBiff8WbProtPassword(), pnDocId, pnSaltData, pnSaltHash );
    if( !isValid() )
        init( queryPassword(), pnDocId, pnSaltData, pnSaltHash );
}

void BiffDecoder_RCF::init( const OUString& rPass, sal_uInt8 pnDocId[ 16 ], sal_uInt8 pnSaltData[ 16 ], sal_uInt8 pnSaltHash[ 16 ] )
{
    sal_Int32 nLen = rPass.getLength();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // copy string to sal_uInt16 array
        sal_uInt16 pnPassw[ 16 ];
        memset( pnPassw, 0, sizeof( pnPassw ) );
        const sal_Unicode* pcChar = rPass.getStr();
        const sal_Unicode* pcCharEnd = pcChar + nLen;
        sal_uInt16* pnCurrPass = pnPassw;
        for( ; pcChar < pcCharEnd; ++pcChar, ++pnCurrPass )
            *pnCurrPass = static_cast< sal_uInt16 >( *pcChar );

        // init codec
        maCodec.initKey( pnPassw, pnDocId );
        bValid = maCodec.verifyKey( pnSaltData, pnSaltHash );
    }

    setHasValidPassword( bValid );
}

void BiffDecoder_RCF::implDecode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int64 nStreamPos, sal_uInt16 nBytes )
{
    sal_uInt8* pnCurrDest = pnDestData;
    const sal_uInt8* pnCurrSrc = pnSrcData;
    sal_Int64 nCurrPos = nStreamPos;
    sal_uInt16 nBytesLeft = nBytes;
    while( nBytesLeft > 0 )
    {
        // initialize codec for current stream position
        maCodec.startBlock( lclGetRcfBlock( nCurrPos ) );
        maCodec.skip( lclGetRcfOffset( nCurrPos ) );

        // decode the block
        sal_uInt16 nBlockLeft = static_cast< sal_uInt16 >( BIFF_RCF_BLOCKSIZE - lclGetRcfOffset( nCurrPos ) );
        sal_uInt16 nDecBytes = ::std::min( nBytesLeft, nBlockLeft );
        maCodec.decode( pnCurrDest, pnCurrSrc, static_cast< sal_Int32 >( nDecBytes ) );

        // prepare for next block
        pnCurrDest += nDecBytes;
        pnCurrSrc += nDecBytes;
        nCurrPos += nDecBytes;
        nBytesLeft = nBytesLeft - nDecBytes;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

