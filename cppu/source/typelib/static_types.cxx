/*************************************************************************
 *
 *  $RCSfile: static_types.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:52 $
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

#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/memory.h>

#include <typelib/typedescription.h>


using namespace osl;
using namespace rtl;

//------------------------------------------------------------------------
sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset,
    sal_Int32 & rMaxIntegralTypeSize );
//------------------------------------------------------------------------
void SAL_CALL typelib_typedescription_newEmpty(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName );
//-----------------------------------------------------------------------------
void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet,
    rtl_uString * pName );

extern "C"
{

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

/**
 * The double member determin the alignment.
 * Under Os2 and MS-Windows the Alignment is min( 8, sizeof( type ) ).
 * The aligment of a strukture is min( 8, sizeof( max basic type ) ), the greatest basic type
 * determine the aligment.
 */
struct AlignSize_Impl
{
    sal_Int16   nInt16;
    double      dDouble;
};

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

// the value of the maximal alignment
static sal_Int32 nMaxAlignment = (sal_Int32)&((AlignSize_Impl *) 16)->dDouble - 16;

static inline sal_Int32 adjustAlignment( sal_Int32 nRequestedAlignment )
{
    if( nRequestedAlignment > nMaxAlignment )
        nRequestedAlignment = nMaxAlignment;
    return nRequestedAlignment;
}

/**
 * Calculate the new size of the struktur.
 */
static inline sal_Int32 newAlignedSize( sal_Int32 OldSize, sal_Int32 ElementSize, sal_Int32 NeededAlignment )
{
    NeededAlignment = adjustAlignment( NeededAlignment );
    return (OldSize + NeededAlignment -1) / NeededAlignment * NeededAlignment + ElementSize;
}

//--------------------------------------------------------------------------------------------------
static Mutex & typelib_getStaticInitMutex()
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMutex)
        {
            static Mutex s_aMutex;
            s_pMutex = &s_aMutex;
        }
    }
    return *s_pMutex;
}

//##################################################################################################
SAL_DLLEXPORT typelib_TypeDescriptionReference ** SAL_CALL typelib_static_type_getByTypeClass(
    typelib_TypeClass eTypeClass )
{
    static typelib_TypeDescriptionReference * s_aTypes[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 };

    if (! s_aTypes[eTypeClass])
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! s_aTypes[eTypeClass])
        {
            static const char * s_aTypeNames[] = {
                "void", "char", "boolean", "byte",
                "short", "unsigned short", "long", "unsigned long",
                "hyper", "unsigned hyper", "float", "double",
                "string", "type", "any" };

            switch (eTypeClass)
            {
            case typelib_TypeClass_EXCEPTION:
            case typelib_TypeClass_INTERFACE:
            {
                // type
                if (! s_aTypes[typelib_TypeClass_TYPE])
                {
                    OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("type") );
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_TYPE], typelib_TypeClass_TYPE, sTypeName.pData );
#ifndef CPPU_LEAK_STATIC_DATA
                    // another static ref
                    ++(*(sal_Int32 *)&s_aTypes[typelib_TypeClass_TYPE]->pReserved);
#endif
                }
                // any
                if (! s_aTypes[typelib_TypeClass_ANY])
                {
                    OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("any") );
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_ANY], typelib_TypeClass_ANY, sTypeName.pData );
#ifndef CPPU_LEAK_STATIC_DATA
                    // another static ref
                    ++(*(sal_Int32 *)&s_aTypes[typelib_TypeClass_ANY]->pReserved);
#endif
                }
                // string
                if (! s_aTypes[typelib_TypeClass_STRING])
                {
                    OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("string") );
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_STRING], typelib_TypeClass_STRING, sTypeName.pData );
#ifndef CPPU_LEAK_STATIC_DATA
                    // another static ref
                    ++(*(sal_Int32 *)&s_aTypes[typelib_TypeClass_STRING]->pReserved);
