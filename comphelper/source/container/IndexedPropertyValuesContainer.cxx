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

#include "comphelper_module.hxx"

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <vector>

using namespace com::sun::star;

typedef std::vector < uno::Sequence< beans::PropertyValue > > IndexedPropertyValues;

class IndexedPropertyValuesContainer : public cppu::WeakImplHelper2< container::XIndexContainer, lang::XServiceInfo >
{
public:
    IndexedPropertyValuesContainer() throw();
    virtual ~IndexedPropertyValuesContainer() throw();

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL removeByIndex( sal_Int32 nIndex )
        throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception);

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& aElement )
        throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException,
            ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex )
        throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception);

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(::com::sun::star::uno::RuntimeException, std::exception);

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

private:
    IndexedPropertyValues maProperties;
};

IndexedPropertyValuesContainer::IndexedPropertyValuesContainer() throw()
{
}

IndexedPropertyValuesContainer::~IndexedPropertyValuesContainer() throw()
{
}

// XIndexContainer
void SAL_CALL IndexedPropertyValuesContainer::insertByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    sal_Int32 nSize(maProperties.size());
    if ((nSize >= nIndex) && (nIndex >= 0))
    {
        uno::Sequence<beans::PropertyValue> aProps;
        if (!(aElement >>= aProps))
            throw lang::IllegalArgumentException();
        if (nSize == nIndex)
            maProperties.push_back(aProps);
        else
        {
            IndexedPropertyValues::iterator aItr;
            if ((nIndex * 2) < nSize)
            {
                aItr = maProperties.begin();
                sal_Int32 i(0);
                while(i < nIndex)
                {
                    ++i;
                    ++aItr;
                }
            }
            else
            {
                aItr = maProperties.end();
                sal_Int32 i(nSize - 1);
                while(i > nIndex)
                {
                    --i;
                    --aItr;
                }
            }
            maProperties.insert(aItr, aProps);
        }
    }
    else
        throw lang::IndexOutOfBoundsException();
}

void SAL_CALL IndexedPropertyValuesContainer::removeByIndex( sal_Int32 nIndex )
    throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException, std::exception)
{
    sal_Int32 nSize(maProperties.size());
    if ((nIndex < nSize) && (nIndex >= 0))
    {
        IndexedPropertyValues::iterator aItr;
        if ((nIndex * 2) < nSize)
        {
            aItr = maProperties.begin();
            sal_Int32 i(0);
            while(i < nIndex)
            {
                ++i;
                ++aItr;
            }
        }
        else
        {
            aItr = maProperties.end();
            sal_Int32 i(nSize - 1);
            while(i > nIndex)
            {
                --i;
                --aItr;
            }
        }
        maProperties.erase(aItr);
    }
    else
        throw lang::IndexOutOfBoundsException();
}

// XIndexReplace
void SAL_CALL IndexedPropertyValuesContainer::replaceByIndex( sal_Int32 nIndex, const ::com::sun::star::uno::Any& aElement )
    throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException,
        ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    sal_Int32 nSize(maProperties.size());
    if ((nIndex < nSize) && (nIndex >= 0))
    {
        uno::Sequence<beans::PropertyValue> aProps;
        if (!(aElement >>= aProps))
            throw lang::IllegalArgumentException();
        maProperties[nIndex] = aProps;
    }
    else
        throw lang::IndexOutOfBoundsException();
}

// XIndexAccess
sal_Int32 SAL_CALL IndexedPropertyValuesContainer::getCount(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return maProperties.size();
}

::com::sun::star::uno::Any SAL_CALL IndexedPropertyValuesContainer::getByIndex( sal_Int32 nIndex )
    throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException,
        ::com::sun::star::uno::RuntimeException, std::exception)
{
    sal_Int32 nSize(maProperties.size());
    if (!((nIndex < nSize) && (nIndex >= 0)))
        throw lang::IndexOutOfBoundsException();

    uno::Any aAny;
    aAny <<= maProperties[nIndex];
    return aAny;
}

// XElementAccess
::com::sun::star::uno::Type SAL_CALL IndexedPropertyValuesContainer::getElementType(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return ::getCppuType((uno::Sequence<beans::PropertyValue> *)0);
}

sal_Bool SAL_CALL IndexedPropertyValuesContainer::hasElements(  )
    throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return !maProperties.empty();
}

//XServiceInfo
OUString SAL_CALL IndexedPropertyValuesContainer::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getImplementationName_static();
}

OUString SAL_CALL IndexedPropertyValuesContainer::getImplementationName_static(  )
{
    return OUString( "IndexedPropertyValuesContainer" );
}

sal_Bool SAL_CALL IndexedPropertyValuesContainer::supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

::com::sun::star::uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}


::com::sun::star::uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer::getSupportedServiceNames_static(  )
{
    const OUString aServiceName( "com.sun.star.document.IndexedPropertyValues" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}


uno::Reference< uno::XInterface > SAL_CALL IndexedPropertyValuesContainer::Create(
                SAL_UNUSED_PARAMETER const uno::Reference< uno::XComponentContext >&)
{
    return (cppu::OWeakObject*)new IndexedPropertyValuesContainer();
}

void createRegistryInfo_IndexedPropertyValuesContainer()
{
    static ::comphelper::module::OAutoRegistration< IndexedPropertyValuesContainer > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
