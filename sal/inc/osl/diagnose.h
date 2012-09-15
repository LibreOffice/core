/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifndef _OSL_DIAGNOSE_H_
#define _OSL_DIAGNOSE_H_

#include "sal/config.h"

#include "sal/detail/log.h"
#include "sal/saldllapi.h"
#include "sal/types.h"

/** provides simple diagnostic support

    The facilities provided by this header are deprecated.  True assertions
    (that detect broken program logic) should use standard assert (which aborts
    if an assertion fails, and is controlled by the standard NDEBUG macro).
    Logging of warnings (e.g., about malformed input) and traces (e.g., about
    steps taken while executing some protocol) should use the facilities
    provided by (C++ only) sal/log.hxx.

    Because the assertion macros (OSL_ASSERT, OSL_ENSURE, OSL_FAIL, OSL_PRECOND,
    and OSL_POSTCOND) have been used for true assertions as well as for logged
    warnings, they map to SAL_WARN instead of standard assert.  OSL_TRACE maps
    to SAL_INFO.

    The functions defined in this header are not intended to be used directly,
    but through defined macros. The macros can be divided into three categories:
    assertions, traces and other stuff .-) Their usability depends on the value
    of OSL_DEBUG_LEVEL macro: assertions are only active if OSL_DEBUG_LEVEL is 1
    or greater, traces if OSL_DEBUG_LEVEL is 2 or greater.

    Assertions (cond is bool, msg is char*):
    OSL_ASSERT(cond)
        If cond is false, reports an error.

    OSL_ENSURE(cond, msg)
        If cond is false, reports an error with message msg.

    OSL_FAIL(msg)
        Reports an error with message msg unconditionally.

    OSL_PRECOND(cond, msg)
    OSL_POSTCOND(cond, msg)
        These two are functionally equivalent to OSL_ENSURE(cond, msg). They are
        intended to be used for checking pre- and postconditions of functions.

    Traces:
    OSL_TRACE(fmt, args...)
        Prints trace message. The arguments have the same meaning as the
        arguments of printf.

    Other:
    OSL_VERIFY(expr)
        Evaluates the expression and if it is false, reports an error. The
        expression is evaluated once without regard of the value of
        OSL_DEBUG_LEVEL.

        Example:

        void extractBool(Any const& rAny, bool& rBool)
        {
            OSL_VERIFY(rAny >>= rBool);
        }

    OSL_DEBUG_ONLY(expr)
 */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/* //////////////////////////////////////////////////////////////////////////
    Diagnostic support
*/

SAL_DLLPUBLIC void        SAL_CALL osl_breakDebug(void);
SAL_DLLPUBLIC sal_Bool    SAL_CALL osl_assertFailedLine(const sal_Char* pszFileName, sal_Int32 nLine, const sal_Char* pszMessage);
SAL_DLLPUBLIC void        SAL_CALL osl_trace(const sal_Char* pszFormat, ...);
SAL_DLLPUBLIC sal_Int32   SAL_CALL osl_reportError(sal_uInt32 nType, const sal_Char* pszErrorMessage);

/*
    For message delivery
*/

/** a message delivery function which receives a pre-formatted message string
*/
typedef void (SAL_CALL *pfunc_osl_printDebugMessage)( const sal_Char * pszMessage );

/** a message delivery function which receives detailed information about where the message was triggered
*/
typedef void (SAL_CALL *pfunc_osl_printDetailedDebugMessage)( const sal_Char * pszFileName, sal_Int32 nLine, const sal_Char* pszMessage );

/** sets a message delivery function

    The function set here is ignored if a function for detailed message information
    (pfunc_osl_printDetailedDebugMessage) has been set.

    The given message handler must be able to cope with a null message.
*/
SAL_DLLPUBLIC pfunc_osl_printDebugMessage SAL_CALL osl_setDebugMessageFunc( pfunc_osl_printDebugMessage pNewFunc );

/** sets a delivery function for detailed message information.

    The given message handler must be able to cope with a null message.
*/
SAL_DLLPUBLIC pfunc_osl_printDetailedDebugMessage SAL_CALL osl_setDetailedDebugMessageFunc( pfunc_osl_printDetailedDebugMessage pNewFunc );

#ifdef __cplusplus
}
#endif

#define OSL_THIS_FILE       __FILE__

/* the macro OSL_LOG_PREFIX is intended to be an office internal macro for now

   it is deprecated and superseded by (C++ only) SAL_WHERE
*/
#define OSL_LOG_PREFIX SAL_DETAIL_WHERE

#define OSL_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)

#define OSL_TRACE(...) \
    SAL_DETAIL_INFO_IF_FORMAT(OSL_DEBUG_LEVEL > 0, "legacy.osl", __VA_ARGS__)

#if OSL_DEBUG_LEVEL > 0
#define OSL_ASSERT(c) \
    SAL_DETAIL_WARN_IF_FORMAT(!(c), "legacy.osl", "OSL_ASSERT: %s", #c)
#define OSL_ENSURE(c, m) SAL_DETAIL_WARN_IF_FORMAT(!(c), "legacy.osl", "%s", m)
#define OSL_FAIL(m) SAL_DETAIL_WARN_IF_FORMAT(sal_True, "legacy.osl", "%s", m)
#else
#define OSL_ASSERT(c) ((void) 0)
#define OSL_ENSURE(c, m) ((void) 0)
#define OSL_FAIL(m) ((void) 0)
#endif

#define OSL_VERIFY(c) do { if (!(c)) OSL_ASSERT(0); } while (0)
#define OSL_PRECOND(c, m)   OSL_ENSURE(c, m)
#define OSL_POSTCOND(c, m)  OSL_ENSURE(c, m)


#ifdef __cplusplus
#define _OSL_GLOBAL ::
#else
#define _OSL_GLOBAL
#endif  /* __cplusplus */

#if OSL_DEBUG_LEVEL > 0

#define _OSL_DEBUG_ONLY(f)  (f)

#else

#define _OSL_DEBUG_ONLY(f)          ((void)0)

#endif /* OSL_DEBUG_LEVEL */

/* the macro OSL_THIS_FUNC is intended to be an office internal macro for now */
/* copied from boost/current_function.hpp to make it usable from C
 * sources as well
 *
 * Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt) */
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600))
#define OSL_THIS_FUNC __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define OSL_THIS_FUNC __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define OSL_THIS_FUNC __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define OSL_THIS_FUNC __FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define OSL_THIS_FUNC __func__
#else
#define OSL_THIS_FUNC ""
#endif

#endif /* _OSL_DIAGNOSE_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
