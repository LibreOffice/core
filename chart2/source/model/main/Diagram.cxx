/*************************************************************************
 *
 *  $RCSfile: Diagram.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-04 12:37:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "Diagram.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "algohelper.hxx"
#include "Wall.hxx"
#include "ContextHelper.hxx"
#include "UserDefinedProperties.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include <algorithm>
#include <iterator>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using namespace ::drafts::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::osl::MutexGuard;

// ======================================================================

namespace
{

// enum
// {
//     PROP_DIAGRAM_PREFERRED_ALIGNMENT,
//     PROP_DIAGRAM_PREFERRED_STRETCH_MODE
// };

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
//     rOutProperties.push_back(
//         Property( C2U( "PreferredAlignment" ),
//                   PROP_DIAGRAM_PREFERRED_ALIGNMENT,
//                   ::getCppuType( reinterpret_cast< const layout::Alignment * >(0)),
//                   beans::PropertyAttribute::BOUND
//                   | beans::PropertyAttribute::MAYBEDEFAULT ));

//     rOutProperties.push_back(
//         Property( C2U( "PreferredStretchMode" ),
//                   PROP_DIAGRAM_PREFERRED_STRETCH_MODE,
//                   ::getCppuType( reinterpret_cast< const layout::StretchMode * >(0)),
//                   beans::PropertyAttribute::BOUND
//                   | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
//     layout::Alignment aCenterAlign( 0.5, 0.5, true );
//     OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DIAGRAM_PREFERRED_ALIGNMENT ));
//     rOutMap[ PROP_DIAGRAM_PREFERRED_ALIGNMENT ] =
//         uno::makeAny( aCenterAlign );

//     OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_DIAGRAM_PREFERRED_STRETCH_MODE ));
//     rOutMap[ PROP_DIAGRAM_PREFERRED_STRETCH_MODE ] =
//         uno::makeAny( layout::StretchMode_BOTH );
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
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

        // and sort them for access via bsearch
        ::std::sort( aProperties.begin(), aProperties.end(),
                     ::chart::helper::PropertyNameLess() );

        // transfer result to static Sequence
        aPropSeq = ::chart::helper::VectorToSequence( aProperties );
    }

    return aPropSeq;
}

} // anonymous namespace

// ======================================================================

namespace chart
{

Diagram::Diagram( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
    m_xContext( xContext ),
    m_xWall( new Wall()),
    m_xTitle( NULL ),
    m_aIdentifier( C2U( "@diagram" ))
{
    if( ! m_xContext.is() ||
        ! ( xContext->getValueByName( C2U( "TemplateServiceName" )) >>= m_aTemplateServiceName ))
    {
        OSL_ENSURE( false, "Missing argument TemplateServiceName" );
    }
}

Diagram::~Diagram()
{}

// ____ XDiagram ____
Reference< chart2::XDataSeriesTreeParent > SAL_CALL Diagram::getTree()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xSeriesTree;
    // \--
}

void SAL_CALL Diagram::setTree(
    const Reference< chart2::XDataSeriesTreeParent >& xTree )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Reference< lang::XServiceInfo > xInfo( xTree, uno::UNO_QUERY );
    if( xInfo.is() &&
        xInfo->supportsService( C2U( "drafts.com.sun.star.chart2.DataSeriesTree" )) )
    {
        // /--
        MutexGuard aGuard( GetMutex() );
        m_xSeriesTree = xTree;
        // \--
    }
    else
    {
        throw lang::IllegalArgumentException();
    }
}

// ::rtl::OUString SAL_CALL Diagram::getChartTypeTemplateServiceName()
//     throw (uno::RuntimeException)
// {
//     return m_aTemplateServiceName;
// }

uno::Reference< beans::XPropertySet > SAL_CALL Diagram::getWall()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xWall;
    // \--
}

uno::Reference< chart2::XLegend > SAL_CALL Diagram::getLegend()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xLegend;
    // \--
}

void SAL_CALL Diagram::setLegend( const uno::Reference< chart2::XLegend >& xLegend )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    m_xLegend = xLegend;
    // \--
}

// ____ XAxisContainer ____
void SAL_CALL Diagram::addAxis( const uno::Reference< chart2::XAxis >& aAxis )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Reference< chart2::XIdentifiable > xIdent( aAxis, uno::UNO_QUERY );
    if( ! xIdent.is() ||
        m_aAxes.find( xIdent->getIdentifier() ) != m_aAxes.end() )
        throw lang::IllegalArgumentException();

    m_aAxes.insert( tAxisContainerType::value_type( xIdent->getIdentifier(), aAxis ) );
}

void SAL_CALL Diagram::removeAxis( const uno::Reference< chart2::XAxis >& aAxis )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( ! aAxis.is())
        throw container::NoSuchElementException();

    uno::Reference< chart2::XIdentifiable > xIdent( aAxis, uno::UNO_QUERY );
    if( xIdent.is())
    {
        tAxisContainerType::iterator aIt( m_aAxes.find( xIdent->getIdentifier() ));

        if( aIt == m_aAxes.end())
            throw container::NoSuchElementException();

        m_aAxes.erase( aIt );
    }
    else
        throw container::NoSuchElementException();
}

uno::Sequence< uno::Reference< chart2::XAxis > > SAL_CALL Diagram::getAxes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< chart2::XAxis > > aResult( m_aAxes.size());

    ::std::transform( m_aAxes.begin(), m_aAxes.end(),
                      aResult.getArray(),
                      ::std::select2nd< tAxisContainerType::value_type >() );

    return aResult;
}

uno::Reference< chart2::XAxis > SAL_CALL Diagram::getAxisByIdentifier( const ::rtl::OUString& aIdentifier )
    throw (uno::RuntimeException)
{
    if(!aIdentifier.getLength())
        return NULL;
    uno::Reference< chart2::XAxis > aResult;
    tAxisContainerType::const_iterator aIt( m_aAxes.find( aIdentifier ));

    if( aIt != m_aAxes.end())
        aResult = (*aIt).second;

    return aResult;
}

// ____ XGridContainer ____
void SAL_CALL Diagram::addGrid( const uno::Reference< chart2::XGrid >& aGrid )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Reference< chart2::XIdentifiable > xIdent( aGrid, uno::UNO_QUERY );
    if( ! xIdent.is() ||
        m_aGrids.find( xIdent->getIdentifier() ) != m_aGrids.end() )
        throw lang::IllegalArgumentException();

    m_aGrids.insert( tGridContainerType::value_type( xIdent->getIdentifier(), aGrid ));
}

void SAL_CALL Diagram::removeGrid( const uno::Reference< chart2::XGrid >& aGrid )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    if( ! aGrid.is())
        throw container::NoSuchElementException();

    uno::Reference< chart2::XIdentifiable > xIdent( aGrid, uno::UNO_QUERY );
    if( xIdent.is())
    {
        tGridContainerType::iterator aIt( m_aGrids.find( xIdent->getIdentifier() ));

        if( aIt == m_aGrids.end())
            throw container::NoSuchElementException();

        m_aGrids.erase( aIt );
    }
    else
        throw container::NoSuchElementException();
}

uno::Sequence< uno::Reference< chart2::XGrid > > SAL_CALL Diagram::getGrids()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Reference< chart2::XGrid > > aResult( m_aGrids.size());

    ::std::transform( m_aGrids.begin(), m_aGrids.end(),
                      aResult.getArray(),
                      ::std::select2nd< tGridContainerType::value_type >() );

    return aResult;
}

uno::Reference< chart2::XGrid > SAL_CALL Diagram::getGridByIdentifier( const ::rtl::OUString& aIdentifier )
    throw (uno::RuntimeException)
{
    if(!aIdentifier.getLength())
        return NULL;
    uno::Reference< chart2::XGrid > aResult;
    tGridContainerType::const_iterator aIt( m_aGrids.find( aIdentifier ));

    if( aIt != m_aGrids.end())
        aResult = (*aIt).second;

    return aResult;
}

// ____ XTitled ____
uno::Reference< chart2::XTitle > SAL_CALL Diagram::getTitle()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_xTitle;
    // \--
}

void SAL_CALL Diagram::setTitle( const uno::Reference< chart2::XTitle >& Title )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    m_xTitle = Title;
    // \--
}

// ____ XBoundedCoordinateSystemContainer ____
void SAL_CALL Diagram::addCoordinateSystem(
    const uno::Reference< chart2::XBoundedCoordinateSystem >& aCoordSys )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
//     if( m_aCoordSystems.find( aCoordSys ) != m_aCoordSystems.end())
    if( ::std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys )
        != m_aCoordSystems.end())
        throw lang::IllegalArgumentException();

//     m_aCoordSystems.insert( aCoordSys );
    m_aCoordSystems.push_back( aCoordSys );
}

void SAL_CALL Diagram::removeCoordinateSystem(
    const uno::Reference< chart2::XBoundedCoordinateSystem >& aCoordSys )
    throw (container::NoSuchElementException,
           uno::RuntimeException)
{
    ::std::vector< uno::Reference< chart2::XBoundedCoordinateSystem > >::iterator
          aIt( ::std::find( m_aCoordSystems.begin(), m_aCoordSystems.end(), aCoordSys ));
//     if( m_aCoordSystems.find( aCoordSys ) == m_aCoordSystems.end())
    if( aIt != m_aCoordSystems.end())
        throw container::NoSuchElementException();

//     m_aCoordSystems.erase( aCoordSys );
    m_aCoordSystems.erase( aIt );
}

uno::Sequence< uno::Reference< chart2::XBoundedCoordinateSystem > > SAL_CALL Diagram::getCoordinateSystems()
    throw (uno::RuntimeException)
{
//     return helper::SetToSequence( m_aCoordSystems );
    return helper::VectorToSequence( m_aCoordSystems );
}

// ____ XIdentifiable ____
::rtl::OUString SAL_CALL Diagram::getIdentifier()
    throw (uno::RuntimeException)
{
    return m_aIdentifier;
}

// ________

Sequence< OUString > Diagram::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 3 );

    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.Diagram" );
    aServices[ 1 ] = C2U( "drafts.com.sun.star.layout.LayoutElement" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any Diagram::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper & SAL_CALL Diagram::getInfoHelper()
{
    static ::cppu::OPropertyArrayHelper aArrayHelper( lcl_GetPropertySequence(),
                                                      /* bSorted = */ sal_True );

    return aArrayHelper;
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    Diagram::getPropertySetInfo()
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

using impl::Diagram_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Diagram, Diagram_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Diagram, Diagram_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Diagram,
                             C2U( "com.sun.star.comp.chart2.Diagram" ));

} //  namespace chart
