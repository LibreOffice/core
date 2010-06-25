/*************************************************************************
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

#include "svtools/table/tablecontrol.hxx"
#include "svtools/table/tabledatawindow.hxx"
#include "tablecontrol_impl.hxx"
#include <vcl/help.hxx>

//........................................................................
namespace svt { namespace table
{
     class TableControl_Impl;
//........................................................................

    //====================================================================
    //= TableDataWindow
    //====================================================================
    //--------------------------------------------------------------------
    TableDataWindow::TableDataWindow( TableControl_Impl& _rTableControl )
        :Window( &_rTableControl.getAntiImpl() )
        ,m_rTableControl        ( _rTableControl )
        ,m_nRowAlreadySelected( -1 )
    {
        // by default, use the background as determined by the style settings
        const Color aWindowColor( GetSettings().GetStyleSettings().GetFieldColor() );
        SetBackground( Wallpaper( aWindowColor ) );
        SetFillColor( aWindowColor );
    }

    //--------------------------------------------------------------------
    void TableDataWindow::Paint( const Rectangle& rUpdateRect )
    {
        m_rTableControl.doPaintContent( rUpdateRect );
    }

    //--------------------------------------------------------------------
    void TableDataWindow::MouseMove( const MouseEvent& rMEvt )
    {
        Point aPoint = rMEvt.GetPosPixel();
        if ( !m_rTableControl.getInputHandler()->MouseMove( m_rTableControl, rMEvt ) )
        {
            if(m_rTableControl.getCurrentRow(aPoint)>=0 )
            {
                SetPointer(POINTER_ARROW);
                rtl::OUString& rHelpText = m_rTableControl.setTooltip(aPoint);
                Help::EnableBalloonHelp();
                Window::SetHelpText( rHelpText.getStr());
            }
            else if(m_rTableControl.getCurrentRow(aPoint) == -1)
            {
                if(Help::IsBalloonHelpEnabled())
                    Help::DisableBalloonHelp();
                m_rTableControl.resizeColumn(aPoint);
            }
            else
            {
                if(Help::IsBalloonHelpEnabled())
                    Help::DisableBalloonHelp();
                Window::MouseMove( rMEvt );
            }
        }
    }
    //--------------------------------------------------------------------
    void TableDataWindow::MouseButtonDown( const MouseEvent& rMEvt )
    {
        Point aPoint = rMEvt.GetPosPixel();
        RowPos nCurRow = m_rTableControl.getCurrentRow(aPoint);
        if ( !m_rTableControl.getInputHandler()->MouseButtonDown( m_rTableControl, rMEvt ) )
            Window::MouseButtonDown( rMEvt );
        else
        {
            if(nCurRow >= 0 && m_rTableControl.getSelEngine()->GetSelectionMode() != NO_SELECTION)
            {
                if( m_nRowAlreadySelected != nCurRow )
                {
                    m_nRowAlreadySelected = nCurRow;
                    m_aSelectHdl.Call( NULL );
                }
                else
                    m_aMouseButtonDownHdl.Call((MouseEvent*) &rMEvt);
            }
            else
                m_aMouseButtonDownHdl.Call((MouseEvent*) &rMEvt);
        }
        m_rTableControl.getAntiImpl().LoseFocus();
    }
    //--------------------------------------------------------------------
    void TableDataWindow::MouseButtonUp( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseButtonUp( m_rTableControl, rMEvt ) )
            Window::MouseButtonUp( rMEvt );
        else
            m_aMouseButtonUpHdl.Call((MouseEvent*) &rMEvt);
        m_rTableControl.getAntiImpl().GetFocus();
    }
    //--------------------------------------------------------------------
    void TableDataWindow::SetPointer( const Pointer& rPointer )
    {
        Window::SetPointer(rPointer);
    }
    //--------------------------------------------------------------------
    void TableDataWindow::CaptureMouse()
    {
        Window::CaptureMouse();
    }
    //--------------------------------------------------------------------
    void TableDataWindow::ReleaseMouse(  )
    {
        Window::ReleaseMouse();
    }
    // -----------------------------------------------------------------------
    long TableDataWindow::Notify(NotifyEvent& rNEvt )
    {
        long nDone = 0;
        if ( rNEvt.GetType() == EVENT_COMMAND )
        {
            const CommandEvent& rCEvt = *rNEvt.GetCommandEvent();
            if ( rCEvt.GetCommand() == COMMAND_WHEEL )
            {
                const CommandWheelData* pData = rCEvt.GetWheelData();
                if( !pData->GetModifier() && ( pData->GetMode() == COMMAND_WHEEL_SCROLL ) )
                {
                    nDone = HandleScrollCommand( rCEvt, m_rTableControl.getHorzScrollbar(), m_rTableControl.getVertScrollbar() );
                }
            }
        }
        return nDone ? nDone : Window::Notify( rNEvt );
    }
//........................................................................
} } // namespace svt::table
//........................................................................
