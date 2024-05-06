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

#include <chartview/ExplicitValueProvider.hxx>
#include <AxisHelper.hxx>
#include <ChartModel.hxx>
#include <Diagram.hxx>
#include <DiagramHelper.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>
#include <TitleHelper.hxx>
#include <ObjectIdentifier.hxx>

#include <comphelper/servicehelper.hxx>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

namespace
{
constexpr sal_Int32 constDiagramTitleSpace = 200; //=0,2 cm spacing

bool lcl_getPropertySwapXAndYAxis(const rtl::Reference<Diagram>& xDiagram)
{
    bool bSwapXAndY = false;

    if (xDiagram.is())
    {
        const std::vector<rtl::Reference<BaseCoordinateSystem>>& aCooSysList(
            xDiagram->getBaseCoordinateSystems());
        if (!aCooSysList.empty())
        {
            try
            {
                aCooSysList[0]->getPropertyValue(u"SwapXAndYAxis"_ustr) >>= bSwapXAndY;
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("chart2", "");
            }
        }
    }
    return bSwapXAndY;
}

} // end anonymous namespace

sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForAxis(
    const rtl::Reference<::chart::Axis>& xAxis,
    const rtl::Reference<::chart::BaseCoordinateSystem>& xCorrespondingCoordinateSystem,
    const rtl::Reference<::chart::ChartModel>& xChartDoc)
{
    return AxisHelper::getExplicitNumberFormatKeyForAxis(
        xAxis, xCorrespondingCoordinateSystem, xChartDoc,
        true /*bSearchForParallelAxisIfNothingIsFound*/);
}

sal_Int32 ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel(
    const uno::Reference<beans::XPropertySet>& xSeriesOrPointProp)
{
    sal_Int32 nFormat = 0;
    if (!xSeriesOrPointProp.is())
        return nFormat;

    try
    {
        xSeriesOrPointProp->getPropertyValue(CHART_UNONAME_NUMFMT) >>= nFormat;
    }
    catch (const beans::UnknownPropertyException&)
    {
    }

    if (nFormat < 0)
        nFormat = 0;
    return nFormat;
}

sal_Int32 ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
    const uno::Reference<beans::XPropertySet>& xSeriesOrPointProp,
    const uno::Reference<util::XNumberFormatsSupplier>& xNumberFormatsSupplier)
{
    sal_Int32 nFormat = 0;
    if (!xSeriesOrPointProp.is())
        return nFormat;
    if (!(xSeriesOrPointProp->getPropertyValue(u"PercentageNumberFormat"_ustr) >>= nFormat))
    {
        nFormat = DiagramHelper::getPercentNumberFormat(xNumberFormatsSupplier);
    }
    if (nFormat < 0)
        nFormat = 0;
    return nFormat;
}

awt::Rectangle ExplicitValueProvider::AddSubtractAxisTitleSizes(
    ChartModel& rModel, ExplicitValueProvider* pChartView, const awt::Rectangle& rPositionAndSize,
    bool bSubtract)
{
    awt::Rectangle aRet(rPositionAndSize);

    //add axis title sizes to the diagram size
    rtl::Reference<::chart::Title> xTitle_Height(
        TitleHelper::getTitle(TitleHelper::TITLE_AT_STANDARD_X_AXIS_POSITION, rModel));
    rtl::Reference<::chart::Title> xTitle_Width(
        TitleHelper::getTitle(TitleHelper::TITLE_AT_STANDARD_Y_AXIS_POSITION, rModel));
    rtl::Reference<::chart::Title> xSecondTitle_Height(
        TitleHelper::getTitle(TitleHelper::SECONDARY_X_AXIS_TITLE, rModel));
    rtl::Reference<::chart::Title> xSecondTitle_Width(
        TitleHelper::getTitle(TitleHelper::SECONDARY_Y_AXIS_TITLE, rModel));
    if (xTitle_Height.is() || xTitle_Width.is() || xSecondTitle_Height.is()
        || xSecondTitle_Width.is())
    {
        ExplicitValueProvider* pExplicitValueProvider = pChartView;
        if (pExplicitValueProvider)
        {
            //detect whether x axis points into x direction or not
            if (lcl_getPropertySwapXAndYAxis(rModel.getFirstChartDiagram()))
            {
                std::swap(xTitle_Height, xTitle_Width);
                std::swap(xSecondTitle_Height, xSecondTitle_Width);
            }

            sal_Int32 nTitleSpaceWidth = 0;
            sal_Int32 nTitleSpaceHeight = 0;
            sal_Int32 nSecondTitleSpaceWidth = 0;
            sal_Int32 nSecondTitleSpaceHeight = 0;

            if (xTitle_Height.is())
            {
                OUString aCID_X(
                    ObjectIdentifier::createClassifiedIdentifierForObject(xTitle_Height, &rModel));
                nTitleSpaceHeight
                    = pExplicitValueProvider->getRectangleOfObject(aCID_X, true).Height;
                if (nTitleSpaceHeight)
                    nTitleSpaceHeight += constDiagramTitleSpace;
            }
            if (xTitle_Width.is())
            {
                OUString aCID_Y(
                    ObjectIdentifier::createClassifiedIdentifierForObject(xTitle_Width, &rModel));
                nTitleSpaceWidth = pExplicitValueProvider->getRectangleOfObject(aCID_Y, true).Width;
                if (nTitleSpaceWidth)
                    nTitleSpaceWidth += constDiagramTitleSpace;
            }
            if (xSecondTitle_Height.is())
            {
                OUString aCID_X(ObjectIdentifier::createClassifiedIdentifierForObject(
                    xSecondTitle_Height, &rModel));
                nSecondTitleSpaceHeight
                    = pExplicitValueProvider->getRectangleOfObject(aCID_X, true).Height;
                if (nSecondTitleSpaceHeight)
                    nSecondTitleSpaceHeight += constDiagramTitleSpace;
            }
            if (xSecondTitle_Width.is())
            {
                OUString aCID_Y(ObjectIdentifier::createClassifiedIdentifierForObject(
                    xSecondTitle_Width, &rModel));
                nSecondTitleSpaceWidth
                    += pExplicitValueProvider->getRectangleOfObject(aCID_Y, true).Width;
                if (nSecondTitleSpaceWidth)
                    nSecondTitleSpaceWidth += constDiagramTitleSpace;
            }
            if (bSubtract)
            {
                aRet.X += nTitleSpaceWidth;
                aRet.Y += nSecondTitleSpaceHeight;
                aRet.Width -= (nTitleSpaceWidth + nSecondTitleSpaceWidth);
                aRet.Height -= (nTitleSpaceHeight + nSecondTitleSpaceHeight);
            }
            else
            {
                aRet.X -= nTitleSpaceWidth;
                aRet.Y -= nSecondTitleSpaceHeight;
                aRet.Width += nTitleSpaceWidth + nSecondTitleSpaceWidth;
                aRet.Height += nTitleSpaceHeight + nSecondTitleSpaceHeight;
            }
        }
    }
    return aRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
