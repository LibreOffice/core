/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFX_SFXTYPES_HXX
#define _SFX_SFXTYPES_HXX

#include <tools/debug.hxx>
#ifndef _RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#ifndef _RCID_H //autogen
#include <tools/rcid.h>
#endif
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#include <tools/link.hxx>
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include <vcl/wrkwin.hxx>

#ifndef _VOS_MUTEX_HXX //autogen
#include <vos/mutex.hxx>
#endif

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
class Timer;

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

    String GetString(){ return String( aResId ); }
    operator sal_Bool(){return IsAvailableRes(aResId.SetRT(RSC_STRING));}

};

#define SFX_DEL_PTRARR(pArr)                                    \
            {                                                   \
                for ( sal_uInt16 n = (pArr)->Count(); n--; )        \
                    delete (pArr)->GetObject(n);                \
                DELETEX(pArr);                                  \
            }

class SfxBoolResetter
{
    sal_Bool&               _rVar;
    sal_Bool                _bOld;

public:
                        SfxBoolResetter( sal_Bool &rVar )
                        :   _rVar( rVar ),
                            _bOld( rVar )
                        {}

                        ~SfxBoolResetter()
                        { _rVar = _bOld; }
};

#define GPF() *(int*)0 = 0

#endif // #ifndef _SFX_SFXTYPES_HXX


