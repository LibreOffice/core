/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GridWrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:18:14 $
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

#include "GridWrapper.hxx"
#include "macros.hxx"
#include "AxisHelper.hxx"
#include "Scaling.hxx"
#include "Chart2ModelContact.hxx"
#include "ContainerHelper.hxx"
#include "AxisIndexDefines.hxx"

#ifndef INCLUDED_COMPHELPER_INLINE_CONTAINER_HXX
#include <comphelper/InlineContainer.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include "LineProperties.hxx"
// #include "NamedLineProperties.hxx"
#include "UserDefinedProperties.hxx"
// #include "WrappedNamedProperty.hxx"
#include "WrappedDefaultProperty.hxx"

#include <algorithm>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Grid" ));

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
//         ::chart::NamedLineProperties::AddPropertiesToVector( aProperties );
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

namespace chart
{
namespace wrapper
{

GridWrapper::GridWrapper(
    tGridType eType, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact ) :
        m_spChart2ModelContact( spChart2ModelContact ),
        m_aEventListenerContainer( m_aMutex ),
        m_eType( eType )
{
}

GridWrapper::~GridWrapper()
{}

// static
void GridWrapper::getDimensionAndSubGridBool( tGridType eType, sal_Int32& rnDimensionIndex, bool& rbSubGrid )
{
    rnDimensionIndex = 1;
    rbSubGrid = false;

    switch( eType )
    {
        case X_MAIN_GRID:
            rnDimensionIndex = 0; rbSubGrid = false; break;
        case Y_MAIN_GRID:
            rnDimensionIndex = 1; rbSubGrid = false; break;
        case Z_MAIN_GRID:
            rnDimensionIndex = 2; rbSubGrid = false; break;
        case X_SUB_GRID:
            rnDimensionIndex = 0; rbSubGrid = true;  break;
        case Y_SUB_GRID:
            rnDimensionIndex = 1; rbSubGrid = true;  break;
        case Z_SUB_GRID:
            rnDimensionIndex = 2; rbSubGrid = true;  break;
    }
}

// ____ XComponent ____
void SAL_CALL GridWrapper::dispose()
    throw (uno::RuntimeException)
{
    Reference< uno::XInterface > xSource( static_cast< ::cppu::OWeakObject* >( this ) );
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( xSource ) );

    clearWrappedPropertySet();
}

void SAL_CALL GridWrapper::addEventListener(
    const Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL GridWrapper::removeEventListener(
    const Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

// ================================================================================

// WrappedPropertySet

Reference< beans::XPropertySet > GridWrapper::getInnerPropertySet()
{
    Reference< beans::XPropertySet > xRet;
    try
    {
        Reference< chart2::XDiagram > xDiagram( m_spChart2ModelContact->getChart2Diagram() );
        uno::Reference< XCoordinateSystem > xCooSys( AxisHelper::getCoordinateSystemByIndex( xDiagram, 0 /*nCooSysIndex*/ ) );

        sal_Int32 nDimensionIndex = 1;
        bool bSubGrid = false;
        getDimensionAndSubGridBool( m_eType, nDimensionIndex, bSubGrid );

        sal_Int32 nSubGridIndex = bSubGrid ? 0 : -1;
        xRet.set( AxisHelper::getGridProperties( xCooSys , nDimensionIndex, MAIN_AXIS_INDEX, nSubGridIndex ) );
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return xRet;
}

const Sequence< beans::Property >& GridWrapper::getPropertySequence()
{
    return lcl_GetPropertySequence();
}

const std::vector< WrappedProperty* > GridWrapper::createWrappedProperties()
{
    ::std::vector< ::chart::WrappedProperty* > aWrappedProperties;

//     WrappedNamedProperty::addWrappedLineProperties( aWrappedProperties, m_spChart2ModelContact );
    aWrappedProperties.push_back( new WrappedDefaultProperty( C2U("LineColor"), C2U("LineColor"), uno::makeAny( sal_Int32( 0x000000) ) ) ); // black

    return aWrappedProperties;
}

// ================================================================================

Sequence< OUString > GridWrapper::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartGrid" );
    aServices[ 1 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ 3 ] = C2U( "com.sun.star.beans.PropertySet" );

    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( GridWrapper, lcl_aServiceName );

} //  namespace wrapper
} //  namespace chart
