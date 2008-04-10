/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txtconv.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_transex3.hxx"
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// local includes
#include "utf8conv.hxx"

extern void ConvertHalfwitdhToFullwidth( String& rString );

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "\n" );
    fprintf( stdout, "txtconv (c)2001 by StarOffice Entwicklungs GmbH\n" );
    fprintf( stdout, "===============================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "txtconv converts textfiles from or to UTF-8\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: txtconv -t|-f charset filename (destinationfile)\n" );
    fprintf( stdout, "Switches: -t   => conversion from charset to UTF-8\n" );
    fprintf( stdout, "          -f   => conversion from UTF-8 to charset\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Allowed charsets:\n" );
    fprintf( stdout, "          MS_932  => Japanese\n" );
    fprintf( stdout, "          MS_936  => Chinese Simplified\n" );
    fprintf( stdout, "          MS_949  => Korean\n" );
    fprintf( stdout, "          MS_950  => Chinese Traditional\n" );
    fprintf( stdout, "          MS_1250 => East Europe\n" );
    fprintf( stdout, "          MS_1251 => Cyrillic\n" );
    fprintf( stdout, "          MS_1252 => West Europe\n" );
    fprintf( stdout, "          MS_1253 => Greek\n" );
    fprintf( stdout, "          MS_1254 => Turkish\n" );
    fprintf( stdout, "          MS_1255 => Hebrew\n" );
    fprintf( stdout, "          MS_1256 => Arabic\n" );
    fprintf( stdout, "          HW2FW   => Only with -t, converts half to full width katakana" );
    fprintf( stdout, "\n" );
}

/*****************************************************************************/
#if defined( UNX )
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if (( argc != 4 ) && ( argc != 5 )) {
        Help();
        exit ( 0 );
    }

    if ( ByteString( argv[ 1 ] ) == "-t" || ByteString( argv[ 1 ] ) == "-f" ) {
        rtl_TextEncoding nEncoding = RTL_TEXTENCODING_MS_1252;

        BOOL bHW2FW = FALSE;

        ByteString sCharset( argv[ 2 ] );
        sCharset.ToUpperAscii();

        if      ( sCharset == "MS_932" )    nEncoding = RTL_TEXTENCODING_MS_932;
        else if ( sCharset == "MS_936" )    nEncoding = RTL_TEXTENCODING_MS_936;
        else if ( sCharset == "MS_949" )    nEncoding = RTL_TEXTENCODING_MS_949;
        else if ( sCharset == "MS_950" )    nEncoding = RTL_TEXTENCODING_MS_950;
        else if ( sCharset == "MS_1250" )   nEncoding = RTL_TEXTENCODING_MS_1250;
        else if ( sCharset == "MS_1251" )   nEncoding = RTL_TEXTENCODING_MS_1251;
        else if ( sCharset == "MS_1252" )   nEncoding = RTL_TEXTENCODING_MS_1252;
        else if ( sCharset == "MS_1253" )   nEncoding = RTL_TEXTENCODING_MS_1253;
        else if ( sCharset == "MS_1254" )   nEncoding = RTL_TEXTENCODING_MS_1254;
        else if ( sCharset == "MS_1255" )   nEncoding = RTL_TEXTENCODING_MS_1255;
        else if ( sCharset == "MS_1256" )   nEncoding = RTL_TEXTENCODING_MS_1256;
        else if ( sCharset == "MS_1257" )   nEncoding = RTL_TEXTENCODING_MS_1257;
        else if (( sCharset == "HW2FW" ) && ( ByteString( argv[ 1 ] ) == "-t" )) bHW2FW = TRUE;

        else {
            Help();
            exit ( 1 );
        }

        DirEntry aSource = DirEntry( String( argv[ 3 ], RTL_TEXTENCODING_ASCII_US ));
        if ( !aSource.Exists()) {
            fprintf( stderr, "\nERROR: File %s not found!\n\n", ByteString( argv[ 3 ] ).GetBuffer());
            exit ( 2 );
        }

        String sOutput;
        SvFileStream aOutput;
        if ( argc == 5 ) {
            sOutput= String( argv[ 4 ], RTL_TEXTENCODING_ASCII_US );
            aOutput.Open( sOutput, STREAM_STD_WRITE | STREAM_TRUNC );
            if ( !aOutput.IsOpen()) {
                fprintf( stderr, "\nERROR: Could not open output file %s!\n\n", argv[ 4 ]);
                exit ( 3 );
            }
        }

        String sGSI( argv[ 3 ], RTL_TEXTENCODING_ASCII_US );
        SvFileStream aGSI( sGSI, STREAM_STD_READ );
        if ( !aGSI.IsOpen()) {
            fprintf( stderr, "\nERROR: Could not open input file %s!\n\n", argv[ 3 ]);
            exit ( 3 );
        }

        ByteString sGSILine;
        while ( !aGSI.IsEof()) {

            aGSI.ReadLine( sGSILine );
            if ( bHW2FW ) {
                String sConverter( sGSILine, RTL_TEXTENCODING_UTF8 );
                ConvertHalfwitdhToFullwidth( sConverter );
                sGSILine = ByteString( sConverter, RTL_TEXTENCODING_UTF8 );
            }
            else {
                if ( ByteString( argv[ 1 ] ) == "-t" )
                    sGSILine = UTF8Converter::ConvertToUTF8( sGSILine, nEncoding );
                else
                    sGSILine = UTF8Converter::ConvertFromUTF8( sGSILine, nEncoding );
            }

            if ( aOutput.IsOpen())
                aOutput.WriteLine( sGSILine );
            else
                fprintf( stdout, "%s\n", sGSILine.GetBuffer());
        }

        aGSI.Close();
        if ( aOutput.IsOpen())
            aOutput.Close();
    }
    else {
        Help();
        exit( 1 );
    }

    return 0;
}
