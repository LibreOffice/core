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
#include "precompiled_vcl.hxx"
#include <svsys.h>
#include "tools/debug.hxx"
#include "i18npool/mslangid.hxx"
#include "vcl/svapp.hxx"
#include "vcl/svdata.hxx"
#include "vcl/event.hxx"
#include "vcl/settings.hxx"
#include "vcl/i18nhelp.hxx"
#include "unotools/fontcfg.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/gradient.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/bitmapex.hxx"
#include "vcl/impimagetree.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/collatorwrapper.hxx"
#include "unotools/configmgr.hxx"
#include "unotools/confignode.hxx"
#include <unotools/syslocaleoptions.hxx>

using ::rtl::OUString;
// =======================================================================

DBG_NAME( AllSettings )

// =======================================================================

#define STDSYS_STYLE            (STYLE_OPTION_SCROLLARROW |     \
                                 STYLE_OPTION_SPINARROW |       \
                                 STYLE_OPTION_SPINUPDOWN |      \
                                 STYLE_OPTION_NOMNEMONICS)

// =======================================================================
ImplMachineData::ImplMachineData()
{
    mnRefCount                  = 1;
    mnOptions                   = 0;
    mnScreenOptions             = 0;
    mnPrintOptions              = 0;
    mnScreenRasterFontDeviation = 0;
}

// -----------------------------------------------------------------------

ImplMachineData::ImplMachineData( const ImplMachineData& rData )
{
    mnRefCount                  = 1;
    mnOptions                   = rData.mnOptions;
    mnScreenOptions             = rData.mnScreenOptions;
    mnPrintOptions              = rData.mnPrintOptions;
    mnScreenRasterFontDeviation = rData.mnScreenRasterFontDeviation;
}

// -----------------------------------------------------------------------

MachineSettings::MachineSettings()
{
    mpData = new ImplMachineData();
}

// -----------------------------------------------------------------------

