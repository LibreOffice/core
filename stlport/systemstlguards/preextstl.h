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


//1. Force inclusion of a std:: using header to ensure the stlport define
//of std as "stlport"
#include <algorithm>
#if defined(ADAPT_EXT_STL)
//2. Force inclusion of stlport headers to get their stlport:: definitions
#   include <ostream>
#   include <istream>
#   include <fstream>
#   include <iostream>
#   include <sstream>
#   include <vector>
#   include <deque>
#   include <set>
#   include <list>
#   include <map>
//3. Now force inclusion of native headers to get their std:: definitions
#   if defined(std)
#       define std_was_redefined_as_stlport std
#       undef std
#       define _STLP_OUTERMOST_HEADER_ID 0xdeadbeaf
#       if defined(_GNUC__)
#           pragma GCC visibility push(default)
#       endif
#       include _STLP_NATIVE_HEADER(exception_defines.h)
#       include _STLP_NATIVE_HEADER(limits)
#       include _STLP_NATIVE_HEADER(memory)
#       include _STLP_NATIVE_HEADER(exception)
#       include _STLP_NATIVE_HEADER(iosfwd)
#       include _STLP_NATIVE_HEADER(algorithm)
#       include _STLP_NATIVE_HEADER(string)
#       include _STLP_NATIVE_HEADER(streambuf)
#       include _STLP_NATIVE_HEADER(ios)
#       include _STLP_NATIVE_HEADER(locale)
#       include _STLP_NATIVE_HEADER(stdexcept)
#       include _STLP_NATIVE_HEADER(ostream)
#       include _STLP_NATIVE_HEADER(istream)
#       include _STLP_NATIVE_HEADER(iostream)
#       include _STLP_NATIVE_HEADER(sstream)
#       include _STLP_NATIVE_HEADER(vector)
#       include _STLP_NATIVE_HEADER(deque)
#       include _STLP_NATIVE_HEADER(set)
#       include _STLP_NATIVE_HEADER(list)
#       include _STLP_NATIVE_HEADER(map)
#       if defined(_GNUC__)
#           pragma GCC visibility pop
#       endif
#   endif
#endif
//ext_std resolves to the std that external c++ libs, e.g. Graphite were built
//against regardless of whether that is stlport or system stl
namespace ext_std = std;
