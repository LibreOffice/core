/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macosxint.cxx,v $
 * $Revision: 1.5 $
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

#include <macosxint.hxx>
#include <tools/config.hxx>
#include <vcl/settings.hxx>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <unistd.h>
#include <cstdio>

using namespace rtl;
using namespace osl;

MACOSXIntegrator::MACOSXIntegrator()
{
    meType = DtMACOSX;
}

MACOSXIntegrator::~MACOSXIntegrator()
{
}

void MACOSXIntegrator::GetSystemLook( AllSettings& rSettings )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();

    StyleSettings aStyleSettings( rSettings.GetStyleSettings() );
    // #i48001# set a default blink rate
    aStyleSettings.SetCursorBlinkTime( 500 );

// #i61174# aquacolors
// aUserConfigFile : string containing the user install directory completed with "/user/macosxrc.txt"
// currently : ~/Library/Application Support/OpenOffice.org 2.0/user/macosxrc.txt
// aDefaultConfigFile : string containing the OpenOffice.org install directory + presets/macosxrc.txt
// default should be /Applications/OpenOffice.org 2.0/Contents/openoffice.org2/presets/macosxrc.txt


    rtl::OUString aUserConfigFile;
    rtl::OUString aDefaultConfigFile;
    rtl::OUString aTryFiles[2];

// read the content of bootstraprc is necessary to find the path to the user configuration file
// ~/Library/Application Support/OpenOffice.org 2.0/user/macosxrc.txt

    rtl::Bootstrap aBootstrap( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("bootstraprc") ) );
    if( aBootstrap.getFrom( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("MacOSXIntegrationUserFile") ), aUserConfigFile ) )
    {
        rtl::OUString aFile = aUserConfigFile ;
        osl::FileBase::getSystemPathFromFileURL(aFile, aTryFiles[0]);
    }

