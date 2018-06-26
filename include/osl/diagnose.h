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


#ifndef INCLUDED_OSL_DIAGNOSE_H
#define INCLUDED_OSL_DIAGNOSE_H

#include "sal/config.h"

#include "sal/detail/log.h"
#include "sal/types.h"

/** @file
    Provides simple diagnostic support.

    @deprecated
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

    Traces:
    OSL_TRACE(fmt, args...)
        Prints trace message. The arguments have the same meaning as the
        arguments of printf.
 */

#if !defined OSL_DEBUG_LEVEL
#define OSL_DEBUG_LEVEL 0
#endif

/** @internal The macro OSL_LOG_PREFIX is intended to be an office internal macro for now
    @deprecated superseded by (C++ only) SAL_WHERE
*/
#define OSL_LOG_PREFIX SAL_DETAIL_WHERE

/** Prints trace message.

    The arguments have the same meaning as the arguments of printf.
*/
#define OSL_TRACE(...) \
    SAL_DETAIL_INFO_IF_FORMAT(OSL_DEBUG_LEVEL > 0, "legacy.osl", __VA_ARGS__)

/** @defgroup assert Assertions

    Assertions (cond is bool, msg is char*).

    @{
 */

/** If cond is false, reports an error. */
#define OSL_ASSERT(c) \
    SAL_DETAIL_WARN_IF_FORMAT(!(c), "legacy.osl", "OSL_ASSERT: %s", #c)
/** If cond is false, reports an error with message msg. */
#define OSL_ENSURE(c, m) SAL_DETAIL_WARN_IF_FORMAT(!(c), "legacy.osl", "%s", m)
/** Reports an error with message msg unconditionally. */
#define OSL_FAIL(m) SAL_DETAIL_WARN_IF_FORMAT(sal_True, "legacy.osl", "%s", m)

/** Evaluates the expression and if it is false, reports an error. The
    expression is evaluated once without regard of the value of
    OSL_DEBUG_LEVEL.

    Example:

    @code{.c}

    void extractBool(Any const& rAny, bool& rBool)
    {
        OSL_VERIFY(rAny >>= rBool);
    }

    @endcode
*/
#define OSL_VERIFY(c) do { if (!(c)) OSL_ASSERT(0); } while (0)

/** Check the precondition of functions.

    Functionally equivalent to OSL_ENSURE(cond, msg).
*/
#define OSL_PRECOND(c, m)   OSL_ENSURE(c, m)

/** Check the postcondition of functions.

    Functionally equivalent to OSL_ENSURE(cond, msg).
*/
#define OSL_POSTCOND(c, m)  OSL_ENSURE(c, m)

/** @} */

/* the macro OSL_THIS_FUNC is intended to be an office internal macro for now */
/* copied from boost/current_function.hpp to make it usable from C
 * sources as well
 *
 * Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt) */
#if defined(__GNUC__)
#define OSL_THIS_FUNC __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define OSL_THIS_FUNC __FUNCSIG__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define OSL_THIS_FUNC __func__
#else
#define OSL_THIS_FUNC ""
#endif

#endif // INCLUDED_OSL_DIAGNOSE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