MachineSettings::MachineSettings( const MachineSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MachineSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

MachineSettings::~MachineSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const MachineSettings& MachineSettings::operator =( const MachineSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MachineSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void MachineSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplMachineData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool MachineSettings::operator ==( const MachineSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->mnOptions                     == rSet.mpData->mnOptions)                  &&
         (mpData->mnScreenOptions               == rSet.mpData->mnScreenOptions)            &&
         (mpData->mnPrintOptions                == rSet.mpData->mnPrintOptions)             &&
         (mpData->mnScreenRasterFontDeviation   == rSet.mpData->mnScreenRasterFontDeviation) )
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplMouseData::ImplMouseData()
{
    mnRefCount                  = 1;
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
    mbContextMenuDown           = sal_True;
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
    mnRefCount                  = 1;
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
{
    mpData = new ImplMouseData();
}

// -----------------------------------------------------------------------

MouseSettings::MouseSettings( const MouseSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MouseSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

MouseSettings::~MouseSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const MouseSettings& MouseSettings::operator =( const MouseSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MouseSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void MouseSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplMouseData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

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
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplKeyboardData::ImplKeyboardData()
{
    mnRefCount                  = 1;
    mnOptions                   = 0;
}

// -----------------------------------------------------------------------

ImplKeyboardData::ImplKeyboardData( const ImplKeyboardData& rData )
{
    mnRefCount                  = 1;
    mnOptions                   = rData.mnOptions;
}

// -----------------------------------------------------------------------

KeyboardSettings::KeyboardSettings()
{
    mpData = new ImplKeyboardData();
}

// -----------------------------------------------------------------------

KeyboardSettings::KeyboardSettings( const KeyboardSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "KeyboardSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

KeyboardSettings::~KeyboardSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const KeyboardSettings& KeyboardSettings::operator =( const KeyboardSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "KeyboardSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void KeyboardSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplKeyboardData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool KeyboardSettings::operator ==( const KeyboardSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions) )
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplStyleData::ImplStyleData()
{
    mnRefCount                  = 1;
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
    mnDragFullOptions           = DRAGFULL_OPTION_WINDOWMOVE | DRAGFULL_OPTION_WINDOWSIZE |
                                  DRAGFULL_OPTION_OBJECTMOVE | DRAGFULL_OPTION_OBJECTSIZE |
                                  DRAGFULL_OPTION_DOCKING    | DRAGFULL_OPTION_SPLIT      |
                                  DRAGFULL_OPTION_SCROLL;
    mnAnimationOptions          = 0;
    mnSelectionOptions          = 0;
    mnDisplayOptions            = 0;
    mnOptions                   = 0;
    mnAutoMnemonic              = 1;
    mnToolbarIconSize           = STYLE_TOOLBAR_ICONSIZE_UNKNOWN;
    mnSymbolsStyle              = STYLE_SYMBOLS_AUTO;
    mnUseImagesInMenus          = STYLE_MENUIMAGES_AUTO;
    mnPreferredSymbolsStyle         = STYLE_SYMBOLS_AUTO;
    mpFontOptions              = NULL;

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
    maMenuBorderColor( rData.maMenuBorderColor ),
    maMenuColor( rData.maMenuColor ),
    maMenuHighlightColor( rData.maMenuHighlightColor ),
    maMenuHighlightTextColor( rData.maMenuHighlightTextColor ),
    maMenuTextColor( rData.maMenuTextColor ),
    maMenuBarTextColor( rData.maMenuBarTextColor ),
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
    maWorkspaceGradient( rData.maWorkspaceGradient )
{
    mnRefCount                  = 1;
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
    mnHighContrast              = rData.mnHighContrast;
    mnUseSystemUIFonts          = rData.mnUseSystemUIFonts;
    mnUseFlatBorders            = rData.mnUseFlatBorders;
    mnUseFlatMenues             = rData.mnUseFlatMenues;
    mnAutoMnemonic              = rData.mnAutoMnemonic;
    mnUseImagesInMenus          = rData.mnUseImagesInMenus;
    mbPreferredUseImagesInMenus = rData.mbPreferredUseImagesInMenus;
    mnSkipDisabledInMenus       = rData.mnSkipDisabledInMenus;
    mnToolbarIconSize           = rData.mnToolbarIconSize;
    mnSymbolsStyle              = rData.mnSymbolsStyle;
    mnPreferredSymbolsStyle         = rData.mnPreferredSymbolsStyle;
    mpFontOptions               = rData.mpFontOptions;
}

// -----------------------------------------------------------------------

void ImplStyleData::SetStandardStyles()
{
    Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( gsl_getSystemTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    aStdFont.SetName( utl::DefaultFontConfiguration::get()->getUserInterfaceFont(com::sun::star::lang::Locale( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("en") ), rtl::OUString(), rtl::OUString() ) ) );
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
    maWorkspaceColor            = Color( COL_GRAY );
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
    maMenuBorderColor           = Color( COL_LIGHTGRAY );
    maMenuTextColor             = Color( COL_BLACK );
    maMenuBarTextColor          = Color( COL_BLACK );
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
    mnHighContrast              = 0;
    mnUseSystemUIFonts          = 1;
    mnUseFlatBorders            = 0;
    mnUseFlatMenues             = 0;
    mbPreferredUseImagesInMenus         = sal_True;
    mnSkipDisabledInMenus       = (sal_uInt16)sal_False;

    Gradient aGrad( GRADIENT_LINEAR, DEFAULT_WORKSPACE_GRADIENT_START_COLOR, DEFAULT_WORKSPACE_GRADIENT_END_COLOR );
    maWorkspaceGradient = Wallpaper( aGrad );
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings()
{
    mpData = new ImplStyleData();
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings( const StyleSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "StyleSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

StyleSettings::~StyleSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
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
        mpData->maLightColor.IncreaseLuminance( 64 );
        mpData->maShadowColor.DecreaseLuminance( 64 );
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

// -----------------------------------------------------------------------

::rtl::OUString StyleSettings::ImplSymbolsStyleToName( sal_uLong nStyle ) const
{
    switch ( nStyle )
    {
        case STYLE_SYMBOLS_DEFAULT:    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("default"));
        case STYLE_SYMBOLS_HICONTRAST: return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("hicontrast"));
        case STYLE_SYMBOLS_INDUSTRIAL: return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tango")); // industrial is dead
        case STYLE_SYMBOLS_CRYSTAL:    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("crystal"));
        case STYLE_SYMBOLS_TANGO:      return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tango"));
        case STYLE_SYMBOLS_OXYGEN:     return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("oxygen"));
        case STYLE_SYMBOLS_CLASSIC:    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("classic"));
        case STYLE_SYMBOLS_HUMAN:      return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("human"));
    }

    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("auto"));
}

// -----------------------------------------------------------------------

sal_uLong StyleSettings::ImplNameToSymbolsStyle( const ::rtl::OUString &rName ) const
{
    if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("default")) )
        return STYLE_SYMBOLS_DEFAULT;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("hicontrast")) )
        return STYLE_SYMBOLS_HICONTRAST;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("industrial")) )
        return STYLE_SYMBOLS_TANGO; // industrial is dead
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("crystal")) )
        return STYLE_SYMBOLS_CRYSTAL;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tango")) )
        return STYLE_SYMBOLS_TANGO;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("oxygen")) )
        return STYLE_SYMBOLS_OXYGEN;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("classic")) )
        return STYLE_SYMBOLS_CLASSIC;
    else if ( rName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("human")) )
        return STYLE_SYMBOLS_HUMAN;

    return STYLE_SYMBOLS_AUTO;
}

