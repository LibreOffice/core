/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "LoggedResources.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>

#include "PropertyIds.hxx"

#include <oox/helper/grabbagstack.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>

#include <memory>
#include <optional>

namespace writerfilter::dmapper
{
/// Class to process all text effects like glow, textOutline, ...
class ThemeColorHandler : public LoggedProperties
{
public:
    sal_Int32 mnColor = 0;
    sal_Int32 mnIndex = -1;
    sal_Int32 mnTint = 0;
    sal_Int32 mnShade = 0;

    explicit ThemeColorHandler()
        : LoggedProperties("ThemeColorHandler")
    {
    }

    virtual void lcl_attribute(Id aName, Value& rValue) override
    {
        sal_Int32 nIntValue = rValue.getInt();

        switch (aName)
        {
            case NS_ooxml::LN_CT_Color_val:
                mnColor = nIntValue;
                break;
            case NS_ooxml::LN_CT_Color_themeColor:
                mnIndex = nIntValue;
                break;

            case NS_ooxml::LN_CT_Color_themeTint:
                mnTint = nIntValue;
                break;

            case NS_ooxml::LN_CT_Color_themeShade:
                mnShade = nIntValue;
                break;
        }
    }

    virtual void lcl_sprm(Sprm& /*sprm*/) override {}
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
