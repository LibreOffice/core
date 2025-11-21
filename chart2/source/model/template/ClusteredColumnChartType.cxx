/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ClusteredColumnChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
ClusteredColumnChartType::ClusteredColumnChartType() {}

ClusteredColumnChartType::ClusteredColumnChartType(const ClusteredColumnChartType& rOther)
    : ChartType(rOther)
{
}

ClusteredColumnChartType::~ClusteredColumnChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL ClusteredColumnChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new ClusteredColumnChartType(*this));
}

rtl::Reference<ChartType> ClusteredColumnChartType::cloneChartType() const
{
    return new ClusteredColumnChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL ClusteredColumnChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_CLUSTEREDCOLUMN;
}

uno::Sequence<OUString> ClusteredColumnChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL ClusteredColumnChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.ClusteredColumnChartType";
}

sal_Bool SAL_CALL ClusteredColumnChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ClusteredColumnChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_CLUSTEREDCOLUMN, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_ClusteredColumnChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::ClusteredColumnChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