// -----------------------------------------------------------------------

/**
    The preferred style name can be read from the desktop setting. We
    need to find the closest theme name registered in OOo. Therefore
    we check if any registered style name is a case-insensitive
    substring of the preferred style name.
*/
void StyleSettings::SetPreferredSymbolsStyleName( const ::rtl::OUString &rName )
{
    if ( rName.getLength() > 0 )
    {
        ::rtl::OUString rNameLowCase( rName.toAsciiLowerCase() );

        for( sal_uInt32 n = 0; n <= STYLE_SYMBOLS_THEMES_MAX; n++ )
            if ( rNameLowCase.indexOf( ImplSymbolsStyleToName( n ) ) != -1 )
                SetPreferredSymbolsStyle( n );
    }
}

// -----------------------------------------------------------------------

sal_uLong StyleSettings::GetCurrentSymbolsStyle() const
{
    // style selected in Tools -> Options... -> OpenOffice.org -> View
    sal_uLong nStyle = GetSymbolsStyle();

    if ( nStyle == STYLE_SYMBOLS_AUTO || ( !CheckSymbolStyle (nStyle) ) )
    {
        // the preferred style can be read from the desktop setting by the desktop native widgets modules
        sal_uLong nPreferredStyle = GetPreferredSymbolsStyle();

        if ( nPreferredStyle == STYLE_SYMBOLS_AUTO || ( !CheckSymbolStyle (nPreferredStyle) ) )
        {

            // use a hardcoded desktop-specific fallback if no preferred style has been detected
            static bool sbFallbackDesktopChecked = false;
            static sal_uLong snFallbackDesktopStyle = STYLE_SYMBOLS_DEFAULT;

            if ( !sbFallbackDesktopChecked )
            {
                snFallbackDesktopStyle = GetAutoSymbolsStyle();
                sbFallbackDesktopChecked = true;
            }

            nPreferredStyle = snFallbackDesktopStyle;
        }

        if (GetHighContrastMode() && CheckSymbolStyle (STYLE_SYMBOLS_HICONTRAST) )
            nStyle = STYLE_SYMBOLS_HICONTRAST;
        else
            nStyle = nPreferredStyle;
    }

    return nStyle;
}

// -----------------------------------------------------------------------

