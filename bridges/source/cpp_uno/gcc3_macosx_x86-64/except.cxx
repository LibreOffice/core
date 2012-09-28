/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <cxxabi.h>
#ifndef _GLIBCXX_CDTOR_CALLABI // new in GCC 4.7 cxxabi.h
#define _GLIBCXX_CDTOR_CALLABI
#endif

#include <boost/unordered_map.hpp>

#include <sal/log.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <typelib/typedescription.hxx>
#include <uno/any2.h>

#include "share.hxx"


using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::__cxxabiv1;


namespace CPPU_CURRENT_NAMESPACE
{

#if MACOSX_SDK_VERSION >= 1070

// MacOSX10.4u.sdk/usr/include/c++/4.0.0/cxxabi.h defined
// __cxxabiv1::__class_type_info and __cxxabiv1::__si_class_type_info but
// MacOSX10.7.sdk/usr/include/cxxabi.h no longer does, so instances of those
// classes need to be created manually:

// std::type_info defined in <typeinfo> offers a protected ctor:
struct FAKE_type_info: public std::type_info {
    FAKE_type_info(char const * name): type_info(name) {}
};

// Modeled after __cxxabiv1::__si_class_type_info defined in
// MacOSX10.4u.sdk/usr/include/c++/4.0.0/cxxabi.h (i.e.,
// abi::__si_class_type_info documented at
// <http://www.codesourcery.com/public/cxx-abi/abi.html#rtti>):
struct FAKE_si_class_type_info: public FAKE_type_info {
    FAKE_si_class_type_info(char const * name, std::type_info const * theBase):
        FAKE_type_info(name), base(theBase) {}

