/*************************************************************************
 *
 *  $RCSfile: constr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-30 13:41:39 $
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
#ifndef __CONSTR_HXX__
#define __CONSTR_HXX__

#include "prim.hxx"

namespace cppu
{

//##################################################################################################
//#### construction ################################################################################
//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline void __defaultConstructUnion(
    void * pMem,
    typelib_TypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    ::uno_type_constructData(
        (char *)pMem + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
        ((typelib_UnionTypeDescription *)pTypeDescr)->pDefaultTypeRef );
    *(sal_Int64 *)pMem = ((typelib_UnionTypeDescription *)pTypeDescr)->nDefaultDiscriminant;
}
//==================================================================================================
void defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pCompType )
    SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
inline void __defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        defaultConstructStruct( pMem, pTypeDescr->pBaseTypeDescription );
    }

    typelib_TypeDescriptionReference ** ppTypeRefs = (pTypeDescr)->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;

    while (nDescr--)
    {
        ::uno_type_constructData( (char *)pMem + pMemberOffsets[nDescr], ppTypeRefs[nDescr] );
    }
}

//--------------------------------------------------------------------------------------------------
inline void __defaultConstructArray(
    void * pMem,
    typelib_ArrayTypeDescription * pTypeDescr )
{
    typelib_TypeDescription * pElementType = NULL;
    TYPELIB_DANGER_GET( &pElementType, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );
    sal_Int32 nTotalElements = pTypeDescr->nTotalElements;
    sal_Int32 nElementSize = pElementType->nSize;
    sal_Int32 i;
    switch ( pElementType->eTypeClass )
    {
    case typelib_TypeClass_CHAR:
    case typelib_TypeClass_BOOLEAN:
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
    case typelib_TypeClass_FLOAT:
    case typelib_TypeClass_DOUBLE:
    case typelib_TypeClass_INTERFACE:
        ::rtl_zeroMemory(pMem, nElementSize * nTotalElements);
        break;

    case typelib_TypeClass_STRING:
        for (i=0; i < nTotalElements; i++)
        {
            rtl_uString** ppElement = (rtl_uString **)pMem + i;
            *ppElement = 0;
            RTL_USTRING_NEW( ppElement);
        }
        break;
    case typelib_TypeClass_TYPE:
        for (i=0; i < nTotalElements; i++)
        {
            typelib_TypeDescriptionReference** ppElement = (typelib_TypeDescriptionReference **)pMem + i;
            *ppElement = __getVoidType();
        }
        break;
    case typelib_TypeClass_ANY:
        for (i=0; i < nTotalElements; i++)
        {
            __CONSTRUCT_EMPTY_ANY( (uno_Any *)pMem + i );
        }
        break;
    case typelib_TypeClass_ENUM:
        for (i=0; i < nTotalElements; i++)
        {
            *((int *)pMem + i) = ((typelib_EnumTypeDescription *)pElementType)->nDefaultEnumValue;
        }
        break;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        for (i=0; i < nTotalElements; i++)
        {
            __defaultConstructStruct( (sal_Char*)pMem + i * nElementSize, (typelib_CompoundTypeDescription *)pElementType );
        }
        break;
    case typelib_TypeClass_UNION:
        for (i=0; i < nTotalElements; i++)
        {
            __defaultConstructUnion( (sal_Char*)pMem + i * nElementSize, pElementType );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        for (i=0; i < nTotalElements; i++)
        {
            uno_Sequence** ppElement = (uno_Sequence **)pMem + i;
            *ppElement = __getEmptySequence();
        }
        break;
    }
    TYPELIB_DANGER_RELEASE( pElementType );
}

//--------------------------------------------------------------------------------------------------
inline void __defaultConstructData(
    void * pMem,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *(sal_Unicode *)pMem = '\0';
        break;
    case typelib_TypeClass_BOOLEAN:
        *(sal_Bool *)pMem = sal_False;
        break;
    case typelib_TypeClass_BYTE:
        *(sal_Int8 *)pMem = 0;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *(sal_Int16 *)pMem = 0;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *(sal_Int32 *)pMem = 0;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        *(sal_Int64 *)pMem = 0;
        break;
    case typelib_TypeClass_FLOAT:
        *(float *)pMem = 0.0;
        break;
    case typelib_TypeClass_DOUBLE:
        *(double *)pMem = 0.0;
        break;
    case typelib_TypeClass_STRING:
        *(rtl_uString **)pMem = 0;
        RTL_USTRING_NEW( (rtl_uString **)pMem );
        break;
    case typelib_TypeClass_TYPE:
        *(typelib_TypeDescriptionReference **)pMem = __getVoidType();
        break;
    case typelib_TypeClass_ANY:
        __CONSTRUCT_EMPTY_ANY( (uno_Any *)pMem );
        break;
    case typelib_TypeClass_ENUM:
        if (pTypeDescr)
        {
            *(int *)pMem = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            *(int *)pMem = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            __defaultConstructStruct( pMem, (typelib_CompoundTypeDescription *)pTypeDescr );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __defaultConstructStruct( pMem, (typelib_CompoundTypeDescription *)pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_ARRAY:
        if (pTypeDescr)
        {
            __defaultConstructArray( pMem, (typelib_ArrayTypeDescription *)pTypeDescr );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __defaultConstructArray( pMem, (typelib_ArrayTypeDescription *)pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            __defaultConstructUnion( pMem, pTypeDescr );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __defaultConstructUnion( pMem, pTypeDescr );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        *(uno_Sequence **)pMem = __getEmptySequence();
        break;
    case typelib_TypeClass_INTERFACE:
        *(void **)pMem = 0; // either cpp or c-uno interface
        break;
    }
}

}

#endif
