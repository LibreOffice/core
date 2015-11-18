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

#include <vcl/event.hxx>

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

/** inits this vcl KeyEvent with all settings from the given awt event **/
MouseEvent::MouseEvent( const css::awt::MouseEvent& rEvent )
: maPos( rEvent.X, rEvent.Y )
, mnMode( MouseEventModifiers::NONE )
, mnClicks( static_cast< sal_uInt16 >( rEvent.ClickCount ) )
, mnCode( 0 )
{
    if( rEvent.Modifiers )
    {
        if( (rEvent.Modifiers & css::awt::KeyModifier::SHIFT) != 0 )
            mnCode |= KEY_SHIFT;
        if( (rEvent.Modifiers & css::awt::KeyModifier::MOD1) != 0 )
            mnCode |= KEY_MOD1;
        if( (rEvent.Modifiers & css::awt::KeyModifier::MOD2) != 0 )
            mnCode |= KEY_MOD2;
        if( (rEvent.Modifiers & css::awt::KeyModifier::MOD3) != 0 )
            mnCode |= KEY_MOD3;
    }

    if( rEvent.Buttons )
    {
        if( (rEvent.Buttons & css::awt::MouseButton::LEFT) != 0 )
            mnCode |= MOUSE_LEFT;
        if( (rEvent.Buttons & css::awt::MouseButton::RIGHT) != 0 )
            mnCode |= MOUSE_RIGHT;
        if( (rEvent.Buttons & css::awt::MouseButton::MIDDLE) != 0 )
            mnCode |= MOUSE_MIDDLE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
