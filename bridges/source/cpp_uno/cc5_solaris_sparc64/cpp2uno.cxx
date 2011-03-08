/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_bridges.hxx"
#include "sal/config.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"
#include "com/sun/star/uno/genfunc.hxx"
#include "osl/diagnose.h"
#include "sal/alloca.h"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"
#include "uno/data.h"

#include "exceptions.hxx"
#include "flushcode.hxx"
#include "fp.hxx"
#include "isdirectreturntype.hxx"
#include "vtableslotcall.hxx"

namespace {

namespace css = com::sun::star;

void loadFpRegsFromStruct(typelib_TypeDescription * type, void * data) {
    for (typelib_CompoundTypeDescription * t =
             reinterpret_cast< typelib_CompoundTypeDescription * >(type);
         t != NULL; t = t->pBaseTypeDescription)
    {
        for (sal_Int32 i = 0; i < t->nMembers; ++i) {
            switch (t->ppTypeRefs[i]->eTypeClass) {
            case typelib_TypeClass_FLOAT:
                switch (t->pMemberOffsets[i]) {
                case 0:
                    fp_loadf0(reinterpret_cast< float * >(data));
                    break;
                case 4:
                    fp_loadf1(reinterpret_cast< float * >(data) + 1);
                    break;
                case 8:
                    fp_loadf2(reinterpret_cast< float * >(data) + 2);
                    break;
                case 12:
                    fp_loadf3(reinterpret_cast< float * >(data) + 3);
                    break;
                case 16:
                    fp_loadf4(reinterpret_cast< float * >(data) + 4);
                    break;
                case 20:
                    fp_loadf5(reinterpret_cast< float * >(data) + 5);
                    break;
                case 24:
                    fp_loadf6(reinterpret_cast< float * >(data) + 6);
                    break;
                case 28:
                    fp_loadf7(reinterpret_cast< float * >(data) + 7);
                    break;
                default:
                    OSL_ASSERT(false);
                    break;
                }
                break;
            case typelib_TypeClass_DOUBLE:
                switch (t->pMemberOffsets[i]) {
                case 0:
                    fp_loadd0(reinterpret_cast< double * >(data));
                    break;
                case 8:
                    fp_loadd2(reinterpret_cast< double * >(data) + 1);
                    break;
                case 16:
                    fp_loadd4(reinterpret_cast< double * >(data) + 2);
                    break;
                case 24:
                    fp_loadd6(reinterpret_cast< double * >(data) + 3);
                    break;
                default:
                    OSL_ASSERT(false);
                    break;
                }
                break;
            case typelib_TypeClass_STRUCT:
                {
                    typelib_TypeDescription * td = NULL;
                    TYPELIB_DANGER_GET(&td, t->ppTypeRefs[i]);
                    loadFpRegsFromStruct(td, data);
                    TYPELIB_DANGER_RELEASE(td);
                    break;
                }
            }
        }
    }
}

void call(
    bridges::cpp_uno::shared::CppInterfaceProxy * proxy,
    css::uno::TypeDescription const & description,
    bool directReturn, typelib_TypeDescriptionReference * returnType,
    sal_Int32 count, typelib_MethodParameter * parameters,
    unsigned long * callStack)
{
    typelib_TypeDescription * rtd = NULL;
    if (returnType != NULL) {
        TYPELIB_DANGER_GET(&rtd, returnType);
    }
    bool retconv =
        rtd != NULL && bridges::cpp_uno::shared::relatesToInterfaceType(rtd);
    OSL_ASSERT(!(directReturn && retconv));
    void * retin;
    void * retout;
    char retbuf[32];
    if (directReturn) {
        retin = returnType == NULL ? NULL : retbuf;
    } else {
        retout = reinterpret_cast< void * >(callStack[0]);
        retin = retconv ? alloca(rtd->nSize) : retout;
    }
    void ** args = static_cast< void ** >(alloca(count * sizeof (void *)));
    void ** cppArgs = static_cast< void ** >(alloca(count * sizeof (void *)));
    typelib_TypeDescription ** argtds =
        static_cast< typelib_TypeDescription ** >(
            alloca(count * sizeof (typelib_TypeDescription *)));
    union fp { float f; double d; };
    fp copies[15];
    sal_Int32 stackPos = directReturn ? 1 : 2; // skip return ptr and this ptr
    for (sal_Int32 i = 0; i < count; ++i) {
        typelib_TypeDescription * ptd = NULL;
        TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
        if (!parameters[i].bOut && bridges::cpp_uno::shared::isSimpleType(ptd))
        {
            switch (ptd->eTypeClass) {
            case typelib_TypeClass_FLOAT:
                if (stackPos <= 15) {
                    switch (stackPos) {
                    case 1:
                        fp_storef3(&copies[0].f);
                        break;
                    case 2:
                        fp_storef5(&copies[1].f);
                        break;
                    case 3:
                        fp_storef7(&copies[2].f);
                        break;
                    case 4:
                        fp_storef9(&copies[3].f);
                        break;
                    case 5:
                        fp_storef11(&copies[4].f);
                        break;
                    case 6:
                        fp_storef13(&copies[5].f);
                        break;
                    case 7:
                        fp_storef15(&copies[6].f);
                        break;
                    case 8:
                        fp_storef17(&copies[7].f);
                        break;
                    case 9:
                        fp_storef19(&copies[8].f);
                        break;
                    case 10:
                        fp_storef21(&copies[9].f);
                        break;
                    case 11:
                        fp_storef23(&copies[10].f);
                        break;
                    case 12:
                        fp_storef25(&copies[11].f);
                        break;
                    case 13:
                        fp_storef27(&copies[12].f);
                        break;
                    case 14:
                        fp_storef29(&copies[13].f);
                        break;
                    case 15:
                        fp_storef31(&copies[14].f);
                        break;
                    default:
                        OSL_ASSERT(false);
                        break;
                    }
                    args[i] = &copies[stackPos - 1].f;
                } else {
                    args[i] = reinterpret_cast< char * >(callStack + stackPos) +
                        (sizeof (unsigned long) - sizeof (float));
                }
                break;
            case typelib_TypeClass_DOUBLE:
                if (stackPos <= 15) {
                    switch (stackPos) {
                    case 1:
                        fp_stored2(&copies[0].d);
                        break;
                    case 2:
                        fp_stored4(&copies[1].d);
                        break;
                    case 3:
                        fp_stored6(&copies[2].d);
                        break;
                    case 4:
                        fp_stored8(&copies[3].d);
                        break;
                    case 5:
                        fp_stored10(&copies[4].d);
                        break;
                    case 6:
                        fp_stored12(&copies[5].d);
                        break;
                    case 7:
                        fp_stored14(&copies[6].d);
                        break;
                    case 8:
                        fp_stored16(&copies[7].d);
                        break;
                    case 9:
                        fp_stored18(&copies[8].d);
                        break;
                    case 10:
                        fp_stored20(&copies[9].d);
                        break;
                    case 11:
                        fp_stored22(&copies[10].d);
                        break;
                    case 12:
                        fp_stored24(&copies[11].d);
                        break;
                    case 13:
                        fp_stored26(&copies[12].d);
                        break;
                    case 14:
                        fp_stored28(&copies[13].d);
                        break;
                    case 15:
                        fp_stored30(&copies[14].d);
                        break;
                    default:
                        OSL_ASSERT(false);
                        break;
                    }
                    args[i] = &copies[stackPos - 1].d;
                } else {
                    args[i] = reinterpret_cast< char * >(callStack + stackPos) +
                        (sizeof (unsigned long) - sizeof (double));
                }
                break;
            default:
                OSL_ASSERT(ptd->nSize <= 8);
                args[i] = reinterpret_cast< char * >(callStack + stackPos) +
                    (sizeof (unsigned long) - ptd->nSize);
                break;
            }
            argtds[i] = NULL;
            TYPELIB_DANGER_RELEASE(ptd);
        } else {
            cppArgs[i] = reinterpret_cast< void * >(callStack[stackPos]);
            if (!parameters[i].bIn) {
                args[i] = alloca(ptd->nSize);
                argtds[i] = ptd;
            } else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd)) {
                args[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(
                    args[i], reinterpret_cast< void * >(callStack[stackPos]),
                    ptd, proxy->getBridge()->getCpp2Uno());
                argtds[i] = ptd;
            } else {
                args[i] = reinterpret_cast< void * >(callStack[stackPos]);
                argtds[i] = NULL;
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
        ++stackPos;
    }
    uno_Any exc;
    uno_Any * pexc = &exc;
    proxy->getUnoI()->pDispatcher(
        proxy->getUnoI(), description.get(), retin, args, &pexc);
    if (pexc != NULL) {
        for (sal_Int32 i = 0; i < count; ++i) {
            if (argtds[i] != NULL) {
                if (parameters[i].bIn) {
                    uno_destructData(args[i], argtds[i], NULL);
                }
                TYPELIB_DANGER_RELEASE(argtds[i]);
            }
        }
        if (rtd != NULL) {
            TYPELIB_DANGER_RELEASE(rtd);
        }
        bridges::cpp_uno::cc5_solaris_sparc64::raiseException(
            &exc, proxy->getBridge()->getUno2Cpp());
        std::abort(); // just in case
    }
    for (sal_Int32 i = 0; i < count; ++i) {
        if (argtds[i] != NULL) {
            if (parameters[i].bOut) {
                uno_destructData(
                    cppArgs[i], argtds[i],
                    reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release));
                uno_copyAndConvertData(
                    cppArgs[i], args[i], argtds[i],
                    proxy->getBridge()->getUno2Cpp());
            }
            uno_destructData(args[i], argtds[i], NULL);
            TYPELIB_DANGER_RELEASE(argtds[i]);
        }
    }
    if (directReturn) {
        if (rtd != NULL) {
            switch (rtd->eTypeClass) {
            case typelib_TypeClass_VOID:
                break;
            case typelib_TypeClass_BOOLEAN:
                callStack[0] = *reinterpret_cast< sal_Bool * >(retbuf);
                break;
            case typelib_TypeClass_BYTE:
                callStack[0] = *reinterpret_cast< sal_Int8 * >(retbuf);
                break;
            case typelib_TypeClass_SHORT:
                callStack[0] = *reinterpret_cast< sal_Int16 * >(retbuf);
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
                callStack[0] = *reinterpret_cast< sal_uInt16 * >(retbuf);
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_ENUM:
                callStack[0] = *reinterpret_cast< sal_Int32 * >(retbuf);
                break;
            case typelib_TypeClass_UNSIGNED_LONG:
                callStack[0] = *reinterpret_cast< sal_uInt32 * >(retbuf);
                break;
            case typelib_TypeClass_HYPER:
                callStack[0] = *reinterpret_cast< sal_Int64 * >(retbuf);
                break;
            case typelib_TypeClass_UNSIGNED_HYPER:
                callStack[0] = *reinterpret_cast< sal_uInt64 * >(retbuf);
                break;
            case typelib_TypeClass_FLOAT:
                fp_loadf0(reinterpret_cast< float * >(retbuf));
                break;
            case typelib_TypeClass_DOUBLE:
                fp_loadd0(reinterpret_cast< double * >(retbuf));
                break;
            case typelib_TypeClass_CHAR:
                callStack[0] = *reinterpret_cast< sal_Unicode * >(retbuf);
                break;
            case typelib_TypeClass_STRING:
            case typelib_TypeClass_TYPE:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_INTERFACE:
                callStack[0] = reinterpret_cast< unsigned long >(
                    *reinterpret_cast< void ** >(retbuf));
                break;
            case typelib_TypeClass_STRUCT:
                loadFpRegsFromStruct(rtd, retbuf);
                // fall through
            case typelib_TypeClass_ANY:
                std::memcpy(callStack, retbuf, rtd->nSize);
                break;
            default:
                OSL_ASSERT(false);
                break;
            }
        }
    } else if (retconv) {
        uno_copyAndConvertData(
            retout, retin, rtd, proxy->getBridge()->getUno2Cpp());
        uno_destructData(retin, rtd, NULL);
    }
    if (rtd != NULL) {
        TYPELIB_DANGER_RELEASE(rtd);
    }
}

extern "C" void vtableCall(
    sal_Int32 functionIndex, sal_Int32 vtableOffset, unsigned long * callStack)
{
    bool direct = static_cast< sal_uInt32 >((functionIndex) & 0x80000000) == 0;
    functionIndex = static_cast< sal_uInt32 >(functionIndex) & 0x7FFFFFFF;
    bridges::cpp_uno::shared::CppInterfaceProxy * proxy
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            reinterpret_cast< char * >(callStack[direct ? 0 : 1]) -
            vtableOffset);
    typelib_InterfaceTypeDescription * type = proxy->getTypeDescr();
    OSL_ASSERT(functionIndex < type->nMapFunctionIndexToMemberIndex);
    sal_Int32 pos = type->pMapFunctionIndexToMemberIndex[functionIndex];
    css::uno::TypeDescription desc(type->ppAllMembers[pos]);
    switch (desc.get()->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        if (type->pMapMemberIndexToFunctionIndex[pos] == functionIndex) {
            // Getter:
            call(
                proxy, desc, direct,
                reinterpret_cast< typelib_InterfaceAttributeTypeDescription * >(
                    desc.get())->pAttributeTypeRef,
                0, NULL, callStack);
        } else {
            // Setter:
            typelib_MethodParameter param = {
                NULL,
                reinterpret_cast< typelib_InterfaceAttributeTypeDescription * >(
                    desc.get())->pAttributeTypeRef,
                true, false };
            call(proxy, desc, true, NULL, 1, &param, callStack);
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
        switch (functionIndex) {
        case 1:
            proxy->acquireProxy();
            break;
        case 2:
            proxy->releaseProxy();
            break;
        case 0:
            {
                typelib_TypeDescription * td = NULL;
                TYPELIB_DANGER_GET(
                    &td,
                    reinterpret_cast< css::uno::Type * >(
                        callStack[2])->getTypeLibType());
                if (td != NULL) {
                    css::uno::XInterface * ifc = NULL;
                    proxy->getBridge()->getCppEnv()->getRegisteredInterface(
                        proxy->getBridge()->getCppEnv(),
                        reinterpret_cast< void ** >(&ifc),
                        proxy->getOid().pData,
                        reinterpret_cast< typelib_InterfaceTypeDescription * >(
                            td));
                    if (ifc != NULL) {
                        uno_any_construct(
                            reinterpret_cast< uno_Any * >(callStack[0]), &ifc,
                            td,
                            reinterpret_cast< uno_AcquireFunc >(
                                css::uno::cpp_acquire));
                        ifc->release();
                        TYPELIB_DANGER_RELEASE(td);
                        break;
                    }
                    TYPELIB_DANGER_RELEASE(td);
                }
            } // fall through
        default:
            call(
                proxy, desc, direct,
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    desc.get())->pReturnTypeRef,
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    desc.get())->nParams,
                reinterpret_cast< typelib_InterfaceMethodTypeDescription * >(
                    desc.get())->pParams,
                callStack);
        }
        break;
    default:
        OSL_ASSERT(false);
        break;
    }
}

