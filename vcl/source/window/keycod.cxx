/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <salinst.hxx>
#include <salframe.hxx>
#include <svdata.hxx>

#include <vcl/window.hxx>
#include <vcl/keycod.hxx>

#include <tools/rc.h>



static const sal_uInt16 aImplKeyFuncTab[(KEYFUNC_FRONT+1)*4] =
{
    0, 0, 0, 0,                                                    
    KEY_N | KEY_MOD1, 0, 0, 0,                                     
    KEY_O | KEY_MOD1, KEY_OPEN, 0, 0,                              
    KEY_S | KEY_MOD1, 0, 0, 0,                                     
    KEY_S | KEY_SHIFT | KEY_MOD1, 0, 0, 0,                         
    KEY_P | KEY_MOD1, 0, 0, 0,                                     
    KEY_W | KEY_MOD1, KEY_F4 | KEY_MOD1, 0, 0,                     
    KEY_Q | KEY_MOD1, KEY_F4 | KEY_MOD2, 0, 0,                     
    KEY_X | KEY_MOD1, KEY_DELETE | KEY_SHIFT, KEY_CUT, 0,          
    KEY_C | KEY_MOD1, KEY_INSERT | KEY_MOD1, KEY_COPY, 0,          
    KEY_V | KEY_MOD1, KEY_INSERT | KEY_SHIFT, KEY_PASTE, 0,        
    KEY_Z | KEY_MOD1, KEY_BACKSPACE | KEY_MOD2, KEY_UNDO, 0,       
    KEY_Y | KEY_MOD1, KEY_UNDO | KEY_SHIFT, 0, 0,                  
    KEY_DELETE, 0, 0, 0,                                           
    KEY_REPEAT, 0, 0, 0,                                           
    KEY_F | KEY_MOD1, KEY_FIND, 0, 0,                              
    KEY_F | KEY_SHIFT | KEY_MOD1, KEY_SHIFT | KEY_FIND, 0, 0,      
    KEY_RETURN | KEY_MOD2, 0, 0, 0,                                
    0, 0, 0, 0                                                     
};



void ImplGetKeyCode( KeyFuncType eFunc, sal_uInt16& rCode1, sal_uInt16& rCode2, sal_uInt16& rCode3, sal_uInt16& rCode4 )
{
    sal_uInt16 nIndex = (sal_uInt16)eFunc;
    nIndex *= 4;
    rCode1 = aImplKeyFuncTab[nIndex];
    rCode2 = aImplKeyFuncTab[nIndex+1];
    rCode3 = aImplKeyFuncTab[nIndex+2];
        rCode4 = aImplKeyFuncTab[nIndex+3];
}



KeyCode::KeyCode( KeyFuncType eFunction )
{
    sal_uInt16 nDummy;
    ImplGetKeyCode( eFunction, nCode, nDummy, nDummy, nDummy );
    eFunc = eFunction;
}



KeyCode::KeyCode( const ResId& rResId )
{
    rResId.SetRT( RSC_KEYCODE );

    ResMgr* pResMgr = rResId.GetResMgr();
    if ( pResMgr && pResMgr->GetResource( rResId ) )
    {
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        sal_uLong nKeyCode  = pResMgr->ReadLong();
        sal_uLong nModifier = pResMgr->ReadLong();
        sal_uLong nKeyFunc  = pResMgr->ReadLong();

        eFunc = (KeyFuncType)nKeyFunc;
        if ( eFunc != KEYFUNC_DONTKNOW )
        {
            sal_uInt16 nDummy;
            ImplGetKeyCode( eFunc, nCode, nDummy, nDummy, nDummy );
        }
        else
            nCode = sal::static_int_cast<sal_uInt16>(nKeyCode | nModifier);
    }
}



OUString KeyCode::GetName( Window* pWindow ) const
{
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    return pWindow ? pWindow->ImplGetFrame()->GetKeyName( GetFullCode() ) : "";
}



KeyFuncType KeyCode::GetFunction() const
{
    if ( eFunc != KEYFUNC_DONTKNOW )
        return eFunc;

    sal_uInt16 nCompCode = GetModifier() | GetCode();
    if ( nCompCode )
    {
        for ( sal_uInt16 i = (sal_uInt16)KEYFUNC_NEW; i < (sal_uInt16)KEYFUNC_FRONT; i++ )
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

    return KEYFUNC_DONTKNOW;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
