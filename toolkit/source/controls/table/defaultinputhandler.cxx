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

#include <controls/table/defaultinputhandler.hxx>
#include <controls/table/tablecontrolinterface.hxx>

#include <vcl/event.hxx>
#include <osl/diagnose.h>

namespace svt::table
{


    //= DefaultInputHandler


    DefaultInputHandler::DefaultInputHandler()
    {
        aMouseFunctions.push_back( new ColumnResize );
        aMouseFunctions.push_back( new RowSelection );
        aMouseFunctions.push_back( new ColumnSortHandler );
    }


    DefaultInputHandler::~DefaultInputHandler()
    {
    }


    bool DefaultInputHandler::delegateMouseEvent( ITableControl& i_control, const MouseEvent& i_event,
        FunctionResult ( MouseFunction::*i_handlerMethod )( ITableControl&, const MouseEvent& ) )
    {
        if ( pActiveFunction.is() )
        {
            bool furtherHandler = false;
            switch ( (pActiveFunction.get()->*i_handlerMethod)( i_control, i_event ) )
            {
            case ActivateFunction:
                OSL_ENSURE( false, "lcl_delegateMouseEvent: unexpected - function already *is* active!" );
                break;
            case ContinueFunction:
                break;
            case DeactivateFunction:
                pActiveFunction.clear();
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
        for (auto const& mouseFunction : aMouseFunctions)
        {
            if (handled)
                break;
            if (mouseFunction == pActiveFunction)
                // we already invoked this function
                continue;

            switch ( (mouseFunction.get()->*i_handlerMethod)( i_control, i_event ) )
            {
            case ActivateFunction:
                pActiveFunction = mouseFunction;
                handled = true;
                break;
            case ContinueFunction:
            case DeactivateFunction:
                OSL_ENSURE( false, "lcl_delegateMouseEvent: unexpected: inactive handler cannot be continued or deactivated!" );
                break;
            case SkipFunction:
                handled = false;
                break;
            }
        }
        return handled;
    }


    bool DefaultInputHandler::MouseMove( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return delegateMouseEvent( i_tableControl, i_event, &MouseFunction::handleMouseMove );
    }


    bool DefaultInputHandler::MouseButtonDown( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return delegateMouseEvent( i_tableControl, i_event, &MouseFunction::handleMouseDown );
    }


    bool DefaultInputHandler::MouseButtonUp( ITableControl& i_tableControl, const MouseEvent& i_event )
    {
        return delegateMouseEvent( i_tableControl, i_event, &MouseFunction::handleMouseUp );
    }


    bool DefaultInputHandler::KeyInput( ITableControl& _rControl, const KeyEvent& rKEvt )
    {
        bool bHandled = false;

        const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
        sal_uInt16 nKeyCode = rKeyCode.GetCode();

        struct ActionMapEntry
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
            { KEY_HOME,     KEY_SHIFT,  cursorSelectRowAreaTop }
        };
        for (const ActionMapEntry& rAction : aKnownActions)
        {
            if ( ( rAction.nKeyCode == nKeyCode ) && ( rAction.nKeyModifier == rKeyCode.GetModifier() ) )
            {
                bHandled = _rControl.dispatchAction( rAction.eAction );
                break;
            }
        }

        return bHandled;
    }

} // namespace svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
