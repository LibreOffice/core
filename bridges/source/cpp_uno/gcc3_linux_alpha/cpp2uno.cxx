/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "share.hxx"
#include <stdio.h>






using namespace ::com::sun::star::uno;

namespace
{
static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, 
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "as far as cpp2uno_call\n");
#endif
    int nregs = 0; 

    
    
    

    
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; 

    if (pReturnTypeDescr)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pReturnTypeDescr ))
        {
            pUnoReturn = pRegisterReturn; 
        }
        else 
        {
            pCppReturn = *(void **)gpreg;
            gpreg++;
            fpreg++;
            nregs++;

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); 
        }
    }
    
    gpreg++;
    fpreg++;
    nregs++;

    
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int64), "### unexpected size!" );
    
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    
    sal_Int32 * pTempIndices = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndices   = 0;
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

#if OSL_DEBUG_LEVEL > 2
        fprintf(stderr, "arg %d of %d\n", nPos, nParams);
#endif

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) 
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "simple type is %d\n", pParamTypeDescr->eTypeClass);
#endif

            switch (pParamTypeDescr->eTypeClass)
            {
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    if (nregs < axp::MAX_WORDS_IN_REGS)
                    {
                        if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT)
                        {
                            float tmp = (float) (*((double *)fpreg));
                            (*((float *) fpreg)) = tmp;
                        }

                        pCppArgs[nPos] = pUnoArgs[nPos] = fpreg;
                        gpreg++;
                        fpreg++;
                        nregs++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_BYTE:
                case typelib_TypeClass_BOOLEAN:
                    if (nregs < axp::MAX_WORDS_IN_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                        gpreg++;
                        fpreg++;
                        nregs++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                    if (nregs < axp::MAX_WORDS_IN_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                        gpreg++;
                        fpreg++;
                        nregs++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
                    break;
                case typelib_TypeClass_ENUM:
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_UNSIGNED_LONG:
                    if (nregs < axp::MAX_WORDS_IN_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                        gpreg++;
                        fpreg++;
                        nregs++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
                    break;
                default:
                    if (nregs < axp::MAX_WORDS_IN_REGS)
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = gpreg;
                        gpreg++;
                        fpreg++;
                        nregs++;
                    }
                    else
                    {
                        pCppArgs[nPos] = pUnoArgs[nPos] = ovrflw;
                        ovrflw++;
                    }
                    break;
            }

            
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else 
        {
#if OSL_DEBUG_LEVEL > 2
            fprintf(stderr, "complex, nregs is %d\n", nregs);
#endif

            void *pCppStack; 

            if (nregs < axp::MAX_WORDS_IN_REGS)
            {
                pCppArgs[nPos] = pCppStack = *gpreg;
                gpreg++;
                fpreg++;
                nregs++;
            }
            else
            {
                pCppArgs[nPos] = pCppStack = *ovrflw;
                ovrflw++;
            }

            if (! rParam.bIn) 
            {
                
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndices[nTempIndices] = nPos;
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndices[nTempIndices] = nPos; 
                
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else 
            {
                pUnoArgs[nPos] = pCppStack;
                
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "end of params\n");
#endif

    
    uno_Any aUnoExc; 
    uno_Any * pUnoExc = &aUnoExc;

    
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    
    if (pUnoExc)
    {
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];

            if (pParams[nIndex].bIn) 
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndices], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() ); 
        
        return typelib_TypeClass_VOID;
    }
    else 
    {
        
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bOut) 
            {
                
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        
        if (pCppReturn) 
        {
            if (pUnoReturn != pCppReturn) 
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            
            *(void **)pRegisterReturn = pCppReturn;
        }
        if (pReturnTypeDescr)
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}



static typelib_TypeClass cpp_mediate(
    sal_uInt64 nOffsetAndIndex,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int64)==sizeof(void *), "### unexpected!" );

    sal_Int32 nVtableOffset = (nOffsetAndIndex >> 32);
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);

#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "nVTableOffset, nFunctionIndex are %x %x\n", nVtableOffset, nFunctionIndex);
#endif

#if OSL_DEBUG_LEVEL > 2
        
        {
            fprintf( stderr, "= cpp_mediate () =\nGPR's (%d): ", 6 );
            for ( unsigned int i = 0; i < 6; ++i )
                fprintf( stderr, "0x%lx, ", gpreg[i] );
            fprintf( stderr, "\n");
            fprintf( stderr, "\nFPR's (%d): ", 6 );
            for ( unsigned int i = 0; i < 6; ++i )
                fprintf( stderr, "0x%lx (%f), ", fpreg[i], fpreg[i] );
            fprintf( stderr, "\n");
        }
