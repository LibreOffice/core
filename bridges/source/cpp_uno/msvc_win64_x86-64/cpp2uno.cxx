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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "abi.hxx"
#include "mscx.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace
{

//==================================================================================================

// Perform the UNO call
//
// We must convert the paramaters stored in pCallStack to UNO
// arguments and call pThis->getUnoI()->pDispatcher.
//
// pCallStack:  ret addr, this, [ret *], [params]
//
// [ret *] is present when we are returning a structure bigger than 8 bytes
// Simple types are returned in rax, or xmm0 (fp).
// Similarly structures <= 8 bytes are in rax, xmm0 as necessary.
static inline typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
        {
            // complex return via ptr (pCppReturn)
            pCppReturn = pCallStack[2];
            ++pCallStack;

            pUnoReturn = ( bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )
                           ? alloca( pReturnTypeDescr->nSize )
                           : pCppReturn ); // direct way
        }
        else
            pUnoReturn = pRegisterReturn; // direct way for simple types
    }

    // skip to first argument
    pCallStack += 2;

    // stack space
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if ( !rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr )) // value
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = pCallStack++;
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value || ref
        {
            void *pCppStack;
            pCppArgs[nPos] = pCppStack = *pCallStack++;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndizes[nTempIndizes] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else if ( bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ) ) // is in/inout
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        pCppStack, pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else // direct way
            {
                pUnoArgs[nPos] = pCppStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->getUnoI()->pDispatcher)( pThis->getUnoI(), pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occurred...
    if ( pUnoExc )
    {
        // destruct temporary in/inout params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        CPPU_CURRENT_NAMESPACE::mscx_raiseException( &aUnoExc, pThis->getBridge()->getUno2Cpp() ); // has to destruct the any
        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // else no exception occurred...
    {
        // temporary params
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

            if ( pParams[nIndex].bOut ) // inout/out
            {
                // convert and assign
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
            }
            // destroy temp uno param
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return
        if ( pCppReturn ) // has complex return
        {
            if ( pUnoReturn != pCppReturn ) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        pThis->getBridge()->getUno2Cpp() );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            // complex return ptr is set to rax
            *(void **)pRegisterReturn = pCppReturn;
        }
        if ( pReturnTypeDescr )
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}


//==================================================================================================
extern "C" typelib_TypeClass cpp_vtable_call(
    void ** pCallStack, sal_Int32 nFunctionIndex, sal_Int32 nVtableOffset,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    // pCallStack: ret adr, this, [ret *], params
    void * pThis = static_cast< char * >(pCallStack[1]) - nVtableOffset;

    bridges::cpp_uno::shared::CppInterfaceProxy * pCppI =
        bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy( pThis );

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->getTypeDescr();

    OSL_ENSURE( nFunctionIndex < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if ( nFunctionIndex >= pTypeDescr->nMapFunctionIndexToMemberIndex )
    {
        throw RuntimeException( OUString::createFromAscii("illegal vtable index!"),
                                reinterpret_cast<XInterface *>( pCppI ) );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch ( aMemberDescr.get()->eTypeClass )
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( aMemberDescr.get() )->pAttributeTypeRef;

            if ( pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex )
            {
                // is GET method
                eRet = cpp2uno_call( pCppI, aMemberDescr.get(), pAttrTypeRef,
                        0, 0, // no params
                        pCallStack, pRegisterReturn );
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;

                eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                        0, // indicates void return
                        1, &aParam,
                        pCallStack, pRegisterReturn );
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            // is METHOD
            switch ( nFunctionIndex )
            {
                // standard XInterface vtable calls
                case 1: // acquire()
                    pCppI->acquireProxy(); // non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 2: // release()
                    pCppI->releaseProxy(); // non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 0: // queryInterface() opt
                {
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, reinterpret_cast<Type *>( pCallStack[3] )->getTypeLibType() );
                    if ( pTD )
                    {
                        XInterface * pInterface = 0;
                        (*pCppI->getBridge()->getCppEnv()->getRegisteredInterface)
                            ( pCppI->getBridge()->getCppEnv(),
                              (void **)&pInterface,
                              pCppI->getOid().pData,
                              reinterpret_cast<typelib_InterfaceTypeDescription *>( pTD ) );

                        if ( pInterface )
                        {
                            ::uno_any_construct( reinterpret_cast<uno_Any *>( pCallStack[1] ),
                                                 &pInterface, pTD, cpp_acquire );

                            pInterface->release();
                            TYPELIB_DANGER_RELEASE( pTD );

                            reinterpret_cast<void **>( pRegisterReturn )[0] = pCallStack[1];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                } // else perform queryInterface()
                default:
                {
                    typelib_InterfaceMethodTypeDescription *pMethodTD =
                        reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( aMemberDescr.get() );

                    eRet = cpp2uno_call( pCppI, aMemberDescr.get(),
                                         pMethodTD->pReturnTypeRef,
                                         pMethodTD->nParams,
                                         pMethodTD->pParams,
                                         pCallStack, pRegisterReturn );
                }
            }
            break;
        }
        default:
        {
            throw RuntimeException( OUString::createFromAscii("no member description found!"),
                                    reinterpret_cast<XInterface *>( pCppI ) );
            // is here for dummy
            eRet = typelib_TypeClass_VOID;
        }
    }

    return eRet;
}

//==================================================================================================
extern "C" void privateSnippetExecutor( ... );

int const codeSnippetSize = 44;

unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset, bool isArgFloat[4])
{
    unsigned char * p = code;


    //
    //         | ...                        |
    //         +----------------------------+
    //         | argument 4                 |
    //  rsp+40 +----------------------------+ -------
    //         | argument 3, r9/xmm3 home   | ^ shadow
    //  rsp+32 +----------------------------+ | space,
    //         | argument 2, r8/xmm2 home   | | guaranteed to be present but uninitialized,
    //  rsp+24 +----------------------------+ | we have to copy
    //         | argument 1, rdx/xmm1 home  | | the first 4 parameters there from the registers,
    //  rsp+16 +----------------------------+ | to form the continuous array of arguments.
    //         | argument 0, rcx/xmm0 home  | v
    //  rsp+08 +----------------------------+ -------
    //         | return address             |
    //  rsp--> +----------------------------+
    //
    //

    if (isArgFloat[0])
    {
        // movsd QWORD[rsp+8], XMM0
        *p++ = 0xf2; *p++ = 0x0f; *p++ = 0x11; *p++ = 0x44; *p++ = 0x24; *p++ = 0x08;
    }
    else
    {
        // mov QWORD[rsp+8], rcx
        *p++ = 0x48; *p++ = 0x49; *p++ = 0x4c; *p++ = 0x24; *p++ = 0x08;
    }

    if (isArgFloat[1])
    {
        // movsd QWORD[rsp+16], XMM1
        *p++ = 0xf2; *p++ = 0x0f; *p++ = 0x11; *p++ = 0x4c; *p++ = 0x24; *p++ = 0x10;
    }
    else
    {
        // mov QWORD[rsp+16], rdx
        *p++ = 0x48; *p++ = 0x49; *p++ = 0x54; *p++ = 0x24; *p++ = 0x10;
    }

    if (isArgFloat[2])
    {
        // movsd QWORD[rsp+24], XMM2
        *p++ = 0xf2; *p++ = 0x0f; *p++ = 0x11; *p++ = 0x54; *p++ = 0x24; *p++ = 0x18;
    }
    else
    {
        // mov QWORD[rsp+24], r8
        *p++ = 0x4c; *p++ = 0x89; *p++ = 0x44; *p++ = 0x24; *p++ = 0x18;
    }

    if (isArgFloat[3])
    {
        // movsd QWORD[rsp+32], XMM3
        *p++ = 0xf2; *p++ = 0x0f; *p++ = 0x11; *p++ = 0x5c; *p++ = 0x24; *p++ = 0x20;
    }
    else
    {
        // mov QWORD[rsp+32], r9
        *p++ = 0x4c; *p++ = 0x89; *p++ = 0x4c; *p++ = 0x24; *p++ = 0x20;
    }

    // mov rax, functionIndex
    *p++ = 0xb8;
    *p++ = functionIndex & 0xff;
    *p++ = (functionIndex >> 8) & 0xff;
    *p++ = (functionIndex >> 16) & 0xff;
    *p++ = (functionIndex >> 24) & 0xff;

    // mov r10, vtableOffset
    *p++ = 0x41;
    *p++ = 0xba;
    *p++ = vtableOffset & 0xff;
    *p++ = (vtableOffset >> 8) & 0xff;
    *p++ = (vtableOffset >> 16) & 0xff;
    *p++ = (vtableOffset >> 24) & 0xff;

    // mov r11, privateSnippetExecutor
    *p++ = 0x49;
    *p++ = 0xbb;
    *p++ = ((sal_uIntPtr)(&privateSnippetExecutor)) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 8) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 16) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 24) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 32) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 40) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 48) & 0xff;
    *p++ = (((sal_uIntPtr)(&privateSnippetExecutor)) >> 56) & 0xff;

    // jmp r11
    *p++ = 0x49;
    *p++ = 0xff;
    *p++ = 0xe3;

    OSL_ASSERT(p - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 1;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    struct RttiCompleteObjectLocator {
        sal_uInt32 signature;
        sal_uInt32 offset;    // offset of vtable in objects of the class
        sal_uInt32 cdOffset;  // constructor displacement offset
        // On AMD64, these 2 are offsets from the module's base address (when signature == 0):
        sal_uInt32 typeDescriptorAddress;
        sal_uInt32 classDescriptorAddress;
        sal_uInt32 objectBase; // ignored when signature == 0

        RttiCompleteObjectLocator(): signature(0), offset(0), cdOffset(0)
        {
            type_info *typeInfo = CPPU_CURRENT_NAMESPACE::mscx_getRTTI(
                     rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                       "com.sun.star.uno.XInterface")));
            HMODULE thisDLLBase;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)&bridges::cpp_uno::shared::VtableFactory::initializeBlock, &thisDLLBase);
            typeDescriptorAddress = (sal_uInt32) (((char*)typeInfo) - (char*)thisDLLBase);
            classDescriptorAddress = 0;
            objectBase = 0;
        }
    };
    static RttiCompleteObjectLocator rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

