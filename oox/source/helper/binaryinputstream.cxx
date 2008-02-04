/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binaryinputstream.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:36:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

