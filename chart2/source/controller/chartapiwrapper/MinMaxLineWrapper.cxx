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
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <comphelper/sequence.hxx>

#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

namespace
{

struct StaticMinMaxLineWrapperDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        ::chart::LinePropertiesHelper::AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
};

struct StaticMinMaxLineWrapperDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticMinMaxLineWrapperDefaults_Initializer >
{
};

struct StaticMinMaxLineWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;

        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticMinMaxLineWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticMinMaxLineWrapperPropertyArray_Initializer >
{
};

struct StaticMinMaxLineWrapperInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( *StaticMinMaxLineWrapperPropertyArray::get() );
        return &aPropHelper;
    }
};

struct StaticMinMaxLineWrapperInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticMinMaxLineWrapperInfoHelper_Initializer >
{
};

struct StaticMinMaxLineWrapperInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticMinMaxLineWrapperInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticMinMaxLineWrapperInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticMinMaxLineWrapperInfo_Initializer >
{
};

} // anonymous namespace

namespace chart::wrapper
{

MinMaxLineWrapper::MinMaxLineWrapper(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_aWrappedLineJointProperty( "LineJoint", uno::Any( drawing::LineJoint_NONE ))
{
}

MinMaxLineWrapper::~MinMaxLineWrapper()
{
}

// ____ XComponent ____
void SAL_CALL MinMaxLineWrapper::dispose()
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );
}

void SAL_CALL MinMaxLineWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL MinMaxLineWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL MinMaxLineWrapper::getPropertySetInfo()
{
    return *StaticMinMaxLineWrapperInfo::get();
}

void SAL_CALL MinMaxLineWrapper::setPropertyValue( const OUString& rPropertyName, const uno::Any& rValue )
{
    Reference< beans::XPropertySet > xPropSet;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    const Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( Reference< chart2::XChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            Reference< chart2::XDataSeriesContainer > xSeriesContainer(xType,uno::UNO_QUERY);
            if( xSeriesContainer.is() )
            {
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                if(aSeriesSeq.hasElements())
                {
                    xPropSet.set(aSeriesSeq[0],uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        if( rPropertyName == "LineColor" )
                            xPropSet->setPropertyValue( "Color", rValue );
                        else if( rPropertyName == "LineTransparence" )
                            xPropSet->setPropertyValue( "Transparency", rValue );
                        else if( rPropertyName == m_aWrappedLineJointProperty.getOuterName() )
                            m_aWrappedLineJointProperty.setPropertyValue( rValue, xPropSet );
                        else
                            xPropSet->setPropertyValue( rPropertyName, rValue );
                        return;
                    }
                }
            }
        }
    }
}
uno::Any SAL_CALL MinMaxLineWrapper::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;

    Reference< beans::XPropertySet > xPropSet;

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    const Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( Reference< chart2::XChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            Reference< chart2::XDataSeriesContainer > xSeriesContainer(xType,uno::UNO_QUERY);
            if( xSeriesContainer.is() )
            {
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                if(aSeriesSeq.hasElements())
                {
                    xPropSet.set(aSeriesSeq[0],uno::UNO_QUERY);
                    break;
                }
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
    const tPropertyValueMap& rStaticDefaults = *StaticMinMaxLineWrapperDefaults::get();
    return rStaticDefaults.get( StaticMinMaxLineWrapperInfoHelper::get()->getHandleByName( rPropertyName ) );
}

//XMultiPropertyStates
//getPropertyStates() already declared in XPropertyState
void SAL_CALL MinMaxLineWrapper::setAllPropertiesToDefault(  )
{
    const Sequence< beans::Property >& rPropSeq = *StaticMinMaxLineWrapperPropertyArray::get();
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
