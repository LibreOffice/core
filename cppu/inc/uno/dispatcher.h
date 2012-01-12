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


#ifndef _UNO_DISPATCHER_H_
#define _UNO_DISPATCHER_H_

#include <sal/types.h>
#include <rtl/ustring.h>
#include <uno/any2.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _typelib_TypeDescription;
struct _uno_Interface;

/** Function pointer declaration for the binary C uno dispatch function. Any pure out or return
    value will be constructed by the callee, iff no exception is signalled.
    If an exception is signalled, the any *ppException is properly constructed by the callee,
    otherwise the pointer *ppException is set to 0.
    An attribute get call is indicated by a non-null return pointer.

    @param pUnoI        uno interface the call is performed on
    @param pMemberType  member type description of a method or attribute
    @param pReturn      pointer to return value memory;
                        pointer may be undefined if void method, null if attribute set call.
    @param pArgs        an array of pointers to arguments values.
                        (remark: the value of an interface reference stores a
                         uno_interface *, so you get it by *(uno_Interface **)pArgs[n])
    @param ppException  pointer to pointer to unconstructed any to signal an exception.
*/
typedef void (SAL_CALL * uno_DispatchMethod)(
    struct _uno_Interface * pUnoI,
    const struct _typelib_TypeDescription * pMemberType,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException );

#if defined( SAL_W32)
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(push, 8)
#endif

/** The binary C uno interface description.
*/
typedef struct _uno_Interface
{
    /** Acquires uno interface.

        @param pInterface uno interface
    */
    void (SAL_CALL * acquire)( struct _uno_Interface * pInterface );
    /** Releases uno interface.

        @param pInterface uno interface
    */
    void (SAL_CALL * release)( struct _uno_Interface * pInterface );
    /** dispatch function
    */
    uno_DispatchMethod pDispatcher;
} uno_Interface;

#if defined( SAL_W32) ||  defined(SAL_OS2)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif
