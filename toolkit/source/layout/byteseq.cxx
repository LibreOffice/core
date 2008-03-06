/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: byteseq.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 11:43:43 $
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

/* We do not link to xmlscript, because that would introduce an extra
   toolkit dependency.  */
#include <xml_helper/xml_byteseq.cxx>

#include <stdio.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace layoutimpl
{

uno::Reference< io::XInputStream >
getFileAsStream( const uno::Reference< lang::XMultiServiceFactory > &xFactory,
                 const rtl::OUString &rName );

uno::Reference< io::XInputStream >
getFileAsStream( const uno::Reference< lang::XMultiServiceFactory > & /* xFactory */,
                 const rtl::OUString &rName )
{
    rtl::OString fname = rtl::OUStringToOString( rName, RTL_TEXTENCODING_UTF8 );
    // create the input stream
    FILE *f = ::fopen( fname, "rb" );
    if (f)
    {
        ::fseek( f, 0 ,SEEK_END );
        int nLength = ::ftell( f );
        ::fseek( f, 0, SEEK_SET );

        ByteSequence bytes( nLength );
        ::fread( bytes.getArray(), nLength, 1, f );
        ::fclose( f );

        return new xmlscript::BSeqInputStream( bytes );
    }

    throw Exception( OUString( RTL_CONSTASCII_USTRINGPARAM("### Cannot read file!") ),
                     Reference< XInterface >() );
}

} // namespace layoutimpl
