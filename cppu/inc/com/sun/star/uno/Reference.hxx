/*************************************************************************
 *
 *  $RCSfile: Reference.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-20 15:14:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#define _COM_SUN_STAR_UNO_REFERENCE_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HDL_
#include <com/sun/star/uno/XInterface.hdl>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#include <com/sun/star/uno/genfunc.hxx>
#endif

/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( XInterface * pInterface ) const SAL_THROW( () )
{
    if (_pInterface == pInterface)
        return sal_True;
    // only the query to XInterface must return the same pointer if they belong to same objects
    Reference< XInterface > x1( _pInterface, UNO_QUERY );
    Reference< XInterface > x2( pInterface, UNO_QUERY );
    return (x1.get() == x2.get());
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( XInterface * pInterface ) const SAL_THROW( () )
{
    return (! operator == ( pInterface ));
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( const BaseReference & rRef ) const SAL_THROW( () )
{
    return operator == ( rRef.get() );
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator != ( const BaseReference & rRef ) const SAL_THROW( () )
{
    return (! operator == ( rRef.get() ));
}

//##################################################################################################

//__________________________________________________________________________________________________
template< class interface_type >
inline interface_type * Reference< interface_type >::__query(
    XInterface * pInterface ) SAL_THROW( (RuntimeException) )
{
    if (pInterface)
    {
        const Type & rType = ::getCppuType( (const Reference< interface_type > *)0 );
        Any aRet( pInterface->queryInterface( rType ) );
        if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
        {
            interface_type * pRet = * reinterpret_cast< interface_type ** >( aRet.pData );
            * reinterpret_cast< interface_type ** >( aRet.pData ) = 0;
            return pRet;
        }
    }
    return 0;
}

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
    _pInterface = pInterface;
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, __sal_NoAcquire ) SAL_THROW( () )
{
    _pInterface = pInterface;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( interface_type * pInterface, __UnoReference_NoAcquire ) SAL_THROW( () )
{
    _pInterface = pInterface;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( const BaseReference & rRef, __UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    _pInterface = __query( rRef.get() );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type >::Reference( XInterface * pInterface, __UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    _pInterface = __query( pInterface );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline void Reference< interface_type >::clear() SAL_THROW( () )
{
    if (_pInterface)
    {
        _pInterface->release();
        _pInterface = 0;
    }
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface ) SAL_THROW( () )
{
    if (pInterface != _pInterface)
    {
        if (pInterface)
            pInterface->acquire();
        if (_pInterface)
            _pInterface->release();
        _pInterface = pInterface;
    }
    return (pInterface != 0);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface, __sal_NoAcquire ) SAL_THROW( () )
{
    if (_pInterface)
        _pInterface->release();
    _pInterface = pInterface;
    return (pInterface != 0);
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    interface_type * pInterface, __UnoReference_NoAcquire ) SAL_THROW( () )
{
    return set( pInterface, SAL_NO_ACQUIRE );
}

//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    const Reference< interface_type > & rRef ) SAL_THROW( () )
{
    return set( rRef.get() );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    XInterface * pInterface, __UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    return set( __query( pInterface ), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline sal_Bool Reference< interface_type >::set(
    const BaseReference & rRef, __UnoReference_Query ) SAL_THROW( (RuntimeException) )
{
    return set( __query( rRef.get() ), SAL_NO_ACQUIRE );
}

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
    set( rRef.get() );
    return *this;
}

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    const BaseReference & rRef ) SAL_THROW( (RuntimeException) )
{
    return Reference< interface_type >( __query( rRef.get() ), SAL_NO_ACQUIRE );
}
//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > Reference< interface_type >::query(
    XInterface * pInterface ) SAL_THROW( (RuntimeException) )
{
    return Reference< interface_type >( __query( pInterface ), SAL_NO_ACQUIRE );
}

}
}
}
}

#endif
