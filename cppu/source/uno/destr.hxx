/*************************************************************************
 *
 *  $RCSfile: destr.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 10:53:23 $
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
#ifndef DESTR_HXX
#define DESTR_HXX

#include "prim.hxx"


namespace cppu
{

//##################################################################################################
//#### destruction #################################################################################
//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline void _destructUnion(
    void * pValue,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    typelib_TypeDescriptionReference * pType = _unionGetSetType( pValue, pTypeDescr );
    ::uno_type_destructData(
        (char *)pValue + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
        pType, release );
    ::typelib_typedescriptionreference_release( pType );
}
//==================================================================================================
void destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
inline void _destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        destructStruct( pValue, pTypeDescr->pBaseTypeDescription, release );
    }

    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;
    while (nDescr--)
    {
        ::uno_type_destructData(
            (char *)pValue + pMemberOffsets[nDescr],
            ppTypeRefs[nDescr], release );
    }
}

//--------------------------------------------------------------------------------------------------
inline void _destructArray(
    void * pValue,
    typelib_ArrayTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    throw ()
{
    typelib_TypeDescription * pElementType = NULL;
    TYPELIB_DANGER_GET( &pElementType, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType );
    sal_Int32 nElementSize = pElementType->nSize;
    TYPELIB_DANGER_RELEASE( pElementType );

    sal_Int32 nTotalElements = pTypeDescr->nTotalElements;
    for(sal_Int32 i=0; i < nTotalElements; i++)
    {
        ::uno_type_destructData(
            (sal_Char *)pValue + i * nElementSize,
            ((typelib_IndirectTypeDescription *)pTypeDescr)->pType, release );
    }

    typelib_typedescriptionreference_release(((typelib_IndirectTypeDescription *)pTypeDescr)->pType);
}

//==============================================================================
void destructSequence(
    uno_Sequence * pSequence,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release );

//--------------------------------------------------------------------------------------------------
inline void _destructAny(
    uno_Any * pAny,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    typelib_TypeDescriptionReference * pType = pAny->pType;

    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        if (sizeof(void *) < sizeof(sal_Int64))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_FLOAT:
        if (sizeof(void *) < sizeof(float))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_DOUBLE:
        if (sizeof(void *) < sizeof(double))
        {
            ::rtl_freeMemory( pAny->pData );
        }
        break;
    case typelib_TypeClass_STRING:
        ::rtl_uString_release( (rtl_uString *)pAny->pReserved );
        break;
    case typelib_TypeClass_TYPE:
        ::typelib_typedescriptionreference_release(
            (typelib_TypeDescriptionReference *)pAny->pReserved );
        break;
    case typelib_TypeClass_ANY:
        OSL_ENSURE( sal_False, "### unexpected nested any!" );
        ::uno_any_destruct( (uno_Any *)pAny->pData, release );
        ::rtl_freeMemory( pAny->pData );
        break;
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( 0, "### unexpected typedef!" );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        _destructStruct( pAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        ::rtl_freeMemory( pAny->pData );
        break;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pType );
        _destructUnion( pAny->pData, pTypeDescr, release );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        ::rtl_freeMemory( pAny->pData );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        destructSequence(
            *(uno_Sequence **) &pAny->pReserved, pType, 0, release );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        _release( pAny->pReserved, release );
        break;
    }
#if OSL_DEBUG_LEVEL > 0
    pAny->pData = (void *)0xdeadbeef;
#endif

    ::typelib_typedescriptionreference_release( pType );
}
//--------------------------------------------------------------------------------------------------
inline sal_Int32 idestructElements(
    void * pElements, typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    switch (pElementType->eTypeClass)
    {
    case typelib_TypeClass_CHAR:
        return sizeof(sal_Unicode);
    case typelib_TypeClass_BOOLEAN:
        return sizeof(sal_Bool);
    case typelib_TypeClass_BYTE:
        return sizeof(sal_Int8);
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
        return sizeof(sal_Int16);
    case typelib_TypeClass_LONG:
    case typelib_TypeClass_UNSIGNED_LONG:
        return sizeof(sal_Int32);
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        return sizeof(sal_Int64);
    case typelib_TypeClass_FLOAT:
        return sizeof(float);
    case typelib_TypeClass_DOUBLE:
        return sizeof(double);

    case typelib_TypeClass_STRING:
    {
        rtl_uString ** pDest = (rtl_uString **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            ::rtl_uString_release( pDest[nPos] );
        }
        return sizeof(rtl_uString *);
    }
    case typelib_TypeClass_TYPE:
    {
        typelib_TypeDescriptionReference ** pDest = (typelib_TypeDescriptionReference **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            ::typelib_typedescriptionreference_release( pDest[nPos] );
        }
        return sizeof(typelib_TypeDescriptionReference *);
    }
    case typelib_TypeClass_ANY:
    {
        uno_Any * pDest = (uno_Any *)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructAny( &pDest[nPos], release );
        }
        return sizeof(uno_Any);
    }
    case typelib_TypeClass_ENUM:
        return sizeof(sal_Int32);
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( 0, "### unexpected typedef!" );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructStruct(
                (char *)pElements + (nElementSize * nPos),
                (typelib_CompoundTypeDescription *)pElementTypeDescr,
                release );
        }
        sal_Int32 nSize = pElementTypeDescr->nSize;
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return nSize;
    }
    case typelib_TypeClass_UNION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            _destructUnion(
                (char *)pElements + (nElementSize * nPos),
                pElementTypeDescr,
                release );
        }
        sal_Int32 nSize = pElementTypeDescr->nSize;
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return nSize;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        uno_Sequence ** pDest = (uno_Sequence **)pElements;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            destructSequence(
                pDest[nPos],
                pElementTypeDescr->pWeakRef, pElementTypeDescr,
                release );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return sizeof(uno_Sequence *);
    }
    case typelib_TypeClass_ARRAY:
        OSL_ENSURE( 0, "### unexpected array!" );
        break;
    case typelib_TypeClass_INTERFACE:
    {
        if (release)
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                void * p = ((void **)pElements)[nPos];
                if (p)
                {
                    (*release)( p );
                }
            }
        }
        else
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                uno_Interface * p = ((uno_Interface **)pElements)[nPos];
                if (p)
                {
                    (*p->release)( p );
                }
            }
        }
        return sizeof(void *);
    }
    }
    return 0;
}

//------------------------------------------------------------------------------
inline void idestructSequence(
    uno_Sequence * pSeq,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    if (::osl_decrementInterlockedCount( &pSeq->nRefCount ) == 0)
    {
        if (pSeq->nElements > 0)
        {
            if (pTypeDescr)
            {
                idestructElements(
                    pSeq->elements,
                    ((typelib_IndirectTypeDescription *) pTypeDescr)->pType, 0,
                    pSeq->nElements, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                idestructElements(
                    pSeq->elements,
                    ((typelib_IndirectTypeDescription *) pTypeDescr)->pType, 0,
                    pSeq->nElements, release );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        ::rtl_freeMemory( pSeq );
    }
}

//--------------------------------------------------------------------------------------------------
inline void _destructData(
    void * pValue,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    switch (pType->eTypeClass)
    {
    case typelib_TypeClass_STRING:
        ::rtl_uString_release( *(rtl_uString **)pValue );
        break;
    case typelib_TypeClass_TYPE:
        ::typelib_typedescriptionreference_release( *(typelib_TypeDescriptionReference **)pValue );
        break;
    case typelib_TypeClass_ANY:
        _destructAny( (uno_Any *)pValue, release );
        break;
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( 0, "### unexpected typedef!" );
        break;
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (pTypeDescr)
        {
            _destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_ARRAY:
        if (pTypeDescr)
        {
            _destructArray( pValue, (typelib_ArrayTypeDescription *)pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructArray( pValue, (typelib_ArrayTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            _destructUnion( pValue, pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            _destructUnion( pValue, pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
    {
        idestructSequence(
            *(uno_Sequence **)pValue, pType, pTypeDescr, release );
        break;
    }
    case typelib_TypeClass_INTERFACE:
        _release( *(void **)pValue, release );
        break;
    }
}

}

#endif
