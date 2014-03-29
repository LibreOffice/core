/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChart.hxx"
#include <unonames.hxx>

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/LabelOrigin.hpp>

#include <stdio.h>

using namespace com::sun::star;

namespace chart {

GL3DBarChart::GL3DBarChart( const css::uno::Reference<css::chart2::XChartType>& xChartTypeModel ) :
    VSeriesPlotter(xChartTypeModel, 3, false)
{
}

GL3DBarChart::~GL3DBarChart()
{
}

void GL3DBarChart::createShapes()
{
    fprintf(stdout, "GL3DBarChart::createShapes:   type = '%s'\n",
            rtl::OUStringToOString(m_xChartTypeModel->getChartType(), RTL_TEXTENCODING_UTF8).getStr());

    uno::Reference<beans::XPropertySet> xPropSet(m_xChartTypeModel, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        bool bRoundedEdge = false;
        if (xPropSet->getPropertyValue(CHART_UNONAME_ROUNDED_EDGE) >>= bRoundedEdge)
            fprintf(stdout, "GL3DBarChart::createShapes:   rounded edge = %d (%p)\n", bRoundedEdge, m_xChartTypeModel.get());
    }

#if 0
    if (m_pExplicitCategoriesProvider)
    {
        uno::Reference<chart2::data::XDataSequence> xCats = m_pExplicitCategoriesProvider->getOriginalCategories();

        OUString aSrcRange = xCats->getSourceRangeRepresentation();

        fprintf(stdout, "GL3DBarChart::createShapes:   source range = '%s'\n", rtl::OUStringToOString(aSrcRange, RTL_TEXTENCODING_UTF8).getStr());

        uno::Sequence<OUString> aCats = m_pExplicitCategoriesProvider->getSimpleCategories();
        for (sal_Int32 i = 0; i < aCats.getLength(); ++i)
            fprintf(stdout, "GL3DBarChart::createShapes:   category = '%s'\n", rtl::OUStringToOString(aCats[i], RTL_TEXTENCODING_UTF8).getStr());
    }

    uno::Sequence<OUString> aSeriesNames = getSeriesNames();
    fprintf(stdout, "GL3DBarChart::createShapes:   series name count = %d\n", aSeriesNames.getLength());
    for (sal_Int32 i = 0; i < aSeriesNames.getLength(); ++i)
        fprintf(stdout, "GL3DBarChart::createShapes:   name = '%s'\n", rtl::OUStringToOString(aSeriesNames[i], RTL_TEXTENCODING_UTF8).getStr());

    std::vector<VDataSeries*> aAllSeries = getAllSeries();
    fprintf(stdout, "GL3DBarChart::createShapes:   series count = %d\n", aAllSeries.size());
    for (size_t i = 0, n = aAllSeries.size(); i < n; ++i)
    {
        const VDataSeries* pSeries = aAllSeries[i];
        fprintf(stdout, "GL3DBarChart::createShapes:   series %d: cid = '%s'  particle = '%s'\n",
                i,
                rtl::OUStringToOString(pSeries->getCID(), RTL_TEXTENCODING_UTF8).getStr(),
                rtl::OUStringToOString(pSeries->getSeriesParticle(), RTL_TEXTENCODING_UTF8).getStr());

        uno::Sequence<double> aXValues = pSeries->getAllX();
        uno::Sequence<double> aYValues = pSeries->getAllY();
        for (size_t j = 0; j < aXValues.getLength(); ++j)
            fprintf(stdout, "GL3DBarChart::createShapes:     (x=%g,y=%g)\n", aXValues[j], aYValues[j]);
    }
#endif
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
