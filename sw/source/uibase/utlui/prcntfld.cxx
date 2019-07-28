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
#include <vcl/field.hxx>

SwPercentField::SwPercentField(std::unique_ptr<weld::MetricSpinButton> pControl)
    : m_pField(std::move(pControl))
    , nOldMax(0)
    , nOldMin(0)
    , nLastPercent(-1)
    , nLastValue(-1)
    , nOldDigits(m_pField->get_digits())
    , eOldUnit(FieldUnit::NONE)
    , bLockAutoCalculation(false)
{
    int nMin, nMax;
    m_pField->get_range(nMin, nMax, FieldUnit::TWIP);
    nRefValue = DenormalizePercent(nMax);
    m_pField->get_increments(nOldSpinSize, nOldPageSize, FieldUnit::NONE);
}

void SwPercentField::SetRefValue(int nValue)
{
    int nRealValue = GetRealValue(eOldUnit);

    nRefValue = nValue;

    if (!bLockAutoCalculation && (m_pField->get_unit() == FieldUnit::PERCENT))
        set_value(nRealValue, eOldUnit);
}

void SwPercentField::ShowPercent(bool bPercent)
{
    if ((bPercent && m_pField->get_unit() == FieldUnit::PERCENT) ||
        (!bPercent && m_pField->get_unit() != FieldUnit::PERCENT))
        return;

    int nOldValue;

    if (bPercent)
    {
        nOldValue = get_value();

        eOldUnit = m_pField->get_unit();
        nOldDigits = m_pField->get_digits();
        m_pField->get_range(nOldMin, nOldMax, FieldUnit::NONE);
        m_pField->get_increments(nOldSpinSize, nOldPageSize, FieldUnit::NONE);
        m_pField->set_unit(FieldUnit::PERCENT);
        m_pField->set_digits(0);

        int nCurrentWidth = MetricField::ConvertValue(nOldMin, 0, nOldDigits, eOldUnit, FieldUnit::TWIP);
        // round to 0.5 percent
        int nPercent = nRefValue ? (((nCurrentWidth * 10) / nRefValue + 5) / 10) : 0;

        m_pField->set_range(std::max(1, nPercent), 100, FieldUnit::NONE);
        m_pField->set_increments(5, 10, FieldUnit::NONE);
        if (nOldValue != nLastValue)
        {
            nCurrentWidth = MetricField::ConvertValue(nOldValue, 0, nOldDigits, eOldUnit, FieldUnit::TWIP);
            nPercent = nRefValue ? (((nCurrentWidth * 10) / nRefValue + 5) / 10) : 0;
            m_pField->set_value(nPercent, FieldUnit::NONE);
            nLastPercent = nPercent;
            nLastValue = nOldValue;
        }
        else
            m_pField->set_value(nLastPercent, FieldUnit::NONE);
    }
    else
    {
        int nOldPercent = get_value(FieldUnit::PERCENT);

        nOldValue = Convert(get_value(), m_pField->get_unit(), eOldUnit);

        m_pField->set_unit(eOldUnit);
        m_pField->set_digits(nOldDigits);
        m_pField->set_range(nOldMin, nOldMax, FieldUnit::NONE);
        m_pField->set_increments(nOldSpinSize, nOldPageSize, FieldUnit::NONE);

        if (nOldPercent != nLastPercent)
        {
            set_value(nOldValue, eOldUnit);
            nLastPercent = nOldPercent;
            nLastValue = nOldValue;
        }
        else
            set_value(nLastValue, eOldUnit);
    }
}

void SwPercentField::set_value(int nNewValue, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT || eInUnit == FieldUnit::PERCENT)
        m_pField->set_value(Convert(nNewValue, eInUnit, m_pField->get_unit()), FieldUnit::NONE);
    else
    {
        // Overwrite output value, do not restore later
        int nPercent, nCurrentWidth;
        if(eInUnit == FieldUnit::TWIP)
        {
            nCurrentWidth = MetricField::ConvertValue(nNewValue, 0, nOldDigits, FieldUnit::TWIP, FieldUnit::TWIP);
        }
        else
        {
            int nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nCurrentWidth = MetricField::ConvertValue(nValue, 0, nOldDigits, eOldUnit, FieldUnit::TWIP);
        }
        nPercent = nRefValue ? (((nCurrentWidth * 10) / nRefValue + 5) / 10) : 0;
        m_pField->set_value(nPercent, FieldUnit::NONE);
    }
}

int SwPercentField::get_value(FieldUnit eOutUnit)
{
    return Convert(m_pField->get_value(FieldUnit::NONE), m_pField->get_unit(), eOutUnit);
}

void SwPercentField::set_min(int nNewMin, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        m_pField->set_min(nNewMin, eInUnit);
    else
    {
        if (eInUnit == FieldUnit::NONE)
            eInUnit = eOldUnit;
        nOldMin = Convert(nNewMin, eInUnit, eOldUnit);

        int nPercent = Convert(nNewMin, eInUnit, FieldUnit::PERCENT);
        m_pField->set_min(std::max(1, nPercent), FieldUnit::NONE);
    }
}

void SwPercentField::set_max(int nNewMax, FieldUnit eInUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        m_pField->set_max(nNewMax, eInUnit);
}

int SwPercentField::NormalizePercent(int nValue)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        nValue = m_pField->normalize(nValue);
    else
        nValue = nValue * ImpPower10(nOldDigits);
    return nValue;
}

int SwPercentField::DenormalizePercent(int nValue)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        nValue = m_pField->denormalize(nValue);
    else
    {
        int nFactor = ImpPower10(nOldDigits);
        nValue = ((nValue+(nFactor/2)) / nFactor);
    }
    return nValue;
}

int SwPercentField::ImpPower10(sal_uInt16 n)
{
    int nValue = 1;

    for (sal_uInt16 i=0; i < n; ++i)
        nValue *= 10;

    return nValue;
}

int SwPercentField::GetRealValue(FieldUnit eOutUnit)
{
    if (m_pField->get_unit() != FieldUnit::PERCENT)
        return get_value(eOutUnit);
    else
        return Convert(get_value(), m_pField->get_unit(), eOutUnit);
}

int SwPercentField::Convert(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if (eInUnit == eOutUnit ||
        (eInUnit == FieldUnit::NONE && eOutUnit == m_pField->get_unit()) ||
        (eOutUnit == FieldUnit::NONE && eInUnit == m_pField->get_unit()))
        return nValue;

    if (eInUnit == FieldUnit::PERCENT)
    {
        // Convert to metric
        int nTwipValue = (nRefValue * nValue + 50) / 100;

        if (eOutUnit == FieldUnit::TWIP) // Only convert if necessary
            return NormalizePercent(nTwipValue);
        else
            return MetricField::ConvertValue(NormalizePercent(nTwipValue), 0, nOldDigits, FieldUnit::TWIP, eOutUnit);
    }

    if (eOutUnit == FieldUnit::PERCENT)
    {
        // Convert to percent
        int nCurrentWidth;
        nValue = DenormalizePercent(nValue);

        if (eInUnit == FieldUnit::TWIP)  // Only convert if necessary
            nCurrentWidth = nValue;
        else
            nCurrentWidth = MetricField::ConvertValue(nValue, 0, nOldDigits, eInUnit, FieldUnit::TWIP);
        // Round to 0.5 percent
        return nRefValue ? (((nCurrentWidth * 1000) / nRefValue + 5) / 10) : 0;
    }

    return MetricField::ConvertValue(nValue, 0, nOldDigits, eInUnit, eOutUnit);
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
