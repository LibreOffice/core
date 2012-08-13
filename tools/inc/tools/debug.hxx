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
#ifndef _TOOLS_DEBUG_HXX
#define _TOOLS_DEBUG_HXX

#include "tools/toolsdllapi.h"

#include <sal/detail/log.h>
#include <sal/types.h>
#include <tools/solar.h>

/** The facilities provided by this header are deprecated.  True assertions
    (that detect broken program logic) should use standard assert (which aborts
    if an assertion fails, and is controlled by the standard NDEBUG macro).
    Logging of warnings (e.g., about malformed input) and traces (e.g., about
    steps taken while executing some protocol) should use the facilities
    provided by sal/log.hxx.

    Because the assertion macros (DBG_ASSERT, DBG_BF_ASSERT) have been used for
    true assertions as well as to log warnings, they map to SAL_WARN instead of
    standard assert.  The warning and error macros (DBG_ASSERTWARNING,
    DBG_WARNING, DBG_WARNING1, ..., DBG_WARNING3, DBG_ERRORFILE) all map to
    SAL_INFO.
*/

#ifdef DBG_UTIL

typedef void (*DbgPrintLine)( const sal_Char* pLine );
typedef const sal_Char* (*DbgUsr)(const void* pThis );
typedef void (*DbgTestSolarMutexProc)();

#define DBG_BUF_MAXLEN              16384

#define DBG_TEST_XTOR               (0x00000FFF)
#define DBG_TEST_XTOR_THIS          (0x00000001)
#define DBG_TEST_XTOR_FUNC          (0x00000002)
#define DBG_TEST_XTOR_EXIT          (0x00000004)
#define DBG_TEST_XTOR_REPORT        (0x00000008)
#define DBG_TEST_XTOR_TRACE         (0x00000010)

#define DBG_TEST_PROFILING          (0x01000000)
#define DBG_TEST_RESOURCE           (0x02000000)
#define DBG_TEST_DIALOG             (0x04000000)
#define DBG_TEST_BOLDAPPFONT        (0x08000000)

#define DBG_OUT_NULL                0
#define DBG_OUT_FILE                1
#define DBG_OUT_WINDOW              2
#define DBG_OUT_SHELL               3
#define DBG_OUT_MSGBOX              4
#define DBG_OUT_TESTTOOL            5
#define DBG_OUT_DEBUGGER            6
#define DBG_OUT_ABORT               7

#define DBG_OUT_COUNT               8

// user (runtime) defined output channels
#define DBG_OUT_USER_CHANNEL_0    100

struct DbgData
{
    sal_uIntPtr       nTestFlags;
    sal_uIntPtr       bOverwrite;
    sal_uIntPtr       nTraceOut;
    sal_uIntPtr       nWarningOut;
    sal_uIntPtr       nErrorOut;
    sal_uIntPtr       bHookOSLAssert;
    sal_Char    aDebugName[260];
    sal_Char    aInclFilter[512];
    sal_Char    aExclFilter[512];
    sal_Char    aInclClassFilter[512];
    sal_Char    aExclClassFilter[512];
    sal_Char    aDbgWinState[50];           // DbgGUIData for VCL
};

struct DbgDataType
{
    void*       pData;
    sal_Char const *   pName;
};

// Dbg prototypes
#define DBG_FUNC_DEBUGSTART         1
#define DBG_FUNC_DEBUGEND           2
#define DBG_FUNC_GLOBALDEBUGEND     3
#define DBG_FUNC_GETDATA            4
#define DBG_FUNC_SAVEDATA           5
#define DBG_FUNC_SETPRINTMSGBOX     6
#define DBG_FUNC_SETPRINTWINDOW     7
#define DBG_FUNC_SETPRINTTESTTOOL   8
#define DBG_FUNC_XTORINFO           10
#define DBG_FUNC_COREDUMP           12
#define DBG_FUNC_ALLERROROUT        13
#define DBG_FUNC_SETTESTSOLARMUTEX  14
#define DBG_FUNC_TESTSOLARMUTEX     15
#define DBG_FUNC_PRINTFILE          16
#define DBG_FUNC_GETPRINTMSGBOX     17
#define DBG_FUNC_FILTERMESSAGE      18          // new for #i38967
#define DBG_FUNC_UPDATEOSLHOOK      19
#define DBG_FUNC_SET_ABORT          20

TOOLS_DLLPUBLIC void* DbgFunc( sal_uInt16 nAction, void* pData = NULL );

