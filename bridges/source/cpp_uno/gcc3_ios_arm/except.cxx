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

#include <typeinfo>

#include <dlfcn.h>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <typelib/typedescription.hxx>
#include <uno/any2.h>
#include <unordered_map>
#include "share.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE {

namespace {

struct Fake_type_info {
    virtual ~Fake_type_info() {}
    char const * name;
};

static_assert(
    sizeof (Fake_type_info) == sizeof (std::type_info), "must be the same size");

struct Fake_class_type_info: Fake_type_info {};

struct Fake_si_class_type_info: Fake_class_type_info {
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
#ifdef _LIBCPP_NONUNIQUE_RTTI_BIT
    *(uintptr_t*)(&fake->name) |= _LIBCPP_NONUNIQUE_RTTI_BIT;
#endif
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
#ifdef _LIBCPP_NONUNIQUE_RTTI_BIT
    *(uintptr_t*)(&fake->name) |= _LIBCPP_NONUNIQUE_RTTI_BIT;
#endif
    fake->base = base;
    return reinterpret_cast<std::type_info *>(
        static_cast<Fake_type_info *>(fake));
}

}

void dummy_can_throw_anything( char const * )
{
}

static OUString toUNOname( char const * p )
{
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

    OUString result( buf.makeStringAndClear() );

    return result;
}

class RTTI
{
    typedef std::unordered_map< OUString, std::type_info *, OUStringHash > t_rtti_map;

    Mutex m_mutex;
    t_rtti_map m_rttis;
    t_rtti_map m_generatedRttis;

    void * m_hApp;

public:
    RTTI();
    ~RTTI();

    std::type_info * getRTTI( typelib_CompoundTypeDescription * );
};

RTTI::RTTI()
    : m_hApp( dlopen( 0, RTLD_LAZY ) )
{
    // Insert commonly needed type_infos to avoid dlsym() calls
    // Ideally we should insert all needed ones, and we actually must,
    // for arm64, as the dynamically generated type_infos don't seem
    // to work correctly. Luckily it seems that quite few types of
    // exceptions are thrown through the C++/UNO bridge at least in
    // the TiledLibreOffice test app.

    // (As no Java, Basic or Python is supported in LO code on iOS, we
    // can know the set of types of exceptions throws a priori, so
    // keeping this list complete should be possible.)

    m_rttis.insert( t_rtti_map::value_type( "com.sun.star.ucb.InteractiveAugmentedIOException",
                                            (std::type_info*) &typeid( com::sun::star::ucb::InteractiveAugmentedIOException ) ) );
    m_rttis.insert( t_rtti_map::value_type( "com.sun.star.ucb.NameClashException",
                                            (std::type_info*) &typeid( com::sun::star::ucb::NameClashException ) ) );
}

RTTI::~RTTI()
{
    dlclose( m_hApp );
}

std::type_info * RTTI::getRTTI( typelib_CompoundTypeDescription *pTypeDescr )
{
    std::type_info * rtti;

    OUString const & unoName = *(OUString const *)&pTypeDescr->aBase.pTypeName;

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
        rtti = (std::type_info *)dlsym( m_hApp, symName.getStr() );

        if (rtti)
        {
            SAL_INFO( "bridges.ios", "getRTTI: dlsym() found type_info for " << unoName << ": " << symName );
            std::pair< t_rtti_map::iterator, bool > insertion(
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            SAL_WARN_IF( !insertion.second,
                         "bridges",
                         "inserting new rtti failed" );
        }
        else
        {
            SAL_INFO( "bridges.ios", "getRTTI: dlsym() could NOT find type_info for " << unoName << ": " << symName );

            // try to lookup the symbol in the generated rtti map
            t_rtti_map::const_iterator iFind2( m_generatedRttis.find( unoName ) );
            if (iFind2 == m_generatedRttis.end())
            {
                // we must generate it !
                // symbol and rtti-name is nearly identical,
                // the symbol is prefixed with _ZTI
                char * rttiName = strdup(symName.getStr() + 4);
                if (rttiName == 0) {
                    throw std::bad_alloc();
                }

                SAL_INFO( "bridges.ios", "getRTTI: generating typeinfo " << rttiName );

                if (pTypeDescr->pBaseTypeDescription)
                {
                    // ensure availability of base
                    std::type_info * base_rtti = getRTTI(
                        (typelib_CompoundTypeDescription *)pTypeDescr->pBaseTypeDescription );
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
    __cxa_exception const * header = ((__cxa_exception const *)pExc - 1);
    typelib_TypeDescription * pTD = 0;
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
    SAL_INFO( "bridges.ios", "raiseException: " << OUString::unacquired( &pUnoExc->pType->pTypeName ) );

    void * pCppExc;
    std::type_info * rtti;

    {
    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = 0;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );
    assert(pTypeDescr);
    if (! pTypeDescr)
    {
        throw RuntimeException(
            OUString("cannot get typedescription for type ") +
            OUString::unacquired( &pUnoExc->pType->pTypeName ) );
    }

    pCppExc = __cxa_allocate_exception( pTypeDescr->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // destruct uno exception
    ::uno_any_destruct( pUnoExc, 0 );
    // avoiding locked counts
    static RTTI * s_rtti = 0;
    if (! s_rtti)
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if (! s_rtti)
        {
#ifdef LEAK_STATIC_DATA
            s_rtti = new RTTI();
#else
            static RTTI rtti_data;
            s_rtti = &rtti_data;
#endif
        }
    }
    rtti = s_rtti->getRTTI( (typelib_CompoundTypeDescription *) pTypeDescr );
    TYPELIB_DANGER_RELEASE( pTypeDescr );
    assert( rtti );
    if (! rtti)
    {
        throw RuntimeException(
            OUString("no rtti for type ") +
            OUString::unacquired( &pUnoExc->pType->pTypeName ) );
    }
    }

    __cxa_throw( pCppExc, rtti, deleteException );
}

void fillUnoException( __cxa_exception * header, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno )
{
    if (! header)
    {
        RuntimeException aRE( "no exception header!" );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges.ios", aRE.Message);
        return;
    }

    typelib_TypeDescription * pExcTypeDescr = 0;
    OUString unoName( toUNOname( header->exceptionType->name() ) );

    SAL_INFO( "bridges.ios", "fillUnoException: " << unoName );

    typelib_typedescription_getByName( &pExcTypeDescr, unoName.pData );
    if (0 == pExcTypeDescr)
    {
        RuntimeException aRE( OUString("exception type not found: ") + unoName );
        Type const & rType = cppu::UnoType<decltype(aRE)>::get();
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
        SAL_WARN("bridges.ios", aRE.Message);
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
