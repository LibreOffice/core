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

#include "Wall.hxx"
#include "macros.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "ContainerHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.Wall" );

struct StaticWallDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );
    }
};

struct StaticWallDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticWallDefaults_Initializer >
{
};

struct StaticWallInfoHelper_Initializer
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

struct StaticWallInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticWallInfoHelper_Initializer >
{
};

struct StaticWallInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticWallInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticWallInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticWallInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

Wall::Wall() :
        ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

Wall::Wall( const Wall & rOther ) :
        MutexContainer(),
        impl::Wall_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

Wall::~Wall()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL Wall::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new Wall( *this ));
}

// ____ OPropertySet ____
uno::Any Wall::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticWallDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL Wall::getInfoHelper()
{
    return *StaticWallInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL Wall::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticWallInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL Wall::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL Wall::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL Wall::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Wall::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void Wall::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Wall::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

uno::Sequence< OUString > Wall::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = "com.sun.star.chart2.Wall";
    aServices[ 1 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Wall, lcl_aServiceName );

using impl::Wall_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Wall, Wall_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
