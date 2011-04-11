/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    /** Returns true, if the object accossiated to the passed key exists.
        Returns false, if the key exists but points to an empty reference. */
    inline bool         has( key_type nKey ) const
                        {
                            const mapped_type* pxRef = getRef( nKey );
                            return pxRef && pxRef->get();
                        }

    /** Returns a reference to the object accossiated to the passed key, or an
        empty reference on error. */
    inline mapped_type  get( key_type nKey ) const
                        {
                            if( const mapped_type* pxRef = getRef( nKey ) ) return *pxRef;
                            return mapped_type();
                        }

    /** Calls the passed functor for every contained object, automatically
        skips all elements that are empty references. */
    template< typename FunctorType >
    inline void         forEach( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctor< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType >
    inline void         forEachMem( FuncType pFunc ) const
                        {
                            forEach( ::boost::bind( pFunc, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMem( FuncType pFunc, ParamType aParam ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object,
        automatically skips all elements that are empty references. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3, typename ParamType4 >
    inline void         forEachMem( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3, ParamType4 aParam4 ) const
                        {
                            forEach( ::boost::bind( pFunc, _1, aParam1, aParam2, aParam3, aParam4 ) );
                        }


    /** Calls the passed functor for every contained object. Passes the key as
        first argument and the object reference as second argument to rFunctor. */
    template< typename FunctorType >
    inline void         forEachWithKey( const FunctorType& rFunctor ) const
                        {
                            ::std::for_each( this->begin(), this->end(), ForEachFunctorWithKey< FunctorType >( rFunctor ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as argument to the member function. */
    template< typename FuncType >
    inline void         forEachMemWithKey( FuncType pFunc ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType aParam ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2 >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2 ) );
                        }

    /** Calls the passed member function of ObjType on every contained object.
        Passes the object key as first argument to the member function. */
    template< typename FuncType, typename ParamType1, typename ParamType2, typename ParamType3 >
    inline void         forEachMemWithKey( FuncType pFunc, ParamType1 aParam1, ParamType2 aParam2, ParamType3 aParam3 ) const
                        {
                            forEachWithKey( ::boost::bind( pFunc, _2, _1, aParam1, aParam2, aParam3 ) );
                        }

private:
    template< typename FunctorType >
    struct ForEachFunctor
    {
        FunctorType         maFunctor;
        inline explicit     ForEachFunctor( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( *rValue.second ); }
    };

    template< typename FunctorType >
    struct ForEachFunctorWithKey
    {
        FunctorType         maFunctor;
        inline explicit     ForEachFunctorWithKey( const FunctorType& rFunctor ) : maFunctor( rFunctor ) {}
        inline void         operator()( const value_type& rValue ) { if( rValue.second.get() ) maFunctor( rValue.first, *rValue.second ); }
    };

    inline const mapped_type* getRef( key_type nKey ) const
                        {
                            typename container_type::const_iterator aIt = find( nKey );
                            return (aIt == this->end()) ? 0 : &aIt->second;
                        }
};

// ============================================================================

} // namespace oox

#endif
