/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryoutputstream.cxx,v $
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

#include "oox/helper/binaryoutputstream.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/io/XOutputStream.hpp>
#include "oox/helper/binaryinputstream.hxx"
#include <string.h>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XOutputStream;

namespace oox {

// ============================================================================

BinaryOutputStream::BinaryOutputStream( const Reference< XOutputStream >& rxOutStrm, bool bAutoClose ) :
    BinaryStreamBase( rxOutStrm ),
    mxOutStrm( rxOutStrm ),
    mbAutoClose( bAutoClose )
{
}

BinaryOutputStream::~BinaryOutputStream()
{
    if( mbAutoClose )
        close();
}

void BinaryOutputStream::write( const Sequence< sal_Int8 >& rBuffer )
{
    try
    {
        OSL_ENSURE( mxOutStrm.is(), "BinaryOutputStream::write - invalid call" );
        mxOutStrm->writeBytes( rBuffer );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryOutputStream::write - stream read error" );
    }
}

void BinaryOutputStream::write( const void* pBuffer, sal_Int32 nBytes )
{
    if( nBytes > 0 )
    {
        maBuffer.realloc( nBytes );
        memcpy( maBuffer.getArray(), pBuffer, static_cast< size_t >( nBytes ) );
        write( maBuffer );
    }
}

void BinaryOutputStream::copy( BinaryInputStream& rInStrm, sal_Int64 nBytes )
{
    if( rInStrm.is() && (nBytes > 0) )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, 0x8000 );
        Sequence< sal_Int8 > aBuffer( nBufferSize );
        while( nBytes > 0 )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = rInStrm.read( aBuffer, nReadSize );
            write( aBuffer );
            if( nReadSize == nBytesRead )
                nBytes -= nReadSize;
            else
                nBytes = 0;
        }
    }
}

void BinaryOutputStream::close()
{
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->flush();
        mxOutStrm->closeOutput();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryOutputStream::close - closing output stream failed" );
    }
}

// ============================================================================

} // namespace oox

