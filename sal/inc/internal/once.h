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



#ifndef INCLUDED_SAL_INTERNAL_ONCE_H
#define INCLUDED_SAL_INTERNAL_ONCE_H

/** sal_once_type
 *  (platform dependent)
 */

#if defined(SAL_UNX) || defined(SAL_OS2)

#include <pthread.h>

typedef pthread_once_t sal_once_type;

#define SAL_ONCE_INIT PTHREAD_ONCE_INIT
#define SAL_ONCE(once, init) pthread_once((once), (init))

#elif defined(SAL_W32)

#define WIN32_LEAN_AND_MEAN
#pragma warning(push,1) /* disable warnings within system headers */
#include <windows.h>
#pragma warning(pop)

typedef struct sal_once_st sal_once_type;
struct sal_once_st
{
    LONG volatile m_done;
    LONG volatile m_lock;
};

#define SAL_ONCE_INIT { 0, 0 }
#define SAL_ONCE(once, init) \
{ \
    sal_once_type * control = (once); \
    if (!(control->m_done)) \
    { \
        while (InterlockedExchange(&(control->m_lock), 1) == 1) Sleep(0); \
        if (!(control->m_done)) \
        { \
            void (*init_routine)(void) = (init); \
            (*init_routine)(); \
            control->m_done = 1; \
        } \
        InterlockedExchange(&(control->m_lock), 0); \
    } \
}

#else
#error Unknown platform
#endif /* SAL_UNX | SAL_W32 */

#endif /* INCLUDED_SAL_INTERNAL_ONCE_H */
