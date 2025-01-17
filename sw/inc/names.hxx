/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <rtl/ustring.hxx>
#include <functional>

/// Thin wrapper around OUString to make visible in code when we are dealing with a UIName vs a Programmatic Name
class ProgName
{
public:
    ProgName() {}
    constexpr explicit ProgName(const OUString& s)
        : m_s(s)
    {
    }
    const OUString& toString() const { return m_s; }
    bool isEmpty() const { return m_s.isEmpty(); }
    bool operator==(const ProgName& s) const = default;
    bool operator==(const OUString& s) const { return m_s == s; }
    bool operator==(std::u16string_view s) const { return m_s == s; }
    bool operator<(const ProgName& s) const { return m_s < s.m_s; }
    bool operator>(const ProgName& s) const { return m_s > s.m_s; }

private:
    OUString m_s;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