    std::type_info const * base;
        // actually a __cxxabiv1::__class_type_info pointer
};

struct Base {};
struct Derived: Base {};

std::type_info * create_FAKE_class_type_info(char const * name) {
    std::type_info * p = new FAKE_type_info(name);
        // cxxabiv1::__class_type_info has no data members in addition to
        // std::type_info
    *reinterpret_cast< void ** >(p) = *reinterpret_cast< void * const * >(
        &typeid(Base));
        // copy correct __cxxabiv1::__class_type_info vtable into place
    return p;
}

std::type_info * create_FAKE_si_class_type_info(
    char const * name, std::type_info const * base)
{
    std::type_info * p = new FAKE_si_class_type_info(name, base);
    *reinterpret_cast< void ** >(p) = *reinterpret_cast< void * const * >(
        &typeid(Derived));
        // copy correct __cxxabiv1::__si_class_type_info vtable into place
    return p;
}

#endif

//==================================================================================================
static OUString toUNOname( char const * p ) SAL_THROW(())
{
#if OSL_DEBUG_LEVEL > 1
    char const * start = p;
#endif

    // example: N3com3sun4star4lang24IllegalArgumentExceptionE

    OUStringBuffer buf( 64 );
    OSL_ASSERT( 'N' == *p );
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
            buf.append( (sal_Unicode)'.' );
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

//==================================================================================================
class RTTI
{
    typedef boost::unordered_map< OUString, type_info *, OUStringHash > t_rtti_map;

    Mutex m_mutex;
    t_rtti_map m_rttis;
    t_rtti_map m_generatedRttis;

    void * m_hApp;

public:
    RTTI() SAL_THROW(());
    ~RTTI() SAL_THROW(());

    type_info * getRTTI( typelib_CompoundTypeDescription * ) SAL_THROW(());
};
//__________________________________________________________________________________________________
RTTI::RTTI() SAL_THROW(())
#if defined(FREEBSD) && __FreeBSD_version < 702104
    : m_hApp( dlopen( 0, RTLD_NOW | RTLD_GLOBAL ) )
#else
    : m_hApp( dlopen( 0, RTLD_LAZY ) )
#endif
{
}
//__________________________________________________________________________________________________
RTTI::~RTTI() SAL_THROW(())
{
    dlclose( m_hApp );
}

//__________________________________________________________________________________________________
type_info * RTTI::getRTTI( typelib_CompoundTypeDescription *pTypeDescr ) SAL_THROW(())
{
    type_info * rtti;

    OUString const & unoName = *(OUString const *)&pTypeDescr->aBase.pTypeName;

    MutexGuard guard( m_mutex );
    t_rtti_map::const_iterator iFind( m_rttis.find( unoName ) );
    if (iFind == m_rttis.end())
    {
        // RTTI symbol
        OStringBuffer buf( 64 );
        buf.append( RTL_CONSTASCII_STRINGPARAM("_ZTIN") );
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
#if defined(FREEBSD) && __FreeBSD_version < 702104 /* #i22253# */
        rtti = (type_info *)dlsym( RTLD_DEFAULT, symName.getStr() );
#else
        rtti = (type_info *)dlsym( m_hApp, symName.getStr() );
#endif

        if (rtti)
        {
            pair< t_rtti_map::iterator, bool > insertion (
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            SAL_WARN_IF( !insertion.second, "bridges", "key " << unoName << " already in rtti map" );
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
                char const * rttiName = symName.getStr() +4;
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr,"generated rtti for %s\n", rttiName );
#endif
                if (pTypeDescr->pBaseTypeDescription)
                {
                    // ensure availability of base
                    type_info * base_rtti = getRTTI(
                        (typelib_CompoundTypeDescription *)pTypeDescr->pBaseTypeDescription );
#if MACOSX_SDK_VERSION < 1070
                    rtti = new __si_class_type_info(
                        strdup( rttiName ), (__class_type_info *)base_rtti );
#else
                    rtti = create_FAKE_si_class_type_info(
                        strdup( rttiName ), base_rtti );
#endif
                }
                else
                {
                    // this class has no base class
#if MACOSX_SDK_VERSION < 1070
                    rtti = new __class_type_info( strdup( rttiName ) );
#else
                    rtti = create_FAKE_class_type_info( strdup( rttiName ) );
#endif
                }

                pair< t_rtti_map::iterator, bool > insertion (
                    m_generatedRttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
                SAL_WARN_IF( !insertion.second, "bridges", "key " << unoName << " already in generated rtti map" );
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

struct RTTISingleton: public rtl::Static< RTTI, RTTISingleton > {};

//--------------------------------------------------------------------------------------------------
extern "C" {
static void _GLIBCXX_CDTOR_CALLABI deleteException( void * pExc )
{
    __cxa_exception const * header = ((__cxa_exception const *)pExc - 1);
    typelib_TypeDescription * pTD = 0;
    OUString unoName( toUNOname( header->exceptionType->name() ) );
    ::typelib_typedescription_getByName( &pTD, unoName.pData );
    OSL_ENSURE( pTD, "### unknown exception type! leaving out destruction => leaking!!!" );
    if (pTD)
    {
        ::uno_destructData( pExc, pTD, cpp_release );
        ::typelib_typedescription_release( pTD );
    }
}
}

//==================================================================================================
void raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
#if OSL_DEBUG_LEVEL > 1
    OString cstr(
        OUStringToOString(
            *reinterpret_cast< OUString const * >( &pUnoExc->pType->pTypeName ),
            RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> uno exception occurred: %s\n", cstr.getStr() );
#endif
    void * pCppExc;
    type_info * rtti;

    {
    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = 0;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );
    OSL_ASSERT( pTypeDescr );
    if (! pTypeDescr)
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("cannot get typedescription for type ") ) +
            *reinterpret_cast< OUString const * >( &pUnoExc->pType->pTypeName ),
            Reference< XInterface >() );
    }

    pCppExc = __cxa_allocate_exception( pTypeDescr->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // destruct uno exception
    ::uno_any_destruct( pUnoExc, 0 );
    // avoiding locked counts
    rtti = (type_info *)RTTISingleton::get().getRTTI( (typelib_CompoundTypeDescription *) pTypeDescr );
    TYPELIB_DANGER_RELEASE( pTypeDescr );
    OSL_ENSURE( rtti, "### no rtti for throwing exception!" );
    if (! rtti)
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no rtti for type ") ) +
            *reinterpret_cast< OUString const * >( &pUnoExc->pType->pTypeName ),
            Reference< XInterface >() );
    }
    }

    __cxa_throw( pCppExc, rtti, deleteException );
}

//==================================================================================================
void fillUnoException( __cxa_exception * header, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno )
{
    if (! header)
    {
        RuntimeException aRE(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no exception header!") ),
            Reference< XInterface >() );
        Type const & rType = ::getCppuType( &aRE );
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
#if OSL_DEBUG_LEVEL > 0
        OString cstr( OUStringToOString( aRE.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr.getStr() );
#endif
        return;
    }

    typelib_TypeDescription * pExcTypeDescr = 0;
    OUString unoName( toUNOname( header->exceptionType->name() ) );
#if OSL_DEBUG_LEVEL > 1
    OString cstr_unoName( OUStringToOString( unoName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> c++ exception occurred: %s\n", cstr_unoName.getStr() );
#endif
    typelib_typedescription_getByName( &pExcTypeDescr, unoName.pData );
    if (0 == pExcTypeDescr)
    {
        RuntimeException aRE(
            OUString( RTL_CONSTASCII_USTRINGPARAM("exception type not found: ") ) + unoName,
            Reference< XInterface >() );
        Type const & rType = ::getCppuType( &aRE );
        uno_type_any_constructAndConvert( pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
#if OSL_DEBUG_LEVEL > 0
        OString cstr( OUStringToOString( aRE.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr.getStr() );
#endif
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
