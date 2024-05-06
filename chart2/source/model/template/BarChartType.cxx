/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "BarChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

namespace chart
{

BarChartType::BarChartType()
{}

BarChartType::BarChartType( const BarChartType & rOther ) :
        ChartType( rOther )
{
}

BarChartType::~BarChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL BarChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new BarChartType( *this ));
}

rtl::Reference< ChartType > BarChartType::cloneChartType() const
{
    return new BarChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL BarChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_BAR;
}

uno::Sequence< OUString > BarChartType::getSupportedPropertyRoles()
{
    return { u"FillColor"_ustr, u"BorderColor"_ustr };
}

OUString SAL_CALL BarChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.BarChartType"_ustr;
}

sal_Bool SAL_CALL BarChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL BarChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_BAR,
        u"com.sun.star.chart2.ChartType"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_BarChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::BarChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
