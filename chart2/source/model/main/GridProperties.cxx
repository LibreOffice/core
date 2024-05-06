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

#include <GridProperties.hxx>
#include <LinePropertiesHelper.hxx>
#include <UserDefinedProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::beans { class XPropertySetInfo; }
namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_GRID_SHOW
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "Show",
                  PROP_GRID_SHOW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );
}

const ::chart::tPropertyValueMap & StaticGridDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []()
        {
            ::chart::tPropertyValueMap aTmp;
            ::chart::LinePropertiesHelper::AddDefaultsToMap( aTmp );

            ::chart::PropertyHelper::setPropertyValueDefault( aTmp, PROP_GRID_SHOW, false );

            // override other defaults
            ::chart::PropertyHelper::setPropertyValue< sal_Int32 >(
                aTmp, ::chart::LinePropertiesHelper::PROP_LINE_COLOR, 0xb3b3b3 );  // gray30
            return aTmp;
        }();
    return aStaticDefaults;
};

::cppu::OPropertyArrayHelper& StaticGridInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< Property > aProperties;
            lcl_AddPropertiesToVector( aProperties );
            ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
            ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{

GridProperties::GridProperties() :
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

GridProperties::GridProperties( const GridProperties & rOther ) :
        impl::GridProperties_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{
}

GridProperties::~GridProperties()
{}

// ____ OPropertySet ____
void GridProperties::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticGridDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL GridProperties::getInfoHelper()
{
    return StaticGridInfoHelper();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL GridProperties::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticGridInfoHelper() ) );
    return xPropertySetInfo;
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL GridProperties::createClone()
{
    return uno::Reference< util::XCloneable >( new GridProperties( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL GridProperties::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL GridProperties::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL GridProperties::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL GridProperties::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void GridProperties::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL GridProperties::getImplementationName()
{
    return u"com.sun.star.comp.chart2.GridProperties"_ustr;
}

sal_Bool SAL_CALL GridProperties::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL GridProperties::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart2.GridProperties"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

// needed by MSC compiler
using impl::GridProperties_Base;

IMPLEMENT_FORWARD_XINTERFACE2( GridProperties, GridProperties_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( GridProperties, GridProperties_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_GridProperties_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::GridProperties);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
