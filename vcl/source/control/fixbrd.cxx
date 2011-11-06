/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/event.hxx>
#include <vcl/fixbrd.hxx>



// =======================================================================

void FixedBorder::ImplInit( Window* pParent, WinBits nStyle )
{
    mnType          = FIXEDBORDER_TYPE_DOUBLEOUT;
    mbTransparent   = sal_True;

    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedBorder::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedBorder::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( (pParent->IsChildTransparentModeEnabled() ||
          !(pParent->GetStyle() & WB_CLIPCHILDREN) ) &&
         !IsControlBackground() && mbTransparent )
    {
        SetMouseTransparent( sal_True );
        EnableChildTransparentMode( sal_True );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( sal_True );
        SetBackground();
    }
    else
    {
        SetMouseTransparent( sal_False );
        EnableChildTransparentMode( sal_False );
        SetParentClipMode( 0 );
        SetPaintTransparent( sal_False );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

FixedBorder::FixedBorder( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDBORDER )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedBorder::FixedBorder( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDBORDER )
{
    rResId.SetRT( RSC_CONTROL );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

FixedBorder::~FixedBorder()
{
}

// -----------------------------------------------------------------------

void FixedBorder::ImplDraw( OutputDevice* pDev, sal_uLong nDrawFlags,
                            const Point& rPos, const Size& rSize )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle               aRect( rPos, rSize );
    sal_uInt16                  nBorderStyle = mnType;

    if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nBorderStyle |= FRAME_DRAW_MONO;

    /*
    // seems only to be used in tools->options around a tabpage (ie, no tabcontrol!)
    // as tabpages that are not embedded in a tabcontrol should not be drawn natively
    // the fixedborder must also not be drawn (reason was, that it looks too ugly, dialogs must be redesigned)
    Window *pWin = pDev->GetOutDevType() == OUTDEV_WINDOW ? (Window*) pDev : NULL;
    if( !(nBorderStyle & FRAME_DRAW_MONO) && pWin && pWin->IsNativeControlSupported( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aControlValue;
        Point aPt;
        Region aCtrlRegion( Rectangle( aPt, GetOutputSizePixel() ) );
        ControlState nState = IsEnabled() ? CTRL_STATE_ENABLED : 0;
        pWin->DrawNativeControl( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                                 aControlValue, rtl::OUString() );
    }
    else
    */
    {
        DecorationView  aDecoView( pDev );
        aDecoView.DrawFrame( aRect, nBorderStyle );
    }
}

// -----------------------------------------------------------------------

void FixedBorder::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedBorder::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        sal_uLong nFlags )
{
    Point   aPos  = pDev->LogicToPixel( rPos );
    Size    aSize = pDev->LogicToPixel( rSize );

    pDev->Push();
    pDev->SetMapMode();
    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedBorder::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedBorder::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_DATA) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
        SetStyle( ImplInitStyle( GetStyle() ) );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::SetTransparent( sal_Bool bTransparent )
{
    if ( mbTransparent != bTransparent )
    {
        mbTransparent = bTransparent;
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::SetBorderType( sal_uInt16 nType )
{
    if ( mnType != nType )
    {
        mnType = nType;
        Invalidate();
    }
}
