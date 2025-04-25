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
class UIName
{
public:
    UIName() {}
    constexpr explicit UIName(const OUString& s)
        : m_s(s)
    {
    }
    const OUString& toString() const { return m_s; }
    bool isEmpty() const { return m_s.isEmpty(); }
    bool operator==(const UIName& s) const { return m_s == s.m_s; }
    bool operator==(const OUString& s) const { return m_s == s; }
    bool operator==(std::u16string_view s) const { return m_s == s; }
    bool operator<(const UIName& s) const { return m_s < s.m_s; }
    bool operator>(const UIName& s) const { return m_s > s.m_s; }

private:
    OUString m_s;
};

namespace std
{
template <> struct hash<UIName>
{
    std::size_t operator()(UIName const& s) const { return std::hash<OUString>()(s.toString()); }
};
}

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

class SwMarkName
{
public:
    SwMarkName() {}
    explicit SwMarkName(const OUString& s)
        : m_s(s)
    {
    }
    const OUString& toString() const { return m_s; }
    bool isEmpty() const { return m_s.isEmpty(); }
    bool operator==(const SwMarkName& s) const = default;
    bool operator==(const OUString& s) const { return m_s == s; }
    bool operator==(std::u16string_view s) const { return m_s == s; }
    bool operator<(const SwMarkName& s) const { return m_s < s.m_s; }
    bool operator>(const SwMarkName& s) const { return m_s > s.m_s; }

private:
    OUString m_s;
};

namespace std
{
template <> struct hash<SwMarkName>
{
    std::size_t operator()(SwMarkName const& s) const
    {
        return std::hash<OUString>()(s.toString());
    }
};
}

// SwTableAutoFormat names are their special little snowflake. Mostly they are UINames, but sometimes they
// are programmatic names, so I isolated them into their own world.
class TableStyleName
{
public:
    TableStyleName() {}
    constexpr explicit TableStyleName(const OUString& s)
        : m_s(s)
    {
    }
    const OUString& toString() const { return m_s; }
    bool isEmpty() const { return m_s.isEmpty(); }
    bool operator==(const TableStyleName& s) const = default;
    bool operator==(const OUString& s) const { return m_s == s; }
    bool operator==(std::u16string_view s) const { return m_s == s; }
    bool operator<(const TableStyleName& s) const { return m_s < s.m_s; }
    bool operator>(const TableStyleName& s) const { return m_s > s.m_s; }

private:
    OUString m_s;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
