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
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace chart
{
namespace wrapper
{

class WrappedStockProperty : public WrappedProperty
{
public:
    explicit WrappedStockProperty( const OUString& rOuterName
        , const ::com::sun::star::uno::Any& rDefaulValue
        , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedStockProperty();

    void setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                    throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Any getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( sal_Bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const = 0;

protected:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    mutable ::com::sun::star::uno::Any     m_aOuterValue;
    ::com::sun::star::uno::Any             m_aDefaultValue;
};

WrappedStockProperty::WrappedStockProperty( const OUString& rOuterName
    , const ::com::sun::star::uno::Any& rDefaulValue
    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
    : WrappedProperty(rOuterName,OUString())
        , m_spChart2ModelContact(spChart2ModelContact)
        , m_aOuterValue()
        , m_aDefaultValue(rDefaulValue)
{
}
WrappedStockProperty::~WrappedStockProperty()
{
}

void WrappedStockProperty::setPropertyValue( const ::com::sun::star::uno::Any& rOuterValue, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    sal_Bool bNewValue = false;
    if( ! (rOuterValue >>= bNewValue) )
        throw lang::IllegalArgumentException( "stock properties require type sal_Bool", 0, 0 );

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
                ControllerLockGuard aCtrlLockGuard( m_spChart2ModelContact->getChartModel() );
                xTemplate->changeDiagram( xDiagram );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }
}

::com::sun::star::uno::Any WrappedStockProperty::getPropertyDefault( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
{
    return m_aDefaultValue;
}

class WrappedVolumeProperty : public WrappedStockProperty
{
public:
    explicit WrappedVolumeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedVolumeProperty();

    ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( sal_Bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const;
};

WrappedVolumeProperty::WrappedVolumeProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStockProperty( "Volume", uno::makeAny(sal_False) , spChart2ModelContact )
{
}
WrappedVolumeProperty::~WrappedVolumeProperty()
{
}

::com::sun::star::uno::Any WrappedVolumeProperty::getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
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
                m_aOuterValue <<= sal_Bool(sal_True);
            else if( !aTemplateAndService.second.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= sal_Bool(sal_False);
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= sal_Bool(sal_False);
    }
    return m_aOuterValue;
}

uno::Reference< chart2::XChartTypeTemplate > WrappedVolumeProperty::getNewTemplate( sal_Bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const
{
    uno::Reference< chart2::XChartTypeTemplate > xTemplate(0);

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
    explicit WrappedUpDownProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedUpDownProperty();

    ::com::sun::star::uno::Any getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xInnerPropertySet ) const
                            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    uno::Reference< chart2::XChartTypeTemplate > getNewTemplate( sal_Bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const;
};
WrappedUpDownProperty::WrappedUpDownProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedStockProperty( "UpDown", uno::makeAny(sal_False) , spChart2ModelContact )
{
}
WrappedUpDownProperty::~WrappedUpDownProperty()
{
}
::com::sun::star::uno::Any WrappedUpDownProperty::getPropertyValue( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& /*xInnerPropertySet*/ ) const
                        throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
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
                m_aOuterValue <<= sal_Bool(sal_True);
            else if( !aTemplateAndService.second.isEmpty() || !m_aOuterValue.hasValue() )
                m_aOuterValue <<= sal_Bool(sal_False);
        }
        else if(!m_aOuterValue.hasValue())
            m_aOuterValue <<= sal_Bool(sal_False);
    }
    return m_aOuterValue;
}
uno::Reference< chart2::XChartTypeTemplate > WrappedUpDownProperty::getNewTemplate( sal_Bool bNewValue, const OUString& rCurrentTemplate, const Reference< lang::XMultiServiceFactory >& xFactory ) const
{
    uno::Reference< chart2::XChartTypeTemplate > xTemplate(0);
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
                  ::getCppuType( reinterpret_cast< sal_Bool * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "UpDown",
                  PROP_CHART_STOCK_UPDOWN,
                  ::getCppuType( reinterpret_cast< sal_Bool * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void WrappedStockProperties::addWrappedProperties( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
{
    rList.push_back( new WrappedVolumeProperty( spChart2ModelContact ) );
    rList.push_back( new WrappedUpDownProperty( spChart2ModelContact ) );
}

} //namespace wrapper
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
