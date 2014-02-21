/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include <officecfg/Office/Common.hxx>

#include <svsys.h>
#include "comphelper/processfactory.hxx"
#include <rtl/bootstrap.hxx>
#include "tools/debug.hxx"

#include "i18nlangtag/mslangid.hxx"
#include "i18nlangtag/languagetag.hxx"

#include <vcl/graphicfilter.hxx>
#include <vcl/IconThemeScanner.hxx>
#include <vcl/IconThemeSelector.hxx>
#include <vcl/IconThemeInfo.hxx>
#include "vcl/svapp.hxx"
#include "vcl/event.hxx"
#include "vcl/settings.hxx"
#include "vcl/i18nhelp.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/gradient.hxx"
#include "vcl/outdev.hxx"

#include "unotools/fontcfg.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/collatorwrapper.hxx"
#include "unotools/confignode.hxx"
#include "unotools/syslocaleoptions.hxx"

using namespace ::com::sun::star;

#include "svdata.hxx"
#include "impimagetree.hxx"

#include <boost/make_shared.hpp>

// =======================================================================

ImplMouseData::ImplMouseData()
{
    mnOptions                   = 0;
    mnDoubleClkTime             = 500;
    mnDoubleClkWidth            = 2;
    mnDoubleClkHeight           = 2;
    mnStartDragWidth            = 2;
    mnStartDragHeight           = 2;
    mnStartDragCode             = MOUSE_LEFT;
    mnDragMoveCode              = 0;
    mnDragCopyCode              = KEY_MOD1;
    mnDragLinkCode              = KEY_SHIFT | KEY_MOD1;
    mnContextMenuCode           = MOUSE_RIGHT;
    mnContextMenuClicks         = 1;
    mbContextMenuDown           = true;
    mnMiddleButtonAction        = MOUSE_MIDDLE_AUTOSCROLL;
    mnScrollRepeat              = 100;
    mnButtonStartRepeat         = 370;
    mnButtonRepeat              = 90;
    mnActionDelay               = 250;
    mnMenuDelay                 = 150;
    mnFollow                    = MOUSE_FOLLOW_MENU | MOUSE_FOLLOW_DDLIST;
    mnWheelBehavior             = MOUSE_WHEEL_ALWAYS;
}

// -----------------------------------------------------------------------

ImplMouseData::ImplMouseData( const ImplMouseData& rData )
{
    mnOptions                   = rData.mnOptions;
    mnDoubleClkTime             = rData.mnDoubleClkTime;
    mnDoubleClkWidth            = rData.mnDoubleClkWidth;
    mnDoubleClkHeight           = rData.mnDoubleClkHeight;
    mnStartDragWidth            = rData.mnStartDragWidth;
    mnStartDragHeight           = rData.mnStartDragHeight;
    mnStartDragCode             = rData.mnStartDragCode;
    mnDragMoveCode              = rData.mnDragMoveCode;
    mnDragCopyCode              = rData.mnDragCopyCode;
    mnDragLinkCode              = rData.mnDragLinkCode;
    mnContextMenuCode           = rData.mnContextMenuCode;
    mnContextMenuClicks         = rData.mnContextMenuClicks;
    mbContextMenuDown           = rData.mbContextMenuDown;
    mnMiddleButtonAction        = rData.mnMiddleButtonAction;
    mnScrollRepeat              = rData.mnScrollRepeat;
    mnButtonStartRepeat         = rData.mnButtonStartRepeat;
    mnButtonRepeat              = rData.mnButtonRepeat;
    mnActionDelay               = rData.mnActionDelay;
    mnMenuDelay                 = rData.mnMenuDelay;
    mnFollow                    = rData.mnFollow;
    mnWheelBehavior             = rData.mnWheelBehavior;
}

// -----------------------------------------------------------------------

MouseSettings::MouseSettings()
: mpData(boost::make_shared<ImplMouseData>())
{
}

// -----------------------------------------------------------------------

MouseSettings::MouseSettings( const MouseSettings& rSet )
{
    mpData = rSet.mpData;
}

// -----------------------------------------------------------------------

MouseSettings::~MouseSettings()
{
}

// -----------------------------------------------------------------------

const MouseSettings& MouseSettings::operator =( const MouseSettings& rSet )
{
    mpData = rSet.mpData;
    return *this;
}

// -----------------------------------------------------------------------

void MouseSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplMouseData>(*mpData);
    }
}

// -----------------------------------------------------------------------

bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions)              &&
         (mpData->mnDoubleClkTime       == rSet.mpData->mnDoubleClkTime)        &&
         (mpData->mnDoubleClkWidth      == rSet.mpData->mnDoubleClkWidth)       &&
         (mpData->mnDoubleClkHeight     == rSet.mpData->mnDoubleClkHeight)      &&
         (mpData->mnStartDragWidth      == rSet.mpData->mnStartDragWidth)       &&
         (mpData->mnStartDragHeight     == rSet.mpData->mnStartDragHeight)      &&
         (mpData->mnStartDragCode       == rSet.mpData->mnStartDragCode)        &&
         (mpData->mnDragMoveCode        == rSet.mpData->mnDragMoveCode)         &&
         (mpData->mnDragCopyCode        == rSet.mpData->mnDragCopyCode)         &&
         (mpData->mnDragLinkCode        == rSet.mpData->mnDragLinkCode)         &&
         (mpData->mnContextMenuCode     == rSet.mpData->mnContextMenuCode)      &&
         (mpData->mnContextMenuClicks   == rSet.mpData->mnContextMenuClicks)    &&
         (mpData->mbContextMenuDown     == rSet.mpData->mbContextMenuDown)      &&
         (mpData->mnMiddleButtonAction  == rSet.mpData->mnMiddleButtonAction)   &&
         (mpData->mnScrollRepeat        == rSet.mpData->mnScrollRepeat)         &&
         (mpData->mnButtonStartRepeat   == rSet.mpData->mnButtonStartRepeat)    &&
         (mpData->mnButtonRepeat        == rSet.mpData->mnButtonRepeat)         &&
         (mpData->mnActionDelay         == rSet.mpData->mnActionDelay)          &&
         (mpData->mnMenuDelay           == rSet.mpData->mnMenuDelay)            &&
         (mpData->mnFollow              == rSet.mpData->mnFollow)               &&
         (mpData->mnWheelBehavior       == rSet.mpData->mnWheelBehavior ) )
        return true;
    else
        return false;
}

// =======================================================================

