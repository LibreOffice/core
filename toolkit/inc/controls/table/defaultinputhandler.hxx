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

#pragma once

#include <controls/table/mousefunction.hxx>
#include <rtl/ref.hxx>

#include <memory>
#include <vector>

class KeyEvent;
class MouseEvent;

namespace svt::table
{

    class DefaultInputHandler final
    {
    public:
        DefaultInputHandler();
        ~DefaultInputHandler();

        // all those methods have the same semantics as the equal-named methods of ->Window,
        // with the additional option to return a boolean value indicating whether
        // the event should be further processed by the ->Window implementations (<FALSE/>),
        // or whether it has been sufficiently handled by this class  (<FALSE/>).
        bool MouseMove(ITableControl& _rControl, const MouseEvent& rMEvt);
        bool MouseButtonDown(ITableControl& _rControl, const MouseEvent& rMEvt);
        bool MouseButtonUp(ITableControl& _rControl, const MouseEvent& rMEvt);
        static bool KeyInput(ITableControl& _rControl, const KeyEvent& rKEvt);
        static bool GetFocus(ITableControl& _rControl);
        static bool LoseFocus(ITableControl& _rControl);

    private:
        bool delegateMouseEvent( ITableControl& i_control, const MouseEvent& i_event,
            FunctionResult ( MouseFunction::*i_handlerMethod )( ITableControl&, const MouseEvent& ) );

        rtl::Reference< MouseFunction >  pActiveFunction;
        std::vector< rtl::Reference< MouseFunction > >  aMouseFunctions;
    };

    typedef std::shared_ptr<DefaultInputHandler> PTableInputHandler;

} // namespace svt::table



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