int const codeSnippetSize = 10 * 4;

unsigned char * generateCodeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset,
    bool directReturn)
{
    sal_uInt32 index = functionIndex;
    if (!directReturn) {
        index |= 0x80000000;
    }
    unsigned int * p = reinterpret_cast< unsigned int * >(code);
    OSL_ASSERT(sizeof (unsigned int) == 4);
    // 0*4: save %sp, -176, %sp ! minimal stack frame:
    *p++ = 0x9DE3BF50;
    // 1*4: rd %pc, %l0:
    *p++ = 0xA1414000;
    // 2*4: ldx %l0, (8-1)*4, %l0:
    *p++ = 0xE05C201C;
    // 3*4: sethi %hi(index), %o0:
    *p++ = 0x11000000 | (index >> 10);
    // 4*4: or %o0, %lo(index), %o0:
    *p++ = 0x90122000 | (index & 0x3FF);
    // 5*4: sethi %hi(vtableOffset), %o1:
    *p++ = 0x13000000 | (vtableOffset >> 10);
    // 6*4: jmpl %l0, %g0, %g0:
    *p++ = 0x81C40000;
    // 7*4: or %o1, %lo(vtableOffset), %o1:
    *p++ = 0x92126000 | (vtableOffset & 0x3FF);
    // 8*4: .xword privateSnippetExecutor:
    *reinterpret_cast< unsigned long * >(p) =
        reinterpret_cast< unsigned long >(vtableSlotCall);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block) {
    return static_cast< Slot * >(block) + 1;
}

sal_Size bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 3) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount)
{
    Slot * slots = mapBlockToVtable(block) + 2;
    slots[-3].fn = NULL; // RTTI
    slots[-2].fn = NULL; // null
    slots[-1].fn = NULL; // destructor
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < type->nMembers; ++i) {
        typelib_TypeDescription * member = 0;
        TYPELIB_DANGER_GET(&member, type->ppMembers[i]);
        OSL_ASSERT(member != 0);
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            // Getter:
            (s++)->fn = code;
            code = generateCodeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::cc5_solaris_sparc64::isDirectReturnType(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(
                        member)->pAttributeTypeRef));
            // Setter:
            if (!reinterpret_cast<
                typelib_InterfaceAttributeTypeDescription * >(
                    member)->bReadOnly)
            {
                (s++)->fn = code;
                code = generateCodeSnippet(
                    code, functionOffset++, vtableOffset, true);
            }
            break;

        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code;
            code = generateCodeSnippet(
                code, functionOffset++, vtableOffset,
                bridges::cpp_uno::cc5_solaris_sparc64::isDirectReturnType(
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

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const * begin, unsigned char const * end)
{
    bridges::cpp_uno::cc5_solaris_sparc64::flushCode(begin, end);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
