/*************************************************************************
 *
 *  $RCSfile: prgsbar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
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

#define _SV_PRGSBAR_CXX

#ifndef _TOOLS_DEBUGS_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VCL_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <prgsbar.hxx>
#endif

// =======================================================================

#define PROGRESSBAR_OFFSET          3
#define PROGRESSBAR_WIN_OFFSET      2

// =======================================================================

void ProgressBar::ImplInit()
{
    mnPercent   = 0;
    mbCalcNew   = TRUE;

    ImplInitSettings( TRUE, TRUE, TRUE );
}

// -----------------------------------------------------------------------

ProgressBar::ProgressBar( Window* pParent, WinBits nWinStyle ) :
    Window( pParent, nWinStyle )
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

void ProgressBar::ImplInitSettings( BOOL bFont,
                                    BOOL bForeground, BOOL bBackground )
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
        Color aColor;
        if ( IsControlBackground() )
            aColor = GetControlBackground();
        else
            aColor = rStyleSettings.GetFaceColor();
        SetBackground( aColor );
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

void ProgressBar::ImplDrawProgress( USHORT nOldPerc, USHORT nNewPerc )
{
    if ( mbCalcNew )
    {
        mbCalcNew = FALSE;

        Size aSize = GetOutputSizePixel();
        mnPrgsHeight = aSize.Height()-(PROGRESSBAR_WIN_OFFSET*2);
        mnPrgsWidth = (mnPrgsHeight*2)/3;
        maPos.Y() = PROGRESSBAR_WIN_OFFSET;
        long nMaxWidth = (aSize.Width()-(PROGRESSBAR_WIN_OFFSET*2)+PROGRESSBAR_OFFSET);
        USHORT nMaxCount = (USHORT)(nMaxWidth / (mnPrgsWidth+PROGRESSBAR_OFFSET));
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
                    nOldPerc*100, nNewPerc*100, mnPercentCount );
}

// -----------------------------------------------------------------------

void ProgressBar::Paint( const Rectangle& )
{
    ImplDrawProgress( 0, mnPercent );
}

// -----------------------------------------------------------------------

void ProgressBar::Resize()
{
    mbCalcNew = TRUE;
    if ( IsReallyVisible() )
        Invalidate();
}

// -----------------------------------------------------------------------

void ProgressBar::SetValue( USHORT nNewPercent )
{
    DBG_ASSERTWARNING( nNewPercent <= 100, "StatusBar::SetProgressValue(): nPercent > 100" );

    if ( nNewPercent < mnPercent )
    {
        mbCalcNew = TRUE;
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
        ImplInitSettings( TRUE, FALSE, FALSE );
        Invalidate();
    }
    else
*/
    if ( nType == STATE_CHANGE_CONTROLFOREGROUND )
    {
        ImplInitSettings( FALSE, TRUE, FALSE );
        Invalidate();
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings( FALSE, FALSE, TRUE );
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
        ImplInitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }

    Window::DataChanged( rDCEvt );
}

