/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: keycod.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 16:19:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_KEYCODE_HXX
#define _SV_KEYCODE_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _SV_KEYCODES_HXX
#include <vcl/keycodes.hxx>
#endif

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
