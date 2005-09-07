/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:02:22 $
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
#include "TitleWrapper.hxx"
#include "macros.hxx"
#include "InlineContainer.hxx"
#include "algohelper.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include "CharacterProperties.hxx"
#include "LineProperties.hxx"
#include "FillProperties.hxx"
#include "UserDefinedProperties.hxx"

#include <algorithm>

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::beans::Property;
using ::osl::MutexGuard;
using ::property::OPropertySet;

namespace
{
static const ::rtl::OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.Title" ));

enum
{
    PROP_TITLE_STRING,
    PROP_TITLE_TEXT_ROTATION,
    PROP_TITLE_TEXT_STACKED
};

typedef ::std::map< sal_Int32, ::rtl::OUString > lcl_PropertyMapType;
typedef ::comphelper::MakeMap< sal_Int32, ::rtl::OUString > lcl_MakePropertyMapType;

lcl_PropertyMapType & lcl_GetPropertyMap()
{
    static lcl_PropertyMapType aMap(
        lcl_MakePropertyMapType
        ( PROP_TITLE_TEXT_ROTATION,           C2U( "TextRotation" ))
        ( PROP_TITLE_TEXT_STACKED,            C2U( "StackCharacters" ))
        );

    return aMap;
};

void lcl_AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( C2U( "String" ),
                  PROP_TITLE_STRING,
                  ::getCppuType( reinterpret_cast< const ::rtl::OUString * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));