ImplStyleData::ImplStyleData() :
    mIconThemeScanner(vcl::IconThemeScanner::Create(vcl::IconThemeScanner::GetStandardIconThemePath())),
    mIconThemeSelector(new vcl::IconThemeSelector()),
    maPersonaHeaderFooter(),
    maPersonaHeaderBitmap(),
    maPersonaFooterBitmap()
{
    mnScrollBarSize             = 16;
    mnMinThumbSize              = 16;
    mnSplitSize                 = 3;
    mnSpinSize                  = 16;
    mnIconHorzSpace             = 50;
    mnIconVertSpace             = 40;
    mnAntialiasedMin            = 0;
    mnCursorSize                = 2;
    mnCursorBlinkTime           = STYLE_CURSOR_NOBLINKTIME;
    mnScreenZoom                = 100;
    mnScreenFontZoom            = 100;
    mnLogoDisplayTime           = LOGO_DISPLAYTIME_STARTTIME;
    mnDragFullOptions           = DRAGFULL_OPTION_ALL;
    mnAnimationOptions          = 0;
    mnSelectionOptions          = 0;
    mnDisplayOptions            = 0;
    mnOptions                   = 0;
    mnAutoMnemonic              = 1;
    mnToolbarIconSize           = STYLE_TOOLBAR_ICONSIZE_UNKNOWN;
    mnUseImagesInMenus          = AUTO_STATE_AUTO;
    mpFontOptions              = NULL;
    mnEdgeBlending = 35;
    maEdgeBlendingTopLeftColor = RGB_COLORDATA(0xC0, 0xC0, 0xC0);
    maEdgeBlendingBottomRightColor = RGB_COLORDATA(0x40, 0x40, 0x40);
    mnListBoxMaximumLineCount = 25;
    mnColorValueSetColumnCount = 12;
    mnColorValueSetMaximumRowCount = 40;
    maListBoxPreviewDefaultLogicSize = Size(15, 7);
    maListBoxPreviewDefaultPixelSize = Size(0, 0); // on-demand calculated in GetListBoxPreviewDefaultPixelSize()
    mnListBoxPreviewDefaultLineWidth = 1;
    mbPreviewUsesCheckeredBackground = true;

    SetStandardStyles();
}

// -----------------------------------------------------------------------

ImplStyleData::ImplStyleData( const ImplStyleData& rData ) :
    maActiveBorderColor( rData.maActiveBorderColor ),
    maActiveColor( rData.maActiveColor ),
    maActiveColor2( rData.maActiveColor2 ),
    maActiveTextColor( rData.maActiveTextColor ),
    maButtonTextColor( rData.maButtonTextColor ),
    maButtonRolloverTextColor( rData.maButtonRolloverTextColor ),
    maCheckedColor( rData.maCheckedColor ),
    maDarkShadowColor( rData.maDarkShadowColor ),
    maDeactiveBorderColor( rData.maDeactiveBorderColor ),
    maDeactiveColor( rData.maDeactiveColor ),
    maDeactiveColor2( rData.maDeactiveColor2 ),
    maDeactiveTextColor( rData.maDeactiveTextColor ),
    maDialogColor( rData.maDialogColor ),
    maDialogTextColor( rData.maDialogTextColor ),
    maDisableColor( rData.maDisableColor ),
    maFaceColor( rData.maFaceColor ),
    maFieldColor( rData.maFieldColor ),
    maFieldTextColor( rData.maFieldTextColor ),
    maFieldRolloverTextColor( rData.maFieldRolloverTextColor ),
    maFontColor( rData.maFontColor ),
    maGroupTextColor( rData.maGroupTextColor ),
    maHelpColor( rData.maHelpColor ),
    maHelpTextColor( rData.maHelpTextColor ),
    maHighlightColor( rData.maHighlightColor ),
    maHighlightLinkColor( rData.maHighlightLinkColor ),
    maHighlightTextColor( rData.maHighlightTextColor ),
    maInfoTextColor( rData.maInfoTextColor ),
    maLabelTextColor( rData.maLabelTextColor ),
    maLightBorderColor( rData.maLightBorderColor ),
    maLightColor( rData.maLightColor ),
    maLinkColor( rData.maLinkColor ),
    maMenuBarColor( rData.maMenuBarColor ),
    maMenuBarRolloverColor( rData.maMenuBarRolloverColor ),
    maMenuBorderColor( rData.maMenuBorderColor ),
    maMenuColor( rData.maMenuColor ),
    maMenuHighlightColor( rData.maMenuHighlightColor ),
    maMenuHighlightTextColor( rData.maMenuHighlightTextColor ),
    maMenuTextColor( rData.maMenuTextColor ),
    maMenuBarTextColor( rData.maMenuBarTextColor ),
    maMenuBarRolloverTextColor( rData.maMenuBarRolloverTextColor ),
    maMonoColor( rData.maMonoColor ),
    maRadioCheckTextColor( rData.maRadioCheckTextColor ),
    maShadowColor( rData.maShadowColor ),
    maVisitedLinkColor( rData.maVisitedLinkColor ),
    maWindowColor( rData.maWindowColor ),
    maWindowTextColor( rData.maWindowTextColor ),
    maWorkspaceColor( rData.maWorkspaceColor ),
    maActiveTabColor( rData.maActiveTabColor ),
    maInactiveTabColor( rData.maInactiveTabColor ),
    maAppFont( rData.maAppFont ),
    maHelpFont( rData.maAppFont ),
    maTitleFont( rData.maTitleFont ),
    maFloatTitleFont( rData.maFloatTitleFont ),
    maMenuFont( rData.maMenuFont ),
    maToolFont( rData.maToolFont ),
    maLabelFont( rData.maLabelFont ),
    maInfoFont( rData.maInfoFont ),
    maRadioCheckFont( rData.maRadioCheckFont ),
    maPushButtonFont( rData.maPushButtonFont ),
    maFieldFont( rData.maFieldFont ),
    maIconFont( rData.maIconFont ),
    maGroupFont( rData.maGroupFont ),
    mIconTheme(rData.mIconTheme),
    maWorkspaceGradient( rData.maWorkspaceGradient ),
    maDialogStyle( rData.maDialogStyle ),
    maFrameStyle( rData.maFrameStyle ),
    maPersonaHeaderFooter( rData.maPersonaHeaderFooter ),
    maPersonaHeaderBitmap( rData.maPersonaHeaderBitmap ),
    maPersonaFooterBitmap( rData.maPersonaFooterBitmap )
{
    mnBorderSize                = rData.mnBorderSize;
    mnTitleHeight               = rData.mnTitleHeight;
    mnFloatTitleHeight          = rData.mnFloatTitleHeight;
    mnTearOffTitleHeight        = rData.mnTearOffTitleHeight;
    mnMenuBarHeight             = rData.mnMenuBarHeight;
    mnScrollBarSize             = rData.mnScrollBarSize;
    mnMinThumbSize              = rData.mnMinThumbSize;
    mnSplitSize                 = rData.mnSplitSize;
    mnSpinSize                  = rData.mnSpinSize;
    mnIconHorzSpace             = rData.mnIconHorzSpace;
    mnIconVertSpace             = rData.mnIconVertSpace;
    mnAntialiasedMin            = rData.mnAntialiasedMin;
    mnCursorSize                = rData.mnCursorSize;
    mnCursorBlinkTime           = rData.mnCursorBlinkTime;
    mnScreenZoom                = rData.mnScreenZoom;
    mnScreenFontZoom            = rData.mnScreenFontZoom;
    mnLogoDisplayTime           = rData.mnLogoDisplayTime;
    mnDragFullOptions           = rData.mnDragFullOptions;
    mnAnimationOptions          = rData.mnAnimationOptions;
    mnSelectionOptions          = rData.mnSelectionOptions;
    mnDisplayOptions            = rData.mnDisplayOptions;
    mnOptions                   = rData.mnOptions;
    mbHighContrast              = rData.mbHighContrast;
    mbUseSystemUIFonts          = rData.mbUseSystemUIFonts;
    mnUseFlatBorders            = rData.mnUseFlatBorders;
    mnUseFlatMenus              = rData.mnUseFlatMenus;
    mnAutoMnemonic              = rData.mnAutoMnemonic;
    mnUseImagesInMenus          = rData.mnUseImagesInMenus;
    mbPreferredUseImagesInMenus = rData.mbPreferredUseImagesInMenus;
    mbSkipDisabledInMenus       = rData.mbSkipDisabledInMenus;
    mbHideDisabledMenuItems     = rData.mbHideDisabledMenuItems;
    mbAcceleratorsInContextMenus = rData.mbAcceleratorsInContextMenus;
    mbPrimaryButtonWarpsSlider  = rData.mbPrimaryButtonWarpsSlider;
    mnToolbarIconSize           = rData.mnToolbarIconSize;
    mIconThemeScanner.reset(new vcl::IconThemeScanner(*rData.mIconThemeScanner));
    mIconThemeSelector.reset(new vcl::IconThemeSelector(*rData.mIconThemeSelector));
    mpFontOptions               = rData.mpFontOptions;
    mnEdgeBlending              = rData.mnEdgeBlending;
    maEdgeBlendingTopLeftColor  = rData.maEdgeBlendingTopLeftColor;
    maEdgeBlendingBottomRightColor = rData.maEdgeBlendingBottomRightColor;
    mnListBoxMaximumLineCount   = rData.mnListBoxMaximumLineCount;
    mnColorValueSetColumnCount  = rData.mnColorValueSetColumnCount;
    mnColorValueSetMaximumRowCount = rData.mnColorValueSetMaximumRowCount;
    maListBoxPreviewDefaultLogicSize = rData.maListBoxPreviewDefaultLogicSize;
    maListBoxPreviewDefaultPixelSize = rData.maListBoxPreviewDefaultPixelSize;
    mnListBoxPreviewDefaultLineWidth = rData.mnListBoxPreviewDefaultLineWidth;
    mbPreviewUsesCheckeredBackground = rData.mbPreviewUsesCheckeredBackground;
}

