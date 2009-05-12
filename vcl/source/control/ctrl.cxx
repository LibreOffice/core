/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ctrl.cxx,v $
 * $Revision: 1.25 $
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

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/decoview.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/salnativewidgets.hxx>



using namespace vcl;

// =======================================================================

void Control::ImplInitControlData()
{
    mbHasFocus      = FALSE;
    mpLayoutData    = NULL;
}

// -----------------------------------------------------------------------

Control::Control( WindowType nType ) :
    Window( nType )
{
    ImplInitControlData();
}

// -----------------------------------------------------------------------

Control::Control( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_CONTROL )
{
    ImplInitControlData();
    Window::ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Control::Control( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_CONTROL )
{
    ImplInitControlData();
    rResId.SetRT( RSC_CONTROL );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Control::~Control()
{
    delete mpLayoutData, mpLayoutData = NULL;
}

// -----------------------------------------------------------------------

void Control::GetFocus()
{
    Window::GetFocus();
}

// -----------------------------------------------------------------------

void Control::LoseFocus()
{
    Window::LoseFocus();
}

// -----------------------------------------------------------------------

void Control::Resize()
{
    delete mpLayoutData, mpLayoutData = NULL;
    Window::Resize();
}

// -----------------------------------------------------------------------

void Control::FillLayoutData() const
{
}

// -----------------------------------------------------------------------

void Control::SetText( const String& rStr )
{
    delete mpLayoutData;
    mpLayoutData = NULL;
    Window::SetText( rStr );
}

// -----------------------------------------------------------------------

Rectangle ControlLayoutData::GetCharacterBounds( long nIndex ) const
{
    return (nIndex >= 0 && nIndex < (long) m_aUnicodeBoundRects.size()) ? m_aUnicodeBoundRects[ nIndex ] : Rectangle();
}


// -----------------------------------------------------------------------

Rectangle Control::GetCharacterBounds( long nIndex ) const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->GetCharacterBounds( nIndex ) : Rectangle();
}

// -----------------------------------------------------------------------

long ControlLayoutData::GetIndexForPoint( const Point& rPoint ) const
{
    long nIndex = -1;
    for( long i = m_aUnicodeBoundRects.size()-1; i >= 0; i-- )
    {
        if( m_aUnicodeBoundRects[ i ].IsInside( rPoint ) )
        {
            nIndex = i;
            break;
        }
    }
    return nIndex;
}

// -----------------------------------------------------------------------

long Control::GetIndexForPoint( const Point& rPoint ) const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->GetIndexForPoint( rPoint ) : -1;
}

// -----------------------------------------------------------------------

long ControlLayoutData::GetLineCount() const
{
    long nLines = m_aLineIndices.size();
    if( nLines == 0 && m_aDisplayText.Len() )
        nLines = 1;
    return nLines;
}

// -----------------------------------------------------------------------

long Control::GetLineCount() const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->GetLineCount() : 0;
}

// -----------------------------------------------------------------------

Pair ControlLayoutData::GetLineStartEnd( long nLine ) const
{
    Pair aPair( -1, -1 );

    int nDisplayLines = m_aLineIndices.size();
    if( nLine >= 0 && nLine < nDisplayLines )
    {
        aPair.A() = m_aLineIndices[nLine];
        if( nLine+1 < nDisplayLines )
            aPair.B() = m_aLineIndices[nLine+1]-1;
        else
            aPair.B() = m_aDisplayText.Len()-1;
    }
    else if( nLine == 0 && nDisplayLines == 0 && m_aDisplayText.Len() )
    {
        // special case for single line controls so the implementations
        // in that case do not have to fill in the line indices
        aPair.A() = 0;
        aPair.B() = m_aDisplayText.Len()-1;
    }
    return aPair;
}

// -----------------------------------------------------------------------

Pair Control::GetLineStartEnd( long nLine ) const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->GetLineStartEnd( nLine ) : Pair( -1, -1 );
}

// -----------------------------------------------------------------------

