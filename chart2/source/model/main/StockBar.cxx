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

#include <algorithm>

namespace com::sun::star::beans { class XPropertySetInfo; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;

namespace
{

::cppu::OPropertyArrayHelper& StaticStockBarInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::FillProperties::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

const ::chart::tPropertyValueMap & StaticStockBarDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
        {
            ::chart::tPropertyValueMap aTmp;
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aTmp );
            ::chart::FillProperties::AddDefaultsToMap( aTmp );

            // override other defaults
            ::chart::PropertyHelper::setPropertyValue< sal_Int32 >( aTmp, ::chart::FillProperties::PROP_FILL_COLOR, 0xffffff ); // white
            return aTmp;
        }();
    return aStaticDefaults;
};

} // anonymous namespace

namespace chart
{

StockBar::StockBar( bool bRisingCourse ) :
    m_xModifyEventForwarder( new ModifyEventForwarder() )
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
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

StockBar::~StockBar()
{}

// ____ XTypeProvider ____
uno::Sequence< css::uno::Type > SAL_CALL StockBar::getTypes()
{
    return ::comphelper::concatSequences(
        impl::StockBar_Base::getTypes(),
        ::property::OPropertySet::getTypes());
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL StockBar::createClone()
{
    return uno::Reference< util::XCloneable >( new StockBar( *this ));
}

// ____ OPropertySet ____
void StockBar::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticStockBarDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL StockBar::getInfoHelper()
{
    return StaticStockBarInfoHelper();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL StockBar::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticStockBarInfoHelper() ) );
    return xPropertySetInfo;
}

// ____ XModifyBroadcaster ____
void SAL_CALL StockBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL StockBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
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