// -----------------------------------------------------------------------

void ImplStyleData::SetStandardStyles()
{
    Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( osl_getThreadTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    aStdFont.SetName( utl::DefaultFontConfiguration::get().getUserInterfaceFont( LanguageTag("en")) );
    maAppFont                   = aStdFont;
    maHelpFont                  = aStdFont;
    maMenuFont                  = aStdFont;
    maToolFont                  = aStdFont;
    maGroupFont                 = aStdFont;
    maLabelFont                 = aStdFont;
    maInfoFont                  = aStdFont;
    maRadioCheckFont            = aStdFont;
    maPushButtonFont            = aStdFont;
    maFieldFont                 = aStdFont;
    maIconFont                  = aStdFont;
    aStdFont.SetWeight( WEIGHT_BOLD );
    maFloatTitleFont            = aStdFont;
    maTitleFont                 = aStdFont;

    maFaceColor                 = Color( COL_LIGHTGRAY );
    maCheckedColor              = Color( 0xCC, 0xCC, 0xCC );
    maLightColor                = Color( COL_WHITE );
    maLightBorderColor          = Color( COL_LIGHTGRAY );
    maShadowColor               = Color( COL_GRAY );
    maDarkShadowColor           = Color( COL_BLACK );
    maButtonTextColor           = Color( COL_BLACK );
    maButtonRolloverTextColor   = Color( COL_BLACK );
    maRadioCheckTextColor       = Color( COL_BLACK );
    maGroupTextColor            = Color( COL_BLACK );
    maLabelTextColor            = Color( COL_BLACK );
    maInfoTextColor             = Color( COL_BLACK );
    maWindowColor               = Color( COL_WHITE );
    maWindowTextColor           = Color( COL_BLACK );
    maDialogColor               = Color( COL_LIGHTGRAY );
    maDialogTextColor           = Color( COL_BLACK );
    maWorkspaceColor            = Color( 0xF0, 0xF0, 0xF0 );
    maMonoColor                 = Color( COL_BLACK );
    maFieldColor                = Color( COL_WHITE );
    maFieldTextColor            = Color( COL_BLACK );
    maFieldRolloverTextColor    = Color( COL_BLACK );
    maActiveColor               = Color( COL_BLUE );
    maActiveColor2              = Color( COL_BLACK );
    maActiveTextColor           = Color( COL_WHITE );
    maActiveBorderColor         = Color( COL_LIGHTGRAY );
    maDeactiveColor             = Color( COL_GRAY );
    maDeactiveColor2            = Color( COL_BLACK );
    maDeactiveTextColor         = Color( COL_LIGHTGRAY );
    maDeactiveBorderColor       = Color( COL_LIGHTGRAY );
    maMenuColor                 = Color( COL_LIGHTGRAY );
    maMenuBarColor              = Color( COL_LIGHTGRAY );
    maMenuBarRolloverColor      = Color( COL_BLUE );
    maMenuBorderColor           = Color( COL_LIGHTGRAY );
    maMenuTextColor             = Color( COL_BLACK );
    maMenuBarTextColor          = Color( COL_BLACK );
    maMenuBarRolloverTextColor  = Color( COL_WHITE );
    maMenuHighlightColor        = Color( COL_BLUE );
    maMenuHighlightTextColor    = Color( COL_WHITE );
    maHighlightColor            = Color( COL_BLUE );
    maHighlightTextColor        = Color( COL_WHITE );
    maActiveTabColor            = Color( COL_WHITE );
    maInactiveTabColor          = Color( COL_LIGHTGRAY );
    maDisableColor              = Color( COL_GRAY );
    maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maHelpTextColor             = Color( COL_BLACK );
    maLinkColor                 = Color( COL_BLUE );
    maVisitedLinkColor          = Color( 0x00, 0x00, 0xCC );
    maHighlightLinkColor        = Color( COL_LIGHTBLUE );
    maFontColor                 = Color( COL_BLACK );

    mnBorderSize                = 1;
    mnTitleHeight               = 18;
    mnFloatTitleHeight          = 13;
    mnTearOffTitleHeight        = 8;
    mnMenuBarHeight             = 14;
    mbHighContrast              = false;
    mbUseSystemUIFonts          = true;
    mnUseFlatBorders            = false;
    mnUseFlatMenus              = false;
    mbPreferredUseImagesInMenus = true;
    mbSkipDisabledInMenus       = false;
    mbHideDisabledMenuItems     = false;
    mbAcceleratorsInContextMenus = true;
    mbPrimaryButtonWarpsSlider = false;

    Gradient aGrad( GradientStyle_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
    maWorkspaceGradient = Wallpaper( aGrad );
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings()
: mpData(boost::make_shared<ImplStyleData>())
{
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings( const StyleSettings& rSet )
{
    mpData = rSet.mpData;
}

// -----------------------------------------------------------------------

StyleSettings::~StyleSettings()
{
}

const Size& StyleSettings::GetListBoxPreviewDefaultPixelSize() const
{
    if(0 == mpData->maListBoxPreviewDefaultPixelSize.Width() || 0 == mpData->maListBoxPreviewDefaultPixelSize.Height())
    {
        const_cast< StyleSettings* >(this)->mpData->maListBoxPreviewDefaultPixelSize =
            Application::GetDefaultDevice()->LogicToPixel(mpData->maListBoxPreviewDefaultLogicSize, MAP_APPFONT);
    }

    return mpData->maListBoxPreviewDefaultPixelSize;
}

// -----------------------------------------------------------------------

void StyleSettings::Set3DColors( const Color& rColor )
{
    CopyData();
    mpData->maFaceColor         = rColor;
    mpData->maLightBorderColor  = rColor;
    mpData->maMenuBorderColor   = rColor;
    mpData->maDarkShadowColor   = Color( COL_BLACK );
    if ( rColor != Color( COL_LIGHTGRAY ) )
    {
        mpData->maLightColor    = rColor;
        mpData->maShadowColor   = rColor;
        mpData->maDarkShadowColor=rColor;
        mpData->maLightColor.IncreaseLuminance( 64 );
        mpData->maShadowColor.DecreaseLuminance( 64 );
        mpData->maDarkShadowColor.DecreaseLuminance( 100 );
        sal_uLong   nRed    = mpData->maLightColor.GetRed();
        sal_uLong   nGreen  = mpData->maLightColor.GetGreen();
        sal_uLong   nBlue   = mpData->maLightColor.GetBlue();
        nRed   += (sal_uLong)(mpData->maShadowColor.GetRed());
        nGreen += (sal_uLong)(mpData->maShadowColor.GetGreen());
        nBlue  += (sal_uLong)(mpData->maShadowColor.GetBlue());
        mpData->maCheckedColor = Color( (sal_uInt8)(nRed/2), (sal_uInt8)(nGreen/2), (sal_uInt8)(nBlue/2) );
    }
    else
    {
        mpData->maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mpData->maLightColor    = Color( COL_WHITE );
        mpData->maShadowColor   = Color( COL_GRAY );
    }
}

void StyleSettings::SetCheckedColorSpecialCase( )
{
    CopyData();
    // Light gray checked color special case
    if ( GetFaceColor() == COL_LIGHTGRAY )
        mpData->maCheckedColor = Color( 0xCC, 0xCC, 0xCC );
    else
    {
        sal_uInt8 nRed   = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetRed()   + (sal_uInt16)mpData->maLightColor.GetRed())/2);
        sal_uInt8 nGreen = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetGreen() + (sal_uInt16)mpData->maLightColor.GetGreen())/2);
        sal_uInt8 nBlue  = (sal_uInt8)(((sal_uInt16)mpData->maFaceColor.GetBlue()  + (sal_uInt16)mpData->maLightColor.GetBlue())/2);
        mpData->maCheckedColor = Color( nRed, nGreen, nBlue );
    }
}

