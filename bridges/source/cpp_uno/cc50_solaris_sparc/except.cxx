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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include <cstddef>
#include <dlfcn.h>
#include <new.h>
#include <typeinfo>
#include <list>
#include <map>
#include <rtl/alloc.h>
#include <osl/diagnose.h>
#include <typelib/typedescription.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"

#include "cc50_solaris_sparc.hxx"
#include "flushcode.hxx"
#include <rtl/strbuf.hxx>

#include "bridges/cpp_uno/shared/arraypointer.hxx"

#include <hash.cxx>

// need a += operator for OString and sal_Char
using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;

{
    inline OString& operator+=( OString& rString, sal_Char cAdd )
    {
        sal_Char add[2];
        add[0] = cAdd;
        add[1] = 0;
        return rString += add;
    }
}

using namespace std;
using namespace osl;
using namespace com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE
{

//==================================================================================================
static OString toUNOname( const OString & rRTTIname )
{
    OString aRet;

    const sal_Char* pRTTI = rRTTIname.getStr();
    const sal_Char* pOrg  = pRTTI;
    const sal_Char* pLast = pRTTI;

    while( 1 )
    {
        if( *pRTTI == ':' || ! *pRTTI )
        {
            if( aRet.getLength() )
                aRet += ".";
            aRet += rRTTIname.copy( pLast - pOrg, pRTTI - pLast );
            while( *pRTTI == ':' )
                pRTTI++;
            pLast = pRTTI;
            if( ! *pRTTI )
                break;
        }
        else
            pRTTI++;
    }

    return aRet;
}
//==================================================================================================
static OString toRTTIname( const OString & rUNOname )
{
    OStringBuffer aRet( rUNOname.getLength()*2 );

    sal_Int32 nIndex = 0;
    do
    {
        if( nIndex > 0 )
            aRet.append( "::" );
        aRet.append( rUNOname.getToken( 0, '.', nIndex ) );
    } while( nIndex != -1 );

    return aRet.makeStringAndClear();
}
//==================================================================================================

static OString toRTTImangledname( const OString & rRTTIname )
{
    if( ! rRTTIname.getLength() )
        return OString();

    OStringBuffer aRet( rRTTIname.getLength()*2 );

    aRet.append( "__1n" );
    sal_Int32 nIndex = 0;
    do
    {
        OString aToken( rRTTIname.getToken( 0, ':', nIndex ) );
        int nBytes = aToken.getLength();
        if( nBytes )
        {
            if( nBytes  > 25 )
            {
                aRet.append( (sal_Char)( nBytes/26 + 'a' ) );
                aRet.append( (sal_Char)( nBytes%26 + 'A' ) );
            }
            else
                aRet.append( (sal_Char)( nBytes + 'A' ) );
            for (sal_Int32 i = 0; i < aToken.getLength(); ++i) {
                char c = aToken[i];
                if (c == 'Q') {
                    aRet.append("QdD");
                } else {
                    aRet.append(c);
                }
            }
        }
    } while( nIndex != -1 );

    aRet.append( '_' );

    return aRet.makeStringAndClear();
}


//##################################################################################################
//#### RTTI simulation #############################################################################
//##################################################################################################

class RTTIHolder
{
    std::map< OString, void* > aAllRTTI;
public:
    ~RTTIHolder();

    void* getRTTI( const OString& rTypename );
    void* getRTTI_UnoName( const OString& rUnoTypename )
        { return getRTTI( toRTTIname( rUnoTypename ) ); }

    void* insertRTTI( const OString& rTypename );
    void* insertRTTI_UnoName( const OString& rTypename )
        { return insertRTTI( toRTTIname( rTypename ) ); }
    void* generateRTTI( typelib_CompoundTypeDescription* pCompTypeDescr );
};

RTTIHolder::~RTTIHolder()
{
    for ( std::map< OString, void* >::const_iterator iPos( aAllRTTI.begin() );
          iPos != aAllRTTI.end(); ++iPos )
    {
        delete[] static_cast< char * >(iPos->second);
    }
}

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

void* RTTIHolder::getRTTI( const OString& rTypename )
{
    std::map< OString, void* >::iterator element;

    element = aAllRTTI.find( rTypename );
    if( element != aAllRTTI.end() )
        return (*element).second;

    // create rtti structure
    element = aAllRTTI.find( rTypename );
    if( element != aAllRTTI.end() )
        return (*element).second;

    return NULL;
}

void* RTTIHolder::insertRTTI( const OString& rTypename )
{
    OString aMangledName( toRTTImangledname( rTypename ) );
    NIST_Hash aHash( aMangledName.getStr(), aMangledName.getLength() );

    std::size_t const RTTI_SIZE = 19; // 14???
    void** pRTTI = reinterpret_cast< void ** >(
        new char[RTTI_SIZE * sizeof (void *) + strlen(rTypename.getStr()) + 1]);
    pRTTI[  0 ] = reinterpret_cast< void * >(RTTI_SIZE * sizeof (void *));
    pRTTI[  1 ] = NULL;
    pRTTI[  2 ] = (void*)(7*sizeof(void*));
    pRTTI[  3 ] = (void*)aHash.getHash()[0];
    pRTTI[  4 ] = (void*)aHash.getHash()[1];
    pRTTI[  5 ] = (void*)aHash.getHash()[2];
    pRTTI[  6 ] = (void*)aHash.getHash()[3];
    pRTTI[  7 ] = NULL;
    pRTTI[  8 ] = NULL;

    pRTTI[  9 ] = pRTTI[ 3 ];
    pRTTI[ 10 ] = pRTTI[ 4 ];
    pRTTI[ 11 ] = pRTTI[ 5 ];
    pRTTI[ 12 ] = pRTTI[ 6 ];
    pRTTI[ 13 ] = (void*)0x80000000;
    strcpy(reinterpret_cast< char * >(pRTTI + RTTI_SIZE), rTypename.getStr());

    aAllRTTI[ rTypename ] = (void*)pRTTI;
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr,
             "generating base RTTI for type %s:\n"
             "   mangled: %s\n"
             "   hash: %.8x %.8x %.8x %.8x\n",
             rTypename.getStr(),
             aMangledName.getStr(),
             pRTTI[ 3 ], pRTTI[ 4 ], pRTTI[ 5 ], pRTTI[ 6 ]
             );
#endif
    return pRTTI;
}

