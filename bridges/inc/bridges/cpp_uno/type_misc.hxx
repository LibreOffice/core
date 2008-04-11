/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: type_misc.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _BRIDGES_CPP_UNO_TYPE_MISC_HXX_
#define _BRIDGES_CPP_UNO_TYPE_MISC_HXX_

#include <sal/types.h>
#include <typelib/typedescription.h>


/** Determines whether given type might relate or relates to an interface,
    i.e. values of this type are interface or may contain interface(s).<br>
    @param pTypeDescr type description of type
    @return true if type might relate to an interface, false otherwise
*/
inline bool cppu_relatesToInterface( typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    switch (pTypeDescr->eTypeClass)
    {
//      case typelib_TypeClass_TYPEDEF:
    case typelib_TypeClass_SEQUENCE:
    {
        switch (((typelib_IndirectTypeDescription *)pTypeDescr)->pType->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_UNION: // might relate to interface
        case typelib_TypeClass_ANY: // might relate to interface
            return true;
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );
            bool bRel = cppu_relatesToInterface( pTD );
            TYPELIB_DANGER_RELEASE( pTD );
            return bRel;
        }
        default:
            return false;
        }
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        // ...optimized... to avoid getDescription() calls!
        typelib_CompoundTypeDescription * pComp    = (typelib_CompoundTypeDescription *)pTypeDescr;
        typelib_TypeDescriptionReference ** pTypes = pComp->ppTypeRefs;
        for ( sal_Int32 nPos = pComp->nMembers; nPos--; )
        {
            switch (pTypes[nPos]->eTypeClass)
            {
            case typelib_TypeClass_INTERFACE:
            case typelib_TypeClass_UNION: // might relate to interface
            case typelib_TypeClass_ANY: // might relate to interface
                return true;
//              case typelib_TypeClass_TYPEDEF:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, pTypes[nPos] );
                bool bRel = cppu_relatesToInterface( pTD );
                TYPELIB_DANGER_RELEASE( pTD );
                if (bRel)
                    return true;
            }
            default:
                break;
            }
        }
        if (pComp->pBaseTypeDescription)
            return cppu_relatesToInterface( (typelib_TypeDescription *)pComp->pBaseTypeDescription );
        return false;
    }
    case typelib_TypeClass_UNION: // might relate to interface
    case typelib_TypeClass_ANY: // might relate to interface
    case typelib_TypeClass_INTERFACE:
        return true;
    default:
        return false;
    }
}

/** Determines whether given type is a cpp simple type, e.g. int, enum.<br>
    @param eTypeClass type class of type
    @return true if type is a cpp simple type, false otherwise
*/
inline bool cppu_isSimpleType( typelib_TypeClass eTypeClass ) SAL_THROW( () )
{
    return (eTypeClass <= typelib_TypeClass_ENUM &&
            eTypeClass != typelib_TypeClass_STRING &&
            eTypeClass != typelib_TypeClass_ANY &&
            eTypeClass != typelib_TypeClass_TYPE);
}
/** Determines whether given type is a cpp simple type, e.g. int, enum.<br>
    @param pTypeDescr type description of type
    @return true if type is a cpp simple type, false otherwise
*/
inline bool cppu_isSimpleType( typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    return cppu_isSimpleType( pTypeDescr->eTypeClass );
}

#endif
