/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include <cassert>

#include "cppu/macros.hxx"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "callvirtualmethod.hxx"
#include "share.hxx"










//






void CPPU_CURRENT_NAMESPACE::callVirtualMethod(
    void * pAdjustedThisPtr, sal_Int32 nVtableIndex, void * pRegisterReturn,
    typelib_TypeDescription * pReturnTypeDescr, bool bSimpleReturn,
    sal_Int32 * pStackLongs, sal_Int32 nStackLongs)
{
    
    

    assert(pStackLongs && pAdjustedThisPtr);
    assert(sizeof (void *) == 4 && sizeof (sal_Int32) == 4);
        
    assert(nStackLongs && pStackLongs); 

#if defined __clang__ || __GNUC__ == 4 && __GNUC_MINOR__ <= 6
    if (! pAdjustedThisPtr) CPPU_CURRENT_NAMESPACE::dummy_can_throw_anything("xxx"); 
#endif

    long edx, eax; 
    void * stackptr;
    asm volatile (
        "mov   %%esp, %2\n\t"
        
        "and   $0xfffffff0, %%esp\n\t"
        "mov   %3, %%eax\n\t"
        "lea   -4(,%%eax,4), %%eax\n\t"
        "and   $0xf, %%eax\n\t"
        "sub   $0xc, %%eax\n\t"
        "add   %%eax, %%esp\n\t"
        
        "mov   %3, %%eax\n\t"
        "mov   %%eax, %%edx\n\t"
        "dec   %%edx\n\t"
        "shl   $2, %%edx\n\t"
        "add   %4, %%edx\n"
        "Lcopy:\n\t"
        "pushl 0(%%edx)\n\t"
        "sub   $4, %%edx\n\t"
        "dec   %%eax\n\t"
        "jne   Lcopy\n\t"
        
        "mov   %5, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "mov   %6, %%eax\n\t"
        "shl   $2, %%eax\n\t"
        "add   %%eax, %%edx\n\t"
        "mov   0(%%edx), %%edx\n\t"
        "call  *%%edx\n\t"
        
         "mov   %%eax, %0\n\t"
         "mov   %%edx, %1\n\t"
        
        "mov   %2, %%esp\n\t"
        : "=m"(eax), "=m"(edx), "=m"(stackptr)
        : "m"(nStackLongs), "m"(pStackLongs), "m"(pAdjustedThisPtr), "m"(nVtableIndex)
        : "eax", "ecx", "edx" );
    switch( pReturnTypeDescr->eTypeClass )
    {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
            ((long*)pRegisterReturn)[1] = edx;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            ((long*)pRegisterReturn)[0] = eax;
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
            *(unsigned short*)pRegisterReturn = eax;
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *(unsigned char*)pRegisterReturn = eax;
            break;
        case typelib_TypeClass_FLOAT:
            asm ( "fstps %0" : : "m"(*(char *)pRegisterReturn) );
            break;
        case typelib_TypeClass_DOUBLE:
            asm ( "fstpl %0\n\t" : : "m"(*(char *)pRegisterReturn) );
            break;
        default:
        {
#if defined (FREEBSD) || defined(NETBSD) || defined(OPENBSD) || defined(MACOSX) || \
    defined(DRAGONFLY)
            sal_Int32 const nRetSize = pReturnTypeDescr->nSize;
            if (bSimpleReturn && nRetSize <= 8 && nRetSize > 0)
            {
                if (nRetSize > 4)
                    static_cast<long *>(pRegisterReturn)[1] = edx;
                static_cast<long *>(pRegisterReturn)[0] = eax;
            }
#else
            (void)bSimpleReturn;
#endif
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
