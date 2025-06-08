/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/units/LengthBase.hxx>
#include <ostream>

namespace gfx
{
typedef LengthBase<sal_Int64> Length;

// define the minimum function for our concrete type
template <> constexpr Length Length::minimum() { return Length(SAL_MIN_INT64); }

// define the maximum function for our concrete type
template <> constexpr Length Length::maximum() { return Length(SAL_MAX_INT64); }

/** Traits for the Length class
 *
 * Needed for example to define a Range2D implementation that uses the Length class.
 */
struct LengthTraits
{
    static constexpr Length minVal() { return Length::minimum(); }
    static constexpr Length maxVal() { return Length::maximum(); }
    static constexpr Length neutral() { return Length(); }

    typedef Length DifferenceType;
    typedef Length CenterType;
};

} // end namespace gfx

// user-defined literals for Length

// integer input literals
constexpr gfx::Length operator"" _emu(unsigned long long value) { return gfx::Length::emu(value); }
constexpr gfx::Length operator"" _in(unsigned long long value) { return gfx::Length::in(value); }
constexpr gfx::Length operator"" _cm(unsigned long long value) { return gfx::Length::cm(value); }
constexpr gfx::Length operator"" _mm(unsigned long long value) { return gfx::Length::mm(value); }
constexpr gfx::Length operator"" _m(unsigned long long value) { return gfx::Length::meter(value); }
constexpr gfx::Length operator"" _hmm(unsigned long long value) { return gfx::Length::hmm(value); }
constexpr gfx::Length operator"" _twip(unsigned long long value)
{
    return gfx::Length::twip(value);
}
constexpr gfx::Length operator"" _pt(unsigned long long value) { return gfx::Length::pt(value); }
constexpr gfx::Length operator"" _px(unsigned long long value) { return gfx::Length::px(value); }

// double input literals
constexpr gfx::Length operator"" _in(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_in_to_EMU * value));
}

constexpr gfx::Length operator"" _cm(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_cm_to_EMU * value));
}

constexpr gfx::Length operator"" _mm(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_mm_to_EMU * value));
}

constexpr gfx::Length operator"" _m(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_meter_to_EMU * value));
}

constexpr gfx::Length operator"" _hmm(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_hmm_to_EMU * value));
}

constexpr gfx::Length operator"" _twip(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_twip_to_EMU * value));
}

constexpr gfx::Length operator"" _pt(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_pt_to_EMU * value));
}

constexpr gfx::Length operator"" _px(long double value)
{
    return gfx::Length::emu(std::round(gfx::constFactor_px_to_EMU * value));
}

// stream operator
template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     gfx::Length const& length)
{
    return stream << length.data() << " (twip=" << length.as_twip() << ", hmm=" << length.as_hmm()
                  << ")";
}

#include <basegfx/units/LengthTypes.hxx>
