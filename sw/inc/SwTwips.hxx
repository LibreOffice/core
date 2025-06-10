/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/units/Length.hxx>
#include <o3tl/concepts.hxx>

class SW_DLLPUBLIC SwTwips
{
private:
    gfx::Length maValue;

public:
    constexpr SwTwips() = default;

    constexpr SwTwips(tools::Long aInput)
        : maValue(gfx::Length::from(gfx::LengthUnit::twip, aInput))
    {
    }

    constexpr SwTwips(gfx::Length const& rLength)
        : maValue(rLength)
    {
    }

    operator tools::Long() const { return std::lround(maValue.as_twip()); }

    constexpr SwTwips& operator+=(tools::Long const& value)
    {
        gfx::Length nLength = gfx::Length::from(gfx::LengthUnit::twip, value);
        maValue += nLength;
        return *this;
    }

    constexpr SwTwips& operator++()
    {
        maValue += 1_twip;
        return *this;
    }

    constexpr SwTwips& operator-=(tools::Long const& value)
    {
        gfx::Length nLength = gfx::Length::from(gfx::LengthUnit::twip, value);
        maValue -= nLength;
        return *this;
    }

    template <typename INPUT> constexpr SwTwips& operator/=(INPUT const& value)
    {
        maValue /= value;
        return *this;
    }

    template <typename INPUT> constexpr SwTwips& operator*=(INPUT const& value)
    {
        maValue *= value;
        return *this;
    }

    constexpr SwTwips& operator-()
    {
        maValue = -maValue;
        return *this;
    }

    constexpr gfx::Length const& data() const { return maValue; }
};

inline SwTwips operator+(const SwTwips& rA, const SwTwips& rB)
{
    SwTwips aNew(rA.data() + rB.data());
    return aNew;
}

inline SwTwips operator-(const SwTwips& rA, const SwTwips& rB)
{
    SwTwips aNew(rA.data() - rB.data());
    return aNew;
}

inline SwTwips operator+(const SwTwips& rA, const gfx::Length& rB)
{
    SwTwips aNew(rA.data() + rB);
    return aNew;
}

inline SwTwips operator-(const SwTwips& rA, const gfx::Length& rB)
{
    SwTwips aNew(rA.data() - rB);
    return aNew;
}

inline SwTwips operator+(const SwTwips& rA, const tools::Long& rB)
{
    SwTwips aNew(rA.data() + SwTwips(rB).data());
    return aNew;
}

inline SwTwips operator-(const SwTwips& rA, const tools::Long& rB)
{
    SwTwips aNew(rA.data() - SwTwips(rB).data());
    return aNew;
}

inline SwTwips operator+(const tools::Long& rA, const SwTwips& rB)
{
    SwTwips aNew(SwTwips(rA).data() + rB.data());
    return aNew;
}

inline SwTwips operator-(const tools::Long& rA, const SwTwips& rB)
{
    SwTwips aNew(SwTwips(rA).data() - rB.data());
    return aNew;
}

namespace std
{
template <> struct hash<SwTwips>
{
    std::size_t operator()(SwTwips const& rLength) const
    {
        return std::hash<sal_Int64>()(rLength.data().data());
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
