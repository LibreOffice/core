/*************************************************************************
 *
 *  $RCSfile: Reference.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-16 16:38:07 $
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
inline BaseReference::BaseReference() throw ()
    : _pInterface( 0 )
{
}
//__________________________________________________________________________________________________
inline BaseReference::BaseReference( XInterface * pInterface ) throw ()
    : _pInterface( pInterface )
{
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
inline BaseReference::BaseReference( XInterface * pInterface, __UnoReference_NoAcquire ) throw ()
    : _pInterface( pInterface )
{
}
//__________________________________________________________________________________________________
inline BaseReference::BaseReference( const BaseReference & rRef ) throw ()
    : _pInterface( rRef._pInterface )
{
    if (_pInterface)
        _pInterface->acquire();
}
//__________________________________________________________________________________________________
inline BaseReference::~BaseReference() throw ()
{
    if (_pInterface)
        _pInterface->release();
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::operator == ( const BaseReference & rRef ) const throw ()
{
    if (_pInterface == rRef._pInterface)
        return sal_True;
    // only the query to XInterface must return the same pointer if they belong to same objects
    Reference< XInterface > x1( _pInterface, UNO_QUERY );
    Reference< XInterface > x2( rRef.get(), UNO_QUERY );
    return (x1.get() == x2.get());
}
//__________________________________________________________________________________________________
inline sal_Bool BaseReference::set( XInterface * pInterface ) throw ()
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
inline sal_Bool BaseReference::set( XInterface * pInterface, __UnoReference_NoAcquire ) throw ()
{
    if (_pInterface)
        _pInterface->release();
    _pInterface = pInterface;
    return (pInterface != 0);
}
//__________________________________________________________________________________________________
inline void BaseReference::clear() throw ()
{
    if (_pInterface)
    {
        _pInterface->release();
        _pInterface = 0;
    }
}

//__________________________________________________________________________________________________
template< class interface_type >
inline Reference< interface_type > & Reference< interface_type >::operator = (
    interface_type * pInterface ) throw ()
{
    BaseReference::set( pInterface );
    return *this;
}
//__________________________________________________________________________________________________
template< class interface_type >
inline XInterface * Reference< interface_type >::__query(
    XInterface * pInterface ) throw (RuntimeException)
{
    if (pInterface)
    {
        const Type & rType = ::getCppuType( (const Reference< interface_type > *)0 );
        Any aRet( pInterface->queryInterface( rType ) );
        if (aRet.hasValue())
        {
            XInterface * pRet = * reinterpret_cast< XInterface * const * >( aRet.getValue() );
            pRet->acquire();
            return pRet;
        }
    }
    return 0;
}

}
}
}
}

#endif
