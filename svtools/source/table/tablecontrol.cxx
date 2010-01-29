/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtools/table/tablecontrol.hxx"
#include "tablegeometry.hxx"
#include "tablecontrol_impl.hxx"
#include "svtools/table/tabledatawindow.hxx"
//........................................................................
namespace svt { namespace table
{
//........................................................................

    //====================================================================
    //= TableControl
    //====================================================================
    //--------------------------------------------------------------------
    TableControl::TableControl( Window* _pParent, WinBits _nStyle )
        :Control( _pParent, _nStyle )
        ,m_pImpl( new TableControl_Impl( *this ) )
    {
        m_pImpl->getDataWindow()->SetMouseButtonDownHdl( LINK( this, TableControl, ImplMouseButtonDownHdl ) );
        m_pImpl->getDataWindow()->SetMouseButtonUpHdl( LINK( this, TableControl, ImplMouseButtonUpHdl ) );
    }

    //--------------------------------------------------------------------
    TableControl::~TableControl()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    void TableControl::GetFocus()
    {
        if ( !m_pImpl->getInputHandler()->GetFocus( *m_pImpl ) )
        {
            Control::GetFocus();
            GrabFocus();
        }
    }

    //--------------------------------------------------------------------
    void TableControl::LoseFocus()
    {
        if ( !m_pImpl->getInputHandler()->LoseFocus( *m_pImpl ) )
            Control::LoseFocus();
    }

    //--------------------------------------------------------------------
    void TableControl::KeyInput( const KeyEvent& rKEvt )
    {
        if ( !m_pImpl->getInputHandler()->KeyInput( *m_pImpl, rKEvt ) )
            Control::KeyInput( rKEvt );
    }
    //--------------------------------------------------------------------
    void TableControl::Resize()
    {
        Control::Resize();
        m_pImpl->onResize();
    }

    //--------------------------------------------------------------------
    void TableControl::SetModel( PTableModel _pModel )
    {
        m_pImpl->setModel( _pModel );
    }

    //--------------------------------------------------------------------
    PTableModel TableControl::GetModel() const
    {
        return m_pImpl->getModel();
    }

    //--------------------------------------------------------------------
    RowPos TableControl::GetTopRow() const
    {
        return m_pImpl->getTopRow();
    }

    //--------------------------------------------------------------------
    void TableControl::SetTopRow( RowPos _nRow )
    {
        // TODO
        (void)_nRow;
    }

    //--------------------------------------------------------------------
    RowPos TableControl::GetCurrentRow() const
    {
        return m_pImpl->getCurRow();
    }

    //--------------------------------------------------------------------
    ColPos TableControl::GetCurrentColumn() const
    {
        return m_pImpl->getCurColumn();
    }

    //--------------------------------------------------------------------
    bool TableControl::GoTo( ColPos _nColumn, RowPos _nRow )
    {
        return m_pImpl->goTo( _nColumn, _nRow );
    }
    //--------------------------------------------------------------------
    void TableControl::InvalidateDataWindow(RowPos _nRowStart, bool _bRemoved)
    {
        Rectangle _rRect;
        if(_bRemoved)
            return m_pImpl->invalidateRows(_nRowStart, _rRect);
        else
            return m_pImpl->invalidateRow(_nRowStart, _rRect);
    }
    //--------------------------------------------------------------------
    std::vector<RowPos> TableControl::getSelectedRows()
    {
        return m_pImpl->getSelectedRows();
    }
    //--------------------------------------------------------------------
    void TableControl::removeSelectedRow(RowPos _nRowPos)
    {
        m_pImpl->removeSelectedRow(_nRowPos);
    }
    //--------------------------------------------------------------------

    RowPos TableControl::GetCurrentRow(const Point& rPoint)
    {
        return m_pImpl->getCurrentRow( rPoint );
    }

    //--------------------------------------------------------------------

    IMPL_LINK( TableControl, ImplMouseButtonDownHdl, MouseEvent*, pData )
    {
        CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pData );
        return 1;
    }

    IMPL_LINK( TableControl, ImplMouseButtonUpHdl, MouseEvent*, pData )
    {
        CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, pData );
        return 1;
    }

    SelectionEngine* TableControl::getSelEngine()
    {
        return m_pImpl->getSelEngine();
    }

    TableDataWindow* TableControl::getDataWindow()
    {
        return m_pImpl->getDataWindow();
    }
//........................................................................
} } // namespace svt::table
//........................................................................
