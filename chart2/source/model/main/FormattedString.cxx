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

#include <FormattedString.hxx>

#include <CharacterProperties.hxx>
#include <PropertyHelper.hxx>
#include <ModifyListenerHelper.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::osl::MutexGuard;

namespace chart
{
const ::chart::tPropertyValueMap & StaticFormattedStringDefaults()
{
    static ::chart::tPropertyValueMap aStaticDefaults = []
        {
            ::chart::tPropertyValueMap aMap;
            ::chart::CharacterProperties::AddDefaultsToMap( aMap );
            return aMap;
        }();
    return aStaticDefaults;
};
} // namespace chart

namespace
{

::cppu::OPropertyArrayHelper& StaticFormattedStringInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aPropHelper = []()
        {
            std::vector< css::beans::Property > aProperties;
            ::chart::CharacterProperties::AddPropertiesToVector( aProperties );

            std::sort( aProperties.begin(), aProperties.end(),
                         ::chart::PropertyNameLess() );

            return comphelper::containerToSequence( aProperties );
        }();
    return aPropHelper;
};

} // anonymous namespace

namespace chart
{

FormattedString::FormattedString() :
    m_aType(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT),
    m_bDataLabelsRange(false),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

FormattedString::FormattedString( const FormattedString & rOther ) :
        impl::FormattedString_Base(rOther),
        ::property::OPropertySet( rOther ),
    m_aString( rOther.m_aString ),
    m_aType(rOther.m_aType),
    m_aGuid(rOther.m_aGuid),
    m_bDataLabelsRange(rOther.m_bDataLabelsRange),
    m_xModifyEventForwarder( new ModifyEventForwarder() )
{}

FormattedString::~FormattedString()
{}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL FormattedString::createClone()
{
    return uno::Reference< util::XCloneable >( new FormattedString( *this ));
}

// ____ XFormattedString ____
OUString SAL_CALL FormattedString::getString()
{
    MutexGuard aGuard( m_aMutex);
    return m_aString;
}

void SAL_CALL FormattedString::setString( const OUString& String )
{
    {
        MutexGuard aGuard( m_aMutex);
        m_aString = String;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();

}

// ____ XDataPointCustomLabelField ____
css::chart2::DataPointCustomLabelFieldType SAL_CALL FormattedString::getFieldType()
{
    MutexGuard aGuard(m_aMutex);
    return m_aType;
}

void SAL_CALL
FormattedString::setFieldType(const css::chart2::DataPointCustomLabelFieldType Type)
{
    {
        MutexGuard aGuard(m_aMutex);
        m_aType = Type;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();
}

OUString SAL_CALL FormattedString::getGuid()
{
    MutexGuard aGuard( m_aMutex);
    return m_aGuid;
}

void SAL_CALL FormattedString::setGuid( const OUString& guid )
{
    {
        MutexGuard aGuard( m_aMutex);
        m_aGuid= guid;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();

}

sal_Bool SAL_CALL FormattedString::getDataLabelsRange()
{
    MutexGuard aGuard( m_aMutex);
    return m_bDataLabelsRange;
}

void SAL_CALL FormattedString::setDataLabelsRange( sal_Bool dataLabelsRange )
{
    {
        MutexGuard aGuard( m_aMutex);
        m_bDataLabelsRange = dataLabelsRange;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();
}

OUString SAL_CALL FormattedString::getCellRange()
{
    MutexGuard aGuard( m_aMutex);
    return m_aCellRange;
}

void SAL_CALL FormattedString::setCellRange( const OUString& cellRange )
{
    {
        MutexGuard aGuard( m_aMutex);
        m_aCellRange = cellRange;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();
}

// ____ XModifyBroadcaster ____
void SAL_CALL FormattedString::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->addModifyListener( aListener );
}

void SAL_CALL FormattedString::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    m_xModifyEventForwarder->removeModifyListener( aListener );
}

// ____ XModifyListener ____
void SAL_CALL FormattedString::modified( const lang::EventObject& aEvent )
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL FormattedString::disposing( const lang::EventObject& /* Source */ )
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

// ____ OPropertySet ____
void FormattedString::GetDefaultValue( sal_Int32 nHandle, uno::Any& rAny ) const
{
    const tPropertyValueMap& rStaticDefaults = StaticFormattedStringDefaults();
    tPropertyValueMap::const_iterator aFound( rStaticDefaults.find( nHandle ) );
    if( aFound == rStaticDefaults.end() )
        rAny.clear();
    else
        rAny = (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL FormattedString::getInfoHelper()
{
    return StaticFormattedStringInfoHelper();
}

// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL FormattedString::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xPropertySetInfo(
        ::cppu::OPropertySetHelper::createPropertySetInfo(StaticFormattedStringInfoHelper() ) );
    return xPropertySetInfo;
}

using impl::FormattedString_Base;

IMPLEMENT_FORWARD_XINTERFACE2( FormattedString, FormattedString_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( FormattedString, FormattedString_Base, ::property::OPropertySet )

// do this in derived classes!

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL FormattedString::getImplementationName()
{
    return u"com.sun.star.comp.chart.FormattedString"_ustr;
}

sal_Bool SAL_CALL FormattedString::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FormattedString::getSupportedServiceNames()
{
    return {
        u"com.sun.star.chart2.DataPointCustomLabelField"_ustr,
        u"com.sun.star.chart2.FormattedString"_ustr,
        u"com.sun.star.beans.PropertySet"_ustr };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart_FormattedString_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new ::chart::FormattedString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
