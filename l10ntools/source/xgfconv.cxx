/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <stdio.h>

#include "export.hxx"
#include "utf8conv.hxx"

/*****************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"
#if defined(UNX) || defined(OS2)
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( argc != 3 ) {
        fprintf( stderr, "xgfconv InputFile OutputFile\n" );
        return ( 5 );
    }

    ByteString sInput( argv[ 1 ] );
    ByteString sOutput( argv[ 2 ] );

    SvFileStream aInput( String( sInput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    if ( !aInput.IsOpen()) {
        fprintf( stderr, "ERROR: Unable to open input file!\n" );
        return ( 5 );
    }

    SvFileStream aOutput( String( sOutput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_WRITE | STREAM_TRUNC );
    if ( !aOutput.IsOpen()) {
        fprintf( stderr, "ERROR: Unable to open output file!\n" );
        aInput.Close();
        return ( 5 );
    }

    ByteString sLine;
    sal_Bool bFirst = sal_True;
    while ( !aInput.IsEof()) {
        aInput.ReadLine( sLine );
        ByteString sLangId = sLine.GetToken( 0, '\t' );
        ByteString sFile = sLine.GetToken( 1, '\t' );
        ByteString sText = sLine.Copy( sLangId.Len() + sFile.Len() + 2 );

        sal_uInt16 nLangId = sLangId.ToInt32();
        CharSet aCharSet = Export::GetCharSet( nLangId );
        if ( aCharSet != 0xFFFF && sText.Len()) {
            sText = UTF8Converter::ConvertToUTF8( sText, aCharSet );
            ByteString sOutput = sFile;
            sOutput += "\t";
            sOutput += sText;
            if ( !bFirst ) {
                ByteString sEmpty;
                aOutput.WriteLine( sEmpty );
            }
            else
                bFirst = sal_False;
            aOutput.Write( sOutput.GetBuffer(), sOutput.Len());
        }
    }
    aInput.Close();
    aOutput.Close();
    return ( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
