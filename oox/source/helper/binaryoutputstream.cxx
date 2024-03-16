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

#include <oox/helper/binaryoutputstream.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <osl/diagnose.h>
#include <string.h>

namespace oox {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

namespace {

const sal_Int32 OUTPUTSTREAM_BUFFERSIZE     = 0x8000;

} // namespace

BinaryXOutputStream::BinaryXOutputStream( const Reference< XOutputStream >& rxOutStrm, bool bAutoClose ) :
    BinaryStreamBase( Reference< XSeekable >( rxOutStrm, UNO_QUERY ).is() ),
    BinaryXSeekableStream( Reference< XSeekable >( rxOutStrm, UNO_QUERY ) ),
    maBuffer( OUTPUTSTREAM_BUFFERSIZE ),
    mxOutStrm( rxOutStrm ),
    mbAutoClose( bAutoClose && rxOutStrm.is() )
{
    mbEof = !mxOutStrm.is();
}

BinaryXOutputStream::~BinaryXOutputStream()
{
    close();
}

void BinaryXOutputStream::close()
{
    OSL_ENSURE( !mbAutoClose || mxOutStrm.is(), "BinaryXOutputStream::close - invalid call" );
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->flush();
        if ( mbAutoClose )
            mxOutStrm->closeOutput();
    }
    catch( Exception& )
    {
        OSL_FAIL( "BinaryXOutputStream::close - closing output stream failed" );
    }
    mxOutStrm.clear();
    mbAutoClose = false;
    BinaryXSeekableStream::close();
}

void BinaryXOutputStream::writeData( const StreamDataSequence& rData, size_t /*nAtomSize*/ )
{
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->writeBytes( rData );
    }
    catch( Exception& )
    {
        OSL_FAIL( "BinaryXOutputStream::writeData - stream read error" );
    }
}

void BinaryXOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize )
{
    if( !(mxOutStrm.is() && (nBytes > 0)) )
        return;

    sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, (OUTPUTSTREAM_BUFFERSIZE / nAtomSize) * nAtomSize );
    const sal_uInt8* pnMem = static_cast< const sal_uInt8* >( pMem );
    while( nBytes > 0 )
    {
        sal_Int32 nWriteSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, nBufferSize );
        maBuffer.realloc( nWriteSize );
        memcpy( maBuffer.getArray(), pnMem, static_cast< size_t >( nWriteSize ) );
        writeData( maBuffer, nAtomSize );
        pnMem += nWriteSize;
        nBytes -= nWriteSize;
    }
}

void
BinaryOutputStream::writeCharArrayUC( std::u16string_view rString, rtl_TextEncoding eTextEnc )
{
    OString sBuf( OUStringToOString( rString, eTextEnc ) );
    sBuf = sBuf.replace( '\0', '?' );
    writeMemory( static_cast< const void* >( sBuf.getStr() ), sBuf.getLength() );
}

void
BinaryOutputStream::writeUnicodeArray( const OUString& rString )
{
    OUString sBuf = rString.replace( '\0', '?' );
#ifdef OSL_BIGENDIAN
    // need a non-const buffer for swapping byte order
    sal_Unicode notConst[sBuf.getLength()];
    memcpy( notConst, sBuf.getStr(), sizeof(sal_Unicode)*sBuf.getLength() );
    writeArray( notConst, sBuf.getLength() );
#else
    writeArray( sBuf.getStr(), sBuf.getLength() );
#endif
}

void BinaryOutputStream::writeCompressedUnicodeArray( const OUString& rString, bool bCompressed )
{
    if ( bCompressed )
         // ISO-8859-1 maps all byte values 0xHH to the same Unicode code point U+00HH
        writeCharArrayUC( rString, RTL_TEXTENCODING_ISO_8859_1 );
    else
        writeUnicodeArray( rString );
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
