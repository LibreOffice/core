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
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include <vector>
#include <algorithm>

namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace
{

struct StaticPageBackgroundDefaults_Initializer
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
    static uno::Sequence< Property > lcl_GetPropertySequence()
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

PageBackground::PageBackground() :
    ::property::OPropertySet( m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

PageBackground::PageBackground( const PageBackground & rOther ) :
        MutexContainer(),
        impl::PageBackground_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

PageBackground::~PageBackground()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL PageBackground::createClone()
{
    return uno::Reference< util::XCloneable >( new PageBackground( *this ));
}

// ____ OPropertySet ____
uno::Any PageBackground::GetDefaultValue( sal_Int32 nHandle ) const
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
{
    return *StaticPageBackgroundInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL PageBackground::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL PageBackground::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModifyListener ____
void SAL_CALL PageBackground::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL PageBackground::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void PageBackground::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

OUString SAL_CALL PageBackground::getImplementationName()
{
    return OUString("com.sun.star.comp.chart2.PageBackground");
}

sal_Bool SAL_CALL PageBackground::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PageBackground::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.PageBackground",
        "com.sun.star.beans.PropertySet" };
}

using impl::PageBackground_Base;

IMPLEMENT_FORWARD_XINTERFACE2( PageBackground, PageBackground_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_PageBackground_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::PageBackground );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
