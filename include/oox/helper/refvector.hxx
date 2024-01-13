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

#ifndef INCLUDED_OOX_HELPER_REFVECTOR_HXX
#define INCLUDED_OOX_HELPER_REFVECTOR_HXX

#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include <sal/types.h>

namespace oox {


/** Template for a vector of ref-counted objects with additional accessor functions.

    An instance of the class RefVector< Type > stores elements of the type
    std::shared_ptr< Type >. The new accessor functions has() and get()
    work correctly for indexes out of the current range, there is no need to
    check the passed index before.
 */
template< typename ObjType >
class RefVector : public ::std::vector< std::shared_ptr< ObjType > >
{
public:
    typedef ::std::vector< std::shared_ptr< ObjType > > container_type;
    typedef typename container_type::value_type             value_type;
    typedef typename container_type::size_type              size_type;

public:

    /** Returns a reference to the object with the passed index, or 0 on error. */
    value_type          get( sal_Int32 nIndex ) const
                        {
                            if( const value_type* pxRef = getRef( nIndex ) ) return *pxRef;
                            return value_type();
                        }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template< typename FunctorType >
    void                forEach( FunctorType aFunctor ) const
    {
        std::for_each(this->begin(), this->end(), ForEachFunctor<FunctorType>(std::move(aFunctor)));
    }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType >
    void                forEachMem( FuncType pFunc ) const
                        {
                            forEach( ::std::bind( pFunc, std::placeholders::_1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType >
    void                forEachMem( FuncType pFunc, ParamType aParam ) const
                        {
                            forEach( ::std::bind( pFunc, std::placeholders::_1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    void                forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEach( ::std::bind( pFunc, std::placeholders::_1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    void                forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEach( ::std::bind( pFunc, std::placeholders::_1, aParam1, aParam2, aParam3 ) );
                        }

    /** Calls the passed functor for every contained object. Passes the index as
        first argument and the object reference as second argument to rFunctor. */
    template< typename FunctorType >
    void                forEachWithIndex( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctorWithIndex< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the vector index as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    void                forEachMemWithIndex( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEachWithIndex( ::std::bind( pFunc, std::placeholders::_2, std::placeholders::_1, aParam1, aParam2 ) );
                        }

    /** Searches for an element by using the passed functor that takes a
        constant reference of the object type (const ObjType&). */
    template< typename FunctorType >
    value_type          findIf( const FunctorType& rFunctor ) const
                        {
                            typename container_type::const_iterator aIt = ::std::find_if( this->begin(), this->end(), FindFunctor< FunctorType >( rFunctor ) );
                            return (aIt == this->end()) ? value_type() : *aIt;
                        }

private:
    template< typename FunctorType >
    struct ForEachFunctor
    {
        FunctorType         maFunctor;
        explicit            ForEachFunctor( FunctorType aFunctor ) : maFunctor(std::move( aFunctor )) {}
        void                operator()( const value_type& rxValue ) { if( rxValue.get() ) maFunctor( *rxValue ); }
    };

    template< typename FunctorType >
    struct ForEachFunctorWithIndex
    {
        FunctorType         maFunctor;
        sal_Int32           mnIndex;
        explicit            ForEachFunctorWithIndex( FunctorType aFunctor ) : maFunctor(std::move( aFunctor )), mnIndex( 0 ) {}
        void                operator()( const value_type& rxValue ) {
            if( rxValue.get() ) maFunctor( mnIndex, *rxValue );
            ++mnIndex;
        }
    };

    template< typename FunctorType >
    struct FindFunctor
    {
        FunctorType         maFunctor;
        explicit            FindFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        bool                operator()( const value_type& rxValue ) { return rxValue.get() && maFunctor( *rxValue ); }
    };

    const value_type*   getRef( sal_Int32 nIndex ) const
                        {
                            return ((0 <= nIndex) && (static_cast< size_type >( nIndex ) < this->size())) ?
                                &(*this)[ static_cast< size_type >( nIndex ) ] : nullptr;
                        }
};


} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
