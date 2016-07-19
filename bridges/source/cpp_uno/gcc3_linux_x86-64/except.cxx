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
#include "com/sun/star/uno/RuntimeException.hpp"
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
    assert( 'N' == *p );
    ++p; // skip N

    while ('E' != *p)
    {
        // read chars count
        long n = (*p++ - '0');
        while ('0' <= *p && '9' >= *p)
        {
            n *= 10;
            n += (*p++ - '0');
        }
        buf.appendAscii( p, n );
        p += n;
        if ('E' != *p)
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
    __cxa_exception const * header = (static_cast<__cxa_exception const *>(pExc) - 1);
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

void fillUnoException( __cxa_exception * header, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno )
{
    if (! header)
    {
        RuntimeException aRE( "no exception header!" );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges", aRE.Message);
        return;
    }

    typelib_TypeDescription * pExcTypeDescr = nullptr;
    OUString unoName( toUNOname( header->exceptionType->name() ) );
#if OSL_DEBUG_LEVEL > 1
    OString cstr_unoName( OUStringToOString( unoName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> c++ exception occurred: %s\n", cstr_unoName.getStr() );
#endif
    typelib_typedescription_getByName( &pExcTypeDescr, unoName.pData );
    if (nullptr == pExcTypeDescr)
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
