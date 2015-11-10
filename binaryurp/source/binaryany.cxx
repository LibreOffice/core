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

#include "sal/config.h"

#include <cassert>

#include "typelib/typeclass.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"

#include "binaryany.hxx"

namespace binaryurp {

BinaryAny::BinaryAny() throw () {
    uno_any_construct(&data_, nullptr, nullptr, nullptr);
}

BinaryAny::BinaryAny(css::uno::TypeDescription const & type, void * value)
    throw ()
{
    assert(type.is());
    uno_any_construct(&data_, value, type.get(), nullptr);
}

BinaryAny::BinaryAny(uno_Any const & raw) throw () {
    assert(raw.pType != nullptr);
    data_.pType = raw.pType;
    typelib_typedescriptionreference_acquire(data_.pType);
    data_.pData = raw.pData == &raw.pReserved ? &data_.pReserved : raw.pData;
    data_.pReserved = raw.pReserved;
}

BinaryAny::BinaryAny(BinaryAny const & other) throw () {
    uno_type_any_construct(&data_, other.data_.pData, other.data_.pType, nullptr);
}

BinaryAny::~BinaryAny() throw () {
    uno_any_destruct(&data_, nullptr);
}

BinaryAny & BinaryAny::operator =(BinaryAny const & other) throw () {
    if (&other != this) {
        uno_type_any_assign(&data_, other.data_.pData, other.data_.pType, nullptr, nullptr);
    }
    return *this;
}

css::uno::TypeDescription BinaryAny::getType() const throw () {
    return css::uno::TypeDescription(data_.pType);
}

void * BinaryAny::getValue(css::uno::TypeDescription const & type) const
    throw ()
{
    assert(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_ANY ||
         type.equals(css::uno::TypeDescription(data_.pType))));
    return type.get()->eTypeClass == typelib_TypeClass_ANY
        ? &data_ : data_.pData;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
