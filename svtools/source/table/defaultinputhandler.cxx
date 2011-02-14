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
#include "mousefunction.hxx"

#include <tools/debug.hxx>
#include <vcl/event.hxx>
#include <vcl/cursor.hxx>

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    typedef ::rtl::Reference< IMouseFunction >  PMouseFunction;
    typedef ::std::vector< PMouseFunction >     MouseFunctions;
    struct DefaultInputHandler_Impl
    {
        PMouseFunction  pActiveFunction;
        MouseFunctions  aMouseFunctions;
    };

    //==================================================================================================================
    //= DefaultInputHandler
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    DefaultInputHandler::DefaultInputHandler()
        :m_pImpl( new DefaultInputHandler_Impl )
    {
        m_pImpl->aMouseFunctions.push_back( new ColumnResize );
        m_pImpl->aMouseFunctions.push_back( new RowSelection );
        m_pImpl->aMouseFunctions.push_back( new ColumnSortHandler );
    }

    //------------------------------------------------------------------------------------------------------------------
    DefaultInputHandler::~DefaultInputHandler()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        bool lcl_delegateMouseEvent( DefaultInputHandler_Impl& i_impl, ITableControl& i_control, const MouseEvent& i_event,
            FunctionResult ( IMouseFunction::*i_handlerMethod )( ITableControl&, const MouseEvent& ) )
        {
            if ( i_impl.pActiveFunction.is() )
            {
                bool furtherHandler = false;
                switch ( (i_impl.pActiveFunction.get()->*i_handlerMethod)( i_control, i_event ) )
                {
                case ActivateFunction:
                    OSL_ENSURE( false, "lcl_delegateMouseEvent: unexpected - function already *is* active!" );
                    break;
                case ContinueFunction:
                    break;
                case DeactivateFunction:
                    i_impl.pActiveFunction.clear();
                    break;
                case SkipFunction:
                    furtherHandler = true;
                    break;
                }
                if ( !furtherHandler )
                    // handled the event
                    return true;
            }

            // ask all other handlers
            bool handled = false;
            for (   MouseFunctions::iterator handler = i_impl.aMouseFunctions.begin();
                    ( handler != i_impl.aMouseFunctions.end() ) && !handled;
                    ++handler
                )
            {
                if ( *handler == i_impl.pActiveFunction )
                    // we already invoked this function
                    continue;

                switch ( (handler->get()->*i_handlerMethod)( i_control, i_event ) )
                {
                case ActivateFunction:
                    i_impl.pActiveFunction = *handler;
                    handled = true;
                    break;
                case ContinueFunction:
                case DeactivateFunction:
                    OSL_ENSURE( false, "lcl_delegateMouseEvent: unexpected: inactivate handler cannot be continued or deactivated!" );
                    break;
                case SkipFunction:
                    handled = false;
                    break;
                }
            }
            return handled;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::MouseMove( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &IMouseFunction::handleMouseMove );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonDown( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &IMouseFunction::handleMouseDown );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::MouseButtonUp( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &IMouseFunction::handleMouseUp );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::KeyInput( ITableControl& _rControl, const KeyEvent& rKEvt )
    {
        bool bHandled = false;

        const KeyCode& rKeyCode = rKEvt.GetKeyCode();
        sal_uInt16 nKeyCode = rKeyCode.GetCode();

        struct _ActionMapEntry
        {
            sal_uInt16              nKeyCode;
            sal_uInt16              nKeyModifier;
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

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::GetFocus( ITableControl& _rControl )
    {
        _rControl.showCursor();
        return false;   // continue processing
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::LoseFocus( ITableControl& _rControl )
    {
        _rControl.hideCursor();
        return false;   // continue processing
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::RequestHelp( ITableControl& _rControl, const HelpEvent& _rHEvt )
    {
        (void)_rControl;
        (void)_rHEvt;
        // TODO
        return false;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::Command( ITableControl& _rControl, const CommandEvent& _rCEvt )
    {
        (void)_rControl;
        (void)_rCEvt;
        // TODO
        return false;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::PreNotify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }

    //------------------------------------------------------------------------------------------------------------------
    bool DefaultInputHandler::Notify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }
//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................
