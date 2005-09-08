/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genfunc.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:36:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#define _COM_SUN_STAR_UNO_GENFUNC_HXX_

#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#include <com/sun/star/uno/genfunc.h>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

//==================================================================================================
inline void SAL_CALL cpp_acquire( void * pCppI )
    SAL_THROW( () )
{
    reinterpret_cast< XInterface * >( pCppI )->acquire();
}
//==================================================================================================
inline void SAL_CALL cpp_release( void * pCppI )
    SAL_THROW( () )
{
    reinterpret_cast< XInterface * >( pCppI )->release();
}
//==================================================================================================
inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () )
{
    if (pCppI)
    {
#ifndef EXCEPTIONS_OFF
        try
        {
#endif
            Any aRet( reinterpret_cast< XInterface * >( pCppI )->queryInterface(
                * reinterpret_cast< const Type * >( &pType ) ) );
            if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
            {
                XInterface * pRet = reinterpret_cast< XInterface * >( aRet.pReserved );
                aRet.pReserved = 0;
                return pRet;
            }
#ifndef EXCEPTIONS_OFF
        }
        catch (RuntimeException &)
        {
        }
#endif
    }
    return 0;
}

}
}
}
}

#endif
