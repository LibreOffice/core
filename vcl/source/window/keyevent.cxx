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

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <vcl/event.hxx>

KeyEvent KeyEvent::LogicalTextDirectionality (TextDirectionality eMode) const
{
    KeyEvent aClone(*this);

    sal_uInt16 nCode = maKeyCode.GetCode();
    sal_uInt16 nMod  = maKeyCode.GetModifier();

    switch (eMode)
    {
        case TextDirectionality::RightToLeft_TopToBottom:
            switch (nCode)
            {
                case KEY_LEFT:  aClone.maKeyCode = vcl::KeyCode(KEY_RIGHT, nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = vcl::KeyCode(KEY_LEFT,  nMod); break;
            }
            break;

        case TextDirectionality::TopToBottom_RightToLeft:
            switch (nCode)
            {
                case KEY_DOWN:  aClone.maKeyCode = vcl::KeyCode(KEY_RIGHT, nMod); break;
                case KEY_UP:    aClone.maKeyCode = vcl::KeyCode(KEY_LEFT,  nMod); break;
                case KEY_LEFT:  aClone.maKeyCode = vcl::KeyCode(KEY_DOWN,  nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = vcl::KeyCode(KEY_UP,    nMod); break;
            }
            break;

        case TextDirectionality::BottomToTop_LeftToRight:
            switch (nCode)
            {
                case KEY_DOWN:  aClone.maKeyCode = vcl::KeyCode(KEY_LEFT, nMod); break;
                case KEY_UP:    aClone.maKeyCode = vcl::KeyCode(KEY_RIGHT, nMod); break;
                case KEY_LEFT:  aClone.maKeyCode = vcl::KeyCode(KEY_UP, nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = vcl::KeyCode(KEY_DOWN, nMod); break;
            }
            break;

        case TextDirectionality::LeftToRight_TopToBottom:
            /* do nothing */
            break;
    }

    return aClone;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
