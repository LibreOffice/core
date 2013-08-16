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
#ifndef _PRCNTFLD_HXX
#define _PRCNTFLD_HXX

#include <vcl/field.hxx>
#include "swdllapi.h"
#include "uitool.hxx"

//Wraps a MetricField with extra features, preferred to PercentField
class SW_DLLPUBLIC PercentFieldWrap
{
    MetricField* m_pField;

    sal_Int64   nRefValue;      // 100% value for conversion (in Twips)
    sal_Int64   nOldMax;
    sal_Int64   nOldMin;
    sal_Int64   nOldSpinSize;
    sal_Int64   nOldBaseValue;
    sal_Int64   nLastPercent;
    sal_Int64   nLastValue;
    sal_uInt16  nOldDigits;
    FieldUnit   eOldUnit;
    bool bLockAutoCalculation; //prevent recalcution of percent values when the
                               //reference value is changed

    SW_DLLPRIVATE sal_Int64      ImpPower10(sal_uInt16 n);

public:

    PercentFieldWrap();
    void set(MetricField *pField);
    const MetricField* get() const { return m_pField; }
    MetricField* get() { return m_pField; }
    void SetUpHdl(const Link& rLink) { m_pField->SetUpHdl(rLink); }
    void SetDownHdl(const Link& rLink) { m_pField->SetDownHdl(rLink); }
    void SetModifyHdl(const Link& rLink) { m_pField->SetModifyHdl(rLink); }
    void SetLoseFocusHdl(const Link& rLink) { m_pField->SetLoseFocusHdl(rLink); }
    void SetMetric(FieldUnit eUnit) { ::SetMetric(*m_pField, eUnit); }
    void Enable(bool bEnable = true, bool bChild = true) { m_pField->Enable(bEnable, bChild); }
    void Disable(bool bChild = true) { m_pField->Disable(bChild); }
    bool HasFocus() const { return m_pField->HasFocus(); }
    void SetAccessibleName(const OUString& rName) { m_pField->SetAccessibleName(rName); }
    void SetText(const OUString& rStr) { m_pField->SetText(rStr); }
    void SaveValue() { m_pField->SaveValue(); }
    void ClearModifyFlag() { m_pField->ClearModifyFlag(); }
    OUString GetSavedValue() const { return m_pField->GetSavedValue(); }
    OUString GetText() const { return m_pField->GetText(); }
    void SetMetricFieldMin(sal_Int64 nNewMin) { m_pField->SetMin(nNewMin); }
    void SetMetricFieldMax(sal_Int64 nNewMax) { m_pField->SetMax(nNewMax); }

    void SetValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE) { m_pField->SetValue(nNewValue, eInUnit); }

    void SetLast(sal_Int64 nNewLast) { m_pField->SetLast(nNewLast); }

    void SetPrcntValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    void SetUserValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    void SetBaseValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    sal_Int64 GetValue(FieldUnit eOutUnit = FUNIT_NONE);

    bool IsValueModified();

    void SetMax(sal_Int64 nNewMax, FieldUnit eInUnit = FUNIT_NONE);

    void SetMin(sal_Int64 nNewMin, FieldUnit eInUnit = FUNIT_NONE);

    sal_Int64 GetMin() const { return m_pField->GetMin(); }
    sal_Int64 GetMax() const { return m_pField->GetMax(); }

    sal_Int64 NormalizePercent(sal_Int64 nValue);
    sal_Int64 DenormalizePercent(sal_Int64 nValue);

    sal_Int64 Normalize( sal_Int64 nValue ) const { return m_pField->Normalize(nValue); }
    sal_Int64 Denormalize( sal_Int64 nValue ) const { return m_pField->Denormalize(nValue); }

    void SetRefValue(sal_Int64 nValue);
    sal_Int64 GetRefValue() const { return nRefValue; }
    sal_Int64 GetRealValue(FieldUnit eOutUnit = FUNIT_NONE);

    sal_Int64 Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void ShowPercent(bool bPercent);

    sal_uInt16 GetOldDigits() const {return nOldDigits;}

    void LockAutoCalculation(bool bLock) {bLockAutoCalculation = bLock;}
    bool IsAutoCalculationLocked() const {return bLockAutoCalculation;}
};


class SW_DLLPUBLIC PercentField : public MetricField
{
    sal_Int64   nRefValue;      // 100% value for conversion (in Twips)
    sal_Int64   nOldMax;
    sal_Int64   nOldMin;
    sal_Int64   nOldSpinSize;
    sal_Int64   nOldBaseValue;
    sal_Int64   nLastPercent;
    sal_Int64   nLastValue;
    sal_uInt16      nOldDigits;
    FieldUnit   eOldUnit;
    sal_Bool    bLockAutoCalculation; //prevent recalcution of percent values when the
                                        //reference value is changed

    SW_DLLPRIVATE sal_Int64      ImpPower10(sal_uInt16 n);

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

    sal_Bool        IsValueModified();

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

    void        ShowPercent(sal_Bool bPercent);

    sal_uInt16      GetOldDigits() const {return nOldDigits;}

    void        LockAutoCalculation(sal_Bool bLock) {bLockAutoCalculation = bLock;}
    sal_Bool    IsAutoCalculationLocked()const {return bLockAutoCalculation;}
};

#endif // _PRCNTFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
