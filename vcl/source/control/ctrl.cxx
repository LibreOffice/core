/*************************************************************************
 *
 *  $RCSfile: ctrl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 16:53:26 $
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

#define _SV_CTRL_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_CTRL_HXX
#include <ctrl.hxx>
#endif
#ifndef _VCL_CONTROLLAYOUT_HXX
#include <controllayout.hxx>
#endif

#pragma hdrstop

using namespace vcl;

// =======================================================================

void Control::ImplInitData()
{
    mbHasFocus      = FALSE;
    mpLayoutData    = NULL;
}

// -----------------------------------------------------------------------

Control::Control( WindowType nType ) :
    Window( nType )
{
    ImplInitData();
}

// -----------------------------------------------------------------------

Control::Control( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_CONTROL )
{
    ImplInitData();
    Window::ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

Control::Control( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_CONTROL )
{
    ImplInitData();
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
    return (nIndex >= 0 && nIndex < m_aUnicodeBoundRects.size()) ? m_aUnicodeBoundRects[ nIndex ] : Rectangle();
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
    long nIndex = -1;
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
        if( nLine+1 < m_aLineIndices.size() )
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
            ImplDelData aDelData;
            ImplAddDel( &aDelData );
            ImplCallEventListeners( VCLEVENT_CONTROL_GETFOCUS );
            if ( aDelData.IsDelete() )
                return TRUE;
            maGetFocusHdl.Call( this );
            if ( aDelData.IsDelete() )
                return TRUE;
            ImplRemoveDel( &aDelData );
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
                ImplDelData aDelData;
                ImplAddDel( &aDelData );
                ImplCallEventListeners( VCLEVENT_CONTROL_LOSEFOCUS );
                if ( aDelData.IsDelete() )
                    return TRUE;
                maLoseFocusHdl.Call( this );
                if ( aDelData.IsDelete() )
                    return TRUE;
                ImplRemoveDel( &aDelData );
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

void Control::SetLayoutDataParent( const Control* pParent ) const
{
    if( mpLayoutData )
        mpLayoutData->m_pParent = pParent;
}

// -----------------------------------------------------------------

void Control::ImplSubControlLayoutChanged() const
{
    delete mpLayoutData, mpLayoutData = NULL;
}

// -----------------------------------------------------------------

ControlLayoutData::~ControlLayoutData()
{
    if( m_pParent )
        m_pParent->ImplSubControlLayoutChanged();
}
