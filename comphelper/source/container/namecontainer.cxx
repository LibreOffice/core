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
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

typedef std::map<OUString, css::uno::Any> SvGenericNameContainerMapImpl;

namespace comphelper
{
    class NameContainerImpl
    {
    public:
        osl::Mutex maMutex;
    };

    /** this is the base helper class for NameContainer thats also declared in this header. */
    class NameContainer : public ::cppu::WeakImplHelper< css::container::XNameContainer >, private NameContainerImpl
    {
    public:
        explicit NameContainer( css::uno::Type aType );
        virtual ~NameContainer();

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
        virtual sal_Bool SAL_CALL hasElements(  )
            throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Type SAL_CALL getElementType(  )
            throw(css::uno::RuntimeException, std::exception) override;

    private:
        SvGenericNameContainerMapImpl maProperties;
        const css::uno::Type maType;
    };
}

using namespace ::comphelper;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;


NameContainer::NameContainer( css::uno::Type aType )
: maType( aType )
{
}

NameContainer::~NameContainer()
{
}

// XNameContainer
void SAL_CALL NameContainer::insertByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, ElementExistException,
        WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    if( maProperties.find( aName ) != maProperties.end() )
        throw ElementExistException();

    if( aElement.getValueType() != maType )
        throw IllegalArgumentException();

    maProperties.insert( SvGenericNameContainerMapImpl::value_type(aName,aElement));
}

void SAL_CALL NameContainer::removeByName( const OUString& Name )
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( Name );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    maProperties.erase( aIter );
}

// XNameReplace

void SAL_CALL NameContainer::replaceByName( const OUString& aName, const Any& aElement )
    throw(IllegalArgumentException, NoSuchElementException,
        WrappedTargetException, RuntimeException, std::exception)
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
    throw(NoSuchElementException, WrappedTargetException,
        RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    if( aIter == maProperties.end() )
        throw NoSuchElementException();

    return (*aIter).second;
}

Sequence< OUString > SAL_CALL NameContainer::getElementNames(  )
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    return comphelper::mapKeysToSequence(maProperties);
}

sal_Bool SAL_CALL NameContainer::hasByName( const OUString& aName )
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    SvGenericNameContainerMapImpl::iterator aIter = maProperties.find( aName );
    return aIter != maProperties.end();
}

sal_Bool SAL_CALL NameContainer::hasElements(  )
    throw(RuntimeException, std::exception)
{
    MutexGuard aGuard( maMutex );

    return !maProperties.empty();
}

Type SAL_CALL NameContainer::getElementType()
    throw( RuntimeException, std::exception )
{
    return maType;
}

Reference< XNameContainer > comphelper::NameContainer_createInstance( Type aType )
{
    return static_cast<XNameContainer*>(new NameContainer( aType ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
