/*************************************************************************
 *
 *  $RCSfile: data.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: dbo $ $Date: 2001-08-22 11:07:10 $
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

#include "constr.hxx"
#include "destr.hxx"
#include "copy.hxx"
#include "assign.hxx"
#include "eq.hxx"

using namespace cppu;
using namespace rtl;
using namespace com::sun::star::uno;


namespace cppu
{

uno_Sequence g_emptySeq = { 1, 0, { 0 } }; // static empty sequence
typelib_TypeDescriptionReference * g_pVoidType = 0;
typelib_TypeDescription * g_pQITD = 0;

//==================================================================================================
void defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pCompType )
    SAL_THROW( () )
{
    __defaultConstructStruct( pMem, pCompType );
}
//==================================================================================================
void copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW( () )
{
    __copyConstructStruct( pDest, pSource, pTypeDescr, acquire, mapping );
}
//==================================================================================================
void destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    __destructStruct( pValue, pTypeDescr, release );
}
//==================================================================================================
sal_Bool equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    return __equalStruct( pDest, pSource, pTypeDescr, queryInterface, release );
}
//==================================================================================================
sal_Bool assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    return __assignStruct( pDest, pSource, pTypeDescr, queryInterface, acquire, release );
}
//==================================================================================================
void copyConstructSequence(
    uno_Sequence ** ppDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
    SAL_THROW( () )
{
    __copyConstructSequence( ppDest, pSource, pElementType, acquire, mapping );
}
//==================================================================================================
void destructSequence(
    uno_Sequence ** ppSequence,
    typelib_TypeDescriptionReference * pElementType,
    uno_ReleaseFunc release )
    SAL_THROW( () )
{
    uno_Sequence * pSequence = *ppSequence;
    OSL_ASSERT( pSequence );
    if (! ::osl_decrementInterlockedCount( &pSequence->nRefCount ))
    {
        if (pSequence->nElements)
        {
            __destructElements(
                pSequence->elements, pElementType,
                0, pSequence->nElements, release );
        }
        ::rtl_freeMemory( pSequence );
    }
}
//==================================================================================================
sal_Bool equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW( () )
{
    return __equalSequence( pDest, pSource, pElementType, queryInterface, release );
}

extern "C"
{
//##################################################################################################
void SAL_CALL uno_type_constructData(
    void * pMem, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    __defaultConstructData( pMem, pType, 0 );
}
//##################################################################################################
void SAL_CALL uno_constructData(
    void * pMem, typelib_TypeDescription * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    __defaultConstructData( pMem, pTypeDescr->pWeakRef, pTypeDescr );
}
//##################################################################################################
void SAL_CALL uno_type_destructData(
    void * pValue, typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    __destructData( pValue, pType, 0, release );
}
//##################################################################################################
void SAL_CALL uno_destructData(
    void * pValue,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    __destructData( pValue, pTypeDescr->pWeakRef, pTypeDescr, release );
}
//##################################################################################################
void SAL_CALL uno_type_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    __copyConstructData( pDest, pSource, pType, 0, acquire, 0 );
}
//##################################################################################################
void SAL_CALL uno_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    __copyConstructData( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, 0 );
}
//##################################################################################################
void SAL_CALL uno_type_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    __copyConstructData( pDest, pSource, pType, 0, 0, mapping );
}
//##################################################################################################
void SAL_CALL uno_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    __copyConstructData( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, 0, mapping );
}
//##################################################################################################
sal_Bool SAL_CALL uno_type_equalData(
    void * pVal1, typelib_TypeDescriptionReference * pVal1Type,
    void * pVal2, typelib_TypeDescriptionReference * pVal2Type,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return __equalData(
        pVal1, pVal1Type, 0,
        pVal2, pVal2Type, 0,
        queryInterface, release );
}
//##################################################################################################
sal_Bool SAL_CALL uno_equalData(
    void * pVal1, typelib_TypeDescription * pVal1TD,
    void * pVal2, typelib_TypeDescription * pVal2TD,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return __equalData(
        pVal1, pVal1TD->pWeakRef, pVal1TD,
        pVal2, pVal2TD->pWeakRef, pVal2TD,
        queryInterface, release );
}
//##################################################################################################
sal_Bool SAL_CALL uno_type_assignData(
    void * pDest, typelib_TypeDescriptionReference * pDestType,
    void * pSource, typelib_TypeDescriptionReference * pSourceType,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return __assignData(
        pDest, pDestType, 0,
        pSource, pSourceType, 0,
        queryInterface, acquire, release );
}
//##################################################################################################
sal_Bool SAL_CALL uno_assignData(
    void * pDest, typelib_TypeDescription * pDestTD,
    void * pSource, typelib_TypeDescription * pSourceTD,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return __assignData(
        pDest, pDestTD->pWeakRef, pDestTD,
        pSource, pSourceTD->pWeakRef, pSourceTD,
        queryInterface, acquire, release );
}
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


#ifdef _DEBUG

#ifdef SAL_W32
#   pragma pack(push, 8)
#elif defined(SAL_OS2)
#   pragma pack(8)
#endif

struct M
{
    sal_Int32   n;
    sal_Int16   o;
};

struct N : public M
{
    sal_Int16   p;
};

struct O : public M
{
    double  p;
};

struct P : public N
{
    double  p;
};

struct empty
{
};
struct second : public empty
{
    int a;
};

class BinaryCompatible_Impl
{
public:
    BinaryCompatible_Impl();
};
BinaryCompatible_Impl::BinaryCompatible_Impl()
{
    // sequence
    if ((SAL_SEQUENCE_HEADER_SIZE % 8) != 0)
    {
        OSL_ENSURE( 0, "### sal sequence header size not aligned to 8?!" );
        abort();
    }
    // enum
    if( sizeof( TypeClass ) != sizeof( sal_Int32 ) )
    {
        OSL_ENSURE( 0, "### enum not 32bit?!" );
        abort();
    }
    // any
    if (sizeof(void *) < sizeof(sal_Int32))
    {
        OSL_ENSURE( 0, "### size of pointer less than 32bit?!" );
        abort();
    }
    if( sizeof( Any ) != sizeof( uno_Any ) )
    {
        OSL_ENSURE( 0, "### Any inheritance adds space to uno_Any?!" );
        abort();
    }
    if( sizeof( Any ) != sizeof( void * ) * 3 )
    {
        OSL_ENSURE( 0, "### sizeof( Any ) != sizeof( void * ) * 3 ?!" );
        abort();
    }
    if( (sal_Int32)&((Any *) 16)->pType != 16 )
    {
        OSL_ENSURE( 0, "### (sal_Int32)&((Any *) 16)->pType != 16 ?!" );
        abort();
    }
    if( (sal_Int32)&((Any *) 16)->pData != 16 + sizeof( void * ) )
    {
        OSL_ENSURE( 0, "### (sal_Int32)&((Any *) 16)->pData != 16 + sizeof( void * ) ?!" );
        abort();
    }
    if( (sal_Int32)&((Any *) 16)->pReserved != 16 + (2* sizeof( void * )) )
    {
        OSL_ENSURE( 0, "### (sal_Int32)&((Any *) 16)->pReserved != 16 + (2* sizeof( void * )) ?!" );
        abort();
    }
    // interface
    if( sizeof( Reference< XInterface > ) != sizeof( XInterface * ) )
    {
        OSL_ENSURE( 0, "### unexpected size of interface reference?!" );
        abort();
    }
    // string
    if( sizeof( OUString ) != sizeof( rtl_uString * ) )
    {
        OSL_ENSURE( 0, "### unexpected size of string?!" );
        abort();
    }
    // struct
    if( sizeof( M ) != 8 || sizeof( N ) != 12 || sizeof( O ) != 16 )
    {
        OSL_ENSURE( 0, "### unexpected struct alignment?!" );
        abort();
    }
#ifdef SAL_W32
    if( sizeof( P ) != 24 )
    {
        OSL_ENSURE( 0, "### [win] unexpected struct alignment?!" );
        abort();
    }
#endif
#ifndef __GNUC__
    if( sizeof( second ) != sizeof( int ) )
    {
        OSL_ENSURE( 0, "### unexpected struct size?!" );
        abort();
    }
#endif
}

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
#endif

static BinaryCompatible_Impl aTest;

#endif

}
