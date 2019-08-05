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
#include <cppuhelper/supportsservice.hxx>
#include <vector>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace chart2
{

ChartObjectStyle::ChartObjectStyle(::cppu::IPropertyArrayHelper& rArrayHelper, const chart::tPropertyValueMap& rPropertyMap):
    OPropertySet( m_aMutex ),
    mrArrayHelper(rArrayHelper),
    mrPropertyMap(rPropertyMap)
{
}

ChartObjectStyle::~ChartObjectStyle()
{
}

sal_Bool SAL_CALL ChartObjectStyle::isInUse()
{
    return true;
}

sal_Bool SAL_CALL ChartObjectStyle::isUserDefined()
{
    return true;
}

OUString SAL_CALL ChartObjectStyle::getParentStyle()
{
    return "";
}

void SAL_CALL ChartObjectStyle::setParentStyle(const OUString&)
{
}

// ____ OPropertySet ____
css::uno::Any ChartObjectStyle::GetDefaultValue( sal_Int32 nHandle ) const
{
    chart::tPropertyValueMap::const_iterator aFound( mrPropertyMap.find( nHandle ) );
    if( aFound == mrPropertyMap.end() )
        return css::uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ChartObjectStyle::getInfoHelper()
{
    return mrArrayHelper;
}

// ____ XPropertySet ____
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL ChartObjectStyle::getPropertySetInfo()
{
    //return *mrArrayHelper;
    return css::uno::Reference<css::beans::XPropertySetInfo>();
}

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

// _____ XServiceInfo _____
OUString SAL_CALL ChartStyle::getImplementationName()
{
    return OUString("com.sun.star.comp.chart2.ChartStyle");
}

sal_Bool SAL_CALL ChartStyle::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ChartStyle::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.ChartStyle" };
}

} //  namespace chart2

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_ChartStyle_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart2::ChartStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
