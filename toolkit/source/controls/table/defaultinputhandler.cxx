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

} // namespace svt::table


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
