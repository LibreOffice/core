/*************************************************************************
 *
 *  $RCSfile: Axis.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:30 $
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
#include "Axis.hxx"
#include "macros.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "algohelper.hxx"
#include "PropertyHelper.hxx"
#include "Increment.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_ALIGNMENT_HPP_
#include <drafts/com/sun/star/layout/Alignment.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_STRETCHMODE_HPP_
#include <drafts/com/sun/star/layout/StretchMode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_NUMBERFORMAT_HPP_
#include <drafts/com/sun/star/chart2/NumberFormat.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEDASH_HPP_
#include <com/sun/star/drawing/LineDash.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINEJOINT_HPP_
#include <com/sun/star/drawing/LineJoint.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTAXISARRANGEORDERTYPE_HPP_
#include <com/sun/star/chart/ChartAxisArrangeOrderType.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.Axis" ));

enum
{
    PROP_AXIS_DISPLAY_LABELS,
    PROP_AXIS_TEXT_ROTATION,
    PROP_AXIS_TEXT_BREAK,
    PROP_AXIS_TEXT_OVERLAP,
    PROP_AXIS_TEXT_STACKED,
    PROP_AXIS_TEXT_ARRANGE_ORDER,
    PROP_AXIS_NUMBER_FORMAT,

    // for Testing only!
    PROP_AXIS_MAJOR_TICKMARKS,
    // for Testing only!
    PROP_AXIS_MINOR_TICKMARKS,

    // com.sun.star.drawing.LineProperties
    PROP_AXIS_LINE_STYLE,
    PROP_AXIS_LINE_WIDTH,
    PROP_AXIS_LINE_DASH,
    PROP_AXIS_LINE_COLOR,
    PROP_AXIS_LINE_TRANSPARENCE,
    PROP_AXIS_LINE_JOINT
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "DisplayLabels" ),
                  PROP_AXIS_DISPLAY_LABELS,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_AXIS_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextBreak" ),
                  PROP_AXIS_TEXT_BREAK,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextOverlap" ),
                  PROP_AXIS_TEXT_OVERLAP,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "StackCharacters" ),
                  PROP_AXIS_TEXT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ArrangeOrder" ),
                  PROP_AXIS_TEXT_ARRANGE_ORDER,
                  ::getCppuType( reinterpret_cast< const ::com::sun::star::chart::ChartAxisArrangeOrderType * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "NumberFormat" ),
                  PROP_AXIS_NUMBER_FORMAT,
                  ::getCppuType( reinterpret_cast< const chart2::NumberFormat * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // for Testing only!
    rOutProperties.push_back(
        Property( C2U( "MajorTickmarks" ),
                  PROP_AXIS_MAJOR_TICKMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    // for Testing only!
    rOutProperties.push_back(
        Property( C2U( "MinorTickmarks" ),
                  PROP_AXIS_MINOR_TICKMARKS,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    // com.sun.star.drawing.LineProperties
    // -----------------------------------
    rOutProperties.push_back(
        Property( C2U( "LineStyle" ),
                  PROP_AXIS_LINE_STYLE,
                  ::getCppuType( reinterpret_cast< const drawing::LineStyle * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineWidth" ),
                  PROP_AXIS_LINE_WIDTH,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineDash" ),
                  PROP_AXIS_LINE_DASH,
                  ::getCppuType( reinterpret_cast< const drawing::LineDash * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( C2U( "LineColor" ),
                  PROP_AXIS_LINE_COLOR,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineTransparence" ),
                  PROP_AXIS_LINE_TRANSPARENCE,
                  ::getCppuType( reinterpret_cast< const sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "LineJoint" ),
                  PROP_AXIS_LINE_JOINT,
                  ::getCppuType( reinterpret_cast< const drawing::LineJoint * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_DISPLAY_LABELS ));
    rOutMap[ PROP_AXIS_DISPLAY_LABELS ] =
        uno::makeAny( sal_Bool( sal_True ) );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_TEXT_ROTATION ));
    rOutMap[ PROP_AXIS_TEXT_ROTATION ] =
        uno::makeAny( double( 0.0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_TEXT_BREAK ));
    rOutMap[ PROP_AXIS_TEXT_BREAK ] =
        uno::makeAny( sal_Bool( sal_True ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_TEXT_OVERLAP ));
    rOutMap[ PROP_AXIS_TEXT_OVERLAP ] =
        uno::makeAny( sal_Bool( sal_False ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_TEXT_STACKED ));
    rOutMap[ PROP_AXIS_TEXT_STACKED ] =
        uno::makeAny( sal_Bool( sal_False ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_TEXT_ARRANGE_ORDER ));
    rOutMap[ PROP_AXIS_TEXT_ARRANGE_ORDER ] =
        uno::makeAny( ::com::sun::star::chart::ChartAxisArrangeOrderType_AUTO );
    chart2::NumberFormat aFormat( C2U( "Standard" ),
                                  lang::Locale( C2U( "DE" ), C2U( "de" ), ::rtl::OUString()));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_NUMBER_FORMAT ));
    rOutMap[ PROP_AXIS_NUMBER_FORMAT ] =
        uno::makeAny( aFormat );

    // for Testing only!
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_MAJOR_TICKMARKS ));
    rOutMap[ PROP_AXIS_MAJOR_TICKMARKS ] =
        uno::makeAny( sal_Int32( 2 /* CHAXIS_MARK_OUTER */ ) );
    // for Testing only!
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_MINOR_TICKMARKS ));
    rOutMap[ PROP_AXIS_MINOR_TICKMARKS ] =
        uno::makeAny( sal_Int32( 0 /* CHAXIS_MARK_NONE */ ) );


    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_LINE_STYLE ));
    rOutMap[ PROP_AXIS_LINE_STYLE ] =
        uno::makeAny( drawing::LineStyle_SOLID );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_LINE_WIDTH ));
    rOutMap[ PROP_AXIS_LINE_WIDTH ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_LINE_COLOR ));
    rOutMap[ PROP_AXIS_LINE_COLOR ] =
        uno::makeAny( sal_Int32( 0x000000 ) );  // black
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_LINE_TRANSPARENCE ));
    rOutMap[ PROP_AXIS_LINE_TRANSPARENCE ] =
        uno::makeAny( sal_Int16( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_AXIS_LINE_JOINT ));
    rOutMap[ PROP_AXIS_LINE_JOINT ] =
        uno::makeAny( drawing::LineJoint_NONE );
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::CharacterProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );
        ::chart::UserDefinedProperties::AddPropertiesToVector( aProperties );

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

