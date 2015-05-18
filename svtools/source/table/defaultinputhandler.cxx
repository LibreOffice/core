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


#include "table/defaultinputhandler.hxx"
#include "table/tablecontrolinterface.hxx"

#include "tabledatawindow.hxx"
#include "mousefunction.hxx"

#include <vcl/event.hxx>
#include <vcl/cursor.hxx>


namespace svt { namespace table
{


    typedef ::rtl::Reference< MouseFunction >  PMouseFunction;
    typedef ::std::vector< PMouseFunction >     MouseFunctions;
    struct DefaultInputHandler_Impl
    {
        PMouseFunction  pActiveFunction;
        MouseFunctions  aMouseFunctions;
    };


    //= DefaultInputHandler


    DefaultInputHandler::DefaultInputHandler()
        :m_pImpl( new DefaultInputHandler_Impl )
    {
        m_pImpl->aMouseFunctions.push_back( new ColumnResize );
        m_pImpl->aMouseFunctions.push_back( new RowSelection );
        m_pImpl->aMouseFunctions.push_back( new ColumnSortHandler );
    }


    DefaultInputHandler::~DefaultInputHandler()
    {
    }


    namespace
    {
        bool lcl_delegateMouseEvent( DefaultInputHandler_Impl& i_impl, ITableControl& i_control, const MouseEvent& i_event,
            FunctionResult ( MouseFunction::*i_handlerMethod )( ITableControl&, const MouseEvent& ) )
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


    bool DefaultInputHandler::MouseMove( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &MouseFunction::handleMouseMove );
    }


    bool DefaultInputHandler::MouseButtonDown( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &MouseFunction::handleMouseDown );
    }


    bool DefaultInputHandler::MouseButtonUp( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return lcl_delegateMouseEvent( *m_pImpl, i_tableControl, i_event, &MouseFunction::handleMouseUp );
    }


    bool DefaultInputHandler::KeyInput( ITableControl& _rControl, const KeyEvent& rKEvt )
    {
        bool bHandled = false;

        const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
        sal_uInt16 nKeyCode = rKeyCode.GetCode();

        struct _ActionMapEntry
        {
            sal_uInt16              nKeyCode;
            sal_uInt16              nKeyModifier;
            TableControlAction  eAction;
        }
        static const aKnownActions[] = {
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
            if ( ( pActions->nKeyCode == nKeyCode ) && ( pActions->nKeyModifier == rKeyCode.GetModifier() ) )
            {
                bHandled = _rControl.dispatchAction( pActions->eAction );
                break;
            }
        }

        return bHandled;
    }


    bool DefaultInputHandler::GetFocus( ITableControl& _rControl )
    {
        _rControl.showCursor();
        return false;   // continue processing
    }


    bool DefaultInputHandler::LoseFocus( ITableControl& _rControl )
    {
        _rControl.hideCursor();
        return false;   // continue processing
    }


    bool DefaultInputHandler::RequestHelp( ITableControl& _rControl, const HelpEvent& _rHEvt )
    {
        (void)_rControl;
        (void)_rHEvt;
        // TODO
        return false;
    }


    bool DefaultInputHandler::Command( ITableControl& _rControl, const CommandEvent& _rCEvt )
    {
        (void)_rControl;
        (void)_rCEvt;
        // TODO
        return false;
    }


    bool DefaultInputHandler::PreNotify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }


    bool DefaultInputHandler::Notify( ITableControl& _rControl, NotifyEvent& _rNEvt )
    {
        (void)_rControl;
        (void)_rNEvt;
        // TODO
        return false;
    }

} }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
