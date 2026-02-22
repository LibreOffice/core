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

#include <svx/dialmgr.hxx>
#include <svx/spacinglistbox.hxx>
#include <tools/fldunit.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <spacing.hrc>

namespace SpacingListBox
{
    void Fill(SpacingType eType, weld::ComboBox& rComboBox)
    {
        auto nSelected = rComboBox.get_active();
        if (nSelected == -1)
            nSelected = 0;
        rComboBox.clear();

        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        OUString sSuffix;

        const measurement* pResources;
        switch (eType)
        {
            case SpacingType::SPACING_INCH:
                pResources = RID_SVXSTRARY_SPACING_INCH;
                sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::INCH);
                break;
            case SpacingType::MARGINS_INCH:
                pResources = RID_SVXSTRARY_MARGINS_INCH;
                sSuffix = weld::MetricSpinButton::MetricToString(FieldUnit::INCH);
                break;
            case SpacingType::SPACING_CM:
                pResources = RID_SVXSTRARY_SPACING_CM;
                sSuffix = " " + weld::MetricSpinButton::MetricToString(FieldUnit::CM);
                break;
            default:
            case SpacingType::MARGINS_CM:
                sSuffix = " " + weld::MetricSpinButton::MetricToString(FieldUnit::CM);
                pResources = RID_SVXSTRARY_MARGINS_CM;
                break;
        }

        while (pResources->key)
        {
            OUString sMeasurement = rLocaleData.getNum(pResources->human, 2, true, false) + sSuffix;
            OUString aStr = SvxResId(pResources->key).replaceFirst("%1", sMeasurement);
            sal_uInt32 nData = pResources->twips;
            rComboBox.append(OUString::number(nData), aStr);
            ++pResources;
        }

        rComboBox.set_active(nSelected);

        rComboBox.set_size_request(150, -1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