bool StyleSettings::GetUseImagesInMenus() const
{
    // icon mode selected in Tools -> Options... -> OpenOffice.org -> View
    switch (mpData->mnUseImagesInMenus) {
    case AUTO_STATE_OFF:
        return false;
    case AUTO_STATE_ON:
        return true;
    default: // AUTO_STATE_AUTO:
        return GetPreferredUseImagesInMenus();
    }
}

// -----------------------------------------------------------------------

static BitmapEx readBitmapEx( const OUString& rPath )
{
    OUString aPath( rPath );
    rtl::Bootstrap::expandMacros( aPath );

    // import the image
    Graphic aGraphic;
    if ( GraphicFilter::LoadGraphic( aPath, OUString(), aGraphic ) != GRFILTER_OK )
        return BitmapEx();

    return aGraphic.GetBitmapEx();
}

enum WhichPersona { PERSONA_HEADER, PERSONA_FOOTER };

/** Update the setting of the Persona header / footer in ImplStyleData */
static void setupPersonaHeaderFooter( WhichPersona eWhich, OUString& rHeaderFooter, BitmapEx& rHeaderFooterBitmap, Color& maMenuBarTextColor )
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if ( !xContext.is() )
        return;

    // read from the configuration
    OUString aPersona( officecfg::Office::Common::Misc::Persona::get( xContext ) );
    OUString aPersonaSettings( officecfg::Office::Common::Misc::PersonaSettings::get( xContext ) );

    // have the settings changed?
    OUString aOldValue( aPersona + ";" + aPersonaSettings );
    if ( rHeaderFooter == aOldValue )
        return;

    rHeaderFooter = aOldValue;
    rHeaderFooterBitmap = BitmapEx();

    // now read the new values and setup bitmaps
    OUString aHeader, aFooter;
    if ( aPersona == "own" )
    {
        sal_Int32 nIndex = 0;
        aHeader = aPersonaSettings.getToken( 0, ';', nIndex );
        if ( nIndex > 0 )
            aFooter = aPersonaSettings.getToken( 0, ';', nIndex );

        // change menu text color, advance nIndex to skip the '#'
        if ( nIndex > 0 )
        {
            OUString aColor = aPersonaSettings.getToken( 0, ';', ++nIndex );
            maMenuBarTextColor = Color( aColor.toUInt64( 16 ) );
        }
    }
    else if ( aPersona == "default" )
    {
        aHeader = "header.jpg";
        aFooter = "footer.jpg";
    }

    OUString aName;
    switch ( eWhich ) {
        case PERSONA_HEADER: aName = aHeader; break;
        case PERSONA_FOOTER: aName = aFooter; break;
    }

    if ( !aName.isEmpty() )
    {
        OUString gallery("");
        // try the gallery first, then the program path:
        if ( aPersona == "own")
        {
            gallery = "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}";
            rtl::Bootstrap::expandMacros( gallery );
            gallery += "/user/gallery/personas/";
        }
        else if (aPersona == "default")
        {
            gallery = "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER;
            gallery += "/gallery/personas/";
        }
        rHeaderFooterBitmap = readBitmapEx( gallery + aName );

        if ( rHeaderFooterBitmap.IsEmpty() )
            rHeaderFooterBitmap = readBitmapEx( "$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" + aName );
    }
}

const BitmapEx StyleSettings::GetPersonaHeader() const
{
    setupPersonaHeaderFooter( PERSONA_HEADER, mpData->maPersonaHeaderFooter, mpData->maPersonaHeaderBitmap, mpData->maMenuBarTextColor );
    return mpData->maPersonaHeaderBitmap;
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardStyles()
{
    CopyData();
    mpData->SetStandardStyles();
}

// -----------------------------------------------------------------------

Color StyleSettings::GetFaceGradientColor() const
{
    // compute a brighter face color that can be used in gradients
    // for a convex look (eg toolbars)

    sal_uInt16 h, s, b;
    GetFaceColor().RGBtoHSB( h, s, b );
    if( s > 1) s=1;
    if( b < 98) b=98;
    return Color( Color::HSBtoRGB( h, s, b ) );
}

// -----------------------------------------------------------------------

Color StyleSettings::GetSeparatorColor() const
{
    // compute a brighter shadow color for separators (used in toolbars or between menubar and toolbars on Windows XP)
    sal_uInt16 h, s, b;
    GetShadowColor().RGBtoHSB( h, s, b );
    b += b/4;
    s -= s/4;
    return Color( Color::HSBtoRGB( h, s, b ) );
}

// -----------------------------------------------------------------------

const StyleSettings& StyleSettings::operator =( const StyleSettings& rSet )
{
    mpData = rSet.mpData;
    return *this;
}

// -----------------------------------------------------------------------

void StyleSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplStyleData>(*mpData);
    }
}

// -----------------------------------------------------------------------

