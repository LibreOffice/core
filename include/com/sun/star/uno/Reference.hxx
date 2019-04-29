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
#ifndef INCLUDED_COM_SUN_STAR_UNO_REFERENCE_HXX
#define INCLUDED_COM_SUN_STAR_UNO_REFERENCE_HXX

#include "sal/config.h"

#include <cstddef>
#include <ostream>

#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "cppu/cppudllapi.h"

extern "C" CPPU_DLLPUBLIC rtl_uString * SAL_CALL cppu_unsatisfied_iquery_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();
extern "C" CPPU_DLLPUBLIC rtl_uString * SAL_CALL cppu_unsatisfied_iset_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{


inline XInterface * BaseReference::iquery(
    XInterface * pInterface, const Type & rType )
{
    if (pInterface)
    {
        Any aRet( pInterface->queryInterface( rType ) );
        if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
        {
            XInterface * pRet = static_cast< XInterface * >( aRet.pReserved );
            aRet.pReserved = NULL;
            return pRet;
        }
    }
    return NULL;
}

template< class interface_type >
inline XInterface * Reference< interface_type >::iquery(
    XInterface * pInterface )
{
    return BaseReference::iquery(pInterface, interface_type::static_type());
}

inline XInterface * BaseReference::iquery_throw(
    XInterface * pInterface, const Type & rType )
{
    XInterface * pQueried = iquery( pInterface, rType );
    if (pQueried)
        return pQueried;
    throw RuntimeException(
        ::rtl::OUString( cppu_unsatisfied_iquery_msg( rType.getTypeLibType() ), SAL_NO_ACQUIRE ),
        Reference< XInterface >( pInterface ) );
}

template< class interface_type >
inline XInterface * Reference< interface_type >::iquery_throw(
    XInterface * pInterface )
{
    return BaseReference::iquery_throw(
        pInterface, interface_type::static_type());
}

template< class interface_type >
inline interface_type * Reference< interface_type >::iset_throw(
    interface_type * pInterface )
{
    if (pInterface)
    {
        castToXInterface(pInterface)->acquire();
        return pInterface;
    }
    throw RuntimeException(
        ::rtl::OUString( cppu_unsatisfied_iset_msg( interface_type::static_type().getTypeLibType() ), SAL_NO_ACQUIRE ),
        NULL );
}

template< class interface_type >
inline Reference< interface_type >::~Reference() COVERITY_NOEXCEPT_FALSE
{
    if (_pInterface)
        _pInterface->release();
}

template< class interface_type >
inline Reference< interface_type >::Reference()
{
    _pInterface = NULL;
}

template< class interface_type >
inline Reference< interface_type >::Reference( const Reference< interface_type > & rRef )
{
    _pInterface = rRef._pInterface;
    if (_pInterface)
        _pInterface->acquire();
}

#if defined LIBO_INTERNAL_ONLY
template< class interface_type >
inline Reference< interface_type >::Reference( Reference< interface_type > && rRef )
{
    _pInterface = rRef._pInterface;
    rRef._pInterface = nullptr;
}
#endif

template< class interface_type > template< class derived_type >
inline Reference< interface_type >::Reference(
    const Reference< derived_type > & rRef,
    typename detail::UpCast< interface_type, derived_type >::t )
{
    interface_type * p = rRef.get();
    _pInterface = p;
    if (_pInterface)
        _pInterface->acquire();
}

template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface )
{
    _pInterface = castToXInterface(pInterface);
    if (_pInterface)
        _pInterface->acquire();
}

template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, __sal_NoAcquire )
{
    _pInterface = castToXInterface(pInterface);
}

template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_NoAcquire )
{
    _pInterface = castToXInterface(pInterface);
}

template< class interface_type >
inline Reference< interface_type >::Reference( const BaseReference & rRef, UnoReference_Query )
{
    _pInterface = iquery( rRef.get() );
}

template< class interface_type >
inline Reference< interface_type >::Reference( XInterface * pInterface, UnoReference_Query )
{
    _pInterface = iquery( pInterface );
}

template< class interface_type >
inline Reference< interface_type >::Reference( const Any & rAny, UnoReference_Query )
{
    _pInterface = (typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass
                   ? iquery( static_cast< XInterface * >( rAny.pReserved ) ) : NULL);
}

template< class interface_type >
inline Reference< interface_type >::Reference( const BaseReference & rRef, UnoReference_QueryThrow )
{
    _pInterface = iquery_throw( rRef.get() );
}

template< class interface_type >
inline Reference< interface_type >::Reference( XInterface * pInterface, UnoReference_QueryThrow )
{
    _pInterface = iquery_throw( pInterface );
}

template< class interface_type >
inline Reference< interface_type >::Reference( const Any & rAny, UnoReference_QueryThrow )
{
    _pInterface = iquery_throw( typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass
                                ? static_cast< XInterface * >( rAny.pReserved ) : NULL );
}

template< class interface_type >
inline Reference< interface_type >::Reference( const Reference< interface_type > & rRef, UnoReference_SetThrow )
{
    _pInterface = castToXInterface( iset_throw( rRef.get() ) );
}

template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_SetThrow )
{
    _pInterface = castToXInterface( iset_throw( pInterface ) );
}


template< class interface_type >
inline void Reference< interface_type >::clear()
{
    if (_pInterface)
    {
        XInterface * const pOld = _pInterface;
        _pInterface = NULL;
        pOld->release();
    }
}

template< class interface_type >
inline bool Reference< interface_type >::set(
    interface_type * pInterface )
{
    if (pInterface)
        castToXInterface(pInterface)->acquire();
    XInterface * const pOld = _pInterface;
    _pInterface = castToXInterface(pInterface);
    if (pOld)
        pOld->release();
    return (NULL != pInterface);
}

