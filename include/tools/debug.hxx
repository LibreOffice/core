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

/** The facilities provided by this header are deprecated.  True assertions
    (that detect broken program logic) should use standard assert (which aborts
    if an assertion fails, and is controlled by the standard NDEBUG macro).
    Logging of warnings (e.g., about malformed input) should use the facilities
    provided by sal/log.hxx.

    Because the assertion macro (DBG_ASSERT) has been used for
    true assertions as well as to log warnings, it maps to SAL_WARN instead of
    standard assert.
*/

typedef void (*DbgTestSolarMutexProc)(bool);

TOOLS_DLLPUBLIC void DbgSetTestSolarMutex( DbgTestSolarMutexProc pParam );
TOOLS_DLLPUBLIC void DbgTestSolarMutex(bool owned);

#ifndef NDEBUG
// we want the solar mutex checking to be enabled in the assert-enabled builds that the QA people use

#define DBG_TESTSOLARMUTEX()   \
do                             \
{                              \
    DbgTestSolarMutex(true);   \
} while(false)

#define DBG_TESTNOTSOLARMUTEX() \
do                             \
{                              \
    DbgTestSolarMutex(false);  \
} while(false)

#else

#define DBG_TESTSOLARMUTEX() ((void)0)
#define DBG_TESTNOTSOLARMUTEX() ((void)0)

#endif

#define DBG_ASSERT( sCon, aError ) \
    SAL_DETAIL_WARN_IF_FORMAT(!(sCon), "legacy.tools", "%s", aError)


#endif // INCLUDED_TOOLS_DEBUG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
