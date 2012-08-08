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

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <tools/stream.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>
#include <svtools/langhelp.hxx>

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
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/rendergraphicrasterizer.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;

enum AboutDialogButton
{
    CREDITS_BUTTON,
    WEBSITE_BUTTON
};

AboutDialog::AboutDialog( Window* pParent, const ResId& rId) :
    SfxModalDialog       ( pParent,  rId ),
    aVersionText         ( this,     ResId( ABOUT_VERSION_TEXT, *rId.GetResMgr() ) ),
    aDescriptionText     ( this,     ResId( ABOUT_DESCRIPTION_TEXT, *rId.GetResMgr() ) ),
    aCopyrightText       ( this,     ResId( ABOUT_COPYRIGHT_TEXT, *rId.GetResMgr() ) ),
    aLogoImage           ( this ),
    aLogoReplacement     ( this,     ResId( ABOUT_LOGO_REPLACEMENT, *rId.GetResMgr() ) ),
    aCreditsButton       ( this,     ResId( ABOUT_BTN_CREDITS, *rId.GetResMgr() ) ),
    aWebsiteButton       ( this,     ResId( ABOUT_BTN_WEBSITE, *rId.GetResMgr() ) ),
    aCancelButton        ( this,     ResId( ABOUT_BTN_CANCEL, *rId.GetResMgr() ) ),
    m_aVersionTextStr(ResId(ABOUT_STR_VERSION, *rId.GetResMgr()).toString().trim()),
    m_aVendorTextStr(ResId(ABOUT_STR_VENDOR, *rId.GetResMgr())),
    m_aCopyrightTextStr(ResId(ABOUT_STR_COPYRIGHT, *rId.GetResMgr())),
    m_aBasedTextStr(ResId(ABOUT_STR_BASED, *rId.GetResMgr())),
    m_aBasedDerivedTextStr(ResId(ABOUT_STR_BASED_DERIVED, *rId.GetResMgr())),
    m_aCreditsLinkStr(ResId( ABOUT_STR_LINK_CREDITS, *rId.GetResMgr())),
    m_sBuildStr(ResId(ABOUT_STR_BUILD, *rId.GetResMgr())),
    m_aDescriptionTextStr(ResId(ABOUT_STR_DESCRIPTION, *rId.GetResMgr()))
{
    // Populate text items
    aLogoReplacement.SetText ( utl::ConfigManager::getProductName() );

    aVersionText.SetText( GetVersionString() );

    aDescriptionText.SetText( m_aDescriptionTextStr );

    rtl::OUString aCopyrightString = GetCopyrightString();
    aCopyrightText.SetText( aCopyrightString );

    StyleControls();
    LayoutControls();

    // Allow the button to be identifiable once they are clicked
    aCreditsButton.SetData( (void*)CREDITS_BUTTON );
    aWebsiteButton.SetData( (void*)WEBSITE_BUTTON );

    // Connect all handlers
    aCreditsButton.SetClickHdl( LINK( this, AboutDialog, HandleClick ) );
    aWebsiteButton.SetClickHdl( LINK( this, AboutDialog, HandleClick ) );

    aCancelButton.SetClickHdl( LINK( this, AboutDialog, CancelHdl ) );

    FreeResource();

    // explicit Help-Id
    SetHelpId( CMD_SID_ABOUT );
}

