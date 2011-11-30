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



// Turn off DEBUG Assertions
#ifdef _DEBUG
    #define _DEBUG_WAS_DEFINED _DEBUG
    #ifndef MOZILLA_ENABLE_DEBUG
        #undef _DEBUG
    #endif
#else
    #undef _DEBUG_WAS_DEFINED
    #ifdef MOZILLA_ENABLE_DEBUG
        #define _DEBUG 1
    #endif
#endif

// and turn off the additional virtual methods which are part of some interfaces when compiled
// with debug
#ifdef DEBUG
    #define DEBUG_WAS_DEFINED DEBUG
    #ifndef MOZILLA_ENABLE_DEBUG
        #undef DEBUG
    #endif
#else
    #undef DEBUG_WAS_DEFINED
    #ifdef MOZILLA_ENABLE_DEBUG
        #define DEBUG 1
    #endif
#endif

#if defined __GNUC__
    #pragma GCC system_header
#elif defined __SUNPRO_CC
    #pragma disable_warn
#elif defined _MSC_VER
    #pragma warning(push, 1)
    #pragma warning(disable:4946 4710)
#endif