sal_uLong StyleSettings::GetAutoSymbolsStyle() const
{
    const ::rtl::OUString&      rDesktopEnvironment = Application::GetDesktopEnvironment();
    sal_uLong                       nRet = STYLE_SYMBOLS_DEFAULT;
    bool                        bCont = true;

    try
    {
        const ::com::sun::star::uno::Any aAny( ::utl::ConfigManager::GetDirectConfigProperty( ::utl::ConfigManager::OPENSOURCECONTEXT ) );
        sal_Int32 nValue( 0 );

        aAny >>= nValue;

        if( 0 == nValue )
            bCont = false;
    }
    catch ( ::com::sun::star::uno::Exception& )
    {
    }

    if( bCont )
    {
        if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "gnome" ) ||
            rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "windows" ) )
            nRet = STYLE_SYMBOLS_TANGO;
        else if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "kde" ) )
            nRet = STYLE_SYMBOLS_CRYSTAL;
        else if( rDesktopEnvironment.equalsIgnoreAsciiCaseAscii( "kde4" ) )
            nRet = STYLE_SYMBOLS_OXYGEN;
    }

    // falback to any existing style
    if ( ! CheckSymbolStyle (nRet) )
    {
        for ( sal_uLong n = 0 ; n <= STYLE_SYMBOLS_THEMES_MAX  ; n++ )
        {
            sal_uLong nStyleToCheck = n;

            // auto is not a real theme => can't be fallback
            if ( nStyleToCheck == STYLE_SYMBOLS_AUTO )
                continue;

            // will check hicontrast in the end
            if ( nStyleToCheck == STYLE_SYMBOLS_HICONTRAST )
                continue;
            if ( nStyleToCheck == STYLE_SYMBOLS_THEMES_MAX )
                nStyleToCheck = STYLE_SYMBOLS_HICONTRAST;

            if ( CheckSymbolStyle ( nStyleToCheck ) )
            {
                nRet = nStyleToCheck;
                n = STYLE_SYMBOLS_THEMES_MAX;
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

bool StyleSettings::CheckSymbolStyle( sal_uLong nStyle ) const
{
    if ( nStyle == STYLE_SYMBOLS_INDUSTRIAL )
        return false; // industrial is dead

    static ImplImageTreeSingletonRef aImageTree;
    return aImageTree->checkStyle( ImplSymbolsStyleToName( nStyle ) );
}

// -----------------------------------------------------------------------

sal_Bool StyleSettings::GetUseImagesInMenus() const
{
    // icon mode selected in Tools -> Options... -> OpenOffice.org -> View
    sal_uInt16 nStyle = mpData->mnUseImagesInMenus;

    if ( nStyle == STYLE_MENUIMAGES_AUTO )
        return GetPreferredUseImagesInMenus();

    return (sal_Bool)nStyle;
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "StyleSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void StyleSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplStyleData( *mpData );
    }
}

// -----------------------------------------------------------------------

inline sal_Bool ImplIsBackOrWhite( const Color& rColor )
{
    sal_uInt8 nLuminance = rColor.GetLuminance();
    return ( nLuminance < 8 ) || ( nLuminance > 250 );
}

sal_Bool StyleSettings::IsHighContrastBlackAndWhite() const
{
    sal_Bool bBWOnly = sal_True;

    // Only use B&W if fully B&W, like on GNOME.
    // Some colors like CheckedColor and HighlightColor are not B&W in Windows Standard HC Black,
    // and we don't want to be B&W then, so check these color first, very probably not B&W.

    // Unfortunately, GNOME uses a very very dark color (0x000033) instead of BLACK (0x000000)

    if ( !ImplIsBackOrWhite( GetFaceColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetHighlightTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetWindowColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetWindowTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetButtonTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetButtonTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetGroupTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetLabelTextColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetDialogColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetFieldColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetMenuColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetMenuBarColor() ) )
        bBWOnly = sal_False;
    else if ( !ImplIsBackOrWhite( GetMenuHighlightColor() ) )
        bBWOnly = sal_False;

    return bBWOnly;
}

// -----------------------------------------------------------------------

sal_Bool StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

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
         (mpData->mnHighContrast            == rSet.mpData->mnHighContrast)             &&
         (mpData->mnUseSystemUIFonts        == rSet.mpData->mnUseSystemUIFonts)         &&
         (mpData->mnUseFlatBorders          == rSet.mpData->mnUseFlatBorders)           &&
         (mpData->mnUseFlatMenues           == rSet.mpData->mnUseFlatMenues)            &&
         (mpData->mnSymbolsStyle            == rSet.mpData->mnSymbolsStyle)             &&
         (mpData->mnPreferredSymbolsStyle   == rSet.mpData->mnPreferredSymbolsStyle)    &&
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
         (mpData->maMenuBorderColor         == rSet.mpData->maMenuBorderColor)          &&
         (mpData->maMenuTextColor           == rSet.mpData->maMenuTextColor)            &&
         (mpData->maMenuBarTextColor        == rSet.mpData->maMenuBarTextColor)         &&
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
         (mpData->mnSkipDisabledInMenus     == rSet.mpData->mnSkipDisabledInMenus)      &&
         (mpData->maFontColor               == rSet.mpData->maFontColor ))
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplMiscData::ImplMiscData()
{
    mnRefCount                  = 1;
    mnEnableATT                 = sal::static_int_cast<sal_uInt16>(~0U);
    mnDisablePrinting           = sal::static_int_cast<sal_uInt16>(~0U);
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != NULL) ? sal_True : sal_False;
}

// -----------------------------------------------------------------------

ImplMiscData::ImplMiscData( const ImplMiscData& rData )
{
    mnRefCount                  = 1;
    mnEnableATT                 = rData.mnEnableATT;
    mnDisablePrinting           = rData.mnDisablePrinting;
    mbEnableLocalizedDecimalSep = rData.mbEnableLocalizedDecimalSep;
}

// -----------------------------------------------------------------------

MiscSettings::MiscSettings()
{
    mpData = new ImplMiscData();
}

// -----------------------------------------------------------------------

