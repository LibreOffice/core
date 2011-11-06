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



#include "oox/xls/biffcodec.hxx"

#include <osl/thread.h>
#include <string.h>
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

using ::oox::core::FilterBase;
using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OStringToOUString;

// ============================================================================

BiffDecoderBase::BiffDecoderBase() :
    mbValid( false )
{
}

BiffDecoderBase::~BiffDecoderBase()
{
}

::comphelper::DocPasswordVerifierResult BiffDecoderBase::verifyPassword( const OUString& rPassword, Sequence< NamedValue >& o_rEncryptionData )
{
    o_rEncryptionData = implVerifyPassword( rPassword );
    mbValid = o_rEncryptionData.hasElements();
    return mbValid ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

::comphelper::DocPasswordVerifierResult BiffDecoderBase::verifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    mbValid = implVerifyEncryptionData( rEncryptionData );
    return mbValid ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

void BiffDecoderBase::decode( sal_uInt8* pnDestData, const sal_uInt8* pnSrcData, sal_Int64 nStreamPos, sal_uInt16 nBytes )
{
    if( pnDestData && pnSrcData && (nBytes > 0) )
    {
        if( mbValid )
            implDecode( pnDestData, pnSrcData, nStreamPos, nBytes );
        else
            memcpy( pnDestData, pnSrcData, nBytes );
    }
}

// ============================================================================

BiffDecoder_XOR::BiffDecoder_XOR( sal_uInt16 nKey, sal_uInt16 nHash ) :
    maCodec( ::oox::core::BinaryCodec_XOR::CODEC_EXCEL ),
    mnKey( nKey ),
    mnHash( nHash )
{
}

BiffDecoder_XOR::BiffDecoder_XOR( const BiffDecoder_XOR& rDecoder ) :
    BiffDecoderBase(),  // must be called to prevent compiler warning
    maCodec( ::oox::core::BinaryCodec_XOR::CODEC_EXCEL ),
    maEncryptionData( rDecoder.maEncryptionData ),
    mnKey( rDecoder.mnKey ),
    mnHash( rDecoder.mnHash )
{
    if( isValid() )
        maCodec.initCodec( maEncryptionData );
}

BiffDecoder_XOR* BiffDecoder_XOR::implClone()
{
    return new BiffDecoder_XOR( *this );
}

Sequence< NamedValue > BiffDecoder_XOR::implVerifyPassword( const OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    /*  Convert password to a byte string. TODO: this needs some finetuning
        according to the spec... */
    OString aBytePassword = OUStringToOString( rPassword, osl_getThreadTextEncoding() );
    sal_Int32 nLen = aBytePassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // init codec
        maCodec.initKey( reinterpret_cast< const sal_uInt8* >( aBytePassword.getStr() ) );

        if( maCodec.verifyKey( mnKey, mnHash ) )
            maEncryptionData = maCodec.getEncryptionData();
    }

    return maEncryptionData;
}

