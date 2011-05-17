/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "prcntfld.hxx"

PercentField::PercentField( Window* pWin, const ResId& rResId ) :
        MetricField ( pWin, rResId ),

        nOldMax     (0),
        nOldMin     (0),
        nLastPercent(-1),
        nLastValue  (-1),
        eOldUnit    (FUNIT_NONE),
        bLockAutoCalculation(sal_False)
{

    nOldSpinSize = GetSpinSize();
    nRefValue = DenormalizePercent(MetricField::GetMax(FUNIT_TWIP));
    nOldDigits = GetDecimalDigits();
    SetCustomUnitText('%');
}

void PercentField::SetRefValue(sal_Int64 nValue)
{
    sal_Int64 nRealValue = GetRealValue(eOldUnit);

    nRefValue = nValue;

    if (!bLockAutoCalculation && (GetUnit() == FUNIT_CUSTOM))
        SetPrcntValue(nRealValue, eOldUnit);
}

void PercentField::ShowPercent(sal_Bool bPercent)
{
    if ((bPercent && GetUnit() == FUNIT_CUSTOM) ||
        (!bPercent && GetUnit() != FUNIT_CUSTOM))
        return;

    sal_Int64 nOldValue;

    if (bPercent)
    {
        sal_Int64 nAktWidth, nPercent;

        nOldValue = GetValue();

        eOldUnit = GetUnit();
        nOldDigits = GetDecimalDigits();
        nOldMin = GetMin();
        nOldMax = GetMax();
        nOldSpinSize = GetSpinSize();
        nOldBaseValue = GetBaseValue();
        SetUnit(FUNIT_CUSTOM);
        SetDecimalDigits( 0 );

        nAktWidth = ConvertValue(nOldMin, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        // round to 0.5 percent
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;

        MetricField::SetMin(Max(static_cast< sal_Int64 >(1), nPercent));
        MetricField::SetMax(100);
        SetSpinSize(5);
        MetricField::SetBaseValue(0);
        if (nOldValue != nLastValue)
        {
            nAktWidth = ConvertValue(nOldValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
            nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
            MetricFormatter::SetValue(nPercent);
            nLastPercent = nPercent;
            nLastValue = nOldValue;
        }
        else
            MetricFormatter::SetValue(nLastPercent);
    }
    else
    {
        sal_Int64 nOldPercent = GetValue(FUNIT_CUSTOM);

        nOldValue = Convert(GetValue(), GetUnit(), eOldUnit);

        SetUnit(eOldUnit);
        SetDecimalDigits(nOldDigits);
        MetricField::SetMin(nOldMin);
        MetricField::SetMax(nOldMax);
        SetSpinSize(nOldSpinSize);
        MetricField::SetBaseValue(nOldBaseValue);

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

void PercentField::SetValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
   MetricFormatter::SetValue(nNewValue, eInUnit);
}

void PercentField::SetPrcntValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        MetricFormatter::SetValue(Convert(nNewValue, eInUnit, GetUnit()));

    else
    {
        // Ausgangswert ueberschreiben, nicht spaeter restaurieren
        sal_Int64 nPercent, nAktWidth;
        if(eInUnit == FUNIT_TWIP)
        {
            nAktWidth = ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            sal_Int64 nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        MetricFormatter::SetValue(nPercent);
    }
}

void PercentField::SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit )
{
    if (GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        MetricField::SetUserValue(Convert(nNewValue, eInUnit, GetUnit()),FUNIT_NONE);

    else
    {
        // Ausgangswert ueberschreiben, nicht spaeter restaurieren
        sal_Int64 nPercent, nAktWidth;
        if(eInUnit == FUNIT_TWIP)
        {
            nAktWidth = ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            sal_Int64 nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        MetricField::SetUserValue(nPercent,FUNIT_NONE);
    }

}

void PercentField::SetBaseValue(sal_Int64 nNewValue, FieldUnit eInUnit)
{
    if (GetUnit() == FUNIT_CUSTOM)
        nOldBaseValue = ConvertValue(nNewValue, 0, nOldDigits, eInUnit, eOldUnit);
    else
        MetricField::SetBaseValue(nNewValue, eInUnit);
}

sal_Int64 PercentField::GetValue( FieldUnit eOutUnit )
{
    return Convert(MetricField::GetValue(), GetUnit(), eOutUnit);
}

void PercentField::SetMin(sal_Int64 nNewMin, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        MetricField::SetMin(nNewMin, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
        nOldMin = Convert(nNewMin, eInUnit, eOldUnit);

        sal_Int64 nPercent = Convert(nNewMin, eInUnit, FUNIT_CUSTOM);
        MetricField::SetMin(Max( static_cast< sal_Int64 >(1), nPercent));
    }
}

void PercentField::SetMax(sal_Int64 nNewMax, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        MetricField::SetMax(nNewMax, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
    }
}

sal_Int64 PercentField::NormalizePercent(sal_Int64 nValue)
{
    if (GetUnit() != FUNIT_CUSTOM)
        nValue = MetricField::Normalize(nValue);
    else
        nValue = nValue * ImpPower10(nOldDigits);

    return nValue;
}

sal_Int64 PercentField::DenormalizePercent(sal_Int64 nValue)
{
    if (GetUnit() != FUNIT_CUSTOM)
        nValue = MetricField::Denormalize(nValue);
    else
    {
        sal_Int64 nFactor = ImpPower10(nOldDigits);
        nValue = ((nValue+(nFactor/2)) / nFactor);
    }

    return nValue;
}

sal_Bool PercentField::IsValueModified()
{
    if (GetUnit() == FUNIT_CUSTOM)
        return sal_True;
    else
        return MetricField::IsValueModified();
}

sal_Int64 PercentField::ImpPower10( sal_uInt16 n )
{
    sal_uInt16 i;
    sal_Int64   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

sal_Int64 PercentField::GetRealValue(FieldUnit eOutUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        return GetValue(eOutUnit);
    else
        return Convert(GetValue(), GetUnit(), eOutUnit);
}

sal_Int64 PercentField::Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if (eInUnit == eOutUnit ||
        (eInUnit == FUNIT_NONE && eOutUnit == GetUnit()) ||
        (eOutUnit == FUNIT_NONE && eInUnit == GetUnit()))
        return nValue;

    if (eInUnit == FUNIT_CUSTOM)
    {
        // Umrechnen in Metrik
        sal_Int64 nTwipValue = (nRefValue * nValue + 50) / 100;

        if (eOutUnit == FUNIT_TWIP) // Nur wandeln, wenn unbedingt notwendig
            return NormalizePercent(nTwipValue);
        else
            return ConvertValue(NormalizePercent(nTwipValue), 0, nOldDigits, FUNIT_TWIP, eOutUnit);
    }

    if (eOutUnit == FUNIT_CUSTOM)
    {
        // Umrechnen in Prozent
        sal_Int64 nAktWidth;
        nValue = DenormalizePercent(nValue);

        if (eInUnit == FUNIT_TWIP)  // Nur wandeln, wenn unbedingt notwendig
            nAktWidth = nValue;
        else
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eInUnit, FUNIT_TWIP);
        // Round to 0.5 percent
        return ((nAktWidth * 1000) / nRefValue + 5) / 10;
    }

    return ConvertValue(nValue, 0, nOldDigits, eInUnit, eOutUnit);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
