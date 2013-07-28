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

#include "GridProperties.hxx"
#include "LinePropertiesHelper.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.GridProperties" );

enum
{
    PROP_GRID_SHOW
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "Show",
                  PROP_GRID_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticGridDefaults_Initializer
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
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_GRID_SHOW, false );

        // override other defaults
        ::chart::PropertyHelper::setPropertyValue< sal_Int32 >(
            rOutMap, ::chart::LinePropertiesHelper::PROP_LINE_COLOR, 0xb3b3b3 );  // gray30
    }
};

struct StaticGridDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticGridDefaults_Initializer >
{
};

struct StaticGridInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticGridInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticGridInfoHelper_Initializer >
{
};

struct StaticGridInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticGridInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticGridInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticGridInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

GridProperties::GridProperties( Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

GridProperties::GridProperties() :
        ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

GridProperties::GridProperties( const GridProperties & rOther ) :
        MutexContainer(),
        impl::GridProperties_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

GridProperties::~GridProperties()
{}

// ____ OPropertySet ____
uno::Any GridProperties::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticGridDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL GridProperties::getInfoHelper()
{
    return *StaticGridInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL GridProperties::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticGridInfo::get();
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL GridProperties::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new GridProperties( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL GridProperties::addModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL GridProperties::removeModifyListener( const Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL GridProperties::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL GridProperties::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void GridProperties::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void GridProperties::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

Sequence< OUString > GridProperties::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = "com.sun.star.chart2.GridProperties";
    aServices[ 1 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( GridProperties, lcl_aServiceName );

// needed by MSC compiler
using impl::GridProperties_Base;

IMPLEMENT_FORWARD_XINTERFACE2( GridProperties, GridProperties_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( GridProperties, GridProperties_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
