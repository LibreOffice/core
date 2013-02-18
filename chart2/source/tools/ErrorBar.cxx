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

#include "ErrorBar.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "ContainerHelper.hxx"
#include "EventListenerHelper.hxx"
#include "PropertyHelper.hxx"
#include "CloneHelper.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{
static const OUString lcl_aServiceName( "com.sun.star.comp.chart2.ErrorBar" );

enum
{
    PROP_ERROR_BAR_STYLE,
    PROP_ERROR_BAR_POS_ERROR,
    PROP_ERROR_BAR_NEG_ERROR,
    PROP_ERROR_BAR_WEIGHT,
    PROP_ERROR_BAR_SHOW_POS_ERROR,
    PROP_ERROR_BAR_SHOW_NEG_ERROR
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ErrorBarStyle",
                  PROP_ERROR_BAR_STYLE,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "PositiveError",
                  PROP_ERROR_BAR_POS_ERROR,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "NegativeError",
                  PROP_ERROR_BAR_NEG_ERROR,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "Weight",
                  PROP_ERROR_BAR_WEIGHT,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( "ShowPositiveError",
                  PROP_ERROR_BAR_SHOW_POS_ERROR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( "ShowNegativeError",
                  PROP_ERROR_BAR_SHOW_NEG_ERROR,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

struct StaticErrorBarDefaults_Initializer
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
        ::chart::LineProperties::AddDefaultsToMap( rOutMap );

        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_ERROR_BAR_STYLE, ::com::sun::star::chart::ErrorBarStyle::NONE );
        ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_ERROR_BAR_POS_ERROR, 0.0 );
        ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_ERROR_BAR_NEG_ERROR, 0.0 );
        ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_ERROR_BAR_WEIGHT, 1.0 );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_ERROR_BAR_SHOW_POS_ERROR, true );
        ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_ERROR_BAR_SHOW_NEG_ERROR, true );
    }
};

struct StaticErrorBarDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticErrorBarDefaults_Initializer >
{
};

struct StaticErrorBarInfoHelper_Initializer
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
        ::chart::LineProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticErrorBarInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticErrorBarInfoHelper_Initializer >
{
};

struct StaticErrorBarInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticErrorBarInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticErrorBarInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticErrorBarInfo_Initializer >
{
};

bool lcl_isInternalData( const uno::Reference< chart2::data::XLabeledDataSequence > & xLSeq )
{
    uno::Reference< lang::XServiceInfo > xServiceInfo( xLSeq, uno::UNO_QUERY );
    return ( xServiceInfo.is() && xServiceInfo->getImplementationName() == "com.sun.star.comp.chart2.LabeledDataSequence" );
}

} // anonymous namespace

namespace chart
{

uno::Reference< beans::XPropertySet > createErrorBar( const uno::Reference< uno::XComponentContext > & xContext )
{
    return new ErrorBar( xContext );
}

ErrorBar::ErrorBar(
    uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
    m_xContext( xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

ErrorBar::ErrorBar( const ErrorBar & rOther ) :
        MutexContainer(),
        impl::ErrorBar_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_xContext( rOther.m_xContext ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    if( ! rOther.m_aDataSequences.empty())
    {
        if( lcl_isInternalData( rOther.m_aDataSequences.front()))
            CloneHelper::CloneRefVector< tDataSequenceContainer::value_type >(
                rOther.m_aDataSequences, m_aDataSequences );
        else
            m_aDataSequences = rOther.m_aDataSequences;
        ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
    }
}

ErrorBar::~ErrorBar()
{}

uno::Reference< util::XCloneable > SAL_CALL ErrorBar::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new ErrorBar( *this ));
}

// ================================================================================

// ____ OPropertySet ____
uno::Any ErrorBar::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticErrorBarDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

::cppu::IPropertyArrayHelper & SAL_CALL ErrorBar::getInfoHelper()
{
    return *StaticErrorBarInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL ErrorBar::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticErrorBarInfo::get();
}

// ____ XModifyBroadcaster ____
void SAL_CALL ErrorBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL ErrorBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL ErrorBar::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL ErrorBar::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ XDataSink ____
void SAL_CALL ErrorBar::setData( const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > >& aData )
    throw (uno::RuntimeException)
{
    ModifyListenerHelper::removeListenerFromAllElements( m_aDataSequences, m_xModifyEventForwarder );
    EventListenerHelper::removeListenerFromAllElements( m_aDataSequences, this );
    m_aDataSequences = ContainerHelper::SequenceToVector( aData );
    EventListenerHelper::addListenerToAllElements( m_aDataSequences, this );
    ModifyListenerHelper::addListenerToAllElements( m_aDataSequences, m_xModifyEventForwarder );
}

// ____ XDataSource ____
uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > SAL_CALL ErrorBar::getDataSequences()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aDataSequences );
}

// ____ XChild ____
uno::Reference< uno::XInterface > SAL_CALL ErrorBar::getParent()
    throw (uno::RuntimeException)
{
    return m_xParent;
}

void SAL_CALL ErrorBar::setParent(
    const uno::Reference< uno::XInterface >& Parent )
    throw (lang::NoSupportException,
           uno::RuntimeException)
{
    m_xParent.set( Parent );
}

// ____ OPropertySet ____
void ErrorBar::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void ErrorBar::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ================================================================================

uno::Sequence< ::rtl::OUString > ErrorBar::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = "com.sun.star.chart2.ErrorBar";
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ErrorBar, lcl_aServiceName );

// needed by MSC compiler
using impl::ErrorBar_Base;

IMPLEMENT_FORWARD_XINTERFACE2( ErrorBar, ErrorBar_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( ErrorBar, ErrorBar_Base, OPropertySet )

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
