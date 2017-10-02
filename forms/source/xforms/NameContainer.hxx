/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        OSL_ENSURE( hasItem( rName ), "can't get non-existent item" );
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
