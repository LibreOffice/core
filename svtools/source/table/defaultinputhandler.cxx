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

#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablecontrolinterface.hxx"

#include "tabledatawindow.hxx"

#include <tools/debug.hxx>
#include <vcl/event.hxx>
#include <vcl/cursor.hxx>

//........................................................................
namespace svt { namespace table
{
//.......................................................................

    struct DefaultInputHandler_Impl
    {
    };

    //====================================================================
    //= DefaultInputHandler
    //====================================================================
    //--------------------------------------------------------------------
    DefaultInputHandler::DefaultInputHandler()
        :m_pImpl( new DefaultInputHandler_Impl )
        ,m_nResizingColumn( COL_INVALID )
    {
    }

    //--------------------------------------------------------------------
    DefaultInputHandler::~DefaultInputHandler()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseMove( ITableControl& i_control, const MouseEvent& _rMEvt )
    {
        Point const aPoint = _rMEvt.GetPosPixel();

        // resize test
        if ( m_nResizingColumn != COL_INVALID )
        {
            ::Size const tableSize = i_control.getTableSizePixel();

            // set proper pointer
            Pointer aNewPointer( POINTER_ARROW );
            ColumnMetrics const & columnMetrics( i_control.getColumnMetrics( m_nResizingColumn ) );
            if  (   ( aPoint.X() > tableSize.Width() )
                ||  ( aPoint.X() < columnMetrics.nStartPixel )
                )
            {
                aNewPointer = Pointer( POINTER_NOTALLOWED );
            }
            else
            {
                aNewPointer = Pointer( POINTER_HSPLIT );
            }
            i_control.setPointer( aNewPointer );

            // show tracking line
            i_control.hideTracking();
            i_control.showTracking(
                Rectangle(
                    Point( aPoint.X(), 0 ),
                    Size( 1, tableSize.Height() )
                ),
                SHOWTRACK_SPLIT | SHOWTRACK_WINDOW
            );
            return true;
        }

        // test for column divider, adjust the mouse pointer, if necessary
        Pointer aNewPointer( POINTER_ARROW );

        TableCell const tableCell = i_control.hitTest( aPoint );
        if ( ( tableCell.nRow == ROW_COL_HEADERS ) && ( tableCell.eArea == ColumnDivider ) )
        {
            aNewPointer = Pointer( POINTER_HSPLIT );
        }

        i_control.setPointer( aNewPointer );
        return true;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonDown( ITableControl& i_tableControl, const MouseEvent& _rMEvt )
    {
        bool bHandled = false;
        Point const aPoint = _rMEvt.GetPosPixel();
        TableCell const tableCell( i_tableControl.hitTest( aPoint ) );
        if ( tableCell.nRow == ROW_COL_HEADERS )
        {
            if  (   ( tableCell.nColumn != COL_INVALID )
                &&  ( tableCell.eArea == ColumnDivider )
                )
            {
                m_nResizingColumn = tableCell.nColumn;
                i_tableControl.captureMouse();
            }
            bHandled = true;
        }
        else if ( tableCell.nRow >= 0 )
        {
            bool bSetCursor = false;
            if ( i_tableControl.getSelEngine()->GetSelectionMode() == NO_SELECTION )
            {
                bSetCursor = true;
            }
            else
            {
                if ( !i_tableControl.isRowSelected( tableCell.nRow ) )
                {
                    bHandled = i_tableControl.getSelEngine()->SelMouseButtonDown( _rMEvt );
                }
                else
                {
                    bSetCursor = true;
                }
            }

            if ( bSetCursor )
            {
                i_tableControl.activateCellAt( aPoint );
                bHandled = true;
            }
        }
        return bHandled;
    }
    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonUp( ITableControl& i_tableControl, const MouseEvent& _rMEvt )
    {
        bool bHandled = false;
        Point const aPoint = _rMEvt.GetPosPixel();
        if ( m_nResizingColumn != COL_INVALID )
        {
            i_tableControl.hideTracking();
            PColumnModel const pColumn = i_tableControl.getModel()->getColumnModel( m_nResizingColumn );
            long const maxWidthLogical = pColumn->getMaxWidth();
            long const minWidthLogical = pColumn->getMinWidth();

            // new position of mouse
            long const requestedEnd = aPoint.X();

            // old position of right border
            long const oldEnd = i_tableControl.getColumnMetrics( m_nResizingColumn ).nEndPixel;

            // position of left border if cursor in the to-be-resized column
            long const columnStart = i_tableControl.getColumnMetrics( m_nResizingColumn ).nStartPixel;
            long const requestedWidth = requestedEnd - columnStart;
                // TODO: this is not correct, strictly: It assumes that the mouse was pressed exactly on the "end" pos,
                // but for a while now, we have relaxed this, and allow clicking a few pixels aside, too

            if ( requestedEnd >= columnStart )
            {
                long requestedWidthLogical = i_tableControl.pixelWidthToAppFont( requestedWidth );
                // respect column width limits
                if ( oldEnd > requestedEnd )
                {
                    // column has become smaller, check against minimum width
                    if ( ( minWidthLogical != 0 ) && ( requestedWidthLogical < minWidthLogical ) )
                        requestedWidthLogical = minWidthLogical;
                }
                else if ( oldEnd < requestedEnd )
                {
                    // column has become larger, check against max width
                    if ( ( maxWidthLogical != 0 ) && ( requestedWidthLogical >= maxWidthLogical ) )
                        requestedWidthLogical = maxWidthLogical;
                }
                pColumn->setWidth( requestedWidthLogical );
                i_tableControl.invalidate();
            }

            i_tableControl.setPointer( Pointer() );
            i_tableControl.releaseMouse();

            m_nResizingColumn = COL_INVALID;
            bHandled = true;
        }
        else if ( i_tableControl.getRowAtPoint( aPoint ) >= 0 )
        {
            if ( i_tableControl.getSelEngine()->GetSelectionMode() == NO_SELECTION )
            {
                bHandled = true;
            }
            else
            {
                bHandled = i_tableControl.getSelEngine()->SelMouseButtonUp( _rMEvt );
            }
        }
        return bHandled;
    }
    //--------------------------------------------------------------------
    bool DefaultInputHandler::KeyInput( ITableControl& _rControl, const KeyEvent& rKEvt )
    {
        bool bHandled = false;

        const KeyCode& rKeyCode = rKEvt.GetKeyCode();
        USHORT nKeyCode = rKeyCode.GetCode();

        struct _ActionMapEntry
        {
            USHORT              nKeyCode;
            USHORT              nKeyModifier;
            TableControlAction  eAction;
        }
        static aKnownActions[] = {
            { KEY_DOWN,     0,          cursorDown },
            { KEY_UP,       0,          cursorUp },
            { KEY_LEFT,     0,          cursorLeft },
            { KEY_RIGHT,    0,          cursorRight },
            { KEY_HOME,     0,          cursorToLineStart },
            { KEY_END,      0,          cursorToLineEnd },
            { KEY_PAGEUP,   0,          cursorPageUp },
            { KEY_PAGEDOWN, 0,          cursorPageDown },
            { KEY_PAGEUP,   KEY_MOD1,   cursorToFirstLine },
            { KEY_PAGEDOWN, KEY_MOD1,   cursorToLastLine },
            { KEY_HOME,     KEY_MOD1,   cursorTopLeft },
            { KEY_END,      KEY_MOD1,   cursorBottomRight },
            { KEY_SPACE,    KEY_MOD1,   cursorSelectRow },
            { KEY_UP,       KEY_SHIFT,  cursorSelectRowUp },
            { KEY_DOWN,     KEY_SHIFT,  cursorSelectRowDown },
            { KEY_END,      KEY_SHIFT,  cursorSelectRowAreaBottom },
            { KEY_HOME,     KEY_SHIFT,  cursorSelectRowAreaTop },

            { 0, 0, invalidTableControlAction }
        };

        const _ActionMapEntry* pActions = aKnownActions;
        for ( ; pActions->eAction != invalidTableControlAction; ++pActions )
        {
            if ( ( pActions->nKeyCode == nKeyCode ) && ( pActions->nKeyModifier == rKeyCode.GetAllModifier() ) )
            {
                bHandled = _rControl.dispatchAction( pActions->eAction );
                break;
            }
        }

        return bHandled;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::GetFocus( ITableControl& _rControl )
    {
        _rControl.showCursor();
        return false;   // continue processing
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::LoseFocus( ITableControl& _rControl )
    {
        _rControl.hideCursor();
        return false;   // continue processing
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::RequestHelp( ITableControl& _rControl, const HelpEvent& _rHEvt )
    {
        (void)_rControl;
        (void)_rHEvt;
        // TODO
        return false;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::Command( ITableControl& _rControl, const CommandEvent& _rCEvt )
    {
        (void)_rControl;
        (void)_rCEvt;
        // TODO
        return false;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::PreNotify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::Notify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }
//........................................................................
} } // namespace svt::table
//........................................................................
