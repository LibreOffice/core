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



#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef MOZ_BOOL
# undef BOOL
# undef Bool
#endif

#ifdef DEBUG_WAS_DEFINED
    #define DEBUG DEBUG_WAS_DEFINED
#endif

#ifdef _DEBUG_WAS_DEFINED
    #define _DEBUG _DEBUG_WAS_DEFINED
#endif

#ifndef _CONNECTIVITY_MOZILLA_REINTERPRET_CAST_MINGW_ONLY_
#define _CONNECTIVITY_MOZILLA_REINTERPRET_CAST_MINGW_ONLY_

#ifdef __cplusplus
#ifdef __MINGW32__
template<class T1, class T2> T1 inline reinterpret_cast_mingw_only(T2 p) { return reinterpret_cast<T1>(p); }
#else
template<class T1, class T2> T1 inline reinterpret_cast_mingw_only(T2 p) { return p; }
#endif
#endif

#endif
