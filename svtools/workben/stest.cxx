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
#include "precompiled_svtools.hxx"
#include <svtools/svmedit.hxx>
#include <txtcmp.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/wrkwin.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>

class MyApp : public Application
{
public:
    virtual void Main( );
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
        sal_Bool bRet = sal_False;
        sal_uInt16 nStt = 0, nEnd = sText.Len();

        {
            aParam.SetSrchStr( sSrch );
            SearchText aSrchText( aParam, GetpApp()->GetAppInternational() );
            bRet = aSrchText.SearchFrwrd( sText, &nStt, &nEnd );

//          sal_Bool SearchBkwrd( const String &rStr, sal_uInt16* pStart, sal_uInt16* pEnde );
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

void MyApp::Main( )
{
    SearchWindow*   pSearchWindow = new SearchWindow;
    pSearchWindow->Show();
    Execute();
    delete pSearchWindow;

}

// --- aMyApp ------------------------------------------------------

MyApp aMyApp;
