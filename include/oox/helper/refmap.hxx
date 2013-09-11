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

#ifndef OOX_HELPER_REFMAP_HXX
#define OOX_HELPER_REFMAP_HXX

#include <map>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <sal/types.h>

namespace oox {

// ============================================================================

/** Template for a map of ref-counted objects with additional accessor functions.

    An instance of the class RefMap< Type > stores elements of the type
    ::boost::shared_ptr< Type >. The new accessor functions has() and get()
    work correctly for nonexisting keys, there is no need to check the passed
    key before.
 */
template< typename KeyType, typename ObjType, typename CompType = ::std::less< KeyType > >
class RefMap : public ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType >
{
public:
    typedef ::std::map< KeyType, ::boost::shared_ptr< ObjType >, CompType > container_type;
    typedef typename container_type::key_type                               key_type;
    typedef typename container_type::mapped_type                            mapped_type;
    typedef typename container_type::value_type                             value_type;
    typedef typename container_type::key_compare                            key_compare;

public:
    /** Returns true, if the object associated to the passed key exists.
        Returns false, if the key exists but points to an empty reference. */
    bool                has( key_type nKey ) const
                        {
                            const mapped_type* pxRef = getRef( nKey );
                            return pxRef && pxRef->get();
                        }

    /** Returns a reference to the object associated to the passed key, or an
        empty reference on error. */
    mapped_type         get( key_type nKey ) const
                        {
                            if( const mapped_type* pxRef = getRef( nKey ) ) return *pxRef;
                            return mapped_type();
                        }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template< typename FunctorType >
    void                forEach( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctor< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType >
    void                forEachMem( FuncType pFunc ) const
                        {
                            forEach( ::boost::bind( pFunc, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType >
    void                forEachMem( FuncType pFunc, ParamType aParam ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    void                forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    void                forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3, typename ParamType4 >
    void                forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3, ParamType4 aParam4 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3, aParam4 ) );
                        }


    /** Calls the passed functor for every contained object. Passes the key as
        first argument and the object reference as second argument to rFunctor. */
    template< typename FunctorType >
    void                forEachWithKey( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctorWithKey< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as argument to the member function. */
    template< typename FuncType >
    void                forEachMemWithKey( FuncType pFunc ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType >
    void                forEachMemWithKey( FuncType pFunc, ParamType aParam ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    void                forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    void                forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2, aParam3 ) );
                        }

private:
    template< typename FunctorType >
    struct ForEachFunctor
    {
        FunctorType         maFunctor;
        explicit     ForEachFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( *rValue.second ); }
    };

    template< typename FunctorType >
    struct ForEachFunctorWithKey
    {
        FunctorType         maFunctor;
        explicit     ForEachFunctorWithKey( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( rValue.first, *rValue.second ); }
    };

    const mapped_type* getRef( key_type nKey ) const
    {
        typename container_type::const_iterator aIt = this->find( nKey );
        return (aIt == this->end()) ? 0 : &aIt->second;
    }
};

// ============================================================================

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
