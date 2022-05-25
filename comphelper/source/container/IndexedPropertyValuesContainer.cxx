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

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/indexedpropertyvalues.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>

namespace com::sun::star::uno { class XComponentContext; }

using namespace com::sun::star;


namespace comphelper {


IndexedPropertyValuesContainer::IndexedPropertyValuesContainer() noexcept
{
}

// XIndexContainer
void SAL_CALL IndexedPropertyValuesContainer::insertByIndex( sal_Int32 nIndex, const css::uno::Any& aElement )
{
    sal_Int32 nSize(maProperties.size());
    if ((nSize < nIndex) || (nIndex < 0))
        throw lang::IndexOutOfBoundsException();

    uno::Sequence<beans::PropertyValue> aProps;
    if (!(aElement >>= aProps))
        throw lang::IllegalArgumentException("element is not beans::PropertyValue", static_cast<cppu::OWeakObject*>(this), 2);
    if (nSize == nIndex)
        maProperties.push_back(aProps);
    else
        maProperties.insert(maProperties.begin() + nIndex, aProps);
}

void SAL_CALL IndexedPropertyValuesContainer::removeByIndex( sal_Int32 nIndex )
{
    if ((nIndex < 0) || (o3tl::make_unsigned(nIndex) >= maProperties.size()))
        throw lang::IndexOutOfBoundsException();

    maProperties.erase(maProperties.begin() + nIndex);
}

// XIndexReplace
void SAL_CALL IndexedPropertyValuesContainer::replaceByIndex( sal_Int32 nIndex, const css::uno::Any& aElement )
{
    sal_Int32 nSize(maProperties.size());
    if ((nIndex >= nSize) || (nIndex < 0))
        throw lang::IndexOutOfBoundsException();

    uno::Sequence<beans::PropertyValue> aProps;
    if (!(aElement >>= aProps))
        throw lang::IllegalArgumentException("element is not beans::PropertyValue", static_cast<cppu::OWeakObject*>(this), 2);
    maProperties[nIndex] = aProps;
}

// XIndexAccess
sal_Int32 SAL_CALL IndexedPropertyValuesContainer::getCount(  )
{
    return maProperties.size();
}

css::uno::Any SAL_CALL IndexedPropertyValuesContainer::getByIndex( sal_Int32 nIndex )
{
    sal_Int32 nSize(maProperties.size());
    if ((nIndex >= nSize) || (nIndex < 0))
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
    return "IndexedPropertyValuesContainer";
}

sal_Bool SAL_CALL IndexedPropertyValuesContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL IndexedPropertyValuesContainer::getSupportedServiceNames(  )
{
    return { "com.sun.star.document.IndexedPropertyValues" };
}

} // namespace comphelper

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
IndexedPropertyValuesContainer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new comphelper::IndexedPropertyValuesContainer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