static void findFirst4Win64ArgumentTypes(
    typelib_MethodParameter *pParams, sal_Int32 nParams,
    typelib_TypeDescriptionReference *pReturnType, bool isArgFloat[4])
{
    sal_uInt32 index = 0;

    isArgFloat[0] = isArgFloat[1] = isArgFloat[2] = isArgFloat[3] = false;

    // C++ "this" pointer:
    ++index;

    // https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=vs-2017
    // "Otherwise the caller assumes the responsibility of allocating memory
    // and passing a pointer for the return value as the first argument.
    // Subsequent arguments are then shifted one argument to the right.
    // The same pointer must be returned by the callee in RAX."
    if ( pReturnType && x86_64::return_in_hidden_param( pReturnType ) )
        ++index;

    for (int param = 0; param < nParams; param++)
    {
        if (index >= 4)
            break;

        typelib_TypeDescription *pParamTypeDescr = 0;

        TYPELIB_DANGER_GET( &pParamTypeDescr, pParams[param].pTypeRef );
        OSL_ASSERT( pParamTypeDescr );

        isArgFloat[index++] = !pParams[param].bOut &&
            (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT ||
            pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE);

        TYPELIB_DANGER_RELEASE( pParamTypeDescr );
    }
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const *type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 nPos = 0; nPos < type->nMembers; ++nPos) {
        typelib_TypeDescription * pTD = 0;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ nPos ] );
        OSL_ASSERT( pTD );

        bool isArgFloat[4];
        if ( typelib_TypeClass_INTERFACE_ATTRIBUTE == pTD->eTypeClass )
        {
            typelib_InterfaceAttributeTypeDescription *pAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            // get method
            findFirst4Win64ArgumentTypes(0, 0, pAttrTD->pAttributeTypeRef, isArgFloat);
            (s++)->fn = code;
            code = codeSnippet(code, functionOffset++, vtableOffset, isArgFloat);

            if ( ! pAttrTD->bReadOnly )
            {
                // set method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTD->pAttributeTypeRef;
                aParam.bIn      = sal_True;
                aParam.bOut     = sal_False;
                findFirst4Win64ArgumentTypes(&aParam, 1, 0, isArgFloat);
                (s++)->fn = code;
                code = codeSnippet(code, functionOffset++, vtableOffset, isArgFloat);
            }
        }
        else if ( typelib_TypeClass_INTERFACE_METHOD == pTD->eTypeClass )
        {
            typelib_InterfaceMethodTypeDescription *pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );
            findFirst4Win64ArgumentTypes(pMethodTD->pParams, pMethodTD->nParams,
                pMethodTD->pReturnTypeRef, isArgFloat);
            (s++)->fn = code;
            code = codeSnippet(code, functionOffset++, vtableOffset, isArgFloat);
        }
        else
            OSL_ASSERT( false );

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *, unsigned char const *)
{}
