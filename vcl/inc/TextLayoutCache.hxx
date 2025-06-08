/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <o3tl/hash_combine.hxx>

#include <vcl/dllapi.h>
#include <vcl/dropcache.hxx>

#include <unicode/uscript.h>

#include <new>
#include <vector>

namespace vcl::text
{
struct Run
{
    int32_t nStart;
    int32_t nEnd;
    UScriptCode nCode;
    Run(int32_t nStart_, int32_t nEnd_, UScriptCode nCode_)
        : nStart(nStart_)
        , nEnd(nEnd_)
        , nCode(nCode_)
    {
    }
};

class VCL_DLLPUBLIC TextLayoutCache
{
public:
#if defined __cpp_lib_memory_resource
    std::pmr::vector<vcl::text::Run> runs;
#else
    std::vector<vcl::text::Run> runs;
#endif
    TextLayoutCache(sal_Unicode const* pStr, sal_Int32 const nEnd);
    // Creates a cached instance.
    static std::shared_ptr<const vcl::text::TextLayoutCache> Create(OUString const&);
};

struct FirstCharsStringHash
{
    size_t operator()(const OUString& str) const
    {
        // Strings passed to GenericSalLayout::CreateTextLayoutCache() may be very long,
        // and computing an entire hash could almost negate the gain of hashing. Hash just first
        // characters, that should be good enough.
        size_t hash
            = rtl_ustr_hashCode_WithLength(str.getStr(), std::min<size_t>(100, str.getLength()));
        o3tl::hash_combine(hash, str.getLength());
        return hash;
    }
};

struct FastStringCompareEqual
{
    bool operator()(const OUString& str1, const OUString& str2) const
    {
        // Strings passed to GenericSalLayout::CreateTextLayoutCache() may be very long,
        // and OUString operator == compares backwards and using hard-written code, while
        // memcmp() compares much faster.
        if (str1.getLength() != str2.getLength())
            return false;
        if (str1.getStr() == str2.getStr())
            return true;
        return memcmp(str1.getStr(), str2.getStr(), str1.getLength() * sizeof(str1.getStr()[0]))
               == 0;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
