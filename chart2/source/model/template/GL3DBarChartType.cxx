/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartType.hxx"
#include <servicenames_charttypes.hxx>

using namespace com::sun::star;

namespace chart {

GL3DBarChartType::GL3DBarChartType( const uno::Reference<uno::XComponentContext>& xContext ) :
    ChartType(xContext) {}

GL3DBarChartType::~GL3DBarChartType() {}

APPHELPER_XSERVICEINFO_IMPL(
    GL3DBarChartType, OUString("com.sun.star.comp.chart.GL3DBarChartType") );

uno::Sequence<OUString> GL3DBarChartType::getSupportedServiceNames_Static()
{
    uno::Sequence<OUString> aServices(2);
    aServices[0] = CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR;
    aServices[1] = "com.sun.star.chart2.ChartType";
    return aServices;
}

GL3DBarChartType::GL3DBarChartType( const GL3DBarChartType& rOther ) :
    ChartType(rOther) {}

OUString SAL_CALL GL3DBarChartType::getChartType()
        throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_GL3DBAR;
}

com::sun::star::uno::Reference<com::sun::star::util::XCloneable>
GL3DBarChartType::createClone()
    throw (com::sun::star::uno::RuntimeException, std::exception)
{
    return uno::Reference<util::XCloneable>(new GL3DBarChartType(*this));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
