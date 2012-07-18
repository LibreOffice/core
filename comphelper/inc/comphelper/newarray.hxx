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

#ifndef COMPHELPER_NEW_ARRAY_HXX
#define COMPHELPER_NEW_ARRAY_HXX

#include <limits>
#include <new>
#include <stddef.h>

namespace comphelper {

template<typename T> T *
newArray_null(size_t const n) throw()
{
    if ((::std::numeric_limits<size_t>::max() / sizeof(T)) <= n) {
        return 0;
    }
    return new (::std::nothrow) T[n];
}

template<typename T> T *
newArray_ex(size_t const n)
{
    if ((::std::numeric_limits<size_t>::max() / sizeof(T)) <= n) {
        throw ::std::bad_alloc();
    }
    return new T[n];
}

} // namespace comphelper

#endif // COMPHELPER_NEW_ARRAY_HXX
