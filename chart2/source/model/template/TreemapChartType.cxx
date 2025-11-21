/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "TreemapChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
TreemapChartType::TreemapChartType() {}

TreemapChartType::TreemapChartType(const TreemapChartType& rOther)
    : ChartType(rOther)
{
}

TreemapChartType::~TreemapChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL TreemapChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new TreemapChartType(*this));
}

rtl::Reference<ChartType> TreemapChartType::cloneChartType() const
{
    return new TreemapChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL TreemapChartType::getChartType() { return CHART2_SERVICE_NAME_CHARTTYPE_TREEMAP; }

uno::Sequence<OUString> TreemapChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL TreemapChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.TreemapChartType";
}

sal_Bool SAL_CALL TreemapChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL TreemapChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_TREEMAP, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_TreemapChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::TreemapChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
