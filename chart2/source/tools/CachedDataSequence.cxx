/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CachedDataSequence.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:15:58 $
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

#include "CachedDataSequence.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include "CommonFunctors.hxx"
#include "ModifyListenerHelper.hxx"

#include <comphelper/sequenceashashmap.hxx>

#include <algorithm>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::chart::ContainerHelper;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::osl::MutexGuard;

// necessary for MS compiler
using ::comphelper::OPropertyContainer;
using ::comphelper::OMutexAndBroadcastHelper;
using ::comphelper::OPropertyArrayUsageHelper;
using ::chart::impl::CachedDataSequence_Base;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.CachedDataSequence" ));

enum
{
//     PROP_SOURCE_IDENTIFIER,
    PROP_NUMBERFORMAT_KEY,
    PROP_PROPOSED_ROLE,
    PROP_HIDDEN,
    PROP_HIDDEN_VALUES
};
}  // anonymous namespace


// ____________________
namespace chart
{

CachedDataSequence::CachedDataSequence()
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( NUMERICAL ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    registerProperties();
}
CachedDataSequence::CachedDataSequence( const Reference< uno::XComponentContext > & /*xContext*/ )
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( MIXED ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder( ))
{
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const ::std::vector< double > & rVector )
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( NUMERICAL ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    m_aNumericalSequence = ContainerToSequence( rVector );
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const ::std::vector< OUString > & rVector )
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( TEXTUAL ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    m_aTextualSequence = ContainerToSequence( rVector );
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const OUString & rSingleText )
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( TEXTUAL ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    m_aTextualSequence.realloc(1);
    m_aTextualSequence[0] = rSingleText;
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const ::std::vector< Any > & rVector )
        : OPropertyContainer( GetBroadcastHelper()),
          CachedDataSequence_Base( GetMutex()),
          m_bIsHidden( true ),
          m_eCurrentDataType( MIXED ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    m_aMixedSequence = ContainerToSequence( rVector );
    registerProperties();
}

CachedDataSequence::CachedDataSequence( const CachedDataSequence & rSource )
        : OMutexAndBroadcastHelper(),
          OPropertyContainer( GetBroadcastHelper()),
          OPropertyArrayUsageHelper< CachedDataSequence >(),
          CachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey( rSource.m_nNumberFormatKey ),
          m_sRole( rSource.m_sRole ),
          m_bIsHidden( rSource.m_bIsHidden ),
          m_aHiddenValues( rSource.m_aHiddenValues ),
          m_eCurrentDataType( rSource.m_eCurrentDataType ),
          m_xModifyEventForwarder( new ModifyListenerHelper::ModifyEventForwarder())
{
    switch( m_eCurrentDataType )
    {
        case TEXTUAL:
            m_aTextualSequence = rSource.m_aTextualSequence;
            break;
        case NUMERICAL:
            m_aNumericalSequence = rSource.m_aNumericalSequence;
            break;
        case MIXED:
            m_aMixedSequence = rSource.m_aMixedSequence;
            break;
    }

    registerProperties();
}

CachedDataSequence::~CachedDataSequence()
{}

void CachedDataSequence::registerProperties()
{
    registerProperty( C2U( "NumberFormatKey" ),
                      PROP_NUMBERFORMAT_KEY,
                      0,   // PropertyAttributes
                      & m_nNumberFormatKey,
                      ::getCppuType( & m_nNumberFormatKey ) );

    registerProperty( C2U( "Role" ),
                      PROP_PROPOSED_ROLE,
                      0,   // PropertyAttributes
                      & m_sRole,
                      ::getCppuType( & m_sRole ) );

    registerProperty( C2U( "Hidden" ),
                      PROP_HIDDEN,
                      0,   // PropertyAttributes
                      & m_bIsHidden,
                      ::getCppuType( & m_bIsHidden ) );

    registerProperty( C2U( "HiddenValues" ),
                      PROP_HIDDEN_VALUES,
                      0,   // PropertyAttributes
                      & m_aHiddenValues,
                      ::getCppuType( & m_aHiddenValues ) );
}

void CachedDataSequence::PreferMixedData()
{
    if( m_eCurrentDataType != MIXED )
        Impl_setMixedData( Impl_getMixedData() );
}

void CachedDataSequence::PreferNumericalData()
{
    if( m_eCurrentDataType != NUMERICAL )
        Impl_setNumericalData( Impl_getNumericalData() );
}

void CachedDataSequence::PreferTextualData()
{
    if( m_eCurrentDataType != TEXTUAL )
        Impl_setTextualData( Impl_getTextualData() );
}

Sequence< double > CachedDataSequence::Impl_getNumericalData() const
{
    if( m_eCurrentDataType == NUMERICAL )
        return m_aNumericalSequence;

    sal_Int32 nSize = ( m_eCurrentDataType == TEXTUAL )
        ? m_aTextualSequence.getLength()
        : m_aMixedSequence.getLength();

    Sequence< double > aResult( nSize );
    double * pResultArray = aResult.getArray();

    if( m_eCurrentDataType == TEXTUAL )
    {
        const OUString * pTextArray = m_aTextualSequence.getConstArray();
        ::std::transform( pTextArray, pTextArray + nSize,
                          pResultArray,
                          CommonFunctors::OUStringToDouble() );
    }
    else
    {
        OSL_ASSERT( m_eCurrentDataType == MIXED );
        const Any * pMixedArray = m_aMixedSequence.getConstArray();
        ::std::transform( pMixedArray, pMixedArray + nSize,
                          pResultArray,
                          CommonFunctors::AnyToDouble() );
    }
    return aResult;
}

Sequence< OUString > CachedDataSequence::Impl_getTextualData() const
{
    if( m_eCurrentDataType == TEXTUAL )
        return m_aTextualSequence;

    sal_Int32 nSize = ( m_eCurrentDataType == NUMERICAL )
        ? m_aNumericalSequence.getLength()
        : m_aMixedSequence.getLength();

    Sequence< OUString > aResult( nSize );
    OUString * pResultArray = aResult.getArray();

    if( m_eCurrentDataType == NUMERICAL )
    {
        const double * pTextArray = m_aNumericalSequence.getConstArray();
        ::std::transform( pTextArray, pTextArray + nSize,
                          pResultArray,
                          CommonFunctors::DoubleToOUString() );
    }
    else
    {
        OSL_ASSERT( m_eCurrentDataType == MIXED );
        const Any * pMixedArray = m_aMixedSequence.getConstArray();
        ::std::transform( pMixedArray, pMixedArray + nSize,
                          pResultArray,
                          CommonFunctors::AnyToString() );
    }

    return aResult;
}

Sequence< Any > CachedDataSequence::Impl_getMixedData() const
{
    if( m_eCurrentDataType == MIXED )
        return m_aMixedSequence;

    sal_Int32 nSize = ( m_eCurrentDataType == NUMERICAL )
        ? m_aNumericalSequence.getLength()
        : m_aTextualSequence.getLength();

    Sequence< Any > aResult( nSize );
    Any * pResultArray = aResult.getArray();

    if( m_eCurrentDataType == NUMERICAL )
    {
        const double * pTextArray = m_aNumericalSequence.getConstArray();
        ::std::transform( pTextArray, pTextArray + nSize,
                          pResultArray,
                          CommonFunctors::makeAny< double >() );
    }
    else
    {
        OSL_ASSERT( m_eCurrentDataType == TEXTUAL );
        const OUString * pMixedArray = m_aTextualSequence.getConstArray();
        ::std::transform( pMixedArray, pMixedArray + nSize,
                          pResultArray,
                          CommonFunctors::makeAny< OUString >() );
    }

    return aResult;
}

void CachedDataSequence::Impl_setNumericalData( const Sequence< double > & rSeq )
{
    if( m_eCurrentDataType != NUMERICAL )
    {
        m_aTextualSequence.realloc( 0 );
        m_aMixedSequence.realloc( 0 );
        m_eCurrentDataType = NUMERICAL;
    }
    m_aNumericalSequence = rSeq;
}

void CachedDataSequence::Impl_setTextualData( const Sequence< OUString > & rSeq )
{
    if( m_eCurrentDataType != TEXTUAL )
    {
        m_aNumericalSequence.realloc( 0 );
        m_aMixedSequence.realloc( 0 );
        m_eCurrentDataType = TEXTUAL;
    }
    m_aTextualSequence = rSeq;
}

void CachedDataSequence::Impl_setMixedData( const Sequence< uno::Any > & rSeq )
{
    if( m_eCurrentDataType != MIXED )
    {
        m_aNumericalSequence.realloc( 0 );
        m_aTextualSequence.realloc( 0 );
        m_eCurrentDataType = MIXED;
    }
    m_aMixedSequence = rSeq;
}

// ================================================================================

Sequence< OUString > CachedDataSequence::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.data.DataSequence" );
    aServices[ 2 ] = C2U( "com.sun.star.chart2.data.NumericalDataSequence" );
    aServices[ 3 ] = C2U( "com.sun.star.chart2.data.TextualDataSequence" );
    return aServices;
}

