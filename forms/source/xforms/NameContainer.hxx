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

#ifndef INCLUDED_FORMS_SOURCE_XFORMS_NAMECONTAINER_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_NAMECONTAINER_HXX

#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <map>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <osl/diagnose.h>

typedef cppu::WeakImplHelper<
    css::container::XNameContainer
> NameContainer_t;

template<class T>
class NameContainer : public NameContainer_t
{
    typedef std::map<OUString,T> map_t;
    map_t maItems;

protected:
    typename map_t::const_iterator findItem( const OUString& rName )
    {
        return maItems.find( rName );
    }

    bool hasItem( const OUString& rName )
    {
        return findItem( rName ) != maItems.end();
    }

    void replace( const OUString& rName,
                  const T& aElement )
    {
        OSL_ENSURE( hasItem( rName ), "unknown item" );
        maItems[ rName ] = aElement;
    }

    void insert( const OUString& rName,
                 const T& aElement )
    {
        OSL_ENSURE( ! hasItem( rName ), "item already in set" );
        maItems[ rName ] = aElement;
    }

    void remove( const OUString& rName )
    {
        OSL_ENSURE( hasItem( rName ), "item not in set" );
        maItems.erase( rName );
    }


public:

    NameContainer() {}


    // methods for XElementAccess


    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<T>::get();
    }

    virtual sal_Bool SAL_CALL hasElements() override
    {
        return ! maItems.empty();
    }


    // methods for XNameAccess (inherits XElementAccess)


    virtual css::uno::Any SAL_CALL getByName(
        const OUString& rName ) override
    {
        typename map_t::const_iterator aIter = findItem( rName );
        if( aIter == maItems.end() )
            throw css::container::NoSuchElementException();
        return css::uno::makeAny( aIter->second );
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getElementNames() override
    {
        return comphelper::mapKeysToSequence(maItems);
    }

    virtual sal_Bool SAL_CALL hasByName(
        const OUString& rName ) override
    {
        return hasItem( rName );
    }


    // methods for XNameReplace (inherits XNameAccess)


    virtual void SAL_CALL replaceByName(
        const OUString& rName,
        const css::uno::Any& aElement ) override
    {
        T aItem;
        if( !(aElement >>= aItem) )
            throw css::lang::IllegalArgumentException();
        if( !hasByName( rName ) )
            throw css::container::NoSuchElementException();
        replace( rName, aItem );
    }


    // methods for XNameContainer (inherits XNameReplace)


    virtual void SAL_CALL insertByName(
        const OUString& rName,
        const css::uno::Any& aElement ) override
    {
        T aItem;
        if( !(aElement >>= aItem) )
            throw css::lang::IllegalArgumentException();
        if( hasByName( rName ) )
            throw css::container::ElementExistException();
        insert( rName, aItem );
    }

    virtual void SAL_CALL removeByName(
        const OUString& rName ) override
    {
        if( !hasByName( rName ) )
            throw css::container::NoSuchElementException();
        remove( rName );
    }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
