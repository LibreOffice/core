/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: genfunc.h,v $
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
#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#define _COM_SUN_STAR_UNO_GENFUNC_H_

#include "sal/types.h"

typedef struct _typelib_TypeDescriptionReference typelib_TypeDescriptionReference;

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Function to acquire a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_acquire( void * pCppI )
    SAL_THROW( () );
/** Function to release a C++ interface.

    @param pCppI C++ interface pointer
*/
inline void SAL_CALL cpp_release( void * pCppI )
    SAL_THROW( () );
/** Function to query for a C++ interface.

    @param pCppI C++ interface pointer
    @param pType demanded interface type
    @return acquired C++ interface pointer or null
*/
inline void * SAL_CALL cpp_queryInterface( void * pCppI, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () );

}
}
}
}

#endif
