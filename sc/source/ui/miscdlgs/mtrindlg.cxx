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

#undef SC_DLLIMPLEMENTATION

#include <mtrindlg.hxx>

ScMetricInputDlg::ScMetricInputDlg( weld::Window*         pParent,
                                    const OUString&  sDialogName,
                                    tools::Long            nCurrent,
                                    tools::Long            nDefault,
                                    FieldUnit       eFUnit,
                                    sal_uInt16      nDecimals,
                                    tools::Long            nMaximum,
                                    tools::Long            nMinimum)

    : GenericDialogController(pParent, "modules/scalc/ui/" + sDialogName.toAsciiLowerCase() + ".ui", sDialogName)
    , m_xEdValue(m_xBuilder->weld_metric_spin_button(u"value"_ustr, FieldUnit::CM))
    , m_xBtnDefVal(m_xBuilder->weld_check_button(u"default"_ustr))
{
    m_xBtnDefVal->connect_toggled(LINK(this, ScMetricInputDlg, SetDefValHdl));
    m_xEdValue->connect_value_changed(LINK( this, ScMetricInputDlg, ModifyHdl));

    m_xEdValue->set_unit(eFUnit);
    m_xEdValue->set_digits(nDecimals);
    m_xEdValue->set_range(m_xEdValue->normalize(nMinimum),
                          m_xEdValue->normalize(nMaximum), FieldUnit::TWIP);

    sal_Int64 nMin(0), nMax(0);
    m_xEdValue->get_range(nMin, nMax, FieldUnit::TWIP);

    auto nIncrement = m_xEdValue->normalize(1);
    m_xEdValue->set_increments(nIncrement / 10, nIncrement, FieldUnit::NONE);
    m_xEdValue->set_value(m_xEdValue->normalize(nDefault), FieldUnit::TWIP);
    nDefaultValue = m_xEdValue->get_value(FieldUnit::NONE);
    m_xEdValue->set_value(m_xEdValue->normalize(nCurrent), FieldUnit::TWIP);
    nCurrentValue = m_xEdValue->get_value(FieldUnit::NONE);
    m_xBtnDefVal->set_active(nCurrentValue == nDefaultValue);
}

ScMetricInputDlg::~ScMetricInputDlg()
{
}

int ScMetricInputDlg::GetInputValue() const
{
/*
    with decimal digits

    double  nVal    = m_xEdValue->GetValue( eUnit );
    sal_uInt16  nDecs   = m_xEdValue->GetDecimalDigits();
    double  nFactor = 0.0;

    // static long ImpPower10( sal_uInt16 nDecs )
    {
        nFactor = 1.0;

        for ( sal_uInt16 i=0; i < nDecs; i++ )
            nFactor *= 10.0;
    }

    return nVal / nFactor;
*/
    // first cut off the decimal digits - not that great...

    return m_xEdValue->denormalize(m_xEdValue->get_value(FieldUnit::TWIP));
}

// Handler:

IMPL_LINK_NOARG(ScMetricInputDlg, SetDefValHdl, weld::Toggleable&, void)
{
    if (m_xBtnDefVal->get_active())
    {
        nCurrentValue = m_xEdValue->get_value(FieldUnit::NONE);
        m_xEdValue->set_value(nDefaultValue, FieldUnit::NONE);
    }
    else
        m_xEdValue->set_value(nCurrentValue, FieldUnit::NONE);
}

IMPL_LINK_NOARG(ScMetricInputDlg, ModifyHdl, weld::MetricSpinButton&, void)
{
    m_xBtnDefVal->set_active(nDefaultValue == m_xEdValue->get_value(FieldUnit::NONE));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
