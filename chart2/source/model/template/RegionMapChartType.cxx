/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "RegionMapChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
RegionMapChartType::RegionMapChartType() {}

RegionMapChartType::RegionMapChartType(const RegionMapChartType& rOther)
    : ChartType(rOther)
{
}

RegionMapChartType::~RegionMapChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL RegionMapChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new RegionMapChartType(*this));
}

rtl::Reference<ChartType> RegionMapChartType::cloneChartType() const
{
    return new RegionMapChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL RegionMapChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_REGIONMAP;
}

uno::Sequence<OUString> RegionMapChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL RegionMapChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.RegionMapChartType";
}

sal_Bool SAL_CALL RegionMapChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL RegionMapChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_REGIONMAP, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_RegionMapChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::RegionMapChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
