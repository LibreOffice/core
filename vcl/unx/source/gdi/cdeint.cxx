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
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <salunx.h>
#include <saldisp.hxx>
#include <cdeint.hxx>
#include <vcl/settings.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>

CDEIntegrator::CDEIntegrator()
{
    meType = DtCDE;
}

CDEIntegrator::~CDEIntegrator()
{
}

static int getHexDigit( const char c )
{
    if( c >= '0' && c <= '9' )
        return (int)(c-'0');
    else if( c >= 'a' && c <= 'f' )
        return (int)(c-'a'+10);
    else if( c >= 'A' && c <= 'F' )
        return (int)(c-'A'+10);
    return -1;
}


void CDEIntegrator::GetSystemLook( AllSettings& rSettings )
{
    static Color aColors[ 8 ];
    static sal_Bool bRead  = sal_False;
    static sal_Bool bValid = sal_False;

    if( ! bRead )
    {
        // get used palette from xrdb
        char **ppStringList = 0;
        int nStringCount;
        XTextProperty aTextProperty;
        aTextProperty.value = 0;

        static Atom nResMgrAtom = XInternAtom( mpDisplay, "RESOURCE_MANAGER", False );

        if( XGetTextProperty( mpDisplay,
                              RootWindow( mpDisplay, 0 ),
                              &aTextProperty,
                              nResMgrAtom )
            && aTextProperty.value
            && XTextPropertyToStringList( &aTextProperty, &ppStringList, &nStringCount )
            )
        {
            // format of ColorPalette resource:
            // *n*ColorPalette: palettefile

            ByteString aLines;
            int i;
            for( i=0; i < nStringCount; i++ )
                aLines += ppStringList[i];
            for( i = aLines.GetTokenCount( '\n' )-1; i >= 0; i-- )
            {
                ByteString aLine = aLines.GetToken( i, '\n' );
                int nIndex = aLine.Search( "ColorPalette" );
                if( nIndex != STRING_NOTFOUND )
                {
                    int nPos = nIndex;

                    nIndex+=12;
                    const char* pStr = aLine.GetBuffer() +nIndex;
                    while( *pStr && isspace( *pStr ) && *pStr != ':' )
                    {
                        pStr++;
                        nIndex++;
                    }
                    if( *pStr != ':' )
                        continue;
                    pStr++, nIndex++;
                    for( ; *pStr && isspace( *pStr ); pStr++, nIndex++ )
                        ;
                    if( ! *pStr )
                        continue;
                    int nIndex2 = nIndex;
                    for( ; *pStr && ! isspace( *pStr ); pStr++, nIndex2++ )
                        ;
                    ByteString aPaletteFile( aLine.Copy( nIndex, nIndex2 - nIndex ) );
                    // extract number before ColorPalette;
                    for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
                        ;
                    nPos--;
                    for( ; nPos >= 0 && aLine.GetChar( nPos ) != '*'; nPos-- )
                        ;
                    int nNumber = aLine.Copy( ++nPos ).ToInt32();

                    OSL_TRACE( "found palette %d in resource \"%s\"", nNumber, aLine.GetBuffer() );

                    // found no documentation what this number actually means;
                    // might be the screen number. 0 seems to be the right one
                    // in most cases.
                    if( nNumber )
                        continue;

                    OSL_TRACE( "Palette file is \"%s\".\n", aPaletteFile.GetBuffer() );

                    String aPath( aHomeDir );
                    aPath.AppendAscii( "/.dt/palettes/" );
                    aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );

                    SvFileStream aStream( aPath, STREAM_READ );
                    if( ! aStream.IsOpen() )
                    {
                        aPath = String::CreateFromAscii( "/usr/dt/palettes/" );
                        aPath += String( aPaletteFile, gsl_getSystemTextEncoding() );
                        aStream.Open( aPath, STREAM_READ );
                        if( ! aStream.IsOpen() )
                            continue;
                    }

                    ByteString aBuffer;
                    for( nIndex = 0; nIndex < 8; nIndex++ )
                    {
                        aStream.ReadLine( aBuffer );
                        // format is "#RRRRGGGGBBBB"

                        OSL_TRACE( "\t\"%s\".\n", aBuffer.GetBuffer() );

                        if( aBuffer.Len() )
                        {
                            const char* pArr = (const char*)aBuffer.GetBuffer()+1;
                            aColors[nIndex] = Color(
                                getHexDigit( pArr[1] )
                                | ( getHexDigit( pArr[0] ) << 4 ),
                                getHexDigit( pArr[5] )
                                | ( getHexDigit( pArr[4] ) << 4 ),
                                getHexDigit( pArr[9] )
                                | ( getHexDigit( pArr[8] ) << 4 )
                                );

                            OSL_TRACE( "\t\t%lx\n", aColors[nIndex].GetColor() );
                        }
                    }

                    bValid = sal_True;
                    break;
                }
            }
        }

        if( ppStringList )
            XFreeStringList( ppStringList );
        if( aTextProperty.value )
            XFree( aTextProperty.value );
    }


    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    // #i48001# set a default blink rate
    aStyleSettings.SetCursorBlinkTime( 500 );
    if (bValid)
    {
        aStyleSettings.SetActiveColor( aColors[0] );
        aStyleSettings.SetActiveColor2( aColors[0] );
        aStyleSettings.SetActiveBorderColor( aColors[0] );

        aStyleSettings.SetDeactiveColor( aColors[0] );
        aStyleSettings.SetDeactiveColor2( aColors[0] );
        aStyleSettings.SetDeactiveBorderColor( aColors[0] );

        Color aActive =
            aColors[ 0 ].GetBlue() < 128        ||
            aColors[ 0 ].GetGreen() < 128       ||
            aColors[ 0 ].GetRed() < 128
            ? Color( COL_WHITE ) : Color( COL_BLACK );
        Color aDeactive =
            aColors[ 1 ].GetBlue() < 128        ||
            aColors[ 1 ].GetGreen() < 128       ||
            aColors[ 1 ].GetRed() < 128
            ? Color( COL_WHITE ) : Color( COL_BLACK );
        aStyleSettings.SetActiveTextColor( aActive );
        aStyleSettings.SetDeactiveTextColor( aDeactive );

        aStyleSettings.SetDialogTextColor( aDeactive );
        aStyleSettings.SetMenuTextColor( aDeactive );
        aStyleSettings.SetMenuBarTextColor( aDeactive );
        aStyleSettings.SetButtonTextColor( aDeactive );
        aStyleSettings.SetRadioCheckTextColor( aDeactive );
        aStyleSettings.SetGroupTextColor( aDeactive );
        aStyleSettings.SetLabelTextColor( aDeactive );
        aStyleSettings.SetInfoTextColor( aDeactive );

        aStyleSettings.Set3DColors( aColors[1] );
        aStyleSettings.SetFaceColor( aColors[1] );
        aStyleSettings.SetDialogColor( aColors[1] );
        aStyleSettings.SetMenuColor( aColors[1] );
        aStyleSettings.SetMenuBarColor( aColors[1] );
        if ( aStyleSettings.GetFaceColor() == COL_LIGHTGRAY )
            aStyleSettings.SetCheckedColor( Color( 0xCC, 0xCC, 0xCC ) );
        else
        {
            // calculate Checked color
            Color   aColor2 = aStyleSettings.GetLightColor();
            BYTE    nRed    = (BYTE)(((USHORT)aColors[1].GetRed()   + (USHORT)aColor2.GetRed())/2);
            BYTE    nGreen  = (BYTE)(((USHORT)aColors[1].GetGreen() + (USHORT)aColor2.GetGreen())/2);
            BYTE    nBlue   = (BYTE)(((USHORT)aColors[1].GetBlue()  + (USHORT)aColor2.GetBlue())/2);
            aStyleSettings.SetCheckedColor( Color( nRed, nGreen, nBlue ) );
        }
    }
    rSettings.SetStyleSettings( aStyleSettings );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
