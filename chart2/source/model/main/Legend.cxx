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

#include "Legend.hxx"
#include "macros.hxx"
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "LegendHelper.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"
#include "PropertyHelper.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::Property;

namespace
{

static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.Legend" );

enum
{
    PROP_LEGEND_ANCHOR_POSITION,
    PROP_LEGEND_EXPANSION,
    PROP_LEGEND_SHOW,
    PROP_LEGEND_REF_PAGE_SIZE,
    PROP_LEGEND_REL_POS,
    PROP_LEGEND_REL_SIZE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "AnchorPosition",
                  PROP_LEGEND_ANCHOR_POSITION,
                  ::getCppuType( reinterpret_cast< const chart2::LegendPosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "Expansion",
                  PROP_LEGEND_EXPANSION,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartLegendExpansion * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "Show",
                  PROP_LEGEND_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ReferencePageSize",
                  PROP_LEGEND_REF_PAGE_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "RelativePosition",
                  PROP_LEGEND_REL_POS,
                  ::getCppuType( reinterpret_cast< const chart2::RelativePosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "RelativeSize",
                  PROP_LEGEND_REL_SIZE,
                  ::getCppuType( reinterpret_cast< const chart2::RelativeSize * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

}

struct StaticLegendDefaults_Initializer
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
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_ANCHOR_POSITION, chart2::LegendPosition_LINE_END );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_EXPANSION, ::com::sun::star::chart::ChartLegendExpansion_HIGH );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_SHOW, true );

        float fDefaultCharHeight = 10.0;
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
    }
};

struct StaticLegendDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticLegendDefaults_Initializer >
{
};

struct StaticLegendInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper( lcl_GetPropertySequence() );
        return &aPropHelper;
    }

private:
    Sequence< Property > lcl_GetPropertySequence()
    {
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }
};

struct StaticLegendInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticLegendInfoHelper_Initializer >
{
};

struct StaticLegendInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticLegendInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticLegendInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticLegendInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

Legend::Legend( Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

Legend::Legend( const Legend & rOther ) :
        MutexContainer(),
        impl::Legend_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

Legend::~Legend()
{
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Legend::createClone()
    throw (uno::RuntimeException)
{
    return Reference< util::XCloneable >( new Legend( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL Legend::addModifyListener( const Reference< util::XModifyListener >& aListener )
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

void SAL_CALL Legend::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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
void SAL_CALL Legend::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Legend::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void Legend::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Legend::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

Sequence< OUString > Legend::getSupportedServiceNames_Static()
{
    const sal_Int32 nNumServices( 6 );
    sal_Int32 nI = 0;
    Sequence< OUString > aServices( nNumServices );
    aServices[ nI++ ] = "com.sun.star.chart2.Legend";
    aServices[ nI++ ] = "com.sun.star.beans.PropertySet";
    aServices[ nI++ ] = "com.sun.star.drawing.FillProperties";
    aServices[ nI++ ] = "com.sun.star.drawing.LineProperties";
    aServices[ nI++ ] = "com.sun.star.style.CharacterProperties";
    aServices[ nI++ ] = "com.sun.star.layout.LayoutElement";
    OSL_ASSERT( nNumServices == nI );
    return aServices;
}

// ____ OPropertySet ____
Any Legend::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticLegendDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL Legend::getInfoHelper()
{
    return *StaticLegendInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL Legend::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticLegendInfo::get();
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Legend, lcl_aServiceName );

// needed by MSC compiler
using impl::Legend_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Legend, Legend_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Legend, Legend_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