bool StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if (mpData->mIconTheme != rSet.mpData->mIconTheme) {
        return sal_False;
    }

    if (*mpData->mIconThemeSelector != *rSet.mpData->mIconThemeSelector) {
        return sal_False;
    }

    if ( (mpData->mnOptions                 == rSet.mpData->mnOptions)                  &&
         (mpData->mnAutoMnemonic            == rSet.mpData->mnAutoMnemonic)             &&
         (mpData->mnLogoDisplayTime         == rSet.mpData->mnLogoDisplayTime)          &&
         (mpData->mnDragFullOptions         == rSet.mpData->mnDragFullOptions)          &&
         (mpData->mnAnimationOptions        == rSet.mpData->mnAnimationOptions)         &&
         (mpData->mnSelectionOptions        == rSet.mpData->mnSelectionOptions)         &&
         (mpData->mnDisplayOptions          == rSet.mpData->mnDisplayOptions)           &&
         (mpData->mnCursorSize              == rSet.mpData->mnCursorSize)               &&
         (mpData->mnCursorBlinkTime         == rSet.mpData->mnCursorBlinkTime)          &&
         (mpData->mnBorderSize              == rSet.mpData->mnBorderSize)               &&
         (mpData->mnTitleHeight             == rSet.mpData->mnTitleHeight)              &&
         (mpData->mnFloatTitleHeight        == rSet.mpData->mnFloatTitleHeight)         &&
         (mpData->mnTearOffTitleHeight      == rSet.mpData->mnTearOffTitleHeight)       &&
         (mpData->mnMenuBarHeight           == rSet.mpData->mnMenuBarHeight)            &&
         (mpData->mnScrollBarSize           == rSet.mpData->mnScrollBarSize)            &&
         (mpData->mnMinThumbSize            == rSet.mpData->mnMinThumbSize)             &&
         (mpData->mnSplitSize               == rSet.mpData->mnSplitSize)                &&
         (mpData->mnSpinSize                == rSet.mpData->mnSpinSize)                 &&
         (mpData->mnIconHorzSpace           == rSet.mpData->mnIconHorzSpace)            &&
         (mpData->mnIconVertSpace           == rSet.mpData->mnIconVertSpace)            &&
         (mpData->mnAntialiasedMin          == rSet.mpData->mnAntialiasedMin)           &&
         (mpData->mnScreenZoom              == rSet.mpData->mnScreenZoom)               &&
         (mpData->mnScreenFontZoom          == rSet.mpData->mnScreenFontZoom)           &&
         (mpData->mbHighContrast            == rSet.mpData->mbHighContrast)             &&
         (mpData->mbUseSystemUIFonts        == rSet.mpData->mbUseSystemUIFonts)         &&
         (mpData->mnUseFlatBorders          == rSet.mpData->mnUseFlatBorders)           &&
         (mpData->mnUseFlatMenus            == rSet.mpData->mnUseFlatMenus)             &&
         (mpData->maFaceColor               == rSet.mpData->maFaceColor)                &&
         (mpData->maCheckedColor            == rSet.mpData->maCheckedColor)             &&
         (mpData->maLightColor              == rSet.mpData->maLightColor)               &&
         (mpData->maLightBorderColor        == rSet.mpData->maLightBorderColor)         &&
         (mpData->maShadowColor             == rSet.mpData->maShadowColor)              &&
         (mpData->maDarkShadowColor         == rSet.mpData->maDarkShadowColor)          &&
         (mpData->maButtonTextColor         == rSet.mpData->maButtonTextColor)          &&
         (mpData->maRadioCheckTextColor     == rSet.mpData->maRadioCheckTextColor)      &&
         (mpData->maGroupTextColor          == rSet.mpData->maGroupTextColor)           &&
         (mpData->maLabelTextColor          == rSet.mpData->maLabelTextColor)           &&
         (mpData->maInfoTextColor           == rSet.mpData->maInfoTextColor)            &&
         (mpData->maWindowColor             == rSet.mpData->maWindowColor)              &&
         (mpData->maWindowTextColor         == rSet.mpData->maWindowTextColor)          &&
         (mpData->maDialogColor             == rSet.mpData->maDialogColor)              &&
         (mpData->maDialogTextColor         == rSet.mpData->maDialogTextColor)          &&
         (mpData->maWorkspaceColor          == rSet.mpData->maWorkspaceColor)           &&
         (mpData->maMonoColor               == rSet.mpData->maMonoColor)                &&
         (mpData->maFieldColor              == rSet.mpData->maFieldColor)               &&
         (mpData->maFieldTextColor          == rSet.mpData->maFieldTextColor)           &&
         (mpData->maActiveColor             == rSet.mpData->maActiveColor)              &&
         (mpData->maActiveColor2            == rSet.mpData->maActiveColor2)             &&
         (mpData->maActiveTextColor         == rSet.mpData->maActiveTextColor)          &&
         (mpData->maActiveBorderColor       == rSet.mpData->maActiveBorderColor)        &&
         (mpData->maDeactiveColor           == rSet.mpData->maDeactiveColor)            &&
         (mpData->maDeactiveColor2          == rSet.mpData->maDeactiveColor2)           &&
         (mpData->maDeactiveTextColor       == rSet.mpData->maDeactiveTextColor)        &&
         (mpData->maDeactiveBorderColor     == rSet.mpData->maDeactiveBorderColor)      &&
         (mpData->maMenuColor               == rSet.mpData->maMenuColor)                &&
         (mpData->maMenuBarColor            == rSet.mpData->maMenuBarColor)             &&
         (mpData->maMenuBarRolloverColor    == rSet.mpData->maMenuBarRolloverColor)     &&
         (mpData->maMenuBorderColor         == rSet.mpData->maMenuBorderColor)          &&
         (mpData->maMenuTextColor           == rSet.mpData->maMenuTextColor)            &&
         (mpData->maMenuBarTextColor        == rSet.mpData->maMenuBarTextColor)         &&
         (mpData->maMenuBarRolloverTextColor == rSet.mpData->maMenuBarRolloverTextColor) &&
         (mpData->maMenuHighlightColor      == rSet.mpData->maMenuHighlightColor)       &&
         (mpData->maMenuHighlightTextColor  == rSet.mpData->maMenuHighlightTextColor)   &&
         (mpData->maHighlightColor          == rSet.mpData->maHighlightColor)           &&
         (mpData->maHighlightTextColor      == rSet.mpData->maHighlightTextColor)       &&
         (mpData->maActiveTabColor          == rSet.mpData->maActiveTabColor)           &&
         (mpData->maInactiveTabColor        == rSet.mpData->maInactiveTabColor)         &&
         (mpData->maDisableColor            == rSet.mpData->maDisableColor)             &&
         (mpData->maHelpColor               == rSet.mpData->maHelpColor)                &&
         (mpData->maHelpTextColor           == rSet.mpData->maHelpTextColor)            &&
         (mpData->maLinkColor               == rSet.mpData->maLinkColor)                &&
         (mpData->maVisitedLinkColor        == rSet.mpData->maVisitedLinkColor)         &&
         (mpData->maHighlightLinkColor      == rSet.mpData->maHighlightLinkColor)       &&
         (mpData->maAppFont                 == rSet.mpData->maAppFont)                  &&
         (mpData->maHelpFont                == rSet.mpData->maHelpFont)                 &&
         (mpData->maTitleFont               == rSet.mpData->maTitleFont)                &&
         (mpData->maFloatTitleFont          == rSet.mpData->maFloatTitleFont)           &&
         (mpData->maMenuFont                == rSet.mpData->maMenuFont)                 &&
         (mpData->maToolFont                == rSet.mpData->maToolFont)                 &&
         (mpData->maGroupFont               == rSet.mpData->maGroupFont)                &&
         (mpData->maLabelFont               == rSet.mpData->maLabelFont)                &&
         (mpData->maInfoFont                == rSet.mpData->maInfoFont)                 &&
         (mpData->maRadioCheckFont          == rSet.mpData->maRadioCheckFont)           &&
         (mpData->maPushButtonFont          == rSet.mpData->maPushButtonFont)           &&
         (mpData->maFieldFont               == rSet.mpData->maFieldFont)                &&
         (mpData->maIconFont                == rSet.mpData->maIconFont)                 &&
         (mpData->mnUseImagesInMenus        == rSet.mpData->mnUseImagesInMenus)         &&
         (mpData->mbPreferredUseImagesInMenus == rSet.mpData->mbPreferredUseImagesInMenus) &&
         (mpData->mbSkipDisabledInMenus     == rSet.mpData->mbSkipDisabledInMenus)      &&
         (mpData->mbHideDisabledMenuItems   == rSet.mpData->mbHideDisabledMenuItems)    &&
         (mpData->mbAcceleratorsInContextMenus  == rSet.mpData->mbAcceleratorsInContextMenus)&&
         (mpData->mbPrimaryButtonWarpsSlider == rSet.mpData->mbPrimaryButtonWarpsSlider) &&
         (mpData->maFontColor               == rSet.mpData->maFontColor)                &&
         (mpData->mnEdgeBlending                    == rSet.mpData->mnEdgeBlending)                     &&
         (mpData->maEdgeBlendingTopLeftColor        == rSet.mpData->maEdgeBlendingTopLeftColor)         &&
         (mpData->maEdgeBlendingBottomRightColor    == rSet.mpData->maEdgeBlendingBottomRightColor)     &&
         (mpData->mnListBoxMaximumLineCount         == rSet.mpData->mnListBoxMaximumLineCount)          &&
         (mpData->mnColorValueSetColumnCount        == rSet.mpData->mnColorValueSetColumnCount)         &&
         (mpData->mnColorValueSetMaximumRowCount    == rSet.mpData->mnColorValueSetMaximumRowCount)     &&
         (mpData->maListBoxPreviewDefaultLogicSize  == rSet.mpData->maListBoxPreviewDefaultLogicSize)   &&
         (mpData->maListBoxPreviewDefaultPixelSize  == rSet.mpData->maListBoxPreviewDefaultPixelSize)   &&
         (mpData->mnListBoxPreviewDefaultLineWidth  == rSet.mpData->mnListBoxPreviewDefaultLineWidth)   &&
         (mpData->mbPreviewUsesCheckeredBackground == rSet.mpData->mbPreviewUsesCheckeredBackground))
        return true;
    else
        return false;
}

