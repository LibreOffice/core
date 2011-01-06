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
#include "precompiled_svtools.hxx"

#define _SV_PRGSBAR_CXX

#include <tools/debug.hxx>
#include <vcl/status.hxx>
#include <svtools/prgsbar.hxx>

// =======================================================================

#define PROGRESSBAR_OFFSET          3
#define PROGRESSBAR_WIN_OFFSET      2

// =======================================================================

void ProgressBar::ImplInit()
{
    mnPercent   = 0;
    mbCalcNew   = sal_True;

    ImplInitSettings( sal_True, sal_True, sal_True );
}

static WinBits clearProgressBarBorder( Window* pParent, WinBits nOrgStyle )
{
    WinBits nOutStyle = nOrgStyle;
    if( pParent && (nOrgStyle & WB_BORDER) != 0 )
    {
        if( pParent->IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
            nOutStyle &= WB_BORDER;
    }
    return nOutStyle;
}

// -----------------------------------------------------------------------

ProgressBar::ProgressBar( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, clearProgressBarBorder( pParent, nWinStyle ) )
{
    SetOutputSizePixel( Size( 150, 20 ) );
    ImplInit();
}

// -----------------------------------------------------------------------

ProgressBar::ProgressBar( Window* pParent, const ResId& rResId ) :
    Window( pParent, rResId )
{
    ImplInit();
}

// -----------------------------------------------------------------------

ProgressBar::~ProgressBar()
{
}

// -----------------------------------------------------------------------

void ProgressBar::ImplInitSettings( sal_Bool bFont,
                                    sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

/* !!! Derzeit unterstuetzen wir keine Textausgaben
    if ( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetAppFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }
*/

    if ( bBackground )
    {
        if( !IsControlBackground() &&
            IsNativeControlSupported( CTRL_PROGRESS, PART_ENTIRE_CONTROL ) )
        {
            if( (GetStyle() & WB_BORDER) )
                SetBorderStyle( WINDOW_BORDER_REMOVEBORDER );
            EnableChildTransparentMode( sal_True );
            SetPaintTransparent( sal_True );
            SetBackground();
            SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        }
        else
        {
            Color aColor;
            if ( IsControlBackground() )
                aColor = GetControlBackground();
            else
                aColor = rStyleSettings.GetFaceColor();
            SetBackground( aColor );
        }
    }

    if ( bForeground || bFont )
    {
        Color aColor = rStyleSettings.GetHighlightColor();
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        if ( aColor.IsRGBEqual( GetBackground().GetColor() ) )
        {
            if ( aColor.GetLuminance() > 100 )
                aColor.DecreaseLuminance( 64 );
            else
                aColor.IncreaseLuminance( 64 );
        }
        SetLineColor();
        SetFillColor( aColor );
/* !!! Derzeit unterstuetzen wir keine Textausgaben
        SetTextColor( aColor );
        SetTextFillColor();
*/
    }
}

// -----------------------------------------------------------------------

void ProgressBar::ImplDrawProgress( sal_uInt16 nOldPerc, sal_uInt16 nNewPerc )
{
    if ( mbCalcNew )
    {
        mbCalcNew = sal_False;

        Size aSize = GetOutputSizePixel();
        mnPrgsHeight = aSize.Height()-(PROGRESSBAR_WIN_OFFSET*2);
        mnPrgsWidth = (mnPrgsHeight*2)/3;
        maPos.Y() = PROGRESSBAR_WIN_OFFSET;
        long nMaxWidth = (aSize.Width()-(PROGRESSBAR_WIN_OFFSET*2)+PROGRESSBAR_OFFSET);
        sal_uInt16 nMaxCount = (sal_uInt16)(nMaxWidth / (mnPrgsWidth+PROGRESSBAR_OFFSET));
        if ( nMaxCount <= 1 )
            nMaxCount = 1;
        else
        {
            while ( ((10000/(10000/nMaxCount))*(mnPrgsWidth+PROGRESSBAR_OFFSET)) > nMaxWidth )
                nMaxCount--;
        }
        mnPercentCount = 10000/nMaxCount;
        nMaxWidth = ((10000/(10000/nMaxCount))*(mnPrgsWidth+PROGRESSBAR_OFFSET))-PROGRESSBAR_OFFSET;
        maPos.X() = (aSize.Width()-nMaxWidth)/2;
    }

    ::DrawProgress( this, maPos, PROGRESSBAR_OFFSET, mnPrgsWidth, mnPrgsHeight,
                    nOldPerc*100, nNewPerc*100, mnPercentCount,
                    Rectangle( Point(), GetSizePixel() ) );
}

// -----------------------------------------------------------------------

void ProgressBar::Paint( const Rectangle& )
{
    ImplDrawProgress( 0, mnPercent );
}

// -----------------------------------------------------------------------

void ProgressBar::Resize()
{
    mbCalcNew = sal_True;
    if ( IsReallyVisible() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ProgressBar::SetValue( sal_uInt16 nNewPercent )
{
    DBG_ASSERTWARNING( nNewPercent <= 100, "StatusBar::SetProgressValue(): nPercent > 100" );

    if ( nNewPercent < mnPercent )
    {
        mbCalcNew = sal_True;
        mnPercent = nNewPercent;
        if ( IsReallyVisible() )
        {
            Invalidate();
            Update();
        }
    }
    else
    {
        ImplDrawProgress( mnPercent, nNewPercent );
        mnPercent = nNewPercent;
    }
}

// -----------------------------------------------------------------------

void ProgressBar::StateChanged( StateChangedType nType )
{
/* !!! Derzeit unterstuetzen wir keine Textausgaben
    if ( (nType == STATE_CHANGE_ZOOM) ||
         (nType == STATE_CHANGE_CONTROLFONT) )
    {
        ImplInitSettings( sal_True, sal_False, sal_False );
        Invalidate();
    }
    else
*/
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( sal_False, sal_True, sal_False );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( sal_False, sal_False, sal_True );
        Invalidate();
    }

    Window::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ProgressBar::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}

