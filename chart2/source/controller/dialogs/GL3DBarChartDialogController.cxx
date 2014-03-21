/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartDialogController.hxx"
#include <Strings.hrc>
#include <Bitmaps.hrc>
#include <ResId.hxx>

namespace chart {

GL3DBarChartDialogController::GL3DBarChartDialogController() {}

GL3DBarChartDialogController::~GL3DBarChartDialogController() {}

OUString GL3DBarChartDialogController::getName()
{
    return SchResId(STR_TYPE_GL3D_BAR).toString();
}

Image GL3DBarChartDialogController::getImage()
{
    return Image(SchResId(IMG_TYPE_GL3D_BAR));
}

const tTemplateServiceChartTypeParameterMap&
GL3DBarChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap aMap(
        "com.sun.star.chart2.template.GL3DBarRoundedRectangle", ChartTypeParameter(1));
    return aMap;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
