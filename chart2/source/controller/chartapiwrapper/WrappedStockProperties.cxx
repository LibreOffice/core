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

#include "WrappedStockProperties.hxx"
#include "macros.hxx"
#include "FastPropertyIdRanges.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"
#include "ControllerLockGuard.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedStockProperty : public WrappedProperty
{
public:
    explicit WrappedStockProperty( const OUString& rOuterName
        , const css::uno::Any& rDefaulValue
        , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );

    void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

    virtual uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const = 0;

protected:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable css::uno::Any                   m_aOuterValue;
    css::uno::Any                           m_aDefaultValue;
};

WrappedStockProperty::WrappedStockProperty( const OUString& rOuterName
    , const css::uno::Any& rDefaulValue
    , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact )
    : WrappedProperty(rOuterName,OUString())
        , m_spChart2ModelContact(spChart2ModelContact)
        , m_aOuterValue()
        , m_aDefaultValue(rDefaulValue)
{
}

void WrappedStockProperty::setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "stock properties require type sal_Bool", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    sal_Int32 nDimension = ::chart::DiagramHelper::getDimension( xDiagram );
    if( xChartDoc.is() && xDiagram.is() && nDimension==2 )
    {
        Reference< lang::XMultiServiceFactory > xFactory( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
        DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                DiagramHelper::getTemplateForDiagram( xDiagram, xFactory );

        uno::Reference< chart2::XChartTypeTemplate > xTemplate =
                getNewTemplate( bNewValue, aTemplateAndService.second, xFactory );

        if(xTemplate.is())
        {
            try
            {
                // locked controllers
                ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
                xTemplate->changeDiagram( xDiagram );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

css::uno::Any WrappedStockProperty::getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return m_aDefaultValue;
}

class WrappedVolumeProperty : public WrappedStockProperty
{
public:
    explicit WrappedVolumeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const override;
};

WrappedVolumeProperty::WrappedVolumeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : WrappedStockProperty( "Volume", uno::Any(false) , spChart2ModelContact )
{
}

css::uno::Any WrappedVolumeProperty::getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesVector(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        if( aSeriesVector.size() > 0 )
        {
            Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
            DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                    DiagramHelper::getTemplateForDiagram( xDiagram, xFact );

            if(    aTemplateAndService.second == "com.sun.star.chart2.template.StockVolumeLowHighClose"
                || aTemplateAndService.second == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
                m_aOuterValue <<= true;
            else if( !aTemplateAndService.second.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= false;
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= false;
    }
    return m_aOuterValue;
}

uno::Reference< chart2::XChartTypeTemplate > WrappedVolumeProperty::getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const
{
    uno::Reference< chart2::XChartTypeTemplate > xTemplate(nullptr);

    if(!xFactory.is())
        return xTemplate;

    if( bNewValue ) //add volume
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockVolumeLowHighClose" ), uno::UNO_QUERY );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockOpenLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ), uno::UNO_QUERY );
    }
    else //remove volume
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockLowHighClose" ), uno::UNO_QUERY );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockOpenLowHighClose" ), uno::UNO_QUERY );
    }
    return xTemplate;
}

class WrappedUpDownProperty : public WrappedStockProperty
{
public:
    explicit WrappedUpDownProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const override;
};

WrappedUpDownProperty::WrappedUpDownProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : WrappedStockProperty( "UpDown", uno::Any(false) , spChart2ModelContact )
{
}

css::uno::Any WrappedUpDownProperty::getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesVector(
            DiagramHelper::getDataSeriesFromDiagram( xDiagram ) );
        if( aSeriesVector.size() > 0 )
        {
            Reference< lang::XMultiServiceFactory > xFact( xChartDoc->getChartTypeManager(), uno::UNO_QUERY );
            DiagramHelper::tTemplateWithServiceName aTemplateAndService =
                    DiagramHelper::getTemplateForDiagram( xDiagram, xFact );

            if(    aTemplateAndService.second == "com.sun.star.chart2.template.StockOpenLowHighClose"
                || aTemplateAndService.second == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
                m_aOuterValue <<= true;
            else if( !aTemplateAndService.second.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= false;
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= false;
    }
    return m_aOuterValue;
}
uno::Reference< chart2::XChartTypeTemplate > WrappedUpDownProperty::getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const
{
    uno::Reference< chart2::XChartTypeTemplate > xTemplate(nullptr);
    if( bNewValue ) //add open series
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockOpenLowHighClose" ), uno::UNO_QUERY );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ), uno::UNO_QUERY );
    }
    else //remove open series
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockOpenLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockLowHighClose" ), uno::UNO_QUERY );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
            xTemplate.set( xFactory->createInstance( "com.sun.star.chart2.template.StockVolumeLowHighClose" ), uno::UNO_QUERY );
    }
    return xTemplate;
}

namespace
{
enum
{
    //spline properties
      PROP_CHART_STOCK_VOLUME = FAST_PROPERTY_ID_START_CHART_STOCK_PROP
    , PROP_CHART_STOCK_UPDOWN
};

}//anonymous namespace

void WrappedStockProperties::addProperties( ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "Volume",
                  PROP_CHART_STOCK_VOLUME,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "UpDown",
                  PROP_CHART_STOCK_UPDOWN,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void WrappedStockProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.push_back( new WrappedVolumeProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedUpDownProperty( spChart2ModelContact ) );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