    rOutProperties.push_back(
        Property( C2U( "TextRotation" ),
                  PROP_TITLE_TEXT_ROTATION,
                  ::getCppuType( reinterpret_cast< const sal_Int32 * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
    rOutProperties.push_back(
        Property( C2U( "StackedText" ),
                  PROP_TITLE_TEXT_STACKED,
                  ::getBooleanCppuType(),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEDEFAULT ));
}

void lcl_AddDefaultsToMap(
    ::chart::helper::tPropertyValueMap & rOutMap )
{
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_TEXT_ROTATION ));
    rOutMap[ PROP_TITLE_TEXT_ROTATION ] =
        uno::makeAny( sal_Int32( 0 ) );
    OSL_ASSERT( rOutMap.end() == rOutMap.find( PROP_TITLE_TEXT_STACKED ));
    rOutMap[ PROP_TITLE_TEXT_STACKED ] =
        uno::makeAny( sal_Bool( sal_False ) );
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
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::LineProperties::AddPropertiesToVector(
            aProperties, /* bIncludeStyleProperties = */ false );
        ::chart::FillProperties::AddPropertiesToVector(
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

// --------------------------------------------------------------------------------

namespace chart
{
namespace wrapper
{

TitleWrapper::TitleWrapper(
    const uno::Reference<
        ::com::sun::star::chart2::XTitle > & xTitle,
    const uno::Reference< uno::XComponentContext > & xContext,
    ::osl::Mutex & _rMutex ) :
        OPropertySet( _rMutex ),
        m_rMutex( _rMutex ),
        m_xContext( xContext ),
        m_aEventListenerContainer( _rMutex ),
        m_xTitle( xTitle )
{
    if( m_xTitle.is())
    {
        m_xTitleProp.set( m_xTitle, uno::UNO_QUERY );
        m_xTitleFastProp.set( m_xTitle, uno::UNO_QUERY );
    }
}

TitleWrapper::~TitleWrapper()
{}

::osl::Mutex & TitleWrapper::GetMutex() const
{
    return m_rMutex;
}

// ____ XShape ____
awt::Point SAL_CALL TitleWrapper::getPosition()
    throw (uno::RuntimeException)
{
    return awt::Point( 0, 0 );
}

void SAL_CALL TitleWrapper::setPosition( const awt::Point& aPosition )
    throw (uno::RuntimeException)
{
}

awt::Size SAL_CALL TitleWrapper::getSize()
    throw (uno::RuntimeException)
{
    return awt::Size( 0, 0 );
}

void SAL_CALL TitleWrapper::setSize( const awt::Size& aSize )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    OSL_ENSURE( false, "trying to set size of title" );
}

// ____ XShapeDescriptor (base of XShape) ____
::rtl::OUString SAL_CALL TitleWrapper::getShapeType()
    throw (uno::RuntimeException)
{
    return C2U( "com.sun.star.chart.ChartTitle" );
}

// ____ XComponent ____
void SAL_CALL TitleWrapper::dispose()
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.disposeAndClear( lang::EventObject( *this ) );

    // /--
    MutexGuard aGuard( GetMutex());
    m_xTitle = NULL;
    m_xTitleProp = NULL;
    m_xTitleFastProp = NULL;
    // \--
}

void SAL_CALL TitleWrapper::addEventListener(
    const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.addInterface( xListener );
}

void SAL_CALL TitleWrapper::removeEventListener(
    const uno::Reference< lang::XEventListener >& aListener )
    throw (uno::RuntimeException)
{
    m_aEventListenerContainer.removeInterface( aListener );
}

sal_Bool SAL_CALL TitleWrapper::convertFastPropertyValue
    ( uno::Any & rConvertedValue,
      uno::Any & rOldValue,
      sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (lang::IllegalArgumentException)
{
    // /--
    MutexGuard aGuard( GetMutex());
    switch( nHandle )
    {
        case PROP_TITLE_TEXT_ROTATION:
        {
            getFastPropertyValue( rOldValue, nHandle );

            sal_Int32 nVal;
            if( rValue >>= nVal )
            {
                double fDoubleDegrees = ( static_cast< double >( nVal ) / 100.0 );
                rConvertedValue <<= fDoubleDegrees;
                return sal_True;
            }
            break;
        }

        default:
            return OPropertySet::convertFastPropertyValue(
                rConvertedValue, rOldValue, nHandle, rValue );
    }

    rConvertedValue = rValue;

    return sal_True;
    // \--
}

void SAL_CALL TitleWrapper::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle,
      const uno::Any& rValue )
    throw (uno::Exception)
{
    // /--
    MutexGuard aGuard( GetMutex());

    if( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
        nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP )
    {
        setFastCharacterPropertyValue( nHandle, rValue );
    }
    // try same handle for FastPropertySet.  Caution!  Works for global
    // properties like FillProperties, LineProperties and CharacterProperties
    else if( nHandle > FAST_PROPERTY_ID_START )
    {
        if( m_xTitleFastProp.is() )
            m_xTitleFastProp->setFastPropertyValue( nHandle, rValue );
    }
    else
    {
        switch( nHandle )
        {
            case PROP_TITLE_STRING:
                if( m_xTitle.is())
                {
                    ::rtl::OUString aString;

                    if( rValue >>= aString )
                    {
                        uno::Sequence<
                            uno::Reference<
                            ::com::sun::star::chart2::XFormattedString > >
                            aStrings( 1 );

                        aStrings[ 0 ].set(
                            m_xContext->getServiceManager()->createInstanceWithContext(
                                C2U( "com.sun.star.chart2.FormattedString" ),
                                m_xContext ),
                            uno::UNO_QUERY );

                        if( aStrings[ 0 ].is())
                            aStrings[ 0 ]->setString( aString );

                        m_xTitle->setText( aStrings );
                    }
                    else
                    {
                        throw lang::IllegalArgumentException(
                            C2U( "Type Mismatch for property String" ),
                            static_cast< ::cppu::OWeakObject * >( this ),
                            1 );
                    }
                }
                break;

            case PROP_TITLE_TEXT_ROTATION:
            case PROP_TITLE_TEXT_STACKED:
                if( m_xTitleProp.is())
                {
                    lcl_PropertyMapType & rMap( lcl_GetPropertyMap());
                    lcl_PropertyMapType::const_iterator aIt( rMap.find( nHandle ));

                    if( aIt != rMap.end())
                    {
                        // found in map
                        m_xTitleProp->setPropertyValue( (*aIt).second, rValue );
                    }
                }
                break;
        }
    }
    // \--
}

void SAL_CALL TitleWrapper::getFastPropertyValue
    ( uno::Any& rValue,
      sal_Int32 nHandle ) const
{
    // /--
    MutexGuard aGuard( GetMutex());
    if( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
        nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP )
    {
        getFastCharacterPropertyValue( rValue, nHandle );
    }
    else
    {
        switch( nHandle )
        {
            case PROP_TITLE_STRING:
                if( m_xTitle.is())
                {
                    uno::Sequence<
                        uno::Reference<
                        ::com::sun::star::chart2::XFormattedString > >
                        aStrings( m_xTitle->getText());

                    ::rtl::OUStringBuffer aBuf;
                    for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
                    {
                        aBuf.append( aStrings[ i ]->getString());
                    }
                    rValue <<= aBuf.makeStringAndClear();
                }
                break;

            default:
                if( m_xTitleFastProp.is())
                {
                    // Note: handles must be identical ! (Is the case with the
                    // shared helpers for Character-, Line- and FillProperties
                    rValue = m_xTitleFastProp->getFastPropertyValue( nHandle );
                }
                else
                    OPropertySet::getFastPropertyValue( rValue, nHandle );
                break;
        }
    }
    // \--
}

void TitleWrapper::getFastCharacterPropertyValue(
    uno::Any& rValue,
    sal_Int32 nHandle ) const
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    uno::Reference< beans::XFastPropertySet > xProp;

    if( m_xTitle.is())
    {
        uno::Sequence<
            uno::Reference<
            ::com::sun::star::chart2::XFormattedString > >
            aStrings( m_xTitle->getText());

        if( aStrings.getLength() > 0 )
        {
            xProp.set( aStrings[0], uno::UNO_QUERY );
        }
    }

    // Note: handles must be identical ! (Is the case with the
    // shared helpers for Character-, Line- and FillProperties
    if( xProp.is())
        rValue = xProp->getFastPropertyValue( nHandle );
}

void TitleWrapper::setFastCharacterPropertyValue(
    sal_Int32 nHandle,
    const uno::Any& rValue )
    throw (uno::Exception)
{
    OSL_ASSERT( FAST_PROPERTY_ID_START_CHAR_PROP <= nHandle &&
                nHandle < CharacterProperties::FAST_PROPERTY_ID_END_CHAR_PROP );

    if( m_xTitle.is())
    {
        uno::Sequence<
            uno::Reference<
            ::com::sun::star::chart2::XFormattedString > >
            aStrings( m_xTitle->getText());

        for( sal_Int32 i = 0; i < aStrings.getLength(); ++i )
        {
            uno::Reference< beans::XFastPropertySet > xStringProp( aStrings[ i ], uno::UNO_QUERY );
            if( xStringProp.is())
            {
                // Note: handles must be identical ! (Is the case with the
                // shared helpers for Character-, Line- and FillProperties
                xStringProp->setFastPropertyValue( nHandle, rValue );
            }
        }
    }
}

// --------------------------------------------------------------------------------

// ____ OPropertySet ____
uno::Any TitleWrapper::GetDefaultValue( sal_Int32 nHandle ) const
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
            /* bIncludeStyleProperties = */ false );
        LineProperties::AddDefaultsToMap(
            aStaticDefaults,
            /* bIncludeStyleProperties = */ false );
        FillProperties::AddDefaultsToMap(
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

::cppu::IPropertyArrayHelper & SAL_CALL TitleWrapper::getInfoHelper()
{
    return lcl_getInfoHelper();
}


// ____ XPropertySet ____
uno::Reference< beans::XPropertySetInfo > SAL_CALL
    TitleWrapper::getPropertySetInfo()
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

uno::Sequence< ::rtl::OUString > TitleWrapper::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aServices( 4 );
    aServices[ 0 ] = C2U( "com.sun.star.chart.ChartTitle" );
    aServices[ 1 ] = C2U( "com.sun.star.drawing.Shape" );
    aServices[ 2 ] = C2U( "com.sun.star.xml.UserDefinedAttributeSupplier" );
    aServices[ 3 ] = C2U( "com.sun.star.style.CharacterProperties" );
//     aServices[ 4 ] = C2U( "com.sun.star.beans.PropertySet" );
//     aServices[ 5 ] = C2U( "com.sun.star.drawing.FillProperties" );
//     aServices[ 6 ] = C2U( "com.sun.star.drawing.LineProperties" );

    return aServices;
}

// ================================================================================

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( TitleWrapper, lcl_aServiceName );

// needed by MSC compiler
using impl::TitleWrapper_Base;

IMPLEMENT_FORWARD_XINTERFACE2( TitleWrapper, TitleWrapper_Base, OPropertySet )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( TitleWrapper, TitleWrapper_Base, OPropertySet )

} //  namespace wrapper
} //  namespace chart
