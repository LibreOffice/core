/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <cmath>

namespace gfx
{
// Enum of length units
enum class LengthUnit
{
    hmm,
    mm,
    cm,
    meter,
    twip,
    in,
    pt,
    px,
    emu
};

namespace
{
// Unit conversion factors
constexpr const sal_Int64 constFactor_hmm_to_EMU = 360ll;
constexpr const sal_Int64 constFactor_mm_to_EMU = constFactor_hmm_to_EMU * 100ll;
constexpr const sal_Int64 constFactor_cm_to_EMU = constFactor_hmm_to_EMU * 1000ll;
constexpr const sal_Int64 constFactor_meter_to_EMU = constFactor_hmm_to_EMU * 100000ll;

constexpr const sal_Int64 constFactor_twip_to_EMU = 635ll;
constexpr const sal_Int64 constFactor_in_to_EMU = constFactor_twip_to_EMU * 1440ll;
constexpr const sal_Int64 constFactor_pt_to_EMU = constFactor_twip_to_EMU * 20ll;
constexpr const sal_Int64 constFactor_px_to_EMU = constFactor_twip_to_EMU * 15ll;

} // end anonymous namespace

/** Length class base
 *
 * Template class, which defines a length type, with an arbitrary base
 * type to store the values in. The smalles value unit is an EMU, but
 * as all the uses of the type is internal, it could be changed to another
 * unit if necessary.
 * The class takes care of all the conversion to and from EMU unit to
 * other units. Most basic arithmetic and comparsion operators are
 * implemented.
 */
template <typename TYPE> class LengthBase
{
private:
    /// value in EMU units
    TYPE m_nValue;

protected:
    constexpr explicit LengthBase(TYPE nValue)
        : m_nValue(nValue)
    {
    }

public:
    static constexpr LengthBase minimum();
    static constexpr LengthBase maximum();

    template <typename INPUT_TYPE>
    static constexpr LengthBase from(LengthUnit eUnit, INPUT_TYPE nValue)
    {
        switch (eUnit)
        {
            case LengthUnit::hmm:
                return hmm(nValue);
            case LengthUnit::mm:
                return mm(nValue);
            case LengthUnit::cm:
                return cm(nValue);
            case LengthUnit::meter:
                return meter(nValue);
            case LengthUnit::twip:
                return twip(nValue);
            case LengthUnit::in:
                return in(nValue);
            case LengthUnit::pt:
                return pt(nValue);
            case LengthUnit::px:
                return px(nValue);
            case LengthUnit::emu:
                return emu(nValue);
        }
        return emu(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase cm(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_cm_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase mm(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_mm_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase hmm(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_hmm_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase meter(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_meter_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase in(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_in_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase twip(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_twip_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase pt(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_pt_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase px(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(gfx::constFactor_px_to_EMU * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase emu(INPUT_TYPE nValue)
    {
        return LengthBase(TYPE(nValue));
    }

    constexpr explicit LengthBase()
        : m_nValue(0)
    {
    }

    constexpr explicit operator TYPE() const { return m_nValue; }

    constexpr LengthBase& operator+=(LengthBase const& rhs)
    {
        m_nValue += rhs.m_nValue;
        return *this;
    }

    constexpr LengthBase& operator-=(LengthBase const& rhs)
    {
        m_nValue -= rhs.m_nValue;
        return *this;
    }

    template <typename INPUT> constexpr LengthBase& operator*=(INPUT const& rhs)
    {
        m_nValue *= rhs;
        return *this;
    }

    template <typename INPUT> constexpr LengthBase& operator/=(INPUT const& rhs)
    {
        m_nValue /= rhs;
        return *this;
    }

    constexpr LengthBase& operator-()
    {
        m_nValue = -m_nValue;
        return *this;
    }

    constexpr bool operator<(LengthBase const& other) const { return m_nValue < other.m_nValue; }
    constexpr bool operator<=(LengthBase const& other) const { return m_nValue <= other.m_nValue; }
    constexpr bool operator>(LengthBase const& other) const { return m_nValue > other.m_nValue; }
    constexpr bool operator>=(LengthBase const& other) const { return m_nValue >= other.m_nValue; }
    constexpr bool operator==(LengthBase const& other) const { return m_nValue == other.m_nValue; }
    constexpr bool operator!=(LengthBase const& other) const { return m_nValue != other.m_nValue; }

    constexpr TYPE raw() const { return m_nValue; }

    constexpr TYPE data() const { return m_nValue; }

    double as(LengthUnit eUnit) const
    {
        switch (eUnit)
        {
            case LengthUnit::hmm:
                return as_hmm();
            case LengthUnit::mm:
                return as_mm();
            case LengthUnit::cm:
                return as_cm();
            case LengthUnit::meter:
                return as_meter();
            case LengthUnit::twip:
                return as_twip();
            case LengthUnit::in:
                return as_in();
            case LengthUnit::pt:
                return as_pt();
            case LengthUnit::px:
                return as_px();
            case LengthUnit::emu:
                return as_emu();
        }
        return std::numeric_limits<double>::infinity();
    }

    double as_hmm() const { return m_nValue / double(constFactor_hmm_to_EMU); }
    double as_mm() const { return m_nValue / double(constFactor_mm_to_EMU); }
    double as_cm() const { return m_nValue / double(constFactor_cm_to_EMU); }
    double as_meter() const { return m_nValue / double(constFactor_meter_to_EMU); }
    double as_twip() const { return m_nValue / double(constFactor_twip_to_EMU); }
    double as_in() const { return m_nValue / double(constFactor_in_to_EMU); }
    double as_pt() const { return m_nValue / double(constFactor_pt_to_EMU); }
    double as_px() const { return m_nValue / double(constFactor_px_to_EMU); }
    double as_emu() const { return double(m_nValue); }
};

template <typename T> inline LengthBase<T> operator+(LengthBase<T> lhs, const LengthBase<T>& rhs)
{
    return lhs += rhs;
}

template <typename T> inline LengthBase<T> operator-(LengthBase<T> lhs, const LengthBase<T>& rhs)
{
    return lhs -= rhs;
}

/// Multiplication of a length unit with a scalar value.
/// example 1cm * 2 = 2cm
template <typename T, typename INPUT_TYPE>
inline LengthBase<T> operator*(LengthBase<T> lhs, const INPUT_TYPE rhs)
{
    return lhs *= rhs;
}

/// Division of a length unit with a scalar value.
/// example 1cm / 2 = 0.5cm
template <typename T, typename INPUT_TYPE>
inline LengthBase<T> operator/(LengthBase<T> lhs, const INPUT_TYPE rhs)
{
    return lhs /= rhs;
}

/// Division of 2 length units, which results in a ratio.
/// example 1cm / 2cm = 0.5
template <typename T> inline double operator/(LengthBase<T> lhs, const LengthBase<T> rhs)
{
    return lhs.raw() / double(rhs.raw());
}

} // end namespace gfx
