/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AreaWrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:15:43 $
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

#include "AreaWrapper.hxx"
#include "macros.hxx"
#include "ContainerHelper.hxx"
#include "Chart2ModelContact.hxx"
#include "WrappedDirectStateProperty.hxx"

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif

#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"
// #include "NamedProperties.hxx"
// #include "WrappedNamedProperty.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Area" ));

const Sequence< Property > & lcl_GetPropertySequence()
{
    static Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );
//         ::chart::NamedProperties::AddPropertiesToVector( aProperties );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::ContainerHelper::ContainerToSequence( aProperties );
    }

    return aPropSeq;
}
} // anonymous namespace

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

AreaWrapper::AreaWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex )
{
}

AreaWrapper::~AreaWrapper()
{}

// ____ XShape ____
awt::Point SAL_CALL AreaWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return awt::Point(0,0);
}

void SAL_CALL AreaWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set position of chart area" );
}

awt::Size SAL_CALL AreaWrapper::getSize()
    throw (uno::RuntimeException)
{
    return m_spChart2ModelContact->GetPageSize();
}

void SAL_CALL AreaWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set size of chart area" );
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL AreaWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartArea" );
}

// ____ XComponent ____
void SAL_CALL AreaWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    // /--
    MutexGuard aGuard( GetMutex());
    clearWrappedPropertySet();
    // \--
}

void SAL_CALL AreaWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL AreaWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

// WrappedPropertySet
Reference< beans::XPropertySet > AreaWrapper::getInnerPropertySet()
{
    Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
    if( xChartDoc.is() )
        return xChartDoc->getPageBackground();
    return 0;
}

const Sequence< beans::Property >& AreaWrapper::getPropertySequence()
{
    return lcl_GetPropertySequence();
}

const std::vector< WrappedProperty* > AreaWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

    aWrappedProperties.push_back( new WrappedDirectStateProperty( C2U("LineStyle"), C2U("LineStyle") ) );

//     WrappedNamedProperty::addWrappedProperties( aWrappedProperties, m_spChart2ModelContact );

    return aWrappedProperties;
}

// ================================================================================

Sequence< ::rtl::OUString > AreaWrapper::getSupportedServiceNames_Static()
{
    Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ 3 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( AreaWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
