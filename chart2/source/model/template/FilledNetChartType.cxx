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
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "PolarCoordinateSystem.hxx"
#include "Scaling.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/AxisType.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace chart
{

FilledNetChartType::FilledNetChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        NetChartType_Base( xContext )
{}

FilledNetChartType::FilledNetChartType( const FilledNetChartType & rOther ) :
        NetChartType_Base( rOther )
{
}

FilledNetChartType::~FilledNetChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL FilledNetChartType::createClone()
    throw (uno::RuntimeException, std::exception)
{
    return uno::Reference< util::XCloneable >( new FilledNetChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL FilledNetChartType::getChartType()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET);
}

uno::Sequence< OUString > FilledNetChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_FILLED_NET;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    aServices[ 2 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL FilledNetChartType::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString FilledNetChartType::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.chart.FilledNetChartType");
}

sal_Bool SAL_CALL FilledNetChartType::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FilledNetChartType::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart_FilledNetChartType_get_implementation(css::uno::XComponentContext *context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::FilledNetChartType(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
