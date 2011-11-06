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



#ifndef INCLUDED_CODEMAKER_CODEMAKER_HXX
#define INCLUDED_CODEMAKER_CODEMAKER_HXX

#include "sal/config.h"
#include "codemaker/unotype.hxx"
#include "registry/types.h"
#include "sal/types.h"

#include <vector>

namespace rtl {
    class OString;
    class OUString;
}
class TypeManager;

namespace codemaker {

rtl::OString convertString(rtl::OUString const & string);

codemaker::UnoType::Sort decomposeAndResolve(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments);

}

#endif // INCLUDED_CODEMAKER_CODEMAKER_HXX
