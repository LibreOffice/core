/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <prcntfld.hxx>
#include <vcl/fieldvalues.hxx>

SwPercentField::SwPercentField(std::unique_ptr<weld::MetricSpinButton> pControl)
    : m_pField(std::move(pControl))
    , m_nOldMax(0)
    , m_nOldMin(0)
    , m_nLastPercent(-1)
    , m_nLastValue(-1)
    , m_nOldDigits(m_pField->get_digits())
    , m_eOldUnit(FieldUnit::NONE)
    , m_bLockAutoCalculation(false)
{
    sal_Int64 nMin, nMax;
    m_pField->get_range(nMin, nMax, FieldUnit::TWIP);
    m_nRefValue = DenormalizePercent(nMax);
    m_pField->get_increments(m_nOldSpinSize, m_nOldPageSize, FieldUnit::NONE);
}

void SwPercentField::SetRefValue(sal_Int64 nValue)
{
    sal_Int64 nRealValue = GetRealValue(m_eOldUnit);

    m_nRefValue = nValue;

    if (!m_bLockAutoCalculation && (m_pField->get_unit() == FieldUnit::PERCENT))
        set_value(nRealValue, m_eOldUnit);
}

static sal_Int64 UpscaleTwoDecimalPlaces(sal_Int64 nCurrentWidth, int nOldDigits)
{
    // tdf#145847 Typically nOldDigits is two so deriving percentage is direct division
    // by m_nRefValue but it is one for points
    for (int i = nOldDigits; i < 2; ++i)
        nCurrentWidth *= 10;
    return nCurrentWidth;
}

void SwPercentField::ShowPercent(bool bPercent)
{
    if ((bPercent && m_pField->get_unit() == FieldUnit::PERCENT)
        || (!bPercent && m_pField->get_unit() != FieldUnit::PERCENT))
        return;

    sal_Int64 nOldValue;

    if (bPercent)
    {
        nOldValue = get_value();

        m_eOldUnit = m_pField->get_unit();
        m_nOldDigits = m_pField->get_digits();
        m_pField->get_range(m_nOldMin, m_nOldMax, FieldUnit::NONE);
        m_pField->get_increments(m_nOldSpinSize, m_nOldPageSize, FieldUnit::NONE);
        m_pField->set_unit(FieldUnit::PERCENT);
        m_pField->set_digits(0);

        sal_Int64 nCurrentWidth
            = vcl::ConvertValue(m_nOldMin, 0, m_nOldDigits, m_eOldUnit, FieldUnit::TWIP);
        nCurrentWidth = UpscaleTwoDecimalPlaces(nCurrentWidth, m_nOldDigits);

        // round to 0.5 percent
        int nPercent = m_nRefValue ? (((nCurrentWidth * 10) / m_nRefValue + 5) / 10) : 0;

        m_pField->set_range(std::max(1, nPercent), 100, FieldUnit::NONE);
        m_pField->set_increments(5, 10, FieldUnit::NONE);
        if (nOldValue != m_nLastValue)
        {
            nCurrentWidth
                = vcl::ConvertValue(nOldValue, 0, m_nOldDigits, m_eOldUnit, FieldUnit::TWIP);
            nCurrentWidth = UpscaleTwoDecimalPlaces(nCurrentWidth, m_nOldDigits);
            nPercent = m_nRefValue ? (((nCurrentWidth * 10) / m_nRefValue + 5) / 10) : 0;
            m_pField->set_value(nPercent, FieldUnit::NONE);
            m_nLastPercent = nPercent;
            m_nLastValue = nOldValue;
        }
        else
            m_pField->set_value(m_nLastPercent, FieldUnit::NONE);
    }
    else
    {
        sal_Int64 nOldPercent = get_value(FieldUnit::PERCENT);

        nOldValue = Convert(get_value(), m_pField->get_unit(), m_eOldUnit);

        m_pField->set_unit(m_eOldUnit);
        m_pField->set_digits(m_nOldDigits);
        m_pField->set_range(m_nOldMin, m_nOldMax, FieldUnit::NONE);
        m_pField->set_increments(m_nOldSpinSize, m_nOldPageSize, FieldUnit::NONE);

        if (nOldPercent != m_nLastPercent)
        {
            set_value(nOldValue, m_eOldUnit);
            m_nLastPercent = nOldPercent;
            m_nLastValue = nOldValue;
        }
        else
            set_value(m_nLastValue, m_eOldUnit);
    }
}

