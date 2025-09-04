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

#include <optional>

#include <drawingml/chart/stylefragment.hxx>

#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/textbodypropertiescontext.hxx>
#include <drawingml/textcharacterpropertiescontext.hxx>
#include <drawingml/chart/stylemodel.hxx>
#include <drawingml/colorchoicecontext.hxx>
#include <docmodel/styles/ChartStyle.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox::drawingml::chart
{
using namespace ::oox::core;
using namespace model;

//=======
// StyleReferenceContext
//=======
StyleReferenceContext::StyleReferenceContext(ContextHandler2Helper& rParent, sal_Int32 nIdx,
                                             model::FontOrStyleRef& rModel)
    : ContextBase<FontOrStyleRef>(rParent, rModel)
{
    mrModel.mnIdx = nIdx;
}

StyleReferenceContext::~StyleReferenceContext() {}

ContextHandlerRef StyleReferenceContext::onCreateContext(sal_Int32 nElement,
                                                         const AttributeList& rAttribs)
{
    if (isRootElement())
        switch (nElement)
        {
            case CS_TOKEN(styleClr):
            {
                // The attribute here can be an integer, a string, or "auto" (which
                // of course is also a string, but is considered special). So we
                // need to try to convert the input string to an integer, and handle
                // it as an int if we can. OUString provides toInt(), but it returns
                // 0 in the case of failure, which is eminently unhelpful, sinze 0
                // is a perfectly acceptable value. So convert it to a
                // std::basic_string, which has stoi(), which throws if it can't do
                // the conversion.
                //
                // Unfortunately OUString characters are sal_Unicode which can be
                // uint16_t, and while there's string::stoi() and wstring::stoi(),
                // there's no basic_string<uint16_t>::stoi(). So we use wstring and
                // construct character by character.
                std::optional<OUString> str = rAttribs.getString(XML_val);
                if (str)
                {
                    FontOrStyleRef::StyleColorVal v;

                    const sal_Unicode* pRawStr = str->getStr();
                    std::wstring sBStr;
                    sBStr.reserve(str->getLength());
                    for (const sal_Unicode* pS = pRawStr; pS < pRawStr + str->getLength(); ++pS)
                    {
                        sBStr.push_back(*pS);
                    }

                    sal_uInt32 nIntVal = 0;
                    try
                    {
                        nIntVal = stoi(sBStr);
                        v = nIntVal;
                    }
                    catch (std::invalid_argument&)
                    {
                        // Not an integer, so see if it's the fixed enum
                        if (*str == "auto")
                        {
                            v = FontOrStyleRef::StyleColorEnum::AUTO;
                        }
                        else
                        {
                            v = *str;
                        }
                    }
                    mrModel.maStyleClr = std::make_unique<FontOrStyleRef::StyleColorVal>(v);
                }
                return nullptr;
            }
            case A_TOKEN(scrgbClr):
            case A_TOKEN(srgbClr):
            case A_TOKEN(hslClr):
            case A_TOKEN(sysClr):
            case A_TOKEN(schemeClr):
            case A_TOKEN(prstClr):
                return new ColorValueContext(*this, mrModel.maColor, &mrModel.maComplexColor);
        }
    return nullptr;
}

//=======
// StyleEntryContext
//=======
StyleEntryContext::StyleEntryContext(ContextHandler2Helper& rParent, StyleEntryModel& rModel)
    : ContextBase<StyleEntryModel>(rParent, rModel)
{
}

StyleEntryContext::~StyleEntryContext() {}

ContextHandlerRef StyleEntryContext::onCreateContext(sal_Int32 nElement,
                                                     const AttributeList& rAttribs)
{
    if (isRootElement())
        switch (nElement)
        {
            case CS_TOKEN(lnRef): // CT_StyleReference
                return new StyleReferenceContext(*this, rAttribs.getInteger(XML_idx, -1),
                                                 mrModel.mxLnRef.create());
            case CS_TOKEN(lineWidthScale): // double
                return this;
            case CS_TOKEN(fillRef): // CT_StyleReference
                return new StyleReferenceContext(*this, rAttribs.getInteger(XML_idx, -1),
                                                 mrModel.mxFillRef.create());
            case CS_TOKEN(effectRef): // CT_StyleReference
                return new StyleReferenceContext(*this, rAttribs.getInteger(XML_idx, -1),
                                                 mrModel.mxEffectRef.create());
            case CS_TOKEN(fontRef): // CT_FontReference
                return new StyleReferenceContext(*this, rAttribs.getInteger(XML_idx, -1),
                                                 mrModel.mxFontRef.create());
            case CS_TOKEN(spPr): // a:CT_ShapeProperties
                return new ShapePropertiesContext(*this, mrModel.mxShapeProp.create());
            case CS_TOKEN(defRPr): // a:CT_TextCharacterProperties
                return new TextCharacterPropertiesContext(
                    *this, rAttribs, mrModel.mrTextCharacterProperties.create());
            case CS_TOKEN(bodyPr): // a:CT_TextBodyProperties
                return new TextBodyPropertiesContext(*this, rAttribs, mrModel.mxBodyPr.create());
            case CS_TOKEN(extLst): // a:CT_OfficeArtExtensionList
                return nullptr;
        }
    return nullptr;
}

void StyleEntryContext::onCharacters(const OUString& rChars)
{
    switch (getCurrentElement())
    {
        case CS_TOKEN(lineWidthScale):
            mrModel.mfLineWidthScale = rChars.toDouble();
            break;
        default:
            assert(false);
    }
}

//=======
// StyleFragment
//=======
StyleFragment::StyleFragment(XmlFilterBase& rFilter, const OUString& rFragmentPath,
                             StyleModel& rModel)
    : FragmentBase<StyleModel>(rFilter, rFragmentPath, rModel)
{
}

StyleFragment::~StyleFragment() {}

ContextHandlerRef StyleFragment::onCreateContext(sal_Int32 nElement, const AttributeList& rAttribs)
{
    switch (getCurrentElement())
    {
        case XML_ROOT_CONTEXT:
            switch (nElement)
            {
                case CS_TOKEN(chartStyle):
                    mrModel.mnId = rAttribs.getInteger(XML_id, -1);
                    return this;
            }
            break;

        case CS_TOKEN(chartStyle):
            switch (nElement)
            {
                // All of these have "mods" attributes that aren't currently
                // handled. TODO
                case CS_TOKEN(axisTitle):
                    return new StyleEntryContext(*this, mrModel.mxAxisTitle.create());
                case CS_TOKEN(categoryAxis):
                    return new StyleEntryContext(*this, mrModel.mxCategoryAxis.create());
                case CS_TOKEN(chartArea):
                    return new StyleEntryContext(*this, mrModel.mxChartArea.create());
                case CS_TOKEN(dataLabel):
                    return new StyleEntryContext(*this, mrModel.mxDataLabel.create());
                case CS_TOKEN(dataLabelCallout):
                    return new StyleEntryContext(*this, mrModel.mxDataLabelCallout.create());
                case CS_TOKEN(dataPoint):
                    return new StyleEntryContext(*this, mrModel.mxDataPoint.create());
                case CS_TOKEN(dataPoint3D):
                    return new StyleEntryContext(*this, mrModel.mxDataPoint3D.create());
                case CS_TOKEN(dataPointLine):
                    return new StyleEntryContext(*this, mrModel.mxDataPointLine.create());
                case CS_TOKEN(dataPointMarker):
                    return new StyleEntryContext(*this, mrModel.mxDataPointMarker.create());
                case CS_TOKEN(dataPointMarkerLayout):
                    return new StyleEntryContext(*this, mrModel.mxDataPointMarkerLayout.create());
                case CS_TOKEN(dataPointWireframe):
                    return new StyleEntryContext(*this, mrModel.mxDataPointWireframe.create());
                case CS_TOKEN(dataTable):
                    return new StyleEntryContext(*this, mrModel.mxDataTable.create());
                case CS_TOKEN(downBar):
                    return new StyleEntryContext(*this, mrModel.mxDownBar.create());
                case CS_TOKEN(dropLine):
                    return new StyleEntryContext(*this, mrModel.mxDropLine.create());
                case CS_TOKEN(errorBar):
                    return new StyleEntryContext(*this, mrModel.mxErrorBar.create());
                case CS_TOKEN(floor):
                    return new StyleEntryContext(*this, mrModel.mxFloor.create());
                case CS_TOKEN(gridlineMajor):
                    return new StyleEntryContext(*this, mrModel.mxGridlineMajor.create());
                case CS_TOKEN(gridlineMinor):
                    return new StyleEntryContext(*this, mrModel.mxGridlineMinor.create());
                case CS_TOKEN(hiLoLine):
                    return new StyleEntryContext(*this, mrModel.mxHiLoLine.create());
                case CS_TOKEN(leaderLine):
                    return new StyleEntryContext(*this, mrModel.mxLeaderLine.create());
                case CS_TOKEN(legend):
                    return new StyleEntryContext(*this, mrModel.mxLegend.create());
                case CS_TOKEN(plotArea):
                    return new StyleEntryContext(*this, mrModel.mxPlotArea.create());
                case CS_TOKEN(plotArea3D):
                    return new StyleEntryContext(*this, mrModel.mxPlotArea3D.create());
                case CS_TOKEN(seriesAxis):
                    return new StyleEntryContext(*this, mrModel.mxSeriesAxis.create());
                case CS_TOKEN(seriesLine):
                    return new StyleEntryContext(*this, mrModel.mxSeriesLine.create());
                case CS_TOKEN(title):
                    return new StyleEntryContext(*this, mrModel.mxTitle.create());
                case CS_TOKEN(trendline):
                    return new StyleEntryContext(*this, mrModel.mxTrendline.create());
                case CS_TOKEN(trendlineLabel):
                    return new StyleEntryContext(*this, mrModel.mxTrendlineLabel.create());
                case CS_TOKEN(upBar):
                    return new StyleEntryContext(*this, mrModel.mxUpBar.create());
                case CS_TOKEN(valueAxis):
                    return new StyleEntryContext(*this, mrModel.mxValueAxis.create());
                case CS_TOKEN(wall):
                    return new StyleEntryContext(*this, mrModel.mxWall.create());
                case CS_TOKEN(extLst):
                    // Don't handle this, at least yet
                    return nullptr;
            }
            break;
    }
    return nullptr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
