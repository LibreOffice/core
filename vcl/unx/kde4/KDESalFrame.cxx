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

#define Region QtXRegion

#include <QColor>
#include <QStyle>

#include <kconfig.h>
#include <kglobal.h>
#include <kmenubar.h>
#include <kconfiggroup.h>
#include <kmainwindow.h>
#include <kapplication.h>
#include <ktoolbar.h>
#include <qdebug.h>
#include <qtooltip.h>

#undef Region

#include "KDESalFrame.hxx"
#include "KDEXLib.hxx"
#include "KDESalGraphics.hxx"

#include <tools/color.hxx>

#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <vcl/fontmanager.hxx>

#include <svdata.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

KDESalFrame::KDESalFrame( SalFrame* pParent, sal_uLong nState ) :
    X11SalFrame( pParent, nState )
{
}

void KDESalFrame::Show( sal_Bool bVisible, sal_Bool bNoActivate )
{
    if ( !GetParent() && ! (GetStyle() & SAL_FRAME_STYLE_INTRO) )
    {
        KDEXLib* pXLib = static_cast<KDEXLib*>(GetDisplay()->GetXLib());
        pXLib->doStartup();
    }

    X11SalFrame::Show( bVisible, bNoActivate );
}

/** Helper function to convert colors.
*/
static Color toColor( const QColor &rColor )
{
    return Color( rColor.red(), rColor.green(), rColor.blue() );
}

/** Helper function to read untranslated text entry from KConfig configuration repository.
*/
static OUString readEntryUntranslated( KConfigGroup *pGroup, const char *pKey )
{
    return OUString::createFromAscii( (const char *) pGroup->readEntryUntranslated( pKey ).toAscii() );
}

/** Helper function to add information to Font from QFont.

    Mostly grabbed from the Gtk+ vclplug (salnativewidgets-gtk.cxx).
*/
static Font toFont( const QFont &rQFont, const ::com::sun::star::lang::Locale& rLocale )
{
    psp::FastPrintFontInfo aInfo;
    QFontInfo qFontInfo( rQFont );

    // set family name
    aInfo.m_aFamilyName = OUString( (const char *) rQFont.family().toUtf8(), strlen( (const char *) rQFont.family().toUtf8() ), RTL_TEXTENCODING_UTF8 );

    // set italic
    aInfo.m_eItalic = ( qFontInfo.italic()? ITALIC_NORMAL: ITALIC_NONE );

    // set weight
    int nWeight = qFontInfo.weight();
    if ( nWeight <= QFont::Light )
        aInfo.m_eWeight = WEIGHT_LIGHT;
    else if ( nWeight <= QFont::Normal )
        aInfo.m_eWeight = WEIGHT_NORMAL;
    else if ( nWeight <= QFont::DemiBold )
        aInfo.m_eWeight = WEIGHT_SEMIBOLD;
    else if ( nWeight <= QFont::Bold )
        aInfo.m_eWeight = WEIGHT_BOLD;
    else
        aInfo.m_eWeight = WEIGHT_ULTRABOLD;

    // set width
    int nStretch = rQFont.stretch();
    if ( nStretch <= QFont::UltraCondensed )
        aInfo.m_eWidth = WIDTH_ULTRA_CONDENSED;
    else if ( nStretch <= QFont::ExtraCondensed )
        aInfo.m_eWidth = WIDTH_EXTRA_CONDENSED;
    else if ( nStretch <= QFont::Condensed )
        aInfo.m_eWidth = WIDTH_CONDENSED;
    else if ( nStretch <= QFont::SemiCondensed )
        aInfo.m_eWidth = WIDTH_SEMI_CONDENSED;
    else if ( nStretch <= QFont::Unstretched )
        aInfo.m_eWidth = WIDTH_NORMAL;
    else if ( nStretch <= QFont::SemiExpanded )
        aInfo.m_eWidth = WIDTH_SEMI_EXPANDED;
    else if ( nStretch <= QFont::Expanded )
        aInfo.m_eWidth = WIDTH_EXPANDED;
    else if ( nStretch <= QFont::ExtraExpanded )
        aInfo.m_eWidth = WIDTH_EXTRA_EXPANDED;
    else
        aInfo.m_eWidth = WIDTH_ULTRA_EXPANDED;

    SAL_INFO( "vcl.kde4", "font name BEFORE system match: \"" << aInfo.m_aFamilyName << "\"" );

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rLocale );

    SAL_INFO( "vcl.kde4", "font match " <<
              (aInfo.m_nID != 0 ? "succeeded" : "failed") <<
              ", name AFTER: \"" << aInfo.m_aFamilyName << "\"" );

    // font height
    int nPointHeight = qFontInfo.pointSize();
    if ( nPointHeight <= 0 )
        nPointHeight = rQFont.pointSize();

    // Create the font
    Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
    if( aInfo.m_eWeight != WEIGHT_DONTKNOW )
        aFont.SetWeight( aInfo.m_eWeight );
    if( aInfo.m_eWidth != WIDTH_DONTKNOW )
        aFont.SetWidthType( aInfo.m_eWidth );
    if( aInfo.m_eItalic != ITALIC_DONTKNOW )
        aFont.SetItalic( aInfo.m_eItalic );
    if( aInfo.m_ePitch != PITCH_DONTKNOW )
        aFont.SetPitch( aInfo.m_ePitch );

    return aFont;
}

