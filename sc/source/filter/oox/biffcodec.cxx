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

#include "biffcodec.hxx"

#include <osl/thread.h>
#include <string.h>
#include "oox/core/filterbase.hxx"
#include "biffinputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

using ::oox::core::FilterBase;

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



BiffCodecHelper::BiffCodecHelper( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void BiffCodecHelper::cloneDecoder( BiffInputStream& rStrm )
{
    if( mxDecoder.get() )
        rStrm.setDecoder( BiffDecoderRef( mxDecoder->clone() ) );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
