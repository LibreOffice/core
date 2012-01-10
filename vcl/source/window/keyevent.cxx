/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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


#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <tools/debug.hxx>
#include <vcl/event.hxx>

KeyEvent::KeyEvent (const KeyEvent& rKeyEvent) :
    maKeyCode (rKeyEvent.maKeyCode),
    mnRepeat  (rKeyEvent.mnRepeat),
    mnCharCode(rKeyEvent.mnCharCode)
{}

KeyEvent KeyEvent::LogicalTextDirectionality (TextDirectionality eMode) const
{
    KeyEvent aClone(*this);

    sal_uInt16 nCode = maKeyCode.GetCode();
    sal_uInt16 nMod  = maKeyCode.GetAllModifier();

    switch (eMode)
    {
        case TextDirectionality_RightToLeft_TopToBottom:
            switch (nCode)
            {
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
            }
            break;

        case TextDirectionality_TopToBottom_RightToLeft:
            switch (nCode)
            {
                case KEY_DOWN:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_UP:    aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_DOWN,  nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_UP,    nMod); break;
            }
            break;

        case TextDirectionality_LeftToRight_TopToBottom:
            /* do nothing */
            break;
    }

    return aClone;
}


// -------------------------------------------------------

const Point&   HelpEvent::GetMousePosPixel() const
{
    //DBG_ASSERT( !mbKeyboardActivated, "Keyboard help has no mouse position !");
    return maPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
