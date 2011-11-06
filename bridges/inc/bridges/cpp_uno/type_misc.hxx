/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