long ControlLayoutData::ToRelativeLineIndex( long nIndex ) const
{
    // is the index sensible at all ?
    if( nIndex >= 0 && nIndex < m_aDisplayText.Len() )
    {
        int nDisplayLines = m_aLineIndices.size();
        // if only 1 line exists, then absolute and relative index are
        // identical -> do nothing
        if( nDisplayLines > 1 )
        {
            int nLine;
            for( nLine = nDisplayLines-1; nLine >= 0; nLine-- )
            {
                if( m_aLineIndices[nLine] <= nIndex )
                {
                    nIndex -= m_aLineIndices[nLine];
                    break;
                }
            }
            if( nLine < 0 )
            {
                DBG_ASSERT( nLine >= 0, "ToRelativeLineIndex failed" );
                nIndex = -1;
            }
        }
    }
    else
        nIndex = -1;

    return nIndex;
}

// -----------------------------------------------------------------------

long Control::ToRelativeLineIndex( long nIndex ) const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->ToRelativeLineIndex( nIndex ) : -1;
}

// -----------------------------------------------------------------------

String Control::GetDisplayText() const
{
    if( ! mpLayoutData )
        FillLayoutData();
    return mpLayoutData ? mpLayoutData->m_aDisplayText : GetText();
}

// -----------------------------------------------------------------------

long Control::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( !mbHasFocus )
        {
            mbHasFocus = TRUE;
            if ( ImplCallEventListenersAndHandler( VCLEVENT_CONTROL_GETFOCUS, maGetFocusHdl, this ) )
                // been destroyed within the handler
                return TRUE;
        }
    }
    else
    {
        if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            Window* pFocusWin = Application::GetFocusWindow();
            if ( !pFocusWin || !ImplIsWindowOrChild( pFocusWin ) )
            {
                mbHasFocus = FALSE;
                if ( ImplCallEventListenersAndHandler( VCLEVENT_CONTROL_LOSEFOCUS, maLoseFocusHdl, this ) )
                    // been destroyed within the handler
                    return TRUE;
            }
        }
    }

    return Window::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void Control::StateChanged( StateChangedType nStateChange )
{
    if( nStateChange == STATE_CHANGE_INITSHOW   ||
        nStateChange == STATE_CHANGE_VISIBLE    ||
        nStateChange == STATE_CHANGE_FORMAT     ||
        nStateChange == STATE_CHANGE_ZOOM       ||
        nStateChange == STATE_CHANGE_BORDER     ||
        nStateChange == STATE_CHANGE_CONTROLFONT
        )
    {
        delete mpLayoutData;
        mpLayoutData = NULL;
    }
    Window::StateChanged( nStateChange );
}

// -----------------------------------------------------------------------

void Control::AppendLayoutData( const Control& rSubControl ) const
{
    if( ! rSubControl.mpLayoutData )
        rSubControl.FillLayoutData();
    if( ! rSubControl.mpLayoutData || ! rSubControl.mpLayoutData->m_aDisplayText.Len() )
        return;

    long nCurrentIndex = mpLayoutData->m_aDisplayText.Len();
    mpLayoutData->m_aDisplayText.Append( rSubControl.mpLayoutData->m_aDisplayText );
    int nLines = rSubControl.mpLayoutData->m_aLineIndices.size();
    int n;
    mpLayoutData->m_aLineIndices.push_back( nCurrentIndex );
    for( n = 1; n < nLines; n++ )
        mpLayoutData->m_aLineIndices.push_back( rSubControl.mpLayoutData->m_aLineIndices[n] + nCurrentIndex );
    int nRectangles = rSubControl.mpLayoutData->m_aUnicodeBoundRects.size();
        Rectangle aRel = const_cast<Control&>(rSubControl).GetWindowExtentsRelative( const_cast<Control*>(this) );
    for( n = 0; n < nRectangles; n++ )
    {
        Rectangle aRect = rSubControl.mpLayoutData->m_aUnicodeBoundRects[n];
        aRect.Move( aRel.Left(), aRel.Top() );
        mpLayoutData->m_aUnicodeBoundRects.push_back( aRect );
    }
}

