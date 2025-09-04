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
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <com/sun/star/chart2/XChartStyle.hpp>

#include <docmodel/dllapi.h>
#include <docmodel/styles/ChartStyle.hxx>

class UNLESS_MERGELIBS(DOCMODEL_DLLPUBLIC) UnoChartStyle final
    : public cppu::WeakImplHelper<css::chart2::XChartStyle>
#if 0 // TODO
   ,public css::util::XModifyListener       // I think this is needed
   ,public css::util::XModifyBroadcaster    // I think this is needed
#endif

{
private:
    model::StyleSet maStyle;

public:
    UnoChartStyle() = default;

    UnoChartStyle(model::StyleSet const& rStyle)
        : maStyle(rStyle)
    {
    }

    model::StyleSet& getChartStyle() { return maStyle; }

    // XChartStyle
};

namespace model::style
{
DOCMODEL_DLLPUBLIC css::uno::Reference<css::chart2::XChartStyle>
createXChartStyle(model::StyleSet const& rStyle);
DOCMODEL_DLLPUBLIC model::StyleSet*
getFromXChartStyle(css::uno::Reference<css::chart2::XChartStyle> const& rxStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
