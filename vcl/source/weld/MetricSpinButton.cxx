/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svdata.hxx>

#include <i18nutil/unicode.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/fieldvalues.hxx>
#include <vcl/weld/MetricSpinButton.hxx>

namespace weld
{
OUString MetricSpinButton::MetricToString(FieldUnit rUnit)
{
    const FieldUnitStringList& rList = ImplGetFieldUnits();
    // return unit's default string (ie, the first one )
    auto it = std::find_if(
        rList.begin(), rList.end(),
        [&rUnit](const std::pair<OUString, FieldUnit>& rItem) { return rItem.second == rUnit; });
    if (it != rList.end())
        return it->first;

    return OUString();
}

IMPL_LINK_NOARG(MetricSpinButton, spin_button_value_changed, SpinButton&, void)
{
    signal_value_changed();
}

IMPL_LINK(MetricSpinButton, spin_button_output, sal_Int64, nValue, OUString)
{
    return format_number(nValue);
}

void MetricSpinButton::update_width_chars()
{
    sal_Int64 min, max;
    m_xSpinButton->get_range(min, max);
    auto width = std::max(m_xSpinButton->get_pixel_size(format_number(min)).Width(),
                          m_xSpinButton->get_pixel_size(format_number(max)).Width());
    int chars = ceil(width / m_xSpinButton->get_approximate_digit_width());
    m_xSpinButton->set_width_chars(chars);
}

OUString MetricSpinButton::format_number(sal_Int64 nValue) const
{
    OUString aStr;

    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();

    unsigned int nDecimalDigits = m_xSpinButton->get_digits();
    //pawn percent off to icu to decide whether percent is separated from its number for this locale
    if (m_eSrcUnit == FieldUnit::PERCENT)
    {
        double fValue = nValue;
        fValue /= SpinButton::Power10(nDecimalDigits);
        aStr = unicode::formatPercent(fValue, rLocaleData.getLanguageTag());
    }
    else
    {
        aStr = rLocaleData.getNum(nValue, nDecimalDigits, true, true);
        OUString aSuffix = MetricToString(m_eSrcUnit);
        if (m_eSrcUnit != FieldUnit::NONE && m_eSrcUnit != FieldUnit::DEGREE
            && m_eSrcUnit != FieldUnit::INCH && m_eSrcUnit != FieldUnit::FOOT)
            aStr += " ";
        if (m_eSrcUnit == FieldUnit::INCH)
        {
            OUString sDoublePrime = u"\u2033"_ustr;
            if (aSuffix != "\"" && aSuffix != sDoublePrime)
                aStr += " ";
            else
                aSuffix = sDoublePrime;
        }
        else if (m_eSrcUnit == FieldUnit::FOOT)
        {
            OUString sPrime = u"\u2032"_ustr;
            if (aSuffix != "'" && aSuffix != sPrime)
                aStr += " ";
            else
                aSuffix = sPrime;
        }

        assert(m_eSrcUnit != FieldUnit::PERCENT);
        aStr += aSuffix;
    }

    return aStr;
}

void MetricSpinButton::set_digits(unsigned int digits)
{
    sal_Int64 step, page;
    get_increments(step, page, m_eSrcUnit);
    sal_Int64 value = get_value(m_eSrcUnit);
    m_xSpinButton->set_digits(digits);
    set_increments(step, page, m_eSrcUnit);
    set_value(value, m_eSrcUnit);
    update_width_chars();
}

void MetricSpinButton::set_unit(FieldUnit eUnit)
{
    if (eUnit != m_eSrcUnit)
    {
        sal_Int64 step, page;
        get_increments(step, page, m_eSrcUnit);
        sal_Int64 value = get_value(m_eSrcUnit);
        m_eSrcUnit = eUnit;
        set_increments(step, page, m_eSrcUnit);
        set_value(value, m_eSrcUnit);
        const OUString sText = format_number(m_xSpinButton->get_value());
        m_xSpinButton->set_text(sText);
        update_width_chars();
    }
}

sal_Int64 MetricSpinButton::ConvertValue(sal_Int64 nValue, FieldUnit eInUnit,
                                         FieldUnit eOutUnit) const
{
    return vcl::ConvertValue(nValue, 0, m_xSpinButton->get_digits(), eInUnit, eOutUnit);
}

IMPL_LINK(MetricSpinButton, spin_button_input, const OUString&, rText, std::optional<int>)
{
    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
    double fResult(0.0);
    bool bRet
        = vcl::TextToValue(rText, fResult, 0, m_xSpinButton->get_digits(), rLocaleData, m_eSrcUnit);
    if (!bRet)
        return {};

    if (fResult > SAL_MAX_INT32)
        fResult = SAL_MAX_INT32;
    else if (fResult < SAL_MIN_INT32)
        fResult = SAL_MIN_INT32;

    return std::optional<int>(std::round(fResult));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