IMPL_LINK( AboutDialog, HandleClick, PushButton*, pButton )
{
    rtl::OUString sURL = "";

    // Find which button was pressed and from this, get the URL to be opened
    AboutDialogButton* pDialogButton = (AboutDialogButton*)pButton->GetData();
    if ( pDialogButton ==  (AboutDialogButton*)CREDITS_BUTTON )
        sURL = m_aCreditsLinkStr;
    else if ( pDialogButton == (AboutDialogButton*)WEBSITE_BUTTON )
    {
        sURL = officecfg::Office::Common::Help::StartCenter::InfoURL::get();
        localizeWebserviceURI(sURL);
    }

    // If the URL is empty, don't do anything
    if ( sURL.isEmpty() )
        return 1;
    try
    {
        Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
            ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.system.SystemShellExecute") ), UNO_QUERY_THROW );
        xSystemShellExecute->execute( sURL, rtl::OUString(),
                                      com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const Exception&)
    {
        Any exc( ::cppu::getCaughtException() );
        rtl::OUString msg( ::comphelper::anyToString( exc ) );
        const SolarMutexGuard guard;
        ErrorBox aErrorBox( NULL, WB_OK, msg );
        aErrorBox.SetText( GetText() );
        aErrorBox.Execute();
    }

    return 1;
}

void AboutDialog::StyleControls()
{
    // Make all the controls have a transparent background
    aLogoImage.SetBackground();
    aLogoReplacement.SetPaintTransparent( sal_True );
    aVersionText.SetPaintTransparent( sal_True );
    aDescriptionText.SetPaintTransparent( sal_True );
    aCopyrightText.SetPaintTransparent( sal_True );

    Font aLabelFont = GetSettings().GetStyleSettings().GetLabelFont();
    Font aLargeFont = aLabelFont;
    aLargeFont.SetSize( Size( 0, aLabelFont.GetSize().Height() * 3 ) );

    // Logo Replacement Text
    aLogoReplacement.SetControlFont( aLargeFont );
    aLogoReplacement.SetTextSelectable( sal_False );

    // Description Text
    aLargeFont.SetSize( Size( 0, aLabelFont.GetSize().Height() * 1.3 ) );
    aDescriptionText.SetControlFont( aLargeFont );
    aDescriptionText.SetTextSelectable( sal_False );

    // Version Text
    aLargeFont.SetSize( Size( 0, aLabelFont.GetSize().Height() * 1.2 ) );
    aVersionText.SetControlFont( aLargeFont );

    // Copyright Text
    aCopyrightText.SetTextSelectable( sal_False );

    // If not in high-contrast mode, hard-code colors
    if ( !(Application::GetSettings().GetStyleSettings().GetHighContrastMode()) )
    {
    aLogoReplacement.SetControlForeground( Color( 51, 51, 51 ) );
    aVersionText.SetControlForeground( Color( 102, 102, 102 ) );
    aDescriptionText.SetControlForeground( Color( 51, 51, 51 ) );
    aCopyrightText.SetControlForeground( Color( 102, 102, 102 ) );
    }


    aCancelButton.GrabFocus();
}

void AboutDialog::LayoutControls()
{
    // Get the size of the screen
    Rectangle aScreenRect = Application::GetScreenPosSizePixel( (unsigned int)0 );
    // Obtain an appropriate text width from the size of the screen
    sal_Int32 aIdealTextWidth = aScreenRect.GetWidth() / 2.4;

    sal_Int32 aDialogBorder = 12;
    Size aDialogSize ( aIdealTextWidth + aDialogBorder * 2, 0 );

    // Preliminarily layout buttons, so we see if we need a wider window
    Size aButtonSize;
    Point aButtonPos;
    sal_Int32 aButtonsWidth = 0;
    LayoutButtons( aDialogSize.Width(), aDialogBorder, aButtonPos, aButtonSize,
                    aButtonsWidth );

    if (aButtonsWidth > 0)
        aDialogSize.Width() += aButtonsWidth;

    // Render and Position Logo
    Size aLogoSize( aIdealTextWidth, aIdealTextWidth / 20 );
    Point aLogoPos( 0, 0 );

    vcl::RenderGraphicRasterizer aRasterizerLogo = Application::LoadBrandSVG("flat_logo");
    if ( !aRasterizerLogo.GetRenderGraphic().IsEmpty() &&
         aRasterizerLogo.GetDefaultSizePixel().Width() > 0 && aRasterizerLogo.GetDefaultSizePixel().Height() > 0 )
    {
        const float aLogoWidthHeightRatio = (float)aRasterizerLogo.GetDefaultSizePixel().Width() / (float)aRasterizerLogo.GetDefaultSizePixel().Height();
        aLogoSize.Width() = aDialogSize.Width() ;
        aLogoSize.Height() = aLogoSize.Width() / aLogoWidthHeightRatio ;

        aLogoBitmap = aRasterizerLogo.Rasterize( aLogoSize );
        aLogoImage.SetImage( Image( aLogoBitmap ) );
        aLogoImage.SetPosSizePixel( aLogoPos, aLogoSize );
        aLogoImage.Show();
    }
    else
    {
        aLogoPos.X() = aDialogBorder;
        aLogoPos.Y() = aDialogBorder;
        aLogoSize = aLogoReplacement.CalcMinimumSize();
        aLogoSize.Width() = aIdealTextWidth;
        aLogoReplacement.SetPosSizePixel( aLogoPos, aLogoSize );
    }

    // Position version text
    sal_Int32 aLogoVersionSpacing = aLogoSize.Height() * 0.15;
    Point aVersionPos( aDialogBorder,
                       aLogoPos.Y() + aLogoSize.Height() + aLogoVersionSpacing );
    Size aVersionSize = aVersionText.CalcMinimumSize();
    aVersionSize.Width() = aIdealTextWidth;
    aVersionText.SetPosSizePixel( aVersionPos, aVersionSize );

    // Position description text
    sal_Int32 aVersionDescriptionSpacing = aLogoSize.Height() * 0.45;
    Point aDescriptionPos( aDialogBorder, aVersionPos.Y() + aVersionSize.Height() + aVersionDescriptionSpacing );
    Size aDescriptionSize = aDescriptionText.GetSizePixel();
    aDescriptionSize.Width() = aIdealTextWidth;
    aDescriptionText.SetPosSizePixel( aDescriptionPos, aDescriptionSize );
    aDescriptionSize = aDescriptionText.CalcMinimumSize();
    aDescriptionText.SetSizePixel( aDescriptionSize );

    // Layout copyright text
    Point aCopyrightPos( aDialogBorder, aDescriptionPos.Y() + aDescriptionText.GetSizePixel().Height() + aVersionDescriptionSpacing );
    Size aCopyrightSize = aCopyrightText.GetSizePixel();
    aCopyrightSize.Width() = aIdealTextWidth;
    aCopyrightText.SetPosSizePixel( aCopyrightPos, aCopyrightSize );
    aCopyrightSize = aCopyrightText.CalcMinimumSize();
    aCopyrightSize.Width() = aIdealTextWidth;
    aCopyrightText.SetSizePixel( aCopyrightSize );

    // Move buttons into final place
    sal_Int32 aButtonPosY = aDialogBorder + aCopyrightPos.Y() +
            aCopyrightText.GetSizePixel().Height() + aVersionDescriptionSpacing;

    MoveControl(aCreditsButton, 0, aButtonPosY);
    MoveControl(aWebsiteButton, 0, aButtonPosY);
    MoveControl(aCancelButton, 0, aButtonPosY);

    // The centers everything again, if aDialogSize.Width() has become wider
    // before; the -20 "fixes" a weird text centering bug
    MoveControl(aVersionText, (aDialogSize.Width() - aVersionText.GetSizePixel().Width() - 20)/2, 0);
    MoveControl(aDescriptionText, (aDialogSize.Width() - aDescriptionText.GetSizePixel().Width() - 20)/2, 0);
    MoveControl(aCopyrightText, (aDialogSize.Width() - aCopyrightText.GetSizePixel().Width() - 20)/2, 0);

    aDialogSize.Height() = aButtonPosY + aButtonSize.Height() + aDialogBorder;


    // Layout background image
    if ( !(Application::GetSettings().GetStyleSettings().GetHighContrastMode()) )   {
        vcl::RenderGraphicRasterizer aRasterizerBackground = Application::LoadBrandSVG("shell/about");

        if ( !aRasterizerBackground.GetRenderGraphic().IsEmpty() &&
            aRasterizerBackground.GetDefaultSizePixel().Width() > 0 && aRasterizerBackground.GetDefaultSizePixel().Height() > 0 )
        {
            const float aBackgroundWidthHeightRatio = (float)aRasterizerBackground.GetDefaultSizePixel().Width() /
                                       (float)aRasterizerBackground.GetDefaultSizePixel().Height();
            Size aBackgroundSize (aDialogSize.Width(), aDialogSize.Width() / aBackgroundWidthHeightRatio );

            if ( aBackgroundSize.Height() < aDialogSize.Height())
            {
                aBackgroundSize.Width() = aDialogSize.Height() * aBackgroundWidthHeightRatio ;
                aBackgroundSize.Height() = aDialogSize.Height();
            }

            aBackgroundBitmap = aRasterizerBackground.Rasterize( aBackgroundSize );
        }
    }

    SetOutputSizePixel( aDialogSize );

}

void AboutDialog::LayoutButtons(sal_Int32 aDialogWidth, sal_Int32 aDialogBorder,
                                Point& aButtonPos, Size& aButtonSize,
                                sal_Int32& aButtonsWidth )
{
    // Position credits button
    sal_Int32 aButtonVPadding = 5;
    sal_Int32 aButtonHPadding = 4;
    sal_Int32 aAdjacentButtonSpacing = 15;
    sal_Int32 aCreditsButtonWidth = aCreditsButton.CalcMinimumSize().Width();
    sal_Int32 aWebsiteButtonWidth = aWebsiteButton.CalcMinimumSize().Width();
    sal_Int32 aCancelButtonWidth = aCancelButton.CalcMinimumSize().Width();
    sal_Int32 aLargestButtonWidth = 70;

    if ( aLargestButtonWidth < aCreditsButtonWidth )
        aLargestButtonWidth = aCreditsButtonWidth;
    if ( aLargestButtonWidth < aWebsiteButtonWidth )
        aLargestButtonWidth = aWebsiteButtonWidth;
    if ( aLargestButtonWidth < aCancelButtonWidth )
        aLargestButtonWidth = aCancelButtonWidth;

    aButtonSize.Width() = aLargestButtonWidth + ( 2 * aButtonHPadding );
    aButtonSize.Height() = aWebsiteButton.CalcMinimumSize().Height() + ( 2 * aButtonVPadding );

    aCreditsButton.SetSizePixel( aButtonSize );
    aWebsiteButton.SetSizePixel( aButtonSize );
    aCancelButton.SetSizePixel( aButtonSize );

    sal_Int32 aButtonSpacing = aDialogWidth  - ( aDialogBorder * 2 ) - ( aButtonSize.Width() * 3 ) - aAdjacentButtonSpacing ;
    if (aButtonSpacing < aAdjacentButtonSpacing)
    {
        aButtonsWidth = aAdjacentButtonSpacing - aButtonSpacing;
        aButtonSpacing = aAdjacentButtonSpacing;
    }

    aButtonPos.X() = aDialogBorder;
    aButtonPos.Y() = 0;

    aCreditsButton.SetPosPixel( aButtonPos );

    aButtonPos.X() += aButtonSize.Width() + aAdjacentButtonSpacing;
    aWebsiteButton.SetPosPixel( aButtonPos );

    aButtonPos.X() += aButtonSize.Width() + aButtonSpacing;
    aCancelButton.SetPosPixel( aButtonPos );


}

void AboutDialog::MoveControl(Control& rControl, sal_Int32 X, sal_Int32 Y)
{
    Point aControlPos = rControl.GetPosPixel();
    aControlPos.X() += X;
    aControlPos.Y() += Y;
    rControl.SetPosPixel(aControlPos);
}

void AboutDialog::Paint( const Rectangle& rRect )
{
    SetClipRegion( rRect );
    Point aPos ( rRect.GetWidth() - aBackgroundBitmap.GetSizePixel().Width(), rRect.GetHeight() - aBackgroundBitmap.GetSizePixel().Height() );
    DrawBitmapEx( aPos, aBackgroundBitmap );
}

rtl::OUString AboutDialog::GetBuildId()
{
    rtl::OUString sDefault;
    rtl::OUString sBuildId(utl::Bootstrap::getBuildVersion(sDefault));
    if (!sBuildId.isEmpty())
        return sBuildId;

    sBuildId = utl::Bootstrap::getBuildIdData(sDefault);

    if (!sBuildId.isEmpty())
    {
        sal_Int32 nIndex = 0;
        return sBuildId.getToken( 0, '-', nIndex );
    }

    OSL_ENSURE( !sBuildId.isEmpty(), "No BUILDID in bootstrap file" );
    return sBuildId;
}

rtl::OUString AboutDialog::GetVersionString()
{
    rtl::OUString sVersion = m_aVersionTextStr;

    rtl::OUString sBuildId = GetBuildId();

    if (!sBuildId.trim().isEmpty())
    {
        sVersion += " ";
        if (m_sBuildStr.indexOf("$BUILDID") == -1)
        {
            SAL_WARN( "cui.dialogs", "translated Build Id string in translations doesn't contain $BUILDID placeholder" );
            m_sBuildStr += " $BUILDID";
        }
        sVersion += m_sBuildStr.replaceAll("$BUILDID", sBuildId);
    }

    return sVersion;
}

rtl::OUString AboutDialog::GetCopyrightString()
{
    rtl::OUString aCopyrightString = m_aVendorTextStr;
    aCopyrightString += "\n";

    aCopyrightString += m_aCopyrightTextStr;
    aCopyrightString += "\n";

    if (utl::ConfigManager::getProductName().equals("LibreOffice"))
        aCopyrightString += m_aBasedTextStr;
    else
        aCopyrightString += m_aBasedDerivedTextStr;

    return aCopyrightString;
}

IMPL_LINK_NOARG(AboutDialog, CancelHdl)
{
    Close();
    return 0;
}

sal_Bool AboutDialog::Close()
{
    EndDialog( RET_OK );
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
