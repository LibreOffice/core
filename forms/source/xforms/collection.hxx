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

#pragma once

#include "enumeration.hxx"

#include <cppuhelper/implbase.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <vector>


typedef cppu::WeakImplHelper<
    css::container::XIndexReplace,
    css::container::XSet,
    css::container::XContainer>
Collection_t;

template<class ELEMENT_TYPE>
class Collection : public Collection_t
{
public:
    typedef ELEMENT_TYPE T;
    typedef std::vector<css::uno::Reference<css::container::XContainerListener> > Listeners_t;

protected:
    std::vector<T> maItems;
    Listeners_t maListeners;

public:

    Collection() {}

    const T& getItem( sal_Int32 n ) const
    {
        OSL_ENSURE( isValidIndex(n), "invalid index" );
        OSL_ENSURE( isValid( maItems[n] ), "invalid item found" );
        return maItems[n];
    }

    void setItem( sal_Int32 n, const T& t)
    {
        OSL_ENSURE( isValidIndex(n), "invalid index" );
        OSL_ENSURE( isValid ( t ), "invalid item" );

        T& aRef = maItems[ n ];
        _elementReplaced( n, t );
        _remove( aRef );
        aRef = t;
        _insert( t );
    }

    bool hasItem( const T& t ) const
    {
        return maItems.end() != std::find( maItems.begin(), maItems.end(), t );
    }

    sal_Int32 addItem( const T& t )
    {
        OSL_ENSURE( !hasItem( t ), "item to be added already present" );
        OSL_ENSURE( isValid( t ), "invalid item" );

        maItems.push_back( t );
        _insert( t );
        _elementInserted( maItems.size() - 1 );
        return ( maItems.size() - 1 );
    }

    void removeItem( const T& t )
    {
        OSL_ENSURE( hasItem( t ), "item to be removed not present" );
        OSL_ENSURE( isValid( t ), "an invalid item, funny that!" );

        _elementRemoved( t );
        _remove( t );
        maItems.erase( std::find( maItems.begin(), maItems.end(), t ) );
    }

    bool hasItems() const
    {
        return maItems.size() != 0;
    }

    sal_Int32 countItems() const
    {
        return static_cast<sal_Int32>( maItems.size() );
    }

    bool isValidIndex( sal_Int32 n ) const
    {
        return n >= 0  &&  n < static_cast<sal_Int32>( maItems.size() );
    }


    // the following method may be overridden by sub-classes for
    // customized behaviour

    /// called before insertion to determine whether item is valid
    virtual bool isValid( const T& ) const { return true; }


protected:

    // the following methods may be overridden by sub-classes for
    // customized behaviour

    /// called after item has been inserted into the collection
    virtual void _insert( const T& ) { }

    /// called before item is removed from the collection
    virtual void _remove( const T& ) { }

public:

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<T>::get();
    }

    virtual sal_Bool SAL_CALL hasElements() override
    {
        return hasItems();
    }

    // XIndexAccess : XElementAccess
    virtual sal_Int32 SAL_CALL getCount() override
    {
        return countItems();
    }

    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) override
    {
        if( !isValidIndex( nIndex ) )
            throw css::lang::IndexOutOfBoundsException();
        return css::uno::makeAny( getItem( nIndex ) );
    }

    // XIndexReplace : XIndexAccess
    virtual void SAL_CALL replaceByIndex( sal_Int32 nIndex,
                                          const css::uno::Any& aElement ) override
    {
        T t;
        if( !isValidIndex( nIndex) )
            throw css::lang::IndexOutOfBoundsException();
        if( !( aElement >>= t ) || !isValid( t ) )
            throw css::lang::IllegalArgumentException();
        setItem( nIndex, t );
    }

    // XEnumerationAccess : XElementAccess
    virtual css::uno::Reference<css::container::XEnumeration> SAL_CALL createEnumeration() override
    {
        return new Enumeration( this );
    }


    // XSet : XEnumerationAccess
    virtual sal_Bool SAL_CALL has( const css::uno::Any& aElement ) override
    {
        T t;
        return ( aElement >>= t ) && hasItem( t );
    }

    virtual void SAL_CALL insert( const css::uno::Any& aElement ) override
    {
        T t;
        if( !( aElement >>= t )  || !isValid( t ) )
            throw css::lang::IllegalArgumentException();
        if( hasItem( t ) )
            throw css::container::ElementExistException();
        addItem( t );
    }

    virtual void SAL_CALL remove( const css::uno::Any& aElement ) override
    {
        T t;
        if( !(aElement >>= t) )
            throw css::lang::IllegalArgumentException();
        if( !hasItem( t ) )
            throw css::container::NoSuchElementException();
        removeItem( t );
    }


    // XContainer
    virtual void SAL_CALL addContainerListener(
        const css::uno::Reference<css::container::XContainerListener>& xListener ) override
    {
        OSL_ENSURE( xListener.is(), "need listener!" );
        if( std::find( maListeners.begin(), maListeners.end(), xListener)
            == maListeners.end() )
            maListeners.push_back( xListener );
    }

    virtual void SAL_CALL removeContainerListener(
        const css::uno::Reference<css::container::XContainerListener>& xListener ) override
    {
        OSL_ENSURE( xListener.is(), "need listener!" );
        Listeners_t::iterator aIter =
            std::find( maListeners.begin(), maListeners.end(), xListener );
        if( aIter != maListeners.end() )
            maListeners.erase( aIter );
    }

protected:

    // call listeners:
    void _elementInserted( sal_Int32 nPos )
    {
        OSL_ENSURE( isValidIndex(nPos), "invalid index" );
        css::container::ContainerEvent aEvent(
            static_cast<css::container::XIndexReplace*>( this ),
            css::uno::makeAny( nPos ),
            css::uno::makeAny( getItem( nPos ) ),
            css::uno::Any() );
        for (auto const& listener : maListeners)
        {
            listener->elementInserted( aEvent );
        }
    }

    void _elementRemoved( const T& aOld )
    {
        css::container::ContainerEvent aEvent(
            static_cast<css::container::XIndexReplace*>( this ),
            css::uno::Any(),
            css::uno::makeAny( aOld ),
            css::uno::Any() );
        for (auto const& listener : maListeners)
        {
            listener->elementRemoved( aEvent );
        }
    }

    void _elementReplaced( const sal_Int32 nPos, const T& aNew )
    {
        OSL_ENSURE( isValidIndex(nPos), "invalid index" );
        css::container::ContainerEvent aEvent(
            static_cast<css::container::XIndexReplace*>( this ),
            css::uno::makeAny( nPos ),
            css::uno::makeAny( getItem( nPos ) ),
            css::uno::makeAny( aNew ) );
        for (auto const& listener : maListeners)
        {
            listener->elementReplaced( aEvent );
        }
    }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
