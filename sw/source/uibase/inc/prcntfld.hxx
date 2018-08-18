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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_PRCNTFLD_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_PRCNTFLD_HXX

#include <svtools/unitconv.hxx>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>
#include <swdllapi.h>
#include "uitool.hxx"

class SW_DLLPUBLIC PercentField
{
    VclPtr<MetricField> m_pField;

    sal_Int64   nRefValue;      // 100% value for conversion (in Twips)
    sal_Int64   nOldMax;
    sal_Int64   nOldMin;
    sal_Int64   nOldSpinSize;
    sal_Int64   nOldBaseValue;
    sal_Int64   nLastPercent;
    sal_Int64   nLastValue;
    sal_uInt16  nOldDigits;
    FieldUnit   eOldUnit;
    bool bLockAutoCalculation; //prevent recalculation of percent values when the
                               //reference value is changed

    SAL_DLLPRIVATE static sal_Int64 ImpPower10(sal_uInt16 n);

public:

    PercentField();
    void set(MetricField *pField);
    const MetricField* get() const { return m_pField; }
    MetricField* get() { return m_pField; }
    void SetModifyHdl(const Link<Edit&,void>& rLink) { m_pField->SetModifyHdl(rLink); }
    void SetLoseFocusHdl(const Link<Control&,void>& rLink) { m_pField->SetLoseFocusHdl(rLink); }
    void SetMetric(FieldUnit eUnit) { ::SetMetric(*m_pField, eUnit); }
    void Enable(bool bEnable = true) { m_pField->Enable(bEnable); }
    void Disable() { m_pField->Disable(); }
    void SetAccessibleName(const OUString& rName) { m_pField->SetAccessibleName(rName); }
    void SetText(const OUString& rStr) { m_pField->SetText(rStr); }
    void SetMetricFieldMin(sal_Int64 nNewMin) { m_pField->SetMin(nNewMin); }

    void SetValue(sal_Int64 nNewValue) { m_pField->SetValue(nNewValue, FUNIT_NONE); }

    void SetPrcntValue(sal_Int64 nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    void SetUserValue(sal_Int64 nNewValue, FieldUnit eInUnit);

    void SetBaseValue(sal_Int64 nNewValue, FieldUnit eInUnit);

    sal_Int64 GetValue(FieldUnit eOutUnit = FUNIT_NONE);

    bool IsValueModified();

    void SetMax(sal_Int64 nNewMax, FieldUnit eInUnit);

    void SetMin(sal_Int64 nNewMin, FieldUnit eInUnit);

    sal_Int64 GetMin() const { return m_pField->GetMin(); }

    sal_Int64 NormalizePercent(sal_Int64 nValue);
    sal_Int64 DenormalizePercent(sal_Int64 nValue);

    sal_Int64 Normalize( sal_Int64 nValue ) const { return m_pField->Normalize(nValue); }

    void SetRefValue(sal_Int64 nValue);
    sal_Int64 GetRealValue(FieldUnit eOutUnit);

    sal_Int64 Convert(sal_Int64 nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void ShowPercent(bool bPercent);

    void LockAutoCalculation(bool bLock) {bLockAutoCalculation = bLock;}
};

class SW_DLLPUBLIC SwPercentField
{
    std::unique_ptr<weld::MetricSpinButton> m_pField;

    int nRefValue;      // 100% value for conversion (in Twips)
    int nOldMax;
    int nOldMin;
    int nOldSpinSize;
    int nOldPageSize;
    int nLastPercent;
    int nLastValue;
    sal_uInt16  nOldDigits;
    FieldUnit   eOldUnit;

    SAL_DLLPRIVATE static int ImpPower10(sal_uInt16 n);

public:

    SwPercentField(std::unique_ptr<weld::MetricSpinButton> pControl);
    const weld::MetricSpinButton* get() const { return m_pField.get(); }
    weld::MetricSpinButton* get() { return m_pField.get(); }
    void connect_value_changed(const Link<weld::MetricSpinButton&, void>& rLink) { m_pField->connect_value_changed(rLink); }
    void SetMetric(FieldUnit eUnit) { ::SetFieldUnit(*m_pField, eUnit); }
    void set_sensitive(bool bEnable) { m_pField->set_sensitive(bEnable); }
    bool has_focus() const { return m_pField->has_focus(); }
    void save_value() { m_pField->save_value(); }
    bool get_value_changed_from_saved() const { return m_pField->get_value_changed_from_saved(); }
    void set_text(const OUString& rStr) { m_pField->set_text(rStr); }
    void SetMetricFieldMin(int nNewMin) { m_pField->set_min(nNewMin, FUNIT_NONE); }
    void SetMetricFieldMax(int nNewMax) { m_pField->set_max(nNewMax, FUNIT_NONE); }

    void SetPrcntValue(int nNewValue, FieldUnit eInUnit = FUNIT_NONE);

    int get_value(FieldUnit eOutUnit = FUNIT_NONE);

    void set_min(int nNewMin, FieldUnit eInUnit);
    void set_max(int nNewMax, FieldUnit eInUnit);

    int NormalizePercent(int nValue);
    int DenormalizePercent(int nValue);

    void SetRefValue(int nValue);
    int GetRealValue(FieldUnit eOutUnit);

    int Convert(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void ShowPercent(bool bPercent);

};


#endif // INCLUDED_SW_SOURCE_UIBASE_INC_PRCNTFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
