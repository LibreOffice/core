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

#include "AreaChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

namespace chart
{

AreaChartType::AreaChartType()
{}

AreaChartType::AreaChartType( const AreaChartType & rOther ) :
        ChartType( rOther )
{}

AreaChartType::~AreaChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL AreaChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new AreaChartType( *this ));
}

rtl::Reference< ChartType > AreaChartType::cloneChartType() const
{
    return new AreaChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL AreaChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_AREA;
}

OUString SAL_CALL AreaChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.AreaChartType"_ustr;
}

sal_Bool SAL_CALL AreaChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL AreaChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_AREA,
        u"com.sun.star.chart2.ChartType"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_AreaChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::AreaChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
