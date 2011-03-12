/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"

// include ---------------------------------------------------------------

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
#include <unotools/configmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>

#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include <comphelper/processfactory.hxx>
#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "cppuhelper/bootstrap.hxx"

#include <sfx2/sfxuno.hxx>
#include "about.hxx"
#include "sfxresid.hxx"
#include <sfx2/sfxdefs.hxx>
#include <sfx2/app.hxx>

#include "dialog.hrc"

using namespace ::com::sun::star;

// defines ---------------------------------------------------------------

#define SCROLL_OFFSET   1
#define SPACE_OFFSET    5
#define SCROLL_TIMER    30

/** loads the application logo as used in the about dialog and impress slideshow pause screen */
Image SfxApplication::GetApplicationLogo()
{
    BitmapEx aBitmap;
    Application::LoadBrandBitmap ("about", aBitmap);
    return Image( aBitmap );
}

AboutDialog::AboutDialog( Window* pParent, const ResId& rId, const String& rVerStr ) :

    SfxModalDialog  ( pParent,  rId ),

    aOKButton       ( this,     ResId( ABOUT_BTN_OK, *rId.GetResMgr() ) ),
    aVersionText    ( this,     ResId( ABOUT_FTXT_VERSION, *rId.GetResMgr() ) ),
    aCopyrightText  ( this,     ResId( ABOUT_FTXT_COPYRIGHT, *rId.GetResMgr() ) ),
    aInfoLink       ( this,     ResId( ABOUT_FTXT_LINK, *rId.GetResMgr() ) ),
    aDeveloperAry   (           ResId( ABOUT_STR_DEVELOPER_ARY, *rId.GetResMgr() ) ),
    aDevVersionStr  ( rVerStr ),
    aAccelStr       (           ResId( ABOUT_STR_ACCEL, *rId.GetResMgr() ) ),
    aVersionTextStr(            ResId( ABOUT_STR_VERSION, *rId.GetResMgr() ) ),
    aCopyrightTextStr(          ResId( ABOUT_STR_COPYRIGHT, *rId.GetResMgr() ) ),
    aLinkStr        (           ResId( ABOUT_STR_LINK, *rId.GetResMgr() ) ),
    aTimer          (),
    nOff            ( 0 ),
    m_nDeltaWidth   ( 0 ),
    m_nPendingScrolls( 0 ),
    bNormal         ( TRUE )
{
    rtl::OUString sProduct;
    utl::ConfigManager::GetDirectConfigProperty(utl::ConfigManager::PRODUCTNAME) >>= sProduct;

    // load image from module path
    aAppLogo = SfxApplication::GetApplicationLogo();

    // Transparenter Font
    Font aFont = GetFont();
    aFont.SetTransparent( TRUE );
    SetFont( aFont );

    // if necessary more info
    String sVersion = aVersionTextStr;
    sVersion.SearchAndReplaceAscii( "$(VER)", Application::GetDisplayName() );
    sVersion += '\n';
    sVersion += rVerStr;
#ifdef BUILD_VER_STRING
    String aBuildString( DEFINE_CONST_UNICODE( BUILD_VER_STRING ) );
    sVersion += '\n';
    sVersion += aBuildString;
#endif
    aVersionText.SetText( sVersion );

    // Initialisierung fuer Aufruf Entwickler
    if ( aAccelStr.Len() && ByteString(U2S(aAccelStr)).IsAlphaAscii() )
    {
        Accelerator *pAccel = 0, *pPrevAccel = 0, *pFirstAccel = 0;
        aAccelStr.ToUpperAscii();

        for ( USHORT i = 0; i < aAccelStr.Len(); ++i )
        {
            pPrevAccel = pAccel;
            pAccel = new Accelerator;
            aAccelList.push_back( pAccel );
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

    aVersionText.SetBackground();
    aCopyrightText.SetBackground();
    aInfoLink.SetURL( aLinkStr );
    aInfoLink.SetBackground();
    aInfoLink.SetClickHdl( LINK( this, AboutDialog, HandleHyperlink ) );

    Color aTextColor( rSettings.GetWindowTextColor() );
    aVersionText.SetControlForeground( aTextColor );
    aCopyrightText.SetControlForeground( aTextColor );

    aCopyrightText.SetText( aCopyrightTextStr );

    // determine size and position of the dialog & elements
    Size aAppLogoSiz = aAppLogo.GetSizePixel();

    if (aAppLogoSiz.Width() < 300)
        aAppLogoSiz.Width() = 300;

    Size aOutSiz     = GetOutputSizePixel();
    aOutSiz.Width()  = aAppLogoSiz.Width();

    // analyze size of the aVersionText widget
    // character size
    Size a6Size      = aVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    // preferred Version widget size
    Size aVTSize = aVersionText.CalcMinimumSize();
    long nY          = aAppLogoSiz.Height() + ( a6Size.Height() * 2 );
    long nDlgMargin  = a6Size.Width() * 3 ;
    long nCtrlMargin = aVTSize.Height() + ( a6Size.Height() * 2 );
    long nTextWidth  = aOutSiz.Width() - nDlgMargin;

    // finally set the aVersionText widget position and size
    Size aVTCopySize = aVTSize;
    Point aVTCopyPnt;
    aVTCopySize.Width()  = nTextWidth;
    aVTCopyPnt.X() = ( aOutSiz.Width() - aVTCopySize.Width() ) / 2;
    aVTCopyPnt.Y() = nY;
    aVersionText.SetPosSizePixel( aVTCopyPnt, aVTCopySize );

    nY += nCtrlMargin;

    // OK-Button-Position (at the bottom and centered)
    Size aOKSiz = aOKButton.GetSizePixel();
    Point aOKPnt = aOKButton.GetPosPixel();

    // FixedHyperlink with more info link
    Point aLinkPnt = aInfoLink.GetPosPixel();
    Size aLinkSize = aInfoLink.GetSizePixel();

    // Multiline edit with Copyright-Text
    Point aCopyPnt = aCopyrightText.GetPosPixel();
    Size aCopySize = aCopyrightText.GetSizePixel();
    aCopySize.Width()  = nTextWidth;
    aCopySize.Height() = aOutSiz.Height() - nY - ( aOKSiz.Height() * 2 ) - 3*aLinkSize.Height() - nCtrlMargin;

    aCopyPnt.X() = ( aOutSiz.Width() - aCopySize.Width() ) / 2;
    aCopyPnt.Y() = nY;
    aCopyrightText.SetPosSizePixel( aCopyPnt, aCopySize );

    nY += aCopySize.Height() + aLinkSize.Height();

    aLinkSize.Width() = aInfoLink.CalcMinimumSize().Width();
    aLinkPnt.X() = ( aOutSiz.Width() - aLinkSize.Width() ) / 2;
    aLinkPnt.Y() = nY;
    aInfoLink.SetPosSizePixel( aLinkPnt, aLinkSize );

    nY += aLinkSize.Height() + nCtrlMargin;

    aOKPnt.X() = ( aOutSiz.Width() - aOKSiz.Width() ) / 2;
    aOKPnt.Y() = nY;
    aOKButton.SetPosPixel( aOKPnt );

    // Change the width of the dialog
    SetOutputSizePixel( aOutSiz );

    FreeResource();

    // explizite Help-Id
    SetHelpId( SID_ABOUT );
}

// -----------------------------------------------------------------------

AboutDialog::~AboutDialog()
{
    // L"oschen des Entwickleraufrufs
    if ( !aAccelList.empty() )
    {
        GetpApp()->RemoveAccel( aAccelList.front() );

        for ( size_t i = 0, n = aAccelList.size(); i < n; ++i )
            delete aAccelList[ i ];
        aAccelList.clear();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( AboutDialog, TimerHdl, Timer *, pTimer )
{
    (void)pTimer; //unused
    ++m_nPendingScrolls;
    Invalidate( INVALIDATE_NOERASE | INVALIDATE_NOCHILDREN );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( AboutDialog, AccelSelectHdl, Accelerator *, pAccelerator )
{
    (void)pAccelerator; //unused
    // init Timer
    aTimer.SetTimeoutHdl( LINK( this, AboutDialog, TimerHdl ) );

    // init scroll mode
    nOff = GetOutputSizePixel().Height();
    MapMode aMapMode( MAP_PIXEL );
    SetMapMode( aMapMode );
    bNormal = FALSE;

    // start scroll Timer
    aTimer.SetTimeout( SCROLL_TIMER );
    aTimer.Start();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( AboutDialog, HandleHyperlink, svt::FixedHyperlink*, pHyperlink )
{
    rtl::OUString sURL=pHyperlink->GetURL();
    rtl::OUString sTitle=GetText();

    if ( ! sURL.getLength() ) // Nothing to do, when the URL is empty
        return 1;
    try
    {
        uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
            ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.system.SystemShellExecute") ), uno::UNO_QUERY_THROW );
        xSystemShellExecute->execute( sURL, rtl::OUString(),  com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
    }
    catch ( uno::Exception& )
    {
        uno::Any exc( ::cppu::getCaughtException() );
        rtl::OUString msg( ::comphelper::anyToString( exc ) );
        const SolarMutexGuard guard;
        ErrorBox aErrorBox( NULL, WB_OK, msg );
        aErrorBox.SetText( sTitle );
        aErrorBox.Execute();
    }

    return 1;
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
    SetClipRegion( rRect );

    if ( bNormal ) // not in scroll mode
    {
        Point aPos( m_nDeltaWidth / 2, 0 );
        DrawImage( aPos, aAppLogo );
        return;
    }

    // scroll the content
    const int nDeltaY = -SCROLL_OFFSET * m_nPendingScrolls;
    if( !nDeltaY )
        return;
    nOff += nDeltaY;
    Scroll( 0, nDeltaY, SCROLL_NOERASE );
    m_nPendingScrolls = 0;

    // draw the credits text
    const Font aOrigFont = GetFont();
    const int nFullWidth = GetOutputSizePixel().Width();

    int nY = nOff;
    const int nDevCnt = static_cast<int>( aDeveloperAry.Count() );
    for( int i = 0; i < nDevCnt; ++i )
    {
        if( nY >= rRect.Bottom() )
            break;

        int nPos2 = nY + GetTextHeight() + 3;
        if( nPos2 >= rRect.Top() + nDeltaY )
        {
            const String aStr = aDeveloperAry.GetString(i);
            const long nVal = aDeveloperAry.GetValue(i);

            if ( nVal )
            {
                // emphasize the headers
                Font aFont = aOrigFont;
                aFont.SetWeight( (FontWeight)nVal );
                SetFont( aFont );
                nPos2 = nY + GetTextHeight() + 3;
            }

            // clear text background
            Rectangle aEraseRect( Point(0,nY), Size( nFullWidth, nPos2-nY));
            Erase( aEraseRect );

            // draw centered text
            const long nTextWidth = GetTextWidth( aStr );
            long nX = (nFullWidth - 5 - nTextWidth) / 2;
            if( nX < 0 )
                nX = SPACE_OFFSET;
            const Point aPnt( nX, nY );
            DrawText( aPnt, aStr );

            // restore the font if needed
            if( nVal )
                SetFont( aOrigFont );
        }
        nY = nPos2;
    }

    // close dialog if the whole text has been scrolled
    if ( nY <= 0 )
    {
        bNormal = TRUE;
        Close();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
