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
#include "macros.hxx"
#include "servicenames_charttypes.hxx"

using namespace ::com::sun::star;

namespace chart
{

AreaChartType::AreaChartType(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        ChartType( xContext )
{}

AreaChartType::AreaChartType( const AreaChartType & rOther ) :
        ChartType( rOther )
{}

AreaChartType::~AreaChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL AreaChartType::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new AreaChartType( *this ));
}

// ____ XChartType ____
OUString SAL_CALL AreaChartType::getChartType()
    throw (uno::RuntimeException)
{
    return CHART2_SERVICE_NAME_CHARTTYPE_AREA;
}

uno::Sequence< OUString > AreaChartType::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = CHART2_SERVICE_NAME_CHARTTYPE_AREA;
    aServices[ 1 ] = "com.sun.star.chart2.ChartType";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AreaChartType,
                             OUString("com.sun.star.comp.chart.AreaChartType") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
