/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DPLOTTERBASE_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_GL3DPLOTTERBASE_HXX

#include "VDataSeries.hxx"
#include <vector>
#include <memory>

namespace chart {

class ExplicitCategoriesProvider;

class GL3DPlotterBase
{
public:
    virtual ~GL3DPlotterBase();

    virtual void create3DShapes(const std::vector<std::unique_ptr<VDataSeries> >& rDataSeries,
        ExplicitCategoriesProvider& rCatProvider) = 0;
    virtual void render() = 0;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
