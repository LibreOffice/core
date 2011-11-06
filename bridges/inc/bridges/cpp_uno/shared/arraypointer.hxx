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



#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_ARRAYPOINTER_HXX

#include "sal/config.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * A simple smart pointer that holds an array until it is being released.
 */
template< typename T > class ArrayPointer {
public:
    ArrayPointer(T * p): p_(p) {}

    ~ArrayPointer() { delete[] p_; }

    T * release() { T * t = p_; p_ = 0; return t; }

private:
    ArrayPointer(ArrayPointer &); // not defined
    void operator =(ArrayPointer &); // not defined

    T * p_;
};

} } }

#endif
