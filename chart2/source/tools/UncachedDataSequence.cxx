/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "UncachedDataSequence.hxx"
#include "macros.hxx"
#include "PropertyHelper.hxx"
#include "CommonFunctors.hxx"
#include "ModifyListenerHelper.hxx"

#include <algorithm>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <rtl/math.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::osl::MutexGuard;

// necessary for MS compiler
using ::comphelper::OPropertyContainer;
using ::chart::impl::UncachedDataSequence_Base;

namespace
{
static const OUString lcl_aServiceName(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart.UncachedDataSequence" ));

enum
{
    PROP_NUMBERFORMAT_KEY,
    PROP_PROPOSED_ROLE,
    PROP_XML_RANGE
};
}  // anonymous namespace


// ____________________
namespace chart
{

UncachedDataSequence::UncachedDataSequence(
    const Reference< chart2::XInternalDataProvider > & xIntDataProv,
    const OUString & rRangeRepresentation )
        : OPropertyContainer( GetBroadcastHelper()),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey(0),
          m_xDataProvider( xIntDataProv ),
          m_aSourceRepresentation( rRangeRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
}

UncachedDataSequence::UncachedDataSequence(
    const Reference< chart2::XInternalDataProvider > & xIntDataProv,
    const OUString & rRangeRepresentation,
    const OUString & rRole )
        : OPropertyContainer( GetBroadcastHelper()),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey(0),
          m_xDataProvider( xIntDataProv ),
          m_aSourceRepresentation( rRangeRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
    setFastPropertyValue_NoBroadcast( PROP_PROPOSED_ROLE, uno::makeAny( rRole ));
}

UncachedDataSequence::UncachedDataSequence( const UncachedDataSequence & rSource )
        : ::comphelper::OMutexAndBroadcastHelper(),
          OPropertyContainer( GetBroadcastHelper()),
          ::comphelper::OPropertyArrayUsageHelper< UncachedDataSequence >(),
          UncachedDataSequence_Base( GetMutex()),
          m_nNumberFormatKey( rSource.m_nNumberFormatKey ),
          m_sRole( rSource.m_sRole ),
          m_xDataProvider( rSource.m_xDataProvider ),
          m_aSourceRepresentation( rSource.m_aSourceRepresentation ),
          m_xModifyEventForwarder( ModifyListenerHelper::createModifyEventForwarder())
{
    registerProperties();
}

UncachedDataSequence::~UncachedDataSequence()
{}

void UncachedDataSequence::registerProperties()
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

    registerProperty( C2U( "CachedXMLRange" ),
                      PROP_XML_RANGE,
                      0,   // PropertyAttributes
                      & m_aXMLRange,
                      ::getCppuType( & m_aXMLRange ) );
}

// ================================================================================

Sequence< OUString > UncachedDataSequence::getSupportedServiceNames_Static()
{
    Sequence< OUString > aServices( 4 );
    aServices[ 0 ] = lcl_aServiceName;
    aServices[ 1 ] = C2U( "com.sun.star.chart2.data.DataSequence" );
    aServices[ 2 ] = C2U( "com.sun.star.chart2.data.NumericalDataSequence" );
    aServices[ 3 ] = C2U( "com.sun.star.chart2.data.TextualDataSequence" );
    return aServices;
}

IMPLEMENT_FORWARD_XINTERFACE2( UncachedDataSequence, UncachedDataSequence_Base, OPropertyContainer )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( UncachedDataSequence, UncachedDataSequence_Base, OPropertyContainer )

// ____ XPropertySet ____
Reference< beans::XPropertySetInfo > SAL_CALL UncachedDataSequence::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return Reference< beans::XPropertySetInfo >( createPropertySetInfo( getInfoHelper() ) );
}

// ____ ::comphelper::OPropertySetHelper ____
// __________________________________________
::cppu::IPropertyArrayHelper& UncachedDataSequence::getInfoHelper()
{
    return *getArrayHelper();
}

// ____ ::comphelper::OPropertyArrayHelper ____
// ____________________________________________
::cppu::IPropertyArrayHelper* UncachedDataSequence::createArrayHelper() const
{
    Sequence< beans::Property > aProps;
    // describes all properties which have been registered in the ctor
    describeProperties( aProps );

    return new ::cppu::OPropertyArrayHelper( aProps );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( UncachedDataSequence, lcl_aServiceName )

// ================================================================================

// ________ XNumericalDataSequence ________
Sequence< double > SAL_CALL UncachedDataSequence::getNumericalData()
    throw (uno::RuntimeException)
{
    Sequence< double > aResult;
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
    {
        Sequence< uno::Any > aValues( m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation ));
        aResult.realloc( aValues.getLength());
        ::std::transform( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
                          aResult.getArray(), CommonFunctors::AnyToDouble());
    }
    return aResult;
}

