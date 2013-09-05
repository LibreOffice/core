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


#include "prcntfld.hxx"

PercentField::PercentField()
    : m_pField(NULL)
    , nRefValue(0)
    , nOldMax(0)
    , nOldMin(0)
    , nOldSpinSize(0)
    , nOldBaseValue(0)
    , nLastPercent(-1)
    , nLastValue(-1)
    , nOldDigits(0)
    , eOldUnit(FUNIT_NONE)
    , bLockAutoCalculation(sal_False)
{
}

void PercentField::set(MetricField *pField)
{
    m_pField = pField;
    nOldSpinSize = m_pField->GetSpinSize();
    nRefValue = DenormalizePercent(m_pField->GetMax(FUNIT_TWIP));
    nOldDigits = m_pField->GetDecimalDigits();
    m_pField->SetCustomUnitText(OUString('%'));
}

void PercentField::SetRefValue(sal_Int64 nValue)
{
    sal_Int64 nRealValue = GetRealValue(eOldUnit);

    nRefValue = nValue;

    if (!bLockAutoCalculation && (m_pField->GetUnit() == FUNIT_CUSTOM))
        SetPrcntValue(nRealValue, eOldUnit);
}

void PercentField::ShowPercent(bool bPercent)
{
    if ((bPercent && m_pField->GetUnit() == FUNIT_CUSTOM) ||
        (!bPercent && m_pField->GetUnit() != FUNIT_CUSTOM))
        return;

    sal_Int64 nOldValue;

    if (bPercent)
    {
        sal_Int64 nAktWidth, nPercent;

        nOldValue = GetValue();

        eOldUnit = m_pField->GetUnit();
        nOldDigits = m_pField->GetDecimalDigits();
        nOldMin = m_pField->GetMin();
        nOldMax = m_pField->GetMax();
        nOldSpinSize = m_pField->GetSpinSize();
        nOldBaseValue = m_pField->GetBaseValue();
        m_pField->SetUnit(FUNIT_CUSTOM);
        m_pField->SetDecimalDigits( 0 );

        nAktWidth = m_pField->ConvertValue(nOldMin, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        // round to 0.5 percent
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;

        m_pField->SetMin(std::max(static_cast< sal_Int64 >(1), nPercent));
        m_pField->SetMax(100);
        m_pField->SetSpinSize(5);
        m_pField->SetBaseValue(0);
        if (nOldValue != nLastValue)
        {
            nAktWidth = m_pField->ConvertValue(nOldValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
            nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
            m_pField->SetValue(nPercent);
            nLastPercent = nPercent;
            nLastValue = nOldValue;
        }
        else
            m_pField->SetValue(nLastPercent);
    }
    else
    {
        sal_Int64 nOldPercent = GetValue(FUNIT_CUSTOM);

        nOldValue = Convert(GetValue(), m_pField->GetUnit(), eOldUnit);

        m_pField->SetUnit(eOldUnit);
        m_pField->SetDecimalDigits(nOldDigits);
        m_pField->SetMin(nOldMin);
        m_pField->SetMax(nOldMax);
        m_pField->SetSpinSize(nOldSpinSize);
        m_pField->SetBaseValue(nOldBaseValue);

        if (nOldPercent != nLastPercent)
        {
            SetPrcntValue(nOldValue, eOldUnit);
            nLastPercent = nOldPercent;
            nLastValue = nOldValue;
        }
        else
            SetPrcntValue(nLastValue, eOldUnit);
    }
}

void PercentField::SetPrcntValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        m_pField->SetValue(Convert(nNewValue, eInUnit, m_pField->GetUnit()));
    else
    {
        // Overwrite output value, do not restore later
        sal_Int64 nPercent, nAktWidth;
        if(eInUnit == FUNIT_TWIP)
        {
            nAktWidth = m_pField->ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            sal_Int64 nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = m_pField->ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        m_pField->SetValue(nPercent);
    }
}

void PercentField::SetBaseValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (m_pField->GetUnit() == FUNIT_CUSTOM)
        nOldBaseValue = m_pField->ConvertValue(nNewValue, 0, nOldDigits, eInUnit, eOldUnit);
    else
        m_pField->SetBaseValue(nNewValue, eInUnit);
}

sal_Int64 PercentField::GetValue(FieldUnit eOutUnit)
{
    return Convert(m_pField->GetValue(), m_pField->GetUnit(), eOutUnit);
}

void PercentField::SetMin(sal_Int64 nNewMin, FieldUnit eInUnit)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM)
        m_pField->SetMin(nNewMin, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
        nOldMin = Convert(nNewMin, eInUnit, eOldUnit);

        sal_Int64 nPercent = Convert(nNewMin, eInUnit, FUNIT_CUSTOM);
        m_pField->SetMin(std::max( static_cast< sal_Int64 >(1), nPercent));
    }
}