#endif


    
    
    

    
    void * pThis;
    if( nFunctionIndex & 0x80000000 )
    {
        nFunctionIndex &= 0x7fffffff;
        pThis = gpreg[1];
    }
    else
    {
        pThis = gpreg[0];
    }

    pThis = static_cast< char * >(pThis) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            pThis);

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();


    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString( "illegal vtable index!" ),
            (XInterface *)pCppI );
    }

    
    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
        {
            
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, 
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        else
        {
            
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                0, 
                1, &aParam,
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        
        switch (nFunctionIndex)
        {
        case 1: 
            pCppI->acquireProxy(); 
            eRet = typelib_TypeClass_VOID;
            break;
        case 2: 
            pCppI->releaseProxy(); 
            eRet = typelib_TypeClass_VOID;
            break;
        case 0: 
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( gpreg[2] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
                (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)(
                    pCppI->getBridge()->getCppEnv(),
                    (void **)&pInterface, pCppI->getOid().pData,
                    (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( gpreg[0] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = gpreg[0];
                    eRet = typelib_TypeClass_ANY;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } 
        default:
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    default:
    {
        throw RuntimeException(
            OUString( "no member description found!" ),
            (XInterface *)pCppI );
    }
    }

    return eRet;
}

long cpp_vtable_call(long r16, long r17, long r18, long r19, long r20, long r21, long firstonstack)
{
    register long r1 asm("$1");
    sal_uInt64 nOffsetAndIndex = r1;

    long sp = (long)&firstonstack;

    sal_uInt64 gpreg[axp::MAX_GPR_REGS];
    gpreg[0] = r16;
    gpreg[1] = r17;
    gpreg[2] = r18;
    gpreg[3] = r19;
    gpreg[4] = r20;
    gpreg[5] = r21;

    double fpreg[axp::MAX_SSE_REGS];
    register double f16  asm("$f16");  fpreg[0] = f16;
    register double f17  asm("$f17");  fpreg[1] = f17;
    register double f18  asm("$f18");  fpreg[2] = f18;
    register double f19  asm("$f19");  fpreg[3] = f19;
    register double f20  asm("$f20");  fpreg[4] = f20;
    register double f21  asm("$f21");  fpreg[5] = f21;

    volatile long nRegReturn[1];
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "before mediate with %lx\n",nOffsetAndIndex);
    fprintf(stderr, "non-doubles are %x %x %x %x %x %x\n", gpreg[0], gpreg[1], gpreg[2], gpreg[3], gpreg[4], gpreg[5]);
    fprintf(stderr, "doubles are %f %f %f %f %f %f\n", fpreg[0], fpreg[1], fpreg[2], fpreg[3], fpreg[4], fpreg[5]);
#endif
    typelib_TypeClass aType =
        cpp_mediate( nOffsetAndIndex, (void**)gpreg, (void**)fpreg, (void**)sp,
            (sal_Int64*)nRegReturn );
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "after mediate ret is %lx %ld\n", nRegReturn[0], nRegReturn[0]);
#endif

    switch( aType )
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            nRegReturn[0] = (unsigned long)(*(unsigned char *)nRegReturn);
            break;
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_SHORT:
            nRegReturn[0] = (unsigned long)(*(unsigned short *)nRegReturn);
            break;
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_LONG:
            nRegReturn[0] = (unsigned long)(*(unsigned int *)nRegReturn);
            break;
        case typelib_TypeClass_VOID:
        default:
            break;
        case typelib_TypeClass_FLOAT:
            {
                double tmp = (double) (*((float *)nRegReturn));
                (*((double *) nRegReturn)) = tmp;
            }
            
        case typelib_TypeClass_DOUBLE:
            __asm__ ( "ldt $f0,%0\n\t"
                : : "m" (*((double*)nRegReturn)) : "$f0");
            break;
    }
    return nRegReturn[0];
}

const int codeSnippetSize = 32;

unsigned char *codeSnippet( unsigned char * code, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset, bool simple_ret_type )
{
    if (! simple_ret_type)
        nFunctionIndex |= 0x80000000;

    unsigned char * p = code;
    *(unsigned int*)&p[0]  = 0x47fb0401;        /* mov $27,$1           */
    *(unsigned int*)&p[4]  = 0xa43b0010;        /* ldq $1,16($27)      */
    *(unsigned int*)&p[8]  = 0xa77b0018;        /* ldq $27,24($27)      */
    *(unsigned int*)&p[12] = 0x6bfb0000;        /* jmp $31,($27),0      */
    *(unsigned int*)&p[16] = nFunctionIndex;
    *(unsigned int*)&p[20] = nVtableOffset;
    *(unsigned long*)&p[24] = (unsigned long)cpp_vtable_call;
    return (code + codeSnippetSize);
}
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(unsigned char const *, unsigned char const *)
{
    
    __asm__ __volatile__("call_pal 0x86");
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 2;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = 0;
    slots[-1].fn = 0;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code, sal_PtrDiff writetoexecdiff,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
#if OSL_DEBUG_LEVEL > 2
    fprintf(stderr, "in addLocalFunctions functionOffset is %x\n",functionOffset);
    fprintf(stderr, "in addLocalFunctions vtableOffset is %x\n",vtableOffset);
#endif

    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));

            
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code + writetoexecdiff;
                code = codeSnippet(code, functionOffset++, vtableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = codeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::shared::isSimpleType(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(
                        member)->pReturnTypeRef));
            break;

        default:
            OSL_ASSERT(false);
            break;
        }
        TYPELIB_DANGER_RELEASE(member);
    }
    return code;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
