/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prcntfld.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:22:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _PRCNTFLD_HXX
#define _PRCNTFLD_HXX

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SW_DLLPUBLIC PercentField : public MetricField
{
    sal_Int64   nRefValue;      // 100%-Wert fuer Umrechnung (in Twips)
    sal_Int64   nOldMax;
    sal_Int64   nOldMin;
    sal_Int64   nOldSpinSize;
    sal_Int64   nOldBaseValue;
    sal_Int64   nLastPercent;
    sal_Int64   nLastValue;
    USHORT      nOldDigits;
    FieldUnit   eOldUnit;
    sal_Bool    bLockAutoCalculation; //prevent recalcution of percent values when the
                                        //reference value is changed

    SW_DLLPRIVATE sal_Int64      ImpPower10(USHORT n);

    using MetricField::SetValue;
    using MetricField::GetValue;
    using MetricFormatter::SetUserValue;
    using MetricFormatter::SetBaseValue;
    using MetricFormatter::SetMax;
    using MetricFormatter::SetMin;
    using NumericFormatter::IsValueModified;

public:

    virtual void  SetValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    PercentField( Window* pWin, const ResId& rResId );

    void        SetPrcntValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    void        SetUserValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    void        SetBaseValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    sal_Int64        GetValue(FieldUnit eOutUnit = FUNIT_NONE);

    BOOL        IsValueModified();

    //using NumericFormatter::SetMax;
    void        SetMax(sal_Int64 nNewMax, FieldUnit eInUnit = FUNIT_NONE);

    //using NumericFormatter::SetMin;
    void        SetMin(sal_Int64 nNewMin, FieldUnit eInUnit = FUNIT_NONE);

    sal_Int64        NormalizePercent(sal_Int64 nValue);
    sal_Int64        DenormalizePercent(sal_Int64 nValue);

    void        SetRefValue(sal_Int64 nValue);
    inline sal_Int64 GetRefValue() const { return nRefValue; }
    sal_Int64   GetRealValue(FieldUnit eOutUnit = FUNIT_NONE);

    sal_Int64   Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void        ShowPercent(BOOL bPercent);

    USHORT      GetOldDigits() const {return nOldDigits;}

    void        LockAutoCalculation(sal_Bool bLock) {bLockAutoCalculation = bLock;}
    sal_Bool    IsAutoCalculationLocked()const {return bLockAutoCalculation;}
};

#endif // _PRCNTFLD_HXX

