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
#include "svtools/table/tabledatawindow.hxx"
#include "tablecontrol_impl.hxx"

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
    {
        Color backgroundColor = m_rTableControl.getAntiImpl().GetSettings().GetStyleSettings().GetFieldColor();
        SetBackground( backgroundColor );
        SetControlBackground( backgroundColor );
    }

    //--------------------------------------------------------------------
    void TableDataWindow::Paint( const Rectangle& rUpdateRect )
    {
        m_rTableControl.doPaintContent( rUpdateRect );
    }
    void TableDataWindow::MouseMove( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseMove( m_rTableControl, rMEvt ) )
            Window::MouseMove( rMEvt );
    }
    void TableDataWindow::MouseButtonDown( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseButtonDown( m_rTableControl, rMEvt ) )
            Window::MouseButtonDown( rMEvt );
        else
            m_aMouseButtonDownHdl.Call( (MouseEvent*) &rMEvt);
        m_rTableControl.getAntiImpl().LoseFocus();
    }
    void TableDataWindow::MouseButtonUp( const MouseEvent& rMEvt )
    {
        if ( !m_rTableControl.getInputHandler()->MouseButtonUp( m_rTableControl, rMEvt ) )
            Window::MouseButtonUp( rMEvt );
        else
            m_aMouseButtonUpHdl.Call( (MouseEvent*) &rMEvt);
        m_rTableControl.getAntiImpl().GetFocus();
    }

//........................................................................
} } // namespace svt::table
//........................................................................
