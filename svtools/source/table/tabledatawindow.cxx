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
#include "tablegeometry.hxx"
#include "cellvalueconversion.hxx"

#include <vcl/help.hxx>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Any;
    /** === end UNO using === **/

    //====================================================================
    //= TableDataWindow
    //====================================================================
    //--------------------------------------------------------------------
    TableDataWindow::TableDataWindow( TableControl_Impl& _rTableControl )
        :Window( &_rTableControl.getAntiImpl() )
        ,m_rTableControl        ( _rTableControl )
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
    void TableDataWindow::SetBackground( const Wallpaper& rColor )
    {
        Window::SetBackground( rColor );
    }
    //--------------------------------------------------------------------
    void TableDataWindow::SetControlBackground( const Color& rColor )
    {
        Window::SetControlBackground( rColor );
    }
    //--------------------------------------------------------------------
    void TableDataWindow::SetBackground()
    {
        Window::SetBackground();
    }
    //--------------------------------------------------------------------
    void TableDataWindow::SetControlBackground()
    {
        Window::SetControlBackground();
    }

    //--------------------------------------------------------------------
    void TableDataWindow::RequestHelp( const HelpEvent& rHEvt )
    {
        USHORT const nHelpMode = rHEvt.GetMode();
        if ( ( nHelpMode & HELPMODE_QUICK ) != 0 )
        {
            Point const aMousePos( m_rTableControl.getAntiImpl().ScreenToOutputPixel( rHEvt.GetMousePosPixel() ) );

            RowPos const hitRow = m_rTableControl.getRowAtPoint( aMousePos );
            ColPos const hitCol = m_rTableControl.getColAtPoint( aMousePos );
            if  (   ( hitRow >= 0 ) && ( hitRow < m_rTableControl.getRowCount() )
                &&  ( hitCol >= 0 ) && ( hitCol < m_rTableControl.getColumnCount() )
                )
            {
                PTableModel const pTableModel( m_rTableControl.getAntiImpl().GetModel() );

                Any aCellToolTip;
                pTableModel->getCellToolTip( hitCol, hitRow, aCellToolTip );
                if ( !aCellToolTip.hasValue() )
                {
                    // use the cell content
                    pTableModel->getCellContent( hitCol, hitRow, aCellToolTip );
                    // TODO: use the cell content as tool tip only if it doesn't fit into the cell. Need to
                    // ask the renderer for this.
                }

                ::rtl::OUString const sHelpText( CellValueConversion::convertToString( aCellToolTip ) );
                if ( sHelpText.getLength() > 0 )
                {
                    Rectangle const aControlScreenRect(
                        m_rTableControl.getAntiImpl().OutputToScreenPixel( Point( 0, 0 ) ),
                        m_rTableControl.getAntiImpl().GetOutputSizePixel()
                    );
                    Help::ShowQuickHelp( &m_rTableControl.getAntiImpl(), aControlScreenRect, sHelpText );
                }
            }
        }
        else
        {
            Window::RequestHelp( rHEvt );
        }
    }

    //--------------------------------------------------------------------
    void TableDataWindow::MouseMove( const MouseEvent& rMEvt )
    {
        Point aPoint = rMEvt.GetPosPixel();
        if ( !m_rTableControl.getInputHandler()->MouseMove( m_rTableControl, rMEvt ) )
        {
            if ( m_rTableControl.getRowAtPoint( aPoint ) == ROW_COL_HEADERS )
            {
                m_rTableControl.resizeColumn( aPoint );
            }
            else
            {
                Window::MouseMove( rMEvt );
            }
        }
    }
    //--------------------------------------------------------------------
    void TableDataWindow::MouseButtonDown( const MouseEvent& rMEvt )
    {
        const Point aPoint = rMEvt.GetPosPixel();
        const RowPos nCurRow = m_rTableControl.getRowAtPoint( aPoint );
        std::vector<RowPos> selectedRows(m_rTableControl.getSelectedRows());
        if ( !m_rTableControl.getInputHandler()->MouseButtonDown( m_rTableControl, rMEvt ) )
            Window::MouseButtonDown( rMEvt );
        else
        {
            if(nCurRow >= 0 && m_rTableControl.getSelEngine()->GetSelectionMode() != NO_SELECTION)
            {
                bool found = std::find(selectedRows.begin(),selectedRows.end(), nCurRow) != selectedRows.end();

                if( !found )
                {
                    m_aSelectHdl.Call( NULL );
                }
            }
        }
        m_aMouseButtonDownHdl.Call((MouseEvent*) &rMEvt);
    }
    //--------------------------------------------------------------------
    void TableDataWindow::MouseButtonUp( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseButtonUp( m_rTableControl, rMEvt ) )
            Window::MouseButtonUp( rMEvt );
        m_aMouseButtonUpHdl.Call((MouseEvent*) &rMEvt);
        m_rTableControl.getAntiImpl().GrabFocus();
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
