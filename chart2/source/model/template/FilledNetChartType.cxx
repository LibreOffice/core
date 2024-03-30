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

#include "FilledNetChartType.hxx"
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;

namespace chart
{

FilledNetChartType::FilledNetChartType()
{}

FilledNetChartType::FilledNetChartType( const FilledNetChartType & rOther ) :
        NetChartType_Base( rOther )
{
}

FilledNetChartType::~FilledNetChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL FilledNetChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new FilledNetChartType( *this ));
}

rtl::Reference< ChartType > FilledNetChartType::cloneChartType() const
{
    return new FilledNetChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL FilledNetChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET;
}

OUString SAL_CALL FilledNetChartType::getImplementationName()
{
    return "com.sun.star.comp.chart.FilledNetChartType";
}

sal_Bool SAL_CALL FilledNetChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FilledNetChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET,
        "com.sun.star.chart2.ChartType",
        "com.sun.star.beans.PropertySet" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_FilledNetChartType_get_implementation(css::uno::XComponentContext * /*context*/,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::FilledNetChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
