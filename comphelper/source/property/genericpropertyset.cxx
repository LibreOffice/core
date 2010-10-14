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
#include "precompiled_comphelper.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/propertysethelper.hxx>
#include <osl/mutex.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/stl_types.hxx>
#include <vos/mutex.hxx>
#include <rtl/uuid.h>
#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>

///////////////////////////////////////////////////////////////////////

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

DECLARE_STL_USTRINGACCESS_MAP( Any, GenericAnyMapImpl );

namespace comphelper
{
    struct IMPL_GenericPropertySet_MutexContainer
    {
        Mutex maMutex ;
    } ;

    class GenericPropertySet :  public OWeakAggObject,
                                public XServiceInfo,
                                public XTypeProvider,
                                public PropertySetHelper,
                                private IMPL_GenericPropertySet_MutexContainer
    {
    private:
        GenericAnyMapImpl   maAnyMap;
        ::cppu::OMultiTypeInterfaceContainerHelperVar< ::rtl::OUString,UStringHash,UStringEqual> m_aListener;

    protected:
        virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const  Any* pValues ) throw( UnknownPropertyException,  PropertyVetoException,  IllegalArgumentException,  WrappedTargetException );
        virtual void _getPropertyValues( const PropertyMapEntry** ppEntries,  Any* pValue ) throw( UnknownPropertyException,  WrappedTargetException );

    public:
        GenericPropertySet( PropertySetInfo* pInfo ) throw();
        virtual ~GenericPropertySet() throw();

        // XInterface
        virtual  Any SAL_CALL queryAggregation( const  Type & rType ) throw( RuntimeException);
        virtual  Any SAL_CALL queryInterface( const  Type & rType ) throw( RuntimeException);
        virtual void SAL_CALL acquire() throw();
        virtual void SAL_CALL release() throw();

        // XTypeProvider
        virtual  Sequence<  Type > SAL_CALL getTypes(  ) throw( RuntimeException);
        virtual  Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw( RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName() throw(  RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName ) throw(  RuntimeException );
        virtual  Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw(  RuntimeException );

        // XPropertySet
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    };

}

///////////////////////////////////////////////////////////////////////

GenericPropertySet::GenericPropertySet( PropertySetInfo* pInfo ) throw()
: PropertySetHelper( pInfo )
,m_aListener(maMutex)
{
}

GenericPropertySet::~GenericPropertySet() throw()
{
}
void SAL_CALL GenericPropertySet::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    if ( xInfo.is() )
    {
        if ( !aPropertyName.getLength() )
        {
            Sequence< Property> aSeq = xInfo->getProperties();
            const Property* pIter = aSeq.getConstArray();
            const Property* pEnd  = pIter + aSeq.getLength();
            for( ; pIter != pEnd ; ++pIter)
            {
                m_aListener.addInterface(pIter->Name,xListener);
            }
        }
        else if ( xInfo->hasPropertyByName(aPropertyName) )
            m_aListener.addInterface(aPropertyName,xListener);
        else
            throw UnknownPropertyException( aPropertyName, *this );
    }
}

void SAL_CALL GenericPropertySet::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    ResettableMutexGuard aGuard( maMutex );
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    aGuard.clear();
    if ( xInfo.is() )
    {
        if ( !aPropertyName.getLength() )
        {
            Sequence< Property> aSeq = xInfo->getProperties();
            const Property* pIter = aSeq.getConstArray();
            const Property* pEnd  = pIter + aSeq.getLength();
            for( ; pIter != pEnd ; ++pIter)
            {
                m_aListener.removeInterface(pIter->Name,xListener);
            }
        }
        else if ( xInfo->hasPropertyByName(aPropertyName) )
            m_aListener.removeInterface(aPropertyName,xListener);
        else
            throw UnknownPropertyException( aPropertyName, *this );
    }
}

void GenericPropertySet::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
    throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException )
{
    ResettableMutexGuard aGuard( maMutex );

    while( *ppEntries )
    {
        const OUString aPropertyName( (*ppEntries)->mpName, (*ppEntries)->mnNameLen, RTL_TEXTENCODING_ASCII_US );
        OInterfaceContainerHelper * pHelper = m_aListener.getContainer(aPropertyName);

        maAnyMap[ aPropertyName ] = *pValues;

        if ( pHelper )
        {
            PropertyChangeEvent aEvt;
            aEvt.PropertyName = aPropertyName;
            aEvt.NewValue = *pValues;
            aGuard.clear();
            pHelper->notifyEach( &XPropertyChangeListener::propertyChange, aEvt );
            aGuard.reset();
        }

        ppEntries++;
        pValues++;
    }
}

void GenericPropertySet::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, Any* pValue )
    throw( UnknownPropertyException, WrappedTargetException )
{
    MutexGuard aGuard( maMutex );

    while( *ppEntries )
    {
        const OUString aPropertyName( (*ppEntries)->mpName, (*ppEntries)->mnNameLen, RTL_TEXTENCODING_ASCII_US );
        *pValue = maAnyMap[ aPropertyName ];

        ppEntries++;
        pValue++;
    }
}

// XInterface

Any SAL_CALL GenericPropertySet::queryInterface( const Type & rType )
    throw( RuntimeException )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL GenericPropertySet::queryAggregation( const Type & rType )
    throw(RuntimeException)
{
    Any aAny;

    if( rType == ::getCppuType((const Reference< XServiceInfo >*)0) )
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == ::getCppuType((const Reference< XTypeProvider >*)0) )
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == ::getCppuType((const Reference< XPropertySet >*)0) )
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == ::getCppuType((const Reference< XMultiPropertySet >*)0) )
        aAny <<= Reference< XMultiPropertySet >(this);
    else
        aAny <<= OWeakAggObject::queryAggregation( rType );

    return aAny;
}

void SAL_CALL GenericPropertySet::acquire() throw()
{
    OWeakAggObject::acquire();
}

void SAL_CALL GenericPropertySet::release() throw()
{
    OWeakAggObject::release();
}

uno::Sequence< uno::Type > SAL_CALL GenericPropertySet::getTypes()
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( 5 );
    uno::Type* pTypes = aTypes.getArray();

    *pTypes++ = ::getCppuType((const uno::Reference< XAggregation>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XServiceInfo>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XTypeProvider>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XPropertySet>*)0);
    *pTypes++ = ::getCppuType((const uno::Reference< XMultiPropertySet>*)0);

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL GenericPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    MutexGuard aGuard( maMutex );

    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

// XServiceInfo

sal_Bool SAL_CALL GenericPropertySet::supportsService( const  OUString& ServiceName ) throw(RuntimeException)
{
    Sequence< OUString > aSNL( getSupportedServiceNames() );
    const OUString * pArray = aSNL.getConstArray();

    for( sal_Int32 i = 0; i < aSNL.getLength(); ++i )
        if( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

OUString SAL_CALL GenericPropertySet::getImplementationName() throw( RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.comphelper.GenericPropertySet") );
}

Sequence< OUString > SAL_CALL GenericPropertySet::getSupportedServiceNames(  )
    throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.beans.XPropertySet" ));
    return aSNS;
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > comphelper::GenericPropertySet_CreateInstance( comphelper::PropertySetInfo* pInfo )
{
    return (XPropertySet*)new GenericPropertySet( pInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