/** Implementation of KDE integration's main method.
*/
void KDESalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings style( rSettings.GetStyleSettings() );
    bool bSetTitleFont = false;

    // General settings
    QPalette pal = kapp->palette();

    style.SetToolbarIconSize( STYLE_TOOLBAR_ICONSIZE_LARGE );

    style.SetActiveColor(toColor(pal.color(QPalette::Active, QPalette::Window)));
    style.SetDeactiveColor(toColor(pal.color(QPalette::Inactive, QPalette::Window)));

    style.SetActiveColor2(toColor(pal.color(QPalette::Active, QPalette::Window)));
    style.SetDeactiveColor2(toColor(pal.color(QPalette::Inactive, QPalette::Window)));

    style.SetActiveTextColor(toColor(pal.color(QPalette::Active, QPalette::WindowText)));
    style.SetDeactiveTextColor(toColor(pal.color(QPalette::Inactive, QPalette::WindowText)));

    // WM settings
    KConfig *pConfig = KGlobal::config().data();
    if ( pConfig )
    {
        KConfigGroup aGroup = pConfig->group( "WM" );
        const char *pKey;

        pKey = "titleFont";
        if ( aGroup.hasKey( pKey ) )
        {
            Font aFont = toFont( aGroup.readEntry( pKey, QFont() ), rSettings.GetUILanguageTag().getLocale() );
            style.SetTitleFont( aFont );
            bSetTitleFont = true;
        }

        aGroup = pConfig->group( "Icons" );

        pKey = "Theme";
        if ( aGroup.hasKey( pKey ) )
            style.SetPreferredSymbolsStyleName( readEntryUntranslated( &aGroup, pKey ) );

        //toolbar
        pKey = "toolbarFont";
        if ( aGroup.hasKey( pKey ) )
        {
            Font aFont = toFont( aGroup.readEntry( pKey, QFont() ), rSettings.GetUILanguageTag().getLocale() );
            style.SetToolFont( aFont );
        }
    }

    Color aFore = toColor( pal.color( QPalette::Active, QPalette::WindowText ) );
    Color aBack = toColor( pal.color( QPalette::Active, QPalette::Window ) );
    Color aText = toColor( pal.color( QPalette::Active, QPalette::Text ) );
    Color aBase = toColor( pal.color( QPalette::Active, QPalette::Base ) );
    Color aButn = toColor( pal.color( QPalette::Active, QPalette::ButtonText ) );
    Color aMid = toColor( pal.color( QPalette::Active, QPalette::Mid ) );
    Color aHigh = toColor( pal.color( QPalette::Active, QPalette::Highlight ) );

    // Foreground
    style.SetRadioCheckTextColor( aFore );
    style.SetLabelTextColor( aFore );
    style.SetInfoTextColor( aFore );
    style.SetDialogTextColor( aFore );
    style.SetGroupTextColor( aFore );

    // Text
    style.SetFieldTextColor( aText );
    style.SetFieldRolloverTextColor( aText );
    style.SetWindowTextColor( aText );

    // Base
    style.SetFieldColor( aBase );
    style.SetWindowColor( aBase );
    style.SetActiveTabColor( aBase );

    // Buttons
    style.SetButtonTextColor( aButn );
    style.SetButtonRolloverTextColor( aButn );

    // Disable color
    style.SetDisableColor( aMid );

    // Workspace
    style.SetWorkspaceColor( aMid );

    // Background
    style.Set3DColors( aBack );
    style.SetFaceColor( aBack );
    style.SetInactiveTabColor( aBack );
    style.SetDialogColor( aBack );
    style.SetCheckedColorSpecialCase( );

    // Selection
    style.SetHighlightColor( aHigh );
    style.SetHighlightTextColor( toColor(pal.color( QPalette::HighlightedText))  );

    // Tooltip
    style.SetHelpColor( toColor( QToolTip::palette().color( QPalette::Active, QPalette::ToolTipBase )));
    style.SetHelpTextColor( toColor( QToolTip::palette().color( QPalette::Active, QPalette::ToolTipText )));

    // Font
    Font aFont = toFont( kapp->font(), rSettings.GetUILanguageTag().getLocale() );

    style.SetAppFont( aFont );

    style.SetMenuFont( aFont ); // will be changed according to pMenuBar
    //style.SetToolFont( aFont ); //already set above
    style.SetLabelFont( aFont );
    style.SetInfoFont( aFont );
    style.SetRadioCheckFont( aFont );
    style.SetPushButtonFont( aFont );
    style.SetFieldFont( aFont );
    style.SetIconFont( aFont );
    style.SetGroupFont( aFont );

    aFont.SetWeight( WEIGHT_BOLD );
    if( !bSetTitleFont )
    {
        style.SetTitleFont( aFont );
    }
    style.SetFloatTitleFont( aFont );

    style.SetHelpFont( toFont( QToolTip::font(), rSettings.GetUILanguageTag().getLocale()));

    int flash_time = QApplication::cursorFlashTime();
    style.SetCursorBlinkTime( flash_time != 0 ? flash_time/2 : STYLE_CURSOR_NOBLINKTIME );

    // Menu
    style.SetSkipDisabledInMenus( TRUE );
    KMenuBar* pMenuBar = new KMenuBar();
    if ( pMenuBar )
    {
        // Color
        QPalette qMenuCG = pMenuBar->palette();

        // Menu text and background color, theme specific
        Color aMenuFore = toColor( qMenuCG.color( QPalette::ButtonText ) );
        Color aMenuBack = toColor( qMenuCG.color( QPalette::Button ) );

        style.SetMenuTextColor( aMenuFore );
        style.SetMenuBarTextColor( aMenuFore );
        style.SetMenuColor( aMenuBack );
        style.SetMenuBarColor( aMenuBack );
        style.SetMenuHighlightColor( toColor ( qMenuCG.color( QPalette::Highlight ) ) );
        style.SetMenuHighlightTextColor( aMenuFore );

        // set special menubar higlight text color
        if ( kapp->style()->inherits( "HighContrastStyle" ) )
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = toColor( qMenuCG.color( QPalette::HighlightedText ) );
        else
            ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor = aMenuFore;

        // set menubar rollover color
        if ( pMenuBar->style()->styleHint( QStyle::SH_MenuBar_MouseTracking ) )
        {
            style.SetMenuBarRolloverColor( toColor ( qMenuCG.color( QPalette::Highlight ) ) );
            style.SetMenuBarRolloverTextColor( ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor );
        }
        else
        {
            style.SetMenuBarRolloverColor( aMenuBack );
            style.SetMenuBarRolloverTextColor( aMenuFore );
        }

        // Font
        aFont = toFont( pMenuBar->font(), rSettings.GetUILanguageTag().getLocale() );
        style.SetMenuFont( aFont );
    }

    delete pMenuBar;

    // Scroll bar size
    style.SetScrollBarSize( kapp->style()->pixelMetric( QStyle::PM_ScrollBarExtent ) );
    style.SetMinThumbSize( kapp->style()->pixelMetric( QStyle::PM_ScrollBarSliderMin ));

    rSettings.SetStyleSettings( style );
}


void KDESalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].pGraphics == pGraphics )
        {
            m_aGraphics[i].bInUse = false;
            break;
        }
    }
}

void KDESalFrame::updateGraphics( bool bClear )
{
    Drawable aDrawable = bClear ? None : GetWindow();
    for( int i = 0; i < nMaxGraphics; i++ )
    {
        if( m_aGraphics[i].bInUse )
            m_aGraphics[i].pGraphics->SetDrawable( aDrawable, GetScreenNumber() );
    }
}

KDESalFrame::~KDESalFrame()
{
}

KDESalFrame::GraphicsHolder::~GraphicsHolder()
{
    delete pGraphics;
}

SalGraphics* KDESalFrame::GetGraphics()
{
    if( GetWindow() )
    {
        for( int i = 0; i < nMaxGraphics; i++ )
        {
            if( ! m_aGraphics[i].bInUse )
            {
                m_aGraphics[i].bInUse = true;
                if( ! m_aGraphics[i].pGraphics )
                {
                    m_aGraphics[i].pGraphics = new KDESalGraphics();
                    m_aGraphics[i].pGraphics->Init( this, GetWindow(), GetScreenNumber() );
                }
                return m_aGraphics[i].pGraphics;
            }
        }
    }

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
