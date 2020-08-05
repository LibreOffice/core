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
#include <vcl/weld.hxx>
#include <swdllapi.h>

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
    bool bLockAutoCalculation; //prevent recalculation of percent values when the
                               //reference value is changed

    SAL_DLLPRIVATE static int ImpPower10(sal_uInt16 n);

public:

    SwPercentField(std::unique_ptr<weld::MetricSpinButton> pControl);
    const weld::MetricSpinButton* get() const { return m_pField.get(); }
    weld::MetricSpinButton* get() { return m_pField.get(); }
    void connect_value_changed(const Link<weld::MetricSpinButton&, void>& rLink) { m_pField->connect_value_changed(rLink); }
    void SetMetric(FieldUnit eUnit) { ::SetFieldUnit(*m_pField, eUnit); }
    void set_sensitive(bool bEnable) { m_pField->set_sensitive(bEnable); }
    void show() { m_pField->show(); }
    bool has_focus() const { return m_pField->has_focus(); }
    void save_value() { m_pField->save_value(); }
    bool get_value_changed_from_saved() const { return m_pField->get_value_changed_from_saved(); }
    void set_text(const OUString& rStr) { m_pField->set_text(rStr); }
    void set_accessible_name(const OUString& rStr) { m_pField->set_accessible_name(rStr); }
    void SetMetricFieldMin(int nNewMin) { m_pField->set_min(nNewMin, FieldUnit::NONE); }
    void SetMetricFieldMax(int nNewMax) { m_pField->set_max(nNewMax, FieldUnit::NONE); }
    void SetMetricFieldRange(int nNewMin, int nNewMax) { m_pField->set_range(nNewMin, nNewMax, FieldUnit::NONE); }
    void GetMetricFieldRange(int &rOldMin, int& rOldMax) const { m_pField->get_range(rOldMin, rOldMax, FieldUnit::NONE); }

    void set_value(int nNewValue, FieldUnit eInUnit = FieldUnit::NONE);
    int get_value(FieldUnit eOutUnit = FieldUnit::NONE);

    void set_min(int nNewMin, FieldUnit eInUnit);
    void set_max(int nNewMax, FieldUnit eInUnit);

    int get_min(FieldUnit eOutUnit = FieldUnit::NONE) const { return m_pField->get_min(eOutUnit); }

    int NormalizePercent(int nValue);
    int DenormalizePercent(int nValue);

    void SetRefValue(int nValue);
    int GetRealValue(FieldUnit eOutUnit);

    int Convert(int nValue, FieldUnit eInUnit, FieldUnit eOutUnit);

    void ShowPercent(bool bPercent);

    void LockAutoCalculation(bool bLock) {bLockAutoCalculation = bLock;}
};


#endif // INCLUDED_SW_SOURCE_UIBASE_INC_PRCNTFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
