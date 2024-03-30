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

#include "MinMaxLineWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include <ChartType.hxx>
#include <servicenames_charttypes.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <comphelper/sequence.hxx>
#include <DataSeries.hxx>
#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace
{

Sequence< Property >& StaticMinMaxLineWrapperPropertyArray()
{
    static Sequence< Property > aPropSeq = []()
        {
            std::vector< css::beans::Property > aProperties;

            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropSeq;
};

::cppu::OPropertyArrayHelper& StaticMinMaxLineWrapperInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper( StaticMinMaxLineWrapperPropertyArray() );
    return aPropHelper;
};

uno::Reference< beans::XPropertySetInfo >& StaticMinMaxLineWrapperInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo( StaticMinMaxLineWrapperInfoHelper() ) );
    return xPropertySetInfo;
};

} // anonymous namespace

namespace chart::wrapper
{

MinMaxLineWrapper::MinMaxLineWrapper(std::shared_ptr<Chart2ModelContact> spChart2ModelContact)
        : m_spChart2ModelContact(std::move( spChart2ModelContact ))
        , m_aWrappedLineJointProperty( "LineJoint", uno::Any( drawing::LineJoint_NONE ))
{
}

MinMaxLineWrapper::~MinMaxLineWrapper()
{
}

// ____ XComponent ____
void SAL_CALL MinMaxLineWrapper::dispose()
{
    std::unique_lock g(m_aMutex);
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( g, lang::EventObject( xSource ) );
}

void SAL_CALL MinMaxLineWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.addInterface( g, xListener );
}

void SAL_CALL MinMaxLineWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    m_aEventListenerContainer.removeInterface( g, aListener );
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL MinMaxLineWrapper::getPropertySetInfo()
{
    return StaticMinMaxLineWrapperInfo();
}

void SAL_CALL MinMaxLineWrapper::setPropertyValue( const OUString& rPropertyName, const uno::Any& rValue )
{
    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    const std::vector< rtl::Reference< ChartType > > & aTypes = xDiagram->getChartTypes();
    for( rtl::Reference< ChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            const std::vector< rtl::Reference< DataSeries > > & aSeriesSeq( xType->getDataSeries2() );
            if(!aSeriesSeq.empty())
            {
                if(aSeriesSeq[0].is())
                {
                    if( rPropertyName == "LineColor" )
                        aSeriesSeq[0]->setPropertyValue( "Color", rValue );
                    else if( rPropertyName == "LineTransparence" )
                        aSeriesSeq[0]->setPropertyValue( "Transparency", rValue );
                    else if( rPropertyName == m_aWrappedLineJointProperty.getOuterName() )
                        m_aWrappedLineJointProperty.setPropertyValue( rValue, aSeriesSeq[0] );
                    else
                        aSeriesSeq[0]->setPropertyValue( rPropertyName, rValue );
                    return;
                }
            }
        }
    }
}
uno::Any SAL_CALL MinMaxLineWrapper::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;

    rtl::Reference< DataSeries > xPropSet;

    rtl::Reference< ::chart::Diagram > xDiagram( m_spChart2ModelContact->getDiagram() );
    const std::vector< rtl::Reference< ChartType > > aTypes = xDiagram->getChartTypes();
    for( rtl::Reference< ChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            const std::vector< rtl::Reference< DataSeries > > & aSeriesSeq( xType->getDataSeries2() );
            if(!aSeriesSeq.empty())
            {
                xPropSet = aSeriesSeq[0];
                break;
            }
        }
    }
    if(xPropSet.is())
    {
        if( rPropertyName == "LineColor" )
            aRet = xPropSet->getPropertyValue( "Color" );
        else if( rPropertyName == "LineTransparence" )
            aRet = xPropSet->getPropertyValue( "Transparency" );
        else if( rPropertyName == m_aWrappedLineJointProperty.getOuterName() )
            aRet = m_aWrappedLineJointProperty.getPropertyValue( xPropSet );
        else
            aRet = xPropSet->getPropertyValue( rPropertyName );

    }
    return aRet;
}

void SAL_CALL MinMaxLineWrapper::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}