#endif
                }
                // XInterface
                if (! s_aTypes[typelib_TypeClass_INTERFACE])
                {
                    OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface") );

                    typelib_InterfaceTypeDescription * pTD = 0;

                    typelib_TypeDescriptionReference * pMembers[3] = { 0,0,0 };
                    OUString sMethodName0( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::queryInterface") );
                    ::typelib_typedescriptionreference_new(
                        &pMembers[0], typelib_TypeClass_INTERFACE_METHOD, sMethodName0.pData );
                    OUString sMethodName1( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::acquire") );
                    ::typelib_typedescriptionreference_new(
                        &pMembers[1], typelib_TypeClass_INTERFACE_METHOD, sMethodName1.pData );
                    OUString sMethodName2( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::release") );
                    ::typelib_typedescriptionreference_new(
                        &pMembers[2], typelib_TypeClass_INTERFACE_METHOD, sMethodName2.pData );

                    ::typelib_typedescription_newInterface(
                        &pTD, sTypeName.pData, 0xe227a391, 0x33d6, 0x11d1, 0xaabe00a0, 0x249d5590,
                        0, 3, pMembers );

                    ::typelib_typedescription_register( (typelib_TypeDescription **)&pTD );
                    ::typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_INTERFACE] = ((typelib_TypeDescription *)pTD)->pWeakRef );
#ifndef CPPU_LEAK_STATIC_DATA
                    // another static ref
                    ++(*(sal_Int32 *)&s_aTypes[typelib_TypeClass_INTERFACE]->pReserved);
#endif
                    ::typelib_typedescription_release( (typelib_TypeDescription*)pTD );

                    ::typelib_typedescriptionreference_release( pMembers[0] );
                    ::typelib_typedescriptionreference_release( pMembers[1] );
                    ::typelib_typedescriptionreference_release( pMembers[2] );
                    // Exception
                    OSL_ASSERT( ! s_aTypes[typelib_TypeClass_EXCEPTION] );
                    {
                    typelib_TypeDescription * pTD = 0;
                    OUString sTypeName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception") );

                    typelib_CompoundMember_Init aMembers[2];
                    OUString sMemberType0( RTL_CONSTASCII_USTRINGPARAM("string") );
                    OUString sMemberName0( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception::Message") );
                    aMembers[0].eTypeClass = typelib_TypeClass_STRING;
                    aMembers[0].pTypeName = sMemberType0.pData;
                    aMembers[0].pMemberName = sMemberName0.pData;
                    OUString sMemberType1( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface") );
                    OUString sMemberName1( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.Exception::Context") );
                    aMembers[1].eTypeClass = typelib_TypeClass_INTERFACE;
                    aMembers[1].pTypeName = sMemberType1.pData;
                    aMembers[1].pMemberName = sMemberName1.pData;

                    ::typelib_typedescription_new(
                        &pTD, typelib_TypeClass_EXCEPTION, sTypeName.pData, 0, 2, aMembers );
                    typelib_typedescription_register( &pTD );
                    typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_EXCEPTION] = pTD->pWeakRef );
#ifndef CPPU_LEAK_STATIC_DATA
                    // another static ref
                    ++(*(sal_Int32 *)&s_aTypes[typelib_TypeClass_EXCEPTION]->pReserved);
#endif
                    // RuntimeException
                    OUString sTypeName2( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.RuntimeException") );
                    ::typelib_typedescription_new(
                        &pTD, typelib_TypeClass_EXCEPTION, sTypeName2.pData, s_aTypes[typelib_TypeClass_EXCEPTION], 0, 0 );
                    ::typelib_typedescription_register( &pTD );
                    ::typelib_typedescription_release( pTD );
                    }
                    // XInterface members
                    typelib_InterfaceMethodTypeDescription * pMethod = 0;
                    typelib_Parameter_Init aParameters[1];
                    OUString sParamName0( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.XInterface::aType") );
                    OUString sParamType0( RTL_CONSTASCII_USTRINGPARAM("type") );
                    aParameters[0].pParamName = sParamName0.pData;
                    aParameters[0].eTypeClass = typelib_TypeClass_TYPE;
                    aParameters[0].pTypeName = sParamType0.pData;
                    aParameters[0].bIn = sal_True;
                    aParameters[0].bOut = sal_False;
                    rtl_uString * pExceptions[1];
                    OUString sExceptionName0( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.uno.RuntimeException") );
                    pExceptions[0] = sExceptionName0.pData;
                    OUString sReturnType0( RTL_CONSTASCII_USTRINGPARAM("any") );
                    typelib_typedescription_newInterfaceMethod(
                        &pMethod, 0, sal_False, sMethodName0.pData,
                        typelib_TypeClass_ANY, sReturnType0.pData,
                        1, aParameters, 1, pExceptions );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );

                    OUString sReturnType1( RTL_CONSTASCII_USTRINGPARAM("void") );
                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 1, sal_True, sMethodName1.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData, 0, 0, 0, 0 );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );

                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 2, sal_True, sMethodName2.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData,
                        0, 0, 0, 0 );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );
                    ::typelib_typedescription_release( (typelib_TypeDescription*)pMethod );
                }
                break;
            }
            default:
            {
                OUString aTypeName( OUString::createFromAscii( s_aTypeNames[eTypeClass] ) );
                ::typelib_typedescriptionreference_new( &s_aTypes[eTypeClass], eTypeClass, aTypeName.pData );
#ifndef CPPU_LEAK_STATIC_DATA
                // another static ref
                ++(*(sal_Int32 *)&s_aTypes[eTypeClass]->pReserved);
#endif
            }
            }
        }
    }
    return &s_aTypes[eTypeClass];
}

