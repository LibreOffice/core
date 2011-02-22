/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "sal/config.h"

#include "osl/diagnose.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"

#include "binaryany.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

BinaryAny::BinaryAny() throw () {
    uno_any_construct(&data_, 0, 0, 0);
}

BinaryAny::BinaryAny(css::uno::TypeDescription const & type, void * value)
    throw ()
{
    OSL_ASSERT(type.is());
    uno_any_construct(&data_, value, type.get(), 0);
}

BinaryAny::BinaryAny(uno_Any const & raw) throw () {
    OSL_ASSERT(raw.pType != 0);
    data_.pType = raw.pType;
    typelib_typedescriptionreference_acquire(data_.pType);
    data_.pData = raw.pData == &raw.pReserved ? &data_.pReserved : raw.pData;
    data_.pReserved = raw.pReserved;
}

BinaryAny::BinaryAny(BinaryAny const & other) throw () {
    uno_type_any_construct(&data_, other.data_.pData, other.data_.pType, 0);
}

BinaryAny::~BinaryAny() throw () {
    uno_any_destruct(&data_, 0);
}

BinaryAny & BinaryAny::operator =(BinaryAny const & other) throw () {
    if (&other != this) {
        uno_type_any_assign(&data_, other.data_.pData, other.data_.pType, 0, 0);
    }
    return *this;
}

uno_Any * BinaryAny::get() throw () {
    return &data_;
}

css::uno::TypeDescription BinaryAny::getType() const throw () {
    return css::uno::TypeDescription(data_.pType);
}

void * BinaryAny::getValue(css::uno::TypeDescription const & type) const
    throw ()
{
    OSL_ASSERT(
        type.is() &&
        (type.get()->eTypeClass == typelib_TypeClass_ANY ||
         type.equals(css::uno::TypeDescription(data_.pType))));
    return type.get()->eTypeClass == typelib_TypeClass_ANY
        ? &data_ : data_.pData;
}

}
