/*************************************************************************
 *
 *  $RCSfile: Title.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:31 $
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
#include "Title.hxx"
#include "macros.hxx"
#include "algohelper.hxx"
#include "FormattedString.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"

#ifndef CHART_PROPERTYHELPER_HXX
#include "PropertyHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_PARAGRAPHADJUST_HPP_
#include <com/sun/star/style/ParagraphAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
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
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.Title" ));

enum
{
    PROP_TITLE_PARA_ADJUST,
    PROP_TITLE_PARA_LAST_LINE_ADJUST,
    PROP_TITLE_PARA_LEFT_MARGIN,
    PROP_TITLE_PARA_RIGHT_MARGIN,
    PROP_TITLE_PARA_TOP_MARGIN,
    PROP_TITLE_PARA_BOTTOM_MARGIN,
    PROP_TITLE_PARA_IS_HYPHENATION,

    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "ParaAdjust" ),
                  PROP_TITLE_PARA_ADJUST,
                  ::getCppuType( reinterpret_cast< ::com::sun::star::style::ParagraphAdjust * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaLastLineAdjust" ),
                  PROP_TITLE_PARA_LAST_LINE_ADJUST,
                  ::getCppuType( reinterpret_cast< sal_Int16 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaLeftMargin" ),
                  PROP_TITLE_PARA_LEFT_MARGIN,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaRightMargin" ),
                  PROP_TITLE_PARA_RIGHT_MARGIN,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaTopMargin" ),
                  PROP_TITLE_PARA_TOP_MARGIN,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaBottomMargin" ),
                  PROP_TITLE_PARA_BOTTOM_MARGIN,
                  ::getCppuType( reinterpret_cast< sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "ParaIsHyphenation" ),
                  PROP_TITLE_PARA_IS_HYPHENATION,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_TITLE_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const double * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "StackCharacters" ),
                  PROP_TITLE_TEXT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    // ParagraphProperties
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_ADJUST ));
    rOutMap[ PROP_TITLE_PARA_ADJUST ] =
        uno::makeAny( ::com::sun::star::style::ParagraphAdjust_CENTER );
    // PROP_TITLE_PARA_LAST_LINE_ADJUST

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_LEFT_MARGIN ));
    rOutMap[ PROP_TITLE_PARA_LEFT_MARGIN ] =
        uno::makeAny( sal_Int32( 0 ));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_RIGHT_MARGIN ));
    rOutMap[ PROP_TITLE_PARA_RIGHT_MARGIN ] =
        uno::makeAny( sal_Int32( 0 ));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_TOP_MARGIN ));
    rOutMap[ PROP_TITLE_PARA_TOP_MARGIN ] =
        uno::makeAny( sal_Int32( 0 ));
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_BOTTOM_MARGIN ));
    rOutMap[ PROP_TITLE_PARA_BOTTOM_MARGIN ] =
        uno::makeAny( sal_Int32( 0 ));

    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_PARA_IS_HYPHENATION ));
    rOutMap[ PROP_TITLE_PARA_IS_HYPHENATION ] =
        uno::makeAny( sal_True );

    // own properties
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_TEXT_ROTATION ));
    rOutMap[ PROP_TITLE_TEXT_ROTATION ] =
        uno::makeAny( double( 0.0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_TEXT_STACKED ));
    rOutMap[ PROP_TITLE_TEXT_STACKED ] =
        uno::makeAny( sal_Bool( sal_False ) );

    // override other defaults
    rOutMap[ ::chart::FillProperties::PROP_FILL_STYLE ] =
        uno::makeAny( drawing::FillStyle_NONE );
    rOutMap[ ::chart::LineProperties::PROP_LINE_STYLE ] =
        uno::makeAny( drawing::LineStyle_NONE );
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
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ true );
        ::chart::FillProperties::AddPropertiesToVector(
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

// ================================================================================

namespace chart
{

Title::Title( uno::Reference< uno::XComponentContext > const & xContext ) :
        ::property::OPropertySet( m_aMutex )
{
    if( ! ( xContext->getValueByName( C2U( "Identifier" )) >>= m_aIdentifier ))
    {
        OSL_ENSURE( false, "Missing Title identifier" );
    }
}

Title::~Title()
{}

// --------------------------------------------------------------------------------

// ____ XTitle ____
uno::Sequence< uno::Reference< chart2::XFormattedString > > SAL_CALL Title::getText()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return m_aStrings;
    // \--
}

void SAL_CALL Title::setText( const uno::Sequence< uno::Reference< chart2::XFormattedString > >& Strings )
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    m_aStrings = Strings;
    // \--
}

// ____ XIdentifiable ____
::rtl::OUString SAL_CALL Title::getIdentifier()
    throw (uno::RuntimeException)
{
    return m_aIdentifier;
}

// ================================================================================

// ____ OPropertySet ____
uno::Any Title::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static helper::tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );
        FillProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ true );

        // initialize defaults
        // Note: this should be last to override defaults of the previously
        // added defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    helper::tPropertyValueMap::const_iterator aFound(
        aStaticDefaults.find( nHandle ));

    if( aFound == aStaticDefaults.end())
        return uno::Any();

    return (*aFound).second;
    // \--
}

::cppu::IPropertyArrayHelper & SAL_CALL Title::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    Title::getPropertySetInfo()
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

uno::Sequence< ::rtl::OUString > Title::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "drafts.com.sun.star.chart2.Title" );
    aServices[ 1 ] = C2U( "com.sun.star.style.ParagraphProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 3 ] = C2U( "drafts.com.sun.star.layout.LayoutElement" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Title, lcl_aServiceName );

// needed by MSC compiler
using impl::Title_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Title, Title_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Title, Title_Base, ::property::OPropertySet )

} //  namespace chart
