/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <map>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertysethelper.hxx>
#include <osl/mutex.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::osl;
using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

typedef std::map<OUString, Any> GenericAnyMapImpl;

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
        cppu::OMultiTypeInterfaceContainerHelperVar<OUString> m_aListener;

    protected:
        virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const  Any* pValues ) throw( UnknownPropertyException,  PropertyVetoException,  IllegalArgumentException,  WrappedTargetException ) SAL_OVERRIDE;
        virtual void _getPropertyValues( const PropertyMapEntry** ppEntries,  Any* pValue ) throw( UnknownPropertyException,  WrappedTargetException ) SAL_OVERRIDE;

    public:
        GenericPropertySet( PropertySetInfo* pInfo ) throw();
        virtual ~GenericPropertySet() throw();

        // XInterface
        virtual  Any SAL_CALL queryAggregation( const  Type & rType ) throw( RuntimeException, std::exception) SAL_OVERRIDE;
        virtual  Any SAL_CALL queryInterface( const  Type & rType ) throw( RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL acquire() throw() SAL_OVERRIDE;
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;

        // XTypeProvider
        virtual  Sequence<  Type > SAL_CALL getTypes(  ) throw( RuntimeException, std::exception) SAL_OVERRIDE;
        virtual  Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw( RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(  RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(  RuntimeException, std::exception ) SAL_OVERRIDE;
        virtual  Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(  RuntimeException, std::exception ) SAL_OVERRIDE;

        // XPropertySet
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

}



GenericPropertySet::GenericPropertySet( PropertySetInfo* pInfo ) throw()
: PropertySetHelper( pInfo )
,m_aListener(maMutex)
{
}

GenericPropertySet::~GenericPropertySet() throw()
{
}
void SAL_CALL GenericPropertySet::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    if ( xInfo.is() )
    {
        if ( aPropertyName.isEmpty() )
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

void SAL_CALL GenericPropertySet::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener ) throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    ResettableMutexGuard aGuard( maMutex );
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    aGuard.clear();
    if ( xInfo.is() )
    {
        if ( aPropertyName.isEmpty() )
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
        OInterfaceContainerHelper * pHelper = m_aListener.getContainer((*ppEntries)->maName);

        maAnyMap[ (*ppEntries)->maName ] = *pValues;

        if ( pHelper )
        {
            PropertyChangeEvent aEvt;
            aEvt.PropertyName = (*ppEntries)->maName;
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
        *pValue = maAnyMap[ (*ppEntries)->maName ];

        ppEntries++;
        pValue++;
    }
}

// XInterface

Any SAL_CALL GenericPropertySet::queryInterface( const Type & rType )
    throw( RuntimeException, std::exception )
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL GenericPropertySet::queryAggregation( const Type & rType )
    throw(RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
sal_Bool SAL_CALL GenericPropertySet::supportsService( const  OUString& ServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL GenericPropertySet::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.comphelper.GenericPropertySet" );
}

Sequence< OUString > SAL_CALL GenericPropertySet::getSupportedServiceNames(  )
    throw( RuntimeException, std::exception )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = "com.sun.star.beans.XPropertySet";
    return aSNS;
}

::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > comphelper::GenericPropertySet_CreateInstance( comphelper::PropertySetInfo* pInfo )
{
    return (XPropertySet*)new GenericPropertySet( pInfo );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
