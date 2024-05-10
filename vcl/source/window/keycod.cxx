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

#include <sal/config.h>

#include <accel.hxx>
#include <salframe.hxx>
#include <svdata.hxx>

#include <vcl/window.hxx>
#include <vcl/keycod.hxx>

const sal_uInt16 aImplKeyFuncTab[(static_cast<int>(KeyFuncType::DELETE)+1)*4] =
{
    0, 0, 0, 0,                                                    // KeyFuncType::DONTKNOW
    KEY_X | KEY_MOD1, KEY_DELETE | KEY_SHIFT, KEY_CUT, 0,          // KeyFuncType::CUT
    KEY_C | KEY_MOD1, KEY_INSERT | KEY_MOD1, KEY_COPY, 0,          // KeyFuncType::COPY
    KEY_V | KEY_MOD1, KEY_INSERT | KEY_SHIFT, KEY_PASTE, 0,        // KeyFuncType::PASTE
    KEY_Z | KEY_MOD1, KEY_BACKSPACE | KEY_MOD2, KEY_UNDO, 0,       // KeyFuncType::UNDO
    KEY_Y | KEY_MOD1, KEY_UNDO | KEY_SHIFT, 0, 0,                  // KeyFuncType::REDO
    KEY_DELETE, 0, 0, 0                                            // KeyFuncType::DELETE
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

OUString vcl::KeyCode::GetName() const
{
    vcl::Window* pWindow = ImplGetDefaultWindow();
    return pWindow ? pWindow->ImplGetFrame()->GetKeyName( GetFullCode() ) : u""_ustr;
}

KeyFuncType vcl::KeyCode::GetFunction() const
{
    if ( eFunc != KeyFuncType::DONTKNOW )
        return eFunc;

    sal_uInt16 nCompCode = GetModifier() | GetCode();
    if ( nCompCode )
    {
        for ( sal_uInt16 i = sal_uInt16(KeyFuncType::CUT); i <= sal_uInt16(KeyFuncType::DELETE); i++ )
        {
            sal_uInt16 nKeyCode1;
            sal_uInt16 nKeyCode2;
            sal_uInt16 nKeyCode3;
            sal_uInt16 nKeyCode4;
            ImplGetKeyCode( static_cast<KeyFuncType>(i), nKeyCode1, nKeyCode2, nKeyCode3, nKeyCode4 );
            if ( (nCompCode == nKeyCode1) || (nCompCode == nKeyCode2) || (nCompCode == nKeyCode3) || (nCompCode == nKeyCode4) )
                return static_cast<KeyFuncType>(i);
        }
    }

    return KeyFuncType::DONTKNOW;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