inline void DbgUpdateOslHook( DbgData* pData )
{
    DbgFunc( DBG_FUNC_UPDATEOSLHOOK, pData );
}

inline void DbgDebugStart()
{
    DbgFunc( DBG_FUNC_DEBUGSTART );
}

inline void DbgDebugEnd()
{
    DbgFunc( DBG_FUNC_DEBUGEND );
}

inline void DbgGlobalDebugEnd()
{
    DbgFunc( DBG_FUNC_GLOBALDEBUGEND );
}

inline void DbgSetPrintMsgBox( DbgPrintLine pProc )
{
    DbgFunc( DBG_FUNC_SETPRINTMSGBOX, (void*)(long)pProc );
}

inline DbgPrintLine DbgGetPrintMsgBox()
{
    return (DbgPrintLine)(long)DbgFunc( DBG_FUNC_GETPRINTMSGBOX );
}

inline void DbgSetPrintWindow( DbgPrintLine pProc )
{
    DbgFunc( DBG_FUNC_SETPRINTWINDOW, (void*)(long)pProc );
}

inline void DbgSetPrintTestTool( DbgPrintLine pProc )
{
    DbgFunc( DBG_FUNC_SETPRINTTESTTOOL, (void*)(long)pProc );
}

inline void DbgSetAbort( DbgPrintLine pProc )
{
    DbgFunc( DBG_FUNC_SET_ABORT, (void*)(long)pProc );
}

typedef sal_uInt16 DbgChannelId;

/** registers a user-defined channel for emitting the diagnostic messages

    Note that such a user-defined channel cannot be revoked during the lifetime
    of the process. Thus, it's the caller's responsibility to ensure that the
    procedure to which ->pProc points remains valid.

    @param pProc
        the function for emitting the diagnostic messages
    @return
        a unique number for this channel, which can be used for ->DbgData::nErrorOut,
        ->DbgData::nWarningOut and ->DbgData::nTraceOut
    @see DBG_OUT_USER_CHANNEL_0

    (In theory, this function could replace the other hard-coded channels. Well, at least
    the ones for MsgBox, Window, Shell, TestTool. Perhaps in the next life ...)
*/
TOOLS_DLLPUBLIC DbgChannelId DbgRegisterUserChannel( DbgPrintLine pProc );

inline sal_Bool DbgFilterMessage( const char* pMsg )
{
    return (sal_Bool)(long) DbgFunc( DBG_FUNC_FILTERMESSAGE, (void*)pMsg );
}

inline int DbgIsAllErrorOut()
{
    return (DbgFunc( DBG_FUNC_ALLERROROUT ) != 0);
}

inline DbgData* DbgGetData()
{
    return (DbgData*)DbgFunc( DBG_FUNC_GETDATA );
}

inline void DbgSaveData( const DbgData& rData )
{
    DbgFunc( DBG_FUNC_SAVEDATA, (void*)&rData );
}

inline sal_uIntPtr DbgIsTraceOut()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return (pData->nTraceOut != DBG_OUT_NULL);
    else
        return sal_False;
}

inline sal_uIntPtr DbgIsWarningOut()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return (pData->nWarningOut != DBG_OUT_NULL);
    else
        return sal_False;
}

inline sal_uIntPtr DbgIsErrorOut()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return (pData->nErrorOut != DBG_OUT_NULL);
    else
        return sal_False;
}

inline sal_uIntPtr DbgGetErrorOut()   // Testtool: test whether to collect OSL_ASSERTions as well
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nErrorOut;
    else
        return DBG_OUT_NULL;
}

inline sal_uIntPtr DbgIsAssertWarning()
{
    return DbgIsWarningOut();
}

inline sal_uIntPtr DbgIsAssert()
{
    return DbgIsErrorOut();
}

inline sal_uIntPtr DbgIsResource()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_RESOURCE;
    else
        return sal_False;
}

inline sal_uIntPtr DbgIsDialog()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_DIALOG;
    else
        return sal_False;
}

inline sal_uIntPtr DbgIsBoldAppFont()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_BOLDAPPFONT;
    else
        return sal_False;
}

inline void DbgXtorInfo( sal_Char* pBuf )
{
    DbgFunc( DBG_FUNC_XTORINFO, (void*)pBuf );
}

inline void DbgCoreDump()
{
    DbgFunc( DBG_FUNC_COREDUMP );
}

