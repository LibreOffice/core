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

#include "UpDownBarWrapper.hxx"
#include "Chart2ModelContact.hxx"
#include <DiagramHelper.hxx>
#include <servicenames_charttypes.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/propshlp.hxx>
#include <com/sun/star/chart2/XChartType.hpp>
#include <comphelper/sequence.hxx>

#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
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

struct StaticUpDownBarWrapperPropertyArray_Initializer
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
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
    }
};

struct StaticUpDownBarWrapperPropertyArray : public rtl::StaticAggregate< Sequence< Property >, StaticUpDownBarWrapperPropertyArray_Initializer >
{
};

struct StaticUpDownBarWrapperInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( *StaticUpDownBarWrapperPropertyArray::get() );
        return &aPropHelper;
    }
};

struct StaticUpDownBarWrapperInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticUpDownBarWrapperInfoHelper_Initializer >
{
};

struct StaticUpDownBarWrapperInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticUpDownBarWrapperInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticUpDownBarWrapperInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticUpDownBarWrapperInfo_Initializer >
{
};

struct StaticUpDownBarWrapperDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap( aStaticDefaults );
        return &aStaticDefaults;
    }
private:
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );
    }
};

struct StaticUpDownBarWrapperDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticUpDownBarWrapperDefaults_Initializer >
{
};

} // anonymous namespace

namespace chart::wrapper
{

UpDownBarWrapper::UpDownBarWrapper(
    bool bUp, const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact)
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_aPropertySetName( bUp ? OUString( "WhiteDay" ) : OUString( "BlackDay" ))
{
}

UpDownBarWrapper::~UpDownBarWrapper()
{
}

// ____ XComponent ____
void SAL_CALL UpDownBarWrapper::dispose()
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );
}

void SAL_CALL UpDownBarWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL UpDownBarWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
{
    m_aEventListenerContainer.removeInterface( aListener );
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL UpDownBarWrapper::getPropertySetInfo()
{
    return *StaticUpDownBarWrapperInfo::get();
}
void SAL_CALL UpDownBarWrapper::setPropertyValue( const OUString& rPropertyName, const uno::Any& rValue )
{
    Reference< beans::XPropertySet > xPropSet;

    const Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
    for( Reference< chart2::XChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            Reference< beans::XPropertySet > xTypeProps( xType, uno::UNO_QUERY );
            if(xTypeProps.is())
            {
                xTypeProps->getPropertyValue( m_aPropertySetName ) >>= xPropSet;
            }
        }
    }
    if(xPropSet.is())
        xPropSet->setPropertyValue( rPropertyName, rValue );
}
uno::Any SAL_CALL UpDownBarWrapper::getPropertyValue( const OUString& rPropertyName )
{
    Any aRet;

    Reference< beans::XPropertySet > xPropSet;

    const Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
    for( Reference< chart2::XChartType > const & xType : aTypes )
    {
        if( xType->getChartType() == CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK )
        {
            Reference< beans::XPropertySet > xTypeProps( xType, uno::UNO_QUERY );
            if(xTypeProps.is())
            {
                xTypeProps->getPropertyValue( m_aPropertySetName ) >>= xPropSet;
            }
        }
    }
    if(xPropSet.is())
        aRet = xPropSet->getPropertyValue( rPropertyName );
    return aRet;
}

void SAL_CALL UpDownBarWrapper::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
{
    OSL_FAIL("not implemented");
}

//XMultiPropertySet
//getPropertySetInfo() already declared in XPropertySet
void SAL_CALL UpDownBarWrapper::setPropertyValues( const uno::Sequence< OUString >& rNameSeq, const uno::Sequence< uno::Any >& rValueSeq )
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
uno::Sequence< uno::Any > SAL_CALL UpDownBarWrapper::getPropertyValues( const uno::Sequence< OUString >& rNameSeq )
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
void SAL_CALL UpDownBarWrapper::addPropertiesChangeListener( const uno::Sequence< OUString >& /* aPropertyNames */, const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::firePropertiesChangeEvent( const uno::Sequence< OUString >& /* aPropertyNames */, const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}

//XPropertyState
beans::PropertyState SAL_CALL UpDownBarWrapper::getPropertyState( const OUString& rPropertyName )
{
    uno::Any aDefault( getPropertyDefault( rPropertyName ) );
    uno::Any aValue( getPropertyValue( rPropertyName ) );

    if( aDefault == aValue )
        return beans::PropertyState_DEFAULT_VALUE;

    return beans::PropertyState_DIRECT_VALUE;
}
uno::Sequence< beans::PropertyState > SAL_CALL UpDownBarWrapper::getPropertyStates( const uno::Sequence< OUString >& rNameSeq )
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
void SAL_CALL UpDownBarWrapper::setPropertyToDefault( const OUString& rPropertyName )
{
    setPropertyValue( rPropertyName, getPropertyDefault(rPropertyName) );
}

uno::Any SAL_CALL UpDownBarWrapper::getPropertyDefault( const OUString& rPropertyName )
{
    const tPropertyValueMap& rStaticDefaults = *StaticUpDownBarWrapperDefaults::get();
    return rStaticDefaults.get( StaticUpDownBarWrapperInfoHelper::get()->getHandleByName( rPropertyName ) );
}

//XMultiPropertyStates
//getPropertyStates() already declared in XPropertyState
void SAL_CALL UpDownBarWrapper::setAllPropertiesToDefault(  )
{
    const Sequence< beans::Property >& rPropSeq = *StaticUpDownBarWrapperPropertyArray::get();
    for(beans::Property const & prop : rPropSeq)
    {
        setPropertyToDefault( prop.Name );
    }
}
void SAL_CALL UpDownBarWrapper::setPropertiesToDefault( const uno::Sequence< OUString >& rNameSeq )
{
    for(OUString const & s : rNameSeq)
    {
        setPropertyToDefault( s );
    }
}
uno::Sequence< uno::Any > SAL_CALL UpDownBarWrapper::getPropertyDefaults( const uno::Sequence< OUString >& rNameSeq )
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

OUString SAL_CALL UpDownBarWrapper::getImplementationName()
{
    return "com.sun.star.comp.chart.ChartArea";
}

sal_Bool SAL_CALL UpDownBarWrapper::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL UpDownBarWrapper::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart.ChartArea",
        "com.sun.star.drawing.LineProperties",
        "com.sun.star.drawing.FillProperties",
        "com.sun.star.xml.UserDefinedAttributesSupplier"
    };
}

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
