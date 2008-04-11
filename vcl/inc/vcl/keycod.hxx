/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keycod.hxx,v $
 * $Revision: 1.4 $
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
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <vcl/keycodes.hxx>

#include <vcl/vclenum.hxx>

class Window;

// -------------
// - Key-Types -
// -------------

// Logische KeyFunktionen
/*
#ifndef ENUM_KEYFUNCTYPE_DECLARED
#define ENUM_KEYFUNCTYPE_DECLARED
enum KeyFuncType { KEYFUNC_DONTKNOW, KEYFUNC_NEW, KEYFUNC_OPEN,
                   KEYFUNC_SAVE, KEYFUNC_SAVEAS, KEYFUNC_PRINT,
                   KEYFUNC_CLOSE, KEYFUNC_QUIT,
                   KEYFUNC_CUT, KEYFUNC_COPY, KEYFUNC_PASTE, KEYFUNC_UNDO,
                   KEYFUNC_REDO, KEYFUNC_DELETE, KEYFUNC_REPEAT,
                   KEYFUNC_FIND, KEYFUNC_FINDBACKWARD, KEYFUNC_PROPERTIES,
                   KEYFUNC_FRONT };
#endif
*/
// -----------
// - KeyCode -
// -----------

class VCL_DLLPUBLIC KeyCode
{
private:
    USHORT      nCode;
    KeyFuncType eFunc;

public:
                KeyCode() { nCode = 0; eFunc = KEYFUNC_DONTKNOW; }
                KeyCode( const ResId& rResId );
                KeyCode( USHORT nKey, USHORT nModifier = 0 )
                    { nCode = nKey | nModifier; eFunc = KEYFUNC_DONTKNOW; }
                KeyCode( USHORT nKey, BOOL bShift, BOOL bMod1, BOOL bMod2 );
                KeyCode( KeyFuncType eFunction );

    USHORT      GetFullCode() const { return nCode; }
    USHORT      GetFullKeyCode() const { return (nCode) ; }
    KeyFuncType GetFullFunction() const { return eFunc; }
    BOOL        IsDefinedKeyCodeEqual( const KeyCode& rKeyCode ) const;

    USHORT      GetCode() const
                    { return (nCode & KEY_CODE); }

    USHORT      GetModifier() const
                    { return (nCode & KEY_MODTYPE); }
    USHORT      GetAllModifier() const
                    { return (nCode & KEY_ALLMODTYPE); }
    BOOL        IsShift() const
                    { return ((nCode & KEY_SHIFT) != 0); }
    BOOL        IsMod1() const
                    { return ((nCode & KEY_MOD1) != 0); }
    BOOL        IsMod2() const
                    { return ((nCode & KEY_MOD2) != 0); }
    BOOL        IsMod3() const
                    { return ((nCode & KEY_MOD3) != 0); }
    USHORT      GetGroup() const
                    { return (nCode & KEYGROUP_TYPE); }

    XubString   GetName( Window* pWindow = NULL ) const;
    XubString   GetSymbolName( const XubString& rFontName, Window* pWindow = NULL ) const;

    BOOL        IsFunction() const
                    { return ((eFunc != KEYFUNC_DONTKNOW) ? TRUE : FALSE); }

    KeyFuncType GetFunction() const;

    KeyCode&    operator = ( const KeyCode& rKeyCode );
    BOOL        operator ==( const KeyCode& rKeyCode ) const;
    BOOL        operator !=( const KeyCode& rKeyCode ) const;
};

inline KeyCode::KeyCode( USHORT nKey, BOOL bShift, BOOL bMod1, BOOL bMod2 )
{
    nCode = nKey;
    if( bShift )
        nCode |= KEY_SHIFT;
    if( bMod1 )
        nCode |= KEY_MOD1;
    if( bMod2 )
        nCode |= KEY_MOD2;
    eFunc = KEYFUNC_DONTKNOW;
}

inline BOOL KeyCode::operator ==( const KeyCode& rKeyCode ) const
{
    if ( (eFunc == KEYFUNC_DONTKNOW) && (rKeyCode.eFunc == KEYFUNC_DONTKNOW) )
        return (nCode == rKeyCode.nCode);
    else
        return (GetFunction() == rKeyCode.GetFunction());
}

inline BOOL KeyCode::operator !=( const KeyCode& rKeyCode ) const
{
    if ( (eFunc == KEYFUNC_DONTKNOW) && (rKeyCode.eFunc == KEYFUNC_DONTKNOW) )
        return (nCode != rKeyCode.nCode);
    else
        return (GetFunction() != rKeyCode.GetFunction());
}

inline BOOL KeyCode::IsDefinedKeyCodeEqual( const KeyCode& rKeyCode ) const
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