//--------------------------------------------------------------------------------------------------

void* RTTIHolder::generateRTTI( typelib_CompoundTypeDescription * pCompTypeDescr )
{
    OString aUNOCompTypeName( OUStringToOString( pCompTypeDescr->aBase.pTypeName, RTL_TEXTENCODING_ASCII_US ) );
    OString aRTTICompTypeName( toRTTIname( aUNOCompTypeName ) );

    void* pHaveRTTI = getRTTI( aRTTICompTypeName );
    if( pHaveRTTI )
        return pHaveRTTI;

    if( ! pCompTypeDescr->pBaseTypeDescription )
        // this is a base type
        return insertRTTI( aRTTICompTypeName );

    // get base class RTTI
    void* pSuperRTTI = generateRTTI( pCompTypeDescr->pBaseTypeDescription );
    OSL_ENSURE( pSuperRTTI, "could not generate RTTI for supertype !" );

    // find out the size to allocate for RTTI
    void** pInherit = (void**)((sal_uInt32)pSuperRTTI + ((sal_uInt32*)pSuperRTTI)[2] + 8);
    int nInherit;
    for( nInherit = 1; pInherit[ nInherit*5-1 ] != (void*)0x80000000; nInherit++ )
        ;

    OString aMangledName( toRTTImangledname( aRTTICompTypeName ) );
    NIST_Hash aHash( aMangledName.getStr(), aMangledName.getLength() );

    std::size_t const rttiSize = 14 + nInherit * 5;
    void** pRTTI = reinterpret_cast< void ** >(
        new char[
            rttiSize * sizeof (void *)
            + strlen(aRTTICompTypeName.getStr()) + 1]);
    pRTTI[  0 ] = reinterpret_cast< void * >(rttiSize * sizeof (void *));
    pRTTI[  1 ] = NULL;
    pRTTI[  2 ] = (void*)(7*sizeof(void*));
    pRTTI[  3 ] = (void*)aHash.getHash()[0];
    pRTTI[  4 ] = (void*)aHash.getHash()[1];
    pRTTI[  5 ] = (void*)aHash.getHash()[2];
    pRTTI[  6 ] = (void*)aHash.getHash()[3];
    pRTTI[  7 ] = NULL;
    pRTTI[  8 ] = NULL;

    memcpy( pRTTI+9, pInherit, 4*nInherit*5 );
    pRTTI[ 8 +nInherit*5 ] = NULL;
    pRTTI[ 9 +nInherit*5 ] = pRTTI[ 3 ];
    pRTTI[ 10+nInherit*5 ] = pRTTI[ 4 ];
    pRTTI[ 11+nInherit*5 ] = pRTTI[ 5 ];
    pRTTI[ 12+nInherit*5 ] = pRTTI[ 6 ];
    pRTTI[ 13+nInherit*5 ] = (void*)0x80000000;
    strcpy(
        reinterpret_cast< char * >(pRTTI + rttiSize),
        aRTTICompTypeName.getStr());

    aAllRTTI[ aRTTICompTypeName ] = (void*)pRTTI;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr,
             "generating struct RTTI for type %s:\n"
             "   mangled: %s\n"
             "   hash: %.8x %.8x %.8X %.8x\n",
             aRTTICompTypeName.getStr(),
             aMangledName.getStr(),
             pRTTI[ 3 ], pRTTI[ 4 ], pRTTI[ 5 ], pRTTI[ 6 ]
             );
