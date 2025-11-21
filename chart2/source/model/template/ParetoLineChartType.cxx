/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ParetoLineChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace ::com::sun::star;

namespace chart
{
ParetoLineChartType::ParetoLineChartType() {}

ParetoLineChartType::ParetoLineChartType(const ParetoLineChartType& rOther)
    : ChartType(rOther)
{
}

ParetoLineChartType::~ParetoLineChartType() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL ParetoLineChartType::createClone()
{
    return uno::Reference<util::XCloneable>(new ParetoLineChartType(*this));
}

rtl::Reference<ChartType> ParetoLineChartType::cloneChartType() const
{
    return new ParetoLineChartType(*this);
}

// ____ XChartType ____
OUString SAL_CALL ParetoLineChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_PARETOLINE;
}

uno::Sequence<OUString> ParetoLineChartType::getSupportedPropertyRoles()
{
    return { "FillColor", "BorderColor" };
}

OUString SAL_CALL ParetoLineChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.ParetoLineChartType";
}

sal_Bool SAL_CALL ParetoLineChartType::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL ParetoLineChartType::getSupportedServiceNames()
{
    return { CHART2_SERVICE_NAME_CHARTTYPE_PARETOLINE, "com.sun.star.chart2.ChartType" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_ParetoLineChartType_get_implementation(
    css::uno::XComponentContext* /*context*/, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::ParetoLineChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
