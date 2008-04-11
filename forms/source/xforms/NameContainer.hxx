/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NameContainer.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _NAMECONTAINER_HXX
#define _NAMECONTAINER_HXX

#include <cppuhelper/implbase1.hxx>
#include <map>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Type.hxx>

typedef cppu::WeakImplHelper1<
    com::sun::star::container::XNameContainer
> NameContainer_t;

template<class T>
class NameContainer : public NameContainer_t
{
protected:
    typedef std::map<rtl::OUString,T> map_t;
    map_t maItems;


    bool hasItems()
    {
        return ! maItems.empty();
    }

    typename map_t::const_iterator findItem( const rtl::OUString& rName )
    {
        return maItems.find( rName );
    }

    bool hasItem( const rtl::OUString& rName )
    {
        return findItem( rName ) != maItems.end();
    }

    T getItem( const rtl::OUString& rName )
    {
        OSL_ENSURE( hasItem( rName ), "can't get non-existant item" );
        return maItems[ rName ];
    }


    void replace( const rtl::OUString& rName,
                  const T& aElement )
    {
        OSL_ENSURE( hasItem( rName ), "unknown item" );
        maItems[ rName ] = aElement;
    }

    void insert( const rtl::OUString& rName,
                 const T& aElement )
    {
        OSL_ENSURE( ! hasItem( rName ), "item already in set" );
        maItems[ rName ] = aElement;
    }

    void remove( const rtl::OUString& rName )
    {
        OSL_ENSURE( hasItem( rName ), "item not in set" );
        maItems.erase( rName );
    }


public:

    NameContainer() {}
    virtual ~NameContainer() {}

    //
    // methods for XElementAccess
    //

    virtual com::sun::star::uno::Type SAL_CALL getElementType()
        throw( com::sun::star::uno::RuntimeException )
    {
        return getCppuType( static_cast<T*>( NULL ) );
    }

    virtual sal_Bool SAL_CALL hasElements()
        throw( com::sun::star::uno::RuntimeException )
    {
        return hasItems();
    }


    //
    // methods for XNameAccess (inherits XElementAccess)
    //

    virtual com::sun::star::uno::Any SAL_CALL getByName(
        const rtl::OUString& rName )
        throw( com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException )
    {
        typename map_t::const_iterator aIter = findItem( rName );
        if( aIter == maItems.end() )
            throw com::sun::star::container::NoSuchElementException();
        else
            return com::sun::star::uno::makeAny( aIter->second );
    }

    virtual com::sun::star::uno::Sequence<rtl::OUString> SAL_CALL getElementNames()
        throw( com::sun::star::uno::RuntimeException )
    {
        com::sun::star::uno::Sequence<rtl::OUString> aSequence( maItems.size() );
        typename map_t::const_iterator aIter = maItems.begin();
        rtl::OUString* pStrings = aSequence.getArray();
        while( aIter != maItems.end() )
        {
            *pStrings = aIter->first;
            ++aIter;
            ++pStrings;
        }
        OSL_ENSURE( pStrings == aSequence.getArray() + aSequence.getLength(),
                    "sequence not of right size; possible buffer overflow" );
        return aSequence;
    }

    virtual sal_Bool SAL_CALL hasByName(
        const rtl::OUString& rName )
        throw( com::sun::star::uno::RuntimeException )
    {
        return hasItem( rName );
    }


    //
    // methods for XNameReplace (inherits XNameAccess)
    //

    virtual void SAL_CALL replaceByName(
        const rtl::OUString& rName,
        const com::sun::star::uno::Any& aElement )
        throw( com::sun::star::lang::IllegalArgumentException,
               com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException)
    {
        T aItem;
        if( aElement >>= aItem )
            if( hasByName( rName ) )
                replace( rName, aItem );
            else
                throw com::sun::star::container::NoSuchElementException();
        else
            throw com::sun::star::lang::IllegalArgumentException();
    }


    //
    // methods for XNameContainer (inherits XNameReplace)
    //

    virtual void SAL_CALL insertByName(
        const rtl::OUString& rName,
        const com::sun::star::uno::Any& aElement )
        throw( com::sun::star::lang::IllegalArgumentException,
               com::sun::star::container::ElementExistException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException )
    {
        T aItem;
        if( aElement >>= aItem )
            if( ! hasByName( rName ) )
                insert( rName, aItem );
            else
                throw com::sun::star::container::ElementExistException();
        else
            throw com::sun::star::lang::IllegalArgumentException();
    }

    virtual void SAL_CALL removeByName(
        const rtl::OUString& rName )
        throw( com::sun::star::container::NoSuchElementException,
               com::sun::star::lang::WrappedTargetException,
               com::sun::star::uno::RuntimeException)
    {
        if( hasByName( rName ) )
            remove( rName );
        else
            throw com::sun::star::container::NoSuchElementException();
    }

};

#endif
