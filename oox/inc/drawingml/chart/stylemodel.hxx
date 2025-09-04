/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <variant>

#include <oox/core/xmlfilterbase.hxx>
#include <oox/drawingml/chart/modelbase.hxx>
#include <oox/drawingml/shape.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <drawingml/textbodyproperties.hxx>
#include <docmodel/styles/ChartStyle.hxx>

namespace oox::drawingml::chart
{
// Holds the contents of a cs:CT_StyleEntry
//
// CT_StyleReference and CT_FontReference both have the exact same structure,
// simply with different interpretations of the "idx" attribute. So use this
// model for both.
struct StyleEntryModel
{
    typedef ModelRef<model::FontOrStyleRef> StyleRef; // "idx" is ST_StyleMatrixColumnIndex
    typedef ModelRef<model::FontOrStyleRef> FontRef; // "idx" is ST_FontCollectionIndex
    typedef ModelRef<TextCharacterProperties> TextCharacterPropsRef;
    typedef ModelRef<TextBodyProperties> TextBodyPropsRef;
    typedef ModelRef<Shape> ShapeRef;

    StyleRef mxLnRef;
    double mfLineWidthScale = 1.0;
    StyleRef mxFillRef;
    StyleRef mxEffectRef;
    FontRef mxFontRef;
    ShapeRef mxShapeProp;
    TextCharacterPropsRef mrTextCharacterProperties;
    TextBodyPropsRef mxBodyPr;

    model::StyleEntry toStyleEntry(oox::core::XmlFilterBase& rFilter);
};

// Holds the contents of a cs:CT_ChartStyle
struct StyleModel
{
    typedef ModelRef<StyleEntryModel> StyleEntryRef;

    StyleEntryRef mxAxisTitle;
    StyleEntryRef mxCategoryAxis;
    StyleEntryRef mxChartArea;
    StyleEntryRef mxDataLabel;
    StyleEntryRef mxDataLabelCallout;
    StyleEntryRef mxDataPoint;
    StyleEntryRef mxDataPoint3D;
    StyleEntryRef mxDataPointLine;
    StyleEntryRef mxDataPointMarker;
    StyleEntryRef mxDataPointMarkerLayout;
    StyleEntryRef mxDataPointWireframe;
    StyleEntryRef mxDataTable;
    StyleEntryRef mxDownBar;
    StyleEntryRef mxDropLine;
    StyleEntryRef mxErrorBar;
    StyleEntryRef mxFloor;
    StyleEntryRef mxGridlineMajor;
    StyleEntryRef mxGridlineMinor;
    StyleEntryRef mxHiLoLine;
    StyleEntryRef mxLeaderLine;
    StyleEntryRef mxLegend;
    StyleEntryRef mxPlotArea;
    StyleEntryRef mxPlotArea3D;
    StyleEntryRef mxSeriesAxis;
    StyleEntryRef mxSeriesLine;
    StyleEntryRef mxTitle;
    StyleEntryRef mxTrendline;
    StyleEntryRef mxTrendlineLabel;
    StyleEntryRef mxUpBar;
    StyleEntryRef mxValueAxis;
    StyleEntryRef mxWall;

    sal_Int32 mnId;
};

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