void PercentField::SetMax(sal_Int64 nNewMax, FieldUnit eInUnit)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM)
        m_pField->SetMax(nNewMax, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
    }
}

sal_Int64 PercentField::NormalizePercent(sal_Int64 nValue)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM)
        nValue = m_pField->Normalize(nValue);
    else
        nValue = nValue * ImpPower10(nOldDigits);
    return nValue;
}

sal_Int64 PercentField::DenormalizePercent(sal_Int64 nValue)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM)
        nValue = m_pField->Denormalize(nValue);
    else
    {
        sal_Int64 nFactor = ImpPower10(nOldDigits);
        nValue = ((nValue+(nFactor/2)) / nFactor);
    }
    return nValue;
}

bool PercentField::IsValueModified()
{
    if (m_pField->GetUnit() == FUNIT_CUSTOM)
        return true;
    else
        return m_pField->IsValueModified();
}

void PercentField::SetUserValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        m_pField->SetUserValue(Convert(nNewValue, eInUnit, m_pField->GetUnit()),FUNIT_NONE);
    else
    {
        // Overwrite output value, do not restore later
        sal_Int64 nPercent, nAktWidth;
        if (eInUnit == FUNIT_TWIP)
        {
            nAktWidth = m_pField->ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            sal_Int64 nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = m_pField->ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        m_pField->SetUserValue(nPercent,FUNIT_NONE);
    }
}

sal_Int64 PercentField::ImpPower10(sal_uInt16 n)
{
    sal_Int64 nValue = 1;

    for (sal_uInt16 i=0; i < n; ++i)
        nValue *= 10;

    return nValue;
}

sal_Int64 PercentField::GetRealValue(FieldUnit eOutUnit)
{
    if (m_pField->GetUnit() != FUNIT_CUSTOM)
        return GetValue(eOutUnit);
    else
        return Convert(GetValue(), m_pField->GetUnit(), eOutUnit);
}

sal_Int64 PercentField::Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if (eInUnit == eOutUnit ||
        (eInUnit == FUNIT_NONE && eOutUnit == m_pField->GetUnit()) ||
        (eOutUnit == FUNIT_NONE && eInUnit == m_pField->GetUnit()))
        return nValue;

    if (eInUnit == FUNIT_CUSTOM)
    {
        // Convert to metric
        sal_Int64 nTwipValue = (nRefValue * nValue + 50) / 100;

        if (eOutUnit == FUNIT_TWIP) // Only convert if necessary
            return NormalizePercent(nTwipValue);
        else
            return m_pField->ConvertValue(NormalizePercent(nTwipValue), 0, nOldDigits, FUNIT_TWIP, eOutUnit);
    }

    if (eOutUnit == FUNIT_CUSTOM)
    {
        // Convert to percent
        sal_Int64 nAktWidth;
        nValue = DenormalizePercent(nValue);

        if (eInUnit == FUNIT_TWIP)  // Only convert if necessary
            nAktWidth = nValue;
        else
            nAktWidth = m_pField->ConvertValue(nValue, 0, nOldDigits, eInUnit, FUNIT_TWIP);
        // Round to 0.5 percent
        return ((nAktWidth * 1000) / nRefValue + 5) / 10;
    }

    return m_pField->ConvertValue(nValue, 0, nOldDigits, eInUnit, eOutUnit);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
