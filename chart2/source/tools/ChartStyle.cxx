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

#include <ChartStyle.hxx>
#include <com/sun/star/chart2/ChartObjectType.hpp>
#include <vector>

namespace chart2
{

ChartStyle::ChartStyle()
    : m_nNumObjects( css::chart2::ChartObjectType::UNKNOWN )
    , m_xChartStyle( std::vector< css::uno::Reference < css::beans::XPropertySet > >( m_nNumObjects, css::uno::Reference < css::beans::XPropertySet > ()))
{
}

ChartStyle::~ChartStyle() {}

css::uno::Reference< css::beans::XPropertySet> ChartStyle::getStyleForObject( const sal_Int16 nChartObjectType )
{
    if ( nChartObjectType >= 0 && nChartObjectType < m_nNumObjects )
        return m_xChartStyle[nChartObjectType];
    else
        throw css::lang::IllegalArgumentException( "Unknown Chart Object Style requested", nullptr, 0 );
}

} // namespace chart2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