IMPLEMENT_FORWARD_XINTERFACE2( CachedDataSequence, CachedDataSequence_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( CachedDataSequence, CachedDataSequence_Base, OPropertyContainer )

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL CachedDataSequence::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return Reference< beans::XPropertySetInfo >( createPropertySetInfo( getInfoHelper() ) );
}

// ____ ::comphelper::OPropertySetHelper ____
// __________________________________________
::cppu::IPropertyArrayHelper& CachedDataSequence::getInfoHelper()
{
    return *getArrayHelper();
}

// ____ ::comphelper::OPropertyArrayHelper ____
// ____________________________________________
::cppu::IPropertyArrayHelper* CachedDataSequence::createArrayHelper() const
{
    Sequence< beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( CachedDataSequence, lcl_aServiceName )

// ================================================================================

// ________ XNumericalDataSequence ________
Sequence< double > SAL_CALL CachedDataSequence::getNumericalData()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );

    if( m_eCurrentDataType == NUMERICAL )
        return m_aNumericalSequence;
    else
        return Impl_getNumericalData();
    // \--
}

// void SAL_CALL CachedDataSequence::setNumericalData( const Sequence< double >& aData )
//     throw (uno::RuntimeException)
// {
//     // /--
//     MutexGuard aGuard( GetMutex() );
//     Impl_setNumericalData( aData );
//     // \--
// }


