/*************************************************************************
 *
 *  $RCSfile: g2g.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-21 11:11:43 $
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
#include <signal.h>
#include <ctype.h>

#include <sal/main.h>

#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include "filter.hxx"

#define EXIT_NOERROR                0x00000000
#define EXIT_COMMONERROR            0x00000001
#define EXIT_INVALID_FILE           0x00000002
#define EXIT_INVALID_GRAPHICFILTER  0x00000004
#define EXIT_INVALID_INPUTGRAPHIC   0x00000008
#define EXIT_OUTPUTERROR            0x00000010

#define LOWERHEXTONUM( _def_Char )  (((_def_Char)<='9') ? ((_def_Char)-'0') : ((_def_Char)-'a'+10))

// ----------
// - G2GApp -
// ----------

class G2GApp
{
private:

    BYTE            cExitCode;

    void            ShowUsage();
    BOOL            GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rParam );
    void            SetExitCode( BYTE cExit ) { if( ( EXIT_NOERROR == cExitCode ) || ( cExit != EXIT_NOERROR ) ) cExitCode = cExit; }

    virtual void    Message( const String& rText, BYTE cExitCode = EXIT_NOERROR );

public:

                    G2GApp();
                    ~G2GApp();

    int             Start( const ::std::vector< String >& rArgs );
};

// -----------------------------------------------------------------------

G2GApp::G2GApp()
{
}

// -----------------------------------------------------------------------

G2GApp::~G2GApp()
{
}

// -----------------------------------------------------------------------

BOOL G2GApp::GetCommandOption( const ::std::vector< String >& rArgs, const String& rSwitch, String& rParam )
{
    BOOL bRet = FALSE;

    for( int i = 0, nCount = rArgs.size(); ( i < nCount ) && !bRet; i++ )
    {
        String  aTestStr( '-' );

        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.CompareIgnoreCaseToAscii( rArgs[ i ] ) == COMPARE_EQUAL )
            {
                bRet = TRUE;

                if( i < ( nCount - 1 ) )
                    rParam = rArgs[ i + 1 ];
                else
                    rParam = String();
            }

            if( 0 == n )
                aTestStr = '/';
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

void G2GApp::Message( const String& rText, BYTE cExitCode )
{
    if( EXIT_NOERROR != cExitCode )
        SetExitCode( cExitCode );

    ByteString aText( rText, RTL_TEXTENCODING_UTF8 );
    aText.Append( "\r\n" );
    fprintf( stderr, aText.GetBuffer() );
}

// -----------------------------------------------------------------------------

void G2GApp::ShowUsage()
{
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Usage:" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    g2g inputfile outputfile -format exportformat -filterpath path [ -# RRGGBB ]" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Options:" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "   -format       short name of export filter to use ( e.g. gif, png, jpg, ... )" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "   -filterpath   path to externally loaded filter libraries" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "   -#            hex value of color to be set transparent in export file (optional)" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "Examples:" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    g2g /home/test.bmp /home/test.jpg -format jpg -filterpath /home/filter" ) ) );
    Message( String( RTL_CONSTASCII_USTRINGPARAM( "    g2g /home/test.bmp /home/test.gif -format gif -filterpath /home/filter -# C0C0C0" ) ) );
}

// -----------------------------------------------------------------------------

int G2GApp::Start( const ::std::vector< String >& rArgs )
{
    int     nCmdCount = rArgs.size();
    USHORT  nCurCmd = 0;

    cExitCode = EXIT_NOERROR;

    if( nCmdCount >= 6 )
    {
        GraphicFilter   aFilter( sal_False );
        String          aInFile, aOutFile, aFilterStr, aFilterPath, aTransColStr;

        aInFile = rArgs[ nCurCmd++ ];
        aOutFile = rArgs[ nCurCmd++ ];
        GetCommandOption( rArgs, String( RTL_CONSTASCII_USTRINGPARAM( "format" ) ), aFilterStr );
        GetCommandOption( rArgs, String( RTL_CONSTASCII_USTRINGPARAM( "filterpath" ) ), aFilterPath );
        GetCommandOption( rArgs, '#', aTransColStr );

        aFilter.SetFilterPath( aFilterPath );

        if( aInFile.Len() && aOutFile.Len() && aFilterStr.Len() )
        {
            const USHORT nExportFilter = aFilter.GetExportFormatNumberForShortName( aFilterStr );

            if( GRFILTER_FORMAT_NOTFOUND == nExportFilter )
                Message( String( RTL_CONSTASCII_USTRINGPARAM( "invalid graphic filter" ) ), EXIT_INVALID_GRAPHICFILTER );
            else
            {
                if( DirEntry( aInFile ).Exists() )
                {
                    SvFileStream    aInStm( aInFile, STREAM_READ );
                    Graphic         aGraphic;
                    const GfxLink   aGfxLink;

                    aGraphic.SetLink( aGfxLink );

                    if( aFilter.ImportGraphic( aGraphic, aInFile, aInStm ) == GRFILTER_OK )
                    {
                        SvFileStream aOutStm( aOutFile, STREAM_WRITE | STREAM_TRUNC );

                        if( ( aTransColStr.Len() == 6 ) && aFilter.IsExportPixelFormat( nExportFilter ) )
                        {
                            ByteString  aHexStr( aTransColStr, RTL_TEXTENCODING_ASCII_US );
                            sal_Bool    bHex = sal_True;

                            aHexStr.ToLowerAscii();

                            for( sal_uInt16 i = 0; ( i < 6 ) && bHex; i++ )
                                if( !isxdigit( aHexStr.GetChar( i ) ) )
                                    bHex = sal_False;

                            if( bHex )
                            {
                                const BYTE cTransR = ( LOWERHEXTONUM( aHexStr.GetChar( 0 ) ) << 4 ) | LOWERHEXTONUM( aHexStr.GetChar( 1 ) );
                                const BYTE cTransG = ( LOWERHEXTONUM( aHexStr.GetChar( 2 ) ) << 4 ) | LOWERHEXTONUM( aHexStr.GetChar( 3 ) );
                                const BYTE cTransB = ( LOWERHEXTONUM( aHexStr.GetChar( 4 ) ) << 4 ) | LOWERHEXTONUM( aHexStr.GetChar( 5 ) );

                                BitmapEx    aBmpEx( aGraphic.GetBitmapEx() );
                                Bitmap      aOldBmp( aBmpEx.GetBitmap() );
                                Bitmap      aOldMask( aBmpEx.GetMask() );
                                Bitmap      aNewMask( aOldBmp.CreateMask( Color( cTransR, cTransG, cTransB ) ) );

                                if( !aOldMask.IsEmpty() )
                                    aNewMask.CombineSimple( aOldMask, BMP_COMBINE_OR );

                                aGraphic = BitmapEx( aOldBmp, aNewMask );
                            }
                        }

                        aFilter.ExportGraphic( aGraphic, aOutFile, aOutStm, nExportFilter );

                        if( aOutStm.GetError() )
                            Message( String( RTL_CONSTASCII_USTRINGPARAM( "could not write output file" ) ), EXIT_OUTPUTERROR );
                    }
                    else
                        Message( String( RTL_CONSTASCII_USTRINGPARAM( "could import graphic" ) ), EXIT_INVALID_INPUTGRAPHIC );
                }
                else
                    Message( String( RTL_CONSTASCII_USTRINGPARAM( "invalid file(s)" ) ), EXIT_INVALID_FILE );
            }
        }
    }
    else
        ShowUsage();

    return cExitCode;
}

// --------
// - Main -
// --------

int main( int nArgCount, char* ppArgs[] )
{
    ::std::vector< String > aArgs;
    G2GApp                  aG2GApp;

    InitVCL( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >() );

    for( int i = 1; i < nArgCount; i++ )
        aArgs.push_back( String( ppArgs[ i ], RTL_TEXTENCODING_ASCII_US ) );

    return aG2GApp.Start( aArgs );
}
