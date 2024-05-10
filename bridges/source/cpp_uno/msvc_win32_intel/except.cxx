/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/config.h>
#include <malloc.h>
#include <typeinfo>
#include <signal.h>

#include <rtl/alloc.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <unordered_map>
#include <msvc/x86.hxx>
#include <except.hxx>

#pragma pack(push, 8)

using namespace ::com::sun::star;

void * ObjectFunction::operator new ( size_t nSize )
{
    void * pMem = std::malloc( nSize );
    if (pMem != 0)
    {
        DWORD old_protect;
        BOOL success =
            VirtualProtect(pMem, nSize, PAGE_EXECUTE_READWRITE, &old_protect);
        (void) success;
        assert(success && "VirtualProtect() failed!");
    }
    return pMem;
}

void ObjectFunction::operator delete ( void * pMem )
{
    std::free( pMem );
}

ObjectFunction::ObjectFunction( typelib_TypeDescription * pTypeDescr, void * fpFunc ) throw ()
    : _pTypeDescr( pTypeDescr )
{
    ::typelib_typedescription_acquire( _pTypeDescr );

    unsigned char * pCode = (unsigned char *)somecode;
    // a must be!
    assert((void *)this == (void *)pCode);

    // push ObjectFunction this
    *pCode++ = 0x68;
    *(void **)pCode = this;
    pCode += sizeof(void *);
    // jmp rel32 fpFunc
    *pCode++ = 0xe9;
    *(sal_Int32 *)pCode = ((unsigned char *)fpFunc) - pCode - sizeof(sal_Int32);
}

ObjectFunction::~ObjectFunction() throw ()
{
    ::typelib_typedescription_release( _pTypeDescr );
}

static void * __cdecl __copyConstruct( void * pExcThis, void * pSource, ObjectFunction * pThis )
    throw ()
{
    ::uno_copyData(pExcThis, pSource, pThis->_pTypeDescr, uno::cpp_acquire);
    return pExcThis;
}

static void * __cdecl __destruct( void * pExcThis, ObjectFunction * pThis )
    throw ()
{
    ::uno_destructData(pExcThis, pThis->_pTypeDescr, uno::cpp_release);
    return pExcThis;
}

// these are non virtual object methods; there is no this ptr on stack => ecx supplies _this_ ptr

static __declspec(naked) void copyConstruct() throw ()
{
    __asm
    {
        // ObjectFunction this already on stack
        push [esp+8]  // source exc object this
        push ecx      // exc object
        call __copyConstruct
        add  esp, 12  // + ObjectFunction this
        ret  4
    }
}

static __declspec(naked) void destruct() throw ()
{
    __asm
    {
        // ObjectFunction this already on stack
        push ecx    // exc object
        call __destruct
        add  esp, 8 // + ObjectFunction this
        ret
    }
}

ExceptionType::ExceptionType( typelib_TypeDescription * pTypeDescr ) throw ()
    : _n0( 0 )
    , _n1( 0 )
    , _n2( -1 )
    , _n3( 0 )
    , _n4( pTypeDescr->nSize )
    , _pCopyCtor( new ObjectFunction( pTypeDescr, copyConstruct ) )
    , _n5( 0 )
{
    _pTypeInfo = RTTInfos::get(pTypeDescr->pTypeName);
}

ExceptionType::~ExceptionType() throw ()
{
    delete _pCopyCtor;
}

RaiseInfo::RaiseInfo( typelib_TypeDescription * pTypeDescr ) throw ()
    : _n0( 0 )
    , _pDtor( new ObjectFunction( pTypeDescr, destruct ) )
    , _n2( 0 )
    , _n3( 0 )
    , _n4( 0 )
{
    // a must be
    static_assert(sizeof(sal_Int32) == sizeof(ExceptionType *), "### pointer size differs from sal_Int32!");

    typelib_CompoundTypeDescription * pCompTypeDescr;

    // info count
    sal_Int32 nLen = 0;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        ++nLen;
    }

    // info count accompanied by type info ptrs: type, base type, base base type, ...
    _types = std::malloc( sizeof(sal_Int32) + (sizeof(ExceptionType *) * nLen) );
    assert(_types && "Don't handle OOM conditions");
    *(sal_Int32 *)_types = nLen;

    ExceptionType ** ppTypes = (ExceptionType **)((sal_Int32 *)_types + 1);

    sal_Int32 nPos = 0;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        ppTypes[nPos++] = new ExceptionType( (typelib_TypeDescription *)pCompTypeDescr );
    }
}

RaiseInfo::~RaiseInfo() throw ()
{
    ExceptionType ** ppTypes = (ExceptionType **)((sal_Int32 *)_types + 1);
    for ( sal_Int32 nTypes = *(sal_Int32 *)_types; nTypes--; )
    {
        delete ppTypes[nTypes];
    }
    std::free( _types );

    delete _pDtor;
}

#pragma pack(pop)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
