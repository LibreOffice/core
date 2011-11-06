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



#include "precompiled_bridges.hxx"
#include "sal/config.h"

#include <cstddef>

#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "isdirectreturntype.hxx"

namespace {

bool isPodStruct(typelib_CompoundTypeDescription * type) {
    for (; type != NULL; type = type->pBaseTypeDescription) {
        for (sal_Int32 i = 0; i < type->nMembers; ++i) {
            if (!bridges::cpp_uno::cc5_solaris_sparc64::isDirectReturnType(
                    type->ppTypeRefs[i]))
            {
                return false;
            }
        }
    }
    return true;
}

}

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

bool isDirectReturnType(typelib_TypeDescriptionReference * type) {
    // Is POD of size <= 32 bytes:
    switch (type->eTypeClass) {
    default:
        return true;
    case typelib_TypeClass_STRING:
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_ANY:
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_INTERFACE:
        return false;
    case typelib_TypeClass_STRUCT:
        {
            typelib_TypeDescription * t = NULL;
            TYPELIB_DANGER_GET(&t, type);
            bool b = t->nSize <= 32 && isPodStruct(
                reinterpret_cast< typelib_CompoundTypeDescription * >(t));
            TYPELIB_DANGER_RELEASE(t);
            return b;
        }
    }
}

} } }
