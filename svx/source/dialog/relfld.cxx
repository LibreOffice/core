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
    , nRelMin(0)
    , nRelMax(0)
    , bRelativeMode(false)
    , bRelative(false)
    , bNegativeEnabled(false)
    , bFontRelativeMode(false)
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();
    rSpinButton.connect_changed(LINK(this, SvxRelativeField, ModifyHdl));
}

IMPL_LINK_NOARG(SvxRelativeField, ModifyHdl, weld::Entry&, void)
{
    if (bRelativeMode)
    {
        OUString aStr = m_xSpinButton->get_text();
        bool bNewMode = bRelative;

        if (bRelative)
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

        if (bNewMode != bRelative)
            SetRelative(bNewMode);
    }

    if (bFontRelativeMode)
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

        if (eNewFieldUnit != eFontRelativeFieldUnit)
        {
            SetFontRelative(eNewFieldUnit);
        }
    }
}

void SvxRelativeField::EnableRelativeMode(sal_uInt16 nMin, sal_uInt16 nMax)
{
    bRelativeMode = true;
    nRelMin       = nMin;
    nRelMax       = nMax;
    m_xSpinButton->set_unit(FieldUnit::CM);
}

void SvxRelativeField::EnableFontRelativeMode() { bFontRelativeMode = true; }

void SvxRelativeField::SetRelative( bool bNewRelative )
{
    weld::SpinButton& rSpinButton = m_xSpinButton->get_widget();

    int nStartPos, nEndPos;
    rSpinButton.get_selection_bounds(nStartPos, nEndPos);
    OUString aStr = rSpinButton.get_text();

    eFontRelativeFieldUnit = FieldUnit::NONE;

    if ( bNewRelative )
    {
        bRelative = true;
        m_xSpinButton->set_digits(0);
        m_xSpinButton->set_range(nRelMin, nRelMax, FieldUnit::NONE);
        m_xSpinButton->set_unit(FieldUnit::PERCENT);
    }
    else
    {
        bRelative = false;
        m_xSpinButton->set_digits(2);
        m_xSpinButton->set_range(bNegativeEnabled ? -9999 : 0, 9999, FieldUnit::NONE);
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

    bRelative = false;
    eFontRelativeFieldUnit = eNewRelativeUnit;
    m_xSpinButton->set_digits(2);
    m_xSpinButton->set_range(bNegativeEnabled ? -9999 : 0, 9999, FieldUnit::NONE);

    if (eNewRelativeUnit == FieldUnit::NONE)
    {
        m_xSpinButton->set_unit(eAbsoluteFieldUnit);
    }
    else
    {
        m_xSpinButton->set_unit(eNewRelativeUnit);
    }

    rSpinButton.set_text(aStr);
    rSpinButton.select_region(nStartPos, nEndPos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
