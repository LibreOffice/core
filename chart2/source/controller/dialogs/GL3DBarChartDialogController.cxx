/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartDialogController.hxx"
#include <strings.hrc>
#include <bitmaps.hlst>
#include <ResId.hxx>
#include <svtools/valueset.hxx>

namespace chart {

GL3DBarChartDialogController::GL3DBarChartDialogController() {}

GL3DBarChartDialogController::~GL3DBarChartDialogController() {}

bool GL3DBarChartDialogController::shouldShow_GL3DResourceGroup() const
{
    return true;
}

OUString GL3DBarChartDialogController::getName()
{
    return SchResId(STR_TYPE_GL3D_BAR);
}

Image GL3DBarChartDialogController::getImage()
{
    return Image(BitmapEx(BMP_TYPE_GL3D_BAR));
}

const tTemplateServiceChartTypeParameterMap&
GL3DBarChartDialogController::getTemplateMap() const
{
    static tTemplateServiceChartTypeParameterMap aMap{
        {"com.sun.star.chart2.template.GL3DBar", ChartTypeParameter(1)},
        {"com.sun.star.chart2.template.GL3DBarRoundedRectangle", ChartTypeParameter(2)}};
    return aMap;
}

void GL3DBarChartDialogController::fillSubTypeList(
    ValueSet& rSubTypeList, const ChartTypeParameter& /*rParameter*/ )
{
    rSubTypeList.Clear();
    rSubTypeList.InsertItem(1, Image(BitmapEx(BMP_GL3D_BAR_ROUNDRECT)));

    rSubTypeList.SetItemText(1, SchResId(STR_GL3D_BAR));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
