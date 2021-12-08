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
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <CharacterProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <ModifyListenerHelper.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/chart2/LegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::Property;

namespace
{

enum
{
    PROP_LEGEND_ANCHOR_POSITION,
    PROP_LEGEND_EXPANSION,
    PROP_LEGEND_SHOW,
    PROP_LEGEND_OVERLAY,
    PROP_LEGEND_REF_PAGE_SIZE,
    PROP_LEGEND_REL_POS,
    PROP_LEGEND_REL_SIZE
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "AnchorPosition",
                  PROP_LEGEND_ANCHOR_POSITION,
                  cppu::UnoType<chart2::LegendPosition>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Expansion",
                  PROP_LEGEND_EXPANSION,
                  cppu::UnoType<css::chart::ChartLegendExpansion>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Show",
                  PROP_LEGEND_SHOW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "Overlay",
                  PROP_LEGEND_OVERLAY,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ReferencePageSize",
                  PROP_LEGEND_REF_PAGE_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RelativePosition",
                  PROP_LEGEND_REL_POS,
                  cppu::UnoType<chart2::RelativePosition>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RelativeSize",
                  PROP_LEGEND_REL_SIZE,
                  cppu::UnoType<chart2::RelativeSize>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

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
    static void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );

        rOutMap.setPropertyValueDefault( PROP_LEGEND_ANCHOR_POSITION, chart2::LegendPosition_LINE_END );
        rOutMap.setPropertyValueDefault( PROP_LEGEND_EXPANSION, css::chart::ChartLegendExpansion_HIGH );
        rOutMap.setPropertyValueDefault( PROP_LEGEND_SHOW, true );
        rOutMap.setPropertyValueDefault( PROP_LEGEND_OVERLAY, false );

        float fDefaultCharHeight = 10.0;
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        rOutMap.setPropertyValue( ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
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
    static Sequence< Property > lcl_GetPropertySequence()
    {
        std::vector< css::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LinePropertiesHelper::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return comphelper::containerToSequence( aProperties );
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

Legend::Legend() :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

Legend::Legend( const Legend & rOther ) :
        impl::Legend_Base(rOther),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
}

Legend::~Legend()
{
}

// ____ XCloneable ____
Reference< util::XCloneable > SAL_CALL Legend::createClone()
{
    return Reference< util::XCloneable >( new Legend( *this ));
}

// ____ XModifyBroadcaster ____
void SAL_CALL Legend::addModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void SAL_CALL Legend::removeModifyListener( const Reference< util::XModifyListener >& aListener )
{
    try
    {
        Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

// ____ XModifyListener ____
void SAL_CALL Legend::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Legend::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void Legend::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ____ OPropertySet ____
void Legend::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticLegendDefaults::get();
    rStaticDefaults.get(nHandle, rAny);
}

::cppu::IPropertyArrayHelper & SAL_CALL Legend::getInfoHelper()
{
    return *StaticLegendInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL Legend::getPropertySetInfo()
{
    return *StaticLegendInfo::get();
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL Legend::getImplementationName()
{
    return "com.sun.star.comp.chart2.Legend";
}

sal_Bool SAL_CALL Legend::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL Legend::getSupportedServiceNames()
{
    return {
        "com.sun.star.chart2.Legend",
        "com.sun.star.beans.PropertySet",
        "com.sun.star.drawing.FillProperties",
        "com.sun.star.drawing.LineProperties",
        "com.sun.star.style.CharacterProperties",
        "com.sun.star.layout.LayoutElement"
    };
}

// needed by MSC compiler
using impl::Legend_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Legend, Legend_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Legend, Legend_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_Legend_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::Legend);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
