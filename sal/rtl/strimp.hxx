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

#ifndef INCLUDED_SAL_RTL_STRIMP_HXX
#define INCLUDED_SAL_RTL_STRIMP_HXX

#include <config_probes.h>
#if USE_SDT_PROBES
#include <sys/sdt.h>
#endif

#include <algorithm>
#include <cassert>

#include <sal/types.h>
#include <rtl/strbuf.h>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.h>
#include <rtl/ustring.hxx>

/* ======================================================================= */
/* Help functions for String and UString                                   */
/* ======================================================================= */

/*
 * refCount is opaque and includes 2 bit-fields;
 * MSB:   'interned' - is stored in the intern hash
 * MSB-1: 'static'   - is a const / static string,
 *                     do no ref counting
 */
#define SAL_STRING_INTERN_FLAG 0x80000000
#define SAL_STRING_STATIC_FLAG 0x40000000
#define SAL_STRING_REFCOUNT(a) ((a) & 0x3fffffff)

#define SAL_STRING_IS_INTERN(a) ((a)->refCount & SAL_STRING_INTERN_FLAG)
#define SAL_STRING_IS_STATIC(a) ((a)->refCount & SAL_STRING_STATIC_FLAG)

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix );

bool rtl_ImplIsWhitespace( sal_Unicode c );

rtl_uString* rtl_uString_ImplAlloc( sal_Int32 nLen );

rtl_String* rtl_string_ImplAlloc( sal_Int32 nLen );

extern "C" {

typedef void *(SAL_CALL * rtl_allocateStringFn)(size_t size);
typedef void  (*rtl_freeStringFn)(void *);

}

extern rtl_allocateStringFn rtl_allocateString;
extern rtl_freeStringFn rtl_freeString;

// string lifetime instrumentation / diagnostics
#if USE_SDT_PROBES
#  define PROBE_SNAME(n,b) n ## _ ## b
#  define PROBE_NAME(n,b) PROBE_SNAME(n,b)
#  define PROBE_NEW PROBE_NAME (new_string,RTL_LOG_STRING_BITS)
#  define PROBE_DEL PROBE_NAME (delete_string,RTL_LOG_STRING_BITS)
#  define PROBE_INTERN_NEW PROBE_NAME (new_string_intern,RTL_LOG_STRING_BITS)
#  define PROBE_INTERN_DEL PROBE_NAME (delete_string_intern,RTL_LOG_STRING_BITS)
#  define RTL_LOG_STRING_NEW(s) \
    DTRACE_PROBE4(libreoffice, PROBE_NEW, s, \
                  (s)->refCount, (s)->length, (s)->buffer)
#  define RTL_LOG_STRING_DELETE(s) \
    DTRACE_PROBE4(libreoffice, PROBE_DEL, s, \
                  (s)->refCount, (s)->length, (s)->buffer)
#  define RTL_LOG_STRING_INTERN_NEW(s,o) \
    DTRACE_PROBE5(libreoffice, PROBE_INTERN_NEW, s, \
                  (s)->refCount, (s)->length, (s)->buffer, o)
#  define RTL_LOG_STRING_INTERN_DELETE(s) \
    DTRACE_PROBE4(libreoffice, PROBE_INTERN_DEL, s, \
                  (s)->refCount, (s)->length, (s)->buffer)
#else
#  define RTL_LOG_STRING_NEW(s)
#  define RTL_LOG_STRING_DELETE(s)
#  define RTL_LOG_STRING_INTERN_NEW(s,o)
#  define RTL_LOG_STRING_INTERN_DELETE(s)
#endif /* USE_SDT_PROBES */

