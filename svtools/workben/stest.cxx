/*************************************************************************
 *
 *  $RCSfile: stest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:07 $
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
#ifdef VCL
#include <svmedit.hxx>
#endif

#ifndef _TXTCMP_HXX //autogen
#include <txtcmp.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

class MyApp : public Application
{
public:
#ifndef VCL
    virtual void Main( int, char*[] );
#else
    virtual void Main( );
#endif
};

class SearchWindow : public WorkWindow
{
    PushButton      aPB;
    FixedText       aFT1, aFT2, aFT3;
    MultiLineEdit   aEText, aESrch;
    RadioButton     aModeN, aModeR, aModeL;
    SearchParam     aParam;

public:
                    SearchWindow();

    DECL_LINK( ClickHdl, Button * );
};

// --- SearchWindow::SearchWindow() ------------------------------------

SearchWindow::SearchWindow() :
                WorkWindow( NULL, WinBits( WB_APP | WB_STDWORK )),
    aPB( this, WinBits( 0 )),
    aFT1( this, WinBits( 0 )),
    aFT2( this, WinBits( 0 )),
    aFT3( this, WinBits( 0 )),
    aEText( this, WinBits( WB_BORDER )),
    aESrch( this, WinBits( WB_BORDER )),
    aModeN( this, WinBits( 0 )),
    aModeR( this, WinBits( 0 )),
    aModeL( this, WinBits( 0 )),
    aParam( "" )
{
    aPB.SetClickHdl( LINK( this, SearchWindow, ClickHdl ));
    aModeN.SetClickHdl( LINK( this, SearchWindow, ClickHdl ));
    aModeR.SetClickHdl( LINK( this, SearchWindow, ClickHdl ));
    aModeL.SetClickHdl( LINK( this, SearchWindow, ClickHdl ));

    SetMapMode( MapMode( MAP_APPFONT ));
    SetSizePixel( LogicToPixel( Size( 300, 180 ) ) );

    aEText.SetPosSizePixel( LogicToPixel( Point( 0, 22 )), LogicToPixel(Size( 270, 32 )) );
    aFT1.SetPosSizePixel( LogicToPixel( Point( 0, 10 )), LogicToPixel(Size( 18, 11 )) );
    aFT2.SetPosSizePixel( LogicToPixel( Point( 0, 60 )), LogicToPixel(Size( 24, 10 )) );
    aESrch.SetPosSizePixel( LogicToPixel( Point( 0, 70 )), LogicToPixel(Size( 270, 24 )) );
    aPB.SetPosSizePixel( LogicToPixel( Point( 223, 139 )), LogicToPixel(Size( 48, 12 )) );
    aFT3.SetPosSizePixel( LogicToPixel( Point( 0, 104 )), LogicToPixel(Size( 270, 15 )) );
    aModeN.SetPosSizePixel( LogicToPixel( Point( 5, 116 ) ), LogicToPixel( Size( 40, 12 ) ) );
    aModeR.SetPosSizePixel( LogicToPixel( Point( 5, 126 ) ), LogicToPixel( Size( 40, 12 ) ) );
    aModeL.SetPosSizePixel( LogicToPixel( Point( 5, 136 ) ), LogicToPixel( Size( 40, 12 ) ) );

    aEText.Show();
    aFT1.Show();
    aFT2.Show();
    aESrch.Show();
    aPB.Show();
    aFT3.Show();
    aModeN.Show();
    aModeR.Show();
    aModeL.Show();

    aFT3.SetText( "gefunden:" );
    aFT1.SetText( "Text:" );
    aFT2.SetText( "Suche:" );
    aPB.SetText( "starte Suche" );
    aModeN.SetText( "normal" );
    aModeR.SetText( "RegExp" );
    aModeL.SetText( "LevDis" );

    SetText( "Such-Demo" );
}


// --- SearchWindow::SearchSelectHdl() ---------------------------------

IMPL_LINK( SearchWindow, ClickHdl, Button *, pButton )
{
    if( pButton == &aPB )
    {
        String sText( aEText.GetText() );
        String sSrch( aESrch.GetText() );

/*      InfoBox( this, String( "T: " ) + sText +
                       String( "\nS: " ) + sSrch ).Execute();
*/
        BOOL bRet = FALSE;
        USHORT nStt = 0, nEnd = sText.Len();

        {
            aParam.SetSrchStr( sSrch );
            SearchText aSrchText( aParam, GetpApp()->GetAppInternational() );
            bRet = aSrchText.SearchFrwrd( sText, &nStt, &nEnd );

//          BOOL SearchBkwrd( const String &rStr, USHORT* pStart, USHORT* pEnde );
        }

        String sFound( "gefunden" );
        if( !bRet )
            sFound.Insert( "nicht ", 0 );

        sFound += ": S<";
        sFound += nStt;
        sFound += "> E<";
        sFound += nEnd;
        sFound += '>';

        if( bRet )
        {
            sFound += '<';
            sFound += sText.Copy( nStt, nEnd - nStt +1 );
            sFound += '>';
        }

        aFT3.SetText( sFound );
    }
    else if( pButton == &aModeN )
    {
        aParam.SetSrchType( SearchParam::SRCH_NORMAL );
    }
    else if( pButton == &aModeR )
    {
        aParam.SetSrchType( SearchParam::SRCH_REGEXP );
    }
    else if( pButton == &aModeL )
    {
        aParam.SetSrchType( SearchParam::SRCH_LEVDIST );
    }
    return 0;
}


// --- MyApp::Main() -----------------------------------------------

#ifndef VCL
void MyApp::Main( int, char*[] )
#else
void MyApp::Main( )
#endif
{
    SearchWindow*   pSearchWindow = new SearchWindow;
    pSearchWindow->Show();
    Execute();
    delete pSearchWindow;

}

// --- aMyApp ------------------------------------------------------

MyApp aMyApp;
