/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "Legend.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
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
#include <com/sun/star/chart2/LegendExpansion.hpp>
#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;

using ::rtl::OUString;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::Property;

namespace
{

static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.Legend" ));

enum
{
    PROP_LEGEND_ANCHOR_POSITION,
    PROP_LEGEND_PREFERRED_EXPANSION,
    PROP_LEGEND_SHOW,
    PROP_LEGEND_REF_PAGE_SIZE,
    PROP_LEGEND_REL_POS,
    PROP_LEGEND_REL_SIZE
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "AnchorPosition" ),
                  PROP_LEGEND_ANCHOR_POSITION,
                  ::getCppuType( reinterpret_cast< const chart2::LegendPosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Expansion" ),
                  PROP_LEGEND_PREFERRED_EXPANSION,
                  ::getCppuType( reinterpret_cast< const chart2::LegendExpansion * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "Show" ),
                  PROP_LEGEND_SHOW,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "ReferencePageSize" ),
                  PROP_LEGEND_REF_PAGE_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "RelativePosition" ),
                  PROP_LEGEND_REL_POS,
                  ::getCppuType( reinterpret_cast< const chart2::RelativePosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));


    rOutProperties.push_back(
        Property( C2U( "RelativeSize" ),
                  PROP_LEGEND_REL_SIZE,
                  ::getCppuType( reinterpret_cast< const chart2::RelativeSize * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_ANCHOR_POSITION, chart2::LegendPosition_LINE_END );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_PREFERRED_EXPANSION, chart2::LegendExpansion_HIGH );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_LEGEND_SHOW, true );

    float fDefaultCharHeight = 10.0;
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_CHAR_HEIGHT, fDefaultCharHeight );
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_ASIAN_CHAR_HEIGHT, fDefaultCharHeight );
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::CharacterProperties::PROP_CHAR_COMPLEX_CHAR_HEIGHT, fDefaultCharHeight );
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
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

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
    CloneHelper::CloneRefVector< Reference< chart2::XLegendEntry > >( rOther.m_aLegendEntries, m_aLegendEntries );
    ModifyListenerHelper::addListenerToAllElements( m_aLegendEntries, m_xModifyEventForwarder );
}

Legend::~Legend()
{
    try
    {
        ModifyListenerHelper::removeListenerFromAllElements( m_aLegendEntries, m_xModifyEventForwarder );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

// ____ XLegend ____
void SAL_CALL Legend::registerEntry( const Reference< chart2::XLegendEntry >& xEntry )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ::std::find( m_aLegendEntries.begin(),
                     m_aLegendEntries.end(),
                     xEntry ) != m_aLegendEntries.end())
        throw lang::IllegalArgumentException();

    m_aLegendEntries.push_back( xEntry );
    ModifyListenerHelper::addListener( xEntry, m_xModifyEventForwarder );
    fireModifyEvent();
}

void SAL_CALL Legend::revokeEntry( const Reference< chart2::XLegendEntry >& xEntry )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    tLegendEntries::iterator aIt(
        ::std::find( m_aLegendEntries.begin(),
                     m_aLegendEntries.end(),
                     xEntry ));

    if( aIt == m_aLegendEntries.end())
        throw container::NoSuchElementException();

    m_aLegendEntries.erase( aIt );
    ModifyListenerHelper::removeListener( xEntry, m_xModifyEventForwarder );
    fireModifyEvent();
}

Sequence< Reference< chart2::XLegendEntry > > SAL_CALL Legend::getEntries()
    throw (uno::RuntimeException)
{
    return ContainerHelper::ContainerToSequence( m_aLegendEntries );
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

// ================================================================================

Sequence< OUString > Legend::getSupportedServiceNames_Static()
{
    const sal_Int32 nNumServices( 6 );
    sal_Int32 nI = 0;
    Sequence< OUString > aServices( nNumServices );
    aServices[ nI++ ] = C2U( "com.sun.star.chart2.Legend" );
    aServices[ nI++ ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ nI++ ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ nI++ ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ nI++ ] = C2U( "com.sun.star.style.CharacterProperties" );
    aServices[ nI++ ] = C2U( "com.sun.star.layout.LayoutElement" );
    OSL_ASSERT( nNumServices == nI );
    return aServices;
}

// ____ OPropertySet ____
Any Legend::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        LineProperties::AddDefaultsToMap( aStaticDefaults );
        FillProperties::AddDefaultsToMap( aStaticDefaults );
        CharacterProperties::AddDefaultsToMap( aStaticDefaults );
        // call last to overwrite some character property defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL Legend::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    Legend::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

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

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Legend, lcl_aServiceName );

// needed by MSC compiler
using impl::Legend_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Legend, Legend_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Legend, Legend_Base, ::property::OPropertySet )

} //  namespace chart