// =======================================================================

ImplMiscData::ImplMiscData()
{
    mnEnableATT                 = AUTO_STATE_AUTO;
    mnDisablePrinting           = AUTO_STATE_AUTO;
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != NULL) ? sal_True : sal_False;
}

// -----------------------------------------------------------------------

ImplMiscData::ImplMiscData( const ImplMiscData& rData )
{
    mnEnableATT                 = rData.mnEnableATT;
    mnDisablePrinting           = rData.mnDisablePrinting;
    mbEnableLocalizedDecimalSep = rData.mbEnableLocalizedDecimalSep;
}

// -----------------------------------------------------------------------

MiscSettings::MiscSettings()
: mpData(boost::make_shared<ImplMiscData>())
{
}

// -----------------------------------------------------------------------

MiscSettings::MiscSettings( const MiscSettings& rSet )
{
    mpData = rSet.mpData;
}

// -----------------------------------------------------------------------

MiscSettings::~MiscSettings()
{
}

// -----------------------------------------------------------------------

const MiscSettings& MiscSettings::operator =( const MiscSettings& rSet )
{
    mpData = rSet.mpData;
    return *this;
}

// -----------------------------------------------------------------------

void MiscSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplMiscData>(*mpData);
    }
}

// -----------------------------------------------------------------------

bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnEnableATT           == rSet.mpData->mnEnableATT ) &&
         (mpData->mnDisablePrinting     == rSet.mpData->mnDisablePrinting ) &&
         (mpData->mbEnableLocalizedDecimalSep == rSet.mpData->mbEnableLocalizedDecimalSep ) )
        return true;
    else
        return false;
}

// -----------------------------------------------------------------------

bool MiscSettings::GetDisablePrinting() const
{
    if( mpData->mnDisablePrinting == AUTO_STATE_AUTO )
    {
        OUString aEnable =
            vcl::SettingsConfigItem::get()->
            getValue( OUString( "DesktopManagement"  ),
                      OUString( "DisablePrinting"  ) );
        mpData->mnDisablePrinting = aEnable.equalsIgnoreAsciiCase("true") ? AUTO_STATE_ON : AUTO_STATE_OFF;
    }

    return mpData->mnDisablePrinting != AUTO_STATE_OFF;
}
// -----------------------------------------------------------------------

bool MiscSettings::GetEnableATToolSupport() const
{

#ifdef WNT
    if( mpData->mnEnableATT == AUTO_STATE_AUTO )
    {
        // Check in the Windows registry if an AT tool wants Accessibility support to
        // be activated ..
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            sal_uInt8 Data[6]; // possible values: "true", "false", "1", "0", DWORD
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "SupportAssistiveTechnology",
                NULL, &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        mpData->mnEnableATT = ((0 == stricmp((const char *) Data, "1")) || (0 == stricmp((const char *) Data, "true"))) ? AUTO_STATE_ON : AUTO_STATE_OFF;
                        break;
                    case REG_DWORD:
                        switch (((DWORD *) Data)[0]) {
                        case 0:
                            mpData->mnEnableATT = AUTO_STATE_OFF;
                            break;
                        case 1:
                            mpData->mnEnableATT = AUTO_STATE_ON;
                            break;
                        default:
                            mpData->mnEnableATT = AUTO_STATE_AUTO;
                                //TODO: or AUTO_STATE_ON?
                            break;
                        }
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }
    }
#endif

    if( mpData->mnEnableATT == AUTO_STATE_AUTO )
    {
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( !pEnv || !*pEnv )
        {
            OUString aEnable =
                vcl::SettingsConfigItem::get()->
                getValue( OUString( "Accessibility"  ),
                          OUString( "EnableATToolSupport"  ) );
            mpData->mnEnableATT = aEnable.equalsIgnoreAsciiCase("true") ? AUTO_STATE_ON : AUTO_STATE_OFF;
        }
        else
        {
            mpData->mnEnableATT = AUTO_STATE_ON;
        }
    }

    return mpData->mnEnableATT != AUTO_STATE_OFF;
}

