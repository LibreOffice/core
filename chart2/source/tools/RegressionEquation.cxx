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

#include "RegressionEquation.hxx"
#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <UserDefinedProperties.hxx>
#include <CharacterProperties.hxx>
#include <PropertyHelper.hxx>
#include <ContainerHelper.hxx>
#include <unonames.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_EQUATION_SHOW,
    PROP_EQUATION_XNAME,
    PROP_EQUATION_YNAME,
    PROP_EQUATION_SHOW_CORRELATION_COEFF,
    PROP_EQUATION_REF_PAGE_SIZE,
    PROP_EQUATION_REL_POS,
    PROP_EQUATION_NUMBER_FORMAT
};

void lcl_AddPropertiesToVector(
    std::vector< Property > & rOutProperties )
{
    rOutProperties.emplace_back( "ShowEquation",
                  PROP_EQUATION_SHOW,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "XName",
                  PROP_EQUATION_XNAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "YName",
                  PROP_EQUATION_YNAME,
                  cppu::UnoType<OUString>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ShowCorrelationCoefficient",
                  PROP_EQUATION_SHOW_CORRELATION_COEFF,
                  cppu::UnoType<bool>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT );

    rOutProperties.emplace_back( "ReferencePageSize",
                  PROP_EQUATION_REF_PAGE_SIZE,
                  cppu::UnoType<awt::Size>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( "RelativePosition",
                  PROP_EQUATION_REL_POS,
                  cppu::UnoType<chart2::RelativePosition>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );

    rOutProperties.emplace_back( CHART_UNONAME_NUMFMT,
                  PROP_EQUATION_NUMBER_FORMAT,
                  cppu::UnoType<sal_Int32>::get(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID );
}

struct StaticRegressionEquationDefaults_Initializer
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

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SHOW, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_XNAME, OUString("x") );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_YNAME, OUString("f(x)") );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SHOW_CORRELATION_COEFF, false );
        //::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SEPARATOR, OUString( '\n' ));

        // override other defaults
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::FillProperties::PROP_FILL_STYLE, drawing::FillStyle_NONE );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::LinePropertiesHelper::PROP_LINE_STYLE, drawing::LineStyle_NONE );

        float fDefaultCharHeight = 10.0;
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
        ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
    }
};

struct StaticRegressionEquationDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticRegressionEquationDefaults_Initializer >
{
};

struct StaticRegressionEquationInfoHelper_Initializer
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

struct StaticRegressionEquationInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticRegressionEquationInfoHelper_Initializer >
{
};

struct StaticRegressionEquationInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticRegressionEquationInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticRegressionEquationInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticRegressionEquationInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

RegressionEquation::RegressionEquation() :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{}

RegressionEquation::RegressionEquation( const RegressionEquation & rOther ) :
        MutexContainer(),
        impl::RegressionEquation_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{}

RegressionEquation::~RegressionEquation()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL RegressionEquation::createClone()
{
    return uno::Reference< util::XCloneable >( new RegressionEquation( *this ));
}

// ____ OPropertySet ____
uno::Any RegressionEquation::GetDefaultValue( sal_Int32 nHandle ) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticRegressionEquationDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL RegressionEquation::getInfoHelper()
{
    return *StaticRegressionEquationInfoHelper::get();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL RegressionEquation::getPropertySetInfo()
{
    return *StaticRegressionEquationInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL RegressionEquation::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        SAL_WARN("chart2", "Exception caught. " << ex );
    }
}

void SAL_CALL RegressionEquation::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    try
    {
        uno::Reference< util::XModifyBroadcaster > xBroadcaster( m_xModifyEventForwarder, uno::UNO_QUERY_THROW );
        xBroadcaster->removeModifyListener( aListener );
    }
    catch( const uno::Exception & ex )
    {
        SAL_WARN("chart2", "Exception caught. " << ex );
    }
}

// ____ XModifyListener ____
void SAL_CALL RegressionEquation::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL RegressionEquation::disposing( const lang::EventObject& /* Source */ )
{
    // nothing
}

// ____ OPropertySet ____
void RegressionEquation::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void RegressionEquation::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ____ XTitle ____
uno::Sequence< uno::Reference< chart2::XFormattedString > > SAL_CALL RegressionEquation::getText()
{
    MutexGuard aGuard( GetMutex() );
    return m_aStrings;
}

void SAL_CALL RegressionEquation::setText( const uno::Sequence< uno::Reference< chart2::XFormattedString > >& Strings )
{
    MutexGuard aGuard( GetMutex() );
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    m_aStrings = Strings;
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    fireModifyEvent();
}

OUString SAL_CALL RegressionEquation::getImplementationName()
{
    return OUString("com.sun.star.comp.chart2.RegressionEquation");
}

sal_Bool SAL_CALL RegressionEquation::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL RegressionEquation::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.RegressionEquation",
             "com.sun.star.beans.PropertySet",
             "com.sun.star.drawing.FillProperties",
             "com.sun.star.drawing.LineProperties",
             "com.sun.star.style.CharacterProperties" };
}

using impl::RegressionEquation_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RegressionEquation, RegressionEquation_Base, ::property::OPropertySet )

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_RegressionEquation_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::RegressionEquation);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
