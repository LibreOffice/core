/*************************************************************************
 *
 *  $RCSfile: dispatcher.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UNO_DISPATCHER_H_
#define _UNO_DISPATCHER_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

typedef struct _typelib_TypeDescription typelib_TypeDescription;
typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;
typedef struct _uno_Interface uno_Interface;

/** Function pointer declaration for the binary C uno dispatch function.<br>
    Any pure out or return value will be constructed by the callee, iff
    no exception is signalled.<br>
    If an exception is signalled, the any *ppException is properly constructed
    by the callee, otherwise the pointer *ppException is set to 0.<br>
    An attribute <b>get</b> call is indicated by a non-null return pointer.
    <br>
    @param pUnoI        uno interface the call is performed on
    @param pMemberType  member type description of a method or attribute
    @param pReturn      pointer to return value memory;
                        pointer may be undefined if void method, null if attribute set call.
    @param pArgs        an array of pointers to arguments values.
                        (remark: the value of an interface reference stores a
                         uno_interface *, so you get it by *(uno_Interface **)pArgs[n])
    @param ppException   pointer to pointer to unconstructed any to signal an exception.
*/
typedef void (SAL_CALL * uno_DispatchMethod)(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberType,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException );

#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

/** The binary C uno interface description.
    <br>
*/
typedef struct _uno_Interface
{
    /** Acquires uno interface.
        <br>
        @param pInterface uno interface
    */
    void (SAL_CALL * acquire)( uno_Interface * pInterface );
    /** Releases uno interface.
        <br>
        @param pInterface uno interface
    */
    void (SAL_CALL * release)( uno_Interface * pInterface );
    /** dispatch function<br>
    */
    uno_DispatchMethod pDispatcher;
} uno_Interface;

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

#endif
