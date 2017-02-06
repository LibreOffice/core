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

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <vector>

using namespace com::sun::star;

typedef std::vector < uno::Sequence< beans::PropertyValue > > IndexedPropertyValues;

class IndexedPropertyValuesContainer : public cppu::WeakImplHelper< container::XIndexContainer, lang::XServiceInfo >
{
public:
    IndexedPropertyValuesContainer() throw();

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 nIndex, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 nIndex ) override;

    // XIndexReplace
    virtual void SAL_CALL replaceByIndex( sal_Int32 nIndex, const css::uno::Any& aElement ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    IndexedPropertyValues maProperties;
};

IndexedPropertyValuesContainer::IndexedPropertyValuesContainer() throw()
{
}

// XIndexContainer
void SAL_CALL IndexedPropertyValuesContainer::insertByIndex( sal_Int32 nIndex, const css::uno::Any& aElement )
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
                sal_Int32 i(nSize);
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
            sal_Int32 i(nSize);
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
void SAL_CALL IndexedPropertyValuesContainer::replaceByIndex( sal_Int32 nIndex, const css::uno::Any& aElement )
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
{
    return maProperties.size();
}

css::uno::Any SAL_CALL IndexedPropertyValuesContainer::getByIndex( sal_Int32 nIndex )
{
    sal_Int32 nSize(maProperties.size());
    if (!((nIndex < nSize) && (nIndex >= 0)))
        throw lang::IndexOutOfBoundsException();

    return uno::Any( maProperties[nIndex] );
}

// XElementAccess
css::uno::Type SAL_CALL IndexedPropertyValuesContainer::getElementType(  )
{
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SAL_CALL IndexedPropertyValuesContainer::hasElements(  )
{
    return !maProperties.empty();
}

//XServiceInfo
OUString SAL_CALL IndexedPropertyValuesContainer::getImplementationName(  )
{
    return OUString( "IndexedPropertyValuesContainer" );
}

sal_Bool SAL_CALL IndexedPropertyValuesContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer::getSupportedServiceNames(  )
{
    const OUString aServiceName( "com.sun.star.document.IndexedPropertyValues" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
IndexedPropertyValuesContainer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new IndexedPropertyValuesContainer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
