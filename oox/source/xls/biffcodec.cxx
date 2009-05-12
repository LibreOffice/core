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
#include "oox/xls/biffinputstream.hxx"

using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OStringToOUString;

namespace oox {
namespace xls {

// ============================================================================

BiffDecoderBase::BiffDecoderBase( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnError( CODEC_ERROR_UNSUPP_CRYPT )
{
}

BiffDecoderBase::BiffDecoderBase( const BiffDecoderBase& rDecoder ) :
    WorkbookHelper( rDecoder ),
    mnError( rDecoder.mnError )
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
    maCodec( ::oox::core::BinaryCodec_XOR::CODEC_EXCEL ),
    mnKey( nKey ),
    mnHash( nHash )
{
    init( BiffCodecHelper::getBiff5WbProtPassword() );
    if( !isValid() )
        init( OUStringToOString( getCodecHelper().queryPassword(), osl_getThreadTextEncoding() ) );
}

BiffDecoder_XOR::BiffDecoder_XOR( const BiffDecoder_XOR& rDecoder ) :
    BiffDecoderBase( rDecoder ),
    maCodec( ::oox::core::BinaryCodec_XOR::CODEC_EXCEL ),
    mnKey( rDecoder.mnKey ),
    mnHash( rDecoder.mnHash )
{
    init( rDecoder.maPass );
}

void BiffDecoder_XOR::init( const OString& rPass )
{
    maPass = rPass;
    sal_Int32 nLen = maPass.getLength();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // copy byte string to sal_uInt8 array
        sal_uInt8 pnPassw[ 16 ];
        memset( pnPassw, 0, sizeof( pnPassw ) );
        memcpy( pnPassw, maPass.getStr(), static_cast< size_t >( nLen ) );

        // init codec
        maCodec.initKey( pnPassw );
        bValid = maCodec.verifyKey( mnKey, mnHash );
    }

    setHasValidPassword( bValid );
}

BiffDecoder_XOR* BiffDecoder_XOR::implClone()
{
    return new BiffDecoder_XOR( *this );
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
    BiffDecoderBase( rHelper ),
    maDocId( pnDocId, pnDocId + 16 ),
    maSaltData( pnSaltData, pnSaltData + 16 ),
    maSaltHash( pnSaltHash, pnSaltHash + 16 )
{
    init( BiffCodecHelper::getBiff8WbProtPassword() );
    if( !isValid() )
        init( getCodecHelper().queryPassword() );
}

BiffDecoder_RCF::BiffDecoder_RCF( const BiffDecoder_RCF& rDecoder ) :
    BiffDecoderBase( rDecoder ),
    maDocId( rDecoder.maDocId ),
    maSaltData( rDecoder.maSaltData ),
    maSaltHash( rDecoder.maSaltHash )
{
    init( rDecoder.maPass );
}

void BiffDecoder_RCF::init( const OUString& rPass )
{
    maPass = rPass;
    sal_Int32 nLen = maPass.getLength();
    bool bValid = (0 < nLen) && (nLen < 16);

    if( bValid )
    {
        // copy string to sal_uInt16 array
        sal_uInt16 pnPassw[ 16 ];
        memset( pnPassw, 0, sizeof( pnPassw ) );
        const sal_Unicode* pcChar = maPass.getStr();
        const sal_Unicode* pcCharEnd = pcChar + nLen;
        sal_uInt16* pnCurrPass = pnPassw;
        for( ; pcChar < pcCharEnd; ++pcChar, ++pnCurrPass )
            *pnCurrPass = static_cast< sal_uInt16 >( *pcChar );

        // init codec
        maCodec.initKey( pnPassw, &maDocId.front() );
        bValid = maCodec.verifyKey( &maSaltData.front(), &maSaltHash.front() );
    }

    setHasValidPassword( bValid );
}

BiffDecoder_RCF* BiffDecoder_RCF::implClone()
{
    return new BiffDecoder_RCF( *this );
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

namespace {

const sal_uInt16 BIFF_FILEPASS_BIFF2        = 0x0000;
const sal_uInt16 BIFF_FILEPASS_BIFF8        = 0x0001;
const sal_uInt16 BIFF_FILEPASS_BIFF8_RCF    = 0x0001;
const sal_uInt16 BIFF_FILEPASS_BIFF8_STRONG = 0x0002;

} // namespace

// ----------------------------------------------------------------------------

BiffCodecHelper::BiffCodecHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbHasPassword( false )
{
}

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

OUString BiffCodecHelper::queryPassword()
{
    if( !mbHasPassword )
    {
        //! TODO
        maPassword = OUString();
        // set to true, even if dialog has been cancelled (never ask twice)
        mbHasPassword = true;
    }
    return maPassword;
}

bool BiffCodecHelper::importFilePass( BiffInputStream& rStrm )
{
    OSL_ENSURE( !mxDecoder, "BiffCodecHelper::importFilePass - multiple FILEPASS records" );
    rStrm.enableDecoder( false );
    mxDecoder.reset();
    if( getBiff() == BIFF8 ) importFilePass8( rStrm ); else importFilePass2( rStrm );

    // set decoder at import stream
    rStrm.setDecoder( mxDecoder );
    //! TODO remember encryption state for export
//    mrStrm.GetRoot().GetExtDocOptions().GetDocSettings().mbEncrypted = true;

    return mxDecoder.get() && mxDecoder->isValid();
}

void BiffCodecHelper::cloneDecoder( BiffInputStream& rStrm )
{
    if( mxDecoder.get() )
        rStrm.setDecoder( BiffDecoderRef( mxDecoder->clone() ) );
}

// private --------------------------------------------------------------------

void BiffCodecHelper::importFilePass_XOR( BiffInputStream& rStrm )
{
    OSL_ENSURE( rStrm.getRemaining() == 4, "BiffCodecHelper::importFilePass_XOR - wrong record size" );
    if( rStrm.getRemaining() == 4 )
    {
        sal_uInt16 nBaseKey, nHash;
        rStrm >> nBaseKey >> nHash;
        mxDecoder.reset( new BiffDecoder_XOR( *this, nBaseKey, nHash ) );
    }
}

void BiffCodecHelper::importFilePass_RCF( BiffInputStream& rStrm )
{
    OSL_ENSURE( rStrm.getRemaining() == 48, "BiffCodecHelper::importFilePass_RCF - wrong record size" );
    if( rStrm.getRemaining() == 48 )
    {
        sal_uInt8 pnDocId[ 16 ];
        sal_uInt8 pnSaltData[ 16 ];
        sal_uInt8 pnSaltHash[ 16 ];
        rStrm.readMemory( pnDocId, 16 );
        rStrm.readMemory( pnSaltData, 16 );
        rStrm.readMemory( pnSaltHash, 16 );
        mxDecoder.reset( new BiffDecoder_RCF( *this, pnDocId, pnSaltData, pnSaltHash ) );
    }
}

void BiffCodecHelper::importFilePass_Strong( BiffInputStream& /*rStrm*/ )
{
    // not supported
}

void BiffCodecHelper::importFilePass2( BiffInputStream& rStrm )
{
    importFilePass_XOR( rStrm );
}

void BiffCodecHelper::importFilePass8( BiffInputStream& rStrm )
{
    switch( rStrm.readuInt16() )
    {
        case BIFF_FILEPASS_BIFF2:
            importFilePass_XOR( rStrm );
        break;

        case BIFF_FILEPASS_BIFF8:
            rStrm.skip( 2 );
            switch( rStrm.readuInt16() )
            {
                case BIFF_FILEPASS_BIFF8_RCF:
                    importFilePass_RCF( rStrm );
                break;
                case BIFF_FILEPASS_BIFF8_STRONG:
                    importFilePass_Strong( rStrm );
                break;
                default:
                    OSL_ENSURE( false, "BiffCodecHelper::importFilePass8 - unknown BIFF8 encryption sub mode" );
            }
        break;

        default:
            OSL_ENSURE( false, "BiffCodecHelper::importFilePass8 - unknown encryption mode" );
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

