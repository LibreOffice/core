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

#include <sal/config.h>

#include <cassert>
#include <utility>

#include <typelib/typeclass.h>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>

#include "binaryany.hxx"

namespace binaryurp {

namespace {

// Cf. com::sun::star::uno::detail::moveAnyInternals in
// include/com/sun/star/uno/Any.hxx:
void moveInternals(uno_Any & from, uno_Any & to) {
    uno_any_construct(&to, nullptr, nullptr, nullptr);
    std::swap(from.pType, to.pType);
    std::swap(from.pData, to.pData);
    std::swap(from.pReserved, to.pReserved);
    if (to.pData == &from.pReserved) {
        to.pData = &to.pReserved;
    }
    // This leaves to.pData (where "to" is now VOID) dangling to somewhere (cf.
    // CONSTRUCT_EMPTY_ANY, cppu/source/uno/prim.hxx), but what's relevant is
    // only that it isn't a nullptr (as e.g. >>= -> uno_type_assignData ->
    // _assignData takes a null pSource to mean "construct a default value").
}

}

BinaryAny::BinaryAny() noexcept {
    uno_any_construct(&data_, nullptr, nullptr, nullptr);
}

BinaryAny::BinaryAny(css::uno::TypeDescription const & type, void * value)
    noexcept
{
    assert(type.is());
    uno_any_construct(&data_, value, type.get(), nullptr);
}

BinaryAny::BinaryAny(uno_Any const & raw) noexcept {
    assert(raw.pType != nullptr);
    data_.pType = raw.pType;
    typelib_typedescriptionreference_acquire(data_.pType);
    data_.pData = raw.pData == &raw.pReserved ? &data_.pReserved : raw.pData;
    data_.pReserved = raw.pReserved;
}

BinaryAny::BinaryAny(BinaryAny const & other) noexcept {
    uno_type_any_construct(&data_, other.data_.pData, other.data_.pType, nullptr);
}

BinaryAny::BinaryAny(BinaryAny && other) noexcept {
    moveInternals(other.data_, data_);
}

BinaryAny::~BinaryAny() noexcept {
    uno_any_destruct(&data_, nullptr);
}

BinaryAny & BinaryAny::operator =(BinaryAny const & other) noexcept {
    if (&other != this) {
        uno_type_any_assign(&data_, other.data_.pData, other.data_.pType, nullptr, nullptr);
    }
    return *this;
}

BinaryAny & BinaryAny::operator =(BinaryAny && other) noexcept {
    uno_any_destruct(&data_, nullptr);
    moveInternals(other.data_, data_);
    return *this;
}

css::uno::TypeDescription BinaryAny::getType() const noexcept {
    return css::uno::TypeDescription(data_.pType);
}

void * BinaryAny::getValue(css::uno::TypeDescription const & type) const
    noexcept
{
    assert(type.is());
    assert(
        type.get()->eTypeClass == typelib_TypeClass_ANY ||
         type.equals(css::uno::TypeDescription(data_.pType)));
    return type.get()->eTypeClass == typelib_TypeClass_ANY
        ? &data_ : data_.pData;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
