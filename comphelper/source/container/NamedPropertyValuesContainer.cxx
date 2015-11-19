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
#include "comphelper_services.hxx"

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <map>


using namespace com::sun::star;

typedef std::map< OUString, uno::Sequence<beans::PropertyValue> > NamedPropertyValues;

class NamedPropertyValuesContainer : public cppu::WeakImplHelper< container::XNameContainer, lang::XServiceInfo >
{
public:
    NamedPropertyValuesContainer() throw();
    virtual ~NamedPropertyValuesContainer() throw();

    // XNameContainer
    virtual void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException, css::container::ElementExistException,
        css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByName( const OUString& Name )
        throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException,
            css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
        throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
        throw(css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  )
        throw(css::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL getImplementationName_static();
    static uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_static();
    static uno::Reference< uno::XInterface > SAL_CALL Create( const uno::Reference< uno::XComponentContext >& );

private:
    NamedPropertyValues maProperties;
};

NamedPropertyValuesContainer::NamedPropertyValuesContainer() throw()
{
}

NamedPropertyValuesContainer::~NamedPropertyValuesContainer() throw()
{
}

// XNameContainer
void SAL_CALL NamedPropertyValuesContainer::insertByName( const OUString& aName, const uno::Any& aElement )
    throw(css::lang::IllegalArgumentException, css::container::ElementExistException,
        css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    if( maProperties.find( aName ) != maProperties.end() )
        throw container::ElementExistException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps ) )
        throw lang::IllegalArgumentException();

    maProperties.insert(  NamedPropertyValues::value_type(aName ,aProps) );
}

void SAL_CALL NamedPropertyValuesContainer::removeByName( const OUString& Name )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace
void SAL_CALL NamedPropertyValuesContainer::replaceByName( const OUString& aName, const css::uno::Any& aElement )
    throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException,
        css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Sequence<beans::PropertyValue> aProps;
    if( !(aElement >>= aProps) )
        throw lang::IllegalArgumentException();

    (*aIter).second = aProps;
}

// XNameAccess
css::uno::Any SAL_CALL NamedPropertyValuesContainer::getByName( const OUString& aName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw container::NoSuchElementException();

    uno::Any aElement;

    aElement <<= (*aIter).second;

    return aElement;
}

css::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getElementNames(  )
    throw(css::uno::RuntimeException, std::exception)
{
    return comphelper::mapKeysToSequence(maProperties);
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasByName( const OUString& aName )
    throw(css::uno::RuntimeException, std::exception)
{
    NamedPropertyValues::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

// XElementAccess
css::uno::Type SAL_CALL NamedPropertyValuesContainer::getElementType(  )
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::UnoType<uno::Sequence<beans::PropertyValue>>::get();
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::hasElements(  )
    throw(css::uno::RuntimeException, std::exception)
{
    return !maProperties.empty();
}

//XServiceInfo
OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName(  ) throw(css::uno::RuntimeException, std::exception)
{
    return getImplementationName_static();
}

OUString SAL_CALL NamedPropertyValuesContainer::getImplementationName_static(  )
{
    return OUString( "NamedPropertyValuesContainer" );
}

sal_Bool SAL_CALL NamedPropertyValuesContainer::supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames(  ) throw(css::uno::RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}

css::uno::Sequence< OUString > SAL_CALL NamedPropertyValuesContainer::getSupportedServiceNames_static(  )
{
    const OUString aServiceName( "com.sun.star.document.NamedPropertyValues" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL NamedPropertyValuesContainer::Create(
                SAL_UNUSED_PARAMETER const uno::Reference< uno::XComponentContext >&)
{
    return static_cast<cppu::OWeakObject*>(new NamedPropertyValuesContainer());
}

void createRegistryInfo_NamedPropertyValuesContainer()
{
    static ::comphelper::module::OAutoRegistration< NamedPropertyValuesContainer > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
