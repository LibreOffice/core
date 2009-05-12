/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: genfunc.hxx,v $
 * $Revision: 1.12 $
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
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#define _COM_SUN_STAR_UNO_GENFUNC_HXX_

#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/uno/Any.hxx>


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