// -----------------------------------------------------------------

BOOL Control::ImplCallEventListenersAndHandler(  ULONG nEvent, const Link& rHandler, void* pCaller )
{
    ImplDelData aCheckDelete;
    ImplAddDel( &aCheckDelete );

    ImplCallEventListeners( nEvent );
    if ( !aCheckDelete.IsDelete() )
    {
        rHandler.Call( pCaller );

        if ( !aCheckDelete.IsDelete() )
        {
            ImplRemoveDel( &aCheckDelete );
            return FALSE;
        }
    }
    return TRUE;
}

// -----------------------------------------------------------------

void Control::SetLayoutDataParent( const Control* pParent ) const
{
    if( mpLayoutData )
        mpLayoutData->m_pParent = pParent;
}

// -----------------------------------------------------------------

void Control::ImplClearLayoutData() const
{
    delete mpLayoutData, mpLayoutData = NULL;
}

// -----------------------------------------------------------------------

void Control::ImplDrawFrame( OutputDevice* pDev, Rectangle& rRect )
{
    // use a deco view to draw the frame
    // However, since there happens a lot of magic there, we need to fake some (style) settings
    // on the device
    AllSettings aOriginalSettings( pDev->GetSettings() );

    AllSettings aNewSettings( aOriginalSettings );
    StyleSettings aStyle( aNewSettings.GetStyleSettings() );

    // The *only known* clients of the Draw methods of the various VCL-controls are form controls:
    // During print preview, and during printing, Draw is called. Thus, drawing always happens with a
    // mono (colored) border
    aStyle.SetOptions( aStyle.GetOptions() | STYLE_OPTION_MONO );
    aStyle.SetMonoColor( GetSettings().GetStyleSettings().GetMonoColor() );

    aNewSettings.SetStyleSettings( aStyle );
    // #i67023# do not call data changed listeners for this fake
    // since they may understandably invalidate on settings changed
    pDev->OutputDevice::SetSettings( aNewSettings );

    DecorationView aDecoView( pDev );
    rRect = aDecoView.DrawFrame( rRect, FRAME_DRAW_WINDOWBORDER );

    pDev->OutputDevice::SetSettings( aOriginalSettings );
}

// -----------------------------------------------------------------------

void Control::DataChanged( const DataChangedEvent& rDCEvt)
{
    // we don't want to loose some style settings for controls created with the
    // toolkit
    if ( IsCreatedWithToolkit() &&
         (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        AllSettings     aSettings = GetSettings();
        StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
        ULONG           nOldOptions = rDCEvt.GetOldSettings()->GetStyleSettings().GetOptions();
        ULONG           nNewOptions = aStyleSettings.GetOptions();

        if ( !(nNewOptions & STYLE_OPTION_MONO) && ( nOldOptions & STYLE_OPTION_MONO ) )
        {
            nNewOptions |= STYLE_OPTION_MONO;
            aStyleSettings.SetOptions( nNewOptions );
            aStyleSettings.SetMonoColor( rDCEvt.GetOldSettings()->GetStyleSettings().GetMonoColor() );
            aSettings.SetStyleSettings( aStyleSettings );
            SetSettings( aSettings );
        }
    }
}

// -----------------------------------------------------------------

ControlLayoutData::~ControlLayoutData()
{
    if( m_pParent )
        m_pParent->ImplClearLayoutData();
}

// -----------------------------------------------------------------

Size Control::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return Size( GetTextWidth( GetText() ) + 2 * 12,
                     GetTextHeight() + 2 * 6 );
    case WINDOWSIZE_PREFERRED:
        return GetOptimalSize( WINDOWSIZE_MINIMUM );
    case WINDOWSIZE_MAXIMUM:
    default:
        return Size( LONG_MAX, LONG_MAX );
    }
}
