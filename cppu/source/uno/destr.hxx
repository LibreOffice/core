/*************************************************************************
 *
 *  $RCSfile: destr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:53 $
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
#ifndef __DESTR_HXX__
#define __DESTR_HXX__

#include "prim.hxx"

namespace cppu
{


//##################################################################################################
//#### destruction #################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void __destructUnion(
    void * pValue,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    typelib_TypeDescriptionReference * pType = __unionGetSetType( pValue, pTypeDescr );
    ::uno_type_destructData(
        (char *)pValue + ((typelib_UnionTypeDescription *)pTypeDescr)->nValueOffset,
        pType, release );
    ::typelib_typedescriptionreference_release( pType );
}
//==================================================================================================
void destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release );
//--------------------------------------------------------------------------------------------------
inline void __destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
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
            (char *)pValue + pMemberOffsets[nDescr], ppTypeRefs[nDescr], release );
    }
}
//==================================================================================================
void destructSequence(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pElementType,
    uno_ReleaseFunc release );
//--------------------------------------------------------------------------------------------------
inline void __destructAny(
    uno_Any * pAny,
    uno_ReleaseFunc release )
{
    typelib_TypeDescriptionReference * pType = pAny->pType;

    if (typelib_TypeClass_VOID != pType->eTypeClass)
    {
        switch (pType->eTypeClass)
        {
        case typelib_TypeClass_STRING:
            ::rtl_uString_release( *(rtl_uString **)pAny->pData );
            break;
        case typelib_TypeClass_TYPE:
            ::typelib_typedescriptionreference_release( *(typelib_TypeDescriptionReference **)pAny->pData );
            break;
        case typelib_TypeClass_ANY:
            OSL_ENSHURE( sal_False, "### unexpected nested any!" );
            ::uno_any_destruct( (uno_Any *)pAny->pData, release );
            break;
#ifdef CPPU_ASSERTIONS
        case typelib_TypeClass_TYPEDEF:
            OSL_ENSHURE( sal_False, "### unexpected typedef!" );
            break;
#endif
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __destructStruct( pAny->pData, (typelib_CompoundTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
            break;
        }
        case typelib_TypeClass_UNION:
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __destructUnion( pAny->pData, pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
            break;
        }
        case typelib_TypeClass_SEQUENCE:
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            destructSequence(
                (uno_Sequence **)pAny->pData,
                ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
            break;
        }
        case typelib_TypeClass_INTERFACE:
            __releaseRef( (void **)pAny->pData, release );
            break;
        }
        ::rtl_freeMemory( pAny->pData );
#ifdef DEBUG
        pAny->pData = (void *)0xdeadbeef;
#endif
    }
    ::typelib_typedescriptionreference_release( pType );
}
//--------------------------------------------------------------------------------------------------
inline sal_Int32 __destructElements(
    void * pElements, typelib_TypeDescriptionReference * pElementType,
    sal_Int32 nStartIndex, sal_Int32 nStopIndex,
    uno_ReleaseFunc release )
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
            __destructAny( &pDest[nPos], release );
        }
        return sizeof(uno_Any);
    }
    case typelib_TypeClass_ENUM:
        return sizeof(int);
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSHURE( sal_False, "### unexpected typedef!" );
        break;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_TypeDescription * pElementTypeDescr = 0;
        TYPELIB_DANGER_GET( &pElementTypeDescr, pElementType );
        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
        {
            __destructStruct(
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
            __destructUnion(
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
                &pDest[nPos],
                ((typelib_IndirectTypeDescription *)pElementTypeDescr)->pType,
                release );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        return sizeof(uno_Sequence *);
    }
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_ARRAY:
        OSL_ENSHURE( sal_False, "### unexpected array!" );
        break;
#endif
    case typelib_TypeClass_INTERFACE:
    {
        if (release)
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                void * p = ((void **)pElements)[nPos];
                if (p)
                    (*release)( p );
            }
        }
        else
        {
            for ( sal_Int32 nPos = nStartIndex; nPos < nStopIndex; ++nPos )
            {
                uno_Interface * p = ((uno_Interface **)pElements)[nPos];
                if (p)
                    (*p->release)( p );
            }
        }
        return sizeof(void *);
    }
    }
    return 0;
}
//--------------------------------------------------------------------------------------------------
inline void __destructSequence(
    uno_Sequence * pSequence,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    if (! ::osl_decrementInterlockedCount( &pSequence->nRefCount ))
    {
        if (pSequence->nElements)
        {
            if (pTypeDescr)
            {
                __destructElements(
                    pSequence->elements, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    0, pSequence->nElements, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pTypeDescr, pType );
                __destructElements(
                    pSequence->elements, ((typelib_IndirectTypeDescription *)pTypeDescr)->pType,
                    0, pSequence->nElements, release );
                TYPELIB_DANGER_RELEASE( pTypeDescr );
            }
        }
        ::rtl_freeMemory( pSequence );
    }
}
//--------------------------------------------------------------------------------------------------
inline void __destructData(
    void * pValue,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
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
        __destructAny( (uno_Any *)pValue, release );
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
            __destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
        }
        else
        {
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __destructStruct( pValue, (typelib_CompoundTypeDescription *)pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_UNION:
        if (pTypeDescr)
        {
            __destructUnion( pValue, pTypeDescr, release );
        }
        else
        {
            typelib_TypeDescription * pTypeDescr = 0;
            TYPELIB_DANGER_GET( &pTypeDescr, pType );
            __destructUnion( pValue, pTypeDescr, release );
            TYPELIB_DANGER_RELEASE( pTypeDescr );
        }
        break;
    case typelib_TypeClass_SEQUENCE:
        __destructSequence( *(uno_Sequence **)pValue, pType, pTypeDescr, release );
        break;
    case typelib_TypeClass_INTERFACE:
        __releaseRef( (void **)pValue, release );
        break;
    }
}

}

#endif
