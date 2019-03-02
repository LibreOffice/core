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
#include <rtl/ref.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>

using namespace ::osl;
using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper
{
    struct IMPL_GenericPropertySet_MutexContainer
    {
        Mutex maMutex;
    };

    class GenericPropertySet :  public OWeakAggObject,
                                public XServiceInfo,
                                public XTypeProvider,
                                public PropertySetHelper,
                                private IMPL_GenericPropertySet_MutexContainer
    {
    private:
        std::map<OUString, Any>   maAnyMap;
        cppu::OMultiTypeInterfaceContainerHelperVar<OUString> m_aListener;

    protected:
        virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const  Any* pValues ) override;
        virtual void _getPropertyValues( const PropertyMapEntry** ppEntries,  Any* pValue ) override;

    public:
        explicit GenericPropertySet( PropertySetInfo* pInfo ) throw();

        // XInterface
        virtual  Any SAL_CALL queryAggregation( const  Type & rType ) override;
        virtual  Any SAL_CALL queryInterface( const  Type & rType ) override;
        virtual void SAL_CALL acquire() throw() override;
        virtual void SAL_CALL release() throw() override;

        // XTypeProvider
        virtual  Sequence<  Type > SAL_CALL getTypes(  ) override;
        virtual  Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual  Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XPropertySet
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    };

}


GenericPropertySet::GenericPropertySet( PropertySetInfo* pInfo ) throw()
: PropertySetHelper( pInfo )
,m_aListener(maMutex)
{
}

void SAL_CALL GenericPropertySet::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
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

void SAL_CALL GenericPropertySet::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
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
{
    return OWeakAggObject::queryInterface( rType );
}

Any SAL_CALL GenericPropertySet::queryAggregation( const Type & rType )
{
    Any aAny;

    if( rType == cppu::UnoType<XServiceInfo>::get())
        aAny <<= Reference< XServiceInfo >(this);
    else if( rType == cppu::UnoType<XTypeProvider>::get())
        aAny <<= Reference< XTypeProvider >(this);
    else if( rType == cppu::UnoType<XPropertySet>::get())
        aAny <<= Reference< XPropertySet >(this);
    else if( rType == cppu::UnoType<XMultiPropertySet>::get())
        aAny <<= Reference< XMultiPropertySet >(this);
    else
        aAny = OWeakAggObject::queryAggregation( rType );

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
{
    return uno::Sequence {
        cppu::UnoType<XAggregation>::get(),
        cppu::UnoType<XServiceInfo>::get(),
        cppu::UnoType<XTypeProvider>::get(),
        cppu::UnoType<XPropertySet>::get(),
        cppu::UnoType<XMultiPropertySet>::get() };
}

uno::Sequence< sal_Int8 > SAL_CALL GenericPropertySet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo
sal_Bool SAL_CALL GenericPropertySet::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL GenericPropertySet::getImplementationName()
{
    return OUString( "com.sun.star.comp.comphelper.GenericPropertySet" );
}

Sequence< OUString > SAL_CALL GenericPropertySet::getSupportedServiceNames(  )
{
    Sequence<OUString> aSNS { "com.sun.star.beans.XPropertySet" };
    return aSNS;
}

css::uno::Reference< css::beans::XPropertySet > comphelper::GenericPropertySet_CreateInstance( comphelper::PropertySetInfo* pInfo )
{
    return static_cast<XPropertySet*>(new GenericPropertySet( pInfo ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