inline void DbgSetTestSolarMutex( DbgTestSolarMutexProc pProc )
{
    DbgFunc( DBG_FUNC_SETTESTSOLARMUTEX, (void*)(long)pProc );
}

inline void DbgTestSolarMutex()
{
    DbgFunc( DBG_FUNC_TESTSOLARMUTEX );
}

inline void DbgPrintFile( const sal_Char* pLine )
{
    DbgFunc( DBG_FUNC_PRINTFILE, (void*)(sal_Char*)pLine );
}

// Dbg output
#define DBG_OUT_TRACE               1
#define DBG_OUT_WARNING             2
#define DBG_OUT_ERROR               3

TOOLS_DLLPUBLIC void DbgOut( const sal_Char* pMsg, sal_uInt16 nOutType = DBG_OUT_TRACE,
             const sal_Char* pFile = NULL, sal_uInt16 nLine = 0 );
TOOLS_DLLPUBLIC void DbgPrintShell(char const * message);
TOOLS_DLLPUBLIC void DbgOutTypef( sal_uInt16 nOutType, const sal_Char* pFStr, ... );
TOOLS_DLLPUBLIC void DbgOutf( const sal_Char* pFStr, ... );
TOOLS_DLLPUBLIC void ImpDbgOutfBuf( sal_Char* pBuf, const sal_Char* pFStr, ... );

// Dbg test functions

#define DBG_PROF_START              1
#define DBG_PROF_STOP               2
#define DBG_PROF_CONTINUE           3
#define DBG_PROF_PAUSE              4

TOOLS_DLLPUBLIC void DbgProf( sal_uInt16 nAction, DbgDataType* );

#define DBG_XTOR_CTOR               1
#define DBG_XTOR_DTOR               2
#define DBG_XTOR_CHKTHIS            3
#define DBG_XTOR_CHKOBJ             4
#define DBG_XTOR_DTOROBJ            0x8000

TOOLS_DLLPUBLIC void DbgXtor( DbgDataType* pDbgData,
              sal_uInt16 nAction, const void* pThis, DbgUsr fDbgUsr );

class DbgXtorObj
{
private:
    DbgDataType*    pDbgData;
    const void*     pThis;
    DbgUsr          fDbgUsr;
    sal_uInt16          nAction;

public:
                    DbgXtorObj( DbgDataType* pData,
                                sal_uInt16 nAct, const void* pThs, DbgUsr fUsr )
                    {
                        DbgXtor( pData, nAct, pThs, fUsr );
                        pDbgData = pData;
                        nAction  = nAct;
                        pThis    = pThs;
                        fDbgUsr  = fUsr;
                    }

                    ~DbgXtorObj()
                    {
                        DbgXtor( pDbgData, nAction | DBG_XTOR_DTOROBJ,
                                 pThis, fDbgUsr );
                    }
};

// (internally used) defines

