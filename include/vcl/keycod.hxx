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

#ifndef INCLUDED_VCL_KEYCOD_HXX
#define INCLUDED_VCL_KEYCOD_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <vcl/keycodes.hxx>

namespace vcl { class Window; }

enum class KeyFuncType : sal_Int32 { DONTKNOW, NEW, OPEN, SAVE,
                   SAVEAS, PRINT, CLOSE, QUIT,
                   CUT, COPY, PASTE, UNDO,
                   REDO, DELETE, REPEAT, FIND,
                   FINDBACKWARD, PROPERTIES, FRONT };

namespace vcl
{

class VCL_DLLPUBLIC KeyCode
{
private:
    sal_uInt16      nKeyCodeAndModifiers;
    KeyFuncType     eFunc;

public:
                    KeyCode() { nKeyCodeAndModifiers = 0; eFunc = KeyFuncType::DONTKNOW; }
                    KeyCode( sal_uInt16 nKey, sal_uInt16 nModifier = 0 );
                    KeyCode( sal_uInt16 nKey, bool bShift, bool bMod1, bool bMod2, bool bMod3 );
                    KeyCode( KeyFuncType eFunction );

    sal_uInt16      GetFullCode() const { return nKeyCodeAndModifiers; }
    KeyFuncType     GetFullFunction() const { return eFunc; }

    sal_uInt16      GetCode() const
                    { return (nKeyCodeAndModifiers & KEY_CODE_MASK); }

    sal_uInt16      GetModifier() const
                    { return (nKeyCodeAndModifiers & KEY_MODIFIERS_MASK); }
    bool            IsShift() const
                    { return ((nKeyCodeAndModifiers & KEY_SHIFT) != 0); }
    bool            IsMod1() const
                    { return ((nKeyCodeAndModifiers & KEY_MOD1) != 0); }
    bool            IsMod2() const
                    { return ((nKeyCodeAndModifiers & KEY_MOD2) != 0); }
    bool            IsMod3() const
                    { return ((nKeyCodeAndModifiers & KEY_MOD3) != 0); }
    sal_uInt16      GetGroup() const
                    { return (nKeyCodeAndModifiers & KEYGROUP_TYPE); }

    OUString        GetName( vcl::Window* pWindow = nullptr ) const;

    bool            IsFunction() const
                    { return (eFunc != KeyFuncType::DONTKNOW); }

    KeyFuncType     GetFunction() const;

    bool            operator ==( const KeyCode& rKeyCode ) const;
    bool            operator !=( const KeyCode& rKeyCode ) const;
};

} // namespace vcl

inline vcl::KeyCode::KeyCode( sal_uInt16 nKey, sal_uInt16 nModifier )
{
    nKeyCodeAndModifiers = nKey | nModifier;
    eFunc = KeyFuncType::DONTKNOW;
}

inline vcl::KeyCode::KeyCode( sal_uInt16 nKey, bool bShift, bool bMod1, bool bMod2, bool bMod3 )
{
    nKeyCodeAndModifiers = nKey;
    if( bShift )
        nKeyCodeAndModifiers |= KEY_SHIFT;
    if( bMod1 )
        nKeyCodeAndModifiers |= KEY_MOD1;
    if( bMod2 )
        nKeyCodeAndModifiers |= KEY_MOD2;
    if( bMod3 )
        nKeyCodeAndModifiers |= KEY_MOD3;
    eFunc = KeyFuncType::DONTKNOW;
}

inline bool vcl::KeyCode::operator ==( const vcl::KeyCode& rKeyCode ) const
{
    if ( (eFunc == KeyFuncType::DONTKNOW) && (rKeyCode.eFunc == KeyFuncType::DONTKNOW) )
        return (nKeyCodeAndModifiers == rKeyCode.nKeyCodeAndModifiers);
    else
        return (GetFunction() == rKeyCode.GetFunction());
}

inline bool vcl::KeyCode::operator !=( const vcl::KeyCode& rKeyCode ) const
{
    if ( (eFunc == KeyFuncType::DONTKNOW) && (rKeyCode.eFunc == KeyFuncType::DONTKNOW) )
        return (nKeyCodeAndModifiers != rKeyCode.nKeyCodeAndModifiers);
    else
        return (GetFunction() != rKeyCode.GetFunction());
}

#endif // INCLUDED_VCL_KEYCOD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
