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
