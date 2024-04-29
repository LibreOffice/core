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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <map>


namespace com::sun::star::uno { class XComponentContext; }
using namespace com::sun::star;

typedef std::map< OUString, uno::Sequence<beans::PropertyValue> > NamedPropertyValues;

namespace {

class NamedPropertyValuesContainer : public cppu::WeakImplHelper< container::XNameContainer, lang::XServiceInfo >
{
public:
    NamedPropertyValuesContainer() noexcept;

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    virtual void SAL_CALL removeByName( const OUString& Name ) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

private:
    NamedPropertyValues maProperties;
};

}

NamedPropertyValuesContainer::NamedPropertyValuesContainer() noexcept
{
}

// XNameContainer
void SAL_CALL NamedPropertyValuesContainer::insertByName( const OUString& aName, const uno::Any& aElement )
{
    if( maProperties.find( aName ) != maProperties.end() )
        throw container::ElementExistException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps ) )
        throw lang::IllegalArgumentException(u"element is not beans::PropertyValue"_ustr, static_cast<cppu::OWeakObject*>(this), 2);

    maProperties.emplace( aName, aProps );
}

void SAL_CALL NamedPropertyValuesContainer::removeByName( const OUString& Name )
{
    NamedPropertyValues::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace
void SAL_CALL NamedPropertyValuesContainer::replaceByName( const OUString& aName, const css::uno::Any& aElement )
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps) )
        throw lang::IllegalArgumentException(u"element is not beans::PropertyValue"_ustr, static_cast<cppu::OWeakObject*>(this), 2);

    (*aIter).second = aProps;
}

// XNameAccess
css::uno::Any SAL_CALL NamedPropertyValuesContainer::getByName( const OUString& aName )
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Any aElement;

    aElement <<= (*aIter).second;

    return aElement;
}

css::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getElementNames(  )
{
    return comphelper::mapKeysToSequence(maProperties);
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasByName( const OUString& aName )
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

// XElementAccess
css::uno::Type SAL_CALL NamedPropertyValuesContainer::getElementType(  )
{
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasElements(  )
{
    return !maProperties.empty();
}

//XServiceInfo
OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName(  )
{
    return u"NamedPropertyValuesContainer"_ustr;
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames(  )
{
    return { u"com.sun.star.document.NamedPropertyValues"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
NamedPropertyValuesContainer_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new NamedPropertyValuesContainer());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
