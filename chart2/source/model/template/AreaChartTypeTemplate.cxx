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

#include "AreaChartTypeTemplate.hxx"
#include "AreaChartType.hxx"
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;

namespace
{

enum
{
    PROP_AREA_TEMPLATE_DIMENSION
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Dimension",
                  PROP_AREA_TEMPLATE_DIMENSION,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

::cppu::OPropertyArrayHelper& StaticAreaChartTypeTemplateInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{

AreaChartTypeTemplate::AreaChartTypeTemplate(
    uno::Reference<
        uno::XComponentContext > const & xContext,
    const OUString & rServiceName,
    StackMode eStackMode,
    sal_Int32 nDim /* = 2 */ ) :
        ChartTypeTemplate( xContext, rServiceName ),
        m_eStackMode( eStackMode )
{
    setFastPropertyValue_NoBroadcast( PROP_AREA_TEMPLATE_DIMENSION, uno::Any( nDim ));
}

AreaChartTypeTemplate::~AreaChartTypeTemplate()
{}

// ____ OPropertySet ____
void AreaChartTypeTemplate::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
    {
        ::chart::tPropertyValueMap aTmp;
        ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( aTmp, PROP_AREA_TEMPLATE_DIMENSION, 2 );
        return aTmp;
    }();
    tPropertyValueMap::const_iterator aFound( aStaticDefaults.find( nHandle ) );
    if( aFound == aStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL AreaChartTypeTemplate::getInfoHelper()
{
    return StaticAreaChartTypeTemplateInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL AreaChartTypeTemplate::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticAreaChartTypeTemplateInfoHelper() ) );
    return xPropertySetInfo;
}

sal_Int32 AreaChartTypeTemplate::getDimension() const
{
    sal_Int32 nDim = 2;
    try
    {
        // note: UNO-methods are never const
        const_cast< AreaChartTypeTemplate * >( this )->
            getFastPropertyValue( PROP_AREA_TEMPLATE_DIMENSION ) >>= nDim;
    }
    catch( const beans::UnknownPropertyException & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return nDim;
}

StackMode AreaChartTypeTemplate::getStackMode( sal_Int32 /* nChartTypeIndex */ ) const
{
    return m_eStackMode;
}

// ____ ChartTypeTemplate ____
void AreaChartTypeTemplate::applyStyle2(
    const rtl::Reference< DataSeries >& xSeries,
    ::sal_Int32 nChartTypeIndex,
    ::sal_Int32 nSeriesIndex,
    ::sal_Int32 nSeriesCount )
{
    ChartTypeTemplate::applyStyle2( xSeries, nChartTypeIndex, nSeriesIndex, nSeriesCount );
    DataSeriesHelper::setPropertyAlsoToAllAttributedDataPoints( xSeries, "BorderStyle", uno::Any( drawing::LineStyle_NONE ) );
}

void AreaChartTypeTemplate::resetStyles2( const rtl::Reference< ::chart::Diagram >& xDiagram )
{
    ChartTypeTemplate::resetStyles2( xDiagram );
    std::vector< rtl::Reference< ::chart::DataSeries > > aSeriesVec(
        xDiagram->getDataSeries());
    uno::Any aLineStyleAny( drawing::LineStyle_NONE );
    for (auto const& series : aSeriesVec)
    {
        if( series->getPropertyValue( "BorderStyle") == aLineStyleAny )
        {
            series->setPropertyToDefault( "BorderStyle");
        }
    }
}

rtl::Reference< ChartType > AreaChartTypeTemplate::getChartTypeForIndex( sal_Int32 /*nChartTypeIndex*/ )
{
    return new AreaChartType();
}

rtl::Reference< ChartType > AreaChartTypeTemplate::getChartTypeForNewSeries2(
        const std::vector< rtl::Reference< ChartType > >& aFormerlyUsedChartTypes )
{
    rtl::Reference< ChartType > xResult( getChartTypeForIndex( 0 ) );
    ChartTypeTemplate::copyPropertiesFromOldToNewCoordinateSystem( aFormerlyUsedChartTypes, xResult );
    return xResult;
}

IMPLEMENT_FORWARD_XINTERFACE2( AreaChartTypeTemplate, ChartTypeTemplate, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AreaChartTypeTemplate, ChartTypeTemplate, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
