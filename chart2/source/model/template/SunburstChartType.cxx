/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SunburstChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
SunburstChartType::SunburstChartType() {}

SunburstChartType::SunburstChartType(const SunburstChartType& rOther)
    : ChartType(rOther)
{
}

SunburstChartType::~SunburstChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL SunburstChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new SunburstChartType(*this));
}

rtl::Reference<ChartType> SunburstChartType::cloneChartType() const
{
    return new SunburstChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL SunburstChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_SUNBURST;
}

uno::Sequence<OUString> SunburstChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL SunburstChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.SunburstChartType";
}

sal_Bool SAL_CALL SunburstChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL SunburstChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_SUNBURST, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_SunburstChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::SunburstChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
