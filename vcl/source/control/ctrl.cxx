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


#include <comphelper/processfactory.hxx>

#include <tools/rc.h>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/decoview.hxx>
#include <vcl/salnativewidgets.hxx>

#include <textlayout.hxx>
#include <svdata.hxx>
#include <controldata.hxx>


using namespace vcl;

// =======================================================================

void Control::ImplInitControlData()
{
    mbHasControlFocus       = sal_False;
    mpControlData   = new ImplControlData;
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
    ImplInit( pParent, nStyle, NULL );
}

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
    delete mpControlData, mpControlData = NULL;
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
    ImplClearLayoutData();
    Window::Resize();
}

// -----------------------------------------------------------------------

void Control::FillLayoutData() const
{
}

// -----------------------------------------------------------------------

void Control::CreateLayoutData() const
{
    DBG_ASSERT( !mpControlData->mpLayoutData, "Control::CreateLayoutData: should be called with non-existent layout data only!" );
    mpControlData->mpLayoutData = new ::vcl::ControlLayoutData();
}

// -----------------------------------------------------------------------

bool Control::HasLayoutData() const
{
    return mpControlData->mpLayoutData != NULL;
}

// -----------------------------------------------------------------------

::vcl::ControlLayoutData* Control::GetLayoutData() const
{
    return mpControlData->mpLayoutData;
}

// -----------------------------------------------------------------------

void Control::SetText( const OUString& rStr )
{
    ImplClearLayoutData();
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
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->GetCharacterBounds( nIndex ) : Rectangle();
}

// -----------------------------------------------------------------------

long ControlLayoutData::GetIndexForPoint( const Point& rPoint ) const
{
    long nIndex = -1;
    for( long i = m_aUnicodeBoundRects.size()-1; i >= 0; i-- )
    {
        Point aTopLeft = m_aUnicodeBoundRects[i].TopLeft();
        Point aBottomRight = m_aUnicodeBoundRects[i].BottomRight();
        if (rPoint.X() >= aTopLeft.X() && rPoint.Y() >= aTopLeft.Y() &&
            rPoint.X() <= aBottomRight.X() && rPoint.Y() <= aBottomRight.Y())
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
    if( ! HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->GetIndexForPoint( rPoint ) : -1;
}

// -----------------------------------------------------------------------

long ControlLayoutData::GetLineCount() const
{
    long nLines = m_aLineIndices.size();
    if( nLines == 0 && !m_aDisplayText.isEmpty() )
        nLines = 1;
    return nLines;
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
            aPair.B() = m_aDisplayText.getLength()-1;
    }
    else if( nLine == 0 && nDisplayLines == 0 && !m_aDisplayText.isEmpty() )
    {
        // special case for single line controls so the implementations
        // in that case do not have to fill in the line indices
        aPair.A() = 0;
        aPair.B() = m_aDisplayText.getLength()-1;
    }
    return aPair;
}

// -----------------------------------------------------------------------

Pair Control::GetLineStartEnd( long nLine ) const
{
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->GetLineStartEnd( nLine ) : Pair( -1, -1 );
}

// -----------------------------------------------------------------------

long ControlLayoutData::ToRelativeLineIndex( long nIndex ) const
{
    // is the index sensible at all ?
    if( nIndex >= 0 && nIndex < m_aDisplayText.getLength() )
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
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->ToRelativeLineIndex( nIndex ) : -1;
}

// -----------------------------------------------------------------------

OUString Control::GetDisplayText() const
{
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? OUString(mpControlData->mpLayoutData->m_aDisplayText) : GetText();
}

// -----------------------------------------------------------------------

long Control::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( !mbHasControlFocus )
        {
            mbHasControlFocus = sal_True;
            StateChanged( STATE_CHANGE_CONTROL_FOCUS );
            if ( ImplCallEventListenersAndHandler( VCLEVENT_CONTROL_GETFOCUS, maGetFocusHdl, this ) )
                // been destroyed within the handler
                return sal_True;
        }
    }
    else
    {
        if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            Window* pFocusWin = Application::GetFocusWindow();
            if ( !pFocusWin || !ImplIsWindowOrChild( pFocusWin ) )
            {
                mbHasControlFocus = sal_False;
                StateChanged( STATE_CHANGE_CONTROL_FOCUS );
                if ( ImplCallEventListenersAndHandler( VCLEVENT_CONTROL_LOSEFOCUS, maLoseFocusHdl, this ) )
                    // been destroyed within the handler
                    return sal_True;
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
        nStateChange == STATE_CHANGE_ZOOM       ||
        nStateChange == STATE_CHANGE_BORDER     ||
        nStateChange == STATE_CHANGE_CONTROLFONT
        )
    {
        ImplClearLayoutData();
    }
    Window::StateChanged( nStateChange );
}

// -----------------------------------------------------------------------

