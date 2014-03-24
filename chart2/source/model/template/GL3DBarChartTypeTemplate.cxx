/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartTypeTemplate.hxx"
#include "GL3DBarChartType.hxx"

#include <servicenames_charttypes.hxx>
#include <macros.hxx>

using namespace com::sun::star;

namespace chart {

namespace {

const OUString aServiceName("com.sun.star.chart2.BubbleChartTypeTemplate");

}

GL3DBarChartTypeTemplate::GL3DBarChartTypeTemplate(
    const uno::Reference<uno::XComponentContext>& xContext, const OUString& rServiceName ) :
    ChartTypeTemplate(xContext, rServiceName) {}

GL3DBarChartTypeTemplate::~GL3DBarChartTypeTemplate() {}

uno::Reference<chart2::XChartType> GL3DBarChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    uno::Reference<chart2::XChartType> xResult;

    try
    {
        uno::Reference<lang::XMultiServiceFactory> xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);

        xResult.set(xFact->createInstance(CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR), uno::UNO_QUERY_THROW);
    }
    catch (const uno::Exception & ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

uno::Reference<chart2::XChartType>
GL3DBarChartTypeTemplate::getChartTypeForNewSeries(
    const uno::Sequence<uno::Reference<chart2::XChartType> >& xOldChartTypes )
{
    uno::Reference<chart2::XChartType> xResult;

    try
    {
        uno::Reference<lang::XMultiServiceFactory> xFact(
            GetComponentContext()->getServiceManager(), uno::UNO_QUERY_THROW);

#if 1
        // I gave up trying to use UNO just to instantiate this little thing...
        xResult.set(new GL3DBarChartType(GetComponentContext()));
#else
        // This never works for me.
        xResult.set(xFact->createInstance(CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR), uno::UNO_QUERY_THROW);
#endif
        ChartTypeTemplate::copyPropertiesFromOldToNewCoordianteSystem(xOldChartTypes, xResult);
    }
    catch (const uno::Exception & ex)
    {
        ASSERT_EXCEPTION( ex );
    }

    return xResult;
}

sal_Bool GL3DBarChartTypeTemplate::supportsCategories()
{
    return false;
}

uno::Sequence<OUString> GL3DBarChartTypeTemplate::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aServices(2);
    aServices[0] = aServiceName;
    aServices[1] = "com.sun.star.chart2.ChartTypeTemplate";
    return aServices;
}

APPHELPER_XSERVICEINFO_IMPL(GL3DBarChartTypeTemplate, aServiceName);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