// ________ XTextualDataSequence ________
Sequence< OUString > SAL_CALL UncachedDataSequence::getTextualData()
    throw (uno::RuntimeException)
{
    Sequence< OUString > aResult;
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
    {
        Sequence< uno::Any > aValues( m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation ));
        aResult.realloc( aValues.getLength());
        ::std::transform( aValues.getConstArray(), aValues.getConstArray() + aValues.getLength(),
                          aResult.getArray(), CommonFunctors::AnyToString());
    }
    return aResult;
}

// ________ XDataSequence  ________
Sequence< Any > SAL_CALL UncachedDataSequence::getData()
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    if( m_xDataProvider.is())
        return m_xDataProvider->getDataByRangeRepresentation( m_aSourceRepresentation );
    return Sequence< Any >();
}

OUString SAL_CALL UncachedDataSequence::getSourceRangeRepresentation()
    throw (uno::RuntimeException)
{
    return getName();
}


Sequence< OUString > SAL_CALL UncachedDataSequence::generateLabel( chart2::data::LabelOrigin )
    throw (uno::RuntimeException)
{
    // auto-generated label is an empty string
    static const Sequence< OUString > aOneEmptyString( 1 );
    return aOneEmptyString;
}

::sal_Int32 SAL_CALL UncachedDataSequence::getNumberFormatKeyByIndex( ::sal_Int32 )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    return m_nNumberFormatKey;
}

// ____ XIndexReplace ____
void SAL_CALL UncachedDataSequence::replaceByIndex( ::sal_Int32 Index, const uno::Any& Element )
    throw (lang::IllegalArgumentException,
           lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    MutexGuard aGuard( GetMutex() );
    Sequence< Any > aData( getData());
    if( Index < aData.getLength() &&
        m_xDataProvider.is() )
    {
        aData[Index] = Element;
        m_xDataProvider->setDataByRangeRepresentation( m_aSourceRepresentation, aData );
        fireModifyEvent();
    }
}

// ____ XIndexAccess (base of XIndexReplace) ____
::sal_Int32 SAL_CALL UncachedDataSequence::getCount()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Implement!" );
    return 0;
}

uno::Any SAL_CALL UncachedDataSequence::getByIndex( ::sal_Int32 )
    throw (lang::IndexOutOfBoundsException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    OSL_FAIL( "Implement!" );
    return uno::Any();
}

// ____ XElementAccess (base of XIndexAccess) ____
uno::Type SAL_CALL UncachedDataSequence::getElementType()
    throw (uno::RuntimeException)
{
    return ::getCppuType( reinterpret_cast< uno::Any * >(0));
}

::sal_Bool SAL_CALL UncachedDataSequence::hasElements()
    throw (uno::RuntimeException)
{
    if( ! m_xDataProvider.is())
        return sal_False;
    return m_xDataProvider->hasDataByRangeRepresentation( m_aSourceRepresentation );
}

// ____ XNamed ____
::rtl::OUString SAL_CALL UncachedDataSequence::getName()
    throw (uno::RuntimeException)
{
    return m_aSourceRepresentation;
}

void SAL_CALL UncachedDataSequence::setName( const OUString& aName )
    throw (uno::RuntimeException)
{
    m_aSourceRepresentation = aName;
    fireModifyEvent();
}



Reference< util::XCloneable > SAL_CALL UncachedDataSequence::createClone()
    throw (uno::RuntimeException)
{
    UncachedDataSequence * pNewSeq = new UncachedDataSequence( *this );
    return Reference< util::XCloneable >( pNewSeq );
}


// ____ XModifiable ____
::sal_Bool SAL_CALL UncachedDataSequence::isModified()
    throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL UncachedDataSequence::setModified( ::sal_Bool bModified )
    throw (beans::PropertyVetoException,
           uno::RuntimeException)
{
    if( bModified )
        fireModifyEvent();
}

// ____ XModifyBroadcaster (base of XModifiable) ____
void SAL_CALL UncachedDataSequence::addModifyListener( const Reference< util::XModifyListener >& aListener )
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

void SAL_CALL UncachedDataSequence::removeModifyListener( const Reference< util::XModifyListener >& aListener )
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

void UncachedDataSequence::fireModifyEvent()
{
    // @todo: currently never called, as data changes are not yet reported by
    // the data provider
    m_xModifyEventForwarder->modified( lang::EventObject( static_cast< uno::XWeak* >( this )));
}

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