#ifdef WNT
void MiscSettings::SetEnableATToolSupport( sal_Bool bEnable )
{
    if ( (bEnable ? AUTO_STATE_ON : AUTO_STATE_OFF) != mpData->mnEnableATT )
    {
        if( bEnable && !ImplInitAccessBridge() )
            return;

        HKEY hkey;

        // If the accessibility key in the Windows registry exists, change it synchronously
        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\LibreOffice\\Accessibility\\AtToolSupport",
            &hkey) )
        {
            DWORD dwType;
            sal_uInt8 Data[6]; // possible values: "true", "false", 1, 0
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "SupportAssistiveTechnology",
                NULL,   &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            0, dwType,
                            bEnable ? (sal_uInt8 *) "true" : (sal_uInt8 *) "false",
                            bEnable ? sizeof("true") : sizeof("false"));
                        break;
                    case REG_DWORD:
                        ((DWORD *) Data)[0] = bEnable ? 1 : 0;
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            0, dwType, Data, sizeof(DWORD));
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }

        vcl::SettingsConfigItem::get()->
            setValue( OUString( "Accessibility"  ),
                      OUString( "EnableATToolSupport"  ),
                      bEnable ? OUString("true") : OUString("false" ) );
        mpData->mnEnableATT = bEnable ? AUTO_STATE_ON : AUTO_STATE_OFF;
    }
}
#endif

void MiscSettings::SetEnableLocalizedDecimalSep( bool bEnable )
{
    CopyData();
    mpData->mbEnableLocalizedDecimalSep = bEnable;
}

bool MiscSettings::GetEnableLocalizedDecimalSep() const
{
    return mpData->mbEnableLocalizedDecimalSep;
}

// =======================================================================

ImplHelpData::ImplHelpData()
{
    mnOptions                   = 0;
    mnTipDelay                  = 500;
    mnTipTimeout                = 3000;
    mnBalloonDelay              = 1500;
}

// -----------------------------------------------------------------------

ImplHelpData::ImplHelpData( const ImplHelpData& rData )
{
    mnOptions                   = rData.mnOptions;
    mnTipDelay                  = rData.mnTipDelay;
    mnTipTimeout                = rData.mnTipTimeout;
    mnBalloonDelay              = rData.mnBalloonDelay;
}

// -----------------------------------------------------------------------

HelpSettings::HelpSettings()
: mpData(boost::make_shared<ImplHelpData>())
{
}

// -----------------------------------------------------------------------

HelpSettings::HelpSettings( const HelpSettings& rSet )
{
    mpData = rSet.mpData;
}

// -----------------------------------------------------------------------

HelpSettings::~HelpSettings()
{
}

// -----------------------------------------------------------------------

const HelpSettings& HelpSettings::operator =( const HelpSettings& rSet )
{
    mpData = rSet.mpData;
    return *this;
}

// -----------------------------------------------------------------------

void HelpSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplHelpData>(*mpData);
    }
}

// -----------------------------------------------------------------------

bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->mnOptions         == rSet.mpData->mnOptions ) &&
         (mpData->mnTipDelay        == rSet.mpData->mnTipDelay ) &&
         (mpData->mnTipTimeout      == rSet.mpData->mnTipTimeout ) &&
         (mpData->mnBalloonDelay    == rSet.mpData->mnBalloonDelay ) )
        return true;
    else
        return false;
}

// =======================================================================

ImplAllSettingsData::ImplAllSettingsData()
    :
        maLocale( LANGUAGE_SYSTEM ),
        maUILocale( LANGUAGE_SYSTEM )
{
    mnSystemUpdate              = SETTINGS_ALLSETTINGS;
    mnWindowUpdate              = SETTINGS_ALLSETTINGS;
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
    maMiscSettings.SetEnableLocalizedDecimalSep( maSysLocale.GetOptions().IsDecimalSeparatorAsLocale() );
}

// -----------------------------------------------------------------------

ImplAllSettingsData::ImplAllSettingsData( const ImplAllSettingsData& rData ) :
    maMouseSettings( rData.maMouseSettings ),
    maStyleSettings( rData.maStyleSettings ),
    maMiscSettings( rData.maMiscSettings ),
    maHelpSettings( rData.maHelpSettings ),
    maLocale( rData.maLocale ),
    maUILocale( rData.maUILocale )
{
    mnSystemUpdate              = rData.mnSystemUpdate;
    mnWindowUpdate              = rData.mnWindowUpdate;
    // Pointer couldn't shared and objects haven't a copy ctor
    // So we create the cache objects new, if the GetFunction is
    // called
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
}

// -----------------------------------------------------------------------

ImplAllSettingsData::~ImplAllSettingsData()
{
    delete mpLocaleDataWrapper;
    delete mpUILocaleDataWrapper;
    if ( mpI18nHelper )
        delete mpI18nHelper;
    if ( mpUII18nHelper )
        delete mpUII18nHelper;
}

// -----------------------------------------------------------------------

AllSettings::AllSettings()
: mpData(boost::make_shared<ImplAllSettingsData>())
{
}

// -----------------------------------------------------------------------

AllSettings::AllSettings( const AllSettings& rSet )
{
    mpData = rSet.mpData;
}

// -----------------------------------------------------------------------

AllSettings::~AllSettings()
{
}

// -----------------------------------------------------------------------

const AllSettings& AllSettings::operator =( const AllSettings& rSet )
{
    mpData = rSet.mpData;
    return *this;
}

// -----------------------------------------------------------------------

void AllSettings::CopyData()
{
    // copy if other references exist
    if ( ! mpData.unique() ) {
        mpData = boost::make_shared<ImplAllSettingsData>(*mpData);
    }

}

// -----------------------------------------------------------------------

sal_uLong AllSettings::Update( sal_uLong nFlags, const AllSettings& rSet )
{

    sal_uLong nChangeFlags = 0;

    if ( nFlags & SETTINGS_MOUSE )
    {
        if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        {
            CopyData();
            mpData->maMouseSettings = rSet.mpData->maMouseSettings;
            nChangeFlags |= SETTINGS_MOUSE;
        }
    }

    if ( nFlags & SETTINGS_STYLE )
    {
        if ( mpData->maStyleSettings != rSet.mpData->maStyleSettings )
        {
            CopyData();
            mpData->maStyleSettings = rSet.mpData->maStyleSettings;
            nChangeFlags |= SETTINGS_STYLE;
        }
    }

    if ( nFlags & SETTINGS_MISC )
    {
        if ( mpData->maMiscSettings != rSet.mpData->maMiscSettings )
        {
            CopyData();
            mpData->maMiscSettings = rSet.mpData->maMiscSettings;
            nChangeFlags |= SETTINGS_MISC;
        }
    }

    if ( nFlags & SETTINGS_HELP )
    {
        if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        {
            CopyData();
            mpData->maHelpSettings = rSet.mpData->maHelpSettings;
            nChangeFlags |= SETTINGS_HELP;
        }
    }

    if ( nFlags & SETTINGS_LOCALE )
    {
        if ( mpData->maLocale != rSet.mpData->maLocale )
        {
            SetLanguageTag( rSet.mpData->maLocale );
            nChangeFlags |= SETTINGS_LOCALE;
        }
    }

    if ( nFlags & SETTINGS_UILOCALE )
    {
        // UILocale can't be changed
    }

    return nChangeFlags;
}

// -----------------------------------------------------------------------

sal_uLong AllSettings::GetChangeFlags( const AllSettings& rSet ) const
{

    sal_uLong nChangeFlags = 0;

    if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        nChangeFlags |= SETTINGS_MOUSE;

    if ( mpData->maStyleSettings != rSet.mpData->maStyleSettings )
        nChangeFlags |= SETTINGS_STYLE;

    if ( mpData->maMiscSettings != rSet.mpData->maMiscSettings )
        nChangeFlags |= SETTINGS_MISC;

    if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        nChangeFlags |= SETTINGS_HELP;

    if ( mpData->maLocale != rSet.mpData->maLocale )
        nChangeFlags |= SETTINGS_LOCALE;

    return nChangeFlags;
}

