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

#include "UpDownBarWrapper.hxx"
#include "macros.hxx"
#include "Chart2ModelContact.hxx"
#include "DiagramHelper.hxx"
#include "servicenames_charttypes.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/chart2/XChartType.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>

#include "LineProperties.hxx"
#include "FillProperties.hxx"
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
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.ChartArea" ));

struct StaticUpDownBarWrapperPropertyArray_Initializer
{
    Sequence< Property >* operator()()
    {
        static Sequence< Property > aPropSeq( lcl_GetPropertySequence() );
        return &aPropSeq;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;

        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
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
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LineProperties::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );
    }
};

struct StaticUpDownBarWrapperDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticUpDownBarWrapperDefaults_Initializer >
{
};

} // anonymous namespace

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

UpDownBarWrapper::UpDownBarWrapper(
    bool bUp, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : m_spChart2ModelContact( spChart2ModelContact )
        , m_aEventListenerContainer( m_aMutex )
        , m_aPropertySetName( bUp ? C2U("WhiteDay") : C2U("BlackDay") )
{
}

UpDownBarWrapper::~UpDownBarWrapper()
{
}

// ____ XComponent ____
void SAL_CALL UpDownBarWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );
}

void SAL_CALL UpDownBarWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL UpDownBarWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

::cppu::IPropertyArrayHelper& UpDownBarWrapper::getInfoHelper()
{
    return *StaticUpDownBarWrapperInfoHelper::get();
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL UpDownBarWrapper::getPropertySetInfo()
                    throw (uno::RuntimeException)
{
    return *StaticUpDownBarWrapperInfo::get();
}
void SAL_CALL UpDownBarWrapper::setPropertyValue( const ::rtl::OUString& rPropertyName, const uno::Any& rValue )
                    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    Reference< beans::XPropertySet > xPropSet(0);

    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType->getChartType().equals(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        {
            Reference< beans::XPropertySet > xTypeProps( aTypes[nN], uno::UNO_QUERY );
            if(xTypeProps.is())
            {
                xTypeProps->getPropertyValue( m_aPropertySetName ) >>= xPropSet;
            }
        }
    }
    if(xPropSet.is())
        xPropSet->setPropertyValue( rPropertyName, rValue );
}
uno::Any SAL_CALL UpDownBarWrapper::getPropertyValue( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    Any aRet;

    Reference< beans::XPropertySet > xPropSet(0);

    Sequence< Reference< chart2::XChartType > > aTypes(
            ::chart::DiagramHelper::getChartTypesFromDiagram( m_spChart2ModelContact->getChart2Diagram() ) );
    for( sal_Int32 nN = 0; nN < aTypes.getLength(); nN++ )
    {
        Reference< chart2::XChartType > xType( aTypes[nN] );
        if( xType->getChartType().equals(CHART2_SERVICE_NAME_CHARTTYPE_CANDLESTICK) )
        {
            Reference< beans::XPropertySet > xTypeProps( aTypes[nN], uno::UNO_QUERY );
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

void SAL_CALL UpDownBarWrapper::addPropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removePropertyChangeListener( const ::rtl::OUString& /*aPropertyName*/, const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::addVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removeVetoableChangeListener( const ::rtl::OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//XMultiPropertySet
//getPropertySetInfo() already declared in XPropertySet
void SAL_CALL UpDownBarWrapper::setPropertyValues( const uno::Sequence< ::rtl::OUString >& rNameSeq, const uno::Sequence< uno::Any >& rValueSeq )
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
uno::Sequence< uno::Any > SAL_CALL UpDownBarWrapper::getPropertyValues( const uno::Sequence< ::rtl::OUString >& rNameSeq )
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
void SAL_CALL UpDownBarWrapper::addPropertiesChangeListener( const uno::Sequence< ::rtl::OUString >& /* aPropertyNames */, const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL UpDownBarWrapper::firePropertiesChangeEvent( const uno::Sequence< ::rtl::OUString >& /* aPropertyNames */, const uno::Reference< beans::XPropertiesChangeListener >& /* xListener */ )
                    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//XPropertyState
beans::PropertyState SAL_CALL UpDownBarWrapper::getPropertyState( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    uno::Any aDefault( this->getPropertyDefault( rPropertyName ) );
    uno::Any aValue( this->getPropertyValue( rPropertyName ) );

    if( aDefault == aValue )
        return beans::PropertyState_DEFAULT_VALUE;

    return beans::PropertyState_DIRECT_VALUE;
}
uno::Sequence< beans::PropertyState > SAL_CALL UpDownBarWrapper::getPropertyStates( const uno::Sequence< ::rtl::OUString >& rNameSeq )
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
void SAL_CALL UpDownBarWrapper::setPropertyToDefault( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    this->setPropertyValue( rPropertyName, this->getPropertyDefault(rPropertyName) );
}

uno::Any SAL_CALL UpDownBarWrapper::getPropertyDefault( const ::rtl::OUString& rPropertyName )
                    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticUpDownBarWrapperDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( getInfoHelper().getHandleByName( rPropertyName ) ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

//XMultiPropertyStates
//getPropertyStates() already declared in XPropertyState
void SAL_CALL UpDownBarWrapper::setAllPropertiesToDefault(  )
                    throw (uno::RuntimeException)
{
    const Sequence< beans::Property >& rPropSeq = *StaticUpDownBarWrapperPropertyArray::get();
    for(sal_Int32 nN=0; nN<rPropSeq.getLength(); nN++)
    {
        ::rtl::OUString aPropertyName( rPropSeq[nN].Name );
        this->setPropertyToDefault( aPropertyName );
    }
}
void SAL_CALL UpDownBarWrapper::setPropertiesToDefault( const uno::Sequence< ::rtl::OUString >& rNameSeq )
                    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    for(sal_Int32 nN=0; nN<rNameSeq.getLength(); nN++)
    {
        ::rtl::OUString aPropertyName( rNameSeq[nN] );
        this->setPropertyToDefault( aPropertyName );
    }
}
uno::Sequence< uno::Any > SAL_CALL UpDownBarWrapper::getPropertyDefaults( const uno::Sequence< ::rtl::OUString >& rNameSeq )
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

Sequence< OUString > UpDownBarWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.ChartArea"));
    aServices[ 1 ] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.LineProperties"));
    aServices[ 2 ] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.FillProperties"));
    aServices[ 3 ] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.UserDefinedAttributeSupplier"));

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( UpDownBarWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
