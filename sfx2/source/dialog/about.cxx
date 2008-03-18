/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: about.cxx,v $
 *
 *  $Revision: 1.37 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 14:28:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------
#include <aboutbmpnames.hxx>

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef GCC
#endif

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include <sfx2/sfxuno.hxx>
#include "about.hxx"
#include "sfxresid.hxx"
#include <sfx2/sfxdefs.hxx>
#include <sfx2/app.hxx>

#include "dialog.hrc"

// defines ---------------------------------------------------------------

#define SCROLL_OFFSET   ((long)2)
#define SPACE_OFFSET    ((long)5)
#define WELCOME_URL     DEFINE_CONST_UNICODE( "http://www.openoffice.org/welcome/credits.html" )

typedef unsigned short (*fncUshort)();
typedef const char* (*fncChar)();

// class AboutDialog -----------------------------------------------------
static void layoutText( FixedInfo &rText, long &nY, long nTextWidth, Size a6Size )
{
    Point aTextPos = rText.GetPosPixel();
    aTextPos.X() = a6Size.Width() * 2;
    aTextPos.Y() = nY;
    rText.SetPosPixel( aTextPos );

    Size aTxtSiz = rText.GetSizePixel();
    aTxtSiz.Width() = nTextWidth;
    Size aCalcSize = rText.CalcMinimumSize( nTextWidth );
    aTxtSiz.Height() = aCalcSize.Height();
    rText.SetSizePixel( aTxtSiz );

    nY += aTxtSiz.Height();
}

static bool impl_loadBitmap( const rtl::OUString &rBmpFileName, Image &rLogo )
{
    rtl::OUString uri(
        RTL_CONSTASCII_USTRINGPARAM( "$BRAND_BASE_DIR/program" ) );
    rtl::Bootstrap::expandMacros( uri );
    INetURLObject aObj( uri );
    aObj.insertName( rBmpFileName );
    SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
    if ( !aStrm.GetError() )
    {
        Bitmap aBmp;
        aStrm >> aBmp;
        rLogo = Image( aBmp );
        return true;
    }
    return false;
}

AboutDialog::AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr ) :

    SfxModalDialog  ( pParent,  rId ),

    aOKButton       ( this,     ResId( ABOUT_BTN_OK, *rId.GetResMgr() ) ),
    aVersionText    ( this,     ResId( ABOUT_FTXT_VERSION, *rId.GetResMgr() ) ),
    aCopyrightText  ( this,     ResId( ABOUT_FTXT_COPYRIGHT, *rId.GetResMgr() ) ),
    aBuildData      ( this ),
    aDeveloperAry   (           ResId( ABOUT_STR_DEVELOPER_ARY, *rId.GetResMgr() ) ),
    aDevVersionStr  ( rVerStr ),
    aAccelStr       (           ResId( ABOUT_STR_ACCEL, *rId.GetResMgr() ) ),

    aTimer          (),
    nOff            ( 0 ),
    nEnd            ( 0 ),
    m_nDeltaWidth   ( 0 ),
    bNormal         ( TRUE )

