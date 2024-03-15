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
#include "Chart2ModelContact.hxx"
#include <FastPropertyIdRanges.hxx>
#include <DataSeries.hxx>
#include <ControllerLockGuard.hxx>
#include <WrappedProperty.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <ChartTypeManager.hxx>
#include <ChartTypeTemplate.hxx>
#include <utility>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace chart::wrapper
{

namespace {

class WrappedStockProperty : public WrappedProperty
{
public:
    explicit WrappedStockProperty( const OUString& rOuterName
        , css::uno::Any aDefaultValue
        , std::shared_ptr<Chart2ModelContact> spChart2ModelContact );

    void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    css::uno::Any getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

    virtual rtl::Reference< ::chart::ChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const rtl::Reference< ::chart::ChartTypeManager >& xFactory ) const = 0;

protected:
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable css::uno::Any                   m_aOuterValue;
    css::uno::Any                           m_aDefaultValue;
};

}

WrappedStockProperty::WrappedStockProperty( const OUString& rOuterName
    , css::uno::Any aDefaultValue
    , std::shared_ptr<Chart2ModelContact> spChart2ModelContact )
    : WrappedProperty(rOuterName,OUString())
        , m_spChart2ModelContact(std::move(spChart2ModelContact))
        , m_aDefaultValue(std::move(aDefaultValue))
{
}

void WrappedStockProperty::setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "stock properties require type sal_Bool", nullptr, 0 );

    m_aOuterValue = rOuterValue;

    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( !xChartDoc || !xDiagram )
        return;
    sal_Int32 nDimension = xDiagram->getDimension();
    if( nDimension != 2 )
        return;

    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
    Diagram::tTemplateWithServiceName aTemplateAndService =
            xDiagram->getTemplate( xChartTypeManager );

    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate =
            getNewTemplate( bNewValue, aTemplateAndService.sServiceName, xChartTypeManager );

    if(!xTemplate.is())
        return;

    try
    {
        // locked controllers
        ControllerLockGuardUNO aCtrlLockGuard( m_spChart2ModelContact->getDocumentModel() );
        xTemplate->changeDiagram( xDiagram );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

css::uno::Any WrappedStockProperty::getPropertyDefault( const css::uno::Reference< css::beans::XPropertyState >& /*xInnerPropertyState*/ ) const
{
    return m_aDefaultValue;
}

namespace {

class WrappedVolumeProperty : public WrappedStockProperty
{
public:
    explicit WrappedVolumeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    rtl::Reference< ::chart::ChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const rtl::Reference< ::chart::ChartTypeManager >& xFactory ) const override;
};

}

WrappedVolumeProperty::WrappedVolumeProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : WrappedStockProperty( "Volume", uno::Any(false) , spChart2ModelContact )
{
}

css::uno::Any WrappedVolumeProperty::getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        std::vector< rtl::Reference< DataSeries > > aSeriesVector =
            xDiagram->getDataSeries();
        if( !aSeriesVector.empty() )
        {
            rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
            Diagram::tTemplateWithServiceName aTemplateAndService =
                    xDiagram->getTemplate( xChartTypeManager );

            if(    aTemplateAndService.sServiceName == "com.sun.star.chart2.template.StockVolumeLowHighClose"
                || aTemplateAndService.sServiceName == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
                m_aOuterValue <<= true;
            else if( !aTemplateAndService.sServiceName.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= false;
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= false;
    }
    return m_aOuterValue;
}

rtl::Reference< ::chart::ChartTypeTemplate > WrappedVolumeProperty::getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const rtl::Reference< ::chart::ChartTypeManager >& xFactory ) const
{
    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate;

    if(!xFactory.is())
        return xTemplate;

    if( bNewValue ) //add volume
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockVolumeLowHighClose" );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockOpenLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" );
    }
    else //remove volume
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockLowHighClose" );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockOpenLowHighClose" );
    }
    return xTemplate;
}

namespace {

class WrappedUpDownProperty : public WrappedStockProperty
{
public:
    explicit WrappedUpDownProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);

    css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    rtl::Reference< ::chart::ChartTypeTemplate > getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const rtl::Reference< ChartTypeManager >& xFactory ) const override;
};

}

WrappedUpDownProperty::WrappedUpDownProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : WrappedStockProperty( "UpDown", uno::Any(false) , spChart2ModelContact )
{
}

css::uno::Any WrappedUpDownProperty::getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
{
    rtl::Reference< ChartModel > xChartDoc( m_spChart2ModelContact->getDocumentModel() );
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    if( xDiagram.is() && xChartDoc.is() )
    {
        std::vector< rtl::Reference< DataSeries > > aSeriesVector =
            xDiagram->getDataSeries();
        if( !aSeriesVector.empty() )
        {
            rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartDoc->getTypeManager();
            Diagram::tTemplateWithServiceName aTemplateAndService =
                    xDiagram->getTemplate( xChartTypeManager );

            if(    aTemplateAndService.sServiceName == "com.sun.star.chart2.template.StockOpenLowHighClose"
                || aTemplateAndService.sServiceName == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
                m_aOuterValue <<= true;
            else if( !aTemplateAndService.sServiceName.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= false;
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= false;
    }
    return m_aOuterValue;
}
rtl::Reference< ::chart::ChartTypeTemplate > WrappedUpDownProperty::getNewTemplate( bool bNewValue, const OUString& rCurrentTemplate, const rtl::Reference< ChartTypeManager >& xFactory ) const
{
    rtl::Reference< ::chart::ChartTypeTemplate > xTemplate;
    if( bNewValue ) //add open series
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockOpenLowHighClose" );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" );
    }
    else //remove open series
    {
        if( rCurrentTemplate == "com.sun.star.chart2.template.StockOpenLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockLowHighClose" );
        else if( rCurrentTemplate == "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" )
            xTemplate = xFactory->createTemplate( "com.sun.star.chart2.template.StockVolumeLowHighClose" );
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

void WrappedStockProperties::addProperties( std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Volume",
                  PROP_CHART_STOCK_VOLUME,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
    rOutProperties.emplace_back( "UpDown",
                  PROP_CHART_STOCK_UPDOWN,
                  cppu::UnoType<sal_Bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID );
}

void WrappedStockProperties::addWrappedProperties( std::vector< std::unique_ptr<WrappedProperty> >& rList
                                    , const std::shared_ptr< Chart2ModelContact >& spChart2ModelContact )
{
    rList.emplace_back( new WrappedVolumeProperty( spChart2ModelContact ) );
    rList.emplace_back( new WrappedUpDownProperty( spChart2ModelContact ) );
}

} //namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
