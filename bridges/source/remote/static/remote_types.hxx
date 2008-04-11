/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote_types.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _BRIDGES_REMOTE_TYPES_HXX_
#define _BRIDGES_REMOTE_TYPES_HXX_

#include <osl/diagnose.h>
#include <sal/types.h>
#include <typelib/typedescription.h>

namespace bridges_remote
{

inline sal_Bool SAL_CALL remote_relatesToInterface( typelib_TypeDescription *pTypeDescr );
sal_Bool SAL_CALL remote_relatesToInterface2( typelib_TypeDescription * pTypeDescr );


/** Determines whether given type might relate or relates to an interface,
    i.e. values of this type are interface or may contain interface(s).<br>
    @param pTypeDescr type description of type
    @return true if type might relate to an interface, false otherwise
*/
inline sal_Bool SAL_CALL remote_relatesToInterface( typelib_TypeDescription * pTypeDescr )
{
    switch (pTypeDescr->eTypeClass)
    {
    case typelib_TypeClass_SEQUENCE:
        switch (((typelib_IndirectTypeDescription *)pTypeDescr)->pType->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_UNION: // might relate to interface
        case typelib_TypeClass_ANY: // might relate to interface
            return sal_True;
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            return remote_relatesToInterface2( pTypeDescr );
        default:
            return sal_False;
        }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        return remote_relatesToInterface2( pTypeDescr );
    case typelib_TypeClass_UNION: // might relate to interface
    case typelib_TypeClass_ANY: // might relate to interface
    case typelib_TypeClass_INTERFACE:
        return sal_True;
    default:
        return sal_False;
    }
}

/** Determines whether given type is a cpp simple type, e.g. int, enum.<br>
    @param pTypeDescr type description of type
    @return true if type is a cpp simple type, false otherwise
*/
inline sal_Bool SAL_CALL remote_isSimpleType( typelib_TypeDescription * pTypeDescr )
{
    return (pTypeDescr->eTypeClass <= typelib_TypeClass_ENUM &&
            pTypeDescr->eTypeClass != typelib_TypeClass_STRING &&
            pTypeDescr->eTypeClass != typelib_TypeClass_ANY &&
            pTypeDescr->eTypeClass != typelib_TypeClass_TYPE);
}

}
#endif
