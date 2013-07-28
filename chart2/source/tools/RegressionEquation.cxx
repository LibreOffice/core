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
#include "LinePropertiesHelper.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "CharacterProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"

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

static const OUString lcl_aImplementationName( "com.sun.star.comp.chart2.RegressionEquation" );
static const OUString lcl_aServiceName( "com.sun.star.chart2.RegressionEquation" );

enum
{
    PROP_EQUATION_SHOW,
    PROP_EQUATION_SHOW_CORRELATION_COEFF,
    PROP_EQUATION_REF_PAGE_SIZE,
    PROP_EQUATION_REL_POS,
    PROP_EQUATION_NUMBER_FORMAT
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ShowEquation",
                  PROP_EQUATION_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ShowCorrelationCoefficient",
                  PROP_EQUATION_SHOW_CORRELATION_COEFF,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ReferencePageSize",
                  PROP_EQUATION_REF_PAGE_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "RelativePosition",
                  PROP_EQUATION_REL_POS,
                  ::getCppuType( reinterpret_cast< const chart2::RelativePosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( "NumberFormat",
                  PROP_EQUATION_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
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
    void lcl_AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap )
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap( rOutMap );
        ::chart::FillProperties::AddDefaultsToMap( rOutMap );
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SHOW, false );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SHOW_CORRELATION_COEFF, false );
        //::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_EQUATION_SEPARATOR, OUString( sal_Unicode( '\n' )));

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
    uno::Sequence< Property > lcl_GetPropertySequence()
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

RegressionEquation::RegressionEquation( const Reference< uno::XComponentContext > & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
        m_xContext( xContext )
{}

RegressionEquation::RegressionEquation( const RegressionEquation & rOther ) :
        MutexContainer(),
        impl::RegressionEquation_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder()),
    m_xContext( NULL )
{}

RegressionEquation::~RegressionEquation()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL RegressionEquation::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new RegressionEquation( *this ));
}

// ____ OPropertySet ____
uno::Any RegressionEquation::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
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
    throw (uno::RuntimeException)
{
    return *StaticRegressionEquationInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL RegressionEquation::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL RegressionEquation::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL RegressionEquation::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL RegressionEquation::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    return m_aStrings;
}

void SAL_CALL RegressionEquation::setText( const uno::Sequence< uno::Reference< chart2::XFormattedString > >& Strings )
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    m_aStrings = Strings;
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    fireModifyEvent();
}

uno::Sequence< OUString > RegressionEquation::getSupportedServiceNames_Static()
{
    const sal_Int32 nNumServices( 5 );
    sal_Int32 nI = 0;
    uno::Sequence< OUString > aServices( nNumServices );
    aServices[ nI++ ] = lcl_aServiceName;
    aServices[ nI++ ] = "com.sun.star.beans.PropertySet";
    aServices[ nI++ ] = "com.sun.star.drawing.FillProperties";
    aServices[ nI++ ] = "com.sun.star.drawing.LineProperties";
    aServices[ nI++ ] = "com.sun.star.style.CharacterProperties";
    OSL_ASSERT( nNumServices == nI );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( RegressionEquation, lcl_aImplementationName );

using impl::RegressionEquation_Base;

IMPLEMENT_FORWARD_XINTERFACE2( RegressionEquation, RegressionEquation_Base, ::property::OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