namespace
{
namespace detail
{
void acquire(rtl_uString* s) { return rtl_uString_acquire(s); }
void acquire(rtl_String* s) { return rtl_string_acquire(s); }

void release(rtl_uString* s) { return rtl_uString_release(s); }
void release(rtl_String* s) { return rtl_string_release(s); }

void assign(rtl_uString** s, rtl_uString* s1) { return rtl_uString_assign(s, s1); }
void assign(rtl_String** s, rtl_String* s1) { return rtl_string_assign(s, s1); }

void new_WithLength(rtl_uString** s, sal_Int32 len) { return rtl_uString_new_WithLength(s, len); }
void new_WithLength(rtl_String** s, sal_Int32 len) { return rtl_string_new_WithLength(s, len); }

sal_Int32 indexOf(const sal_Unicode* s, sal_Int32 len, const sal_Unicode* subStr, sal_Int32 subLen)
{
    return rtl_ustr_indexOfStr_WithLength(s, len, subStr, subLen);
}
sal_Int32 indexOf(const sal_Unicode* s, sal_Int32 len, const char* subStr, sal_Int32 subLen)
{
    return rtl_ustr_indexOfAscii_WithLength(s, len, subStr, subLen);
}
sal_Int32 indexOf(const char* s, sal_Int32 nLen, const char* subStr, sal_Int32 subLen)
{
    return rtl_str_indexOfStr_WithLength(s, nLen, subStr, subLen);
}

void insert(rtl_uString** s, sal_Int32* capacity, sal_Int32 offset, const sal_Unicode* s1,
            sal_Int32 len)
{
    return rtl_uStringbuffer_insert(s, capacity, offset, s1, len);
}
void insert(rtl_uString** s, sal_Int32* capacity, sal_Int32 offset, const char* s1, sal_Int32 len)
{
    return rtl_uStringbuffer_insert_ascii(s, capacity, offset, s1, len);
}
void insert(rtl_String** s, sal_Int32* capacity, sal_Int32 offset, const char* s1, sal_Int32 len)
{
    return rtl_stringbuffer_insert(s, capacity, offset, s1, len);
}
}

template <class S, typename CharTypeFrom, typename CharTypeTo>
void string_newReplaceAllFromIndex_impl(S** newStr, S* str, CharTypeFrom const* from,
                                        sal_Int32 fromLength, CharTypeTo const* to,
                                        sal_Int32 toLength, sal_Int32 fromIndex)
{
    assert(str != nullptr);
    assert(fromLength >= 0);
    assert(from != nullptr || fromLength == 0);
    assert(toLength >= 0);
    assert(to != nullptr || toLength == 0);
    assert(fromIndex >= 0 && fromIndex <= str->length);
    sal_Int32 i
        = detail::indexOf(str->buffer + fromIndex, str->length - fromIndex, from, fromLength);
    if (i >= 0)
    {
        if (str->length - fromLength > SAL_MAX_INT32 - toLength)
            std::abort();
        detail::acquire(str); // in case *newStr == str
        sal_Int32 nCapacity = str->length + (toLength - fromLength);
        if (fromLength < toLength)
        {
            // Pre-allocate up to 16 replacements more
            const sal_Int32 nMaxMoreFinds = (str->length - fromIndex - i - fromLength) / fromLength;
            const sal_Int32 nIncrease = toLength - fromLength;
            const sal_Int32 nMoreReplacements = std::min(
                { nMaxMoreFinds, (SAL_MAX_INT32 - nCapacity) / nIncrease, sal_Int32(16) });
            nCapacity += nMoreReplacements * nIncrease;
        }
        detail::new_WithLength(newStr, nCapacity);
        i += fromIndex;
        fromIndex = 0;
        do
        {
            detail::insert(newStr, &nCapacity, (*newStr)->length, str->buffer + fromIndex, i);
            detail::insert(newStr, &nCapacity, (*newStr)->length, to, toLength);
            fromIndex += i + fromLength;
            i = detail::indexOf(str->buffer + fromIndex, str->length - fromIndex, from, fromLength);
        } while (i >= 0);
        // the rest
        detail::insert(newStr, &nCapacity, (*newStr)->length, str->buffer + fromIndex,
                       str->length - fromIndex);
        detail::release(str);
    }
    else
        detail::assign(newStr, str);
}
}

#endif // INCLUDED_SAL_RTL_STRIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