//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName )
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            ::typelib_typedescriptionreference_new( ppRef, eTypeClass, aTypeName.pData );

#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}

// !for NOT REALLY WEAK TYPES only!
static inline typelib_TypeDescriptionReference * __getTypeByName( rtl_uString * pTypeName )
{
    typelib_TypeDescriptionReference * pRef = 0;
    ::typelib_typedescriptionreference_getByName( &pRef, pTypeName );
    if (pRef && pRef->pType && pRef->pType->pWeakRef) // found initialized td
        return pRef;
    else
        return 0;
}

//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_sequence_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType )
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OUStringBuffer aBuf( 32 );
            aBuf.appendAscii( "[]" );
            aBuf.append( pElementType->pTypeName );
            OUString aTypeName( aBuf.makeStringAndClear() );

            OSL_ASSERT( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_SEQUENCE) );
            if (! (*ppRef = __getTypeByName( aTypeName.pData )))
            {
                typelib_TypeDescription * pReg = 0;
                ::typelib_typedescription_new(
                    &pReg, typelib_TypeClass_SEQUENCE,
                    aTypeName.pData, pElementType, 0, 0 );

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                OSL_ASSERT( *ppRef == pReg->pWeakRef );
            }
#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}

//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_compound_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers )
{
    OSL_ENSHURE( typelib_TypeClass_STRUCT == eTypeClass ||
                 typelib_TypeClass_EXCEPTION == eTypeClass, "### unexpected type class!" );

    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OSL_ASSERT( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(eTypeClass) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            if (! (*ppRef = __getTypeByName( aTypeName.pData )))
            {
                typelib_CompoundTypeDescription * pComp = 0;
                ::typelib_typedescription_newEmpty(
                    (typelib_TypeDescription **)&pComp, eTypeClass, aTypeName.pData );

                sal_Int32 nOffset = 0;
                if (pBaseType)
                {
                    ::typelib_typedescriptionreference_getDescription(
                        (typelib_TypeDescription **)&pComp->pBaseTypeDescription, pBaseType );
                    OSL_ASSERT( pComp->pBaseTypeDescription );
                    nOffset = ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize;
                    OSL_ENSHURE( newAlignedSize( 0, ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize, ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nAlignment ) == ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize, "### unexpected offset!" );
                }

                if (nMembers)
                {
                    pComp->nMembers = nMembers;
                    pComp->pMemberOffsets = new sal_Int32[ nMembers ];
                    pComp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
                    for ( sal_Int32 i = 0 ; i < nMembers; ++i )
                    {
                        ::typelib_typedescriptionreference_acquire(
                            pComp->ppTypeRefs[i] = ppMembers[i] );
                        // write offset
                        typelib_TypeDescription * pTD = 0;
                        TYPELIB_DANGER_GET( &pTD, pComp->ppTypeRefs[i] );
                        OSL_ENSHURE( pTD->nSize, "### void member?" );
                        nOffset = newAlignedSize( nOffset, pTD->nSize, pTD->nAlignment );
                        pComp->pMemberOffsets[i] = nOffset - pTD->nSize;
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                }

                typelib_TypeDescription * pReg = (typelib_TypeDescription *)pComp;
                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                OSL_ASSERT( *ppRef == pReg->pWeakRef );
            }
#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType )
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OSL_ASSERT( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_INTERFACE) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            if (! (*ppRef = __getTypeByName( aTypeName.pData )))
            {
                typelib_InterfaceTypeDescription * pIface = 0;
                ::typelib_typedescription_newEmpty(
                    (typelib_TypeDescription **)&pIface, typelib_TypeClass_INTERFACE, aTypeName.pData );

                if (pBaseType)
                {
                    ::typelib_typedescriptionreference_getDescription(
                        (typelib_TypeDescription **)&pIface->pBaseTypeDescription, pBaseType );
                }
                else
                {
                    ::typelib_typedescriptionreference_getDescription(
                        (typelib_TypeDescription **)&pIface->pBaseTypeDescription,
                        * ::typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ) );
                }
                OSL_ASSERT( pIface->pBaseTypeDescription );

                typelib_TypeDescription * pReg = (typelib_TypeDescription *)pIface;
                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );

                pReg->nAlignment = adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                OSL_ASSERT( *ppRef == pReg->pWeakRef );
            }
#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}
//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_enum_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nDefaultValue )
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OSL_ASSERT( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_ENUM) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            if (! (*ppRef = __getTypeByName( aTypeName.pData )))
            {
                typelib_TypeDescription * pReg = 0;
                ::typelib_typedescription_newEmpty(
                    &pReg, typelib_TypeClass_ENUM, aTypeName.pData );
                typelib_EnumTypeDescription * pEnum = (typelib_EnumTypeDescription *)pReg;

                pEnum->nDefaultEnumValue = nDefaultValue;

                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = ::adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                OSL_ASSERT( *ppRef == pReg->pWeakRef );
            }
#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}

//##################################################################################################
SAL_DLLEXPORT void SAL_CALL typelib_static_union_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantTypeRef,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultTypeRef,
    sal_Int32 nMembers,
    sal_Int64 * pDiscriminants,
    typelib_TypeDescriptionReference ** pMemberTypes )
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_getStaticInitMutex() );
        if (! *ppRef)
        {
            OSL_ASSERT( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_UNION) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            if (! (*ppRef = __getTypeByName( aTypeName.pData )))
            {
                typelib_UnionTypeDescription * pUnion = 0;
                ::typelib_typedescription_newEmpty(
                    (typelib_TypeDescription **)&pUnion, typelib_TypeClass_UNION, aTypeName.pData );
                // discriminant type
                ::typelib_typedescriptionreference_acquire( pUnion->pDiscriminantTypeRef = pDiscriminantTypeRef );

                sal_Int32 nPos;

                pUnion->nMembers = nMembers;
                // default discriminant
                if (nMembers)
                {
                    pUnion->pDiscriminants = new sal_Int64[ nMembers ];
                    for ( nPos = nMembers; nPos--; )
                    {
                        pUnion->pDiscriminants[nPos] = pDiscriminants[nPos];
                    }
                }
                // default default discriminant
                pUnion->nDefaultDiscriminant = nDefaultDiscriminant;

                // union member types
                pUnion->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
                for ( nPos = nMembers; nPos--; )
                {
                    ::typelib_typedescriptionreference_acquire(
                        pUnion->ppTypeRefs[nPos] = pMemberTypes[nPos] );
                }

                // default union type
                ::typelib_typedescriptionreference_acquire( pUnion->pDefaultTypeRef = pDefaultTypeRef );

                typelib_TypeDescription * pReg = (typelib_TypeDescription *)pUnion;

                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                pReg->nSize = typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                OSL_ASSERT( *ppRef == pReg->pWeakRef );
            }
#ifndef CPPU_LEAK_STATIC_DATA
            // another static ref
            ++(*(sal_Int32 *)&(*ppRef)->pReserved);
#endif
        }
    }
}

}
