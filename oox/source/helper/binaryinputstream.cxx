/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryinputstream.cxx,v $
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

#include "oox/helper/binaryinputstream.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <osl/diagnose.h>
#include <string.h>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XInputStream;

namespace oox {

// ============================================================================

BinaryInputStream::BinaryInputStream( const Reference< XInputStream >& rxInStrm, bool bAutoClose ) :
    BinaryStreamBase( rxInStrm ),
    mxInStrm( rxInStrm ),
    mbAutoClose( bAutoClose )
{
}

BinaryInputStream::~BinaryInputStream()
{
    if( mbAutoClose )
        close();
}

void BinaryInputStream::skip( sal_Int32 nBytes )
{
    try
    {
        OSL_ENSURE( mxInStrm.is(), "BinaryInputStream::skip - invalid call" );
        mxInStrm->skipBytes( nBytes );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryInputStream::skip - exception caught" );
    }
}

sal_Int32 BinaryInputStream::read( Sequence< sal_Int8 >& orBuffer, sal_Int32 nBytes )
{
    sal_Int32 nRet = 0;
    try
    {
        OSL_ENSURE( mxInStrm.is(), "BinaryInputStream::read - invalid call" );
        nRet = mxInStrm->readBytes( orBuffer, nBytes );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryInputStream::read - stream read error" );
    }
    return nRet;
}

sal_Int32 BinaryInputStream::read( void* opBuffer, sal_Int32 nBytes )
{
    sal_Int32 nRet = read( maBuffer, nBytes );
    if( nRet > 0 )
        memcpy( opBuffer, maBuffer.getConstArray(), static_cast< size_t >( nRet ) );
    return nRet;
}

void BinaryInputStream::close()
{
    if( mxInStrm.is() ) try
    {
        mxInStrm->closeInput();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryInputStream::close - closing input stream failed" );
    }
}

// ============================================================================

} // namespace oox