// if macosxrc.txt is not found in user install dir, fallback to the second macosxrc.txt (with default values), located in <install_dir>/presets

    if( aBootstrap.getFrom( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("MacOSXIntegrationDefaultFile") ), aDefaultConfigFile ) )
    {
    rtl::OUString aFile = aDefaultConfigFile ;
    osl::FileBase::getSystemPathFromFileURL(aFile, aTryFiles[1]);
    }

    for( unsigned int i = 0; (i < sizeof(aTryFiles) / sizeof(aTryFiles[0])); i++ )
    {

        #if OSL_DEBUG_LEVEL > 1
        fprintf(stderr, "try accessing %d, %s\n", i, rtl::OUStringToOString( aTryFiles[i], aEncoding ).getStr());
    #endif
        if( access( rtl::OUStringToOString( aTryFiles[i], aEncoding ).getStr(), R_OK ) )
            continue;

        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "using %s for style settings\n", rtl::OUStringToOString( aTryFiles[i], aEncoding ).getStr() );
        #endif

        Config aConfig( aTryFiles[i] );
        ByteString aLine;

        if( aConfig.HasGroup( "General" ) )
        {
            aConfig.SetGroup( "General" );

            aLine = aConfig.ReadKey( "foreground" );
            if( aLine.GetTokenCount( ',' ) >= 3 )
            {
                Color aFore( aLine.GetToken( 0, ',' ).ToInt32(),
                        aLine.GetToken( 1, ',' ).ToInt32(),
                        aLine.GetToken( 2, ',' ).ToInt32() );
                aStyleSettings.SetDialogTextColor( aFore );
                aStyleSettings.SetMenuTextColor( aFore );
                aStyleSettings.SetButtonTextColor( aFore );
                aStyleSettings.SetRadioCheckTextColor( aFore );
                aStyleSettings.SetGroupTextColor( aFore );
                aStyleSettings.SetLabelTextColor( aFore );
                aStyleSettings.SetInfoTextColor( aFore );
                aStyleSettings.SetFieldTextColor( aFore );
            }

            aLine = aConfig.ReadKey( "background" );
            if( aLine.GetTokenCount( ',' ) >= 3 )
            {
                Color aBack( aLine.GetToken( 0, ',' ).ToInt32(),
                        aLine.GetToken( 1, ',' ).ToInt32(),
                        aLine.GetToken( 2, ',' ).ToInt32() );
                aStyleSettings.Set3DColors( aBack );
                aStyleSettings.SetFaceColor( aBack );
                aStyleSettings.SetDialogColor( aBack );
                aStyleSettings.SetMenuColor( aBack );
                aStyleSettings.SetMenuBarColor( aBack );
                aStyleSettings.SetLightBorderColor( aBack );
                if( aBack == COL_LIGHTGRAY )
                            aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
                else
                {
                    Color aColor2 = aStyleSettings.GetLightColor();
                    aStyleSettings.SetCheckedColor(
                        Color( (BYTE)(((USHORT)aBack.GetRed()+(USHORT)aColor2.GetRed())/2),
                                (BYTE)(((USHORT)aBack.GetGreen()+(USHORT)aColor2.GetGreen())/2),
                                (BYTE)(((USHORT)aBack.GetBlue()+(USHORT)aColor2.GetBlue())/2)
                                ) );
                }
            }

            aLine = aConfig.ReadKey( "selectForeground" );
            if( aLine.GetTokenCount( ',' ) >= 3 )
            {
                Color aSelectFore( aLine.GetToken( 0, ',' ).ToInt32(),
                   aLine.GetToken( 1, ',' ).ToInt32(),
                   aLine.GetToken( 2, ',' ).ToInt32());

                aStyleSettings.SetHighlightTextColor( aSelectFore );
                aStyleSettings.SetMenuHighlightTextColor( aSelectFore );
            }
            aLine = aConfig.ReadKey( "selectBackground" );
            if( aLine.GetTokenCount( ',' ) >= 3 )
            {
                Color aSelectBack( aLine.GetToken( 0, ',' ).ToInt32(),
                    aLine.GetToken( 1, ',' ).ToInt32(),
                    aLine.GetToken( 2, ',' ).ToInt32() );

                aStyleSettings.SetHighlightColor( aSelectBack );
                aStyleSettings.SetMenuHighlightColor( aSelectBack );
            }
            aLine = aConfig.ReadKey( "activeForeground" );
            if( aLine.GetTokenCount( ',' ) >= 3 ) {
                Color aActiveFore( aLine.GetToken( 0, ',' ).ToInt32(),
                    aLine.GetToken( 1, ',' ).ToInt32(),
                    aLine.GetToken( 2, ',' ).ToInt32() );

                aStyleSettings.SetActiveTextColor( aActiveFore );
            }
            aLine = aConfig.ReadKey( "activeBackground" );
            if( aLine.GetTokenCount( ',' ) >= 3 ) {
                Color aActiveBack( aLine.GetToken( 0, ',' ).ToInt32(),
                    aLine.GetToken( 1, ',' ).ToInt32(),
                    aLine.GetToken( 2, ',' ).ToInt32() );

                aStyleSettings.SetActiveColor( aActiveBack );
                aStyleSettings.SetActiveColor2( aActiveBack );
            }
            aLine = aConfig.ReadKey( "deactiveForeground" );
            if( aLine.GetTokenCount( ',' ) >= 3 ) {
                Color aDeactiveFore( aLine.GetToken( 0, ',' ).ToInt32(),
                    aLine.GetToken( 1, ',' ).ToInt32(),
                    aLine.GetToken( 2, ',' ).ToInt32() );

                aStyleSettings.SetDeactiveTextColor( aDeactiveFore );
                aStyleSettings.SetDisableColor( aDeactiveFore );
            }
            aLine = aConfig.ReadKey( "deactiveBackground" );
            if( aLine.GetTokenCount( ',' ) >= 3 ) {
                Color aDeactiveBack( aLine.GetToken( 0, ',' ).ToInt32(),
                    aLine.GetToken( 1, ',' ).ToInt32(),
                    aLine.GetToken( 2, ',' ).ToInt32() );

                aStyleSettings.SetDeactiveColor( aDeactiveBack );
                aStyleSettings.SetDeactiveColor2( aDeactiveBack );
                aStyleSettings.SetDeactiveBorderColor( aDeactiveBack );
                aStyleSettings.SetActiveBorderColor( aDeactiveBack );
            }

            aLine = aConfig.ReadKey( "font" );
            if( aLine.Len() )
            {
                Font aFont = aStyleSettings.GetAppFont();
                String aFontName( aLine, RTL_TEXTENCODING_UTF8 );
                if( aFontName.GetTokenCount( ',' ) > 0 )
                    aFontName = aFontName.GetToken( 0, ',' );
                aFont.SetName( aFontName );

                aStyleSettings.SetAppFont( aFont );
                aStyleSettings.SetHelpFont( aFont );
                aStyleSettings.SetTitleFont( aFont );
                aStyleSettings.SetFloatTitleFont( aFont );
                aStyleSettings.SetMenuFont( aFont );
                aStyleSettings.SetToolFont( aFont );
                aStyleSettings.SetLabelFont( aFont );
                aStyleSettings.SetInfoFont( aFont );
                aStyleSettings.SetRadioCheckFont( aFont );
                aStyleSettings.SetPushButtonFont( aFont );
                aStyleSettings.SetFieldFont( aFont );
                aStyleSettings.SetIconFont( aFont );
                aStyleSettings.SetGroupFont( aFont );
            }

            aLine = aConfig.ReadKey( "cursorFlashTime" );
            if( aLine.Len() )
            {
                sal_Int32 nTime = aLine.ToInt32() / 2;
                if( nTime == 0 )
                    nTime = STYLE_CURSOR_NOBLINKTIME;
                aStyleSettings.SetCursorBlinkTime( nTime );
            }
        }

        break;
    }

    rSettings.SetStyleSettings( aStyleSettings );
}