template< class interface_type >
inline bool Reference< interface_type >::set(
    interface_type * pInterface, __sal_NoAcquire )
{
    XInterface * const pOld = _pInterface;
    _pInterface = castToXInterface(pInterface);
    if (pOld)
        pOld->release();
    return (NULL != pInterface);
}

template< class interface_type >
inline bool Reference< interface_type >::set(
    interface_type * pInterface, UnoReference_NoAcquire )
{
    return set( pInterface, SAL_NO_ACQUIRE );
}


template< class interface_type >
inline bool Reference< interface_type >::set(
    const Reference< interface_type > & rRef )
{
    return set( castFromXInterface( rRef._pInterface ) );
}

template< class interface_type >
inline bool Reference< interface_type >::set(
    XInterface * pInterface, UnoReference_Query )
{
    return set( castFromXInterface(iquery( pInterface )), SAL_NO_ACQUIRE );
}

template< class interface_type >
inline bool Reference< interface_type >::set(
    const BaseReference & rRef, UnoReference_Query )
{
    return set( castFromXInterface(iquery( rRef.get() )), SAL_NO_ACQUIRE );
}


template< class interface_type >
inline bool Reference< interface_type >::set(
    Any const & rAny, UnoReference_Query )
{
    return set(
        castFromXInterface(
            iquery(
                rAny.pType->eTypeClass == typelib_TypeClass_INTERFACE
                ? static_cast< XInterface * >( rAny.pReserved ) : NULL )),
        SAL_NO_ACQUIRE );
}


template< class interface_type >
inline void Reference< interface_type >::set(
    XInterface * pInterface, UnoReference_QueryThrow )
{
    set( castFromXInterface(iquery_throw( pInterface )), SAL_NO_ACQUIRE );
}

template< class interface_type >
inline void Reference< interface_type >::set(
    const BaseReference & rRef, UnoReference_QueryThrow )
{
    set( castFromXInterface(iquery_throw( rRef.get() )), SAL_NO_ACQUIRE );
}


template< class interface_type >
inline void Reference< interface_type >::set(
    Any const & rAny, UnoReference_QueryThrow )
{
    set( castFromXInterface(
             iquery_throw(
                 rAny.pType->eTypeClass == typelib_TypeClass_INTERFACE
                 ? static_cast< XInterface * >( rAny.pReserved ) : NULL )),
         SAL_NO_ACQUIRE );
}

template< class interface_type >
inline void Reference< interface_type >::set(
    interface_type * pInterface, UnoReference_SetThrow )
{
    set( iset_throw( pInterface ), SAL_NO_ACQUIRE );
}

template< class interface_type >
inline void Reference< interface_type >::set(
    const Reference< interface_type > & rRef, UnoReference_SetThrow )
{
    set( rRef.get(), UNO_SET_THROW );
}


template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    interface_type * pInterface )
{
    set( pInterface );
    return *this;
}

template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    const Reference< interface_type > & rRef )
{
    set( castFromXInterface( rRef._pInterface ) );
    return *this;
}

#if defined LIBO_INTERNAL_ONLY
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
     Reference< interface_type > && rRef )
{
    if (_pInterface)
        _pInterface->release();
    _pInterface = rRef._pInterface;
    rRef._pInterface = nullptr;
    return *this;
}
#endif

template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    const BaseReference & rRef )
{
    return Reference< interface_type >(
        castFromXInterface(iquery( rRef.get() )), SAL_NO_ACQUIRE );
}

template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    XInterface * pInterface )
{
    return Reference< interface_type >(
        castFromXInterface(iquery( pInterface )), SAL_NO_ACQUIRE );
}


inline bool BaseReference::operator == ( XInterface * pInterface ) const
{
    if (_pInterface == pInterface)
        return true;
    try
    {
        // only the query to XInterface must return the same pointer if they belong to same objects
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( pInterface, UNO_QUERY );
        return (x1._pInterface == x2._pInterface);
    }
    catch (RuntimeException &)
    {
        return false;
    }
}


inline bool BaseReference::operator < (
    const BaseReference & rRef ) const
{
    if (_pInterface == rRef._pInterface)
        return false;
    try
    {
        // only the query to XInterface must return the same pointer:
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( rRef, UNO_QUERY );
        return (x1._pInterface < x2._pInterface);
    }
    catch (RuntimeException &)
    {
        return false;
    }
}


inline bool BaseReference::operator != ( XInterface * pInterface ) const
{
    return (! operator == ( pInterface ));
}

inline bool BaseReference::operator == ( const BaseReference & rRef ) const
{
    return operator == ( rRef._pInterface );
}

inline bool BaseReference::operator != ( const BaseReference & rRef ) const
{
    return (! operator == ( rRef._pInterface ));
}

#if defined LIBO_INTERNAL_ONLY
/**
   Support for BaseReference in std::ostream (and thus in CPPUNIT_ASSERT or
   SAL_INFO macros, for example).

   @since LibreOffice 5.4
*/
template<typename charT, typename traits> std::basic_ostream<charT, traits> &
operator <<(
    std::basic_ostream<charT, traits> & stream, BaseReference const & ref)
{ return stream << ref.get(); }
#endif

}
}
}
}

#if defined LIBO_INTERNAL_ONLY
namespace std
{

/**
  Make css::uno::Reference hashable by default for use in STL containers.

  @since LibreOffice 6.3
*/
template<typename T>
struct hash<::css::uno::Reference<T>>
{
    std::size_t operator()(::css::uno::Reference<T> const & s) const
    { return size_t(s.get()); }
};

}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