// ================================================================================

namespace chart
{

Axis::Axis( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex )
{
    m_xIncrement = new Increment();

    if( ! ( xContext->getValueByName( C2U( "Identifier" )) >>= m_aIdentifier ))
    {
        OSL_ENSURE( false, "Missing Axis identifier" );
    }

#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "Axis CTOR: Id=%s", U2C( m_aIdentifier ));
#endif
}

Axis::~Axis()
{
}

// --------------------------------------------------------------------------------

// ____ XAxis ____
// (nothing)

// ____ XMeter ____
void SAL_CALL Axis::attachCoordinateSystem(
    const uno::Reference< chart2::XBoundedCoordinateSystem >& xCoordSys,
    sal_Int32 nRepresentedDimension )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    if( nRepresentedDimension >= xCoordSys->getDimension())
        throw lang::IndexOutOfBoundsException();

    m_xCoordinateSystem = xCoordSys;
    m_nRepresentedDimension = nRepresentedDimension;
    // \--
}

uno::Reference< chart2::XBoundedCoordinateSystem > SAL_CALL Axis::getCoordinateSystem()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    return m_xCoordinateSystem;
    // \--
}

sal_Int32 SAL_CALL Axis::getRepresentedDimension()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    return m_nRepresentedDimension;
    // \--
}

void SAL_CALL Axis::setIncrement( const uno::Reference< chart2::XIncrement >& aIncrement )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    m_xIncrement = aIncrement;
    // \--
}

uno::Reference< chart2::XIncrement > SAL_CALL Axis::getIncrement()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    return m_xIncrement;
    // \--
}

// ____ XIdentifiable ____
::rtl::OUString SAL_CALL Axis::getIdentifier()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    return m_aIdentifier;
    // \--
}

// ================================================================================

// ____ OPropertySet ____
uno::Any Axis::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        // initialize defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
        CharacterProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL Axis::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    Axis::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !xInfo.is())
    {
        xInfo = ::cppu::OPropertySetHelper::createPropertySetInfo(
            getInfoHelper());
    }

    return xInfo;
    // \--
}

// ================================================================================

uno::Sequence< ::rtl::OUString > Axis::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 3 );
    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.Axis" );
    aServices[ 1 ] = C2U( "drafts.com.sun.star.chart2.ChartElement" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    return aServices;
}

using impl::Axis_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Axis, Axis_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Axis, Axis_Base, ::property::OPropertySet )

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Axis, lcl_aServiceName );

} //  namespace chart
