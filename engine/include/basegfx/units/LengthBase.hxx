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
#include <compare>
#include <limits>
#include <type_traits>

namespace gfx
{
/// Supported length units
enum class LengthUnit
{
    hmm, // 100th mm
    mm,
    cm,
    meter,
    twip,
    in,
    pt,
    px,
    emu // English Metric Units
};

/** Base template for a type-safe length type with EMU internal storage.
 *
 * Stores a length value internally as EMU (English Metric Units).
 * EMU can represent both twips and 100ths of mm as exact integers
 * (1 twip = 635 EMU, 1 hmm = 360 EMU), avoiding rounding errors
 * when converting between metric and imperial units.
 *
 * The template parameter TYPE defines the storage type (e.g.
 * sal_Int64 for the concrete Length type).
 *
 * Named factory methods (twip(), hmm(), emu(), ...) create values
 * from specific units. Named accessors (as_twip(), as_hmm(), ...)
 * convert back. Arithmetic operators work directly in EMU.
 */
template <typename TYPE> class LengthBase
{
private:
    /// Value in EMU units
    TYPE m_nValue;

    // Unit conversion factors to EMU

    // Metric units
    static constexpr sal_Int64 constFactor_hmm_to_EMU = 360ll;
    static constexpr sal_Int64 constFactor_mm_to_EMU = constFactor_hmm_to_EMU * 100ll;
    static constexpr sal_Int64 constFactor_cm_to_EMU = constFactor_hmm_to_EMU * 1000ll;
    static constexpr sal_Int64 constFactor_meter_to_EMU = constFactor_hmm_to_EMU * 100000ll;

    // Imperial units
    static constexpr sal_Int64 constFactor_twip_to_EMU = 635ll;
    static constexpr sal_Int64 constFactor_in_to_EMU = constFactor_twip_to_EMU * 1440ll;
    static constexpr sal_Int64 constFactor_pt_to_EMU = constFactor_twip_to_EMU * 20ll;
    static constexpr sal_Int64 constFactor_px_to_EMU = constFactor_twip_to_EMU * 15ll;
    static constexpr sal_Int64 constFactor_EMU_to_EMU = 1ll;

    static constexpr sal_Int64 factorForUnit(LengthUnit eUnit)
    {
        switch (eUnit)
        {
            case LengthUnit::hmm:
                return constFactor_hmm_to_EMU;
            case LengthUnit::mm:
                return constFactor_mm_to_EMU;
            case LengthUnit::cm:
                return constFactor_cm_to_EMU;
            case LengthUnit::meter:
                return constFactor_meter_to_EMU;
            case LengthUnit::twip:
                return constFactor_twip_to_EMU;
            case LengthUnit::in:
                return constFactor_in_to_EMU;
            case LengthUnit::pt:
                return constFactor_pt_to_EMU;
            case LengthUnit::px:
                return constFactor_px_to_EMU;
            case LengthUnit::emu:
                return constFactor_EMU_to_EMU;
        }
        return constFactor_EMU_to_EMU;
    }

protected:
    constexpr explicit LengthBase(TYPE nValue)
        : m_nValue(nValue)
    {
    }

public:
    static constexpr LengthBase minimum();
    static constexpr LengthBase maximum();

    // Function to convert when type is not known at compile time.
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

    /** Create from a value in the given unit (compile-time unit selection).
     *
     * Floating-point inputs are rounded to the nearest EMU.
     */
    template <LengthUnit UNIT, typename INPUT_TYPE>
    static constexpr LengthBase fromUnit(INPUT_TYPE nValue)
    {
        constexpr sal_Int64 nFactor = factorForUnit(UNIT);
        if constexpr (std::is_floating_point_v<INPUT_TYPE>)
            return LengthBase(TYPE(std::round(nFactor * nValue)));
        else
            return LengthBase(TYPE(nFactor * nValue));
    }

    template <typename INPUT_TYPE> static constexpr LengthBase cm(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::cm>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase mm(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::mm>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase hmm(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::hmm>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase meter(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::meter>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase in(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::in>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase twip(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::twip>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase pt(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::pt>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase px(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::px>(nValue);
    }

    template <typename INPUT_TYPE> static constexpr LengthBase emu(INPUT_TYPE nValue)
    {
        return fromUnit<LengthUnit::emu>(nValue);
    }

    constexpr explicit LengthBase()
        : m_nValue(0)
    {
    }

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
        if constexpr (std::is_floating_point_v<INPUT>)
            m_nValue = TYPE(std::round(m_nValue * rhs));
        else
            m_nValue *= rhs;
        return *this;
    }

    template <typename INPUT> constexpr LengthBase& operator/=(INPUT const& rhs)
    {
        if constexpr (std::is_floating_point_v<INPUT>)
            m_nValue = TYPE(std::round(m_nValue / rhs));
        else
            m_nValue /= rhs;
        return *this;
    }

    constexpr LengthBase operator-() const { return LengthBase(-m_nValue); }

    constexpr std::strong_ordering operator<=>(LengthBase const&) const = default;

    /// Returns the raw internal value. The unit depends on the
    /// LengthBase configuration and is not guaranteed to be EMU.
    /// Prefer as_emu(), as_hmm(), as_twip() etc. for unit-safe access.
    constexpr TYPE data() const { return m_nValue; }

    /** Convert to the given unit (runtime unit selection).
     *  Prefer the named as_hmm(), as_twip() etc. when the unit is known. */
    constexpr double as(LengthUnit eUnit) const
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

    /// Convert to the given unit (compile-time unit selection)
    template <LengthUnit UNIT> constexpr double toUnit() const
    {
        constexpr sal_Int64 nFactor = factorForUnit(UNIT);
        return m_nValue / double(nFactor);
    }

    /// Convert to the given unit as double
    constexpr double as_hmm() const { return toUnit<LengthUnit::hmm>(); }
    constexpr double as_mm() const { return toUnit<LengthUnit::mm>(); }
    constexpr double as_cm() const { return toUnit<LengthUnit::cm>(); }
    constexpr double as_meter() const { return toUnit<LengthUnit::meter>(); }
    constexpr double as_twip() const { return toUnit<LengthUnit::twip>(); }
    constexpr double as_in() const { return toUnit<LengthUnit::in>(); }
    constexpr double as_pt() const { return toUnit<LengthUnit::pt>(); }
    constexpr double as_px() const { return toUnit<LengthUnit::px>(); }
    constexpr double as_emu() const { return toUnit<LengthUnit::emu>(); }

    /// Convert to the given unit, rounded and cast to the requested type T
    /// Example: length.as_twip<sal_Int32>()
    template <typename T> constexpr T as_hmm() const { return T(std::round(as_hmm())); }
    template <typename T> constexpr T as_mm() const { return T(std::round(as_mm())); }
    template <typename T> constexpr T as_cm() const { return T(std::round(as_cm())); }
    template <typename T> constexpr T as_twip() const { return T(std::round(as_twip())); }
    template <typename T> constexpr T as_in() const { return T(std::round(as_in())); }
    template <typename T> constexpr T as_pt() const { return T(std::round(as_pt())); }
    template <typename T> constexpr T as_px() const { return T(std::round(as_px())); }
    template <typename T> constexpr T as_emu() const { return T(std::round(as_emu())); }
};

/// Addition
/// example: 1_cm + 10_mm
template <typename T> constexpr LengthBase<T> operator+(LengthBase<T> lhs, const LengthBase<T>& rhs)
{
    return lhs += rhs;
}

/// Subtraction
/// example: 1_cm - 5_mm
template <typename T> constexpr LengthBase<T> operator-(LengthBase<T> lhs, const LengthBase<T>& rhs)
{
    return lhs -= rhs;
}

/// Multiplication of a length unit with a scalar value.
/// example 1_cm * 2 = 2_cm
template <typename T, typename INPUT_TYPE>
constexpr LengthBase<T> operator*(LengthBase<T> lhs, const INPUT_TYPE rhs)
{
    return lhs *= rhs;
}

/// Multiplication of a scalar value with a length unit.
/// example 2 * 1_cm = 2_cm
template <typename T, typename INPUT_TYPE>
constexpr LengthBase<T> operator*(const INPUT_TYPE lhs, LengthBase<T> rhs)
{
    return rhs *= lhs;
}

/// Division of a length unit with a scalar value.
/// example 1_cm / 2 = 0.5_cm
template <typename T, typename INPUT_TYPE>
constexpr LengthBase<T> operator/(LengthBase<T> lhs, const INPUT_TYPE rhs)
{
    return lhs /= rhs;
}

/// Division of 2 length units, which results in a ratio.
/// example 1_cm / 2_cm = 0.5
template <typename T> constexpr double operator/(LengthBase<T> lhs, const LengthBase<T> rhs)
{
    return lhs.data() / double(rhs.data());
}

} // end namespace gfx
