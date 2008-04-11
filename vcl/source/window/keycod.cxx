/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: keycod.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salinst.hxx>
#include <vcl/salframe.hxx>
#include <vcl/svdata.hxx>
#include <vcl/window.hxx>
#ifndef _SV_KEYCOD_HXX
#include <vcl/keycod.hxx>
#endif

#ifndef _RC_H
#include <tools/rc.h>
#endif



// =======================================================================
#ifdef MACOSX
static USHORT aImplKeyFuncTab[(KEYFUNC_FRONT+1)*3] =
{
    0, 0, 0,                                                // KEYFUNC_DONTKNOW
    KEY_N | KEY_MOD3, 0, 0,                                 // KEYFUNC_NEW
    KEY_O | KEY_MOD3, KEY_OPEN, 0,                          // KEYFUNC_OPEN
    KEY_S | KEY_MOD3, 0, 0,                                 // KEYFUNC_SAVE
    0, 0, 0,                                                // KEYFUNC_SAVEAS
    KEY_P | KEY_MOD3, 0, 0,                                 // KEYFUNC_PRINT
    KEY_W | KEY_MOD3, KEY_F4 | KEY_MOD3, 0,                 // KEYFUNC_CLOSE
    KEY_Q | KEY_MOD3, KEY_F4 | KEY_MOD2, 0,                 // KEYFUNC_QUIT
    KEY_X | KEY_MOD3, KEY_DELETE | KEY_SHIFT, KEY_CUT,      // KEYFUNC_CUT
    KEY_C | KEY_MOD3, KEY_INSERT | KEY_MOD3, KEY_COPY,      // KEYFUNC_COPY
    KEY_V | KEY_MOD3, KEY_INSERT | KEY_SHIFT, KEY_PASTE,    // KEYFUNC_PASTE
    KEY_Z | KEY_MOD3, KEY_BACKSPACE | KEY_MOD2, KEY_UNDO,   // KEYFUNC_UNDO
    0, 0, 0,                                                // KEYFUNC_REDO
    KEY_DELETE, 0, 0,                                       // KEYFUNC_DELETE
    KEY_REPEAT, 0, 0,                                       // KEYFUNC_REPEAT
    KEY_F | KEY_MOD3, KEY_FIND, 0,                          // KEYFUNC_FIND
    KEY_F | KEY_SHIFT | KEY_MOD3, KEY_SHIFT | KEY_FIND, 0,  // KEYFUNC_FINDBACKWARD
    KEY_RETURN | KEY_MOD2, 0, 0,                            // KEYFUNC_PROPERTIES
    0, 0, 0                                                 // KEYFUNC_FRONT
};
#else
static USHORT aImplKeyFuncTab[(KEYFUNC_FRONT+1)*3] =
{
    0, 0, 0,                                                // KEYFUNC_DONTKNOW
    KEY_N | KEY_MOD1, 0, 0,                                 // KEYFUNC_NEW
    KEY_O | KEY_MOD1, KEY_OPEN, 0,                          // KEYFUNC_OPEN
    KEY_S | KEY_MOD1, 0, 0,                                 // KEYFUNC_SAVE
    0, 0, 0,                                                // KEYFUNC_SAVEAS
    KEY_P | KEY_MOD1, 0, 0,                                 // KEYFUNC_PRINT
    KEY_W | KEY_MOD1, KEY_F4 | KEY_MOD1, 0,                 // KEYFUNC_CLOSE
    KEY_Q | KEY_MOD1, KEY_F4 | KEY_MOD2, 0,                 // KEYFUNC_QUIT
    KEY_X | KEY_MOD1, KEY_DELETE | KEY_SHIFT, KEY_CUT,      // KEYFUNC_CUT
    KEY_C | KEY_MOD1, KEY_INSERT | KEY_MOD1, KEY_COPY,      // KEYFUNC_COPY
    KEY_V | KEY_MOD1, KEY_INSERT | KEY_SHIFT, KEY_PASTE,    // KEYFUNC_PASTE
    KEY_Z | KEY_MOD1, KEY_BACKSPACE | KEY_MOD2, KEY_UNDO,   // KEYFUNC_UNDO
    0, 0, 0,                                                // KEYFUNC_REDO
    KEY_DELETE, 0, 0,                                       // KEYFUNC_DELETE
    KEY_REPEAT, 0, 0,                                       // KEYFUNC_REPEAT
    KEY_F | KEY_MOD1, KEY_FIND, 0,                          // KEYFUNC_FIND
    KEY_F | KEY_SHIFT | KEY_MOD1, KEY_SHIFT | KEY_FIND, 0,  // KEYFUNC_FINDBACKWARD
    KEY_RETURN | KEY_MOD2, 0, 0,                            // KEYFUNC_PROPERTIES
    0, 0, 0                                                 // KEYFUNC_FRONT
};
#endif
// -----------------------------------------------------------------------

void ImplGetKeyCode( KeyFuncType eFunc, USHORT& rCode1, USHORT& rCode2, USHORT& rCode3 )
{
    USHORT nIndex = (USHORT)eFunc;
    nIndex *= 3;
    rCode1 = aImplKeyFuncTab[nIndex];
    rCode2 = aImplKeyFuncTab[nIndex+1];
    rCode3 = aImplKeyFuncTab[nIndex+2];
}

// =======================================================================

KeyCode::KeyCode( KeyFuncType eFunction )
{
    USHORT nDummy;
    ImplGetKeyCode( eFunction, nCode, nDummy, nDummy );
    eFunc = eFunction;
}

// -----------------------------------------------------------------------

KeyCode::KeyCode( const ResId& rResId )
{
    rResId.SetRT( RSC_KEYCODE );

    ResMgr* pResMgr = rResId.GetResMgr();
    if ( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        ULONG nKeyCode  = pResMgr->ReadLong();
        ULONG nModifier = pResMgr->ReadLong();
        ULONG nKeyFunc  = pResMgr->ReadLong();

        eFunc = (KeyFuncType)nKeyFunc;
        if ( eFunc != KEYFUNC_DONTKNOW )
        {
            USHORT nDummy;
            ImplGetKeyCode( eFunc, nCode, nDummy, nDummy );
        }
        else
            nCode = sal::static_int_cast<USHORT>(nKeyCode | nModifier);
    }
}

// -----------------------------------------------------------------------

XubString KeyCode::GetName( Window* pWindow ) const
{
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    return pWindow ? pWindow->ImplGetFrame()->GetKeyName( GetFullCode() ) : XubString();
}

// -----------------------------------------------------------------------

XubString KeyCode::GetSymbolName( const XubString& rFontName, Window* pWindow ) const
{
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    return pWindow ? pWindow->ImplGetFrame()->GetSymbolKeyName( rFontName, GetFullCode() ) : XubString();
}

// -----------------------------------------------------------------------

KeyFuncType KeyCode::GetFunction() const
{
    if ( eFunc != KEYFUNC_DONTKNOW )
        return eFunc;

    USHORT nCompCode = GetModifier() | GetCode();
    if ( nCompCode )
    {
        for ( USHORT i = (USHORT)KEYFUNC_NEW; i < (USHORT)KEYFUNC_FRONT; i++ )
        {
            USHORT nKeyCode1;
            USHORT nKeyCode2;
            USHORT nKeyCode3;
            ImplGetKeyCode( (KeyFuncType)i, nKeyCode1, nKeyCode2, nKeyCode3 );
            if ( (nCompCode == nKeyCode1) || (nCompCode == nKeyCode2) || (nCompCode == nKeyCode3) )
                return (KeyFuncType)i;
        }
    }

    return KEYFUNC_DONTKNOW;
}
