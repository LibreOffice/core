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



#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_SHARED_GUARDEDARRAY_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_SHARED_GUARDEDARRAY_HXX

namespace bridges { namespace cpp_uno { namespace shared {

template< typename T > class GuardedArray {
public:
    explicit GuardedArray(T * thePointer): pointer(thePointer) {}

    ~GuardedArray() { delete[] pointer; }

    T * get() const { return pointer; }

    T * release() { T * p = pointer; pointer = 0; return p; }

private:
    GuardedArray(GuardedArray &); // not implemented
    void operator =(GuardedArray); // not implemented

    T * pointer;
};

} } }

#endif
