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
        ,m_bResize(false)
    {
    }

    //--------------------------------------------------------------------
    DefaultInputHandler::~DefaultInputHandler()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseMove( ITableControl& _rControl, const MouseEvent& _rMEvt )
    {
        Point aPoint = _rMEvt.GetPosPixel();
        if ( m_bResize )
        {
            _rControl.resizeColumn( aPoint );
            return true;
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonDown( ITableControl& _rControl, const MouseEvent& _rMEvt )
    {
        bool bHandled = false;
        Point aPoint = _rMEvt.GetPosPixel();
        RowPos nRow = _rControl.getRowAtPoint( aPoint );
        if ( nRow == ROW_COL_HEADERS )
        {
            m_bResize = _rControl.checkResizeColumn(aPoint);
            bHandled = true;
        }
        else if(nRow >= 0)
        {
            bool bSetCursor = false;
            if ( _rControl.getSelEngine()->GetSelectionMode() == NO_SELECTION )
            {
                bSetCursor = true;
            }
            else
            {
                if ( !_rControl.isRowSelected( nRow ) )
                {
                    bHandled = _rControl.getSelEngine()->SelMouseButtonDown( _rMEvt );
                }
                else
                {
                    bSetCursor = true;
                }
            }

            if ( bSetCursor )
            {
                _rControl.activateCellAt( aPoint );
                bHandled = true;
            }
        }
        return bHandled;
    }
    //--------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonUp( ITableControl& _rControl, const MouseEvent& _rMEvt )
    {
        bool bHandled = false;
        const Point aPoint = _rMEvt.GetPosPixel();

        if ( m_bResize )
        {
            m_bResize = _rControl.endResizeColumn( aPoint );
            bHandled = true;
        }
        else if ( _rControl.getRowAtPoint( aPoint ) >= 0 )
        {
            if ( _rControl.getSelEngine()->GetSelectionMode() == NO_SELECTION )
            {
                bHandled = true;
            }
            else
            {
                bHandled = _rControl.getSelEngine()->SelMouseButtonUp( _rMEvt );
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
