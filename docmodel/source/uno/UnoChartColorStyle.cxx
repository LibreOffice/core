/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <docmodel/uno/UnoChartColorStyle.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

namespace model::style
{
uno::Reference<chart2::XChartColorStyle>
createXChartColorStyle(model::ColorStyleSet const& rColorStyle)
{
    return new UnoChartColorStyle(rColorStyle);
}

model::ColorStyleSet*
getFromXChartColorStyle(uno::Reference<chart2::XChartColorStyle> const& rxColorStyle)
{
    UnoChartColorStyle* pUnoChartColorStyle = static_cast<UnoChartColorStyle*>(rxColorStyle.get());

    if (pUnoChartColorStyle)
    {
        return &pUnoChartColorStyle->getChartColorStyle();
    }
    else
    {
        return nullptr;
    }
}

} // end model::style

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
