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
#ifndef _SFX_SFXTYPES_HXX
#define _SFX_SFXTYPES_HXX

#include <tools/debug.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <tools/resid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

#include <osl/mutex.hxx>

#ifndef DELETEZ
#define DELETEZ(pPtr) ( delete pPtr, pPtr = 0 )
#endif

#ifndef DELETEX
#ifdef DBG_UTIL
#define DELETEX(pPtr) ( delete pPtr, (void*&) pPtr = (void*) 0xFFFFFFFF )
#else
#define DELETEX(pPtr) delete pPtr
#endif
#endif

//------------------------------------------------------------------------

// Macro for the Call-Profiler under WinNT
// with S_CAP a measurement can be started, and stopped with E_CAP
#if defined( WNT ) && defined( PROFILE )

extern "C" {
    void StartCAP();
    void StopCAP();
    void DumpCAP();
};

#define S_CAP()   StartCAP();
#define E_CAP()   StopCAP(); DumpCAP();

struct _Capper
{
    _Capper() { S_CAP(); }
    ~_Capper() { E_CAP(); }
};

#define CAP _Capper _aCap_

#else

#define S_CAP()
#define E_CAP()
#define CAP

#endif

#ifdef DBG_UTIL
#ifndef DBG
#define DBG(statement) statement
#endif
#define DBG_OUTF(x) DbgOutf x
#else
#ifndef DBG
#define DBG(statement)
#endif
#define DBG_OUTF(x)
#endif

//------------------------------------------------------------------------

#if defined(DBG_UTIL) && defined(WNT)

class SfxStack
{
    static unsigned nLevel;

public:
    SfxStack( const char *pName )
    {
        ++nLevel;
        DbgOutf( "STACK: enter %3d %s", nLevel, pName );
    }
    ~SfxStack()
    {
        DbgOutf( "STACK: leave %3d", nLevel );
        --nLevel;
    }
};

#define SFX_STACK(s) SfxStack aSfxStack_( #s )
#else
#define SFX_STACK(s)
#endif

//------------------------------------------------------------------------

String SearchAndReplace( const String &rSource,
                         const String &rToReplace,
                         const String &rReplacement );

#define SFX_PASSWORD_CODE "_:;*\x9A?()/&[&"
String SfxStringEncode( const String &rSource,
                        const char *pKey = SFX_PASSWORD_CODE );
String SfxStringDecode( const String &rSource,
                        const char *pKey = SFX_PASSWORD_CODE );


struct StringList_Impl : private Resource
{

    ResId aResId;

    StringList_Impl( const ResId& rErrIdP,  sal_uInt16 nId)
        : Resource( rErrIdP ),aResId(nId, *rErrIdP.GetResMgr()){}
    ~StringList_Impl() { FreeResource(); }

    String GetString(){ return aResId.toString(); }
    operator sal_Bool(){return IsAvailableRes(aResId.SetRT(RSC_STRING));}

};

#define GPF() *(int*)0 = 0

#endif // #ifndef _SFX_SFXTYPES_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
