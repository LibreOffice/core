/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/uno/XAggregation.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <comphelper/propertysethelper.hxx>
#include <mutex>
#include <rtl/ref.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>

using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

namespace comphelper
{
    namespace {

    class GenericPropertySet :  public OWeakObject,
                                public XServiceInfo,
                                public XTypeProvider,
                                public PropertySetHelper
    {
    private:
        std::map<OUString, Any>   maAnyMap;
        std::mutex                maMutex;
        comphelper::OMultiTypeInterfaceContainerHelperVar4<OUString, XPropertyChangeListener> m_aListener;

    protected:
        virtual void _setPropertyValues( const PropertyMapEntry** ppEntries, const  Any* pValues ) override;
        virtual void _getPropertyValues( const PropertyMapEntry** ppEntries,  Any* pValue ) override;

    public:
        explicit GenericPropertySet( PropertySetInfo* pInfo ) noexcept;

        // XInterface
        virtual  Any queryInterface( const  Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XTypeProvider
        virtual  Sequence<  Type > getTypes(  ) override;
        virtual  Sequence< sal_Int8 > getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString getImplementationName() override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual  Sequence< OUString > getSupportedServiceNames() override;

        // XPropertySet
        virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    };

    }
}


GenericPropertySet::GenericPropertySet( PropertySetInfo* pInfo ) noexcept
: PropertySetHelper( pInfo )
{
}

void GenericPropertySet::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
{
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    if ( !xInfo.is() )
        return;

    std::unique_lock aGuard(maMutex);
    if ( aPropertyName.isEmpty() )
    {
        for (auto& prop : xInfo->getProperties())
        {
            m_aListener.addInterface(aGuard, prop.Name, xListener);
        }
    }
    else if ( xInfo->hasPropertyByName(aPropertyName) )
        m_aListener.addInterface(aGuard, aPropertyName,xListener);
    else
        throw UnknownPropertyException( aPropertyName, *this );
}

void GenericPropertySet::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
{
    Reference < XPropertySetInfo > xInfo = getPropertySetInfo(  );
    if ( !xInfo.is() )
        return;

    std::unique_lock aGuard(maMutex);
    if ( aPropertyName.isEmpty() )
    {
        for (auto& prop : xInfo->getProperties())
        {
            m_aListener.removeInterface(aGuard, prop.Name, xListener);
        }
    }
    else if ( xInfo->hasPropertyByName(aPropertyName) )
        m_aListener.removeInterface(aGuard, aPropertyName,xListener);
    else
        throw UnknownPropertyException( aPropertyName, *this );
}

void GenericPropertySet::_setPropertyValues( const PropertyMapEntry** ppEntries, const Any* pValues )
{
    std::unique_lock aGuard(maMutex);

    while( *ppEntries )
    {
        OInterfaceContainerHelper4<XPropertyChangeListener> * pHelper = m_aListener.getContainer(aGuard, (*ppEntries)->maName);

        maAnyMap[ (*ppEntries)->maName ] = *pValues;

        if ( pHelper )
        {
            PropertyChangeEvent aEvt;
            aEvt.PropertyName = (*ppEntries)->maName;
            aEvt.NewValue = *pValues;
            pHelper->notifyEach( aGuard, &XPropertyChangeListener::propertyChange, aEvt );
        }

        ppEntries++;
        pValues++;
    }
}

void GenericPropertySet::_getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, Any* pValue )
{
    std::unique_lock aGuard(maMutex);

    while( *ppEntries )
    {
        *pValue = maAnyMap[ (*ppEntries)->maName ];

        ppEntries++;
        pValue++;
    }
}

// XInterface

Any GenericPropertySet::queryInterface( const Type & rType )
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
        aAny = OWeakObject::queryInterface( rType );

    return aAny;
}

void GenericPropertySet::acquire() noexcept
{
    OWeakObject::acquire();
}

void GenericPropertySet::release() noexcept
{
    OWeakObject::release();
}

cpo::uno::Sequence< cpo::uno::Type > GenericPropertySet::getTypes()
{
    return cpo::uno::Sequence {
        cppu::UnoType<XAggregation>::get(),
        cppu::UnoType<XServiceInfo>::get(),
        cppu::UnoType<XTypeProvider>::get(),
        cppu::UnoType<XPropertySet>::get(),
        cppu::UnoType<XMultiPropertySet>::get() };
}

cpo::uno::Sequence< sal_Int8 > GenericPropertySet::getImplementationId()
{
    return cpo::uno::Sequence<sal_Int8>();
}

// XServiceInfo
bool GenericPropertySet::supportsService( const  OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

OUString GenericPropertySet::getImplementationName()
{
    return u"com.sun.star.comp.comphelper.GenericPropertySet"_ustr;
}

Sequence< OUString > GenericPropertySet::getSupportedServiceNames(  )
{
    return { u"com.sun.star.beans.XPropertySet"_ustr };
}

css::uno::Reference< css::beans::XPropertySet > comphelper::GenericPropertySet_CreateInstance( comphelper::PropertySetInfo* pInfo )
{
    return static_cast<XPropertySet*>(new GenericPropertySet( pInfo ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
