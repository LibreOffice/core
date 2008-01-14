/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StockBar.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:02:06 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "StockBar.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "PropertyHelper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <algorithm>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

// ____________________________________________________________

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.StockBar" ));

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    // override other defaults
    ::chart::PropertyHelper::setPropertyValue< sal_Int32 >( rOutMap, ::chart::FillProperties::PROP_FILL_COLOR, 0xffffff ); // white
}

::cppu::IPropertyArrayHelper & lcl_getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper(
        lcl_GetPropertySequence(),
        /* bSorted = */ sal_True );

    return aArrayHelper;
}

} // anonymous namespace

// ____________________________________________________________

namespace chart
{

StockBar::StockBar( bool bRisingCourse ) :
        ::property::OPropertySet( m_aMutex ),
    m_bRisingCourse( bRisingCourse ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    if( ! m_bRisingCourse )
    {
        setFastPropertyValue_NoBroadcast(
            ::chart::FillProperties::PROP_FILL_COLOR,
            uno::makeAny( sal_Int32( 0x000000 ))); // black
        setFastPropertyValue_NoBroadcast(
            ::chart::LineProperties::PROP_LINE_COLOR,
            uno::makeAny( sal_Int32( 0xb3b3b3 ))); // gray30
    }
}

StockBar::StockBar( const StockBar & rOther ) :
        MutexContainer(),
        impl::StockBar_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
    m_bRisingCourse( rOther.m_bRisingCourse ),
    m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{}

StockBar::~StockBar()
{}


// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL StockBar::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new StockBar( *this ));
}

// ____ OPropertySet ____
uno::Any StockBar::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        LineProperties::AddDefaultsToMap( aStaticDefaults );
        FillProperties::AddDefaultsToMap( aStaticDefaults );

        // overrides a line property
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL StockBar::getInfoHelper()
{
    return lcl_getInfoHelper();
}

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL
    StockBar::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static Reference< beans::XPropertySetInfo > xInfo;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            lcl_getInfoHelper());
    }

    return xInfo;
    // \--
}


// ____ XModifyBroadcaster ____
void SAL_CALL StockBar::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL StockBar::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL StockBar::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL StockBar::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void StockBar::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void StockBar::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

// ================================================================================

uno::Sequence< ::rtl::OUString > StockBar::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 2 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.StockBar" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( StockBar, lcl_aServiceName );

using impl::StockBar_Base;

IMPLEMENT_FORWARD_XINTERFACE2( StockBar, StockBar_Base, ::property::OPropertySet )

} //  namespace chart