void SwPercentField::set_value(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT || eInUnit == FieldUnit::PERCENT)
        m_pField->set_value(Convert(nNewValue, eInUnit, m_pField->get_unit()), FieldUnit::NONE);
    else
    {
        // Overwrite output value, do not restore later
        sal_Int64 nPercent, nCurrentWidth;
        if (eInUnit == FieldUnit::TWIP)
        {
            nCurrentWidth
                = vcl::ConvertValue(nNewValue, 0, m_nOldDigits, FieldUnit::TWIP, FieldUnit::TWIP);
        }
        else
        {
            sal_Int64 nValue = Convert(nNewValue, eInUnit, m_eOldUnit);
            nCurrentWidth = vcl::ConvertValue(nValue, 0, m_nOldDigits, m_eOldUnit, FieldUnit::TWIP);
        }
        nCurrentWidth = UpscaleTwoDecimalPlaces(nCurrentWidth, m_nOldDigits);
        nPercent = m_nRefValue ? (((nCurrentWidth * 10) / m_nRefValue + 5) / 10) : 0;
        m_pField->set_value(nPercent, FieldUnit::NONE);
    }
}

sal_Int64 SwPercentField::get_value(FieldUnit eOutUnit)
{
    return Convert(m_pField->get_value(FieldUnit::NONE), m_pField->get_unit(), eOutUnit);
}

void SwPercentField::set_min(sal_Int64 nNewMin, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        m_pField->set_min(nNewMin, eInUnit);
    else
    {
        if (eInUnit == FieldUnit::NONE)
            eInUnit = m_eOldUnit;
        m_nOldMin = Convert(nNewMin, eInUnit, m_eOldUnit);

        int nPercent = Convert(nNewMin, eInUnit, FieldUnit::PERCENT);
        m_pField->set_min(std::max(1, nPercent), FieldUnit::NONE);
    }
}

void SwPercentField::set_max(sal_Int64 nNewMax, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        m_pField->set_max(nNewMax, eInUnit);
}

sal_Int64 SwPercentField::NormalizePercent(sal_Int64 nValue)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        nValue = m_pField->normalize(nValue);
    else
        nValue = nValue * ImpPower10(m_nOldDigits);
    return nValue;
}

sal_Int64 SwPercentField::DenormalizePercent(sal_Int64 nValue)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        nValue = m_pField->denormalize(nValue);
    else
    {
        int nFactor = ImpPower10(m_nOldDigits);
        nValue = ((nValue + (nFactor / 2)) / nFactor);
    }
    return nValue;
}

int SwPercentField::ImpPower10(sal_uInt16 n)
{
    int nValue = 1;

    for (sal_uInt16 i = 0; i < n; ++i)
        nValue *= 10;

    return nValue;
}

sal_Int64 SwPercentField::GetRealValue(FieldUnit eOutUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        return get_value(eOutUnit);
    else
        return Convert(get_value(), m_pField->get_unit(), eOutUnit);
}

sal_Int64 SwPercentField::Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if (eInUnit == eOutUnit || (eInUnit == FieldUnit::NONE && eOutUnit == m_pField->get_unit())
        || (eOutUnit == FieldUnit::NONE && eInUnit == m_pField->get_unit()))
        return nValue;

    if (eInUnit == FieldUnit::PERCENT)
    {
        // Convert to metric
        sal_Int64 nTwipValue = (m_nRefValue * nValue + 50) / 100;

        if (eOutUnit == FieldUnit::TWIP) // Only convert if necessary
            return NormalizePercent(nTwipValue);
        else
            return vcl::ConvertValue(NormalizePercent(nTwipValue), 0, m_nOldDigits, FieldUnit::TWIP,
                                     eOutUnit);
    }

    if (eOutUnit == FieldUnit::PERCENT)
    {
        // Convert to percent
        sal_Int64 nCurrentWidth;
        nValue = DenormalizePercent(nValue);

        if (eInUnit == FieldUnit::TWIP) // Only convert if necessary
            nCurrentWidth = nValue;
        else
            nCurrentWidth = vcl::ConvertValue(nValue, 0, m_nOldDigits, eInUnit, FieldUnit::TWIP);
        nCurrentWidth = UpscaleTwoDecimalPlaces(nCurrentWidth, m_nOldDigits);
        // Round to 0.5 percent
        return m_nRefValue ? (((nCurrentWidth * 1000) / m_nRefValue + 5) / 10) : 0;
    }

    return vcl::ConvertValue(nValue, 0, m_nOldDigits, eInUnit, eOutUnit);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
