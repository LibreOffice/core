/*************************************************************************
 *
 *  $RCSfile: keycod.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_KEYCOD_CXX

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_KEYCOD_HXX
#include <keycod.hxx>
#endif

#ifndef _RC_H
#include <rc.h>
#endif

#ifdef REMOTE_APPSERVER
#include <rmwindow.hxx>
#endif

#pragma hdrstop

// =======================================================================

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
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    if ( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        USHORT nKeyCode     = pResMgr->ReadShort();
        USHORT nModifier    = pResMgr->ReadShort();
        USHORT nKeyFunc     = pResMgr->ReadShort();

        eFunc = (KeyFuncType)nKeyFunc;
        if ( eFunc != KEYFUNC_DONTKNOW )
        {
            USHORT nDummy;
            ImplGetKeyCode( eFunc, nCode, nDummy, nDummy );
        }
        else
            nCode = nKeyCode | nModifier;
    }
}

// -----------------------------------------------------------------------

XubString KeyCode::GetName( Window* pWindow ) const
{
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
#ifndef REMOTE_APPSERVER
    return pWindow->ImplGetFrame()->GetKeyName( GetFullCode() );
#else
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->mpKeyNames )
    {
        pSVData->mpKeyNames = new KeyNames;
        NMSP_CLIENT::KeyNameSequence aKeyNames;
        pWindow->ImplGetFrame()->GetKeyNames( aKeyNames );
        ULONG nNames = aKeyNames.getLength();
        for ( USHORT n = 0; n < nNames; n++ )
        {
            const NMSP_CLIENT::IDLKeyNameInfo& rInfo = aKeyNames.getConstArray()[n];
            pSVData->mpKeyNames->Insert( rInfo.nCode, new String( rInfo.aName )  );
        }
    }

    String      aName;
    const int   nMods = 3;
    USHORT      nKeyCode = GetFullCode();
    USHORT      aModifiers[nMods] = { KEY_MOD2, KEY_MOD1, KEY_SHIFT };
    for ( USHORT n = 0; n < nMods; n++ )
    {
        USHORT nMod = aModifiers[n];
        if ( nKeyCode & nMod )
        {
            String* pMod = pSVData->mpKeyNames->Get( nMod );
            if ( pMod )
            {
                aName += *pMod;
                aName += '+';
            }
        }
    }

    USHORT nCode = GetCode();
    if ( (nCode >= KEY_0) && (nCode <= KEY_9) )
        aName += (sal_Unicode)'0'+(nCode-KEY_0);
    else if ( (nCode >= KEY_A) && (nCode <= KEY_Z) )
        aName += (sal_Unicode)'A'+(nCode-KEY_A);
    else if ( (nCode >= KEY_F1) && (nCode <= KEY_F9) )
    {
        aName += (sal_Unicode)'F';
        aName += (sal_Unicode)'1' + (nCode-KEY_F1);
    }
    else if ( (nCode >= KEY_F10) && (nCode <= KEY_F19) )
    {
        aName += (sal_Unicode)'F';
        aName += (sal_Unicode)'1';
        aName += (sal_Unicode)'0' + (nCode-KEY_F10);
    }
    else if ( (nCode >= KEY_F20) && (nCode <= KEY_F26) )
    {
        aName += (sal_Unicode)'F';
        aName += (sal_Unicode)'2';
        aName += (sal_Unicode)'0' + (nCode-KEY_F20);
    }
    else
    {
        String* pName = pSVData->mpKeyNames->Get( GetFullCode() );
        if ( pName )
            aName += *pName;
    }
    return aName;
#endif
}

// -----------------------------------------------------------------------

XubString KeyCode::GetSymbolName( const XubString& rFontName, Window* pWindow ) const
{
#ifndef REMOTE_APPSERVER
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    return pWindow->ImplGetFrame()->GetSymbolKeyName( rFontName, GetFullCode() );
#else
    return GetName( pWindow );
#endif
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
