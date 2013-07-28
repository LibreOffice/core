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

#include "FormattedString.hxx"
#include "ContainerHelper.hxx"

#include "CharacterProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

struct StaticFormattedStringDefaults_Initializer
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
        ::chart::CharacterProperties::AddDefaultsToMap( rOutMap );
    }
};

struct StaticFormattedStringDefaults : public rtl::StaticAggregate< ::chart::tPropertyValueMap, StaticFormattedStringDefaults_Initializer >
{
};

struct StaticFormattedStringInfoHelper_Initializer
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
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );

        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        return ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

};

struct StaticFormattedStringInfoHelper : public rtl::StaticAggregate< ::cppu::OPropertyArrayHelper, StaticFormattedStringInfoHelper_Initializer >
{
};

struct StaticFormattedStringInfo_Initializer
{
    uno::Reference< beans::XPropertySetInfo >* operator()()
    {
        static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticFormattedStringInfoHelper::get() ) );
        return &xPropertySetInfo;
    }
};

struct StaticFormattedStringInfo : public rtl::StaticAggregate< uno::Reference< beans::XPropertySetInfo >, StaticFormattedStringInfo_Initializer >
{
};

} // anonymous namespace

namespace chart
{

FormattedString::FormattedString(
        uno::Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
    m_aString(),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

FormattedString::FormattedString( const FormattedString & rOther ) :
        MutexContainer(),
        impl::FormattedString_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_aString( rOther.m_aString ),
    m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{}

FormattedString::~FormattedString()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL FormattedString::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new FormattedString( *this ));
}

// ____ XFormattedString ____
OUString SAL_CALL FormattedString::getString()
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex());
    return m_aString;
}

void SAL_CALL FormattedString::setString( const OUString& String )
    throw (uno::RuntimeException)
{
    {
        MutexGuard aGuard( GetMutex());
        m_aString = String;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();

}

// ____ XModifyBroadcaster ____
void SAL_CALL FormattedString::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL FormattedString::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL FormattedString::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL FormattedString::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void FormattedString::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void FormattedString::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

Sequence< OUString > FormattedString::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );

    aServices[ 0 ] = "com.sun.star.chart2.FormattedString";
    aServices[ 1 ] = "com.sun.star.beans.PropertySet";
    return aServices;
}

// ____ OPropertySet ____
uno::Any FormattedString::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    const tPropertyValueMap& rStaticDefaults = *StaticFormattedStringDefaults::get();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL FormattedString::getInfoHelper()
{
    return *StaticFormattedStringInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL FormattedString::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    return *StaticFormattedStringInfo::get();
}

using impl::FormattedString_Base;

IMPLEMENT_FORWARD_XINTERFACE2( FormattedString, FormattedString_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( FormattedString, FormattedString_Base, ::property::OPropertySet )

// do this in derived classes!

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( FormattedString,
                             OUString("com.sun.star.comp.chart.FormattedString") );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
