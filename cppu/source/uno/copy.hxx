/*************************************************************************
 *
 *  $RCSfile: copy.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:10:57 $
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
#ifndef __COPY_HXX__
#define __COPY_HXX__

#include "prim.hxx"
#include "constr.hxx"

namespace cppu
{


//##################################################################################################
//#### copy construction ###########################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
void copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () );
//--------------------------------------------------------------------------------------------------
inline void __copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        // copy base value
        copyConstructStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription, acquire, mapping );
    }

    // then copy members
    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;

    if (mapping)
    {
        while (nDescr--)
        {
            ::uno_type_copyAndConvertData( (char *)pDest + pMemberOffsets[nDescr],
                                           (char *)pSource + pMemberOffsets[nDescr],
                                           ppTypeRefs[nDescr], mapping );
        }
    }
    else
    {
        while (nDescr--)
        {
            ::uno_type_copyData( (char *)pDest + pMemberOffsets[nDescr],
                                 (char *)pSource + pMemberOffsets[nDescr],
                                 ppTypeRefs[nDescr], acquire );
        }
    }
}
//--------------------------------------------------------------------------------------------------
inline void __copyConstructUnion(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    typelib_TypeDescriptionReference * pSetType = __unionGetSetType( pSource, pTypeDescr );
    if (mapping)
    {
        ::uno_type_copyAndConvertData(
            (char *)pDest + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
            (char *)pSource + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
            pSetType, mapping );
    }
    else
    {
        ::uno_type_copyData(
            (char *)pDest + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
            (char *)pSource + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
            pSetType, acquire );
    }
    *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
    typelib_typedescriptionreference_release( pSetType );
}
//--------------------------------------------------------------------------------------------------
void copyConstructSequence(
    uno_Sequence ** ppDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () );
//--------------------------------------------------------------------------------------------------
inline void __copyConstructAnyFromData(
    uno_Any * pDestAny, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    TYPE_ACQUIRE( pType );
    pDestAny->pType = pType;

    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Unicode) );
        *(sal_Unicode *)pDestAny->pData = *(sal_Unicode *)pSource;
        break;
    case typelib_TypeClass_BOOLEAN:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Bool) );
        *(sal_Bool *)pDestAny->pData = *(sal_Bool *)pSource;
        break;
    case typelib_TypeClass_BYTE:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int8) );
        *(sal_Int8 *)pDestAny->pData = *(sal_Int8 *)pSource;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int16) );
        *(sal_Int16 *)pDestAny->pData = *(sal_Int16 *)pSource;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int32) );
        *(sal_Int32 *)pDestAny->pData = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int64) );
        *(sal_Int64 *)pDestAny->pData = *(sal_Int64 *)pSource;
        break;
    case typelib_TypeClass_FLOAT:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(float) );
        *(float *)pDestAny->pData = *(float *)pSource;
        break;
    case typelib_TypeClass_DOUBLE:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(double) );
        *(double *)pDestAny->pData = *(double *)pSource;
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *(rtl_uString **)pSource );
        pDestAny->pData = ::rtl_allocateMemory( sizeof(rtl_uString *) );
        *(rtl_uString **)pDestAny->pData = *(rtl_uString **)pSource;
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *(typelib_TypeDescriptionReference **)pSource );
        pDestAny->pData = ::rtl_allocateMemory( sizeof(typelib_TypeDescriptionReference *) );
        *(typelib_TypeDescriptionReference **)pDestAny->pData = *(typelib_TypeDescriptionReference **)pSource;
        break;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_ANY:
        OSL_ENSHURE( sal_False, "### unexpected nested any!" );
        break;
#endif
    case typelib_TypeClass_ENUM:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(int) );
        *(int *)pDestAny->pData = *(int *)pSource;
        break;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSHURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            __copyConstructStruct(
                pDestAny->pData, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            __copyConstructStruct(
                pDestAny->pData, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            __copyConstructUnion( pDestAny->pData, pSource, pTypeDescr, acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
            __copyConstructUnion( pDestAny->pData, pSource, pTypeDescr, acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(uno_Sequence *) );
        if (pTypeDescr)
        {
            copyConstructSequence(
                (uno_Sequence **)pDestAny->pData, *(uno_Sequence **)pSource,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            copyConstructSequence(
                (uno_Sequence **)pDestAny->pData, *(uno_Sequence **)pSource,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_INTERFACE:
        pDestAny->pData = ::rtl_allocateMemory( sizeof(void *) );
        if (mapping)
        {
            *(void **)pDestAny->pData = __map( *(void **)pSource, pType, pTypeDescr, mapping );
        }
        else
        {
            __acquire( *(void **)pDestAny->pData = *(void **)pSource, acquire );
        }
        break;
    }
}
//--------------------------------------------------------------------------------------------------
inline void __copyConstructAny(
    uno_Any * pDestAny, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    if (typelib_TypeClass_VOID == pType->eTypeClass)
    {
        __CONSTRUCT_EMPTY_ANY( pDestAny );
    }
    else
    {
        if (typelib_TypeClass_ANY == pType->eTypeClass)
        {
            if (pSource)
            {
                pType = ((uno_Any *)pSource)->pType;
                if (typelib_TypeClass_VOID == pType->eTypeClass)
                {
                    __CONSTRUCT_EMPTY_ANY( pDestAny );
                    return;
                }
                pTypeDescr = 0;
                pSource = ((uno_Any *)pSource)->pData;
            }
            else
            {
                __CONSTRUCT_EMPTY_ANY( pDestAny );
                return;
            }
        }
        if (pSource)
        {
            __copyConstructAnyFromData( pDestAny, pSource, pType, pTypeDescr, acquire, mapping );
        }
        else // default construct
        {
            TYPE_ACQUIRE( pType );
            pDestAny->pType = pType;
            switch (pType->eTypeClass)
            {
            case typelib_TypeClass_CHAR:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Unicode) );
                *(sal_Unicode *)pDestAny->pData = '\0';
                break;
            case typelib_TypeClass_BOOLEAN:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Bool) );
                *(sal_Bool *)pDestAny->pData = sal_False;
                break;
            case typelib_TypeClass_BYTE:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int8) );
                *(sal_Int8 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int16) );
                *(sal_Int16 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int32) );
                *(sal_Int32 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(sal_Int64) );
                *(sal_Int64 *)pDestAny->pData = 0;
                break;
            case typelib_TypeClass_FLOAT:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(float) );
                *(float *)pDestAny->pData = 0.0;
                break;
            case typelib_TypeClass_DOUBLE:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(double) );
                *(double *)pDestAny->pData = 0.0;
                break;
            case typelib_TypeClass_STRING:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(rtl_uString *) );
                *(rtl_uString **)pDestAny->pData = 0;
                RTL_USTRING_NEW( (rtl_uString **)pDestAny->pData );
                break;
            case typelib_TypeClass_TYPE:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(typelib_TypeDescriptionReference *) );
                *(typelib_TypeDescriptionReference **)pDestAny->pData = __getVoidType();
                break;
            case typelib_TypeClass_ENUM:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(int) );
                if (pTypeDescr)
                {
                    *(int *)pDestAny->pData = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    *(int *)pDestAny->pData = ((typelib_EnumTypeDescription *)pTypeDescr)->nDefaultEnumValue;
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
#ifdef CPPU_ASSERTIONS
            case typelib_TypeClass_TYPEDEF:
                OSL_ENSHURE( sal_False, "### unexpected typedef!" );
                break;
#endif
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
                if (pTypeDescr)
                {
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    __defaultConstructStruct(
                        pDestAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr );
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    __defaultConstructStruct(
                        pDestAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr );
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_UNION:
                if (pTypeDescr)
                {
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    __defaultConstructUnion( pDestAny->pData, pTypeDescr );
                }
                else
                {
                    TYPELIB_DANGER_GET( &pTypeDescr, pType );
                    pDestAny->pData = ::rtl_allocateMemory( pTypeDescr->nSize );
                    __defaultConstructUnion( pDestAny->pData, pTypeDescr );
                    TYPELIB_DANGER_RELEASE( pTypeDescr );
                }
                break;
            case typelib_TypeClass_SEQUENCE:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(uno_Sequence *) );
                *(uno_Sequence **)pDestAny->pData = __getEmptySequence();
                break;
            case typelib_TypeClass_INTERFACE:
                pDestAny->pData = ::rtl_allocateMemory( sizeof(void *) );
                *(void **)pDestAny->pData = 0; // either cpp or c-uno interface
                break;
            }
        }
    }
}
//--------------------------------------------------------------------------------------------------
inline void __copyConstructSequence(
    uno_Sequence ** ppDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    typelib_TypeClass eTypeClass = pElementType->eTypeClass;
    if (!mapping ||
        (eTypeClass <= typelib_TypeClass_ENUM && eTypeClass != typelib_TypeClass_ANY))
    {
        ::osl_incrementInterlockedCount( &pSource->nRefCount );
        *ppDest = pSource;
    }
    else // create new sequence
    {
        uno_Sequence * pDest;
        sal_Int32 nElements = pSource->nElements;
        if (nElements)
        {
            switch (eTypeClass)
            {
            case typelib_TypeClass_ANY:
            {
                pDest = (uno_Sequence *)::rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + (nElements * sizeof(uno_Any)) );
                uno_Any * pDestElements = (uno_Any *)pDest->elements;
                uno_Any * pSourceElements = (uno_Any *)pSource->elements;
                for ( sal_Int32 nPos = nElements; nPos--; )
                {
                    typelib_TypeDescriptionReference * pType = pSourceElements[nPos].pType;
                    if (typelib_TypeClass_VOID == pType->eTypeClass)
                    {
                        __CONSTRUCT_EMPTY_ANY( &pDestElements[nPos] );
                    }
                    else
                    {
                        __copyConstructAnyFromData(
                            &pDestElements[nPos], pSourceElements[nPos].pData,
                            pType, 0,
                            acquire, mapping );
                    }
                }
                break;
            }
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription * pElementTypeDescr = 0;
                TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                sal_Int32 nElementSize = pElementTypeDescr->nSize;
                char * pSourceElements = pSource->elements;
                pDest = (uno_Sequence *)::rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + (nElements * nElementSize) );
                char * pElements = pDest->elements;
                for ( sal_Int32 nPos = nElements; nPos--; )
                {
                    __copyConstructStruct(
                        pElements + (nPos * nElementSize),
                        pSourceElements + (nPos * nElementSize),
                        (typelib_CompoundTypeDescription *)pElementTypeDescr,
                        acquire, mapping );
                }
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                break;
            }
            case typelib_TypeClass_UNION:
            {
                typelib_TypeDescription * pElementTypeDescr = 0;
                TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                sal_Int32 nElementSize = pElementTypeDescr->nSize;
                sal_Int32 nValueOffset = ((typelib_UnionTypeDescription *)pElementTypeDescr)->nValueOffset;
                pDest = (uno_Sequence *)::rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + (nElements * nElementSize) );
                char * pElements = pDest->elements;
                char * pSourceElements = pSource->elements;
                for ( sal_Int32 nPos = nElements; nPos--; )
                {
                    char * pDest = pElements + (nPos * nElementSize);
                    char * pSource = pSourceElements + (nPos * nElementSize);

                    typelib_TypeDescriptionReference * pSetType = __unionGetSetType( pSource, pElementTypeDescr );
                    ::uno_type_copyAndConvertData( pDest + nValueOffset, pSource + nValueOffset,
                                                   pSetType, mapping );
                    *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
                    ::typelib_typedescriptionreference_release( pSetType );
                }
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                break;
            }
            case typelib_TypeClass_SEQUENCE: // sequence of sequence
            {
                typelib_TypeDescription * pElementTypeDescr = 0;
                TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                typelib_TypeDescriptionReference * pSeqElementType =
                    ((typelib_IndirectTypeDescription *)pElementTypeDescr)->pType;
                pDest = (uno_Sequence *)::rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + (nElements * sizeof(uno_Sequence *)) );
                char * pElements = pDest->elements;
                uno_Sequence ** pSourceElements = (uno_Sequence **)pSource->elements;
                for ( sal_Int32 nPos = nElements; nPos--; )
                {
                    copyConstructSequence(
                        (uno_Sequence **)pElements + nPos, pSourceElements[nPos],
                        pSeqElementType,
                        acquire, mapping );
                }
                TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                break;
            }
            case typelib_TypeClass_INTERFACE:
            {
                pDest = (uno_Sequence *)::rtl_allocateMemory(
                    SAL_SEQUENCE_HEADER_SIZE + (nElements * sizeof(void *)) );
                char * pElements = pDest->elements;
                void ** pSourceElements = (void **)pSource->elements;
                if (mapping)
                {
                    typelib_TypeDescription * pElementTypeDescr = 0;
                    TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        ((void **)pElements)[nPos] = 0;
                        if (((void **)pSourceElements)[nPos])
                        {
                            (*mapping->mapInterface)(
                                mapping, (void **)pElements + nPos,
                                pSourceElements[nPos],
                                (typelib_InterfaceTypeDescription *)pElementTypeDescr );
                        }
                    }
                    TYPELIB_DANGER_RELEASE( pElementTypeDescr );
                }
                else
                {
                    for ( sal_Int32 nPos = nElements; nPos--; )
                    {
                        __acquire( ((void **)pElements)[nPos] = pSourceElements[nPos], acquire );
                    }
                }
                break;
            }
            default:
                pDest = (uno_Sequence *)::rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
                break;
            }
        }
        else
        {
            pDest = (uno_Sequence *)::rtl_allocateMemory( SAL_SEQUENCE_HEADER_SIZE );
        }
        pDest->nElements = nElements;
        pDest->nRefCount = 1;
        *ppDest = pDest;
    }
}

//--------------------------------------------------------------------------------------------------
inline void __copyConstructData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType, typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW ( () )
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        *(sal_Unicode *)pDest = *(sal_Unicode *)pSource;
        break;
    case typelib_TypeClass_BOOLEAN:
        *(sal_Bool *)pDest = *(sal_Bool *)pSource;
        break;
    case typelib_TypeClass_BYTE:
        *(sal_Int8 *)pDest = *(sal_Int8 *)pSource;
        break;
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        *(sal_Int16 *)pDest = *(sal_Int16 *)pSource;
        break;
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
#if defined(GCC) && defined(SPARC)
        *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
        *(((sal_Int32 *)pDest) +1) = *(((sal_Int32 *)pSource) +1);
#else
        *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
#endif
        break;
    case typelib_TypeClass_FLOAT:
        *(float *)pDest = *(float *)pSource;
        break;
    case typelib_TypeClass_DOUBLE:
        *(double *)pDest = *(double *)pSource;
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_acquire( *(rtl_uString **)pSource );
        *(rtl_uString **)pDest = *(rtl_uString **)pSource;
        break;
    case typelib_TypeClass_TYPE:
        TYPE_ACQUIRE( *(typelib_TypeDescriptionReference **)pSource );
        *(typelib_TypeDescriptionReference **)pDest = *(typelib_TypeDescriptionReference **)pSource;
        break;
    case typelib_TypeClass_ANY:
        __copyConstructAny(
            (uno_Any *)pDest, ((uno_Any *)pSource)->pData,
            ((uno_Any *)pSource)->pType, 0,
            acquire, mapping );
        break;
    case typelib_TypeClass_ENUM:
        *(int *)pDest = *(int *)pSource;
        break;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSHURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            __copyConstructStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __copyConstructStruct(
                pDest, pSource,
                (typelib_CompoundTypeDescription *)pTypeDescr,
                acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            __copyConstructUnion( pDest, pSource, pTypeDescr, acquire, mapping );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __copyConstructUnion( pDest, pSource, pTypeDescr, acquire, mapping );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        if (mapping)
        {
            if (pTypeDescr)
            {
                __copyConstructSequence(
                    (uno_Sequence **)pDest, *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    acquire, mapping );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                __copyConstructSequence(
                    (uno_Sequence **)pDest, *(uno_Sequence **)pSource,
                    ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    acquire, mapping );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        else
        {
            ::osl_incrementInterlockedCount( &(*(uno_Sequence **)pSource)->nRefCount );
            *(uno_Sequence **)pDest = *(uno_Sequence **)pSource;
        }
        break;
    case typelib_TypeClass_INTERFACE:
        if (mapping)
            *(void **)pDest = __map( *(void **)pSource, pType, pTypeDescr, mapping );
        else
            __acquire( *(void **)pDest = *(void **)pSource, acquire );
        break;
    }
}

}

#endif
