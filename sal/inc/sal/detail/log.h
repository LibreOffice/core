/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_SAL_DETAIL_LOG_H
#define INCLUDED_SAL_DETAIL_LOG_H

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

/** @cond INTERNAL */

/* This header makes available replacements working in both C and C++ for the
   obsolete osl/diagnose.h functionality that in turn is used from both C and
   C++ code and the obsolete tools/debug.hxx and
   canvas/inc/canvas/verbosetrace.hxx functionality that uses printf-style
   formatting.  Once that obsolete functionality is removed, this header can be
   removed, too.

   This header uses variadic macros in both C (where they are officially only
   supported since C99) and C++ (where they are officially only supported since
   C++11).  It appears that all relevant compilers (esp. GCC 4.0 and MS VS 2008
   Express) already support them in their C and C++ dialects.  See also
   <http://wiki.apache.org/stdcxx/C++0xCompilerSupport>.

   Avoid the use of other sal code in this header as much as possible, so that
   this code can be called from other sal code without causing endless
   recursion.
*/

#if defined __cplusplus
extern "C" {
#endif

/*
 Clang warns about 'sal_True && sal_True' (those being integers and not booleans)
 when it sees preprocessed source (-save-temps or using icecream)
*/
#if defined __cplusplus
#define SAL_LOG_TRUE true
#define SAL_LOG_FALSE false
#else
#define SAL_LOG_TRUE sal_True
#define SAL_LOG_FALSE sal_False
#endif

enum sal_detail_LogLevel {
    SAL_DETAIL_LOG_LEVEL_INFO, SAL_DETAIL_LOG_LEVEL_WARN,
    SAL_DETAIL_LOG_LEVEL_DEBUG = SAL_MAX_ENUM
};

SAL_DLLPUBLIC void SAL_CALL sal_detail_logFormat(
    enum sal_detail_LogLevel level, char const * area, char const * where,
    char const * format, ...)
/* TODO: enabling this will produce a huge amount of -Werror=format errors: */
#if defined GCC && 0
    __attribute__((format(printf, 4, 5)))
#endif
    ;

#if defined __cplusplus
}
#endif

#define SAL_DETAIL_LOG_FORMAT(condition, level, area, where, ...) \
    do { \
        if (condition) { \
            sal_detail_logFormat((level), (area), (where), __VA_ARGS__); \
        } \
    } while (SAL_LOG_FALSE)

#if defined SAL_LOG_INFO
#define SAL_DETAIL_ENABLE_LOG_INFO SAL_LOG_TRUE
#else
#define SAL_DETAIL_ENABLE_LOG_INFO SAL_LOG_FALSE
#endif
#if defined SAL_LOG_WARN
#define SAL_DETAIL_ENABLE_LOG_WARN SAL_LOG_TRUE
#else
#define SAL_DETAIL_ENABLE_LOG_WARN SAL_LOG_FALSE
#endif

#define SAL_DETAIL_WHERE __FILE__ ":" SAL_STRINGIFY(__LINE__) ": "

#define SAL_DETAIL_INFO_IF_FORMAT(condition, area, ...) \
    SAL_DETAIL_LOG_FORMAT( \
        SAL_DETAIL_ENABLE_LOG_INFO && (condition), SAL_DETAIL_LOG_LEVEL_INFO, \
        area, SAL_DETAIL_WHERE, __VA_ARGS__)

#define SAL_DETAIL_WARN_IF_FORMAT(condition, area, ...) \
    SAL_DETAIL_LOG_FORMAT( \
        SAL_DETAIL_ENABLE_LOG_WARN && (condition), SAL_DETAIL_LOG_LEVEL_WARN, \
        area, SAL_DETAIL_WHERE, __VA_ARGS__)

/** @endcond */

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
