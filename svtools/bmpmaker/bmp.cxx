/*************************************************************************
 *
 *  $RCSfile: bmp.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ka $ $Date: 2001-05-10 13:57:08 $
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

#include <cstdio>
#include <csignal>
#include <vcl/svapp.hxx>

#include "solar.hrc"
#include "filedlg.hxx"
#include "bmpcore.hxx"
#include "bmp.hrc"

// ----------
// - BmpApp -
// ----------

class BmpApp : public Application, public BmpCreator
{
private:

    String          aOutputFileName;
    BYTE            cExitCode;

    BOOL            GetCommandOption( const String& rSwitch, String& rSwitchParam );
    void            SetExitCode( BYTE cExit )
                    {
                        if( ( EXIT_NOERROR == cExitCode ) || ( cExit != EXIT_NOERROR ) )
                            cExitCode = cExit;
                    }

    virtual void    Message( const String& rText, BYTE cExitCode );

public:

    void            Main();
};

// -----------------------------------------------------------------------

BOOL BmpApp::GetCommandOption( const String& rSwitch, String& rFollowingParam )
{
    BOOL bRet = FALSE;

    for( USHORT i = 0, nCount = GetCommandLineParamCount(); ( i < nCount ) && !bRet; i++ )
    {
        String  aTestStr( '-' );

        for( long n = 0; ( n < 2 ) && !bRet; n++ )
        {
            aTestStr += rSwitch;

            if( aTestStr.CompareIgnoreCaseToAscii( GetCommandLineParam( i ) ) == COMPARE_EQUAL )
            {
                bRet = TRUE;

                if( i < ( nCount - 1 ) )
                    rFollowingParam = GetCommandLineParam( i + 1 );
                else
                    rFollowingParam = String();
            }

            if( 0 == n )
                aTestStr = '/';
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

void BmpApp::Message( const String& rText, BYTE cExitCode )
{
    if( EXIT_NOERROR != cExitCode )
        SetExitCode( cExitCode );

    ByteString aText( rText, RTL_TEXTENCODING_UTF8 );
#ifdef UNX
    aText.Append( "\r\n" );
#else
    aText.Append( "\n" );
#endif
    fprintf( stderr, aText.GetBuffer() );
}

// -----------------------------------------------------------------------------

void BmpApp::Main( )
{
    // Param1: filename of *.srs input file with image lists
    // Param2: path to local bitmaps
    // Param3: output directory
    // Param4: Langugage dependent directory

    String  aOutName;
    int     nCmdCount = GetCommandLineParamCount();
    USHORT  nCurCmd = 0;

    cExitCode = EXIT_NOERROR;

    if( nCmdCount >= 4 )
    {
        LangInfo            aLangInfo;
        const String        aSrsName( GetCommandLineParam( nCurCmd++ ) );
        const String        aInName( GetCommandLineParam( nCurCmd++ ) );
        ByteString          aLangDir;

        aOutName = GetCommandLineParam( nCurCmd++ );
        aLangDir = ByteString( GetCommandLineParam( nCurCmd++ ), RTL_TEXTENCODING_ASCII_US );

        GetCommandOption( 'f', aOutputFileName );

        // create LangInfo
        memcpy( aLangInfo.maLangDir, aLangDir.GetBuffer(), aLangDir.Len() + 1 );
        aLangInfo.mnLangNum = (USHORT) DirEntry( aOutName ).GetName().ToInt32();

        Create( aSrsName, aInName, aOutName, aLangInfo );
    }
    else
        Message( String( RTL_CONSTASCII_USTRINGPARAM( "ERROR: invalid command line parameters" ) ), EXIT_COMMONERROR );

    if( ( EXIT_NOERROR == cExitCode ) && aOutputFileName.Len() && aOutName.Len() )
    {
        SvFileStream    aOStm( aOutputFileName, STREAM_WRITE | STREAM_TRUNC );
        ByteString      aStr( "Successfully generated ImageList(s) in: " );

        aOStm.WriteLine( aStr.Append( ByteString( aOutName, RTL_TEXTENCODING_UTF8 ) ) );
        aOStm.Close();
    }

    if ( EXIT_NOERROR != cExitCode )
        raise( SIGABRT );
    return;
}

// ---------------
// - Application -
// ---------------

BmpApp aBmpApp;
