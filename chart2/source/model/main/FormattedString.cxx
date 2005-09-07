/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FormattedString.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:59:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "FormattedString.hxx"

#include "CharacterProperties.hxx"
#include "PropertyHelper.hxx"
#include "algohelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

namespace
{

enum
{
    PROP_FORMATTED_STRING_REF_PAGE_SIZE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "ReferencePageSize" ),
                  PROP_FORMATTED_STRING_REF_PAGE_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
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

FormattedString::FormattedString( const ::rtl::OUString & rString ) :
        ::property::OPropertySet( m_aMutex ),
    m_aString( rString )
{}

FormattedString::FormattedString(
        uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
    m_aString()
{}

FormattedString::~FormattedString()
{}

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
    // \--
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
    // todo: default is just for testing. should be void
    if( nHandle == PROP_FORMATTED_STRING_REF_PAGE_SIZE )
    {
        return uno::makeAny( awt::Size( 10000, 7500 ) );
    }
    // remove till here

    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        CharacterProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
    }

    helper::tPropertyValueMap::const_iterator aFound(
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
