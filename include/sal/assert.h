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
#ifndef INCLUDED_SAL_ASSERT_H
#define INCLUDED_SAL_ASSERT_H

#include <sal/saldllapi.h>

/** Provides support for debug and release checks.

The short-term purpose these facilities is to replace the different
assertion methods that are littered in the Libreoffice codebase.
The different behavior of the assertion facilities adds unnecessary
complexity and is problematic.

There is no obvious way to know what to expect when a certain precondition
is broken (by new code, modified code, or by a new external input,) which
is vital for the developer who plans to work on an issue.
Furthermore, reading and modifying the code is made more
difficult by this unnecessary variations.

The following facilities are intended to improve the situation by
carefully designing an assertion API that is standard to the project,
cleaner in specs, and provides more support to the development team
than the current available breed.

3rdParty and CppUnit are out of scope.

The names are chosen to avoid conflicting with existing ones and
to make it easier to track the conversion.

Four types of assertions are provided:

1) Static assertion: Compile-time assertion that fails the build if
violated, otherwise compiles into nop in all configurations.
Enabled for all configurations.
LO_ASSERT_STC(EXP);

2) Permanent assertion: Runtime assertion that is never compiled out
of the code, even in releases (see below). These are very cheap, but
critical, checks. Typically trivial bounds or invariant checks.
The overhead of such an assertion should be <5% (typically 2%)
of the function they are in.
Enabled for all configurations.
LO_ASSERT_REL(EXP);

3) Debug assertion: Runtime assertion that validates pre- and
post-conditions, including some checks that require some nominal
amount of computation. These are the typical debug assertions and
should have an overhead of 5-20%.
Enabled for non-Release builds only.
LO_ASSERT(EXP);

4) Sanity/Validation assertion: Extensive runtime assertions that
validate an algorithm by executing extensive checks and recomputations.
These checks have a too high a cost to be enabled in a build used by
even the wider development community. They are best enabled when
debugging a certain algorithm or tracking down a particularly nasty
issue. The overhead of these assertions are >20%.
Enabled only when LO_USE_ASSERT_CHECKS is defined.
LO_ASSERT_DBG(EXP);

Each type comes in a version that takes an optional second argument
which is a custom message (other versions that take variable
arguments to print upon failure may also be provided).

Failed assertions will not take any action on their own, rather,
a handler function will be called. For a start, this handler can
be defined by configure, but a better approach is to wrap it in
a static library to avoid rebuilding the complete code-base when
a different strategy is necessary to track an issue. In the latter
case the static library is rebuilt when the assertion handler is
changed and only the binaries are linked.

There will be a number of standard handlers provided which can be
chosen at compile time.
Developers may implement their own handlers if necessary, but
these handlers are only for customized needs, and should not be
committed outside of draft/dev branches.
Custom handler that might serve a wider audience should be added
here, documented and standardized.

1) FailLog: Prints to stderr and log (optional).
    Ideally will include the stack-trace as well.
2) FailAbort: Calls FailLog then terminate,
    which might invoke CrashRep when functional.
3) FailBreak: Calls FailLog then breaks into the debugger,
    typically by issuing 'int 3' on x86/x64.
    This might not be useful on non-Windows platforms.
4) FailSpin: Calls FailLog then spins (with short sleeps,)
    to allow for hooking a debugger.
5) FailThrow: Calls FailLog then throws std::logic_error.
    This assumes there are handlers, or a debugger is set to
    break on C++ exceptions.
5) FailSys: Calls FailLog then invokes the system assert().
    This is useful in utilizing the runtime debug facilities.

When a proper CrashRep is implemented, FailLog will dump the stack-trace as well.
LO_ASSERT_REL would nominally FailLog and potentially FailAbort (to be decided).

Some ideas are influenced by Bloomberg's BSL library
(see https://github.com/bloomberg/bde/blob/master/groups/bsl/bsls/bsls_assert.h).
*/

/* SAL_CURRENT_FUNCTION
* Stollen from boost/current_function.hpp 1.57,
* which is not usable from C.
*
* Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
*
* Distributed under the Boost Software License, Version 1.0. (See
* accompanying file LICENSE_1_0.txt or copy at
* http://www.boost.org/LICENSE_1_0.txt)
*/
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
# define SAL_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
# define SAL_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
# define SAL_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
# define SAL_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
# define SAL_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
# define SAL_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
# define SAL_CURRENT_FUNCTION __func__
#else
# define SAL_CURRENT_FUNCTION "(unknown)"
#endif

#ifdef __cplusplus
#define LO_ASSERT_STC(...)         static_assert(__VA_ARGS__)
#else
/* TBD */
#define LO_ASSERT_STC(...)          ((void) 0)
#endif

#define LO_ASSERT_IS_ENABLED        (OSL_DEBUG_LEVEL > 0)
#define LO_ASSERT_DBG_IS_ENABLED    (OSL_DEBUG_LEVEL > 1)

/* TODO: Add custom failed-assertion handler. */

/*
* General failed-assertion handler.
* Formats and dispatches the assertion message.
*/
SAL_DLLPUBLIC void SAL_CALL
failed_assert_handler(char const * func,
                      char const * file,
                      const long line,
                      char const * expr,
                      ...);

/*
* Unconditional failed assertion.
* MSG is a char* which may be a format string and
* its arguments follow.
*/
#define LO_ASSERT_FAIL(MSG, ...)    failed_assert_handler( \
                                        SAL_CURRENT_FUNCTION, \
                                        __FILE__, __LINE__, \
                                        0, MSG, ##__VA_ARGS__, 0)

/*
* Unconditional failed assertion.
* EXP is *not* evaluated. It is only informational.
* The Second argument, if any, *must* be a char*, possibly
* an format string.
*/
#define LO_ASSERT_FAIL_EX(EXP, ...)    failed_assert_handler( \
                                        SAL_CURRENT_FUNCTION, \
                                        __FILE__, __LINE__, \
                                        #EXP, ##__VA_ARGS__, 0)

/*
* Always-enabled Assertion.
* Nigligible overhead (relative to function body).
* Enabled even in production code.
*/
#define LO_ASSERT_REL(EXP, ...)     ((!!(EXP)) ? ((void) 0) : \
                                        LO_ASSERT_FAIL(#EXP, ##__VA_ARGS__))

/*
* Debug-configuration assertion.
* Fails only when EXP evaluates to false.
*/
#if LO_ASSERT_IS_ENABLED
#define LO_ASSERT               LO_ASSERT_REL
#else
#define LO_ASSERT(EXP, ...)     ((void) 0)
#endif

/*
* Debug-configuration assertion with high-overhead.
* Use only when extensive checks are necessary.
* Fails only when EXP evaluates to false.
*/
#if LO_ASSERT_DBG_IS_ENABLED
#define LO_ASSERT_DBG               LO_ASSERT_REL
#else
#define LO_ASSERT_DBG(EXP, ...)     ((void) 0)
#endif

#ifdef assert
#undef assert
#endif
#define assert     LO_ASSERT

#endif // ! defined(INCLUDED_SAL_ASSERT_H)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