#define DBG_FUNC( aName )                   DbgName_##aName()
#define DBG_NAME( aName )                   static DbgDataType aImpDbgData_##aName = { 0, #aName }; \
                                            DbgDataType* DBG_FUNC( aName ) { return &aImpDbgData_##aName; }
#define DBG_NAMEEX_VISIBILITY( aName, vis ) vis DbgDataType* DBG_FUNC( aName );
#define DBG_NAMEEX( aName )                 DBG_NAMEEX_VISIBILITY( aName, )

// (externally used) defines

#define DBG_DEBUGSTART()                    DbgDebugStart()
#define DBG_DEBUGEND()                      DbgDebugEnd()
#define DBG_GLOBALDEBUGEND()                DbgGlobalDebugEnd()

#define DBG_PROFSTART( aName )                      \
    DbgProf( DBG_PROF_START, DBG_FUNC( aName ) )

#define DBG_PROFSTOP( aName )                       \
    DbgProf( DBG_PROF_STOP, DBG_FUNC( aName ) )

#define DBG_PROFCONTINUE( aName )                   \
    DbgProf( DBG_PROF_CONTINUE, DBG_FUNC( aName ) )

#define DBG_PROFPAUSE( aName )                      \
    DbgProf( DBG_PROF_PAUSE, DBG_FUNC( aName ) )

#define DBG_CTOR( aName, fTest )                    \
    DbgXtorObj aDbgXtorObj( DBG_FUNC( aName ),      \
                            DBG_XTOR_CTOR,          \
                            (const void*)this,      \
                            fTest )

#define DBG_DTOR( aName, fTest )                    \
    DbgXtorObj aDbgXtorObj( DBG_FUNC( aName ),      \
                            DBG_XTOR_DTOR,          \
                            (const void*)this,      \
                            fTest )

#define DBG_CHKTHIS( aName, fTest )                 \
    DbgXtorObj aDbgXtorObj( DBG_FUNC( aName ),      \
                            DBG_XTOR_CHKTHIS,       \
                            (const void*)this,      \
                            fTest )

#define DBG_CHKOBJ( pObj, aName, fTest )            \
    DbgXtor( DBG_FUNC( aName ), DBG_XTOR_CHKOBJ,    \
             (const void*)pObj, (DbgUsr)fTest )

#define DBG_ASSERTWARNING( sCon, aWarning ) \
    SAL_DETAIL_INFO_IF_FORMAT(!(sCon), "legacy.tools", aWarning)

#define DBG_ASSERT( sCon, aError ) \
    SAL_DETAIL_WARN_IF_FORMAT(!(sCon), "legacy.tools", aError)

#ifdef DBG_BINFILTER
#define DBG_BF_ASSERT( sCon, aError ) \
    SAL_DETAIL_WARN_IF_FORMAT(!(sCon), "legacy.binfilter", aError)
#else
#define DBG_BF_ASSERT( sCon, aError ) ((void)0)
#endif

#define DBG_WARNING( aWarning ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", aWarning)
#define DBG_WARNING1( aWarning, x1 ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", aWarning, x1)
#define DBG_WARNING2( aWarning, x1, x2 ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", aWarning, x1, x2)
#define DBG_WARNING3( aWarning, x1, x2, x3 ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", aWarning, x1, x2, x3)

#define DBG_ERRORFILE( aError ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", aError, __FILE__, __LINE__)

#define DBG_TESTSOLARMUTEX()                \
do                                          \
{                                           \
    DbgTestSolarMutex();                    \
} while(0)

// en-/disable debug defines

#define DBG_INSTOUTTRACE( nOut )            \
do                                          \
{                                           \
    DbgGetData()->nTraceOut = nOut;         \
} while(0)

#define DBG_INSTOUTWARNING( nOut )          \
do                                          \
{                                           \
    DbgGetData()->nWarningOut = nOut;       \
} while(0)

#define DBG_INSTOUTERROR( nOut )            \
do                                          \
{                                           \
    DbgGetData()->nErrorOut = nOut;         \
} while(0)

#else
// NO DBG_UITL

struct DbgData;
struct DbgDataType;

typedef void (*DbgPrintLine)( const sal_Char* pLine );
typedef const sal_Char* (*DbgUsr)(const void* pThis );

#define DBG_DEBUGSTART() ((void)0)
#define DBG_DEBUGEND() ((void)0)
#define DBG_GLOBALDEBUGEND() ((void)0)

#define DBG_NAME( aName )
#define DBG_NAMEEX( aName )
#define DBG_NAMEEX_VISIBILITY( aName, vis )

#define DBG_PROFSTART( aName ) ((void)0)
#define DBG_PROFSTOP( aName ) ((void)0)
#define DBG_PROFCONTINUE( aName ) ((void)0)
#define DBG_PROFPAUSE( aName ) ((void)0)

#define DBG_CTOR( aName, fTest ) ((void)0)
#define DBG_DTOR( aName, fTest ) ((void)0)
#define DBG_CHKTHIS( aName, fTest ) ((void)0)
#define DBG_CHKOBJ( pObj, aName, fTest ) ((void)0)

#define DBG_ASSERTWARNING( sCon, aWarning ) ((void)0)
#define DBG_ASSERT( sCon, aError ) ((void)0)
#define DBG_BF_ASSERT( sCon, aError ) ((void)0)
#define DBG_WARNING( aWarning ) ((void)0)
#define DBG_WARNING1( aWarning, x1 ) ((void)0)
#define DBG_WARNING2( aWarning, x1, x2 ) ((void)0)
#define DBG_WARNING3( aWarning, x1, x2, x3 ) ((void)0)
#define DBG_ERRORFILE( aError ) ((void)0)

#define DBG_TESTSOLARMUTEX() ((void)0)

#define DBG_INSTOUTTRACE( nOut ) ((void)0)
#define DBG_INSTOUTWARNING( nOut ) ((void)0)
#define DBG_INSTOUTERROR( nOut ) ((void)0)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
