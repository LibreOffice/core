/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/SpinButton.hxx>

namespace weld
{
double SpinButton::convert_value_to_double(sal_Int64 nValue) const
{
    return static_cast<double>(nValue) / Power10(get_digits());
}

sal_Int64 SpinButton::convert_double_to_value(double fDouble) const
{
    return basegfx::fround64(fDouble * Power10(get_digits()));
}

std::optional<OUString> SpinButton::format_floating_point_value(double fValue)
{
    if (!m_aFormatValueHdl.IsSet())
        return {};
    const OUString sText = m_aFormatValueHdl.Call(convert_double_to_value(fValue));
    return sText;
}

TriState SpinButton::parse_text(const OUString& rText, double* pResult)
{
    if (!m_aParseTextHdl.IsSet())
        return TRISTATE_INDET;
    std::optional<int> aValue = m_aParseTextHdl.Call(rText);
    if (!aValue.has_value())
        return TRISTATE_FALSE;

    *pResult = convert_value_to_double(aValue.value());
    return TRISTATE_TRUE;
}

void SpinButton::set_value(sal_Int64 value)
{
    disable_notify_events();
    set_floating_point_value(convert_value_to_double(value));
    enable_notify_events();
}

void SpinButton::set_range(sal_Int64 min, sal_Int64 max)
{
    set_floating_point_range(convert_value_to_double(min), convert_value_to_double(max));
}

void SpinButton::get_range(sal_Int64& min, sal_Int64& max) const
{
    double fMin = 0;
    double fMax = 0;
    get_floating_point_range(fMin, fMax);
    min = convert_double_to_value(fMin);
    max = convert_double_to_value(fMax);
}

void SpinButton::set_min(sal_Int64 min)
{
    sal_Int64 dummy, max;
    get_range(dummy, max);
    set_range(min, max);
}

void SpinButton::set_max(sal_Int64 max)
{
    sal_Int64 min, dummy;
    get_range(min, dummy);
    set_range(min, max);
}

sal_Int64 SpinButton::get_min() const
{
    sal_Int64 min, dummy;
    get_range(min, dummy);
    return min;
}

sal_Int64 SpinButton::get_max() const
{
    sal_Int64 dummy, max;
    get_range(dummy, max);
    return max;
}

void SpinButton::set_increments(sal_Int64 step, sal_Int64 page)
{
    set_floating_point_increments(convert_value_to_double(step), convert_value_to_double(page));
}

void SpinButton::get_increments(sal_Int64& step, sal_Int64& page) const
{
    double fStep = 0;
    double fPage = 0;
    get_floating_point_increments(fStep, fPage);
    step = convert_double_to_value(fStep);
    page = convert_double_to_value(fPage);
}

unsigned int SpinButton::Power10(unsigned int n)
{
    unsigned int nValue = 1;
    for (unsigned int i = 0; i < n; ++i)
        nValue *= 10;
    return nValue;
}

sal_Int64 SpinButton::denormalize(sal_Int64 nValue) const
{
    const int nFactor = Power10(get_digits());

    if ((nValue < (std::numeric_limits<sal_Int64>::min() + nFactor))
        || (nValue > (std::numeric_limits<sal_Int64>::max() - nFactor)))
    {
        return nValue / nFactor;
    }

    const int nHalf = nFactor / 2;

    if (nValue < 0)
        return (nValue - nHalf) / nFactor;
    return (nValue + nHalf) / nFactor;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
