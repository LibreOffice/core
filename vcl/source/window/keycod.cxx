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

#include <salinst.hxx>
#include <salframe.hxx>
#include <svdata.hxx>

#include <vcl/window.hxx>
#include <vcl/keycod.hxx>

static const sal_uInt16 aImplKeyFuncTab[(static_cast<int>(KeyFuncType::FRONT)+1)*4] =
{
    0, 0, 0, 0,                                                    // KeyFuncType::DONTKNOW
    KEY_N | KEY_MOD1, 0, 0, 0,                                     // KeyFuncType::NEW
    KEY_O | KEY_MOD1, KEY_OPEN, 0, 0,                              // KeyFuncType::OPEN
    KEY_S | KEY_MOD1, 0, 0, 0,                                     // KeyFuncType::SAVE
    KEY_S | KEY_SHIFT | KEY_MOD1, 0, 0, 0,                         // KeyFuncType::SAVEAS
    KEY_P | KEY_MOD1, 0, 0, 0,                                     // KeyFuncType::PRINT
    KEY_W | KEY_MOD1, KEY_F4 | KEY_MOD1, 0, 0,                     // KeyFuncType::CLOSE
    KEY_Q | KEY_MOD1, KEY_F4 | KEY_MOD2, 0, 0,                     // KeyFuncType::QUIT
    KEY_X | KEY_MOD1, KEY_DELETE | KEY_SHIFT, KEY_CUT, 0,          // KeyFuncType::CUT
    KEY_C | KEY_MOD1, KEY_INSERT | KEY_MOD1, KEY_COPY, 0,          // KeyFuncType::COPY
    KEY_V | KEY_MOD1, KEY_INSERT | KEY_SHIFT, KEY_PASTE, 0,        // KeyFuncType::PASTE
    KEY_Z | KEY_MOD1, KEY_BACKSPACE | KEY_MOD2, KEY_UNDO, 0,       // KeyFuncType::UNDO
    KEY_Y | KEY_MOD1, KEY_UNDO | KEY_SHIFT, 0, 0,                  // KeyFuncType::REDO
    KEY_DELETE, 0, 0, 0,                                           // KeyFuncType::DELETE
    KEY_REPEAT, 0, 0, 0,                                           // KeyFuncType::REPEAT
    KEY_F | KEY_MOD1, KEY_FIND, 0, 0,                              // KeyFuncType::FIND
    KEY_F | KEY_SHIFT | KEY_MOD1, KEY_SHIFT | KEY_FIND, 0, 0,      // KeyFuncType::FINDBACKWARD
    KEY_RETURN | KEY_MOD2, 0, 0, 0,                                // KeyFuncType::PROPERTIES
    0, 0, 0, 0                                                     // KeyFuncType::FRONT
};

bool ImplGetKeyCode( KeyFuncType eFunc, sal_uInt16& rCode1, sal_uInt16& rCode2, sal_uInt16& rCode3, sal_uInt16& rCode4 )
{
    size_t nIndex = static_cast<size_t>(eFunc);
    nIndex *= 4;

    assert(nIndex + 3 < SAL_N_ELEMENTS(aImplKeyFuncTab) && "bad key code index");
    if (nIndex + 3  >= SAL_N_ELEMENTS(aImplKeyFuncTab))
    {
        rCode1 = rCode2 = rCode3 = rCode4 = 0;
        return false;
    }

    rCode1 = aImplKeyFuncTab[nIndex];
    rCode2 = aImplKeyFuncTab[nIndex+1];
    rCode3 = aImplKeyFuncTab[nIndex+2];
    rCode4 = aImplKeyFuncTab[nIndex+3];
    return true;
}

vcl::KeyCode::KeyCode( KeyFuncType eFunction )
{
    sal_uInt16 nDummy;
    ImplGetKeyCode( eFunction, nKeyCodeAndModifiers, nDummy, nDummy, nDummy );
    eFunc = eFunction;
}

OUString vcl::KeyCode::GetName( vcl::Window* pWindow ) const
{
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    return pWindow ? pWindow->ImplGetFrame()->GetKeyName( GetFullCode() ) : "";
}

KeyFuncType vcl::KeyCode::GetFunction() const
{
    if ( eFunc != KeyFuncType::DONTKNOW )
        return eFunc;

    sal_uInt16 nCompCode = GetModifier() | GetCode();
    if ( nCompCode )
    {
        for ( sal_uInt16 i = (sal_uInt16)KeyFuncType::NEW; i < (sal_uInt16)KeyFuncType::FRONT; i++ )
        {
            sal_uInt16 nKeyCode1;
            sal_uInt16 nKeyCode2;
            sal_uInt16 nKeyCode3;
            sal_uInt16 nKeyCode4;
            ImplGetKeyCode( (KeyFuncType)i, nKeyCode1, nKeyCode2, nKeyCode3, nKeyCode4 );
            if ( (nCompCode == nKeyCode1) || (nCompCode == nKeyCode2) || (nCompCode == nKeyCode3) || (nCompCode == nKeyCode4) )
                return (KeyFuncType)i;
        }
    }

    return KeyFuncType::DONTKNOW;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