//XMultiPropertySet
//getPropertySetInfo() already declared in XPropertySet
void SAL_CALL MinMaxLineWrapper::setPropertyValues( const uno::Sequence< OUString >& rNameSeq, const uno::Sequence< uno::Any >& rValueSeq )
{
    sal_Int32 nMinCount = std::min( rValueSeq.getLength(), rNameSeq.getLength() );
    for(sal_Int32 nN=0; nN<nMinCount; nN++)
    {
        OUString aPropertyName( rNameSeq[nN] );
        try
        {
            setPropertyValue( aPropertyName, rValueSeq[nN] );
        }
        catch( const beans::UnknownPropertyException& )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }
    //todo: store unknown properties elsewhere
}
uno::Sequence< uno::Any > SAL_CALL MinMaxLineWrapper::getPropertyValues( const uno::Sequence< OUString >& rNameSeq )
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            pRetSeq[nN] = getPropertyValue( aPropertyName );
        }
    }
    return aRetSeq;
}
void SAL_CALL MinMaxLineWrapper::addPropertiesChangeListener(
    const uno::Sequence< OUString >& /* aPropertyNames */,
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::firePropertiesChangeEvent(
    const uno::Sequence< OUString >& /* aPropertyNames */,
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

//XPropertyState
beans::PropertyState SAL_CALL MinMaxLineWrapper::getPropertyState( const OUString& rPropertyName )
{
    if( rPropertyName == m_aWrappedLineJointProperty.getOuterName() )
        return beans::PropertyState_DEFAULT_VALUE;

    uno::Any aDefault( getPropertyDefault( rPropertyName ) );
    uno::Any aValue( getPropertyValue( rPropertyName ) );

    if( aDefault == aValue )
        return beans::PropertyState_DEFAULT_VALUE;

    return beans::PropertyState_DIRECT_VALUE;
}
uno::Sequence< beans::PropertyState > SAL_CALL MinMaxLineWrapper::getPropertyStates( const uno::Sequence< OUString >& rNameSeq )
{
    Sequence< beans::PropertyState > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            pRetSeq[nN] = getPropertyState( aPropertyName );
        }
    }
    return aRetSeq;
}
void SAL_CALL MinMaxLineWrapper::setPropertyToDefault( const OUString& rPropertyName )
{
    setPropertyValue( rPropertyName, getPropertyDefault(rPropertyName) );
}

uno::Any SAL_CALL MinMaxLineWrapper::getPropertyDefault( const OUString& rPropertyName )
{
    static const ::chart::tPropertyValueMap aStaticDefaults = []
        {
            ::chart::tPropertyValueMap aTmp;
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aTmp );
            return aTmp;
        }();
    tPropertyValueMap::const_iterator aFound( aStaticDefaults.find( StaticMinMaxLineWrapperInfoHelper().getHandleByName( rPropertyName ) ) );
    if( aFound == aStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

//XMultiPropertyStates
//getPropertyStates() already declared in XPropertyState
void SAL_CALL MinMaxLineWrapper::setAllPropertiesToDefault(  )
{
    const Sequence< beans::Property >& rPropSeq = StaticMinMaxLineWrapperPropertyArray();
    for(beans::Property const & prop : rPropSeq)
    {
        setPropertyToDefault( prop.Name );
    }
}
void SAL_CALL MinMaxLineWrapper::setPropertiesToDefault( const uno::Sequence< OUString >& rNameSeq )
{
    for(OUString const & s : rNameSeq)
    {
        setPropertyToDefault( s );
    }
}
uno::Sequence< uno::Any > SAL_CALL MinMaxLineWrapper::getPropertyDefaults( const uno::Sequence< OUString >& rNameSeq )
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.hasElements() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        auto pRetSeq = aRetSeq.getArray();
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            OUString aPropertyName( rNameSeq[nN] );
            pRetSeq[nN] = getPropertyDefault( aPropertyName );
        }
    }
    return aRetSeq;
}

OUString SAL_CALL MinMaxLineWrapper::getImplementationName()
{
    return "com.sun.star.comp.chart.ChartLine";
}

sal_Bool SAL_CALL MinMaxLineWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL MinMaxLineWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartLine",
        "com.sun.star.xml.UserDefinedAttributesSupplier",
        "com.sun.star.drawing.LineProperties"
    };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
