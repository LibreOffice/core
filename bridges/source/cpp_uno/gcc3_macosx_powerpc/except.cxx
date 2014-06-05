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
#include <dlfcn.h>
#include <cxxabi.h>
#include <boost/unordered_map.hpp>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <com/sun/star/uno/genfunc.hxx>
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

void dummy_can_throw_anything( char const * )
{
}

static OUString toUNOname( char const * p )
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

class RTTI
{
    typedef boost::unordered_map< OUString, type_info *, OUStringHash > t_rtti_map;

    Mutex m_mutex;
    t_rtti_map m_rttis;
    t_rtti_map m_generatedRttis;

    void * m_hApp;

public:
    RTTI();
    ~RTTI();

    type_info * getRTTI( typelib_CompoundTypeDescription * );
};

RTTI::RTTI()
    : m_hApp( dlopen( 0, RTLD_LAZY ) )
{
}

RTTI::~RTTI()
{
    dlclose( m_hApp );
}


type_info * RTTI::getRTTI( typelib_CompoundTypeDescription *pTypeDescr )
{
    type_info * rtti;

    OUString const & unoName = *(OUString const *)&pTypeDescr->aBase.pTypeName;

    MutexGuard guard( m_mutex );
    t_rtti_map::const_iterator iRttiFind( m_rttis.find( unoName ) );
    if (iRttiFind == m_rttis.end())
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
        rtti = (type_info *)dlsym( m_hApp, symName.getStr() );

        if (rtti)
        {
            pair< t_rtti_map::iterator, bool > insertion(
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            OSL_ENSURE( insertion.second, "### inserting new rtti failed?!" );
        }
        else
        {
            // try to lookup the symbol in the generated rtti map
            t_rtti_map::const_iterator iFind( m_generatedRttis.find( unoName ) );
            if (iFind == m_generatedRttis.end())
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
                    rtti = new __si_class_type_info(
                        strdup( rttiName ), (__class_type_info *)base_rtti );
                }
                else
                {
                    // this class has no base class
                    rtti = new __class_type_info( strdup( rttiName ) );
                }

                pair< t_rtti_map::iterator, bool > insertion(
                    m_generatedRttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
                OSL_ENSURE( insertion.second, "### inserting new generated rtti failed?!" );
            }
            else // taking already generated rtti
            {
                rtti = iFind->second;
            }
        }
    }
    else
    {
        rtti = iRttiFind->second;
    }

    return rtti;
}


static void deleteException( void * pExc )
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

void raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    void * pCppExc;
    type_info * rtti;

    {
    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = 0;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );
    OSL_ASSERT( pTypeDescr );
    if (! pTypeDescr)
        terminate();

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
    rtti = (type_info *)s_rtti->getRTTI( (typelib_CompoundTypeDescription *) pTypeDescr );
    TYPELIB_DANGER_RELEASE( pTypeDescr );
    OSL_ENSURE( rtti, "### no rtti for throwing exception!" );
    if (! rtti)
        terminate();
    }

    __cxa_throw( pCppExc, rtti, deleteException );
}

void fillUnoException( __cxa_exception * header, uno_Any * pExc, uno_Mapping * pCpp2Uno )
{
    OSL_ENSURE( header, "### no exception header!!!" );
    if (! header)
        terminate();

    typelib_TypeDescription * pExcTypeDescr = 0;
    OUString unoName( toUNOname( header->exceptionType->name() ) );
    ::typelib_typedescription_getByName( &pExcTypeDescr, unoName.pData );
    OSL_ENSURE( pExcTypeDescr, "### can not get type description for exception!!!" );
    if (! pExcTypeDescr)
        terminate();

    // construct uno exception any
    ::uno_any_constructAndConvert( pExc, header->adjustedPtr, pExcTypeDescr, pCpp2Uno );
    ::typelib_typedescription_release( pExcTypeDescr );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
