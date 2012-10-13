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
    sal_uInt16      GetFullKeyCode() const { return (nCode) ; }
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

    XubString   GetName( Window* pWindow = NULL ) const;

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
        return (GetFullKeyCode() == rKeyCode.GetFullKeyCode());
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
