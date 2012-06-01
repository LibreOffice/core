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

void Control::take_properties(Window &rOther)
{
    if (!GetParent())
    {
        ImplInitControlData();
        ImplInit(rOther.GetParent(), rOther.GetStyle(), NULL);
    }

    Window::take_properties(rOther);

    Control &rOtherControl = static_cast<Control&>(rOther);
    std::swap(mpControlData, rOtherControl.mpControlData);
    mbHasFocus = rOtherControl.mbHasFocus;
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

void Control::SetText( const String& rStr )
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
    if( nLines == 0 && m_aDisplayText.Len() )
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
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->GetLineStartEnd( nLine ) : Pair( -1, -1 );
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
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->ToRelativeLineIndex( nIndex ) : -1;
}

// -----------------------------------------------------------------------

String Control::GetDisplayText() const
{
    if( !HasLayoutData() )
        FillLayoutData();
    return mpControlData->mpLayoutData ? mpControlData->mpLayoutData->m_aDisplayText : GetText();
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
    if( !rSubControl.HasLayoutData() || !rSubControl.mpControlData->mpLayoutData->m_aDisplayText.Len() )
        return;

    long nCurrentIndex = mpControlData->mpLayoutData->m_aDisplayText.Len();
    mpControlData->mpLayoutData->m_aDisplayText.Append( rSubControl.mpControlData->mpLayoutData->m_aDisplayText );
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
        AllSettings     aSettings = GetSettings();
        StyleSettings   aStyleSettings = aSettings.GetStyleSettings();
        sal_uLong           nOldOptions = rDCEvt.GetOldSettings()->GetStyleSettings().GetOptions();
        sal_uLong           nNewOptions = aStyleSettings.GetOptions();

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

void Control::DrawControlText( OutputDevice& _rTargetDevice, Rectangle& _io_rRect, const XubString& _rStr,
    sal_uInt16 _nStyle, MetricVector* _pVector, String* _pDisplayText ) const
{
#ifdef FS_DEBUG
    if ( !_pVector )
    {
        static MetricVector aCharRects;
        static String sDisplayText;
        aCharRects.clear();
        sDisplayText = String();
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
