/*************************************************************************
 *
 *  $RCSfile: settings.cxx,v $
 *
 *  $Revision: 1.49 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 14:01:03 $
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

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif

#ifndef _VCL_I18NHELP_HXX
#include <i18nhelp.hxx>
#endif

#ifndef _VCL_FONTCFG_HXX
#include <fontcfg.hxx>
#endif

#include <unohelp.hxx>
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

#ifdef UNX
#include <prex.h>
#include <postx.h>
#include <dtint.hxx>
#include <stdio.h>
#endif

#ifdef WNT
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#endif

using namespace rtl;



// =======================================================================

DBG_NAME( AllSettings );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MachineSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MachineSettings: RefCount overflow" );

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

BOOL MachineSettings::operator ==( const MachineSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnOptions                     == rSet.mpData->mnOptions)                  &&
         (mpData->mnScreenOptions               == rSet.mpData->mnScreenOptions)            &&
         (mpData->mnPrintOptions                == rSet.mpData->mnPrintOptions)             &&
         (mpData->mnScreenRasterFontDeviation   == rSet.mpData->mnScreenRasterFontDeviation) )
        return TRUE;
    else
        return FALSE;
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
    mbContextMenuDown           = TRUE;
    mnMiddleButtonAction        = MOUSE_MIDDLE_AUTOSCROLL;
    mnScrollRepeat              = 100;
    mnButtonStartRepeat         = 370;
    mnButtonRepeat              = 90;
    mnActionDelay               = 250;
    mnMenuDelay                 = 150;
    mnFollow                    = MOUSE_FOLLOW_MENU | MOUSE_FOLLOW_DDLIST;
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
}

// -----------------------------------------------------------------------

MouseSettings::MouseSettings()
{
    mpData = new ImplMouseData();
}

// -----------------------------------------------------------------------

MouseSettings::MouseSettings( const MouseSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MouseSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MouseSettings: RefCount overflow" );

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

BOOL MouseSettings::operator ==( const MouseSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

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
         (mpData->mnFollow              == rSet.mpData->mnFollow) )
        return TRUE;
    else
        return FALSE;
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "KeyboardSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "KeyboardSettings: RefCount overflow" );

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

BOOL KeyboardSettings::operator ==( const KeyboardSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions) )
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

ImplStyleData::ImplStyleData()
{
    mnRefCount                  = 1;
    mnScrollBarSize             = 16;
    mnSplitSize                 = 3;
    mnSpinSize                  = 16;
    mnIconHorzSpace             = 50;
    mnIconVertSpace             = 40;
    mnAntialiasedMin            = 0;
    mnCursorSize                = 2;
    mnCursorBlinkTime           = STYLE_CURSOR_NOBLINKTIME;
    mnScreenZoom                = 100;
    mnScreenFontZoom            = 100;
    mnRadioButtonStyle          = 0;
    mnCheckBoxStyle             = 0;
    mnPushButtonStyle           = 0;
    mnTabControlStyle           = 0;
    mnLogoDisplayTime           = LOGO_DISPLAYTIME_STARTTIME;
    mnDragFullOptions           = 0;
    mnAnimationOptions          = 0;
    mnSelectionOptions          = 0;
    mnDisplayOptions            = 0;
    mnOptions                   = 0;
    mnAutoMnemonic              = 1;
    mnToolbarIconSize           = STYLE_TOOLBAR_ICONSIZE_UNKNOWN;

    SetStandardStyles();
}

// -----------------------------------------------------------------------

ImplStyleData::ImplStyleData( const ImplStyleData& rData ) :
    maActiveBorderColor( rData.maActiveBorderColor ),
    maActiveColor( rData.maActiveColor ),
    maActiveColor2( rData.maActiveColor2 ),
    maActiveTextColor( rData.maActiveTextColor ),
    maButtonTextColor( rData.maButtonTextColor ),
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
    maFontColor( rData.maFontColor ),
    maGroupTextColor( rData.maGroupTextColor ),
    maHelpColor( rData.maHelpColor ),
    maHelpTextColor( rData.maHelpTextColor ),
    maHighlightColor( rData.maHighlightColor ),
    maHighlightLinkColor( rData.maLinkColor ),
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
    maRadioCheckTextColor( rData.maRadioCheckTextColor ),
    maShadowColor( rData.maShadowColor ),
    maVisitedLinkColor( rData.maLinkColor ),
    maWindowColor( rData.maWindowColor ),
    maWindowTextColor( rData.maWindowTextColor ),
    maWorkspaceColor( rData.maWorkspaceColor ),
    maMonoColor( rData.maMonoColor ),
    maAppFont( rData.maAppFont ),
    maHelpFont( rData.maAppFont ),
    maTitleFont( rData.maTitleFont ),
    maFloatTitleFont( rData.maFloatTitleFont ),
    maMenuFont( rData.maMenuFont ),
    maToolFont( rData.maToolFont ),
    maLabelFont( rData.maLabelFont ),
    maGroupFont( rData.maGroupFont ),
    maInfoFont( rData.maInfoFont ),
    maRadioCheckFont( rData.maRadioCheckFont ),
    maPushButtonFont( rData.maPushButtonFont ),
    maFieldFont( rData.maFieldFont ),
    maIconFont( rData.maIconFont )
{
    mnRefCount                  = 1;
    mnBorderSize                = rData.mnBorderSize;
    mnTitleHeight               = rData.mnTitleHeight;
    mnFloatTitleHeight          = rData.mnFloatTitleHeight;
    mnTearOffTitleHeight        = rData.mnTearOffTitleHeight;
    mnMenuBarHeight             = rData.mnMenuBarHeight;
    mnScrollBarSize             = rData.mnScrollBarSize;
    mnSplitSize                 = rData.mnSplitSize;
    mnSpinSize                  = rData.mnSpinSize;
    mnIconHorzSpace             = rData.mnIconHorzSpace;
    mnIconVertSpace             = rData.mnIconVertSpace;
    mnAntialiasedMin            = rData.mnAntialiasedMin;
    mnCursorSize                = rData.mnCursorSize;
    mnCursorBlinkTime           = rData.mnCursorBlinkTime;
    mnScreenZoom                = rData.mnScreenZoom;
    mnScreenFontZoom            = rData.mnScreenFontZoom;
    mnRadioButtonStyle          = rData.mnRadioButtonStyle;
    mnCheckBoxStyle             = rData.mnCheckBoxStyle;
    mnPushButtonStyle           = rData.mnPushButtonStyle;
    mnTabControlStyle           = rData.mnTabControlStyle;
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
    mnToolbarIconSize           = rData.mnToolbarIconSize;
}

// -----------------------------------------------------------------------

void ImplStyleData::SetStandardStyles()
{
    Font aStdFont( FAMILY_SWISS, Size( 0, 8 ) );
    aStdFont.SetCharSet( gsl_getSystemTextEncoding() );
    aStdFont.SetWeight( WEIGHT_NORMAL );
    aStdFont.SetName( vcl::DefaultFontConfigItem::get()->getUserInterfaceFont(com::sun::star::lang::Locale( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("en") ), rtl::OUString(), rtl::OUString() ) ) );
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
    maFloatTitleFont            = aStdFont;
    aStdFont.SetWeight( WEIGHT_BOLD );
    maTitleFont                 = aStdFont;

    maFaceColor                 = Color( COL_LIGHTGRAY );
    maCheckedColor              = Color( 0xCC, 0xCC, 0xCC );
    maLightColor                = Color( COL_WHITE );
    maLightBorderColor          = Color( COL_LIGHTGRAY );
    maShadowColor               = Color( COL_GRAY );
    maDarkShadowColor           = Color( COL_BLACK );
    maButtonTextColor           = Color( COL_BLACK );
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
    maMenuHighlightColor        = Color( COL_BLUE );
    maMenuHighlightTextColor    = Color( COL_WHITE );
    maHighlightColor            = Color( COL_BLUE );
    maHighlightTextColor        = Color( COL_WHITE );
    maDisableColor              = Color( COL_GRAY );
    maHelpColor                 = Color( 0xFF, 0xFF, 0xE0 );
    maHelpTextColor             = Color( COL_BLACK );
    maLinkColor                 = Color( COL_BLUE );
    maVisitedLinkColor          = Color( COL_RED );
    maHighlightLinkColor        = Color( COL_LIGHTBLUE );
    maFontColor                 = Color( COL_BLACK );

    mnRadioButtonStyle         &= ~STYLE_RADIOBUTTON_STYLE;
    mnCheckBoxStyle            &= ~STYLE_CHECKBOX_STYLE;
    mnPushButtonStyle          &= ~STYLE_PUSHBUTTON_STYLE;
    mnTabControlStyle           = 0;

    mnOptions                  &= ~(STYLE_OPTION_SYSTEMSTYLE | STDSYS_STYLE);
    mnBorderSize                = 1;
    mnTitleHeight               = 18;
    mnFloatTitleHeight          = 13;
    mnTearOffTitleHeight        = 8;
    mnMenuBarHeight             = 14;
    mnHighContrast              = 0;
    mnUseSystemUIFonts          = 1;
    mnUseFlatBorders            = 0;
    mnUseFlatMenues             = 0;
    mnUseImagesInMenus          = (USHORT)TRUE;
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings()
{
    mpData = new ImplStyleData();
}

// -----------------------------------------------------------------------

StyleSettings::StyleSettings( const StyleSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "StyleSettings: RefCount overflow" );

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
        ULONG   nRed    = mpData->maLightColor.GetRed();
        ULONG   nGreen  = mpData->maLightColor.GetGreen();
        ULONG   nBlue   = mpData->maLightColor.GetBlue();
        nRed   += (ULONG)(mpData->maShadowColor.GetRed());
        nGreen += (ULONG)(mpData->maShadowColor.GetGreen());
        nBlue  += (ULONG)(mpData->maShadowColor.GetBlue());
        mpData->maCheckedColor = Color( (BYTE)(nRed/2), (BYTE)(nGreen/2), (BYTE)(nBlue/2) );
    }
    else
    {
        mpData->maCheckedColor  = Color( 0x99, 0x99, 0x99 );
        mpData->maLightColor    = Color( COL_WHITE );
        mpData->maShadowColor   = Color( COL_GRAY );
    }
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardStyles()
{
    CopyData();
    mpData->SetStandardStyles();
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardWinStyles()
{
    return; // no more style changes since NWF
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardOS2Styles()
{
    return; // no more style changes since NWF
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardMacStyles()
{
    return; // no more style changes since NWF
}

// -----------------------------------------------------------------------

void StyleSettings::SetStandardUnixStyles()
{
    return; // no more style changes since NWF
}


// -----------------------------------------------------------------------

const StyleSettings& StyleSettings::operator =( const StyleSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "StyleSettings: RefCount overflow" );

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

inline BOOL ImplIsBackOrWhite( const Color& rColor )
{
    UINT8 nLuminance = rColor.GetLuminance();
    return ( nLuminance < 8 ) || ( nLuminance > 250 );
}

BOOL StyleSettings::IsHighContrastBlackAndWhite() const
{
    BOOL bBWOnly = TRUE;

    // Only use B&W if fully B&W, like on GNOME.
    // Some colors like CheckedColor and HighlightColor are not B&W in Windows Standard HC Black,
    // and we don't want to be B&W then, so check these color first, very probably not B&W.

    // Unfortunately, GNOME uses a very very dark color (0x000033) instead of BLACK (0x000000)

    if ( !ImplIsBackOrWhite( GetFaceColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetHighlightTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetWindowColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetWindowTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetButtonTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetButtonTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetGroupTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetLabelTextColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetDialogColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetFieldColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetMenuColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetMenuBarColor() ) )
        bBWOnly = FALSE;
    else if ( !ImplIsBackOrWhite( GetMenuHighlightColor() ) )
        bBWOnly = FALSE;

    return bBWOnly;
}

// -----------------------------------------------------------------------

BOOL StyleSettings::operator ==( const StyleSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

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
         (mpData->mnSplitSize               == rSet.mpData->mnSplitSize)                &&
         (mpData->mnSpinSize                == rSet.mpData->mnSpinSize)                 &&
         (mpData->mnIconHorzSpace           == rSet.mpData->mnIconHorzSpace)            &&
         (mpData->mnIconVertSpace           == rSet.mpData->mnIconVertSpace)            &&
         (mpData->mnAntialiasedMin          == rSet.mpData->mnAntialiasedMin)           &&
         (mpData->mnScreenZoom              == rSet.mpData->mnScreenZoom)               &&
         (mpData->mnScreenFontZoom          == rSet.mpData->mnScreenFontZoom)           &&
         (mpData->mnRadioButtonStyle        == rSet.mpData->mnRadioButtonStyle)         &&
         (mpData->mnCheckBoxStyle           == rSet.mpData->mnCheckBoxStyle)            &&
         (mpData->mnPushButtonStyle         == rSet.mpData->mnPushButtonStyle)          &&
         (mpData->mnTabControlStyle         == rSet.mpData->mnTabControlStyle)          &&
         (mpData->mnHighContrast            == rSet.mpData->mnHighContrast)             &&
         (mpData->mnUseSystemUIFonts        == rSet.mpData->mnUseSystemUIFonts)         &&
         (mpData->mnUseFlatBorders          == rSet.mpData->mnUseFlatBorders)           &&
         (mpData->mnUseFlatMenues           == rSet.mpData->mnUseFlatMenues)            &&
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
         (mpData->maMenuHighlightColor      == rSet.mpData->maMenuHighlightColor)       &&
         (mpData->maMenuHighlightTextColor  == rSet.mpData->maMenuHighlightTextColor)   &&
         (mpData->maHighlightColor          == rSet.mpData->maHighlightColor)           &&
         (mpData->maHighlightTextColor      == rSet.mpData->maHighlightTextColor)       &&
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
         (mpData->maFontColor               == rSet.mpData->maFontColor ))
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

ImplMiscData::ImplMiscData()
{
    mnRefCount                  = 1;
    mnTwoDigitYearStart         = 1930;
    mnEnableATT                 = ~0;
    static const char* pEnv = getenv("SAL_DECIMALSEP_ENABLED" ); // set default without UI
    mbEnableLocalizedDecimalSep = (pEnv != NULL) ? TRUE : FALSE;
}

// -----------------------------------------------------------------------

ImplMiscData::ImplMiscData( const ImplMiscData& rData )
{
    mnRefCount                  = 1;
    mnTwoDigitYearStart         = rData.mnTwoDigitYearStart;
    mnEnableATT                 = rData.mnEnableATT;
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MiscSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "MiscSettings: RefCount overflow" );

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

BOOL MiscSettings::operator ==( const MiscSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnTwoDigitYearStart   == rSet.mpData->mnTwoDigitYearStart ) &&
         (mpData->mnEnableATT           == rSet.mpData->mnEnableATT ) &&
         (mpData->mbEnableLocalizedDecimalSep == rSet.mpData->mbEnableLocalizedDecimalSep ) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL MiscSettings::GetEnableATToolSupport() const
{
    if( mpData->mnEnableATT == (USHORT)~0 )
    {
#ifdef UNX
        mpData->mnEnableATT = 0;

        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        {
            char buf[16];
            // use 2 shells to suppress the eventual "gcontool-2 not found" message
            // of the shell trying to execute the command
            FILE* fp = popen( "/bin/sh 2>/dev/null -c \"gconftool-2 -g /desktop/gnome/interface/accessibility\"", "r" );
            if( fp )
            {
                if( fgets( buf, sizeof(buf), fp ) )
                {
                    int nCompare = strncasecmp( buf, "true", 4 );
                    mpData->mnEnableATT = (nCompare == 0 ? 1 : 0);
                }
                pclose( fp );
            }
        }
#else
        static const char* pEnv = getenv("SAL_ACCESSIBILITY_ENABLED" );
        if( !pEnv || !*pEnv )
        {
#ifdef WNT
            // Check in the Windows registry if an AT tool wants Accessibility support to
            // be activated ..
            HKEY hkey;

            if( ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER,
                "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
                &hkey) )
            {
                DWORD dwType;
                WIN_BYTE Data[6]; // possible values: "true", "false", "1", "0", DWORD
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
                            mpData->mnEnableATT = (USHORT) (((DWORD *) Data)[0]);
                            break;
                        default:
                            // Unsupported registry type
                            mpData->mnEnableATT = 0;
                            break;
                    }
                }

                RegCloseKey(hkey);
            }

            // Registry does not exist ..
            if( mpData->mnEnableATT == (USHORT)~0 )
            {
                rtl::OUString aEnable =
                    vcl::SettingsConfigItem::get()->
                    getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Accessibility" ) ),
                              rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableATToolSupport" ) ) );
                mpData->mnEnableATT = aEnable.equalsIgnoreAsciiCaseAscii( "true" ) ? 1 : 0;
            }
#else
            rtl::OUString aEnable =
                vcl::SettingsConfigItem::get()->
                getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Accessibility" ) ),
                          rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableATToolSupport" ) ) );
            mpData->mnEnableATT = aEnable.equalsIgnoreAsciiCaseAscii( "true" ) ? 1 : 0;
#endif
        }
        else
            mpData->mnEnableATT = 1;
#endif
    }
    return (BOOL)mpData->mnEnableATT;

}

// -----------------------------------------------------------------------

void MiscSettings::SetEnableATToolSupport( BOOL bEnable )
{
    if ( bEnable != mpData->mnEnableATT )
    {
        BOOL bDummy;
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
            WIN_BYTE Data[6]; // possible values: "true", "false", 1, 0
            DWORD cbData = sizeof(Data);

            if( ERROR_SUCCESS == RegQueryValueEx(hkey, "SupportAssistiveTechnology",
                NULL,   &dwType, Data, &cbData) )
            {
                switch (dwType)
                {
                    case REG_SZ:
                        RegSetValueEx(hkey, "SupportAssistiveTechnology",
                            NULL, dwType,
                            bEnable ? (WIN_BYTE *) "true" : (WIN_BYTE *) "false",
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
                      rtl::OUString::createFromAscii( bEnable ? "true" : "false" ) );
        mpData->mnEnableATT = bEnable ? 1 : 0;
    }
}

void MiscSettings::SetEnableLocalizedDecimalSep( BOOL bEnable )
{
    CopyData();
    mpData->mbEnableLocalizedDecimalSep = bEnable;
}

BOOL MiscSettings::GetEnableLocalizedDecimalSep() const
{
    return mpData->mbEnableLocalizedDecimalSep;
}



// =======================================================================

ImplSoundData::ImplSoundData()
{
    mnRefCount                  = 1;
    mnOptions                   = 0;
}

// -----------------------------------------------------------------------

ImplSoundData::ImplSoundData( const ImplSoundData& rData )
{
    mnRefCount                  = 1;
    mnOptions                   = rData.mnOptions;
}

// -----------------------------------------------------------------------

SoundSettings::SoundSettings()
{
    mpData = new ImplSoundData();
}

// -----------------------------------------------------------------------

SoundSettings::SoundSettings( const SoundSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "SoundSettings: RefCount overflow" );

    // shared Instance Daten uebernehmen und Referenzcounter erhoehen
    mpData = rSet.mpData;
    mpData->mnRefCount++;
}

// -----------------------------------------------------------------------

SoundSettings::~SoundSettings()
{
    // Daten loeschen, wenn letzte Referenz
    if ( mpData->mnRefCount == 1 )
        delete mpData;
    else
        mpData->mnRefCount--;
}

// -----------------------------------------------------------------------

const SoundSettings& SoundSettings::operator =( const SoundSettings& rSet )
{
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "SoundSettings: RefCount overflow" );

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

void SoundSettings::CopyData()
{
    // Falls noch andere Referenzen bestehen, dann kopieren
    if ( mpData->mnRefCount != 1 )
    {
        mpData->mnRefCount--;
        mpData = new ImplSoundData( *mpData );
    }
}

// -----------------------------------------------------------------------

BOOL SoundSettings::operator ==( const SoundSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions) )
        return TRUE;
    else
        return FALSE;
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "NotificationSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "NotificationSettings: RefCount overflow" );

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

BOOL NotificationSettings::operator ==( const NotificationSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnOptions             == rSet.mpData->mnOptions) )
        return TRUE;
    else
        return FALSE;
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "HelpSettings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "HelpSettings: RefCount overflow" );

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

BOOL HelpSettings::operator ==( const HelpSettings& rSet ) const
{
    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->mnOptions         == rSet.mpData->mnOptions ) &&
         (mpData->mnTipDelay        == rSet.mpData->mnTipDelay ) &&
         (mpData->mnTipTimeout      == rSet.mpData->mnTipTimeout ) &&
         (mpData->mnBalloonDelay    == rSet.mpData->mnBalloonDelay ) )
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

static BOOL ImplCompareLocales( const ::com::sun::star::lang::Locale& L1, const ::com::sun::star::lang::Locale& L2 )
{
    return ( ( L1.Language == L2.Language ) &&
             ( L1.Country == L2.Country ) &&
             ( L1.Variant == L2.Variant ) );
}

// =======================================================================

ImplAllSettingsData::ImplAllSettingsData()
{
    mnRefCount                  = 1;
    mnSystemUpdate              = SETTINGS_ALLSETTINGS;
    mnWindowUpdate              = SETTINGS_ALLSETTINGS;
    meLanguage                  = LANGUAGE_SYSTEM;
    meUILanguage                = LANGUAGE_SYSTEM;
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpCollatorWrapper           = NULL;
    mpUICollatorWrapper         = NULL;
    mpI18nHelper                = NULL;
    mpUII18nHelper              = NULL;
}

// -----------------------------------------------------------------------

ImplAllSettingsData::ImplAllSettingsData( const ImplAllSettingsData& rData ) :
    maMouseSettings( rData.maMouseSettings ),
    maKeyboardSettings( rData.maKeyboardSettings ),
    maStyleSettings( rData.maStyleSettings ),
    maMiscSettings( rData.maMiscSettings ),
    maSoundSettings( rData.maSoundSettings ),
    maNotificationSettings( rData.maNotificationSettings ),
    maHelpSettings( rData.maHelpSettings ),
    maInternational( rData.maInternational ),
    maLocale( rData.maLocale ),
    maUILocale( rData.maUILocale )

{
    mnRefCount                  = 1;
    mnSystemUpdate              = rData.mnSystemUpdate;
    mnWindowUpdate              = rData.mnWindowUpdate;
    meLanguage                  = rData.meLanguage;
    meUILanguage                = rData.meUILanguage;
    // Pointer couldn't shared and objects haven't a copy ctor
    // So we create the cache objects new, if the GetFunction is
    // called
    mpLocaleDataWrapper         = NULL;
    mpUILocaleDataWrapper       = NULL;
    mpCollatorWrapper           = NULL;
    mpUICollatorWrapper         = NULL;
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
    if ( mpCollatorWrapper )
        delete mpCollatorWrapper;
    if ( mpUICollatorWrapper )
        delete mpUICollatorWrapper;
    if ( mpI18nHelper )
        delete mpI18nHelper;
    if ( mpI18nHelper )
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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "Settings: RefCount overflow" );

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
    DBG_ASSERT( rSet.mpData->mnRefCount < 0xFFFE, "AllSettings: RefCount overflow" );
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

ULONG AllSettings::Update( ULONG nFlags, const AllSettings& rSet )
{
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    ULONG nChangeFlags = 0;

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

    if ( nFlags & SETTINGS_SOUND )
    {
        if ( mpData->maSoundSettings != rSet.mpData->maSoundSettings )
        {
            CopyData();
            mpData->maSoundSettings = rSet.mpData->maSoundSettings;
            nChangeFlags |= SETTINGS_SOUND;
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
        if ( mpData->maInternational != rSet.mpData->maInternational )
        {
            CopyData();
            mpData->maInternational = rSet.mpData->maInternational;
            mpData->meLanguage = mpData->maInternational.GetFormatLanguage();
            mpData->meUILanguage = mpData->maInternational.GetLanguage();
            // Will be calculated in GetLocale()/GetUILocale()
            mpData->maLocale = ::com::sun::star::lang::Locale();
            mpData->maUILocale = ::com::sun::star::lang::Locale();
            nChangeFlags |= SETTINGS_INTERNATIONAL;
            if ( mpData->mpLocaleDataWrapper )
            {
                delete mpData->mpLocaleDataWrapper;
                mpData->mpLocaleDataWrapper = NULL;
            }
            if ( mpData->mpUILocaleDataWrapper )
            {
                delete mpData->mpUILocaleDataWrapper;
                mpData->mpUILocaleDataWrapper = NULL;
            }
            if ( mpData->mpI18nHelper )
            {
                delete mpData->mpI18nHelper;
                mpData->mpI18nHelper = NULL;
            }
            if ( mpData->mpUII18nHelper )
            {
                delete mpData->mpUII18nHelper;
                mpData->mpUII18nHelper = NULL;
            }
        }
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
        if ( mpData->meUILanguage || rSet.mpData->meUILanguage )
        {
            SetUILanguage( rSet.mpData->meUILanguage );
            nChangeFlags |= SETTINGS_UILOCALE;
        }
    }

    return nChangeFlags;
}

// -----------------------------------------------------------------------

ULONG AllSettings::GetChangeFlags( const AllSettings& rSet ) const
{
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    ULONG nChangeFlags = 0;

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

    if ( mpData->maSoundSettings != rSet.mpData->maSoundSettings )
        nChangeFlags |= SETTINGS_SOUND;

    if ( mpData->maNotificationSettings != rSet.mpData->maNotificationSettings )
        nChangeFlags |= SETTINGS_NOTIFICATION;

    if ( mpData->maHelpSettings != rSet.mpData->maHelpSettings )
        nChangeFlags |= SETTINGS_HELP;

    if ( mpData->maInternational != rSet.mpData->maInternational )
        nChangeFlags |= SETTINGS_INTERNATIONAL;

    if ( mpData->meLanguage || rSet.mpData->meLanguage )
        nChangeFlags |= SETTINGS_LOCALE;

    if ( mpData->meUILanguage || rSet.mpData->meUILanguage )
        nChangeFlags |= SETTINGS_UILOCALE;

    return nChangeFlags;
}

// -----------------------------------------------------------------------

BOOL AllSettings::operator ==( const AllSettings& rSet ) const
{
    DBG_CHKTHIS( AllSettings, NULL );
    DBG_CHKOBJ( &rSet, AllSettings, NULL );

    if ( mpData == rSet.mpData )
        return TRUE;

    if ( (mpData->maMachineSettings         == rSet.mpData->maMachineSettings)      &&
         (mpData->maMouseSettings           == rSet.mpData->maMouseSettings)        &&
         (mpData->maKeyboardSettings        == rSet.mpData->maKeyboardSettings)     &&
         (mpData->maStyleSettings           == rSet.mpData->maStyleSettings)        &&
         (mpData->maMiscSettings            == rSet.mpData->maMiscSettings)         &&
         (mpData->maSoundSettings           == rSet.mpData->maSoundSettings)        &&
         (mpData->maNotificationSettings    == rSet.mpData->maNotificationSettings) &&
         (mpData->maHelpSettings            == rSet.mpData->maHelpSettings)         &&
         (mpData->maInternational           == rSet.mpData->maInternational)        &&
         (mpData->mnSystemUpdate            == rSet.mpData->mnSystemUpdate)         &&
         (mpData->mnWindowUpdate            == rSet.mpData->mnWindowUpdate) )
    {
        // special treatment for Locale, because maLocale is only
        // initialized after first call of GetLocale().
        ::com::sun::star::lang::Locale aEmptyLocale;
        if ( ( ImplCompareLocales( mpData->maLocale, aEmptyLocale ) && ImplCompareLocales( rSet.mpData->maLocale, aEmptyLocale ) )
            || ImplCompareLocales( GetLocale(), rSet.GetLocale() ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

// -----------------------------------------------------------------------

void AllSettings::SetLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    CopyData();

    mpData->maLocale = rLocale;

    if ( !rLocale.Language.getLength() )
        mpData->meLanguage = LANGUAGE_SYSTEM;
    else
        mpData->meLanguage = ConvertIsoNamesToLanguage( rLocale.Language, rLocale.Country );
    mpData->maInternational = International( mpData->meUILanguage, mpData->meLanguage );
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

void AllSettings::SetUILocale( const ::com::sun::star::lang::Locale& rLocale )
{
    CopyData();

    mpData->maUILocale = rLocale;

    if ( !rLocale.Language.getLength() )
        mpData->meUILanguage = LANGUAGE_SYSTEM;
    else
        mpData->meUILanguage = ConvertIsoNamesToLanguage( rLocale.Language, rLocale.Country );
    mpData->maInternational = International( mpData->meUILanguage, mpData->meLanguage );
    if ( mpData->mpUILocaleDataWrapper )
    {
        delete mpData->mpUILocaleDataWrapper;
        mpData->mpUILocaleDataWrapper = NULL;
    }
    if ( mpData->mpUII18nHelper )
    {
        delete mpData->mpUII18nHelper;
        mpData->mpUII18nHelper = NULL;
    }
}

// -----------------------------------------------------------------------

void AllSettings::SetLanguage( LanguageType eLang )
{
    CopyData();

    mpData->meLanguage = eLang;

    // Will be calculated in GetLocale()
    mpData->maLocale = ::com::sun::star::lang::Locale();
    mpData->maInternational = International( mpData->meUILanguage, mpData->meLanguage );
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

void AllSettings::SetUILanguage( LanguageType eLang  )
{
    CopyData();

    mpData->meUILanguage = eLang;

    // Will be calculated in GetUILocale()
    mpData->maUILocale = ::com::sun::star::lang::Locale();
    mpData->maInternational = International( mpData->meUILanguage, mpData->meLanguage );
    if ( mpData->mpUILocaleDataWrapper )
    {
        delete mpData->mpUILocaleDataWrapper;
        mpData->mpUILocaleDataWrapper = NULL;
    }
    if ( mpData->mpUII18nHelper )
    {
        delete mpData->mpUII18nHelper;
        mpData->mpUII18nHelper = NULL;
    }
}

// -----------------------------------------------------------------------

BOOL AllSettings::GetLayoutRTL() const
{
    static const char* pEnv = getenv("SAL_RTL_ENABLED" );
    static int  nUIMirroring = -1;   // -1: undef, 0: auto, 1: on 2: off

    // environment always overrides
    if( pEnv )
        return true;

    BOOL bRTL = FALSE;

    if( nUIMirroring == -1 )
    {
        nUIMirroring = 0; // ask configuration only once
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString::createFromAscii( "org.openoffice.Office.Common/I18N/CTL" ) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            BOOL bTmp;
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString::createFromAscii( "UIMirroring" ) );
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

        switch( aLang )
        {
            // languages with right-to-left UI
            case LANGUAGE_ARABIC:
            case LANGUAGE_ARABIC_SAUDI_ARABIA:
            case LANGUAGE_ARABIC_IRAQ:
            case LANGUAGE_ARABIC_EGYPT:
            case LANGUAGE_ARABIC_LIBYA:
            case LANGUAGE_ARABIC_ALGERIA:
            case LANGUAGE_ARABIC_MOROCCO:
            case LANGUAGE_ARABIC_TUNISIA:
            case LANGUAGE_ARABIC_OMAN:
            case LANGUAGE_ARABIC_YEMEN:
            case LANGUAGE_ARABIC_SYRIA:
            case LANGUAGE_ARABIC_JORDAN:
            case LANGUAGE_ARABIC_LEBANON:
            case LANGUAGE_ARABIC_KUWAIT:
            case LANGUAGE_ARABIC_UAE:
            case LANGUAGE_ARABIC_BAHRAIN:
            case LANGUAGE_ARABIC_QATAR:
            case LANGUAGE_HEBREW:
                bRTL = TRUE;
                break;

            default:
                break;
        }
    }
    else
        bRTL = (nUIMirroring == 1);

    return bRTL;
}

// -----------------------------------------------------------------------

const ::com::sun::star::lang::Locale& AllSettings::GetLocale() const
{
    if ( !mpData->maLocale.Language.getLength() )
    {
        String  aLanguage;
        String  aCountry;
        ConvertLanguageToIsoNames( GetLanguage(), aLanguage, aCountry );
        ((AllSettings*)this)->mpData->maLocale.Language = aLanguage;
        ((AllSettings*)this)->mpData->maLocale.Country = aCountry;
    }

    return mpData->maLocale;
}

// -----------------------------------------------------------------------

const ::com::sun::star::lang::Locale& AllSettings::GetUILocale() const
{
    if ( !mpData->maUILocale.Language.getLength() )
    {
        String  aLanguage;
        String  aCountry;
        ConvertLanguageToIsoNames( GetUILanguage(), aLanguage, aCountry );
        ((AllSettings*)this)->mpData->maUILocale.Language = aLanguage;
        ((AllSettings*)this)->mpData->maUILocale.Country = aCountry;
    }

    return mpData->maUILocale;
}

// -----------------------------------------------------------------------

LanguageType AllSettings::GetLanguage() const
{
    if ( mpData->meLanguage == LANGUAGE_SYSTEM )
        return GetSystemLanguage();

    return mpData->meLanguage;
}

// -----------------------------------------------------------------------

LanguageType AllSettings::GetUILanguage() const
{
    if ( mpData->meUILanguage == LANGUAGE_SYSTEM )
        return GetSystemUILanguage();

    return mpData->meUILanguage;
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


// -----------------------------------------------------------------------
/*
const CollatorWrapper& AllSettings::GetCollatorWrapper() const
{
    if ( !mpData->mpCollatorWrapper )
    {
        ((AllSettings*)this)->mpData->mpCollatorWrapper = new CollatorWrapper( vcl::unohelper::GetMultiServiceFactory() );
        ((AllSettings*)this)->mpData->mpCollatorWrapper->loadDefaultCollator( GetLocale(), 0 );
    }
    return *mpData->mpCollatorWrapper;
}
*/
// -----------------------------------------------------------------------
/*
const CollatorWrapper& AllSettings::GetUICollatorWrapper() const
{
    if ( !mpData->mpUICollatorWrapper )
    {
        ((AllSettings*)this)->mpData->mpUICollatorWrapper = new CollatorWrapper( vcl::unohelper::GetMultiServiceFactory() );
        ((AllSettings*)this)->mpData->mpUICollatorWrapper->loadDefaultCollator( GetUILocale(), 0 );
    }
    return *mpData->mpUICollatorWrapper;
}
*/