MiscSettings::MiscSettings( const MiscSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MiscSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

MiscSettings::~MiscSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const MiscSettings& MiscSettings::operator =( const MiscSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "MiscSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void MiscSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplMiscData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->mnEnableATT           == rSet.mpData->mnEnableATT ) &&
         (mpData->mnDisablePrinting     == rSet.mpData->mnDisablePrinting ) &&
         (mpData->mbEnableLocalizedDecimalSep == rSet.mpData->mbEnableLocalizedDecimalSep ) )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool MiscSettings::GetDisablePrinting() const
{
    if( mpData->mnDisablePrinting == (sal_uInt16)~0 )
    {
        rtl::OUString aEnable =
            vcl::SettingsConfigItem::get()->
            getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DesktopManagement" ) ),
                      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisablePrinting" ) ) );
        mpData->mnDisablePrinting = aEnable.equalsIgnoreAsciiCaseAscii( "true" ) ? 1 : 0;
    }

    return (sal_Bool)mpData->mnDisablePrinting;
}
// -----------------------------------------------------------------------

sal_Bool MiscSettings::GetEnableATToolSupport() const
{

#ifdef WNT
    if( mpData->mnEnableATT == (sal_uInt16)~0 )
    {
        // Check in the Windows registry if an AT tool wants Accessibility support to
        // be activated ..
        HKEY hkey;

        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
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
                        mpData->mnEnableATT = ((0 == stricmp((const char *) Data, "1")) || (0 == stricmp((const char *) Data, "true")));
                        break;
                    case REG_DWORD:
                        mpData->mnEnableATT = (sal_uInt16) (((DWORD *) Data)[0]);
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

    if( mpData->mnEnableATT == (sal_uInt16)~0 )
    {
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( !pEnv || !*pEnv )
        {
            rtl::OUString aEnable =
                vcl::SettingsConfigItem::get()->
                getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Accessibility" ) ),
                          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableATToolSupport" ) ) );
            mpData->mnEnableATT = aEnable.equalsIgnoreAsciiCaseAscii( "true" ) ? 1 : 0;
        }
        else
        {
            mpData->mnEnableATT = 1;
        }
    }

    return (sal_Bool)mpData->mnEnableATT;
}

// -----------------------------------------------------------------------

void MiscSettings::SetDisablePrinting( sal_Bool bEnable )
{
    if ( bEnable != mpData->mnDisablePrinting )
    {
        vcl::SettingsConfigItem::get()->
            setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DesktopManagement" ) ),
                      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DisablePrinting" ) ),
                      bEnable ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("true")) : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("false" )) );
        mpData->mnDisablePrinting = bEnable ? 1 : 0;
    }
}

// -----------------------------------------------------------------------

void MiscSettings::SetEnableATToolSupport( sal_Bool bEnable )
{
    if ( bEnable != mpData->mnEnableATT )
    {
        sal_Bool bDummy;
        if( bEnable && !ImplInitAccessBridge(false, bDummy) )
            return;

#ifdef WNT
        HKEY hkey;

        // If the accessibility key in the Windows registry exists, change it synchronously
        if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
            "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
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
                            NULL, dwType,
                            bEnable ? (sal_uInt8 *) "true" : (sal_uInt8 *) "false",
                            bEnable ? sizeof("true") : sizeof("false"));
                        break;
                    case REG_DWORD:
                        ((DWORD *) Data)[0] = bEnable ? 1 : 0;
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            NULL, dwType, Data, sizeof(DWORD));
                        break;
                    default:
                        // Unsupported registry type
                        break;
                }
            }

            RegCloseKey(hkey);
        }

#endif
        vcl::SettingsConfigItem::get()->
            setValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Accessibility" ) ),
                      rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableATToolSupport" ) ),
                      bEnable ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("true")) : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("false" )) );
        mpData->mnEnableATT = bEnable ? 1 : 0;
    }
}

void MiscSettings::SetEnableLocalizedDecimalSep( sal_Bool bEnable )
{
    CopyData();
    mpData->mbEnableLocalizedDecimalSep = bEnable;
}

sal_Bool MiscSettings::GetEnableLocalizedDecimalSep() const
{
    return mpData->mbEnableLocalizedDecimalSep;
}

// =======================================================================

ImplNotificationData::ImplNotificationData()
{
    mnRefCount                  = 1;
    mnOptions                   = 0;
}

// -----------------------------------------------------------------------

ImplNotificationData::ImplNotificationData( const ImplNotificationData& rData )
{
    mnRefCount                  = 1;
    mnOptions                   = rData.mnOptions;
}

// -----------------------------------------------------------------------

NotificationSettings::NotificationSettings()
{
    mpData = new ImplNotificationData();
}

// -----------------------------------------------------------------------

