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
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <tools/diagnose_ex.h>

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace
{

struct StaticWallDefaults_Initializer
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
        rOutMap.setPropertyValue( ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );
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
        impl::Wall_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

Wall::~Wall()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL Wall::createClone()
{
    return uno::Reference< util::XCloneable >( new Wall( *this ));
}

// ____ OPropertySet ____
void Wall::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticWallDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL Wall::getInfoHelper()
{
    return *StaticWallInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL Wall::getPropertySetInfo()
{
    return *StaticWallInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL Wall::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL Wall::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL Wall::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Wall::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void Wall::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

using impl::Wall_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Wall, Wall_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
