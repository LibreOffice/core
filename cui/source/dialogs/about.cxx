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

// TODO: make the background of the dialog transparent and remove the titlebar

// include ---------------------------------------------------------------

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <tools/stream.hxx>
#include <rtl/bootstrap.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <com/sun/star/uno/Any.h>
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
#include <rtl/ustrbuf.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/rendergraphicrasterizer.hxx>

using namespace ::com::sun::star;

enum AboutDialogButton
{
    CREDITS_BUTTON,
    WEBSITE_BUTTON,
    LICENSE_BUTTON
};

AboutDialog::AboutDialog( Window* pParent, const ResId& rId) :
    SfxModalDialog       ( pParent,  rId ),
    aVersionText         ( this,     ResId( ABOUT_VERSION_TEXT, *rId.GetResMgr() ) ),
    aDescriptionText     ( this,     ResId( ABOUT_DESCRIPTION_TEXT, *rId.GetResMgr() ) ),
    aCopyrightText       ( this,     ResId( ABOUT_COPYRIGHT_TEXT, *rId.GetResMgr() ) ),
    aCopyrightTextShadow ( this,     ResId( ABOUT_COPYRIGHT_TEXT, *rId.GetResMgr() ) ),
    aLogoImage           ( this,     ResId( ABOUT_IMAGE_LOGO, *rId.GetResMgr() ) ),
    aCreditsButton       ( this,     ResId( ABOUT_BTN_CREDITS, *rId.GetResMgr() ) ),
    aWebsiteButton       ( this,     ResId( ABOUT_BTN_WEBSITE, *rId.GetResMgr() ) ),
    aLicenseButton       ( this,     ResId( ABOUT_BTN_LICENSE, *rId.GetResMgr() ) ),
    aVersionTextStr(ResId(ABOUT_STR_VERSION, *rId.GetResMgr())),
    m_aVendorTextStr(ResId(ABOUT_STR_VENDOR, *rId.GetResMgr())),
    m_aCopyrightTextStr(ResId(ABOUT_STR_COPYRIGHT, *rId.GetResMgr())),
    m_aBasedTextStr(ResId(ABOUT_STR_BASED, *rId.GetResMgr())),
    m_aBasedDerivedTextStr(ResId(ABOUT_STR_BASED_DERIVED, *rId.GetResMgr())),
    m_aWebsiteLinkStr(ResId( ABOUT_STR_LINK_WEBSITE, *rId.GetResMgr())),
    m_aCreditsLinkStr(ResId( ABOUT_STR_LINK_CREDITS, *rId.GetResMgr())),
    m_aLicenseLinkStr(ResId( ABOUT_STR_LINK_LICENSE, *rId.GetResMgr())),
    m_sBuildStr(ResId::toString(ResId(ABOUT_STR_BUILD, *rId.GetResMgr()))),
    m_aDescriptionTextStr(ResId(ABOUT_STR_DESCRIPTION, *rId.GetResMgr()))
{
    // Populate text items
    aVersionText.SetText( GetVersionString() );

    aDescriptionText.SetText( m_aDescriptionTextStr );

    rtl::OUString aCopyrightString = GetCopyrightString();
    aCopyrightText.SetText( aCopyrightString );
    aCopyrightTextShadow.SetText( aCopyrightString );

    StyleControls();
    LayoutControls();

    // Allow the button to be identifiable once they are clicked
    aCreditsButton.SetData( (void*)CREDITS_BUTTON );
    aWebsiteButton.SetData( (void*)WEBSITE_BUTTON );
    aLicenseButton.SetData( (void*)LICENSE_BUTTON );

    // Connect all handlers
    aCreditsButton.SetClickHdl( LINK( this, AboutDialog, HandleClick ) );
    aWebsiteButton.SetClickHdl( LINK( this, AboutDialog, HandleClick ) );
    aLicenseButton.SetClickHdl( LINK( this, AboutDialog, HandleClick ) );

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
        sURL = m_aWebsiteLinkStr;
    else if ( pDialogButton == (AboutDialogButton*)LICENSE_BUTTON)
        sURL = m_aLicenseLinkStr;

    // If the URL is empty, don't do anything
    if ( sURL.isEmpty() )
        return 1;
    try
    {
        uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
            ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.system.SystemShellExecute") ), uno::UNO_QUERY_THROW );
        xSystemShellExecute->execute( sURL, rtl::OUString(),
                                      com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
    }
    catch (const uno::Exception&)
    {
        uno::Any exc( ::cppu::getCaughtException() );
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
    aVersionText.SetPaintTransparent( sal_True );
    aDescriptionText.SetPaintTransparent( sal_True );
    aCopyrightText.SetPaintTransparent( sal_True );
    aCopyrightTextShadow.SetPaintTransparent( sal_True );

    Font aLabelFont = GetSettings().GetStyleSettings().GetLabelFont();
    Font aLargeFont = aLabelFont;
    aLargeFont.SetSize( Size( 0, aLabelFont.GetSize().Height() * 1.3 ) );

    // Description Text
    aDescriptionText.SetControlFont( aLargeFont );
    aDescriptionText.SetControlForeground( Color( 46, 52, 54 ) );
    aDescriptionText.SetTextSelectable( sal_False );

    // Version Text
    aLargeFont.SetSize( Size( 0, aLabelFont.GetSize().Height() * 1.2 ) );
    aVersionText.SetControlFont( aLargeFont );
    aVersionText.SetControlForeground( Color( 142, 142, 142 ) );

    // Copyright Text
    aCopyrightText.SetControlForeground( Color( 142, 142, 142 ) );
    aCopyrightTextShadow.SetControlForeground( Color( 255, 255, 255 ) );
    aCopyrightText.SetTextSelectable( sal_False );
    aCopyrightTextShadow.SetTextSelectable( sal_False );

    // Blue text on website button
    aWebsiteButton.SetControlForeground( Color( 27, 102, 215 ) );
    aWebsiteButton.GrabFocus();
}

void AboutDialog::LayoutControls()
{
    // Get the size of the screen
    Rectangle aScreenRect = Application::GetScreenPosSizePixel( (unsigned int)0 );
    // Obtain an appropriate text width from the size of the screen
    sal_Int32 aIdealTextWidth = aScreenRect.GetWidth() / 2.4;

    sal_Int32 aDialogBorder = 24;
    sal_Int32 aContentWidth = aIdealTextWidth + aDialogBorder * 2;
    sal_Int32 aShadowWidth = aContentWidth * 0.02;
    sal_Int32 aDialogWidth = aContentWidth + aShadowWidth * 2;

    // Render and Position Logo
    vcl::RenderGraphicRasterizer aRasterizerLogo = Application::LoadBrandSVG("flat_logo");
    float aLogoWidthHeightRatio = (float)aRasterizerLogo.GetDefaultSizePixel().Width() /
                               (float)aRasterizerLogo.GetDefaultSizePixel().Height();

    Size aLogoSize( aContentWidth * 0.6, (aContentWidth * 0.6) / aLogoWidthHeightRatio );
    Point aLogoPos( aShadowWidth + ( aContentWidth - aLogoSize.Width() ) / 2,
                    aShadowWidth + aDialogBorder );
    aLogoBitmap = aRasterizerLogo.Rasterize( aLogoSize );
    aLogoImage.SetImage( Image( aLogoBitmap ) );
    aLogoImage.SetPosSizePixel( aLogoPos, aLogoSize );

    // Position version text
    sal_Int32 aLogoVersionSpacing = aLogoSize.Height() * 0.15;
    Point aVersionPos( aShadowWidth + aDialogBorder,
                       aLogoPos.Y() + aLogoSize.Height() + aLogoVersionSpacing );
    Size aVersionSize = aVersionText.CalcMinimumSize();
    aVersionSize.Width() = aIdealTextWidth;
    aVersionText.SetPosSizePixel( aVersionPos, aVersionSize );

    // Position description text
    sal_Int32 aVersionDescriptionSpacing = aLogoSize.Height() * 0.45;
    Point aDescriptionPos( aShadowWidth + aDialogBorder, aVersionPos.Y() + aVersionSize.Height() + aVersionDescriptionSpacing );
    Size aDescriptionSize = aDescriptionText.GetSizePixel();
    aDescriptionSize.Width() = aIdealTextWidth;
    aDescriptionText.SetPosSizePixel( aDescriptionPos, aDescriptionSize );
    aDescriptionSize = aDescriptionText.CalcMinimumSize();
    aDescriptionText.SetSizePixel( aDescriptionSize );

    // Layout Buttons
    Size aButtonSize;
    Point aButtonPos;
    LayoutButtons( aContentWidth, aDialogBorder, aShadowWidth, aDescriptionPos,
                   aDescriptionText.GetSizePixel().Height(),
                   aVersionDescriptionSpacing, aButtonPos, aButtonSize );

    // Layout copyright text
    Point aCopyrightPos( aShadowWidth + aDialogBorder, aButtonPos.Y() + aButtonSize.Height() + aVersionDescriptionSpacing );
    Size aCopyrightSize = aCopyrightText.GetSizePixel();
    aCopyrightSize.Width() = aIdealTextWidth;
    aCopyrightText.SetPosSizePixel( aCopyrightPos, aCopyrightSize );
    aCopyrightSize = aCopyrightText.CalcMinimumSize();
    aCopyrightSize.Width() = aIdealTextWidth;
    aCopyrightText.SetSizePixel( aCopyrightSize );

    // Position the copyright text shadow 1px below the real text
    Point aCopyrightShadowPos = aCopyrightPos;
    aCopyrightShadowPos.Y() += 1;
    aCopyrightTextShadow.SetPosSizePixel( aCopyrightShadowPos, aCopyrightSize );

    // Obtain preliminary dimensions for the dialog
    vcl::RenderGraphicRasterizer aRasterizerBackground = Application::LoadBrandSVG("shell/about");
    float aBackgroundWidthHeightRatio = (float)aRasterizerBackground.GetDefaultSizePixel().Width() /
                               (float)aRasterizerBackground.GetDefaultSizePixel().Height();
    Size aBackgroundSize( aDialogWidth, aDialogWidth / aBackgroundWidthHeightRatio );

    // Make sure the dialog is tall enough
    sal_Int32 aBottomY = aCopyrightPos.Y() + aCopyrightText.GetSizePixel().Height() + aDialogBorder + aShadowWidth;
    // If not, make the dialog taller (and to maintain the aspect ratio of the background also wider)
    if (aBottomY > aBackgroundSize.Height())
        aBackgroundSize.Width() = aBottomY * aBackgroundWidthHeightRatio;
        aBackgroundSize.Height() = aBottomY;

    // If needed, adjust all control position to the new width
    if (aBackgroundSize.Width() != aDialogWidth)
    {
        sal_Int32 aWidthDifference = aBackgroundSize.Width() - aDialogWidth;

        MoveControl(aLogoImage, aWidthDifference / 2);
        MoveControl(aVersionText, aWidthDifference / 2);
        MoveControl(aDescriptionText, aWidthDifference / 2);
        MoveControl(aCreditsButton, aWidthDifference / 2);
        MoveControl(aWebsiteButton, aWidthDifference / 2);
        MoveControl(aLicenseButton, aWidthDifference / 2);
        MoveControl(aCopyrightText, aWidthDifference / 2);
        MoveControl(aCopyrightTextShadow, aWidthDifference / 2);
    }

    // Render Background and set final dialog size
    aBackgroundBitmap = aRasterizerBackground.Rasterize( aBackgroundSize );
    SetOutputSizePixel( aBackgroundSize );
}

void AboutDialog::LayoutButtons(sal_Int32 aContentWidth, sal_Int32 aDialogBorder,
                                 sal_Int32 aShadowWidth, Point aDescriptionPos,
                                 sal_Int32 aDescriptionTextHeight, sal_Int32 aVersionDescriptionSpacing,
                                 Point& aButtonPos, Size& aButtonSize)
{
    // Position credits button
    sal_Int32 aButtonVPadding = 5;
    sal_Int32 aButtonHPadding = 4;
    sal_Int32 aCreditsButtonWidth = aCreditsButton.CalcMinimumSize().Width();
    sal_Int32 aLicenseButtonWidth = aLicenseButton.CalcMinimumSize().Width();
    sal_Int32 aWebsiteButtonWidth = aWebsiteButton.CalcMinimumSize().Width();
    sal_Int32 aLargestButtonWidth = 0;

    if ( aCreditsButtonWidth >= aLicenseButtonWidth
         && aCreditsButtonWidth >= aWebsiteButtonWidth )
        aLargestButtonWidth = aCreditsButtonWidth;

    else if ( aLicenseButtonWidth >= aCreditsButtonWidth
              && aLicenseButtonWidth >= aWebsiteButtonWidth )
        aLargestButtonWidth = aLicenseButtonWidth;
    else if ( aWebsiteButtonWidth >= aCreditsButtonWidth
              && aWebsiteButtonWidth >= aLicenseButtonWidth )
        aLargestButtonWidth = aWebsiteButtonWidth;

    aButtonSize.Width() = aLargestButtonWidth + ( 2 * aButtonHPadding );
    aButtonSize.Height() = aWebsiteButton.CalcMinimumSize().Height() + ( 2 * aButtonVPadding );

    aCreditsButton.SetSizePixel( aButtonSize );
    aLicenseButton.SetSizePixel( aButtonSize );
    aWebsiteButton.SetSizePixel( aButtonSize );

    sal_Int32 aButtonSpacing = (aContentWidth - ( aDialogBorder * 2 ) - ( aButtonSize.Width() * 3 ) ) / 2;
    if (aButtonSpacing < 1)
        aButtonSpacing = 6;
    aButtonPos.X() = aShadowWidth + aDialogBorder;
    aButtonPos.Y() = aDescriptionPos.Y() + aDescriptionTextHeight + aVersionDescriptionSpacing;

    aCreditsButton.SetPosPixel( aButtonPos );

    aButtonPos.X() += aButtonSize.Width() + aButtonSpacing;
    aWebsiteButton.SetPosPixel( aButtonPos );

    aButtonPos.X() += aButtonSize.Width() + aButtonSpacing;
    aLicenseButton.SetPosPixel( aButtonPos );
}

void AboutDialog::MoveControl(Control& rControl, sal_Int32 X)
{
    Point aControlPos = rControl.GetPosPixel();
    aControlPos.X() += X;
    rControl.SetPosPixel(aControlPos);
}

void AboutDialog::Paint( const Rectangle& rRect )
{
    SetClipRegion( rRect );
    Point aPos( 0, 0 );

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
    rtl::OUString sVersion = aVersionTextStr;

    rtl::OUString sBuildId = GetBuildId();

    if (!sBuildId.isEmpty())
    {
        sVersion += " ";
        sVersion += m_sBuildStr.replaceAll("%BUILDID", sBuildId);
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
