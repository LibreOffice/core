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

#include <cassert>
#include <string_view>
#include <utility>

#include <prewin.h>
#include <oleauto.h>
#include <postwin.h>

#include <o3tl/char16_t2wchar_t.hxx>

namespace sal::systools
{
// BSTR safe wrapper
class BStr
{
public:
    static BSTR newBSTR(std::u16string_view sv)
    {
        return ::SysAllocStringLen(o3tl::toW(sv.data()), sv.size());
    }
    BStr() = default;
    BStr(std::u16string_view sv)
        : m_Str(newBSTR(sv))
    {
    }
    BStr(BStr&& src) noexcept
        : m_Str(src.release())
    {
    }
    ~BStr() { ::SysFreeString(m_Str); }
    BStr& operator=(std::u16string_view sv)
    {
        ::SysFreeString(std::exchange(m_Str, newBSTR(sv)));
        return *this;
    }
    BStr& operator=(const BStr& src)
    {
        if (&src != this)
            operator=(std::u16string_view(src));
        return *this;
    }
    BStr& operator=(BStr&& src) noexcept
    {
        ::SysFreeString(std::exchange(m_Str, src.release()));
        return *this;
    }
    operator std::u16string_view() const { return { o3tl::toU(m_Str), length() }; }
    operator BSTR() const { return m_Str; }
    BSTR* operator&()
    {
        assert(!m_Str);
        return &m_Str;
    }
    UINT length() const { return ::SysStringLen(m_Str); }
    // similar to std::unique_ptr::release
    BSTR release() { return std::exchange(m_Str, nullptr); }

private:
    BSTR m_Str = nullptr;
};

} // sal::systools

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
