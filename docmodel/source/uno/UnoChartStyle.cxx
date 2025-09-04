/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <docmodel/uno/UnoChartStyle.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace css;

namespace model::style
{
uno::Reference<chart2::XChartStyle> createXChartStyle(model::StyleSet const& rStyle)
{
    return new UnoChartStyle(rStyle);
}

model::StyleSet* getFromXChartStyle(uno::Reference<chart2::XChartStyle> const& rxStyle)
{
    UnoChartStyle* pUnoChartStyle = static_cast<UnoChartStyle*>(rxStyle.get());

    if (pUnoChartStyle)
    {
        return &pUnoChartStyle->getChartStyle();
    }
    else
    {
        return nullptr;
    }
}

} // end model::style

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
