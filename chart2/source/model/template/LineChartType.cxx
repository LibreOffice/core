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

#include "LineChartType.hxx"
#include <PropertyHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <unonames.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;

namespace
{

enum
{
    PROP_LINECHARTTYPE_CURVE_STYLE,
    PROP_LINECHARTTYPE_CURVE_RESOLUTION,
    PROP_LINECHARTTYPE_SPLINE_ORDER
};

::chart::tPropertyValueMap& StaticLineChartTypeDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults =
        []()
        {
            ::chart::tPropertyValueMap aOutMap;
            ::chart::PropertyHelper::setPropertyValueDefault( aOutMap, PROP_LINECHARTTYPE_CURVE_STYLE, ::chart2::CurveStyle_LINES );
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_LINECHARTTYPE_CURVE_RESOLUTION, 20 );

            // todo: check whether order 3 means polygons of order 3 or 2. (see
            // http://www.people.nnov.ru/fractal/Splines/Basis.htm )
            ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aOutMap, PROP_LINECHARTTYPE_SPLINE_ORDER, 3 );
            return aOutMap;
        }();
    return aStaticDefaults;
}

::cppu::OPropertyArrayHelper& StaticLineChartTypeInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper(
        []()
        {
            std::vector< css::beans::Property > aProperties {
                { CHART_UNONAME_CURVE_STYLE,
                  PROP_LINECHARTTYPE_CURVE_STYLE,
                  cppu::UnoType<chart2::CurveStyle>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { CHART_UNONAME_CURVE_RESOLUTION,
                  PROP_LINECHARTTYPE_CURVE_RESOLUTION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT },
                { CHART_UNONAME_SPLINE_ORDER,
                  PROP_LINECHARTTYPE_SPLINE_ORDER,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT } };

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }());
    return aPropHelper;
}

uno::Reference< beans::XPropertySetInfo >& StaticLineChartTypeInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticLineChartTypeInfoHelper() ) );
    return xPropertySetInfo;
}

} // anonymous namespace

namespace chart
{

LineChartType::LineChartType()
{
}

LineChartType::LineChartType( const LineChartType & rOther ) :
        ChartType( rOther )
{
}

LineChartType::~LineChartType()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL LineChartType::createClone()
{
    return uno::Reference< util::XCloneable >( new LineChartType( *this ));
}

rtl::Reference< ChartType > LineChartType::cloneChartType() const
{
    return new LineChartType( *this );
}

// ____ XChartType ____
OUString SAL_CALL LineChartType::getChartType()
{
    return CHART2_SERVICE_NAME_CHARTTYPE_LINE;
}

// ____ OPropertySet ____
void LineChartType::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticLineChartTypeDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL LineChartType::getInfoHelper()
{
    return StaticLineChartTypeInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL LineChartType::getPropertySetInfo()
{
    return StaticLineChartTypeInfo();
}

OUString SAL_CALL LineChartType::getImplementationName()
{
    return u"com.sun.star.comp.chart.LineChartType"_ustr;
}

sal_Bool SAL_CALL LineChartType::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LineChartType::getSupportedServiceNames()
{
    return {
        CHART2_SERVICE_NAME_CHARTTYPE_LINE,
        u"com.sun.star.chart2.ChartType"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_LineChartType_get_implementation(css::uno::XComponentContext * /*context*/,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::LineChartType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
