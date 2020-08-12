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

#include <cassert>
#include <new>
#include <stdio.h>
#include <string.h>
#include <typeinfo>

#include <cxxabi.h>
#include <dlfcn.h>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <sal/log.hxx>
#include <osl/mutex.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <unordered_map>
#include "share.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE {

namespace {

struct Fake_type_info {
    virtual ~Fake_type_info() = delete;
    char const * name;
};

struct Fake_class_type_info: Fake_type_info {
    virtual ~Fake_class_type_info() override = delete;
};

struct Fake_si_class_type_info: Fake_class_type_info {
    virtual ~Fake_si_class_type_info() override = delete;
    void const * base;
};

struct Base {};
struct Derived: Base {};

std::type_info * createFake_class_type_info(char const * name) {
    char * buf = new char[sizeof (Fake_class_type_info)];

    *reinterpret_cast<void **>(buf) = *reinterpret_cast<void * const *>(
        &typeid(Base));
        // copy __cxxabiv1::__class_type_info vtable into place
    Fake_class_type_info * fake = reinterpret_cast<Fake_class_type_info *>(buf);
    fake->name = name;
    return reinterpret_cast<std::type_info *>(
        static_cast<Fake_type_info *>(fake));
}

std::type_info * createFake_si_class_type_info(
    char const * name, std::type_info const * base)
{
    char * buf = new char[sizeof (Fake_si_class_type_info)];

    *reinterpret_cast<void **>(buf) = *reinterpret_cast<void * const *>(
        &typeid(Derived));
        // copy __cxxabiv1::__si_class_type_info vtable into place
    Fake_si_class_type_info * fake
        = reinterpret_cast<Fake_si_class_type_info *>(buf);
    fake->name = name;
    fake->base = base;
    return reinterpret_cast<std::type_info *>(
        static_cast<Fake_type_info *>(fake));
}

}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
void dummy_can_throw_anything( char const * )
{
}
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

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
        long n = *p++ - '0';
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

namespace {

class RTTI
{
    typedef std::unordered_map< OUString, std::type_info * > t_rtti_map;

    Mutex m_mutex;
    t_rtti_map m_rttis;
    t_rtti_map m_generatedRttis;

    void * m_hApp;

public:
    RTTI();
    ~RTTI();

    std::type_info * getRTTI( typelib_CompoundTypeDescription * );
};

}

RTTI::RTTI()
    : m_hApp( dlopen( nullptr, RTLD_LAZY ) )
{
}

RTTI::~RTTI()
{
    dlclose( m_hApp );
}


std::type_info * RTTI::getRTTI( typelib_CompoundTypeDescription *pTypeDescr )
{
    std::type_info * rtti;

    OUString const & unoName = OUString::unacquired(&pTypeDescr->aBase.pTypeName);

    MutexGuard guard( m_mutex );
    t_rtti_map::const_iterator iFind( m_rttis.find( unoName ) );
    if (iFind == m_rttis.end())
    {
        // RTTI symbol
        OStringBuffer buf( 64 );
        buf.append( "_ZTIN" );
        sal_Int32 index = 0;
        do
        {
            OUString token( unoName.getToken( 0, '.', index ) );
            buf.append( token.getLength() );
            OString c_token( OUStringToOString( token, RTL_TEXTENCODING_ASCII_US ) );
            buf.append( c_token );
        }
        while (index >= 0);
        buf.append( 'E' );

        OString symName( buf.makeStringAndClear() );
        rtti = static_cast<std::type_info *>(dlsym( m_hApp, symName.getStr() ));

        if (rtti)
        {
            std::pair< t_rtti_map::iterator, bool > insertion(
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            SAL_WARN_IF( !insertion.second,
                         "bridges",
                         "inserting new rtti failed" );
        }
        else
        {
            // try to lookup the symbol in the generated rtti map
            t_rtti_map::const_iterator iFind2( m_generatedRttis.find( unoName ) );
            if (iFind2 == m_generatedRttis.end())
            {
                // we must generate it !
                // symbol and rtti-name is nearly identical,
                // the symbol is prefixed with _ZTI
                char * rttiName = strdup(symName.getStr() + 4);
                if (rttiName == nullptr) {
                    throw std::bad_alloc();
                }
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr,"generated rtti for %s\n", rttiName );
#endif
                if (pTypeDescr->pBaseTypeDescription)
                {
                    // ensure availability of base
                    std::type_info * base_rtti = getRTTI(
                        pTypeDescr->pBaseTypeDescription );
                    rtti = createFake_si_class_type_info(rttiName, base_rtti);
                }
                else
                {
                    rtti = createFake_class_type_info(rttiName);
                }

                std::pair< t_rtti_map::iterator, bool > insertion(
                    m_generatedRttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
                SAL_WARN_IF( !insertion.second,
                             "bridges",
                             "inserting new generated rtti failed" );
            }
            else // taking already generated rtti
            {
                rtti = iFind2->second;
            }
        }
    }
    else
    {
        rtti = iFind->second;
    }

    return rtti;
}


static void deleteException( void * pExc )
{
    __cxa_exception const * header = static_cast<__cxa_exception const *>(pExc) - 1;
    // The libcxxabi commit
    // <http://llvm.org/viewvc/llvm-project?view=revision&revision=303175>
    // "[libcxxabi] Align unwindHeader on a double-word boundary" towards
    // LLVM 5.0 changed the size of __cxa_exception by adding
    //
    //   __attribute__((aligned))
    //
    // to the final member unwindHeader, on x86-64 effectively adding a hole of
    // size 8 in front of that member (changing its offset from 88 to 96,
    // sizeof(__cxa_exception) from 120 to 128, and alignof(__cxa_exception)
    // from 8 to 16); a hack to dynamically determine whether we run against a
    // new libcxxabi is to look at the exceptionDestructor member, which must
    // point to this function (the use of __cxa_exception in fillUnoException is
    // unaffected, as it only accesses members towards the start of the struct,
    // through a pointer known to actually point at the start).  The libcxxabi commit
    // <https://github.com/llvm/llvm-project/commit/674ec1eb16678b8addc02a4b0534ab383d22fa77>
    // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility" in LLVM 10
    // removes the need for this hack, so it can be removed again once we can be sure that we only
    // run against libcxxabi from LLVM >= 10:
    if (header->exceptionDestructor != &deleteException) {
        header = reinterpret_cast<__cxa_exception const *>(
            reinterpret_cast<char const *>(header) - 8);
        assert(header->exceptionDestructor == &deleteException);
    }
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
    std::type_info * rtti;

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
    static RTTI rtti_data;
    rtti = rtti_data.getRTTI(reinterpret_cast<typelib_CompoundTypeDescription*>(pTypeDescr));
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
    __cxa_exception * header = __cxa_get_globals()->caughtExceptions;
    if (! header)
    {
        RuntimeException aRE( "no exception header!" );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges", aRE.Message);
        return;
    }

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
    // (bridges/source/cpp_uno/gcc3_macosx_x86-64/share.hxx), this hack (together with the "#if 0"
    // there) can be dropped once we can be sure that we only run against new libcxxabi that has the
    // reserve member:
    if (*reinterpret_cast<void **>(header) == nullptr) {
        header = reinterpret_cast<__cxa_exception *>(reinterpret_cast<void **>(header) + 1);
    }

    std::type_info *exceptionType = __cxxabiv1::__cxa_current_exception_type();

    typelib_TypeDescription * pExcTypeDescr = nullptr;
    OUString unoName( toUNOname( exceptionType->name() ) );
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
