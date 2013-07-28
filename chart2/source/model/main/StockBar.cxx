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

#include "StockBar.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include "ModifyListenerHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.StockBar" );

struct StaticStockBarInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    uno::Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticStockBarInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticStockBarInfoHelper_Initializer >
{
};

struct StaticStockBarInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticStockBarInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticStockBarInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticStockBarInfo_Initializer >
{
};

struct StaticStockBarDefaults_Initializer
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
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );

        // override other defaults
        ::chart::PropertyHelper::setPropertyValue< sal_Int32 >( rOutMap, ::chart::FillProperties::PROP_FILL_COLOR, 0xffffff ); // white
    }
};

struct StaticStockBarDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticStockBarDefaults_Initializer >
{
};

} // anonymous namespace

namespace chart
{

StockBar::StockBar( bool bRisingCourse ) :
        ::property::OPropertySet( m_aMutex ),
    m_bRisingCourse( bRisingCourse ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! m_bRisingCourse )
    {
        setFastPropertyValue_NoBroadcast(
            ::chart::FillProperties::PROP_FILL_COLOR,
            uno::makeAny( sal_Int32( 0x000000 ))); // black
        setFastPropertyValue_NoBroadcast(
            ::chart::LinePropertiesHelper::PROP_LINE_COLOR,
            uno::makeAny( sal_Int32( 0xb3b3b3 ))); // gray30
    }
}

StockBar::StockBar( const StockBar & rOther ) :
        MutexContainer(),
        impl::StockBar_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_bRisingCourse( rOther.m_bRisingCourse ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

StockBar::~StockBar()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL StockBar::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new StockBar( *this ));
}

// ____ OPropertySet ____
uno::Any StockBar::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticStockBarDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL StockBar::getInfoHelper()
{
    return *StaticStockBarInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL StockBar::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticStockBarInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL StockBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void SAL_CALL StockBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
    throw (uno::RuntimeException)
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL StockBar::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StockBar::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void StockBar::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void StockBar::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

uno::Sequence< OUString > StockBar::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = "com.sun.star.chart2.StockBar";
    aServices[ 1 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( StockBar, lcl_aServiceName );

using impl::StockBar_Base;

IMPLEMENT_FORWARD_XINTERFACE2( StockBar, StockBar_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
