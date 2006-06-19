/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: type_misc.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:37:51 $
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
#ifndef _BRIDGES_CPP_UNO_TYPE_MISC_HXX_
#define _BRIDGES_CPP_UNO_TYPE_MISC_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif


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
