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

#include <svx/relfld.hxx>
#include <vcl/fieldvalues.hxx>

SvxRelativeField::SvxRelativeField(std::unique_ptr<weld::MetricSpinButton> pControl)
    : m_xSpinButton(std::move(pControl))
    , m_nRelMin(0)
    , m_nRelMax(0)
    , m_bRelativeMode(false)
    , m_bRelative(false)
    , m_bNegativeEnabled(false)
    , m_bFontRelativeMode(false)
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();
    rSpinButton.connect_changed(LINK(this, SvxRelativeField, ModifyHdl));
}

IMPL_LINK_NOARG(SvxRelativeField, ModifyHdl, weld::Entry&, void)
{
    if (m_bRelativeMode)
    {
        OUString aStr = m_xSpinButton->get_text();
        bool bNewMode = m_bRelative;

        if (m_bRelative)
        {
            const sal_Unicode* pStr = aStr.getStr();

            while (*pStr)
            {
                if (((*pStr < '0') || (*pStr > '9')) && (*pStr != '%'))
                {
                    bNewMode = false;
                    break;
                }
                pStr++;
            }
        }
        else
        {
            if (aStr.indexOf("%") != -1)
                bNewMode = true;
        }

        if (bNewMode != m_bRelative)
            SetRelative(bNewMode);
    }

    if (m_bFontRelativeMode)
    {
        OUString aStr = m_xSpinButton->get_text();
        FieldUnit eNewFieldUnit = vcl::GetTextMetricUnit(aStr);

        // Only allow font-relative units
        switch (eNewFieldUnit)
        {
            default:
                eNewFieldUnit = FieldUnit::NONE;
                break;

            case FieldUnit::FONT_EM:
            case FieldUnit::FONT_CJK_ADVANCE:
                break;
        }

        if (eNewFieldUnit != m_eFontRelativeFieldUnit)
        {
            SetFontRelative(eNewFieldUnit);
        }
    }
}

void SvxRelativeField::EnableRelativeMode(sal_uInt16 nMin, sal_uInt16 nMax)
{
    m_bRelativeMode = true;
    m_nRelMin       = nMin;
    m_nRelMax       = nMax;
    m_xSpinButton->set_unit(FieldUnit::CM);
}

void SvxRelativeField::EnableFontRelativeMode() { m_bFontRelativeMode = true; }

void SvxRelativeField::SetRelative( bool bNewRelative )
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();

    int nStartPos, nEndPos;
    rSpinButton.get_selection_bounds(nStartPos, nEndPos);
    OUString aStr = rSpinButton.get_text();

    m_eFontRelativeFieldUnit = FieldUnit::NONE;

    if ( bNewRelative )
    {
        m_bRelative = true;
        m_xSpinButton->set_digits(0);
        m_xSpinButton->set_range(m_nRelMin, m_nRelMax, FieldUnit::NONE);
        m_xSpinButton->set_unit(FieldUnit::PERCENT);
    }
    else
    {
        m_bRelative = false;
        m_xSpinButton->set_digits(2);
        m_xSpinButton->set_range(m_bNegativeEnabled ? -9999 : 0, 9999, FieldUnit::NONE);
        m_xSpinButton->set_unit(FieldUnit::CM);
    }

    rSpinButton.set_text(aStr);
    rSpinButton.select_region(nStartPos, nEndPos);
}

void SvxRelativeField::SetFontRelative(FieldUnit eNewRelativeUnit)
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();

    int nStartPos, nEndPos;
    rSpinButton.get_selection_bounds(nStartPos, nEndPos);
    OUString aStr = rSpinButton.get_text();

    m_bRelative = false;
    m_eFontRelativeFieldUnit = eNewRelativeUnit;
    m_xSpinButton->set_digits(2);
    m_xSpinButton->set_range(m_bNegativeEnabled ? -9999 : 0, 9999, FieldUnit::NONE);

    if (eNewRelativeUnit == FieldUnit::NONE)
    {
        m_xSpinButton->set_unit(m_eAbsoluteFieldUnit);
    }
    else
    {
        m_xSpinButton->set_unit(eNewRelativeUnit);
    }

    rSpinButton.set_text(aStr);
    rSpinButton.select_region(nStartPos, nEndPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