// ________ XTextualDataSequence ________
Sequence< OUString > SAL_CALL CachedDataSequence::getTextualData()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );

    if( m_eCurrentDataType == TEXTUAL )
        return m_aTextualSequence;
    else
        return Impl_getTextualData();
    // \--
}

// void SAL_CALL CachedDataSequence::setTextualData( const Sequence< OUString >& aData )
//     throw (uno::RuntimeException)
// {
//     // /--
//     MutexGuard aGuard( GetMutex() );
//     Impl_setTextualData( aData );
//     // \--
// }

// ________ XDataSequence  ________
Sequence< Any > SAL_CALL CachedDataSequence::getData()
    throw (uno::RuntimeException)
{
    // /--
    MutexGuard aGuard( GetMutex() );
    return Impl_getMixedData();
    // \--
}

OUString SAL_CALL CachedDataSequence::getSourceRangeRepresentation()
    throw (uno::RuntimeException)
{
    return m_sRole;
}

Sequence< OUString > SAL_CALL CachedDataSequence::generateLabel( chart2::data::LabelOrigin  /*eLabelOrigin*/ )
    throw (uno::RuntimeException)
{
    // return empty label, as we have no range representaions to determine something useful
    return Sequence< OUString >();
}

::sal_Int32 SAL_CALL CachedDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 /*nIndex*/ )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    return 0;
}

Reference< util::XCloneable > SAL_CALL CachedDataSequence::createClone()
    throw (uno::RuntimeException)
{
    CachedDataSequence * pNewSeq = new CachedDataSequence( *this );

    return Reference< util::XCloneable >( pNewSeq );
}

void SAL_CALL CachedDataSequence::addModifyListener( const Reference< util::XModifyListener >& aListener )
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

void SAL_CALL CachedDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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

void CachedDataSequence::fireModifyEvent()
{
    // note: currently never called, as the data is not mutable
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}
// lang::XInitialization:
void SAL_CALL CachedDataSequence::initialize(const uno::Sequence< uno::Any > & _aArguments) throw (uno::RuntimeException, uno::Exception)
{
    ::comphelper::SequenceAsHashMap aMap(_aArguments);
    m_aNumericalSequence = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataSequence")),m_aNumericalSequence);
    if ( m_aNumericalSequence.getLength() )
        m_eCurrentDataType = NUMERICAL;
    else
    {
        m_aTextualSequence = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataSequence")),m_aTextualSequence);
        if ( m_aTextualSequence.getLength() )
            m_eCurrentDataType = TEXTUAL;
        else
        {
            m_aMixedSequence = aMap.getUnpackedValueOrDefault(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataSequence")),m_aMixedSequence);
            if ( m_aMixedSequence.getLength() )
                m_eCurrentDataType = MIXED;
        }
    }
}
}  // namespace chart
