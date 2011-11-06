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

#include <algorithm>

#include "osl/diagnose.h"
#include "rtl/byteseq.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.hxx"

#include "lessoperators.hxx"

namespace com { namespace sun { namespace star { namespace uno {

bool operator <(TypeDescription const & left, TypeDescription const & right) {
    OSL_ASSERT(left.is() && right.is());
    typelib_TypeClass tc1 = left.get()->eTypeClass;
    typelib_TypeClass tc2 = right.get()->eTypeClass;
    return tc1 < tc2 ||
        (tc1 == tc2 &&
         (rtl::OUString(left.get()->pTypeName) <
          rtl::OUString(right.get()->pTypeName)));
}

} } } }

namespace rtl {

bool operator <(ByteSequence const & left, ByteSequence const & right) {
    for (sal_Int32 i = 0; i != std::min(left.getLength(), right.getLength());
         ++i)
    {
        if (left[i] < right[i]) {
            return true;
        }
        if (right[i] < left[i]) {
            return false;
        }
    }
    return left.getLength() < right.getLength();
}

}
