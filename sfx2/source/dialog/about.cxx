/*************************************************************************
 *
 *  $RCSfile: about.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-30 11:56:58 $
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

// include ---------------------------------------------------------------

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTDEMO_HXX //autogen
#include <svtools/svtdemo.hxx>
#endif
#ifndef _PVER_HXX //autogen
#include <svtools/pver.hxx>
#endif
#pragma hdrstop

#include "about.hxx"
#include "sfxresid.hxx"
#include "sfxdefs.hxx"
#include "app.hxx"
#include "inimgr.hxx"

#include "dialog.hrc"

// defines ---------------------------------------------------------------

#define SCROLL_OFFSET   ((long)2)
#define SPACE_OFFSET    ((long)5)

typedef unsigned short (*fncUshort)();
typedef const char* (*fncChar)();

// functions -------------------------------------------------------------

long CalcTextRows_Impl( const FixedText& rCtrl, long nTextWidth )
{
    long nRows = 0;
    String aText = rCtrl.GetText();
    while ( aText.Len() > 0 )
    {
        USHORT nBreakPos = rCtrl.GetTextBreak( aText, nTextWidth );
        USHORT nNewLinePos = aText.Search( '\n' );
        if ( nNewLinePos < nBreakPos )
        {
            USHORT nCutLen = nNewLinePos + 1;
            nRows++;
            if ( aText.GetChar( nNewLinePos + 1 ) == '\n' )
            {
                nCutLen++;
                nRows++;
            }
            aText = aText.Erase( 0, nCutLen );
        }
        else
        {
            nRows++;
            if ( nBreakPos != STRING_NOTFOUND )
            {
                // search for word break
                while ( aText.GetChar( nBreakPos ) != ' ' && nBreakPos > 0 )
                    nBreakPos--;
                aText = aText.Erase( 0, nBreakPos + 1 );
            }
            else
                break;
        }
    }

    return nRows;
}

// class AboutDialog -----------------------------------------------------

AboutDialog::AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr ) :

    SfxModalDialog  ( pParent,  rId ),

    aOKButton       ( this,     ResId( ABOUT_BTN_OK ) ),
    aAppLogo        (           ResId( ABOUT_BMP_LOGO ) ),
    aVersionText    ( this,     ResId( ABOUT_FTXT_VERSION ) ),
    aCopyrightText  ( this,     ResId( ABOUT_FTXT_COPYRIGHT ) ),
    aDeveloperAry   (           ResId( ABOUT_STR_DEVELOPER_ARY ) ),
    aDevVersionStr  ( rVerStr ),
    aAccelStr       (           ResId( ABOUT_STR_ACCEL ) ),

    aTimer  (),
    nOff    ( 0 ),
    nEnd    ( 0 ),
    bNormal ( TRUE )

{
    SfxIniManager* pIniMgr = SFX_INIMANAGER();

    // Transparenter Font
    Font aFont = GetFont();
    aFont.SetTransparent( TRUE );
    SetFont( aFont );

    // ggf. Spezial Version
    String aStr = aVersionText.GetText();
    String aMinor;
    USHORT nDemo = SvDemo::GetDemoKind( Application::GetAppName() );
    USHORT nProductVersion = ProductVersion::GetVersion().ToInt32();
    String aVersion( String::CreateFromInt32( nProductVersion / 10 ) );
    aVersion += 0x002E ; // 2Eh ^= '.'
    aVersion += ( String::CreateFromInt32( nProductVersion % 10 ) );
    String aOfficeVer = aVersion;
    aStr.SearchAndReplaceAscii( "$(VER)", aOfficeVer );
    aVersionText.SetText( aStr );

    // Initialisierung fuer Aufruf Entwickler
    if ( aAccelStr.Len() && ByteString(U2S(aAccelStr)).IsAlphaAscii() )
    {
        Accelerator *pAccel = 0, *pPrevAccel = 0, *pFirstAccel = 0;
        aAccelStr.ToUpperAscii();

        for ( USHORT i = 0; i < aAccelStr.Len(); ++i )
        {
            pPrevAccel = pAccel;
            pAccel = new Accelerator;
            aAccelList.Insert( pAccel, LIST_APPEND );
            USHORT nKey = aAccelStr.GetChar(i) - 'A' + KEY_A;
            pAccel->InsertItem( 1, KeyCode( nKey, KEY_MOD1 ) );
            if ( i > 0 )
                pPrevAccel->SetAccel( 1, pAccel );
            if ( i == 0 )
                pFirstAccel = pAccel;
        }
        pAccel->SetSelectHdl( LINK( this, AboutDialog, AccelSelectHdl ) );
        GetpApp()->InsertAccel( pFirstAccel );
    }

    // Hintergrund-Farbe Weiss
    Color aWhiteCol( COL_WHITE );
    Wallpaper aWall( aWhiteCol );
    SetBackground( aWall );
    Font aNewFont( aCopyrightText.GetFont() );
    aNewFont.SetTransparent( TRUE );

    aVersionText.SetFont( aNewFont );
    aCopyrightText.SetFont( aNewFont );

    aVersionText.SetBackground( aWall );
    aCopyrightText.SetBackground( aWall );

    Color aTextColor( COL_BLACK );
    aVersionText.SetControlForeground( aTextColor );
    aCopyrightText.SetControlForeground( aTextColor );

    // Gr"ossen und Positionen berechnen
    Size aAppLogoSiz = aAppLogo.GetSizePixel();
    Size aOutSiz = GetOutputSizePixel();
    Size aTextSize = Size( GetTextWidth( DEFINE_CONST_UNICODE( "StarOfficeAbout" ) ), GetTextHeight() );

    // Fenstergr"osse
    aOutSiz.Width() = aAppLogoSiz.Width();

    // Texte (Gr"osse und Position )
    Size a6Size = aVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nRowH = GetTextHeight();
    long nSpace = SPACE_OFFSET * 2;
    long nY = 0;
    Point aTextPos = aVersionText.GetPosPixel();
    aTextPos.X() = a6Size.Width() * 2;
    aTextPos.Y() = aAppLogoSiz.Height() + ( a6Size.Height() * 2 );
    nY = aTextPos.Y();
    aVersionText.SetPosPixel( aTextPos );
    Size aTxtSiz = aVersionText.GetSizePixel();
    aTxtSiz.Width() = aAppLogoSiz.Width() - ( a6Size.Width() * 4 );
    long nTextWidth = aTxtSiz.Width();
    long nSub = GetTextWidth( DEFINE_CONST_UNICODE( "x" ) );
    long nRows = CalcTextRows_Impl( aVersionText, nTextWidth - nSub );
    aTxtSiz.Height() = nRows * nRowH + nSpace;
    aVersionText.SetSizePixel( aTxtSiz );
    nY += aTxtSiz.Height() + ( a6Size.Height() / 3 );

    aTextPos = aCopyrightText.GetPosPixel();
    aTextPos.X() = a6Size.Width() * 2;
    aTextPos.Y() = nY;
    aCopyrightText.SetPosPixel( aTextPos );
    aTxtSiz = aCopyrightText.GetSizePixel();
    aTxtSiz.Width() = nTextWidth;
    nRows = CalcTextRows_Impl( aCopyrightText, nTextWidth - nSub );
    aTxtSiz.Height() = nRows * nRowH + nSpace;
    aCopyrightText.SetSizePixel( aTxtSiz );
    nY += aTxtSiz.Height() + ( a6Size.Height() / 2 );

    // OK-Button-Position (at the bottom and centered)
    Size aOKSiz = aOKButton.GetSizePixel();
    Point aOKPnt = aOKButton.GetPosPixel();
    aOKPnt.X() = ( aOutSiz.Width() - aOKSiz.Width() ) / 2;
    aOKPnt.Y() = nY + 8;
    aOKButton.SetPosPixel( aOKPnt );
    nY = aOKPnt.Y() + aOKSiz.Height() + a6Size.Height();
    aOutSiz.Height() = nY;
    SetOutputSizePixel( aOutSiz );

    FreeResource();

    // explizite Help-Id
    SetHelpId( SID_ABOUT );
}

// -----------------------------------------------------------------------

AboutDialog::~AboutDialog()
{
    // L"oschen des Entwickleraufrufs
    if ( aAccelList.Count() )
    {
        GetpApp()->RemoveAccel( aAccelList.First() );
        Accelerator* pAccel = aAccelList.Last();

        while ( pAccel )
        {
            delete pAccel;
            pAccel = aAccelList.Prev();
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( AboutDialog, TimerHdl, Timer *, pTimer )
{
    // Text scrollen
    nOff -= SCROLL_OFFSET;
    MapMode aMapMode( MAP_PIXEL, Point( 0, nOff ), Fraction( 1, 1 ), Fraction( 1, 1 ) );
    SetMapMode( aMapMode );
    Scroll( 0, -SCROLL_OFFSET );

    // Wenn der Text zuende gescrollt wurde, Dialogbox beenden
    if ( ( nOff * -1 ) > nEnd )
    {
        bNormal = TRUE;
        Close();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( AboutDialog, AccelSelectHdl, Accelerator *, pAccelerator )
{
    // init Timer
    aTimer.SetTimeoutHdl( LINK( this, AboutDialog, TimerHdl ) );

    // init scroll mode
    nOff = GetOutputSizePixel().Height();
    MapMode aMapMode( MAP_PIXEL, Point( 0, nOff ), Fraction( 1, 1 ), Fraction( 1, 1 ) );
    SetMapMode( aMapMode );
    bNormal = FALSE;

    // text always black on white
    Font aFont = GetFont();
    aFont.SetTransparent( TRUE );
    Color aBlackCol( COL_BLACK );
    aFont.SetColor( aBlackCol );
    SetFont( aFont );

    // start scroll Timer
    aTimer.SetTimeout( 60 );
    aTimer.Start();
    return 0;
}

// -----------------------------------------------------------------------

BOOL AboutDialog::Close()
{
    // stop Timer and finish the dialog
    aTimer.Stop();
    EndDialog( RET_OK );
    return( FALSE );
}

// -----------------------------------------------------------------------

void AboutDialog::Paint( const Rectangle& rRect )
{
    if ( bNormal )
    {
        // not in scroll mode
        DrawImage( Point(), aAppLogo );
        return;
    }

    long nPos = 0;
    long nW = GetOutputSizePixel().Width() / 2 - 5;
    Size aSize = Size( GetTextWidth( aDevVersionStr ), GetTextHeight() );
    USHORT nStartPos = 0;
    USHORT nEndPos = 0;
    Point aPnt( nW - ( aSize.Width() / 2 ), nPos );
    long nPos1 = aPnt.Y(), nPos2 = nPos1 + aSize.Height(), nTop = rRect.Top();

    if ( nPos1 <= nTop && nTop < nPos2 )
        DrawText( aPnt, aDevVersionStr );

    nPos += aSize.Height() + 3;
    USHORT nDevCnt = aDeveloperAry.Count();

    if ( nDevCnt )
    {
        for ( USHORT i = 0; i < nDevCnt; ++i )
        {
            String aStr = aDeveloperAry.GetString(i);
            long nVal = aDeveloperAry.GetValue(i);

            if ( nVal )
            {
                // Versionsnummern gibt es nur in den fetten Zeilen
                USHORT nProductVersion = ProductVersion::GetVersion().ToInt32();
                String aVersion = String::CreateFromInt32( nProductVersion / 10 );
                aVersion += '.';
                aVersion += String::CreateFromInt32( nProductVersion % 10 );
                USHORT nSPos = aStr.SearchAndReplaceAscii( "$(VER)", aVersion );
                while ( STRING_NOTFOUND != nSPos )
                    nSPos = aStr.SearchAndReplaceAscii( "$(VER)", aVersion, nSPos );
            }
            aSize = Size( GetTextWidth( aStr ), GetTextHeight() );
            aPnt = Point( nW - ( aSize.Width() / 2 ), nPos );
            nPos1 = aPnt.Y();
            nPos2 = nPos1 + aSize.Height();

            if ( nPos1 <= nTop && nTop < nPos2 )
            {
                if ( nVal )
                {
                    // eine "Uberschrift hervorheben
                    Font aFont = GetFont();
                    FontWeight eOldWeight = aFont.GetWeight();
                    Color aOldCol = aFont.GetColor();
                    aFont.SetWeight( (FontWeight)nVal );
                    if ( aStr.GetChar(1) != ' ' )
                    {
                        Color aGrayCol( COL_GRAY );
                        aFont.SetColor( aGrayCol );
                    }
                    SetFont( aFont );
                    long nOldW = aSize.Width();
                    aSize = Size(GetTextWidth( aStr ),GetTextHeight());
                    aPnt.X() -= ( aSize.Width() - nOldW ) / 2;
                    DrawText( aPnt, aStr );
                    aFont.SetWeight( eOldWeight );
                    aFont.SetColor( aOldCol );
                    SetFont( aFont );
                }
                else
                    DrawText( aPnt, aStr );
            }
            nPos += aSize.Height() + 3;
        }
    }
    nEnd = nPos - 4;
}


