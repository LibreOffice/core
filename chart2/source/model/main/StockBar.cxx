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

#include <StockBar.hxx>
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>

namespace com::sun::star::beans { class XPropertySetInfo; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace
{

struct StaticStockBarInfoHelper_Initializer
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
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );

        // override other defaults
        rOutMap.setPropertyValue< sal_Int32 >( ::chart::FillProperties::PROP_FILL_COLOR, 0xffffff ); // white
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
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! bRisingCourse )
    {
        setFastPropertyValue_NoBroadcast(
            ::chart::FillProperties::PROP_FILL_COLOR,
            uno::Any( sal_Int32( 0x000000 ))); // black
        setFastPropertyValue_NoBroadcast(
            ::chart::LinePropertiesHelper::PROP_LINE_COLOR,
            uno::Any( sal_Int32( 0xb3b3b3 ))); // gray30
    }
}

StockBar::StockBar( const StockBar & rOther ) :
        impl::StockBar_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

StockBar::~StockBar()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL StockBar::createClone()
{
    return uno::Reference< util::XCloneable >( new StockBar( *this ));
}

// ____ OPropertySet ____
void StockBar::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticStockBarDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL StockBar::getInfoHelper()
{
    return *StaticStockBarInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL StockBar::getPropertySetInfo()
{
    return *StaticStockBarInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL StockBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL StockBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL StockBar::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StockBar::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void StockBar::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

using impl::StockBar_Base;

IMPLEMENT_FORWARD_XINTERFACE2( StockBar, StockBar_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
