/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_l10ntools.hxx"
#include <stdio.h>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// local includes
#include "utf8conv.hxx"

#define GSI_FILE_UNKNOWN        0x0000
#define GSI_FILE_OLDSTYLE       0x0001
#define GSI_FILE_L10NFRAMEWORK  0x0002

/*****************************************************************************/
sal_uInt16 GetGSIFileType( SvStream &rStream )
/*****************************************************************************/
{
    sal_uInt16 nFileType = GSI_FILE_UNKNOWN;

    sal_uLong nPos( rStream.Tell());
    rStream.Seek( STREAM_SEEK_TO_BEGIN );

    ByteString sLine;
    while( !rStream.IsEof() && !sLine.Len())
        rStream.ReadLine( sLine );

    if( sLine.Len()) {
        if( sLine.Search( "($$)" ) != STRING_NOTFOUND )
            nFileType = GSI_FILE_OLDSTYLE;
        else
            nFileType = GSI_FILE_L10NFRAMEWORK;
    }

    rStream.Seek( nPos );

    return nFileType;
}

/*****************************************************************************/
ByteString GetGSILineId( const ByteString &rLine, sal_uInt16 nFileType )
/*****************************************************************************/
{
    ByteString sId;
    switch ( nFileType ) {
        case GSI_FILE_OLDSTYLE:
            sId = rLine;
            sId.SearchAndReplaceAll( "($$)", "\t" );
            sId = sId.GetToken( 0, '\t' );
          break;

        case GSI_FILE_L10NFRAMEWORK:
            sId = rLine.GetToken( 0, '\t' );
            sId += "\t";
            sId += rLine.GetToken( 1, '\t' );
            sId += "\t";
            sId += rLine.GetToken( 4, '\t' );
            sId += "\t";
            sId += rLine.GetToken( 5, '\t' );
          break;
    }
    return sId;
}

/*****************************************************************************/
ByteString GetGSILineLangId( const ByteString &rLine, sal_uInt16 nFileType )
/*****************************************************************************/
{
    ByteString sLangId;
    switch ( nFileType ) {
        case GSI_FILE_OLDSTYLE:
            sLangId = rLine;
            sLangId.SearchAndReplaceAll( "($$)", "\t" );
            sLangId = sLangId.GetToken( 2, '\t' );
          break;

        case GSI_FILE_L10NFRAMEWORK:
            sLangId = rLine.GetToken( 9, '\t' );
          break;
    }
    return sLangId;
}

/*****************************************************************************/
void ConvertGSILine( sal_Bool bToUTF8, ByteString &rLine,
        rtl_TextEncoding nEncoding, sal_uInt16 nFileType )
/*****************************************************************************/
{
    switch ( nFileType ) {
        case GSI_FILE_OLDSTYLE:
            if ( bToUTF8 )
                rLine = UTF8Converter::ConvertToUTF8( rLine, nEncoding );
            else
                rLine = UTF8Converter::ConvertFromUTF8( rLine, nEncoding );
        break;

        case GSI_FILE_L10NFRAMEWORK: {
            ByteString sConverted;
            for ( sal_uInt16 i = 0; i < rLine.GetTokenCount( '\t' ); i++ ) {
                ByteString sToken = rLine.GetToken( i, '\t' );
                if (( i > 9 ) && ( i < 14 )) {
                    if( bToUTF8 )
                        sToken = UTF8Converter::ConvertToUTF8( sToken, nEncoding );
                    else
                        sToken = UTF8Converter::ConvertFromUTF8( sToken, nEncoding );
                }
                if ( i )
                    sConverted += "\t";
                sConverted += sToken;
            }
            rLine = sConverted;
        }
        break;
    }
}

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
#if defined(UNX) || defined(OS2)
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

            sal_uInt16 nFileType( GetGSIFileType( aGSI ));

            sal_uLong nMaxLines = (sal_uLong) ByteString( argv[ 2 ] ).ToInt64();
            if ( !nMaxLines ) {
                fprintf( stderr, "\nERROR: Linecount must be at least 1!\n\n" );
                exit ( 3 );
            }

            ByteString sGSILine;
            ByteString sOldId;
            sal_uLong nLine = 0;
            sal_uLong nOutputFile = 1;

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
                ByteString sId( GetGSILineId( sGSILine, nFileType ));

                nLine++;

                if (( nLine >= nMaxLines ) && ( sId != sOldId )) {
                    aOutputStream.Close();

                    ByteString sText( aOutput.GetFull(), gsl_getSystemTextEncoding());
                    sText += " with ";
                    sText += ByteString::CreateFromInt64( nLine );
                    sText += " lines written.";

                    fprintf( stdout, "%s\n", sText.GetBuffer());
                    String sOutput1( sBase );
                    sOutput1 += String( "_", RTL_TEXTENCODING_ASCII_US );
                    sOutput1 += String::CreateFromInt64( nOutputFile );
                    if ( sExt.Len()) {
                        sOutput1 += String( ".", RTL_TEXTENCODING_ASCII_US );
                        sOutput1 += sExt;
                    }
                    nOutputFile ++;

                    aOutput.SetName( sOutput1 );

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
            else if ( sCharset == "UTF8" )      nEncoding = RTL_TEXTENCODING_UTF8;

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
            sal_uInt16 nFileType( GetGSIFileType( aGSI ));

            ByteString sGSILine;
            while ( !aGSI.IsEof()) {

                aGSI.ReadLine( sGSILine );
                ByteString sLangId( GetGSILineLangId( sGSILine, nFileType ));
                if ( sLangId == sCurLangId )
                    ConvertGSILine(( ByteString( argv[ 1 ] ) == "-t" ), sGSILine, nEncoding, nFileType );

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
