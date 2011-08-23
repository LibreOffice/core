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
#ifndef _SFX_SFXTYPES_HXX
#define _SFX_SFXTYPES_HXX

#ifndef _RCID_H //autogen
#include <tools/rcid.h>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

class Timer;
namespace binfilter {

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

class SfxShell;

#if !defined(_SFX_APPCTOR_CXX) && defined(WNT)
#ifndef _DLL_
#error _DLL_ mal wieder nicht defined => GetpApp() == 0
#endif
#endif

//------------------------------------------------------------------------

// Macro fuer den Call-Profiler unter WinNT
// mit S_CAP kann eine Messung gestarted, mit E_CAP wieder gestoppt werden
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

#define TRIM(s) s.EraseLeadingChars().EraseTrailingChars()

//------------------------------------------------------------------------

String SfxShellIdent_Impl( const SfxShell *pSh );

//------------------------------------------------------------------------

#if defined(DBG_UTIL) && ( defined(WNT) || defined(OS2) )

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

#define SFX_PASSWORD_CODE "_:;*?()/&[&"
String SfxStringDecode( const String &rSource,
                        const char *pKey = SFX_PASSWORD_CODE );


#define SFX_DEL_PTRARR(pArr) 									\
            {                                                   \
                for ( USHORT n = (pArr)->Count(); n--; )        \
                    delete (pArr)->GetObject(n);                \
                DELETEX(pArr);                                  \
            }

class SfxBoolResetter
{
    BOOL&				_rVar;
    BOOL				_bOld;

public:
                        SfxBoolResetter( BOOL &rVar )
                        :	_rVar( rVar ),
                            _bOld( rVar )
                        {}

                        ~SfxBoolResetter()
                        { _rVar = _bOld; }
};

#define GPF() *(int*)0 = 0

}//end of namespace binfilter
#endif // #ifndef _SFX_SFXTYPES_HXX


