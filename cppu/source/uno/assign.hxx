/*************************************************************************
 *
 *  $RCSfile: assign.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 13:27:08 $
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
#ifndef __ASSIGN_HXX__
#define __ASSIGN_HXX__

#include "prim.hxx"
#include "destr.hxx"
#include "constr.hxx"
#include "copy.hxx"

namespace cppu
{


//##################################################################################################
//#### assignment ##################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void __assignInterface(
    void ** ppDest, void * pSource,
    uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    void * pDest = *ppDest;
    if (pSource)
    {
        if (acquire)
            (*acquire)( pSource );
        else
            (*((uno_Interface *)pSource)->acquire)( (uno_Interface *)pSource );
    }
    if (pDest)
    {
        if (release)
            (*release)( pDest );
        else
            (*((uno_Interface *)pDest)->release)( (uno_Interface *)pDest );
    }
    *ppDest = pSource;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool __queryAndAssignInterface(
    void ** ppDest, void * pSource,
    typelib_TypeDescriptionReference * pDestType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    void * pDest = *ppDest;
    if (pSource)
    {
        if (queryInterface)
        {
            pSource = (*queryInterface)( pSource, pDestType );
        }
        else
        {
            uno_Any aRet, aExc;
            uno_Any * pExc = &aExc;

            void * aArgs[1];
            aArgs[0] = &pDestType;

            typelib_TypeDescription * pMTqueryInterface = __getQueryInterfaceTypeDescr();
            (*((uno_Interface *)pSource)->pDispatcher)(
                (uno_Interface *)pSource, pMTqueryInterface, &aRet, aArgs, &pExc );

            OSL_ENSURE( !pExc, "### Exception occured during queryInterface()!" );
            if (pExc)
            {
                __destructAny( pExc, 0 );
                pSource = 0;
            }
            else
            {
                if (typelib_TypeClass_INTERFACE == aRet.pType->eTypeClass)
                {
                    // tweaky... avoiding acquire/ release pair
                    ::typelib_typedescriptionreference_release( aRet.pType );
                    pSource = *(void **)aRet.pData; // serving acquired interface
                    ::rtl_freeMemory( aRet.pData );
                }
                else
                {
                    pSource = 0;
                }
            }
            ::typelib_typedescription_release( pMTqueryInterface );
        }
    }
    if (pDest)
    {
        if (release)
            (*release)( pDest );
        else
            (*((uno_Interface *)pDest)->release)( (uno_Interface *)pDest );
    }
    *ppDest = pSource;
    return (pSource != 0);
}
//==================================================================================================
sal_Bool assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () );
//--------------------------------------------------------------------------------------------------
inline sal_Bool __assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    if (pTypeDescr->pBaseTypeDescription)
    {
        // copy base value
        if (! assignStruct( pDest, pSource, pTypeDescr->pBaseTypeDescription,
                            queryInterface, acquire, release ))
        {
            return sal_False;
        }
    }
    // then copy members
    typelib_TypeDescriptionReference ** ppTypeRefs = pTypeDescr->ppTypeRefs;
    sal_Int32 * pMemberOffsets = pTypeDescr->pMemberOffsets;
    sal_Int32 nDescr = pTypeDescr->nMembers;
    while (nDescr--)
    {
        if (! ::uno_type_assignData( (char *)pDest + pMemberOffsets[nDescr],
                                     ppTypeRefs[nDescr],
                                     (char *)pSource + pMemberOffsets[nDescr],
                                     ppTypeRefs[nDescr],
                                     queryInterface, acquire, release ))
        {
            return sal_False;
        }
    }
    return sal_True;
}
//--------------------------------------------------------------------------------------------------
inline sal_Bool __assignData(
    void * pDest,
    typelib_TypeDescriptionReference * pDestType, typelib_TypeDescription * pDestTypeDescr,
    void * pSource,
    typelib_TypeDescriptionReference * pSourceType, typelib_TypeDescription * pSourceTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    if (pDest == pSource)
        return sal_True;

    if (! pSource)
    {
        __destructData( pDest, pDestType, pDestTypeDescr, release );
        __defaultConstructData( pDest, pDestType, pDestTypeDescr );
        return sal_True;
    }
    while (typelib_TypeClass_ANY == pSourceType->eTypeClass)
    {
        pSourceTypeDescr = 0;
        pSourceType = ((uno_Any *)pSource)->pType;
        pSource = ((uno_Any *)pSource)->pData;
        if (pDest == pSource)
            return sal_True;
    }

    switch (pDestType->eTypeClass)
    {
    case typelib_TypeClass_VOID:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_VOID:
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_CHAR:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_CHAR:
            *(sal_Unicode *)pDest = *(sal_Unicode *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_BOOLEAN:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BOOLEAN:
            *(sal_Bool *)pDest = (*(sal_Bool *)pSource != sal_False);
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_BYTE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int8 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_SHORT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int16 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int16 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_UNSIGNED_SHORT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt16 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt16 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_LONG:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int32 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_Int32 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int32 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_Int32 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_UNSIGNED_LONG:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt32 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_uInt32 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt32 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_uInt32 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_HYPER:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_Int64 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_Int64 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_Int64 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(sal_Int64 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_Int64 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(sal_Int64 *)pDest = *(sal_Int64 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_UNSIGNED_HYPER:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(sal_uInt64 *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(sal_uInt64 *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(sal_uInt64 *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(sal_uInt64 *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(sal_uInt64 *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            *(sal_uInt64 *)pDest = *(sal_uInt64 *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_FLOAT:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(float *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(float *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(float *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_FLOAT:
            *(float *)pDest = *(float *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_DOUBLE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_BYTE:
            *(double *)pDest = *(sal_Int8 *)pSource;
            return sal_True;
        case typelib_TypeClass_SHORT:
            *(double *)pDest = *(sal_Int16 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(double *)pDest = *(sal_uInt16 *)pSource;
            return sal_True;
        case typelib_TypeClass_LONG:
            *(double *)pDest = *(sal_Int32 *)pSource;
            return sal_True;
        case typelib_TypeClass_UNSIGNED_LONG:
            *(double *)pDest = *(sal_uInt32 *)pSource;
            return sal_True;
        case typelib_TypeClass_FLOAT:
            *(double *)pDest = *(float *)pSource;
            return sal_True;
        case typelib_TypeClass_DOUBLE:
            *(double *)pDest = *(double *)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_STRING:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_STRING:
            ::rtl_uString_assign( (rtl_uString **)pDest, *(rtl_uString **)pSource );
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_TYPE:
        switch (pSourceType->eTypeClass)
        {
        case typelib_TypeClass_TYPE:
            ::typelib_typedescriptionreference_release( *(typelib_TypeDescriptionReference **)pDest );
            TYPE_ACQUIRE(
                *(typelib_TypeDescriptionReference **)pDest = *(typelib_TypeDescriptionReference **)pSource );
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_ANY:
        __destructAny( (uno_Any *)pDest, release );
        __copyConstructAny( (uno_Any *)pDest, pSource, pSourceType, pSourceTypeDescr, acquire, 0 );
        return sal_True;
    case typelib_TypeClass_ENUM:
        if (__type_equals( pDestType, pSourceType ))
        {
            *(int *)pDest = *(int *)pSource;
            return sal_True;
        }
        return sal_False;
#ifdef CPPU_ASSERTIONS
    case typelib_TypeClass_TYPEDEF:
        OSL_ENSURE( sal_False, "### unexpected typedef!" );
        return sal_False;
#endif
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        if (typelib_TypeClass_STRUCT == pSourceType->eTypeClass ||
            typelib_TypeClass_EXCEPTION == pSourceType->eTypeClass)
        {
            sal_Bool bRet = sal_False;
            if (pSourceTypeDescr)
            {
                typelib_CompoundTypeDescription * pTypeDescr =
                    (typelib_CompoundTypeDescription *)pSourceTypeDescr;
                while (pTypeDescr && !__type_equals( ((typelib_TypeDescription *)pTypeDescr)->pWeakRef, pDestType ))
                    pTypeDescr = pTypeDescr->pBaseTypeDescription;
                if (pTypeDescr)
                    bRet = __assignStruct( pDest, pSource, pTypeDescr, queryInterface, acquire, release );
            }
            else
            {
                TYPELIB_DANGER_GET( &pSourceTypeDescr, pSourceType );
                typelib_CompoundTypeDescription * pTypeDescr =
                    (typelib_CompoundTypeDescription *)pSourceTypeDescr;
                while (pTypeDescr && !__type_equals( ((typelib_TypeDescription *)pTypeDescr)->pWeakRef, pDestType ))
                    pTypeDescr = pTypeDescr->pBaseTypeDescription;
                if (pTypeDescr)
                    bRet = __assignStruct( pDest, pSource, pTypeDescr, queryInterface, acquire, release );
                TYPELIB_DANGER_RELEASE( pSourceTypeDescr );
            }
            return bRet;
        }
        return sal_False;
    case typelib_TypeClass_UNION:
        if (__type_equals( pDestType, pSourceType ))
        {
            if (pDestTypeDescr)
            {
                __destructUnion( pDest, pDestTypeDescr, release );
                __copyConstructUnion( pDest, pSource, pDestTypeDescr, acquire, 0 );
            }
            else
            {
                TYPELIB_DANGER_GET( &pDestTypeDescr, pDestType );
                __destructUnion( pDest, pDestTypeDescr, release );
                __copyConstructUnion( pDest, pSource, pDestTypeDescr, acquire, 0 );
                TYPELIB_DANGER_RELEASE( pDestTypeDescr );
            }
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_SEQUENCE:
        if (typelib_TypeClass_SEQUENCE != pSourceType->eTypeClass)
            return sal_False;
        if (*(uno_Sequence **)pSource == *(uno_Sequence **)pDest) // self assignment
            return sal_True;
        if (__type_equals( pDestType, pSourceType ))
        {
            __destructSequence( *(uno_Sequence **)pDest, pDestType, pDestTypeDescr, release );
            ::osl_incrementInterlockedCount( &(*(uno_Sequence **)pSource)->nRefCount );
            *(uno_Sequence **)pDest = *(uno_Sequence **)pSource;
            return sal_True;
        }
        return sal_False;
    case typelib_TypeClass_INTERFACE:
        if (typelib_TypeClass_INTERFACE != pSourceType->eTypeClass)
            return sal_False;
        if (__type_equals( pDestType, pSourceType ))
        {
            __assignInterface( (void **)pDest, *(void **)pSource, acquire, release );
            return sal_True;
        }
        else
        {
            sal_Bool bRet;
            if (pSourceTypeDescr)
            {
                typelib_TypeDescription * pTD = pSourceTypeDescr;
                while (pTD && !__type_equals( pTD->pWeakRef, pDestType ))
                    pTD = (typelib_TypeDescription *)((typelib_InterfaceTypeDescription *)pTD)->pBaseTypeDescription;
                if (pTD) // is base of dest
                {
                    __assignInterface( (void **)pDest, *(void **)pSource, acquire, release );
                    bRet = sal_True;
                }
                else
                {
                    // query for interface
                    bRet = __queryAndAssignInterface(
                        (void **)pDest, *(void **)pSource, pDestType, queryInterface, release );
                }
            }
            else
            {
                if (__type_equals( pSourceType, pDestType ))
                {
                    __assignInterface( (void **)pDest, *(void **)pSource, acquire, release );
                    bRet = sal_True;
                }
                else
                {
                    // query for interface
                    bRet = __queryAndAssignInterface(
                        (void **)pDest, *(void **)pSource, pDestType, queryInterface, release );
                }
            }
            return bRet;
        }
    }
    return sal_False;
}

}

#endif
