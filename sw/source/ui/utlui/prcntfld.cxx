/*************************************************************************
 *
 *  $RCSfile: prcntfld.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 18:18:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#pragma hdrstop

#include "prcntfld.hxx"

// STATIC DATA -----------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

PercentField::PercentField( Window* pWin, const ResId& rResId ) :
        MetricField ( pWin, rResId ),

        eOldUnit    (FUNIT_NONE),
        nOldMin     (0),
        nOldMax     (0),
        nLastPercent(-1L),
        nLastValue  (-1L),
        bLockAutoCalculation(sal_False)
{

    nOldSpinSize = GetSpinSize();
    nRefValue = Denormalize(MetricField::GetMax(FUNIT_TWIP));
    nOldDigits = GetDecimalDigits();
    SetCustomUnitText('%');
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::SetRefValue(long nValue)
{
    long nRealValue = GetRealValue(eOldUnit);

    nRefValue = nValue;

    if (!bLockAutoCalculation && (GetUnit() == FUNIT_CUSTOM))
        SetPrcntValue(nRealValue, eOldUnit);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::ShowPercent(BOOL bPercent)
{
    if ((bPercent && GetUnit() == FUNIT_CUSTOM) ||
        (!bPercent && GetUnit() != FUNIT_CUSTOM))
        return;

    long nOldValue;

    if (bPercent)
    {
        long nAktWidth, nPercent;

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
        // Um 0.5 Prozent aufrunden
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;

        MetricField::SetMin(Max(1L, nPercent));
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
//      SetValue(100, FUNIT_CUSTOM);
    }
    else
    {
        long nOldPercent = GetValue(FUNIT_CUSTOM);

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
void PercentField::SetValue(long nNewValue, FieldUnit eInUnit)
{
   MetricFormatter::SetValue(nNewValue, eInUnit);
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
void PercentField::SetPrcntValue(long nNewValue, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        MetricFormatter::SetValue(Convert(nNewValue, eInUnit, GetUnit()));

    else
    {
        // Ausgangswert ueberschreiben, nicht spaeter restaurieren
        long nPercent, nAktWidth;
        if(eInUnit == FUNIT_TWIP)
        {
            nAktWidth = ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            long nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        MetricFormatter::SetValue(nPercent);
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::SetUserValue( long nNewValue, FieldUnit eInUnit )
{
    if (GetUnit() != FUNIT_CUSTOM || eInUnit == FUNIT_CUSTOM)
        MetricField::SetUserValue(Convert(nNewValue, eInUnit, GetUnit()),FUNIT_NONE);

    else
    {
        // Ausgangswert ueberschreiben, nicht spaeter restaurieren
        long nPercent, nAktWidth;
        if(eInUnit == FUNIT_TWIP)
        {
            nAktWidth = ConvertValue(nNewValue, 0, nOldDigits, FUNIT_TWIP, FUNIT_TWIP);
        }
        else
        {
            long nValue = Convert(nNewValue, eInUnit, eOldUnit);
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eOldUnit, FUNIT_TWIP);
        }
        nPercent = ((nAktWidth * 10) / nRefValue + 5) / 10;
        MetricField::SetUserValue(nPercent,FUNIT_NONE);
    }

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::SetBaseValue(long nNewValue, FieldUnit eInUnit)
{
    if (GetUnit() == FUNIT_CUSTOM)
        nOldBaseValue = ConvertValue(nNewValue, 0, nOldDigits, eInUnit, eOldUnit);
    else
        MetricField::SetBaseValue(nNewValue, eInUnit);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::GetValue( FieldUnit eOutUnit )
{
    return Convert(MetricField::GetValue(), GetUnit(), eOutUnit);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::SetMin(long nNewMin, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        MetricField::SetMin(nNewMin, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
        nOldMin = Convert(nNewMin, eInUnit, eOldUnit);

        long nPercent = Convert(nNewMin, eInUnit, FUNIT_CUSTOM);
        MetricField::SetMin(Max(1L, nPercent));
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void PercentField::SetMax(long nNewMax, FieldUnit eInUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        MetricField::SetMax(nNewMax, eInUnit);
    else
    {
        if (eInUnit == FUNIT_NONE)
            eInUnit = eOldUnit;
//      SetRefValue(Convert(nNewMax, eInUnit, FUNIT_TWIP));
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::Normalize(long nValue)
{
    if (GetUnit() != FUNIT_CUSTOM)
        nValue = MetricField::Normalize(nValue);
    else
        nValue = nValue * ImpPower10(nOldDigits);

    return nValue;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::Denormalize(long nValue)
{
    if (GetUnit() != FUNIT_CUSTOM)
        nValue = MetricField::Denormalize(nValue);
    else
    {
        long nFactor = ImpPower10(nOldDigits);
        nValue = ((nValue+(nFactor/2)) / nFactor);
    }

    return nValue;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL PercentField::IsValueModified()
{
    if (GetUnit() == FUNIT_CUSTOM)
        return TRUE;
    else
        return MetricField::IsValueModified();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::ImpPower10( USHORT n )
{
    USHORT i;
    long   nValue = 1;

    for ( i=0; i < n; i++ )
        nValue *= 10;

    return nValue;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::GetRealValue(FieldUnit eOutUnit)
{
    if (GetUnit() != FUNIT_CUSTOM)
        return GetValue(eOutUnit);
    else
        return Convert(GetValue(), GetUnit(), eOutUnit);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

long PercentField::Convert(long nValue, FieldUnit eInUnit, FieldUnit eOutUnit)
{
    if (eInUnit == eOutUnit ||
        (eInUnit == FUNIT_NONE && eOutUnit == GetUnit()) ||
        (eOutUnit == FUNIT_NONE && eInUnit == GetUnit()))
        return nValue;

    if (eInUnit == FUNIT_CUSTOM)
    {
        // Umrechnen in Metrik
        long nTwipValue = (nRefValue * nValue + 50) / 100;

        if (eOutUnit == FUNIT_TWIP) // Nur wandeln, wenn unbedingt notwendig
            return Normalize(nTwipValue);
        else
            return ConvertValue(Normalize(nTwipValue), 0, nOldDigits, FUNIT_TWIP, eOutUnit);
    }

    if (eOutUnit == FUNIT_CUSTOM)
    {
        // Umrechnen in Prozent
        long nAktWidth;
        nValue = Denormalize(nValue);

        if (eInUnit == FUNIT_TWIP)  // Nur wandeln, wenn unbedingt notwendig
            nAktWidth = nValue;
        else
            nAktWidth = ConvertValue(nValue, 0, nOldDigits, eInUnit, FUNIT_TWIP);
        // Um 0.5 Prozent runden
        return ((nAktWidth * 1000) / nRefValue + 5) / 10;
    }

    return ConvertValue(nValue, 0, nOldDigits, eInUnit, eOutUnit);
}


