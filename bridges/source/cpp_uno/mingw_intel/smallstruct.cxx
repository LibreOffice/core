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

#include "bridges/cpp_uno/shared/types.hxx"

#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace bridges { namespace cpp_uno { namespace shared {

namespace {
bool isSimpleStruct(typelib_TypeDescription const * type) {
    switch (type->eTypeClass) {
    case typelib_TypeClass_STRUCT:
        {
            typelib_CompoundTypeDescription const * p
                = reinterpret_cast< typelib_CompoundTypeDescription const * >(
                    type);
            for (sal_Int32 i = 0; i < p->nMembers; ++i) {
                switch (p->ppTypeRefs[i]->eTypeClass) {
                case typelib_TypeClass_STRUCT:
                    {
                        typelib_TypeDescription * t = 0;
                        TYPELIB_DANGER_GET(&t, p->ppTypeRefs[i]);
                        bool b = isSimpleStruct(t);
                        TYPELIB_DANGER_RELEASE(t);
                        if (!b) {
                            return false;
                        }
                    }
                    break;

                default:
                    if (!isSimpleType(p->ppTypeRefs[i]->eTypeClass))
            return false;
            break;
                }
            }
        }
        return true;

    default:
        return false;
    }
}
}

bool isSmallStruct(typelib_TypeDescription const * type) {
    return (type->nSize <= 8 && isSimpleStruct(type));
}

} } }
