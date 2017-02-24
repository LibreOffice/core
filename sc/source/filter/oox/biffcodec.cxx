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
#include <oox/helper/attributelist.hxx>
#include <string.h>

namespace oox {
namespace xls {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;


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
    return mbValid ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
}

::comphelper::DocPasswordVerifierResult BiffDecoderBase::verifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    mbValid = implVerifyEncryptionData( rEncryptionData );
    return mbValid ? ::comphelper::DocPasswordVerifierResult::OK : ::comphelper::DocPasswordVerifierResult::WrongPassword;
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
        maCodec.initKey(aPassVect.data(), maSalt.data());
        if (maCodec.verifyKey(maVerifier.data(), maVerifierHash.data()))
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

        if (maCodec.verifyKey(maVerifier.data(), maVerifierHash.data()))
            maEncryptionData = rEncryptionData;
    }

    return maEncryptionData.hasElements();
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
