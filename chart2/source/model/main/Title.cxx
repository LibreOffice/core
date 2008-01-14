/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Title.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:02:21 $
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
#include "Title.hxx"
#include "macros.hxx"
#include "FormattedString.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "ContainerHelper.hxx"
#include "CloneHelper.hxx"

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
#ifndef _COM_SUN_STAR_CHART2_RELATIVEPOSITION_HPP_
#include <com/sun/star/chart2/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
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
    PROP_TITLE_TEXT_STACKED,
    PROP_TITLE_REL_POS,

    PROP_TITLE_REF_PAGE_SIZE
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

    rOutProperties.push_back(
        Property( C2U( "RelativePosition" ),
                  PROP_TITLE_REL_POS,
                  ::getCppuType( reinterpret_cast< const chart2::RelativePosition * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "ReferencePageSize" ),
                  PROP_TITLE_REF_PAGE_SIZE,
                  ::getCppuType( reinterpret_cast< const awt::Size * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

void lcl_AddDefaultsToMap(
    ::chart::tPropertyValueMap & rOutMap )
{
    // ParagraphProperties
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_PARA_ADJUST,
                                                      ::com::sun::star::style::ParagraphAdjust_CENTER );
    // PROP_TITLE_PARA_LAST_LINE_ADJUST

    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_LEFT_MARGIN, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_RIGHT_MARGIN, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_TOP_MARGIN, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault< sal_Int32 >( rOutMap, PROP_TITLE_PARA_BOTTOM_MARGIN, 0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_PARA_IS_HYPHENATION, true );

    // own properties
    ::chart::PropertyHelper::setPropertyValueDefault< double >( rOutMap, PROP_TITLE_TEXT_ROTATION, 0.0 );
    ::chart::PropertyHelper::setPropertyValueDefault( rOutMap, PROP_TITLE_TEXT_STACKED, false );

    // override other defaults
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::FillProperties::PROP_FILL_STYLE, drawing::FillStyle_NONE );
    ::chart::PropertyHelper::setPropertyValue( rOutMap, ::chart::LineProperties::PROP_LINE_STYLE, drawing::LineStyle_NONE );
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
        ::chart::LineProperties::AddPropertiesToVector( aProperties );
        ::chart::FillProperties::AddPropertiesToVector( aProperties );

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

// ================================================================================

namespace chart
{

Title::Title( uno::Reference< uno::XComponentContext > const & /* xContext */ ) :
        ::property::OPropertySet( m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{}

Title::Title( const Title & rOther ) :
        MutexContainer(),
        impl::Title_Base(),
        ::property::OPropertySet( rOther, m_aMutex ),
        m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    CloneHelper::CloneRefSequence< uno::Reference< chart2::XFormattedString > >(
        rOther.m_aStrings, m_aStrings );
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
}

Title::~Title()
{
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
}

// ____ XCloneable ____
uno::Reference< util::XCloneable > SAL_CALL Title::createClone()
    throw (uno::RuntimeException)
{
    return uno::Reference< util::XCloneable >( new Title( *this ));
}

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
    ModifyListenerHelper::removeListenerFromAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    m_aStrings = Strings;
    ModifyListenerHelper::addListenerToAllElements(
        ContainerHelper::SequenceToVector( m_aStrings ), m_xModifyEventForwarder );
    fireModifyEvent();
    // \--
}

// ================================================================================

// ____ OPropertySet ____
uno::Any Title::GetDefaultValue( sal_Int32 nHandle ) const
    throw(beans::UnknownPropertyException)
{
    static tPropertyValueMap aStaticDefaults;

    // /--
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( 0 == aStaticDefaults.size() )
    {
        LineProperties::AddDefaultsToMap( aStaticDefaults );
        FillProperties::AddDefaultsToMap( aStaticDefaults );

        // initialize defaults
        // Note: this should be last to override defaults of the previously
        // added defaults
        lcl_AddDefaultsToMap( aStaticDefaults );
    }

    tPropertyValueMap::const_iterator aFound(
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

// ____ XModifyBroadcaster ____
void SAL_CALL Title::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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

void SAL_CALL Title::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
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
void SAL_CALL Title::modified( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    m_xModifyEventForwarder->modified( aEvent );
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL Title::disposing( const lang::EventObject& /* Source */ )
    throw (uno::RuntimeException)
{
    // nothing
}

// ____ OPropertySet ____
void Title::firePropertyChangeEvent()
{
    fireModifyEvent();
}

void Title::fireModifyEvent()
{
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}


// ================================================================================

uno::Sequence< ::rtl::OUString > Title::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart2.Title" );
    aServices[ 1 ] = C2U( "com.sun.star.style.ParagraphProperties" );
    aServices[ 2 ] = C2U( "com.sun.star.beans.PropertySet" );
    aServices[ 3 ] = C2U( "com.sun.star.layout.LayoutElement" );
    return aServices;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( Title, lcl_aServiceName );

// needed by MSC compiler
using impl::Title_Base;

IMPLEMENT_FORWARD_XINTERFACE2( Title, Title_Base, ::property::OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( Title, Title_Base, ::property::OPropertySet )

} //  namespace chart
