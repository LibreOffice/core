/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"

#include <msvc/x86.hxx>
#include <msvc/cpp2uno.hxx>

using namespace ::com::sun::star;

namespace
{

/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static __declspec(naked) void __cdecl cpp_vtable_call()
{
__asm
    {
        sub     esp, 8      // space for immediate return type
        push    esp
        push    edx         // vtable offset
        push    eax         // function index
        mov     eax, esp
        add     eax, 20
        push    eax         // original stack ptr

        call    cpp_mediate
        add     esp, 16

        cmp     eax, typelib_TypeClass_FLOAT
        je      Lfloat
        cmp     eax, typelib_TypeClass_DOUBLE
        je      Ldouble
        cmp     eax, typelib_TypeClass_HYPER
        je      Lhyper
        cmp     eax, typelib_TypeClass_UNSIGNED_HYPER
        je      Lhyper
        // rest is eax
        pop     eax
        add     esp, 4
        ret
Lhyper:
        pop     eax
        pop     edx
        ret
Lfloat:
        fld     dword ptr [esp]
        add     esp, 8
        ret
Ldouble:
        fld     qword ptr [esp]
        add     esp, 8
        ret
    }
}

int const codeSnippetSize = 16;

unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset)
{
    unsigned char * p = code;
    static_assert(sizeof (sal_Int32) == 4, "boo");
    // mov eax, functionIndex:
    *p++ = 0xB8;
    *reinterpret_cast< sal_Int32 * >(p) = functionIndex;
    p += sizeof (sal_Int32);
    // mov edx, vtableOffset:
    *p++ = 0xBA;
    *reinterpret_cast< sal_Int32 * >(p) = vtableOffset;
    p += sizeof (sal_Int32);
    // jmp rel32 cpp_vtable_call:
    *p++ = 0xE9;
    *reinterpret_cast< sal_Int32 * >(p)
        = ((unsigned char *) cpp_vtable_call) - p - sizeof (sal_Int32);
    p += sizeof (sal_Int32);
    assert(p - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 1;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    struct Rtti {
        sal_Int32 n0, n1, n2;
        type_info * rtti;
        Rtti():
            n0(0), n1(0), n2(0),
            rtti(RTTInfos::get("com.sun.star.uno.XInterface"))
        {}
    };
    static Rtti rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const *, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < functionCount; ++i) {
        (s++)->fn = code;
        code = codeSnippet(code, functionOffset++, vtableOffset);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *,
    unsigned char const *)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
