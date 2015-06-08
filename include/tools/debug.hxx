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
#ifndef INCLUDED_TOOLS_DEBUG_HXX
#define INCLUDED_TOOLS_DEBUG_HXX

#include <tools/toolsdllapi.h>

#include <sal/detail/log.h>
#include <sal/types.h>

/** The facilities provided by this header are deprecated.  True assertions
    (that detect broken program logic) should use standard assert (which aborts
    if an assertion fails, and is controlled by the standard NDEBUG macro).
    Logging of warnings (e.g., about malformed input) should use the facilities
    provided by sal/log.hxx.

    Because the assertion macro (DBG_ASSERT) has been used for
    true assertions as well as to log warnings, it maps to SAL_WARN instead of
    standard assert.  The warning and error macros (DBG_ASSERTWARNING,
    DBG_WARNING) all map to
    SAL_INFO.
*/

#ifdef DBG_UTIL

typedef void (*DbgTestSolarMutexProc)();

#define DBG_TEST_RESOURCE           (0x02000000)
#define DBG_TEST_DIALOG             (0x04000000)
#define DBG_TEST_BOLDAPPFONT        (0x08000000)

struct DbgData
{
    sal_uIntPtr       nTestFlags;
    sal_Char    aDbgWinState[50];           // DbgGUIData for VCL
};

// Dbg prototypes
#define DBG_FUNC_GETDATA            0
#define DBG_FUNC_SAVEDATA           1
#define DBG_FUNC_SETTESTSOLARMUTEX  2
#define DBG_FUNC_TESTSOLARMUTEX     3

TOOLS_DLLPUBLIC void* DbgFunc( sal_uInt16 nAction, void* pData = NULL );

inline DbgData* DbgGetData()
{
    return static_cast<DbgData*>(DbgFunc( DBG_FUNC_GETDATA ));
}

inline void DbgSaveData( const DbgData& rData )
{
    DbgFunc( DBG_FUNC_SAVEDATA, const_cast<DbgData *>(&rData) );
}

inline bool DbgIsResource()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_RESOURCE;
    else
        return false;
}

inline bool DbgIsDialog()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_DIALOG;
    else
        return false;
}

inline bool DbgIsBoldAppFont()
{
    DbgData* pData = DbgGetData();
    if ( pData )
        return pData->nTestFlags & DBG_TEST_BOLDAPPFONT;
    else
        return false;
}

inline void DbgSetTestSolarMutex( DbgTestSolarMutexProc pProc )
{
    DbgFunc( DBG_FUNC_SETTESTSOLARMUTEX, reinterpret_cast<void*>(reinterpret_cast<sal_uIntPtr>(pProc)) );
}

#define DBG_ASSERTWARNING( sCon, aWarning ) \
    SAL_DETAIL_INFO_IF_FORMAT(!(sCon), "legacy.tools", "%s", aWarning)

#define DBG_ASSERT( sCon, aError ) \
    SAL_DETAIL_WARN_IF_FORMAT(!(sCon), "legacy.tools", "%s", aError)

#define DBG_WARNING( aWarning ) \
    SAL_DETAIL_INFO_IF_FORMAT(true, "legacy.tools", "%s", aWarning)

#define DBG_TESTSOLARMUTEX()                \
do                                          \
{                                           \
    DbgFunc(DBG_FUNC_TESTSOLARMUTEX);       \
} while(false)

#else
// NO DBG_UITL

#define DBG_ASSERTWARNING( sCon, aWarning ) ((void)0)
#define DBG_ASSERT( sCon, aError ) ((void)0)
#define DBG_WARNING( aWarning ) ((void)0)

#define DBG_TESTSOLARMUTEX() ((void)0)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
