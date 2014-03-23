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



#ifndef INCLUDED_BINARYURP_SOURCE_LESSOPERATORS_HXX
#define INCLUDED_BINARYURP_SOURCE_LESSOPERATORS_HXX

#include "sal/config.h"

namespace com { namespace sun { namespace star { namespace uno {
    class TypeDescription;
} } } }
namespace rtl { class ByteSequence; }

namespace com { namespace sun { namespace star { namespace uno {

bool operator <(TypeDescription const & left, TypeDescription const & right);

struct TypeDescHash { sal_Int32 operator()( const TypeDescription&) const; };

struct TypeDescEqual { bool operator()( const TypeDescription&, const TypeDescription&) const; };

} } } }

namespace rtl {

bool operator <(ByteSequence const & left, ByteSequence const & right);

}

#endif
