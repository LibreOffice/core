#include <stdio.h>

#include "export.hxx"
#include "utf8conv.hxx"

/*****************************************************************************/
#if defined( UNX ) || defined( MAC )
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( argc != 2 ) {
        fprintf( stderr, "xgfconv InputFile\n" );
        return ( 5 );
    }

    ByteString sInput( argv[ 1 ] );

    SvFileStream aInput( String( sInput, RTL_TEXTENCODING_ASCII_US ), STREAM_STD_READ );
    if ( !aInput.IsOpen())
        return ( 5 );

    ByteString sLine;
    while ( !aInput.IsEof()) {
        aInput.ReadLine( sLine );
        ByteString sLangId = sLine.GetToken( 0, '\t' );
        ByteString sFile = sLine.GetToken( 1, '\t' );
        ByteString sText = sLine.Copy( sLangId.Len() + sFile.Len() + 2 );

        USHORT nLangId = sLangId.ToInt32();
        CharSet aCharSet = Export::GetCharSet( nLangId );
        if ( aCharSet != 0xFFFF ) {
            sText = UTF8Converter::ConvertToUTF8( sText, aCharSet );
            fprintf( stdout, "%s\t%s\n", sFile.GetBuffer(), sText.GetBuffer());
        }
    }
    aInput.Close();
    return ( 0 );
}