{
    rtl::OUString sProduct;
    utl::ConfigManager::GetDirectConfigProperty(utl::ConfigManager::PRODUCTNAME) >>= sProduct;

    if ( sProduct.equals( rtl::OUString::createFromAscii("StarOffice") ) ||
         sProduct.equals( rtl::OUString::createFromAscii("StarSuite") ) )
    {
        // --> PB 2004-11-18 #118455# new copyright text (only in french version show a french text)
        ::com::sun::star::lang::Locale aLocale = Application::GetSettings().GetUILocale();
        ::rtl::OUString sFrenchLang( DEFINE_CONST_OUSTRING( "fr" ) );
        if ( aLocale.Language.equals( sFrenchLang ) )
        {
            String sNewCopyrightText( ResId( ABOUT_STR_FRENCH_COPYRIGHT, *rId.GetResMgr() ) );
            aCopyrightText.SetText( sNewCopyrightText );
        }
        // <--
    }

    // load image from module path
    rtl::OUString aAbouts( RTL_CONSTASCII_USTRINGPARAM( ABOUT_BITMAP_STRINGLIST ) );
    bool bLoaded = false;
    sal_Int32 nIndex = 0;
    do
    {
        bLoaded = impl_loadBitmap( aAbouts.getToken( 0, ',', nIndex ), aAppLogo );
    }
    while ( !bLoaded && ( nIndex >= 0 ) );

    // fallback to "about.bmp"
    if ( !bLoaded )
        bLoaded = impl_loadBitmap( rtl::OUString::createFromAscii( "about.bmp" ), aAppLogo );

    // Transparenter Font
    Font aFont = GetFont();
    aFont.SetTransparent( TRUE );
    SetFont( aFont );

    // ggf. Spezial Version
    String aStr = aVersionText.GetText();
    aStr.SearchAndReplaceAscii( "$(VER)", Application::GetDisplayName() );
    ::rtl::OUString aDefault;
    String sPatchLevel( utl::Bootstrap::getProductPatchLevel( aDefault ) );
    if ( sPatchLevel.Len() > 0 )
    {
        aStr.EraseTrailingChars();
        aStr += ' ';
        aStr += sPatchLevel;
    }
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

    // set for background and text the correct system color
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    Color aWhiteCol( rSettings.GetWindowColor() );
    Wallpaper aWall( aWhiteCol );
    #if 0
    // pl: FIXME #i78884#
    SetBackground( aWall );
    #else
    SetControlBackground( aWhiteCol );
    #endif
    Font aNewFont( aCopyrightText.GetFont() );
    aNewFont.SetTransparent( TRUE );

    aVersionText.SetFont( aNewFont );
    aCopyrightText.SetFont( aNewFont );

    aVersionText.SetBackground( aWall );
    aCopyrightText.SetBackground( aWall );

    Color aTextColor( rSettings.GetWindowTextColor() );
    aVersionText.SetControlForeground( aTextColor );
    aCopyrightText.SetControlForeground( aTextColor );
    aBuildData.SetBackground( aWall );

    Font aSmallFont = rSettings.GetInfoFont();
    Size aSmaller = aNewFont.GetSize();
    aSmaller.Width() = (long) (aSmaller.Width() * 0.75);
    aSmaller.Height() = (long) (aSmaller.Height() * 0.75);
    aNewFont.SetSize( aSmaller );
    aBuildData.SetFont( aNewFont );
    aBuildData.SetBackground( aWall );
#ifdef BUILD_VER_STRING
    String aBuildString( DEFINE_CONST_UNICODE( BUILD_VER_STRING ) );
#else
    String aBuildString;
#endif
    aBuildData.SetText( aBuildString );
    aBuildData.Show();

    // determine size and position of the dialog & elements
    Size aAppLogoSiz = aAppLogo.GetSizePixel();
    Size aOutSiz = GetOutputSizePixel();
    aOutSiz.Width() = aAppLogoSiz.Width();
    // spacing to the margin
    Size a6Size = aVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nDlgMargin = a6Size.Width() * 4 ;
    // The URL (if found in copyright text) should not be line-wrapped
    if ( aCopyrightText.GetText().Search( WELCOME_URL ) != STRING_NOTFOUND )
    {
        long nURLWidth = GetTextWidth( WELCOME_URL ) + nDlgMargin + (2*SPACE_OFFSET);
        if ( nURLWidth > aAppLogoSiz.Width() )
        {
            // pb: can be used to align the position of the logo
            // m_nDeltaWidth = nURLWidth - aOutSiz.Width();

            aOutSiz.Width() = nURLWidth;
        }
    }

    // layout the text-elements
    long nTextWidth = aOutSiz.Width() - nDlgMargin;
    long nY = aAppLogoSiz.Height() + ( a6Size.Height() * 2 );

    layoutText( aVersionText, nY, nTextWidth, a6Size );
    nY += ( a6Size.Height() / 3 );
    layoutText( aCopyrightText, nY, nTextWidth, a6Size );
    nY += ( a6Size.Height() / 3 );
    if( aBuildString.Len() > 0 )
    {
        layoutText( aBuildData, nY, nTextWidth, a6Size );
        nY += ( a6Size.Height() / 2 );
    }

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

    //#112429# replace occurences of "StarOffice" in the "StarSuite" version
    String sCopyright( aCopyrightText.GetText() );
    if(sProduct.equals(rtl::OUString::createFromAscii("StarSuite")))
    {
        String sSO(String::CreateFromAscii("StarOffice"));
        sCopyright.SearchAndReplaceAll(sSO, sProduct);
    }

    String sNewYear( DEFINE_CONST_UNICODE("2005") );
    xub_StrLen nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2002"), sNewYear );
    if ( STRING_NOTFOUND == nIdx )
        nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2003"), sNewYear );
    if ( STRING_NOTFOUND == nIdx )
        nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2004"), sNewYear );
    aCopyrightText.SetText( sCopyright );
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
    (void)pTimer; //unused
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
#ifdef OS2
    aCopyrightText.SetHelpText( DEFINE_CONST_UNICODE("Conoscere qualcuno ovunque egli sia, con cui comprendersi nonostante le distanze\n"
                      "e le differenze, puo' trasformare la terra in un giardino. baci Valeria") );
#endif

    (void)pAccelerator; //unused
    // init Timer
    aTimer.SetTimeoutHdl( LINK( this, AboutDialog, TimerHdl ) );

    // init scroll mode
    nOff = GetOutputSizePixel().Height();
    MapMode aMapMode( MAP_PIXEL, Point( 0, nOff ), Fraction( 1, 1 ), Fraction( 1, 1 ) );
    SetMapMode( aMapMode );
    bNormal = FALSE;

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
    if ( bNormal ) // not in scroll mode
    {
        Point aPos( m_nDeltaWidth / 2, 0 );
        DrawImage( aPos, aAppLogo );
        return;
    }

    Point aPnt;
    Size aSize;
    long nPos = 0, nPos1, nPos2, nTop = 0;
    long nFullWidth = GetOutputSizePixel().Width();
    long nW = nFullWidth / 2 - 5;
    long nTxtWidth = GetTextWidth( aDevVersionStr );

    // perhaps the devversion text is too wide, so we have to split it in some lines
    // the buildids end with ')'
    String sVersion = aDevVersionStr;
    xub_StrLen nEndPos = sVersion.Search( ')' );
    xub_StrLen nLastPos = STRING_NOTFOUND;
    while ( true )
    {
        bool bDraw = false, bDraw2 = false;
        String sTemp = sVersion.Copy( 0, nEndPos + 1 );
        nTxtWidth = GetTextWidth( sTemp );
        if ( nTxtWidth < nFullWidth )
        {
            nLastPos = nEndPos;
            nEndPos = sVersion.Search( ')', nEndPos + 1 );
        }
        else if ( nLastPos != STRING_NOTFOUND )
        {
            sTemp = sVersion.Copy( 0, nLastPos + 1 );
            sVersion = sVersion.Copy( nLastPos + 2 );
            bDraw = true;
        }

        if ( !bDraw && nEndPos == STRING_NOTFOUND )
        {
            sTemp = sVersion;
            nTxtWidth = GetTextWidth( sTemp );
            if ( nTxtWidth > nFullWidth )
            {
                if ( nLastPos != STRING_NOTFOUND )
                {
                    sTemp = sVersion.Copy( 0, nLastPos + 1 );
                    sVersion = sVersion.Copy( nLastPos + 2 );
                    bDraw2 = true;
                }
                else
                {
                    DBG_ERRORFILE( "error while scanning buildids" );
                }
            }

            bDraw = true;
        }

        if ( bDraw )
        {
            aSize = Size( GetTextWidth( sTemp ), GetTextHeight() );
            aPnt = Point( nW - ( aSize.Width() / 2 ), nPos );
            nPos1 = aPnt.Y();
            nPos2 = nPos1 + aSize.Height();
            nTop = rRect.Top();

            if ( nTop < nPos2 )
                DrawText( aPnt, sTemp );
            nPos += aSize.Height() + 3;

            if ( bDraw2 )
            {
                aSize = Size( GetTextWidth( sVersion ), GetTextHeight() );
                aPnt = Point( nW - ( aSize.Width() / 2 ), nPos );
                nPos1 = aPnt.Y();
                nPos2 = nPos1 + aSize.Height();
                nTop = rRect.Top();

                if ( nTop < nPos2 )
                    DrawText( aPnt, sVersion );
                nPos += aSize.Height() + 3;
            }
        }

        if ( nEndPos == STRING_NOTFOUND )
            break;
    }

    USHORT nDevCnt = static_cast< USHORT >( aDeveloperAry.Count() );
    USHORT nCount = nDevCnt;

    if ( nCount )
    {
        for ( USHORT i = 0; i < nCount; ++i )
        {
            String aStr;
            long nVal = 0;

            if ( i < nDevCnt )
            {
                aStr = aDeveloperAry.GetString(i);
                nVal = aDeveloperAry.GetValue(i);
            }

            aSize = Size( GetTextWidth( aStr ), GetTextHeight() );
            aPnt = Point( nW - ( aSize.Width() / 2 ), nPos );
            nPos1 = aPnt.Y();
            nPos2 = nPos1 + aSize.Height();

            if ( nPos1 <= nTop && nTop < nPos2 )
            {
                if ( nVal )
                {
                    // emphasize the headers
                    Font aFont = GetFont();
                    FontWeight eOldWeight = aFont.GetWeight();
                    aFont.SetWeight( (FontWeight)nVal );
                    SetFont( aFont );
                    long nOldW = aSize.Width();
                    aSize = Size(GetTextWidth( aStr ),GetTextHeight());
                    aPnt.X() -= ( aSize.Width() - nOldW ) / 2;
                    if ( aPnt.X() < 0 )
                        aPnt.X() = SPACE_OFFSET;
                    DrawText( aPnt, aStr );
                    aFont.SetWeight( eOldWeight );
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

