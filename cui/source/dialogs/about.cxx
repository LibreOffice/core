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
#include <sfx2/sfxcommands.h>
#include "about.hxx"
#include "about.hrc"
#include <sfx2/sfxdefs.hxx>
#include <sfx2/app.hxx>

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

/* get good version information */
static String
GetBuildId()
{
    rtl::OUString sDefault;
    rtl::OUString sBuildId( utl::Bootstrap::getBuildIdData( sDefault ) );
    if (!sBuildId.isEmpty() && sBuildId.getLength() > 50)
    {
        rtl::OUStringBuffer aBuffer;
        aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\n\t"));
        sal_Int32 nIndex = 0;
        do
        {
            rtl::OUString aToken = sBuildId.getToken( 0, '-', nIndex );
            if (!aToken.isEmpty())
            {
                aBuffer.append(aToken);
                if (nIndex >= 0)
                {
                    if (nIndex % 5)
                        aBuffer.append(static_cast<sal_Unicode>('-'));
                    else
                        aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\n\t"));
                }
            }
        }
        while ( nIndex >= 0 );
        sBuildId = aBuffer.makeStringAndClear();
    }

    OSL_ENSURE( sBuildId.getLength() > 0, "No BUILDID in bootstrap file" );
    return sBuildId;
}

AboutDialog::AboutDialog( Window* pParent, const ResId& rId) :

    SfxModalDialog  ( pParent,  rId ),

    aOKButton       ( this,     ResId( ABOUT_BTN_OK, *rId.GetResMgr() ) ),
    aVersionText    ( this,     ResId( ABOUT_FTXT_VERSION, *rId.GetResMgr() ) ),
    aCopyrightText  ( this,     ResId( ABOUT_FTXT_COPYRIGHT, *rId.GetResMgr() ) ),
    aInfoLink       ( this,     ResId( ABOUT_FTXT_LINK, *rId.GetResMgr() ) ),
    aVersionTextStr(            ResId( ABOUT_STR_VERSION, *rId.GetResMgr() ) ),
    aCopyrightTextStr(          ResId( ABOUT_STR_COPYRIGHT, *rId.GetResMgr() ) ),
    aLinkStr        (           ResId( ABOUT_STR_LINK, *rId.GetResMgr() ) ),
    m_sBuildStr(ResId(ABOUT_STR_BUILD, *rId.GetResMgr()))
{
    rtl::OUString sProduct;
    utl::ConfigManager::GetDirectConfigProperty(utl::ConfigManager::PRODUCTNAME) >>= sProduct;

    // load image from module path
    aAppLogo = SfxApplication::GetApplicationLogo();

    // Transparent Font
    Font aFont = GetFont();
    aFont.SetTransparent( sal_True );
    SetFont( aFont );

    // if necessary more info
    String sVersion = aVersionTextStr;
    sVersion.SearchAndReplaceAscii( "$(VER)", Application::GetDisplayName() );
    sVersion += '\n';
    sVersion += m_sBuildStr;
    sVersion += ' ';
    sVersion += GetBuildId();
#ifdef BUILD_VER_STRING
    String aBuildString( DEFINE_CONST_UNICODE( BUILD_VER_STRING ) );
    sVersion += '\n';
    sVersion += aBuildString;
#endif
    aVersionText.SetText( sVersion );

    // set for background and text the correct system color
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    Color aWhiteCol( rSettings.GetWindowColor() );
    Wallpaper aWall( aWhiteCol );
    SetBackground( aWall );
    Font aNewFont( aCopyrightText.GetFont() );
    aNewFont.SetTransparent( sal_True );

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

    // analyze size of the aVersionText widget
    // character size
    Size a6Size      = aVersionText.LogicToPixel( Size( 6, 6 ), MAP_APPFONT );
    // preferred Version widget size
    long nY          = aAppLogoSiz.Height() + ( a6Size.Height() * 2 );
    long nDlgMargin  = a6Size.Width() * 2;
    long nCtrlMargin = a6Size.Height() * 2;

    aVersionText.SetSizePixel(Size(800,600));
    Size aVersionTextSize = aVersionText.CalcMinimumSize();
    aVersionTextSize.Width() += nDlgMargin;

    Size aOutSiz = GetOutputSizePixel();
    aOutSiz.Width() = aAppLogoSiz.Width();

    if (aOutSiz.Width() < aVersionTextSize.Width())
        aOutSiz.Width() = aVersionTextSize.Width();

    if (aOutSiz.Width() < 300)
        aOutSiz.Width() = 300;

    long nTextWidth  = aOutSiz.Width() - nDlgMargin;

    // finally set the aVersionText widget position and size
    Size aVTSize = aVersionText.GetSizePixel();
    aVTSize.Width() = nTextWidth;
    aVersionText.SetSizePixel(aVTSize);
    aVTSize = aVersionText.CalcMinimumSize();
    Point aVTPnt;
    aVTPnt.X() = ( aOutSiz.Width() - aVTSize.Width() ) / 2;
    aVTPnt.Y() = nY;
    aVersionText.SetPosSizePixel( aVTPnt, aVTSize );

    nY += aVTSize.Height() + nCtrlMargin;

    // Multiline edit with Copyright-Text
    // preferred Version widget size
    Size aCTSize = aCopyrightText.GetSizePixel();
    aCTSize.Width()  = nTextWidth;
    aCopyrightText.SetSizePixel(aCTSize);
    aCTSize = aCopyrightText.CalcMinimumSize();
    Point aCTPnt;
    aCTPnt.X() = ( aOutSiz.Width() - aCTSize.Width() ) / 2;
    aCTPnt.Y() = nY;
    aCopyrightText.SetPosSizePixel( aCTPnt, aCTSize );

    nY += aCTSize.Height() + nCtrlMargin;

    // FixedHyperlink with more info link
    Size aLTSize = aInfoLink.CalcMinimumSize();
    Point aLTPnt;
    aLTPnt.X() = ( aOutSiz.Width() - aLTSize.Width() ) / 2;
    aLTPnt.Y() = nY;
    aInfoLink.SetPosSizePixel( aLTPnt, aLTSize );

    nY += aLTSize.Height() + nCtrlMargin;

    // OK-Button-Position (at the bottom and centered)
    Size aOKSiz = aOKButton.GetSizePixel();
    Point aOKPnt;
    aOKPnt.X() = ( aOutSiz.Width() - aOKSiz.Width() ) / 2;
    aOKPnt.Y() = nY;
    aOKButton.SetPosPixel( aOKPnt );

    nY += aOKSiz.Height() + nCtrlMargin;

    aOutSiz.Height() = nY;

    // Change the size of the dialog
    SetOutputSizePixel( aOutSiz );

    FreeResource();

    // explicit Help-Id
    SetHelpId( CMD_SID_ABOUT );
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

void AboutDialog::Paint( const Rectangle& rRect )
{
    SetClipRegion( rRect );
    Point aPos( 0, 0 );
    DrawImage( aPos, aAppLogo );
}

sal_Bool AboutDialog::Close()
{
    EndDialog( RET_OK );
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
