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

#include "PageBackground.hxx"
#include "macros.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "ContainerHelper.hxx"
#include "PropertyHelper.hxx"

#include <com/sun/star/drawing/LineStyle.hpp>
#include <rtl/uuid.h>
#include <cppuhelper/queryinterface.hxx>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.PageBackground" );

struct StaticPageBackgroundDefaults_Initializer
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
        ::chart::PropertyHelper::setPropertyValue< sal_Int32 >( rOutMap, ::chart::FillProperties::PROP_FILL_COLOR, 0xffffff );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );
    }
};

struct StaticPageBackgroundDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticPageBackgroundDefaults_Initializer >
{
};

struct StaticPageBackgroundInfoHelper_Initializer
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

struct StaticPageBackgroundInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticPageBackgroundInfoHelper_Initializer >
{
};

struct StaticPageBackgroundInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticPageBackgroundInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticPageBackgroundInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticPageBackgroundInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

PageBackground::PageBackground( const uno::Reference< uno::XComponentContext > & xContext ) :
        ::property::OPropertySet( m_aMutex ),
    m_xContext( xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

PageBackground::PageBackground( const PageBackground & rOther ) :
        MutexContainer(),
        impl::PageBackground_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

PageBackground::~PageBackground()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PageBackground::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new PageBackground( *this ));
}

// ____ OPropertySet ____
uno::Any PageBackground::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticPageBackgroundDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL PageBackground::getInfoHelper()
{
    return *StaticPageBackgroundInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL PageBackground::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticPageBackgroundInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL PageBackground::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL PageBackground::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL PageBackground::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL PageBackground::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void PageBackground::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void PageBackground::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

uno::Sequence< OUString > PageBackground::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aServices( 2 );
    aServices[ 0 ] = "com.sun.star.chart2.PageBackground";
    aServices[ 1 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PageBackground, lcl_aServiceName );

using impl::PageBackground_Base;

IMPLEMENT_FORWARD_XINTERFACE2( PageBackground, PageBackground_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
