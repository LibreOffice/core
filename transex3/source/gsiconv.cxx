/*************************************************************************
 *
 *  $RCSfile: gsiconv.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nf $ $Date: 2000-11-03 16:07:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// local includes
#include "utf8conv.hxx"

/*****************************************************************************/
void Help()
/*****************************************************************************/
{
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsiconv (c)1999 by StarOffice Entwicklungs GmbH\n" );
    fprintf( stdout, "===============================================\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "gsiconv converts strings in GSI-Files (Gutschmitt Interface) from or to UTF-8\n" );
    fprintf( stdout, "\n" );
    fprintf( stdout, "Syntax: gsiconv (-t|-f langid charset)|(-p n) filename\n" );
    fprintf( stdout, "Switches: -t   => conversion from charset to UTF-8\n" );
    fprintf( stdout, "          -f   => conversion from UTF-8 to charset\n" );
    fprintf( stdout, "          -p n => creates several files with ca. n lines\n" );
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
    fprintf( stdout, "\n" );
    fprintf( stdout, "Allowed langids:\n" );
    fprintf( stdout, "          1  => ENGLISH_US\n" );
    fprintf( stdout, "          3  => PORTUGUESE \n" );
    fprintf( stdout, "          4  => GERMAN_DE (new german style)\n" );
    fprintf( stdout, "          7  => RUSSIAN\n" );
    fprintf( stdout, "          30 => GREEK\n" );
    fprintf( stdout, "          31 => DUTCH\n" );
    fprintf( stdout, "          33 => FRENCH\n" );
    fprintf( stdout, "          34 => SPANISH\n" );
    fprintf( stdout, "          35 => FINNISH\n" );
    fprintf( stdout, "          36 => HUNGARIAN\n" );
    fprintf( stdout, "          39 => ITALIAN\n" );
    fprintf( stdout, "          42 => CZECH\n" );
    fprintf( stdout, "          44 => ENGLISH (UK)\n" );
    fprintf( stdout, "          45 => DANISH\n" );
    fprintf( stdout, "          46 => SWEDISH\n" );
    fprintf( stdout, "          47 => NORWEGIAN\n" );
    fprintf( stdout, "          49 => GERMAN (old german style)\n" );
    fprintf( stdout, "          55 => PORTUGUESE_BRAZILIAN\n" );
    fprintf( stdout, "          81 => JAPANESE\n" );
    fprintf( stdout, "          82 => KOREAN\n" );
    fprintf( stdout, "          86 => CHINESE_SIMPLIFIED\n" );
    fprintf( stdout, "          88 => CHINESE_TRADITIONAL\n" );
    fprintf( stdout, "          90 => TURKISH\n" );
    fprintf( stdout, "          96 => ARABIC\n" );
    fprintf( stdout, "          97 => HEBREW\n" );
       fprintf( stdout, "\n" );
}

/*****************************************************************************/
#if defined( UNX ) || defined( MAC )
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if (( argc != 5 ) && ( argc != 4 )) {
        Help();
        exit ( 0 );
    }

    if ( argc == 4 ) {
        if ( ByteString( argv[ 1 ] ) == "-p" ) {

            DirEntry aSource = DirEntry( String( argv[ 3 ], RTL_TEXTENCODING_ASCII_US ));
            if ( !aSource.Exists()) {
                fprintf( stderr, "\nERROR: GSI-File %s not found!\n\n", ByteString( argv[ 3 ] ).GetBuffer());
                exit ( 2 );
            }

            DirEntry aOutput( aSource );

            String sBase = aOutput.GetBase();
            String sExt = aOutput.GetExtension();

            String sGSI( argv[ 3 ], RTL_TEXTENCODING_ASCII_US );
            SvFileStream aGSI( sGSI, STREAM_STD_READ  );
            if ( !aGSI.IsOpen()) {
                fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( argv[ 3 ] ).GetBuffer());
                exit ( 3 );
            }

            ULONG nMaxLines = ByteString( argv[ 2 ] ).ToInt64();
            if ( !nMaxLines ) {
                fprintf( stderr, "\nERROR: Linecount must be at least 1!\n\n" );
                exit ( 3 );
            }

            ByteString sGSILine;
            ByteString sOldId;
            ULONG nLine = 0;
            ULONG nOutputFile = 1;

            String sOutput( sBase );
            sOutput += String( "_", RTL_TEXTENCODING_ASCII_US );
            sOutput += String::CreateFromInt64( nOutputFile );
            if ( sExt.Len()) {
                sOutput += String( ".", RTL_TEXTENCODING_ASCII_US );
                sOutput += sExt;
            }
            nOutputFile ++;

            aOutput.SetName( sOutput );
            SvFileStream aOutputStream( aOutput.GetFull(), STREAM_STD_WRITE | STREAM_TRUNC );

            while ( !aGSI.IsEof()) {

                aGSI.ReadLine( sGSILine );
                ByteString sId( sGSILine );
                sId.SearchAndReplaceAll( "($$)", "\t" );
                sId = sId.GetToken( 0, '\t' );

                nLine++;

                if (( nLine >= nMaxLines ) && ( sId != sOldId )) {
                    aOutputStream.Close();

                    ByteString sText( aOutput.GetFull(), gsl_getSystemTextEncoding());
                    sText += " with ";
                    sText += ByteString::CreateFromInt64( nLine );
                    sText += " lines written.";

                    fprintf( stdout, "%s\n", sText.GetBuffer());
                    String sOutput( sBase );
                    sOutput += String( "_", RTL_TEXTENCODING_ASCII_US );
                    sOutput += String::CreateFromInt64( nOutputFile );
                    if ( sExt.Len()) {
                        sOutput += String( ".", RTL_TEXTENCODING_ASCII_US );
                        sOutput += sExt;
                    }
                    nOutputFile ++;

                    aOutput.SetName( sOutput );

                    aOutputStream.Open( aOutput.GetFull(), STREAM_STD_WRITE | STREAM_TRUNC );
                    nLine = 0;
                }

                aOutputStream.WriteLine( sGSILine );

                sOldId = sId;
            }

            aGSI.Close();
            aOutputStream.Close();

            ByteString sText( aOutput.GetFull(), RTL_TEXTENCODING_ASCII_US );
            sText += " with ";
            sText += ByteString::CreateFromInt64( nLine );
            sText += " lines written.";
        }
        else {
            Help();
            exit( 1 );
        }
    }
    else {
        if ( ByteString( argv[ 1 ] ) == "-t" || ByteString( argv[ 1 ] ) == "-f" ) {
            rtl_TextEncoding nEncoding;

            ByteString sCurLangId( argv[ 2 ] );

            ByteString sCharset( argv[ 3 ] );
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

            else {
                Help();
                exit ( 1 );
            }

            DirEntry aSource = DirEntry( String( argv[ 4 ], RTL_TEXTENCODING_ASCII_US ));
            if ( !aSource.Exists()) {
                fprintf( stderr, "\nERROR: GSI-File %s not found!\n\n", ByteString( argv[ 3 ] ).GetBuffer());
                exit ( 2 );
            }

            String sGSI( argv[ 4 ], RTL_TEXTENCODING_ASCII_US );
            SvFileStream aGSI( sGSI, STREAM_STD_READ );
            if ( !aGSI.IsOpen()) {
                fprintf( stderr, "\nERROR: Could not open GSI-File %s!\n\n", ByteString( argv[ 3 ] ).GetBuffer());
                exit ( 3 );
            }

            ByteString sGSILine;
            while ( !aGSI.IsEof()) {

                aGSI.ReadLine( sGSILine );
                ByteString sLangId( sGSILine );
                sLangId.SearchAndReplaceAll( "($$)", "\t" );
                sLangId = sLangId.GetToken( 2, '\t' );
                if ( sLangId == sCurLangId ) {
                    if ( ByteString( argv[ 1 ] ) == "-t" )
                        sGSILine = UTF8Converter::ConvertToUTF8( sGSILine, nEncoding );
                    else
                        sGSILine = UTF8Converter::ConvertFromUTF8( sGSILine, nEncoding );
                }

                fprintf( stdout, "%s\n", sGSILine.GetBuffer());
            }

            aGSI.Close();
        }
        else {
            Help();
            exit( 1 );
        }
    }
    return 0;
}
