/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "GL3DBarChartTypeTemplate.hxx"

using namespace com::sun::star;

namespace chart {

GL3DBarChartTypeTemplate::GL3DBarChartTypeTemplate(
    const uno::Reference<uno::XComponentContext>& xContext, const OUString& rServiceName ) :
    ChartTypeTemplate(xContext, rServiceName) {}

GL3DBarChartTypeTemplate::~GL3DBarChartTypeTemplate() {}

uno::Reference<chart2::XChartType> GL3DBarChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    return uno::Reference<chart2::XChartType>();
}

uno::Reference<chart2::XChartType>
GL3DBarChartTypeTemplate::getChartTypeForNewSeries(
    const uno::Sequence<uno::Reference<chart2::XChartType> >& /*xOldChartTypes*/ )
{
    return uno::Reference<chart2::XChartType>();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