bool BiffDecoder_XOR::implVerifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    maEncryptionData.realloc( 0 );

    if( rEncryptionData.hasElements() )
    {
        // init codec
        maCodec.initCodec( rEncryptionData );

        if( maCodec.verifyKey( mnKey, mnHash ) )
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.hasElements();
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

BiffDecoder_RCF::BiffDecoder_RCF( sal_uInt8 pnSalt[ 16 ], sal_uInt8 pnVerifier[ 16 ], sal_uInt8 pnVerifierHash[ 16 ] ) :
    maSalt( pnSalt, pnSalt + 16 ),
    maVerifier( pnVerifier, pnVerifier + 16 ),
    maVerifierHash( pnVerifierHash, pnVerifierHash + 16 )
{
}

BiffDecoder_RCF::BiffDecoder_RCF( const BiffDecoder_RCF& rDecoder ) :
    BiffDecoderBase(),  // must be called to prevent compiler warning
    maEncryptionData( rDecoder.maEncryptionData ),
    maSalt( rDecoder.maSalt ),
    maVerifier( rDecoder.maVerifier ),
    maVerifierHash( rDecoder.maVerifierHash )
{
    if( isValid() )
        maCodec.initCodec( maEncryptionData );
}

BiffDecoder_RCF* BiffDecoder_RCF::implClone()
{
    return new BiffDecoder_RCF( *this );
}

Sequence< NamedValue > BiffDecoder_RCF::implVerifyPassword( const OUString& rPassword )
{
    maEncryptionData.realloc( 0 );

    sal_Int32 nLen = rPassword.getLength();
    if( (0 < nLen) && (nLen < 16) )
    {
        // copy string to sal_uInt16 array
        ::std::vector< sal_uInt16 > aPassVect( 16 );
        const sal_Unicode* pcChar = rPassword.getStr();
        const sal_Unicode* pcCharEnd = pcChar + nLen;
        ::std::vector< sal_uInt16 >::iterator aIt = aPassVect.begin();
        for( ; pcChar < pcCharEnd; ++pcChar, ++aIt )
            *aIt = static_cast< sal_uInt16 >( *pcChar );

        // init codec
        maCodec.initKey( &aPassVect.front(), &maSalt.front() );
        if( maCodec.verifyKey( &maVerifier.front(), &maVerifierHash.front() ) )
            maEncryptionData = maCodec.getEncryptionData();
    }

    return maEncryptionData;
}

bool BiffDecoder_RCF::implVerifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    maEncryptionData.realloc( 0 );

    if( rEncryptionData.hasElements() )
    {
        // init codec
        maCodec.initCodec( rEncryptionData );

        if( maCodec.verifyKey( &maVerifier.front(), &maVerifierHash.front() ) )
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.hasElements();
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

const sal_uInt16 BIFF_FILEPASS_XOR                  = 0;
const sal_uInt16 BIFF_FILEPASS_RCF                  = 1;

const sal_uInt16 BIFF_FILEPASS_BIFF8_RCF            = 1;
const sal_uInt16 BIFF_FILEPASS_BIFF8_CRYPTOAPI_2003 = 2;
const sal_uInt16 BIFF_FILEPASS_BIFF8_CRYPTOAPI_2007 = 3;

// ----------------------------------------------------------------------------

BiffDecoderRef lclReadFilePass_XOR( BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;
    OSL_ENSURE( rStrm.getRemaining() == 4, "lclReadFilePass_XOR - wrong record size" );
    if( rStrm.getRemaining() == 4 )
    {
        sal_uInt16 nBaseKey, nHash;
        rStrm >> nBaseKey >> nHash;
        xDecoder.reset( new BiffDecoder_XOR( nBaseKey, nHash ) );
    }
    return xDecoder;
}

BiffDecoderRef lclReadFilePass_RCF( BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;
    OSL_ENSURE( rStrm.getRemaining() == 48, "lclReadFilePass_RCF - wrong record size" );
    if( rStrm.getRemaining() == 48 )
    {
        sal_uInt8 pnSalt[ 16 ];
        sal_uInt8 pnVerifier[ 16 ];
        sal_uInt8 pnVerifierHash[ 16 ];
        rStrm.readMemory( pnSalt, 16 );
        rStrm.readMemory( pnVerifier, 16 );
        rStrm.readMemory( pnVerifierHash, 16 );
        xDecoder.reset( new BiffDecoder_RCF( pnSalt, pnVerifier, pnVerifierHash ) );
    }
    return xDecoder;
}

BiffDecoderRef lclReadFilePass_CryptoApi( BiffInputStream& /*rStrm*/ )
{
    // not supported
    return BiffDecoderRef();
}

BiffDecoderRef lclReadFilePassBiff8( BiffInputStream& rStrm )
{
    BiffDecoderRef xDecoder;
    switch( rStrm.readuInt16() )
    {
        case BIFF_FILEPASS_XOR:
            xDecoder = lclReadFilePass_XOR( rStrm );
        break;

        case BIFF_FILEPASS_RCF:
        {
            sal_uInt16 nMajor = rStrm.readuInt16();
            rStrm.skip( 2 );
            switch( nMajor )
            {
                case BIFF_FILEPASS_BIFF8_RCF:
                    xDecoder = lclReadFilePass_RCF( rStrm );
                break;
                case BIFF_FILEPASS_BIFF8_CRYPTOAPI_2003:
                case BIFF_FILEPASS_BIFF8_CRYPTOAPI_2007:
                    xDecoder = lclReadFilePass_CryptoApi( rStrm );
                break;
                default:
                    OSL_ENSURE( false, "lclReadFilePassBiff8 - unknown BIFF8 encryption sub mode" );
            }
        }
        break;

        default:
            OSL_ENSURE( false, "lclReadFilePassBiff8 - unknown encryption mode" );
    }
    return xDecoder;
}

} // namespace

// ----------------------------------------------------------------------------

BiffCodecHelper::BiffCodecHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

/*static*/ BiffDecoderRef BiffCodecHelper::implReadFilePass( BiffInputStream& rStrm, BiffType eBiff )
{
    rStrm.enableDecoder( false );
    BiffDecoderRef xDecoder = (eBiff == BIFF8) ? lclReadFilePassBiff8( rStrm ) : lclReadFilePass_XOR( rStrm );
    rStrm.setDecoder( xDecoder );
    return xDecoder;
}

bool BiffCodecHelper::importFilePass( BiffInputStream& rStrm )
{
    OSL_ENSURE( !mxDecoder, "BiffCodecHelper::importFilePass - multiple FILEPASS records" );
    mxDecoder = implReadFilePass( rStrm, getBiff() );
    // request and verify a password (decoder implements IDocPasswordVerifier)
    if( mxDecoder.get() )
        getBaseFilter().requestEncryptionData( *mxDecoder );
    // correct password is indicated by isValid() function of decoder
    return mxDecoder.get() && mxDecoder->isValid();
}

void BiffCodecHelper::cloneDecoder( BiffInputStream& rStrm )
{
    if( mxDecoder.get() )
        rStrm.setDecoder( BiffDecoderRef( mxDecoder->clone() ) );
}

// ============================================================================

} // namespace xls
} // namespace oox