NotificationSettings::NotificationSettings( const NotificationSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "NotificationSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

NotificationSettings::~NotificationSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const NotificationSettings& NotificationSettings::operator =( const NotificationSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "NotificationSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void NotificationSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplNotificationData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool NotificationSettings::operator ==( const NotificationSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions) )
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplHelpData::ImplHelpData()
{
    mnRefCount                  = 1;
    mnOptions                   = 0;
    mnTipDelay                  = 500;
    mnTipTimeout                = 3000;
    mnBalloonDelay              = 1500;
}

// -----------------------------------------------------------------------

ImplHelpData::ImplHelpData( const ImplHelpData& rData )
{
    mnRefCount                  = 1;
    mnOptions                   = rData.mnOptions;
    mnTipDelay                  = rData.mnTipDelay;
    mnTipTimeout                = rData.mnTipTimeout;
    mnBalloonDelay              = rData.mnBalloonDelay;
}

// -----------------------------------------------------------------------

HelpSettings::HelpSettings()
{
    mpData = new ImplHelpData();
}

// -----------------------------------------------------------------------

HelpSettings::HelpSettings( const HelpSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "HelpSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

HelpSettings::~HelpSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const HelpSettings& HelpSettings::operator =( const HelpSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "HelpSettings: RefCount overflow" );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void HelpSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplHelpData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_Bool HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->mnOptions         == rSet.mpData->mnOptions ) &&
         (mpData->mnTipDelay        == rSet.mpData->mnTipDelay ) &&
         (mpData->mnTipTimeout      == rSet.mpData->mnTipTimeout ) &&
         (mpData->mnBalloonDelay    == rSet.mpData->mnBalloonDelay ) )
        return sal_True;
    else
        return sal_False;
}

// =======================================================================

ImplAllSettingsData::ImplAllSettingsData()
{
    mnRefCount                  = 1;
    mnSystemUpdate              = SETTINGS_ALLSETTINGS;
    mnWindowUpdate              = SETTINGS_ALLSETTINGS;
    meLanguage                  = LANGUAGE_SYSTEM;
    meUILanguage                  = LANGUAGE_SYSTEM;
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
    maMiscSettings.SetEnableLocalizedDecimalSep( maSysLocale.GetOptions().IsDecimalSeparatorAsLocale() );
}

// -----------------------------------------------------------------------

ImplAllSettingsData::ImplAllSettingsData( const ImplAllSettingsData& rData ) :
    maMouseSettings( rData.maMouseSettings ),
    maKeyboardSettings( rData.maKeyboardSettings ),
    maStyleSettings( rData.maStyleSettings ),
    maMiscSettings( rData.maMiscSettings ),
    maNotificationSettings( rData.maNotificationSettings ),
    maHelpSettings( rData.maHelpSettings ),
    maLocale( rData.maLocale )
{
    mnRefCount                  = 1;
    mnSystemUpdate              = rData.mnSystemUpdate;
    mnWindowUpdate              = rData.mnWindowUpdate;
    meLanguage                  = rData.meLanguage;
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
    if ( mpLocaleDataWrapper )
        delete mpLocaleDataWrapper;
    if ( mpUILocaleDataWrapper )
        delete mpUILocaleDataWrapper;
    if ( mpI18nHelper )
        delete mpI18nHelper;
    if ( mpUII18nHelper )
        delete mpUII18nHelper;
}

// -----------------------------------------------------------------------

AllSettings::AllSettings()
{
    DBG_CTOR( AllSettings, NULL );

    mpData = new ImplAllSettingsData();
}

// -----------------------------------------------------------------------

AllSettings::AllSettings( const AllSettings& rSet )
{
    DBG_CTOR( AllSettings, NULL );
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "Settings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

AllSettings::~AllSettings()
{
    DBG_DTOR( AllSettings, NULL );

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const AllSettings& AllSettings::operator =( const AllSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFFFFFE, "AllSettings: RefCount overflow" );
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    rSet.mpData->mnRefCount++;

    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;

    mpData = rSet.mpData;

    return *this;
}

// -----------------------------------------------------------------------

void AllSettings::CopyData()
{
    DBG_CHKTHIS( AllSettings, NULL );

    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplAllSettingsData( *mpData );
    }
}

// -----------------------------------------------------------------------

