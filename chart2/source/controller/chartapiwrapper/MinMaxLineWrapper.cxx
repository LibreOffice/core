/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "MinMaxLineWrapper.hxx"
#include "macros.hxx"
#include "Chart2ModelContact.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>

#include "LineProperties.hxx"
#include "UserDefinedProperties.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.ChartLine" ));

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;

        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

MinMaxLineWrapper::MinMaxLineWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_pPropertyArrayHelper()
        , m_aWrappedLineJointProperty( C2U("LineJoint"), uno::makeAny( drawing::LineJoint_NONE ))
{
}

MinMaxLineWrapper::~MinMaxLineWrapper()
{
}

// ____ XComponent ____
void SAL_CALL MinMaxLineWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    MutexGuard aGuard( GetMutex());
    m_xInfo.clear();
}

void SAL_CALL MinMaxLineWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL MinMaxLineWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

::cppu::IPropertyArrayHelper& MinMaxLineWrapper::getInfoHelper()
{
    if(!m_pPropertyArrayHelper.get())
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if(!m_pPropertyArrayHelper.get())
        {
            sal_Bool bSorted = sal_True;
            m_pPropertyArrayHelper = ::boost::shared_ptr< ::cppu::OPropertyArrayHelper >( new ::cppu::OPropertyArrayHelper( lcl_GetPropertySequence(), bSorted ) );
        }
    }
    return *m_pPropertyArrayHelper.get();
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL MinMaxLineWrapper::getPropertySetInfo()
                    throw (uno::RuntimeException)
{
    if( !m_xInfo.is() )
    {
        ::osl::MutexGuard aGuard( GetMutex() );
        if( !m_xInfo.is() )
            m_xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
    }
    return m_xInfo;
}
void SAL_CALL MinMaxLineWrapper::setPropertyValue( const ::rtl::OUString& rPropertyName, const uno::Any& rValue )
                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xPropSet(0);

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType->getChartType().equals(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        {
            Reference< chart2::XDataSeriesContainer > xSeriesContainer(xType,uno::UNO_QUERY);
            if( xSeriesContainer.is() )
            {
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                if(aSeriesSeq.getLength())
                {
                    xPropSet = Reference< beans::XPropertySet >(aSeriesSeq[0],uno::UNO_QUERY);
                    if(xPropSet.is())
                    {
                        if( rPropertyName.equals( C2U("LineColor")) )
                            xPropSet->setPropertyValue( C2U("Color"), rValue );
                        else if( rPropertyName.equals( C2U("LineTransparence")) )
                            xPropSet->setPropertyValue( C2U("Transparency"), rValue );
                        else if( rPropertyName.equals( m_aWrappedLineJointProperty.getOuterName() ) )
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
uno::Any SAL_CALL MinMaxLineWrapper::getPropertyValue( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;

    Reference< beans::XPropertySet > xPropSet(0);

    Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( xDiagram ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType->getChartType().equals(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        {
            Reference< chart2::XDataSeriesContainer > xSeriesContainer(xType,uno::UNO_QUERY);
            if( xSeriesContainer.is() )
            {
                Sequence< Reference< chart2::XDataSeries > > aSeriesSeq( xSeriesContainer->getDataSeries() );
                if(aSeriesSeq.getLength())
                {
                    xPropSet = Reference< beans::XPropertySet >(aSeriesSeq[0],uno::UNO_QUERY);
                    break;
                }
            }
        }
    }
    if(xPropSet.is())
    {
        if( rPropertyName.equals( C2U("LineColor")) )
            aRet = xPropSet->getPropertyValue( C2U("Color") );
        else if( rPropertyName.equals( C2U("LineTransparence")) )
            aRet = xPropSet->getPropertyValue( C2U("Transparency") );
        else if( rPropertyName.equals( m_aWrappedLineJointProperty.getOuterName() ) )
            aRet = m_aWrappedLineJointProperty.getPropertyValue( xPropSet );
        else
            aRet = xPropSet->getPropertyValue( rPropertyName );

    }
    return aRet;
}

void SAL_CALL MinMaxLineWrapper::addPropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removePropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::addVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removeVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//XMultiPropertySet
//getPropertySetInfo() already declared in XPropertySet
void SAL_CALL MinMaxLineWrapper::setPropertyValues( const uno::Sequence< ::rtl::OUString >& rNameSeq, const uno::Sequence< uno::Any >& rValueSeq )
                    throw (beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nMinCount = std::min( rValueSeq.getLength(), rNameSeq.getLength() );
    for(sal_Int32 nN=0; nN<nMinCount; nN++)
    {
        ::rtl::OUString aPropertyName( rNameSeq[nN] );
        try
        {
            this->setPropertyValue( aPropertyName, rValueSeq[nN] );
        }
        catch( beans::UnknownPropertyException& ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
    //todo: store unknown properties elsewhere
}
uno::Sequence< uno::Any > SAL_CALL MinMaxLineWrapper::getPropertyValues( const uno::Sequence< ::rtl::OUString >& rNameSeq )
                    throw (uno::RuntimeException)
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            ::rtl::OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = this->getPropertyValue( aPropertyName );
        }
    }
    return aRetSeq;
}
void SAL_CALL MinMaxLineWrapper::addPropertiesChangeListener(
    const uno::Sequence< ::rtl::OUString >& /* aPropertyNames */,
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::removePropertiesChangeListener(
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL MinMaxLineWrapper::firePropertiesChangeEvent(
    const uno::Sequence< ::rtl::OUString >& /* aPropertyNames */,
    const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//XPropertyState
beans::PropertyState SAL_CALL MinMaxLineWrapper::getPropertyState( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    if( rPropertyName.equals( m_aWrappedLineJointProperty.getOuterName() ) )
        return beans::PropertyState_DEFAULT_VALUE;

    uno::Any aDefault( this->getPropertyDefault( rPropertyName ) );
    uno::Any aValue( this->getPropertyValue( rPropertyName ) );

    if( aDefault == aValue )
        return beans::PropertyState_DEFAULT_VALUE;

    return beans::PropertyState_DIRECT_VALUE;
}
uno::Sequence< beans::PropertyState > SAL_CALL MinMaxLineWrapper::getPropertyStates( const uno::Sequence< ::rtl::OUString >& rNameSeq )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    Sequence< beans::PropertyState > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            ::rtl::OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = this->getPropertyState( aPropertyName );
        }
    }
    return aRetSeq;
}
void SAL_CALL MinMaxLineWrapper::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    this->setPropertyValue( rPropertyName, this->getPropertyDefault(rPropertyName) );
}
uno::Any SAL_CALL MinMaxLineWrapper::getPropertyDefault( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    static tPropertyValueMap aStaticDefaults;

    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        LineProperties::AddDefaultsToMap( aStaticDefaults );
    }

    sal_Int32 nHandle = getInfoHelper().getHandleByName( rPropertyName );

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
}

//XMultiPropertyStates
//getPropertyStates() already declared in XPropertyState
void SAL_CALL MinMaxLineWrapper::setAllPropertiesToDefault(  )
                    throw (uno::RuntimeException)
{
    const Sequence< beans::Property >&  rPropSeq = lcl_GetPropertySequence();
    for(sal_Int32 nN=0; nN<rPropSeq.getLength(); nN++)
    {
        ::rtl::OUString aPropertyName( rPropSeq[nN].Name );
        this->setPropertyToDefault( aPropertyName );
    }
}
void SAL_CALL MinMaxLineWrapper::setPropertiesToDefault( const uno::Sequence< ::rtl::OUString >& rNameSeq )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
    {
        ::rtl::OUString aPropertyName( rNameSeq[nN] );
        this->setPropertyToDefault( aPropertyName );
    }
}
uno::Sequence< uno::Any > SAL_CALL MinMaxLineWrapper::getPropertyDefaults( const uno::Sequence< ::rtl::OUString >& rNameSeq )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Sequence< Any > aRetSeq;
    if( rNameSeq.getLength() )
    {
        aRetSeq.realloc( rNameSeq.getLength() );
        for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
        {
            ::rtl::OUString aPropertyName( rNameSeq[nN] );
            aRetSeq[nN] = this->getPropertyDefault( aPropertyName );
        }
    }
    return aRetSeq;
}


// ================================================================================

Sequence< OUString > MinMaxLineWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartLine" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( MinMaxLineWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
