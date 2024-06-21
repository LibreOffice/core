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
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <uno/mapping.h>

#include "abi.hxx"

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
