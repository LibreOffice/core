/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: byteseq.cxx,v $
 * $Revision: 1.3 $
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
