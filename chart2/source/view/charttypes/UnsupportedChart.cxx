/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ResId.hxx>
#include <strings.hrc>

#include "UnsupportedChart.hxx"
#include <PlottingPositionHelper.hxx>
#include <ShapeFactory.hxx>
#include <ObjectIdentifier.hxx>
#include <LabelPositionHelper.hxx>
#include <ChartType.hxx>
#include <PropertyMapper.hxx>
#include <CommonConverters.hxx>

#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/chart2/XFormattedString.hpp>
#include <com/sun/star/chart2/FormattedString.hpp>

#include <limits>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

UnsupportedChart::UnsupportedChart(const rtl::Reference<ChartType>& xChartTypeModel)
    : VSeriesPlotter(xChartTypeModel, 2, false)
{
    if (!m_pMainPosHelper)
        m_pMainPosHelper = new PlottingPositionHelper();
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
}

UnsupportedChart::~UnsupportedChart() { delete m_pMainPosHelper; }

bool UnsupportedChart::isExpandIfValuesCloseToBorder(sal_Int32 /*nDimensionIndex*/) { return true; }

bool UnsupportedChart::isSeparateStackingForDifferentSigns(sal_Int32 /*nDimensionIndex*/)
{
    return false;
}

LegendSymbolStyle UnsupportedChart::getLegendSymbolStyle() { return LegendSymbolStyle::Circle; }

drawing::Direction3D UnsupportedChart::getPreferredDiagramAspectRatio() const
{
    return drawing::Direction3D(-1, -1, -1);
}

void UnsupportedChart::createShapes()
{
    rtl::Reference<SvxShapeGroup> xSeriesTarget = ShapeFactory::createGroup2D(m_xFinalTarget);

    tNameSequence aPropNames;
    tAnySequence aPropValues;

    tPropertyNameValueMap aTextValueMap;
    aTextValueMap[u"CharHeight"_ustr] <<= 10.0f;
    aTextValueMap[u"CharHeightAsian"_ustr] <<= 10.0f;
    aTextValueMap[u"CharHeightComplex"_ustr] <<= 10.0f;
    aTextValueMap[u"FillColor"_ustr] <<= sal_Int32(0xFFFFFF);
    aTextValueMap[u"FillStyle"_ustr] <<= drawing::FillStyle_SOLID;
    aTextValueMap[u"LineColor"_ustr] <<= sal_Int32(0xFFFFFF);
    aTextValueMap[u"LineStyle"_ustr] <<= drawing::LineStyle_SOLID;
    aTextValueMap[u"ParaAdjust"_ustr] <<= style::ParagraphAdjust_CENTER;
    aTextValueMap[u"TextHorizontalAdjust"_ustr] <<= drawing::TextHorizontalAdjust_CENTER;
    aTextValueMap[u"TextVerticalAdjust"_ustr] <<= drawing::TextVerticalAdjust_CENTER;
    aTextValueMap[u"ParaLeftMargin"_ustr] <<= sal_Int32(0);
    aTextValueMap[u"ParaRightMargin"_ustr] <<= sal_Int32(0);
    aTextValueMap[u"TextMaximumFrameWidth"_ustr] <<= sal_Int32(2 * m_aPageReferenceSize.Width / 3);
    aTextValueMap[u"TextAutoGrowHeight"_ustr] <<= true;
    aTextValueMap[u"TextAutoGrowWidth"_ustr] <<= true;

    PropertyMapper::getMultiPropertyListsFromValueMap(aPropNames, aPropValues, aTextValueMap);

    ::basegfx::B2DHomMatrix aM;
    aM.translate(m_aPageReferenceSize.Width / 2, m_aPageReferenceSize.Height / 2);

    rtl::Reference<SvxShapeText> xTextShape
        = ShapeFactory::createText(xSeriesTarget, SchResId(STR_UNSUPPORTED_CHART_TYPE), aPropNames,
                                   aPropValues, uno::Any(B2DHomMatrixToHomogenMatrix3(aM)));
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
