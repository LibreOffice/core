/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WaterfallChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
WaterfallChartType::WaterfallChartType() {}

WaterfallChartType::WaterfallChartType(const WaterfallChartType& rOther)
    : ChartType(rOther)
{
}

WaterfallChartType::~WaterfallChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL WaterfallChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new WaterfallChartType(*this));
}

rtl::Reference<ChartType> WaterfallChartType::cloneChartType() const
{
    return new WaterfallChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL WaterfallChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_WATERFALL;
}

uno::Sequence<OUString> WaterfallChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL WaterfallChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.WaterfallChartType";
}

sal_Bool SAL_CALL WaterfallChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL WaterfallChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_WATERFALL, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_WaterfallChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::WaterfallChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
