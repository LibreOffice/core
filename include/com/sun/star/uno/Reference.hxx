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
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#define _COM_SUN_STAR_UNO_REFERENCE_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/genfunc.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

//__________________________________________________________________________________________________
inline XInterface * BaseReference::iquery(
    XInterface * pInterface, const Type & rType )
    SAL_THROW( (RuntimeException) )
{
    if (pInterface)
    {
        Any aRet( pInterface->queryInterface( rType ) );
        if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
        {
            XInterface * pRet = static_cast< XInterface * >( aRet.pReserved );
            aRet.pReserved = 0;
            return pRet;
        }
    }
    return 0;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline XInterface * Reference< interface_type >::iquery(
    XInterface * pInterface ) SAL_THROW( (RuntimeException) )
{
    return BaseReference::iquery(pInterface, interface_type::static_type());
}
extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iquery_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();
extern "C" rtl_uString * SAL_CALL cppu_unsatisfied_iset_msg(
    typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C();
//__________________________________________________________________________________________________
inline XInterface * BaseReference::iquery_throw(
    XInterface * pInterface, const Type & rType )
    SAL_THROW( (RuntimeException) )
{
    XInterface * pQueried = iquery( pInterface, rType );
    if (pQueried)
        return pQueried;
    throw RuntimeException(
        ::rtl::OUString( cppu_unsatisfied_iquery_msg( rType.getTypeLibType() ), SAL_NO_ACQUIRE ),
        Reference< XInterface >( pInterface ) );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline XInterface * Reference< interface_type >::iquery_throw(
    XInterface * pInterface ) SAL_THROW( (RuntimeException) )
{
    return BaseReference::iquery_throw(
        pInterface, interface_type::static_type());
}
//__________________________________________________________________________________________________
template< class interface_type >
inline interface_type * Reference< interface_type >::iset_throw(
    interface_type * pInterface ) SAL_THROW( (RuntimeException) )
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

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::~Reference() SAL_THROW(())
{
    if (_pInterface)
        _pInterface->release();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference() SAL_THROW(())
{
    _pInterface = 0;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const Reference< interface_type > & rRef ) SAL_THROW(())
{
    _pInterface = rRef._pInterface;
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type > template< class derived_type >
inline Reference< interface_type >::Reference(
    const Reference< derived_type > & rRef,
    typename detail::UpCast< interface_type, derived_type >::t )
    SAL_THROW(())
{
    interface_type * p = rRef.get();
    _pInterface = p;
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface ) SAL_THROW(())
{
    _pInterface = castToXInterface(pInterface);
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, __sal_NoAcquire ) SAL_THROW(())
{
    _pInterface = castToXInterface(pInterface);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_NoAcquire ) SAL_THROW(())
{
    _pInterface = castToXInterface(pInterface);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const BaseReference & rRef, UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iquery( rRef.get() );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( XInterface * pInterface, UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iquery( pInterface );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const Any & rAny, UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    _pInterface = (typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass
                   ? iquery( static_cast< XInterface * >( rAny.pReserved ) ) : 0);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const BaseReference & rRef, UnoReference_QueryThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iquery_throw( rRef.get() );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( XInterface * pInterface, UnoReference_QueryThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iquery_throw( pInterface );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const Any & rAny, UnoReference_QueryThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iquery_throw( typelib_TypeClass_INTERFACE == rAny.pType->eTypeClass
                                ? static_cast< XInterface * >( rAny.pReserved ) : 0 );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const Reference< interface_type > & rRef, UnoReference_SetThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = castToXInterface( iset_throw( rRef.get() ) );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_SetThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = castToXInterface( iset_throw( pInterface ) );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::clear() SAL_THROW(())
{
    if (_pInterface)
    {
        XInterface * const pOld = _pInterface;
        _pInterface = 0;
        pOld->release();
    }
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface ) SAL_THROW(())
{
    if (pInterface)
        castToXInterface(pInterface)->acquire();
    XInterface * const pOld = _pInterface;
    _pInterface = castToXInterface(pInterface);
    if (pOld)
        pOld->release();
    return (0 != pInterface);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface, __sal_NoAcquire ) SAL_THROW(())
{
    XInterface * const pOld = _pInterface;
    _pInterface = castToXInterface(pInterface);
    if (pOld)
        pOld->release();
    return (0 != pInterface);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface, UnoReference_NoAcquire ) SAL_THROW(())
{
    return set( pInterface, SAL_NO_ACQUIRE );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    const Reference< interface_type > & rRef ) SAL_THROW(())
{
    return set( castFromXInterface( rRef._pInterface ) );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    XInterface * pInterface, UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    return set( castFromXInterface(iquery( pInterface )), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    const BaseReference & rRef, UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    return set( castFromXInterface(iquery( rRef.get() )), SAL_NO_ACQUIRE );
}

//______________________________________________________________________________
template< class interface_type >
inline bool Reference< interface_type >::set(
    Any const & rAny, UnoReference_Query )
{
    return set(
        castFromXInterface(
            iquery(
                rAny.pType->eTypeClass == typelib_TypeClass_INTERFACE
                ? static_cast< XInterface * >( rAny.pReserved ) : 0 )),
        SAL_NO_ACQUIRE );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::set(
    XInterface * pInterface, UnoReference_QueryThrow ) SAL_THROW( (RuntimeException) )
{
    set( castFromXInterface(iquery_throw( pInterface )), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::set(
    const BaseReference & rRef, UnoReference_QueryThrow ) SAL_THROW( (RuntimeException) )
{
    set( castFromXInterface(iquery_throw( rRef.get() )), SAL_NO_ACQUIRE );
}

//______________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::set(
    Any const & rAny, UnoReference_QueryThrow )
{
    set( castFromXInterface(
             iquery_throw(
                 rAny.pType->eTypeClass == typelib_TypeClass_INTERFACE
                 ? static_cast< XInterface * >( rAny.pReserved ) : 0 )),
         SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::set(
    interface_type * pInterface, UnoReference_SetThrow ) SAL_THROW( (RuntimeException) )
{
    set( iset_throw( pInterface ), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::set(
    const Reference< interface_type > & rRef, UnoReference_SetThrow ) SAL_THROW( (RuntimeException) )
{
    set( rRef.get(), UNO_SET_THROW );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    interface_type * pInterface ) SAL_THROW(())
{
    set( pInterface );
    return *this;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    const Reference< interface_type > & rRef ) SAL_THROW(())
{
    set( castFromXInterface( rRef._pInterface ) );
    return *this;
}

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    const BaseReference & rRef ) SAL_THROW( (RuntimeException) )
{
    return Reference< interface_type >(
        castFromXInterface(iquery( rRef.get() )), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    XInterface * pInterface ) SAL_THROW( (RuntimeException) )
{
    return Reference< interface_type >(
        castFromXInterface(iquery( pInterface )), SAL_NO_ACQUIRE );
}

//##################################################################################################

//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( XInterface * pInterface ) const SAL_THROW(())
{
    if (_pInterface == pInterface)
        return sal_True;
    try
    {
        // only the query to XInterface must return the same pointer if they belong to same objects
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( pInterface, UNO_QUERY );
        return (x1._pInterface == x2._pInterface);
    }
    catch (RuntimeException &)
    {
        return sal_False;
    }
}

//______________________________________________________________________________
inline sal_Bool BaseReference::operator < (
    const BaseReference & rRef ) const SAL_THROW(())
{
    if (_pInterface == rRef._pInterface)
        return sal_False;
    try
    {
        // only the query to XInterface must return the same pointer:
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( rRef, UNO_QUERY );
        return (x1._pInterface < x2._pInterface);
    }
    catch (RuntimeException &)
    {
        return sal_False;
    }
}

//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( XInterface * pInterface ) const SAL_THROW(())
{
    return (! operator == ( pInterface ));
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( const BaseReference & rRef ) const SAL_THROW(())
{
    return operator == ( rRef._pInterface );
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( const BaseReference & rRef ) const SAL_THROW(())
{
    return (! operator == ( rRef._pInterface ));
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
