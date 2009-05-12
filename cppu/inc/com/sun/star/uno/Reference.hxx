/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Reference.hxx,v $
 * $Revision: 1.27 $
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
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#define _COM_SUN_STAR_UNO_REFERENCE_HXX_

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HDL_
#include <com/sun/star/uno/XInterface.hdl>
#endif
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
#ifndef EXCEPTIONS_OFF
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
        pInterface->acquire();
        return pInterface;
    }
    throw RuntimeException(
        ::rtl::OUString( cppu_unsatisfied_iset_msg( interface_type::static_type().getTypeLibType() ), SAL_NO_ACQUIRE ),
        NULL );
}
#endif

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::~Reference() SAL_THROW( () )
{
    if (_pInterface)
        _pInterface->release();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference() SAL_THROW( () )
{
    _pInterface = 0;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const Reference< interface_type > & rRef ) SAL_THROW( () )
{
    _pInterface = rRef._pInterface;
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface ) SAL_THROW( () )
{
    _pInterface = castToXInterface(pInterface);
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, __sal_NoAcquire ) SAL_THROW( () )
{
    _pInterface = castToXInterface(pInterface);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_NoAcquire ) SAL_THROW( () )
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
#ifndef EXCEPTIONS_OFF
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
    _pInterface = iset_throw( rRef.get() );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, UnoReference_SetThrow ) SAL_THROW( (RuntimeException) )
{
    _pInterface = iset_throw( pInterface );
}
#endif

//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::clear() SAL_THROW( () )
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
    interface_type * pInterface ) SAL_THROW( () )
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
    interface_type * pInterface, __sal_NoAcquire ) SAL_THROW( () )
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
    interface_type * pInterface, UnoReference_NoAcquire ) SAL_THROW( () )
{
    return set( pInterface, SAL_NO_ACQUIRE );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    const Reference< interface_type > & rRef ) SAL_THROW( () )
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

#ifndef EXCEPTIONS_OFF
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

#endif

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    interface_type * pInterface ) SAL_THROW( () )
{
    set( pInterface );
    return *this;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    const Reference< interface_type > & rRef ) SAL_THROW( () )
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
inline sal_Bool BaseReference::operator == ( XInterface * pInterface ) const SAL_THROW( () )
{
    if (_pInterface == pInterface)
        return sal_True;
#ifndef EXCEPTIONS_OFF
    try
    {
#endif
        // only the query to XInterface must return the same pointer if they belong to same objects
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( pInterface, UNO_QUERY );
        return (x1._pInterface == x2._pInterface);
#ifndef EXCEPTIONS_OFF
    }
    catch (RuntimeException &)
    {
        return sal_False;
    }
#endif
}

//______________________________________________________________________________
inline sal_Bool BaseReference::operator < (
    const BaseReference & rRef ) const SAL_THROW( () )
{
    if (_pInterface == rRef._pInterface)
        return sal_False;
#if ! defined EXCEPTIONS_OFF
    try
    {
#endif
        // only the query to XInterface must return the same pointer:
        Reference< XInterface > x1( _pInterface, UNO_QUERY );
        Reference< XInterface > x2( rRef, UNO_QUERY );
        return (x1._pInterface < x2._pInterface);
#if ! defined EXCEPTIONS_OFF
    }
    catch (RuntimeException &)
    {
        return sal_False;
    }
#endif
}

//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( XInterface * pInterface ) const SAL_THROW( () )
{
    return (! operator == ( pInterface ));
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( const BaseReference & rRef ) const SAL_THROW( () )
{
    return operator == ( rRef._pInterface );
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( const BaseReference & rRef ) const SAL_THROW( () )
{
    return (! operator == ( rRef._pInterface ));
}

}
}
}
}

#endif
