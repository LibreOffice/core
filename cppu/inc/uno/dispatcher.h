/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dispatcher.h,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
