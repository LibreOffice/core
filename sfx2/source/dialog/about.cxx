/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: about.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:10:03 $
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

// include ---------------------------------------------------------------

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PVER_HXX //autogen
#include <svtools/pver.hxx>
#endif
#ifndef GCC
#pragma hdrstop
#endif

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include "sfxuno.hxx"
#include "about.hxx"
#include "sfxresid.hxx"
#include "sfxdefs.hxx"
#include "app.hxx"

#include "dialog.hrc"

// defines ---------------------------------------------------------------

#define SCROLL_OFFSET   ((long)2)
#define SPACE_OFFSET    ((long)5)

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

AboutDialog::AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr ) :

    SfxModalDialog  ( pParent,  rId ),

    aOKButton       ( this,     ResId( ABOUT_BTN_OK ) ),
    aVersionText    ( this,     ResId( ABOUT_FTXT_VERSION ) ),
    aCopyrightText  ( this,     ResId( ABOUT_FTXT_COPYRIGHT ) ),
    aBuildData      ( this ),
    aDeveloperAry   (           ResId( ABOUT_STR_DEVELOPER_ARY ) ),
    aDevVersionStr  ( rVerStr ),
    aAccelStr       (           ResId( ABOUT_STR_ACCEL ) ),

    aTimer  (),
    nOff    ( 0 ),
    nEnd    ( 0 ),
    bNormal ( TRUE )

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
            String sNewCopyrightText( ResId( ABOUT_STR_FRENCH_COPYRIGHT ) );
            aCopyrightText.SetText( sNewCopyrightText );
        }
        // <--
    }

    // load image from module path
    String aBmpFileName( DEFINE_CONST_UNICODE("about.bmp") );
    INetURLObject aObj( SvtPathOptions().GetModulePath(), INET_PROT_FILE );
    aObj.insertName( aBmpFileName );
    SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
    if ( !aStrm.GetError() )
    {
        Bitmap aBmp;
        aStrm >> aBmp;
        aAppLogo = Image( aBmp );
    }
    else
    {
        aAppLogo = Image( Bitmap( ResId( RID_DEFAULT_ABOUT_BMP_LOGO, rId.GetResMgr() ) ) );
    }

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
    SetBackground( aWall );
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

    // Gr"ossen und Positionen berechnen
    Size aAppLogoSiz = aAppLogo.GetSizePixel();
    Size aOutSiz = GetOutputSizePixel();
    //Size aTextSize = Size( GetTextWidth( DEFINE_CONST_UNICODE( "StarOfficeAbout" ) ), GetTextHeight() );

    // Fenstergr"osse
    aOutSiz.Width() = aAppLogoSiz.Width();

    // Texte (Gr"osse und Position )
    Size a6Size = aVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    long nTextWidth = aAppLogoSiz.Width() - ( a6Size.Width() * 4 );
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

#if SUPD == 680
    String sNewYear( DEFINE_CONST_UNICODE("2005") );
    xub_StrLen nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2002"), sNewYear );
    if ( STRING_NOTFOUND == nIdx )
        nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2003"), sNewYear );
    if ( STRING_NOTFOUND == nIdx )
        nIdx = sCopyright.SearchAndReplace( DEFINE_CONST_UNICODE("2004"), sNewYear );
#endif
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

#if SUPD == 645
struct ExtraDeveloper_Impl
{
    char*   _pName;
    long    _nValue;
};
static ExtraDeveloper_Impl Developer_Impl[] =
{
    { "", 0 },
    { "S t a r O f f i c e  C h i n a  T e a m", 8 },
    { "", 0 },
    { "William Wan", 0 },
    { "", 0 },
    { "Binary Filter Stripping Project", 8 },
    { "", 0 },
    { "Gary Yang", 0 },
    { "James Meng", 0 },
    { "Minna Wu", 0 },
    { "", 0 },
    { "XML Security Project", 8 },
    { "", 0 },
    { "Michael Mi", 0 },
    { "Andrew Fan", 0 },
    { "Keanu Duan", 0 },
    { "Stone Xiang", 0 },
    { "", 0 },
    { "XSLT Transformation", 8 },
    { "", 0 },
    { "Gavin Lu", 0 },
    { "Tom Chen", 0 },
    { "", 0 },
    { "Evolution Address Book Intergration", 8 },
    { "", 0 },
    { "Berry Jia", 0 },
    { "Gilbert Fang", 0 },
    { "Wind Li", 0 },
    { "", 0 },
    { "SI - OSL", 8 },
    { "", 0 },
    { "Gorden Lin", 0 },
    { "", 0 },
    { "SDK", 8 },
    { "", 0 },
    { "Robert Chen", 0 },
    { "", 0 },
    { "QADEV", 8 },
    { "", 0 },
    { "Mia Xia", 0 },
    { "Mindy Liu", 0 },
    { "", 0 },
    { "QA", 8 },
    { "", 0 },
    { "Hercule Li", 0 },
    { "", 0 },
    { "Release Engineering", 8 },
    { "", 0 },
    { "Tin Tian", 0 },
    { "", 0 },
    { "StarOffice BD", 8 },
    { "", 0 },
    { "Sophia Zhang", 0 },
    { "Xiaoyan Tian", 0 },
    { "", 0 }
};
#endif

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
    Point aPnt( nW - ( aSize.Width() / 2 ), nPos );
    long nPos1 = aPnt.Y(), nPos2 = nPos1 + aSize.Height(), nTop = rRect.Top();

    if ( nPos1 <= nTop && nTop < nPos2 )
        DrawText( aPnt, aDevVersionStr );

    nPos += aSize.Height() + 3;
    USHORT nDevCnt = static_cast< USHORT >( aDeveloperAry.Count() );
    USHORT nCount = nDevCnt;

#if SUPD == 645
    int nExtra = sizeof(Developer_Impl) / sizeof(ExtraDeveloper_Impl);
    nCount += nExtra;
#endif

    if ( nCount )
    {
        USHORT nEmptyString = 0;
        for ( USHORT i = 0; i < nCount; ++i )
        {
            String aStr;
            long nVal = 0;

            if ( i < nDevCnt )
            {
                aStr = aDeveloperAry.GetString(i);
                nVal = aDeveloperAry.GetValue(i);
            }
#if SUPD == 645
            else
            {
                USHORT nDev = i - nDevCnt;
                aStr = String::CreateFromAscii( Developer_Impl[nDev]._pName );
                nVal = Developer_Impl[nDev]._nValue;
            }
#endif

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