#endif

    return pRTTI;
}

//--------------------------------------------------------------------------------------------------

static void deleteException(
    void* pExc, unsigned int* thunk, typelib_TypeDescription* pType )
{
     uno_destructData(
        pExc, pType, reinterpret_cast< uno_ReleaseFunc >(cpp_release) );
     typelib_typedescription_release( pType );
    delete[] thunk;
}

//__________________________________________________________________________________________________

//##################################################################################################
//#### exported ####################################################################################
//##################################################################################################

void cc50_solaris_sparc_raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
#if OSL_DEBUG_LEVEL > 1
    OString cstr(
        OUStringToOString(
            *reinterpret_cast< OUString const * >( &pUnoExc->pType->pTypeName ),
            RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> uno exception occurred: %s\n", cstr.getStr() );
#endif
    bridges::cpp_uno::shared::ArrayPointer< unsigned int > thunkPtr(
        new unsigned int[6]);

    typelib_TypeDescription * pTypeDescr = 0;
    // will be released by deleteException
    typelib_typedescriptionreference_getDescription( &pTypeDescr, pUnoExc->pType );

    void* pRTTI;
    {
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > guard( aMutex );

    static RTTIHolder * s_pRTTI = 0;
    if (! s_pRTTI)
    {
#ifdef LEAK_STATIC_DATA
        s_pRTTI = new RTTIHolder();
#else
        static RTTIHolder s_aRTTI;
        s_pRTTI = &s_aRTTI;
#endif
    }

    pRTTI = s_pRTTI->generateRTTI( (typelib_CompoundTypeDescription *)pTypeDescr );
    }

    // a must be
    OSL_ENSURE( sizeof(sal_Int32) == sizeof(void *), "### pointer size differs from sal_Int32!" );

    void * pCppExc = __Crun::ex_alloc( pTypeDescr->nSize );
    uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // destruct uno exception
    uno_any_destruct( pUnoExc, 0 );

    unsigned int * thunk = thunkPtr.release();
    // sethi %hi(thunk), %o1:
    thunk[0] = 0x13000000 | (reinterpret_cast< unsigned int >(thunk) >> 10);
    // or %o1, %lo(thunk), %o1:
    thunk[1] = 0x92126000 | (reinterpret_cast< unsigned int >(thunk) & 0x3FF);
    // sethi %hi(pTypeDescr), %o2:
    thunk[2] = 0x15000000
        | (reinterpret_cast< unsigned int >(pTypeDescr) >> 10);
    // sethi %hi(deleteException), %o3
    thunk[3] = 0x17000000
        | (reinterpret_cast< unsigned int >(deleteException) >> 10);
    // jmpl %o3, %lo(deleteException), %g0
    thunk[4] = 0x81C2E000
        | (reinterpret_cast< unsigned int >(deleteException) & 0x3FF);
    // or %o2, %lo(pTypeDescr), %o2:
    thunk[5] = 0x9412A000
        | (reinterpret_cast< unsigned int >(pTypeDescr) & 0x3FF);
    bridges::cpp_uno::cc50_solaris_sparc::flushCode(thunk, thunk + 6);

#pragma disable_warn
    void (* f)(void *) = reinterpret_cast< void (*)(void *) >(thunk);
#pragma enable_warn
    __Crun::ex_throw(pCppExc, (const __Crun::static_type_info*)pRTTI, f);
}

void cc50_solaris_sparc_fillUnoException(
    void* pCppExc,
    const char* pInfo,
    uno_Any* pUnoExc,
    uno_Mapping * pCpp2Uno )
{
    OSL_ASSERT( pInfo != 0 );
    OString uno_name( toUNOname( pInfo ) );
    OUString aName( OStringToOUString(
                        uno_name, RTL_TEXTENCODING_ASCII_US ) );
    typelib_TypeDescription * pExcTypeDescr = 0;
    typelib_typedescription_getByName( &pExcTypeDescr, aName.pData );

    if (pExcTypeDescr == 0) // the thing that should not be
    {
        RuntimeException aRE(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "exception type not found: ") ) + aName,
            Reference< XInterface >() );
        Type const & rType = ::getCppuType( &aRE );
        uno_type_any_constructAndConvert(
            pUnoExc, &aRE, rType.getTypeLibType(), pCpp2Uno );
#if OSL_DEBUG_LEVEL > 0
        OString cstr( OUStringToOString(
                          aRE.Message, RTL_TEXTENCODING_ASCII_US ) );
        OSL_FAIL( cstr.getStr() );
#endif
        return;
    }

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "> c++ exception occurred: %s\n",
             ::rtl::OUStringToOString(
                 pExcTypeDescr->pTypeName,
                 RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
    // construct uno exception any
    uno_any_constructAndConvert(
        pUnoExc, pCppExc, pExcTypeDescr, pCpp2Uno );
    typelib_typedescription_release( pExcTypeDescr );
}

}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
