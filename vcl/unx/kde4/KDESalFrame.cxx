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

#undef Region

#include "KDESalFrame.hxx"
#include "KDEXLib.hxx"
#include "KDESalGraphics.hxx"

#include <vcl/settings.hxx>
#include <vcl/font.hxx>
#include <tools/color.hxx>

#include <vcl/svdata.hxx>

#include <pspgraphics.h>

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

#if 0
#endif
/** Helper function to add information to Font from QFont.

    Mostly grabbed from the Gtk+ vclplug (salnativewidgets-gtk.cxx).
*/
static Font toFont( const QFont &rQFont, const ::com::sun::star::lang::Locale& rLocale )
{
    psp::FastPrintFontInfo aInfo;
    QFontInfo qFontInfo( rQFont );

    // set family name
    aInfo.m_aFamilyName = String( (const char *) rQFont.family().toUtf8(), RTL_TEXTENCODING_UTF8 );

    // set italic
    aInfo.m_eItalic = ( qFontInfo.italic()? psp::italic::Italic: psp::italic::Upright );

    // set weight
    int nWeight = qFontInfo.weight();
    if ( nWeight <= QFont::Light )
        aInfo.m_eWeight = psp::weight::Light;
    else if ( nWeight <= QFont::Normal )
        aInfo.m_eWeight = psp::weight::Normal;
    else if ( nWeight <= QFont::DemiBold )
        aInfo.m_eWeight = psp::weight::SemiBold;
    else if ( nWeight <= QFont::Bold )
        aInfo.m_eWeight = psp::weight::Bold;
    else
        aInfo.m_eWeight = psp::weight::UltraBold;

    // set width
    int nStretch = rQFont.stretch();
    if ( nStretch <= QFont::UltraCondensed )
        aInfo.m_eWidth = psp::width::UltraCondensed;
    else if ( nStretch <= QFont::ExtraCondensed )
        aInfo.m_eWidth = psp::width::ExtraCondensed;
    else if ( nStretch <= QFont::Condensed )
        aInfo.m_eWidth = psp::width::Condensed;
    else if ( nStretch <= QFont::SemiCondensed )
        aInfo.m_eWidth = psp::width::SemiCondensed;
    else if ( nStretch <= QFont::Unstretched )
        aInfo.m_eWidth = psp::width::Normal;
    else if ( nStretch <= QFont::SemiExpanded )
        aInfo.m_eWidth = psp::width::SemiExpanded;
    else if ( nStretch <= QFont::Expanded )
        aInfo.m_eWidth = psp::width::Expanded;
    else if ( nStretch <= QFont::ExtraExpanded )
        aInfo.m_eWidth = psp::width::ExtraExpanded;
    else
        aInfo.m_eWidth = psp::width::UltraExpanded;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font name BEFORE system match: \"%s\"\n", OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    // match font to e.g. resolve "Sans"
    psp::PrintFontManager::get().matchFont( aInfo, rLocale );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "font match %s, name AFTER: \"%s\"\n",
             aInfo.m_nID != 0 ? "succeeded" : "failed",
             OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif

    // font height
    int nPointHeight = qFontInfo.pointSize();
    if ( nPointHeight <= 0 )
        nPointHeight = rQFont.pointSize();

    // Create the font
    Font aFont( aInfo.m_aFamilyName, Size( 0, nPointHeight ) );
    if( aInfo.m_eWeight != psp::weight::Unknown )
        aFont.SetWeight( PspGraphics::ToFontWeight( aInfo.m_eWeight ) );
    if( aInfo.m_eWidth != psp::width::Unknown )
        aFont.SetWidthType( PspGraphics::ToFontWidth( aInfo.m_eWidth ) );
    if( aInfo.m_eItalic != psp::italic::Unknown )
        aFont.SetItalic( PspGraphics::ToFontItalic( aInfo.m_eItalic ) );
    if( aInfo.m_ePitch != psp::pitch::Unknown )
        aFont.SetPitch( PspGraphics::ToFontPitch( aInfo.m_ePitch ) );

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
            Font aFont = toFont( aGroup.readEntry( pKey, QFont() ), rSettings.GetUILocale() );
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
            Font aFont = toFont( aGroup.readEntry( pKey, QFont() ), rSettings.GetUILocale() );
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
    style.SetHelpTextColor( aText );

    // Base
    style.SetFieldColor( aBase );
    style.SetHelpColor( aBase );
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

    if( aBack == COL_LIGHTGRAY )
        style.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
    else
    {
        Color aColor2 = style.GetLightColor();
        style.
            SetCheckedColor( Color( (sal_uInt8)(((sal_uInt16)aBack.GetRed()+(sal_uInt16)aColor2.GetRed())/2),
                        (sal_uInt8)(((sal_uInt16)aBack.GetGreen()+(sal_uInt16)aColor2.GetGreen())/2),
                        (sal_uInt8)(((sal_uInt16)aBack.GetBlue()+(sal_uInt16)aColor2.GetBlue())/2)
                        ) );
    }

    // Selection
    style.SetHighlightColor( aHigh );
    style.SetHighlightTextColor( toColor(pal.color( QPalette::HighlightedText))  );

    // Font
    Font aFont = toFont( kapp->font(), rSettings.GetUILocale() );

    style.SetAppFont( aFont );
    style.SetHelpFont( aFont );

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
        Color aMenuFore = toColor( qMenuCG.color( QPalette::WindowText ) );
        Color aMenuBack = toColor( qMenuCG.color( QPalette::Window ) );

        aMenuFore = toColor( qMenuCG.color( QPalette::ButtonText ) );
        aMenuBack = toColor( qMenuCG.color( QPalette::Button ) );

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

        // Font
        aFont = toFont( pMenuBar->font(), rSettings.GetUILocale() );
        style.SetMenuFont( aFont );
    }

    delete pMenuBar;

    // Scroll bar size
    style.SetScrollBarSize( kapp->style()->pixelMetric( QStyle::PM_ScrollBarExtent ) );

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
