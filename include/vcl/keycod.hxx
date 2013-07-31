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

#ifndef _SV_KEYCODE_HXX
#define _SV_KEYCODE_HXX

#include <tools/string.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <vcl/keycodes.hxx>

#include <vcl/vclenum.hxx>

class Window;

// -----------
// - KeyCode -
// -----------

class VCL_DLLPUBLIC KeyCode
{
private:
    sal_uInt16      nCode;
    KeyFuncType eFunc;

public:
                KeyCode() { nCode = 0; eFunc = KEYFUNC_DONTKNOW; }
                KeyCode( const ResId& rResId );
                KeyCode( sal_uInt16 nKey, sal_uInt16 nModifier = 0 )
                    { nCode = nKey | nModifier; eFunc = KEYFUNC_DONTKNOW; }
                KeyCode( sal_uInt16 nKey, sal_Bool bShift, sal_Bool bMod1, sal_Bool bMod2, sal_Bool bMod3 );
                KeyCode( KeyFuncType eFunction );

    sal_uInt16      GetFullCode() const { return nCode; }
    KeyFuncType GetFullFunction() const { return eFunc; }
    sal_Bool        IsDefinedKeyCodeEqual( const KeyCode& rKeyCode ) const;

    sal_uInt16      GetCode() const
                    { return (nCode & KEY_CODE); }

    sal_uInt16      GetModifier() const
                    { return (nCode & KEY_MODTYPE); }
    sal_uInt16      GetAllModifier() const
                    { return (nCode & KEY_ALLMODTYPE); }
    sal_Bool        IsShift() const
                    { return ((nCode & KEY_SHIFT) != 0); }
    sal_Bool        IsMod1() const
                    { return ((nCode & KEY_MOD1) != 0); }
    sal_Bool        IsMod2() const
                    { return ((nCode & KEY_MOD2) != 0); }
    sal_Bool        IsMod3() const
                    { return ((nCode & KEY_MOD3) != 0); }
    sal_uInt16      GetGroup() const
                    { return (nCode & KEYGROUP_TYPE); }

    OUString        GetName( Window* pWindow = NULL ) const;

    sal_Bool        IsFunction() const
                    { return ((eFunc != KEYFUNC_DONTKNOW) ? sal_True : sal_False); }

    KeyFuncType GetFunction() const;

    KeyCode&    operator = ( const KeyCode& rKeyCode );
    sal_Bool        operator ==( const KeyCode& rKeyCode ) const;
    sal_Bool        operator !=( const KeyCode& rKeyCode ) const;
};

inline KeyCode::KeyCode( sal_uInt16 nKey, sal_Bool bShift, sal_Bool bMod1, sal_Bool bMod2, sal_Bool bMod3 )
{
    nCode = nKey;
    if( bShift )
        nCode |= KEY_SHIFT;
    if( bMod1 )
        nCode |= KEY_MOD1;
    if( bMod2 )
        nCode |= KEY_MOD2;
    if( bMod3 )
        nCode |= KEY_MOD3;
    eFunc = KEYFUNC_DONTKNOW;
}

inline sal_Bool KeyCode::operator ==( const KeyCode& rKeyCode ) const
{
    if ( (eFunc == KEYFUNC_DONTKNOW) && (rKeyCode.eFunc == KEYFUNC_DONTKNOW) )
        return (nCode == rKeyCode.nCode);
    else
        return (GetFunction() == rKeyCode.GetFunction());
}

inline sal_Bool KeyCode::operator !=( const KeyCode& rKeyCode ) const
{
    if ( (eFunc == KEYFUNC_DONTKNOW) && (rKeyCode.eFunc == KEYFUNC_DONTKNOW) )
        return (nCode != rKeyCode.nCode);
    else
        return (GetFunction() != rKeyCode.GetFunction());
}

inline sal_Bool KeyCode::IsDefinedKeyCodeEqual( const KeyCode& rKeyCode ) const
{
    if ( (eFunc == KEYFUNC_DONTKNOW) && (rKeyCode.eFunc == KEYFUNC_DONTKNOW) )
        return (GetFullCode() == rKeyCode.GetFullCode());
    return (GetFunction() == rKeyCode.GetFunction());
}

inline KeyCode& KeyCode::operator = ( const KeyCode& rKeyCode )
{
    nCode = rKeyCode.nCode;
    eFunc = rKeyCode.eFunc;

    return *this;
}

#endif // _SV_KEYCODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
