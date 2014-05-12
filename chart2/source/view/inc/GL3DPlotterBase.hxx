/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_GL3DPLOTTERBASE_HXX
#define CHART2_GL3DPLOTTERBASE_HXX

namespace chart {

class GL3DPlotterBase
{
public:
    virtual ~GL3DPlotterBase();

    virtual void create3DShapes() = 0;
    virtual void render() = 0;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