void Control::AppendLayoutData( const Control& rSubControl ) const
{
    if( !rSubControl.HasLayoutData() )
        rSubControl.FillLayoutData();
    if( !rSubControl.HasLayoutData() || rSubControl.mpControlData->mpLayoutData->m_aDisplayText.isEmpty() )
        return;

    long nCurrentIndex = mpControlData->mpLayoutData->m_aDisplayText.getLength();
    mpControlData->mpLayoutData->m_aDisplayText += rSubControl.mpControlData->mpLayoutData->m_aDisplayText;
    int nLines = rSubControl.mpControlData->mpLayoutData->m_aLineIndices.size();
    int n;
    mpControlData->mpLayoutData->m_aLineIndices.push_back( nCurrentIndex );
    for( n = 1; n < nLines; n++ )
        mpControlData->mpLayoutData->m_aLineIndices.push_back( rSubControl.mpControlData->mpLayoutData->m_aLineIndices[n] + nCurrentIndex );
    int nRectangles = rSubControl.mpControlData->mpLayoutData->m_aUnicodeBoundRects.size();
        Rectangle aRel = const_cast<Control&>(rSubControl).GetWindowExtentsRelative( const_cast<Control*>(this) );
    for( n = 0; n < nRectangles; n++ )
    {
        Rectangle aRect = rSubControl.mpControlData->mpLayoutData->m_aUnicodeBoundRects[n];
        aRect.Move( aRel.Left(), aRel.Top() );
        mpControlData->mpLayoutData->m_aUnicodeBoundRects.push_back( aRect );
    }
}

// -----------------------------------------------------------------

sal_Bool Control::ImplCallEventListenersAndHandler(  sal_uLong nEvent, const Link& rHandler, void* pCaller )
{
    ImplDelData aCheckDelete;
    ImplAddDel( &aCheckDelete );

    ImplCallEventListeners( nEvent );
    if ( !aCheckDelete.IsDead() )
    {
        rHandler.Call( pCaller );

        if ( !aCheckDelete.IsDead() )
        {
            ImplRemoveDel( &aCheckDelete );
            return sal_False;
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------

void Control::SetLayoutDataParent( const Control* pParent ) const
{
    if( HasLayoutData() )
        mpControlData->mpLayoutData->m_pParent = pParent;
}

// -----------------------------------------------------------------

void Control::ImplClearLayoutData() const
{
    delete mpControlData->mpLayoutData, mpControlData->mpLayoutData = NULL;
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
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();

        AllSettings aSettings = GetSettings();
        StyleSettings aStyleSettings = aSettings.GetStyleSettings();
        sal_uLong nNewOptions = aStyleSettings.GetOptions();

        if ( pOldSettings && !(nNewOptions & STYLE_OPTION_MONO) && ( pOldSettings->GetStyleSettings().GetOptions() & STYLE_OPTION_MONO ) )
        {
            nNewOptions |= STYLE_OPTION_MONO;
            aStyleSettings.SetOptions( nNewOptions );
            aStyleSettings.SetMonoColor( pOldSettings->GetStyleSettings().GetMonoColor() );
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

Size Control::GetOptimalSize() const
{
    return Size( GetTextWidth( GetText() ) + 2 * 12,
                 GetTextHeight() + 2 * 6 );
}

// -----------------------------------------------------------------

void Control::SetReferenceDevice( OutputDevice* _referenceDevice )
{
    if ( mpControlData->mpReferenceDevice == _referenceDevice )
        return;

    mpControlData->mpReferenceDevice = _referenceDevice;
    Invalidate();
}

// -----------------------------------------------------------------

OutputDevice* Control::GetReferenceDevice() const
{
    return mpControlData->mpReferenceDevice;
}

// -----------------------------------------------------------------

const Font& Control::GetCanonicalFont( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetLabelFont();
}

// -----------------------------------------------------------------
const Color& Control::GetCanonicalTextColor( const StyleSettings& _rStyle ) const
{
    return _rStyle.GetLabelTextColor();
}

// -----------------------------------------------------------------
void Control::ImplInitSettings( const sal_Bool _bFont, const sal_Bool _bForeground )
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    if ( _bFont )
    {
        Font aFont( GetCanonicalFont( rStyleSettings ) );
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );
    }

    if ( _bForeground || _bFont )
    {
        Color aColor;
        if ( IsControlForeground() )
            aColor = GetControlForeground();
        else
            aColor = GetCanonicalTextColor( rStyleSettings );
        SetTextColor( aColor );
        SetTextFillColor();
    }
}

// -----------------------------------------------------------------

void Control::DrawControlText( OutputDevice& _rTargetDevice, Rectangle& _io_rRect, const OUString& _rStr,
    sal_uInt16 _nStyle, MetricVector* _pVector, OUString* _pDisplayText ) const
{
#ifdef FS_DEBUG
    if ( !_pVector )
    {
        static MetricVector aCharRects;
        static OUString sDisplayText;
        aCharRects.clear();
        sDisplayText = OUString();
        _pVector = &aCharRects;
        _pDisplayText = &sDisplayText;
    }
#endif

    if ( !mpControlData->mpReferenceDevice || ( mpControlData->mpReferenceDevice == &_rTargetDevice ) )
    {
        _io_rRect = _rTargetDevice.GetTextRect( _io_rRect, _rStr, _nStyle );
        _rTargetDevice.DrawText( _io_rRect, _rStr, _nStyle, _pVector, _pDisplayText );
    }
    else
    {
        ControlTextRenderer aRenderer( *this, _rTargetDevice, *mpControlData->mpReferenceDevice );
        _io_rRect = aRenderer.DrawText( _io_rRect, _rStr, _nStyle, _pVector, _pDisplayText );
    }

#ifdef FS_DEBUG
    _rTargetDevice.Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
    _rTargetDevice.SetLineColor( COL_LIGHTRED );
    _rTargetDevice.SetFillColor();
    for (   MetricVector::const_iterator cr = _pVector->begin();
            cr != _pVector->end();
            ++cr
        )
    {
        _rTargetDevice.DrawRect( *cr );
    }
    _rTargetDevice.Pop();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
