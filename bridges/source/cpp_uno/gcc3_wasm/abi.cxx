/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <typeinfo>

#include <bridges/emscriptencxxabi/cxxabi.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#include "abi.hxx"

abi_wasm::StructKind abi_wasm::getKind(typelib_CompoundTypeDescription const* type)
{
    if (type->nMembers > 1)
    {
        return StructKind::General;
    }
    auto k = StructKind::Empty;
    if (type->pBaseTypeDescription != nullptr)
    {
        k = getKind(type->pBaseTypeDescription);
    }
    if (type->nMembers == 0)
    {
        return k;
    }
    if (k != StructKind::Empty)
    {
        return StructKind::General;
    }
    switch (type->ppTypeRefs[0]->eTypeClass)
    {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            return StructKind::I32;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            return StructKind::I64;
        case typelib_TypeClass_FLOAT:
            return StructKind::F32;
        case typelib_TypeClass_DOUBLE:
            return StructKind::F64;
        default:
            return StructKind::General;
    }
}

void abi_wasm::mapException(__cxxabiv1::__cxa_exception* exception, std::type_info const* type,
                            uno_Any* any, uno_Mapping* mapping)
{
    assert(exception != nullptr);
    assert(type != nullptr);
    OUString unoName(emscriptencxxabi::toUnoName(type->name()));
    typelib_TypeDescription* td = nullptr;
    typelib_typedescription_getByName(&td, unoName.pData);
    if (td == nullptr)
    {
        css::uno::RuntimeException e("exception type not found: " + unoName);
        uno_type_any_constructAndConvert(
            any, &e, cppu::UnoType<css::uno::RuntimeException>::get().getTypeLibType(), mapping);
    }
    else
    {
        uno_any_constructAndConvert(any, exception->adjustedPtr, td, mapping);
        typelib_typedescription_release(td);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
