/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/chart2/XChartColorStyle.hpp>

#include <docmodel/dllapi.h>
#include <docmodel/styles/ChartColorStyle.hxx>

class UNLESS_MERGELIBS(DOCMODEL_DLLPUBLIC) UnoChartColorStyle final
    : public cppu::WeakImplHelper<css::chart2::XChartColorStyle>
{
private:
    model::ColorStyleSet maColorStyle;

public:
    UnoChartColorStyle() = default;

    UnoChartColorStyle(model::ColorStyleSet const& rStyle)
        : maColorStyle(rStyle)
    {
    }

    model::ColorStyleSet& getChartColorStyle() { return maColorStyle; }
};

namespace model::style
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::chart2::XChartColorStyle>
createXChartColorStyle(model::ColorStyleSet const& rColorStyle);
DOCMODEL_DLLPUBLIC model::ColorStyleSet*
getFromXChartColorStyle(css::uno::Reference<css::chart2::XChartColorStyle> const& rxColorStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
