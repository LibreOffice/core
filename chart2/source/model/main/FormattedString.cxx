/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FormattedString.cxx,v $
 * $Revision: 1.9.44.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "FormattedString.hxx"
#include "ContainerHelper.hxx"

#include "CharacterProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

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
::rtl::OUString SAL_CALL FormattedString::getString()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    return m_aString;
    // \--
}

void SAL_CALL FormattedString::setString( const ::rtl::OUString& String )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    m_aString = String;
    fireModifyEvent();
    // \--
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


// ----------------------------------------

Sequence< OUString > FormattedString::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 2 );

    aServices[ 0 ] = C2U( "com.sun.star.chart2.FormattedString" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any FormattedString::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        CharacterProperties::AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        throw beans::UnknownPropertyException();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL FormattedString::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    FormattedString::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

using impl::FormattedString_Base;

IMPLEMENT_FORWARD_XINTERFACE2( FormattedString, FormattedString_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( FormattedString, FormattedString_Base, ::property::OPropertySet )

// do this in derived classes!

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( FormattedString,
                             C2U( "com.sun.star.comp.chart.FormattedString" ));

} //  namespace chart
