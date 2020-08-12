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


#include <stdio.h>
#include <string.h>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>

#include "rtti.hxx"
#include "share.hxx"


using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::__cxxabiv1;


namespace CPPU_CURRENT_NAMESPACE
{

static OUString toUNOname( char const * p )
{
#if OSL_DEBUG_LEVEL > 1
    char const * start = p;
#endif

    // example: N3com3sun4star4lang24IllegalArgumentExceptionE

    OUStringBuffer buf( 64 );
    assert( *p == 'N' );
    ++p; // skip N

    while (*p != 'E')
    {
        // read chars count
        int n = *p++ - '0';
        while ('0' <= *p && '9' >= *p)
        {
            n *= 10;
            n += (*p++ - '0');
        }
        buf.appendAscii( p, n );
        p += n;
        if (*p != 'E')
            buf.append( '.' );
    }

#if OSL_DEBUG_LEVEL > 1
    OUString ret( buf.makeStringAndClear() );
    OString c_ret( OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> toUNOname(): %s => %s\n", start, c_ret.getStr() );
    return ret;
#else
    return buf.makeStringAndClear();
#endif
}

extern "C" {
static void _GLIBCXX_CDTOR_CALLABI deleteException( void * pExc )
{
    __cxxabiv1::__cxa_exception const * header = static_cast<__cxxabiv1::__cxa_exception const *>(pExc) - 1;
    typelib_TypeDescription * pTD = nullptr;
    OUString unoName( toUNOname( header->exceptionType->name() ) );
    ::typelib_typedescription_getByName( &pTD, unoName.pData );
    assert(pTD && "### unknown exception type! leaving out destruction => leaking!!!");
    if (pTD)
    {
        ::uno_destructData( pExc, pTD, cpp_release );
        ::typelib_typedescription_release( pTD );
    }
}
}

void raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
#if OSL_DEBUG_LEVEL > 1
    OString cstr(
        OUStringToOString(
            OUString::unacquired( &pUnoExc->pType->pTypeName ),
            RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> uno exception occurred: %s\n", cstr.getStr() );
#endif
    void * pCppExc;
    type_info * rtti;

    {
    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = nullptr;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );
    assert(pTypeDescr);
    if (! pTypeDescr)
    {
        throw RuntimeException(
            "cannot get typedescription for type " +
            OUString::unacquired( &pUnoExc->pType->pTypeName ) );
    }

    pCppExc = __cxxabiv1::__cxa_allocate_exception( pTypeDescr->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // destruct uno exception
    ::uno_any_destruct( pUnoExc, nullptr );
    // avoiding locked counts
    rtti = x86_64::getRtti(*pTypeDescr);
    TYPELIB_DANGER_RELEASE( pTypeDescr );
    assert(rtti && "### no rtti for throwing exception!");
    if (! rtti)
    {
        throw RuntimeException(
            "no rtti for type " +
            OUString::unacquired( &pUnoExc->pType->pTypeName ) );
    }
    }

    __cxxabiv1::__cxa_throw( pCppExc, rtti, deleteException );
}

void fillUnoException(uno_Any * pUnoExc, uno_Mapping * pCpp2Uno)
{
    __cxxabiv1::__cxa_exception * header = __cxxabiv1::__cxa_get_globals()->caughtExceptions;
    if (! header)
    {
        RuntimeException aRE( "no exception header!" );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges", aRE.Message);
        return;
    }

#if defined _LIBCPPABI_VERSION // detect libc++abi
    // Very bad HACK to find out whether we run against a libcxxabi that has a new
    // __cxa_exception::reserved member at the start, introduced with LLVM 10
    // <https://github.com/llvm/llvm-project/commit/674ec1eb16678b8addc02a4b0534ab383d22fa77>
    // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility".  The layout of the
    // start of __cxa_exception is
    //
    //  [8 byte  void *reserve]
    //   8 byte  size_t referenceCount
    //
    // where the (bad, hacky) assumption is that reserve (if present) is null
    // (__cxa_allocate_exception in at least LLVM 11 zero-fills the object, and nothing actively
    // sets reserve) while referenceCount is non-null (__cxa_throw sets it to 1, and
    // __cxa_decrement_exception_refcount destroys the exception as soon as it drops to 0; for a
    // __cxa_dependent_exception, the referenceCount member is rather
    //
    //   8 byte  void* primaryException
    //
    // but which also will always be set to a non-null value in __cxa_rethrow_primary_exception).
    // As described in the definition of __cxa_exception
    // (bridges/source/cpp_uno/gcc3_linux_x86-64/share.hxx), this hack (together with the "#if 0"
    // there) can be dropped once we can be sure that we only run against new libcxxabi that has the
    // reserve member:
    if (*reinterpret_cast<void **>(header) == nullptr) {
        header = reinterpret_cast<__cxxabiv1::__cxa_exception*>(reinterpret_cast<void **>(header) + 1);
    }
#endif

    std::type_info *exceptionType = __cxxabiv1::__cxa_current_exception_type();

    typelib_TypeDescription * pExcTypeDescr = nullptr;
    OUString unoName( toUNOname( exceptionType->name() ) );
#if OSL_DEBUG_LEVEL > 1
    OString cstr_unoName( OUStringToOString( unoName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> c++ exception occurred: %s\n", cstr_unoName.getStr() );
#endif
    typelib_typedescription_getByName( &pExcTypeDescr, unoName.pData );
    if (pExcTypeDescr == nullptr)
    {
        RuntimeException aRE( "exception type not found: " + unoName );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges", aRE.Message);
    }
    else
    {
        // construct uno exception any
        uno_any_constructAndConvert( pUnoExc, header->adjustedPtr, pExcTypeDescr, pCpp2Uno );
        typelib_typedescription_release( pExcTypeDescr );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
