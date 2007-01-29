/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ztool.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-01-29 14:36:12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include <stdio.h>

#include <tools/stream.hxx>
#include <tools/zcodec.hxx>

void show_usage()
{
    fputs("ztool usage: ztool <source> <dest>\n", stderr);
}

int
#ifdef WNT
__cdecl
#endif
main( int argc, char **argv )
{
    if ( argc != 3 )
    {
        show_usage();
        return -1;
    }
    UniString aInName = UniString::CreateFromAscii( argv[1] );
    UniString aOutName = UniString::CreateFromAscii( argv[2] );
    SvFileStream aInFile( aInName, STREAM_READ );
    SvFileStream aOutFile( aOutName, STREAM_WRITE | STREAM_TRUNC );

    ZCodec aCompressor;
    aCompressor.BeginCompression();
    aCompressor.Compress( aInFile, aOutFile );
    aCompressor.EndCompression();

    return 0;
}