sal_uLong AllSettings::Update( sal_uLong nFlags, const AllSettings& rSet )
{
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    sal_uLong nChangeFlags = 0;

    if ( nFlags & SETTINGS_MACHINE )
    {
        if ( mpData->maMachineSettings != rSet.mpData->maMachineSettings )
        {
            CopyData();
            mpData->maMachineSettings = rSet.mpData->maMachineSettings;
            nChangeFlags |= SETTINGS_MACHINE;
        }
    }

    if ( nFlags & SETTINGS_MOUSE )
    {
        if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        {
            CopyData();
            mpData->maMouseSettings = rSet.mpData->maMouseSettings;
            nChangeFlags |= SETTINGS_MOUSE;
        }
    }

    if ( nFlags & SETTINGS_KEYBOARD )
    {
        if ( mpData->maKeyboardSettings != rSet.mpData->maKeyboardSettings )
        {
            CopyData();
            mpData->maKeyboardSettings = rSet.mpData->maKeyboardSettings;
            nChangeFlags |= SETTINGS_KEYBOARD;
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

    if ( nFlags & SETTINGS_NOTIFICATION )
    {
        if ( mpData->maNotificationSettings != rSet.mpData->maNotificationSettings )
        {
            CopyData();
            mpData->maNotificationSettings = rSet.mpData->maNotificationSettings;
            nChangeFlags |= SETTINGS_NOTIFICATION;
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

    if ( nFlags & SETTINGS_INTERNATIONAL )
    {
        // Nothing, class International is gone.
        DBG_ERRORFILE("AllSettings::Update: who calls with SETTINGS_INTERNATIONAL and why? You're flogging a dead horse.");
    }

    if ( nFlags & SETTINGS_LOCALE )
    {
        if ( mpData->meLanguage || rSet.mpData->meLanguage )
        {
            SetLanguage( rSet.mpData->meLanguage );
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
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    sal_uLong nChangeFlags = 0;

    if ( mpData->maMachineSettings != rSet.mpData->maMachineSettings )
        nChangeFlags |= SETTINGS_MACHINE;

    if ( mpData->maMouseSettings != rSet.mpData->maMouseSettings )
        nChangeFlags |= SETTINGS_MOUSE;

    if ( mpData->maKeyboardSettings != rSet.mpData->maKeyboardSettings )
        nChangeFlags |= SETTINGS_KEYBOARD;

    if ( mpData->maStyleSettings != rSet.mpData->maStyleSettings )
        nChangeFlags |= SETTINGS_STYLE;

    if ( mpData->maMiscSettings != rSet.mpData->maMiscSettings )
        nChangeFlags |= SETTINGS_MISC;

    if ( mpData->maNotificationSettings != rSet.mpData->maNotificationSettings )
        nChangeFlags |= SETTINGS_NOTIFICATION;

    if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        nChangeFlags |= SETTINGS_HELP;

    if ( mpData->meLanguage || rSet.mpData->meLanguage )
        nChangeFlags |= SETTINGS_LOCALE;

    return nChangeFlags;
}

// -----------------------------------------------------------------------

sal_Bool AllSettings::operator ==( const AllSettings& rSet ) const
{
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    if ( mpData == rSet.mpData )
        return sal_True;

    if ( (mpData->maMachineSettings         == rSet.mpData->maMachineSettings)      &&
         (mpData->maMouseSettings           == rSet.mpData->maMouseSettings)        &&
         (mpData->maKeyboardSettings        == rSet.mpData->maKeyboardSettings)     &&
         (mpData->maStyleSettings           == rSet.mpData->maStyleSettings)        &&
         (mpData->maMiscSettings            == rSet.mpData->maMiscSettings)         &&
         (mpData->maNotificationSettings    == rSet.mpData->maNotificationSettings) &&
         (mpData->maHelpSettings            == rSet.mpData->maHelpSettings)         &&
         (mpData->mnSystemUpdate            == rSet.mpData->mnSystemUpdate)         &&
         (mpData->maLocale                  == rSet.mpData->maLocale)               &&
         (mpData->mnWindowUpdate            == rSet.mpData->mnWindowUpdate) )
    {
        return sal_True;
    }
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void AllSettings::SetLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    CopyData();

    mpData->maLocale = rLocale;

    if ( !rLocale.Language.getLength() )
        mpData->meLanguage = LANGUAGE_SYSTEM;
    else
        mpData->meLanguage = MsLangId::convertLocaleToLanguage( rLocale );
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

// -----------------------------------------------------------------------

void AllSettings::SetUILocale( const ::com::sun::star::lang::Locale& )
{
    // there is only one UILocale per process
}

// -----------------------------------------------------------------------

void AllSettings::SetLanguage( LanguageType eLang )
{
    if ( eLang != mpData->meLanguage )
    {
        CopyData();

        mpData->meLanguage = eLang;
        MsLangId::convertLanguageToLocale( GetLanguage(), ((AllSettings*)this)->mpData->maLocale );
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

void AllSettings::SetUILanguage( LanguageType  )
{
    // there is only one UILanguage per process
}

// -----------------------------------------------------------------------

sal_Bool AllSettings::GetLayoutRTL() const
{
    static const char* pEnv = getenv("SAL_RTL_ENABLED" );
    static int  nUIMirroring = -1;   // -1: undef, 0: auto, 1: on 2: off

    // environment always overrides
    if( pEnv )
        return true;

    sal_Bool bRTL = sal_False;

    if( nUIMirroring == -1 )
    {
        nUIMirroring = 0; // ask configuration only once
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.Common/I18N/CTL")) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            sal_Bool bTmp = sal_Bool();
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString(RTL_CONSTASCII_USTRINGPARAM("UIMirroring")) );
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
            aLang = pSVData->maAppData.mpSettings->GetUILanguage();
        bRTL = MsLangId::isRightToLeft( aLang );
    }
    else
        bRTL = (nUIMirroring == 1);

    return bRTL;
}

// -----------------------------------------------------------------------

const ::com::sun::star::lang::Locale& AllSettings::GetLocale() const
{
    if ( !mpData->maLocale.Language.getLength() )
        mpData->maLocale = mpData->maSysLocale.GetLocale();

    return mpData->maLocale;
}

// -----------------------------------------------------------------------

const ::com::sun::star::lang::Locale& AllSettings::GetUILocale() const
{
    // the UILocale is never changed
    if ( !mpData->maUILocale.Language.getLength() )
        mpData->maUILocale = mpData->maSysLocale.GetUILocale();

    return mpData->maUILocale;
}

// -----------------------------------------------------------------------

LanguageType AllSettings::GetLanguage() const
{
    // meLanguage == LANGUAGE_SYSTEM means: use settings from SvtSysLocale
    if ( mpData->meLanguage == LANGUAGE_SYSTEM )
        return mpData->maSysLocale.GetLanguage();

    return mpData->meLanguage;
}

// -----------------------------------------------------------------------

LanguageType AllSettings::GetUILanguage() const
{
    // the UILanguage is never changed
    return mpData->maSysLocale.GetUILanguage();
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& AllSettings::GetLocaleDataWrapper() const
{
    if ( !mpData->mpLocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpLocaleDataWrapper = new LocaleDataWrapper( vcl::unohelper::GetMultiServiceFactory(), GetLocale() );
    return *mpData->mpLocaleDataWrapper;
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& AllSettings::GetUILocaleDataWrapper() const
{
    if ( !mpData->mpUILocaleDataWrapper )
        ((AllSettings*)this)->mpData->mpUILocaleDataWrapper = new LocaleDataWrapper( vcl::unohelper::GetMultiServiceFactory(), GetUILocale() );
    return *mpData->mpUILocaleDataWrapper;
}

// -----------------------------------------------------------------------

const vcl::I18nHelper& AllSettings::GetLocaleI18nHelper() const
{
    if ( !mpData->mpI18nHelper ) {
        ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> aFactory(vcl::unohelper::GetMultiServiceFactory());
        ((AllSettings*)this)->mpData->mpI18nHelper = new vcl::I18nHelper( aFactory, GetLocale() );
    }
    return *mpData->mpI18nHelper;
}

// -----------------------------------------------------------------------

const vcl::I18nHelper& AllSettings::GetUILocaleI18nHelper() const
{
    if ( !mpData->mpUII18nHelper ) {
        ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> aFactory(vcl::unohelper::GetMultiServiceFactory());
        ((AllSettings*)this)->mpData->mpUII18nHelper = new vcl::I18nHelper( aFactory, GetUILocale() );
    }
    return *mpData->mpUII18nHelper;
}

void AllSettings::LocaleSettingsChanged( sal_uInt32 nHint )
{
    AllSettings aAllSettings( Application::GetSettings() );
    if ( nHint & SYSLOCALEOPTIONS_HINT_DECSEP )
    {
        MiscSettings aMiscSettings = aAllSettings.GetMiscSettings();
        sal_Bool bIsDecSepAsLocale = aAllSettings.mpData->maSysLocale.GetOptions().IsDecimalSeparatorAsLocale();
        if ( aMiscSettings.GetEnableLocalizedDecimalSep() != bIsDecSepAsLocale )
        {
            aMiscSettings.SetEnableLocalizedDecimalSep( bIsDecSepAsLocale );
            aAllSettings.SetMiscSettings( aMiscSettings );
        }
    }

    if ( (nHint & SYSLOCALEOPTIONS_HINT_LOCALE) )
        aAllSettings.SetLocale( aAllSettings.mpData->maSysLocale.GetOptions().GetLocale() );

    Application::SetSettings( aAllSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