// -----------------------------------------------------------------------

bool AllSettings::operator ==( const AllSettings& rSet ) const
{

    if ( mpData == rSet.mpData )
        return true;

    if ( (mpData->maMouseSettings           == rSet.mpData->maMouseSettings)        &&
         (mpData->maStyleSettings           == rSet.mpData->maStyleSettings)        &&
         (mpData->maMiscSettings            == rSet.mpData->maMiscSettings)         &&
         (mpData->maHelpSettings            == rSet.mpData->maHelpSettings)         &&
         (mpData->mnSystemUpdate            == rSet.mpData->mnSystemUpdate)         &&
         (mpData->maLocale                  == rSet.mpData->maLocale)               &&
         (mpData->mnWindowUpdate            == rSet.mpData->mnWindowUpdate) )
    {
        return true;
    }
    else
        return false;
}

// -----------------------------------------------------------------------

void AllSettings::SetLanguageTag( const LanguageTag& rLanguageTag )
{
    if (mpData->maLocale != rLanguageTag)
    {
        CopyData();

        mpData->maLocale = rLanguageTag;

        if ( mpData->mpLocaleDataWrapper )
        {
            delete mpData->mpLocaleDataWrapper;
            mpData->mpLocaleDataWrapper = NULL;
        }
        if ( mpData->mpI18nHelper )
        {
            delete mpData->mpI18nHelper;
            mpData->mpI18nHelper = NULL;
        }
    }
}

// -----------------------------------------------------------------------

void AllSettings::SetUILanguageTag( const LanguageTag& )
{
    // there is only one UILocale per process
}

// -----------------------------------------------------------------------

namespace
{
    bool GetConfigLayoutRTL(bool bMath)
    {
        static const char* pEnv = getenv("SAL_RTL_ENABLED" );
        static int  nUIMirroring = -1;   // -1: undef, 0: auto, 1: on 2: off

        // environment always overrides
        if( pEnv )
            return true;

        bool bRTL = false;

        if( nUIMirroring == -1 )
        {
            nUIMirroring = 0; // ask configuration only once
            utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                comphelper::getProcessComponentContext(),
                OUString("org.openoffice.Office.Common/I18N/CTL") );    // note: case sensitive !
            if ( aNode.isValid() )
            {
                bool bTmp = bool();
                ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("UIMirroring") );
                if( aValue >>= bTmp )
                {
                    // found true or false; if it was nil, nothing is changed
                    nUIMirroring = bTmp ? 1 : 2;
                }
            }
        }

        if( nUIMirroring == 0 )  // no config found (eg, setup) or default (nil) was set: check language
        {
            LanguageType aLang = LANGUAGE_DONTKNOW;
            ImplSVData* pSVData = ImplGetSVData();
            if ( pSVData->maAppData.mpSettings )
                aLang = pSVData->maAppData.mpSettings->GetUILanguageTag().getLanguageType();
            if (bMath)
                bRTL = MsLangId::isRightToLeftMath( aLang );
            else
                bRTL = MsLangId::isRightToLeft( aLang );
        }
        else
            bRTL = (nUIMirroring == 1);

        return bRTL;
    }
}

bool AllSettings::GetLayoutRTL() const
{
    return GetConfigLayoutRTL(false);
}

bool AllSettings::GetMathLayoutRTL() const
{
    return GetConfigLayoutRTL(true);
}

// -----------------------------------------------------------------------

const LanguageTag& AllSettings::GetLanguageTag() const
{
    // SYSTEM locale means: use settings from SvtSysLocale that is resolved
    if ( mpData->maLocale.isSystemLocale() )
        mpData->maLocale = mpData->maSysLocale.GetLanguageTag();

    return mpData->maLocale;
}

// -----------------------------------------------------------------------

const LanguageTag& AllSettings::GetUILanguageTag() const
{
    // the UILocale is never changed
    if ( mpData->maUILocale.isSystemLocale() )
        mpData->maUILocale = mpData->maSysLocale.GetUILanguageTag();

    return mpData->maUILocale;
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mpData->mpLocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpLocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    return *mpData->mpLocaleDataWrapper;
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mpData->mpUILocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpUILocaleDataWrapper = new LocaleDataWrapper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    return *mpData->mpUILocaleDataWrapper;
}

// -----------------------------------------------------------------------

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mpData->mpI18nHelper ) {
        ((AllSettings*)this)->mpData->mpI18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetLanguageTag() );
    }
    return *mpData->mpI18nHelper;
}

// -----------------------------------------------------------------------

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mpData->mpUII18nHelper ) {
        ((AllSettings*)this)->mpData->mpUII18nHelper = new vcl::I18nHelper(
            comphelper::getProcessComponentContext(), GetUILanguageTag() );
    }
    return *mpData->mpUII18nHelper;
}

void AllSettings::LocaleSettingsChanged( sal_uInt32 nHint )
{
    AllSettings aAllSettings( Application::GetSettings() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_DECSEP )
    {
        MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
        bool bIsDecSepAsLocale = aAllSettings.mpData->maSysLocale.GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( (nHint & SYSLOCALEOPTIONS_HINT_LOCALE) )
        aAllSettings.SetLanguageTag( aAllSettings.mpData->maSysLocale.GetOptions().GetLanguageTag() );

    Application::SetSettings( aAllSettings );
}

const StyleSettings&
AllSettings::GetStyleSettings() const
{
    return mpData->maStyleSettings;
}

sal_uLong
StyleSettings::GetOptions() const
{
    return mpData->mnOptions;
}

std::vector<vcl::IconThemeInfo>
StyleSettings::GetInstalledIconThemes() const
{
    return mpData->mIconThemeScanner->GetFoundIconThemes();
}

/*static*/ OUString
StyleSettings::GetAutomaticallyChosenIconTheme() const
{
    OUString desktopEnvironment = Application::GetDesktopEnvironment();
    OUString themeName = mpData->mIconThemeSelector->SelectIconThemeForDesktopEnvironment(
            mpData->mIconThemeScanner->GetFoundIconThemes(),
            desktopEnvironment
            );
    return themeName;
}

void
StyleSettings::SetIconTheme(const OUString& theme)
{
    CopyData();
    mpData->mIconTheme = theme;
}

OUString
StyleSettings::DetermineIconTheme() const
{
    OUString r = mpData->mIconThemeSelector->SelectIconTheme(
                        mpData->mIconThemeScanner->GetFoundIconThemes(),
                        mpData->mIconTheme
                        );
    return r;
}

void
StyleSettings::SetHighContrastMode(bool bHighContrast )
{
    if (mpData->mbHighContrast == bHighContrast) {
        return;
    }

    CopyData();
    mpData->mbHighContrast = bHighContrast;
    mpData->mIconThemeSelector->SetUseHighContrastTheme(bHighContrast);
}

bool
StyleSettings::GetHighContrastMode() const
{
    return mpData->mbHighContrast;
}

void
StyleSettings::SetPreferredIconTheme(const OUString& theme)
{
    mpData->mIconThemeSelector->SetPreferredIconTheme(theme);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
