/*************************************************************************
 *
 *  $RCSfile: Legend.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-20 09:59:31 $
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
#include "Legend.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "CharacterProperties.hxx"
#include "UserDefinedProperties.hxx"
#include "LegendHelper.hxx"
#include "LayoutDefaults.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_ALIGNMENT_HPP_
#include <drafts/com/sun/star/layout/Alignment.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_STRETCHMODE_HPP_
#include <drafts/com/sun/star/layout/StretchMode.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDPOSITION_HPP_
#include <drafts/com/sun/star/chart2/LegendPosition.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_LEGENDEXPANSION_HPP_
#include <drafts/com/sun/star/chart2/LegendExpansion.hpp>
#endif

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans::PropertyAttribute;
using namespace ::drafts::com::sun::star;

using ::com::sun::star::beans::Property;

namespace
{

static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.Legend" ));

enum
{
    PROP_LEGEND_POSITION,
    PROP_LEGEND_PREFERRED_EXPANSION,
    PROP_LEGEND_SHOW
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "Position" ),
                  PROP_LEGEND_POSITION,
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
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LEGEND_POSITION ));
    rOutMap[ PROP_LEGEND_POSITION ] =
        uno::makeAny( chart2::LegendPosition_LINE_END );

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_LEGEND_SHOW ));
    rOutMap[ PROP_LEGEND_SHOW ] =
        uno::makeAny( sal_True );
}

const uno::Sequence< Property > & lcl_GetPropertySequence()
{
    static uno::Sequence< Property > aPropSeq;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aPropSeq.getLength() )
    {
        // get properties
        ::std::vector< ::com::sun::star::beans::Property > aProperties;
        lcl_AddPropertiesToVector( aProperties );
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::FillProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::CharacterProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
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

namespace chart
{

Legend::Legend( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex ),
        m_aIdentifier( LegendHelper::getIdentifierForLegend() )
{
    setAnchorAndRelposFromProperty( GetDefaultValue( PROP_LEGEND_POSITION ));
}

Legend::~Legend()
{
}

// ____ XLegend ____
void SAL_CALL Legend::registerEntry( const uno::Reference< chart2::XLegendEntry >& xEntry )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ::std::find( m_aLegendEntries.begin(),
                     m_aLegendEntries.end(),
                     xEntry ) != m_aLegendEntries.end())
        throw lang::IllegalArgumentException();

    m_aLegendEntries.push_back( xEntry );
}

void SAL_CALL Legend::revokeEntry( const uno::Reference< chart2::XLegendEntry >& xEntry )
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
}

uno::Sequence< uno::Reference< chart2::XLegendEntry > > SAL_CALL Legend::getEntries()
    throw (uno::RuntimeException)
{
    return ::chart::helper::VectorToSequence( m_aLegendEntries );
}

::rtl::OUString SAL_CALL Legend::getIdentifier()
    throw (uno::RuntimeException)
{
    return m_aIdentifier;
}

// ____ XAnchoredObject ____
void SAL_CALL Legend::setAnchor( const layout::AnchorPoint& aAnchor )
    throw (uno::RuntimeException)
{
    m_aAnchor = aAnchor;
}

layout::AnchorPoint SAL_CALL Legend::getAnchor()
    throw (uno::RuntimeException)
{
    return m_aAnchor;
}

void SAL_CALL Legend::setRelativePosition( const layout::RelativePoint& aPosition )
    throw (uno::RuntimeException)
{
    m_aRelativePosition = aPosition;
}

layout::RelativePoint SAL_CALL Legend::getRelativePosition()
    throw (uno::RuntimeException)
{
    return m_aRelativePosition;
}

// private
void Legend::setAnchorAndRelposFromProperty( const uno::Any & rValue )
{
    chart2::LegendPosition ePos;
    if( rValue >>= ePos )
    {
        m_aAnchor.AnchorHolder = uno::Reference< layout::XAnchor >();

        // shift legend about 2% into the primary direction
        m_aRelativePosition.Primary   = 0.02;
        m_aRelativePosition.Secondary = 0.0;

        switch( ePos )
        {
            case chart2::LegendPosition_LINE_START:
                m_aAnchor.Alignment = ::layout_defaults::const_aLineStart;
                m_aAnchor.EscapeDirection = 0.0;
                break;
            case chart2::LegendPosition_LINE_END:
                m_aAnchor.Alignment = ::layout_defaults::const_aLineEnd;
                m_aAnchor.EscapeDirection = 180.0;
                break;
            case chart2::LegendPosition_PAGE_START:
                m_aAnchor.Alignment = ::layout_defaults::const_aPageStart;
                m_aAnchor.EscapeDirection = 270.0;
                break;
            case chart2::LegendPosition_PAGE_END:
                m_aAnchor.Alignment = ::layout_defaults::const_aPageEnd;
                m_aAnchor.EscapeDirection = 90.0;
                break;

            case chart2::LegendPosition_CUSTOM:
            // to avoid warning
            case chart2::LegendPosition_MAKE_FIXED_SIZE:
                // nothing to be set
                break;
        }
    }
}

// ================================================================================

uno::Sequence< ::rtl::OUString > Legend::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 5 );
    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.Legend" );
    aServices[ 1 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 2 ] = C2U( "com.sun.star.drawing.FillProperties" );
    aServices[ 3 ] = C2U( "com.sun.star.drawing.LineProperties" );
    aServices[ 4 ] = C2U( "drafts.com.sun.star.layout.LayoutElement" );
    return aServices;
}

// ____ OPropertySet ____
uno::Any Legend::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        lcl_AddDefaultsToMap( aStaticDefaults );
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        FillProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        CharacterProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL Legend::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    Legend::getPropertySetInfo()
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

void SAL_CALL Legend::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle, const uno::Any& rValue )
    throw (uno::Exception)
{
    if( nHandle == PROP_LEGEND_POSITION )
        setAnchorAndRelposFromProperty( rValue );

    OPropertySet::setFastPropertyValue_NoBroadcast( nHandle, rValue );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Legend, lcl_aServiceName );

// needed by MSC compiler
using impl::Legend_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Legend, Legend_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Legend, Legend_Base, ::property::OPropertySet )

} //  namespace chart
