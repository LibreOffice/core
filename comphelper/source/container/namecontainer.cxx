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

#include <comphelper/namecontainer.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/container/XNameContainer.hpp>

typedef std::map<OUString, css::uno::Any> SvGenericNameContainerMapImpl;

namespace comphelper
{
    /** this is the base helper class for NameContainer that's also declared in this header. */
    class NameContainer : public ::cppu::WeakImplHelper< css::container::XNameContainer >
    {
    public:
        explicit NameContainer( const css::uno::Type& aType );

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
        virtual sal_Bool SAL_CALL hasElements(  ) override;
        virtual css::uno::Type SAL_CALL getElementType(  ) override;

    private:
        SvGenericNameContainerMapImpl maProperties;
        const css::uno::Type maType;
        osl::Mutex maMutex;
    };
}

using namespace ::comphelper;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


NameContainer::NameContainer( const css::uno::Type& aType )
: maType( aType )
{
}

// XNameContainer
void SAL_CALL NameContainer::insertByName( const OUString& aName, const Any& aElement )
{
    MutexGuard aGuard( maMutex );

    if( maProperties.find( aName ) != maProperties.end() )
        throw ElementExistException();

    if( aElement.getValueType() != maType )
        throw IllegalArgumentException();

    maProperties.emplace(aName,aElement);
}

void SAL_CALL NameContainer::removeByName( const OUString& Name )
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace

void SAL_CALL NameContainer::replaceByName( const OUString& aName, const Any& aElement )
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter( maProperties.find( aName ) );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    if( aElement.getValueType() != maType )
        throw IllegalArgumentException();

    (*aIter).second = aElement;
}

// XNameAccess

Any SAL_CALL NameContainer::getByName( const OUString& aName )
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    return (*aIter).second;
}

Sequence< OUString > SAL_CALL NameContainer::getElementNames(  )
{
    MutexGuard aGuard( maMutex );

    return comphelper::mapKeysToSequence(maProperties);
}

sal_Bool SAL_CALL NameContainer::hasByName( const OUString& aName )
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

sal_Bool SAL_CALL NameContainer::hasElements(  )
{
    MutexGuard aGuard( maMutex );

    return !maProperties.empty();
}

Type SAL_CALL NameContainer::getElementType()
{
    return maType;
}

Reference< XNameContainer > comphelper::NameContainer_createInstance( const Type& aType )
{
    return new NameContainer(aType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
