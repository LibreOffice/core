/*************************************************************************
 *
 *  $RCSfile: except.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2000-11-22 20:05:04 $
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

#include <dlfcn.h>
#include <new.h>
#include <typeinfo>
#include <list>
#include <map>
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#include <bridges/cpp_uno/bridge.hxx>
#endif
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include "cc50_solaris_sparc.hxx"

// need a += operator for OString and sal_Char
namespace rtl
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
using namespace rtl;
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
    OString aRet;

    int nTokens = rUNOname.getTokenCount( '.' );
    for( int i = 0; i < nTokens; i++ )
    {
        if( i > 0 )
            aRet += "::";
        aRet += rUNOname.getToken( i, '.' );
    }

    return aRet;
}
//==================================================================================================

static int replaceQdDdD( const OString& rIn, OString& rOutdD, OString& rOutdDdD )
{
    int nRet = 0;
    int nLen = rIn.getLength(), i, n;
    rOutdD      = OString();
    rOutdDdD    = OString();
    for( i = 0, n = 0; ( i = rIn.indexOf( 'Q', i ) ) != -1 && i < nLen; i++ )
    {
        rOutdD   += rIn.copy( n, i-n+1 );
        rOutdDdD += rIn.copy( n, i-n+1 );
        n = i+1;
        rOutdD += "dD";
        rOutdDdD += "dDdD";
        nRet++;
    }
    rOutdD += rIn.copy( n );
    rOutdDdD += rIn.copy( n );
    return nRet;
}

static OString toRTTIsymbolname( const OString & rRTTIname )
{
    if( ! rRTTIname.getLength() )
        return OString();

    OString aRet;
    OString aPrefix;

    int nUnoTokens = rRTTIname.getTokenCount( ':' );
    int nAdjust = 0;
    for( int i = 0; i < nUnoTokens; i++ )
    {
        OString aToken( rRTTIname.getToken( i, ':' ) );
        int nBytes = aToken.getLength();
        if( nBytes )
        {
            OString aAdd;
            if( nBytes > 25 )
            {
                aAdd += (sal_Char)( nBytes/26 + 'a' );
                aAdd += (sal_Char)( nBytes % 26 + 'A' );
            }
            else
                aAdd += (sal_Char)( nBytes + 'A' );
            aRet += aAdd;
            // special case "Q"
            if( ( ( nBytes % 26 ) +'A' ) == 'Q' )
            {
                aRet += "dD";
                nAdjust += 2;
            }

            OString adD, adDdD;
            int nRepl = replaceQdDdD( aToken, adD, adDdD );
            if( nUnoTokens == 1 )
            {
                // must replace "Q" by "QdD"
                aRet += adD;
                nAdjust += 2* nRepl;
            }
            else
            {
                // must replace "Q" by "QdDdD"
                aRet += adDdD;
                nAdjust += 4* nRepl;
            }

            if( i < nUnoTokens - 1  )
            {
                aPrefix += aAdd;
                // must replace "Q" by "QdD"
                aPrefix += adD;
            }
        }
    }
    aRet += '_';

    if( aPrefix.getLength() )
    {
        int nBytes = aRet.getLength() - nAdjust + 10;
        if( nBytes > 25 )
        {
            aPrefix += (sal_Char)( nBytes/26 + 'a' );
                aPrefix += (sal_Char)( nBytes % 26 + 'A' );
        }
        else
            aPrefix += (sal_Char)( nBytes + 'A' );


        aRet = "__1c" + aPrefix + "__RTTI__1n" + aRet + "_";
    }
    else
        aRet = "__RTTI__1n"+aRet;

    return aRet;
}


//##################################################################################################
//#### RTTI simulation #############################################################################
//##################################################################################################

class RTTIHolder
{
    static std::map< OString, void* > aAllRTTI;
public:
    static void* getRTTI( const OString& rTypename );
    static void* getRTTI_UnoName( const OString& rUnoTypename )
        { return getRTTI( toRTTIname( rUnoTypename ) ); }

    static void* insertRTTI( const OString& rTypename );
    static void* insertRTTI_UnoName( const OString& rTypename )
        { return insertRTTI( toRTTIname( rTypename ) ); }
};

std::map< OString, void* > RTTIHolder::aAllRTTI;

#ifdef DEBUG
#include <stdio.h>
#endif

void* RTTIHolder::getRTTI( const OString& rTypename )
{
    std::map< OString, void* >::iterator element;

    element = aAllRTTI.find( rTypename );
    if( element != aAllRTTI.end() )
        return (*element).second;
    // get rtti
    // look for existing type info
    static void* pMain = dlopen( NULL, RTLD_NOW | RTLD_GLOBAL );

    void* pSymbol = dlsym( pMain, toRTTIsymbolname( rTypename ).getStr() );

    if( pSymbol )
    {
        // there exists type info, use it (otherwise it will not be equal
        // since addresses - not contents - are matched in RTTI compare)
        aAllRTTI[ rTypename ] = pSymbol;
        return pSymbol;
    }

#ifdef DEBUG
    fprintf( stderr, "exception type \"%s\" not found via dlsym !!!\n", toRTTIsymbolname( rTypename ).getStr() );
#endif

    // no type found (time for plan b: look for base type )
    return NULL;
}

static long nMagicId = 1;

void* RTTIHolder::insertRTTI( const OString& rTypename )
{
#ifdef DEBUG
    fprintf( stderr, "generating base RTTI for type %s\n", rTypename.getStr() );
#endif
    void** pRTTI = new void*[ 19 ];
    memset( pRTTI, 0, 19*sizeof( void* ) );
    pRTTI[  0 ] = (void*)strdup( rTypename.getStr() );
    pRTTI[  2 ] = (void*)(7*sizeof(void*));
    pRTTI[  3 ] = (void*)nMagicId++;
    pRTTI[  4 ] = (void*)nMagicId++;
    pRTTI[  5 ] = (void*)nMagicId++;
    pRTTI[  6 ] = (void*)nMagicId++;

    pRTTI[  9 ] = pRTTI[ 3 ];
    pRTTI[ 10 ] = pRTTI[ 4 ];
    pRTTI[ 11 ] = pRTTI[ 5 ];
    pRTTI[ 12 ] = pRTTI[ 6 ];
    pRTTI[ 13 ] = (void*)0x80000000;

    aAllRTTI[ rTypename ] = (void*)pRTTI;
    return pRTTI;
}

//--------------------------------------------------------------------------------------------------

static void* generateRTTI( typelib_CompoundTypeDescription * pCompTypeDescr )
{
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > guard( aMutex );

    OString aCompTypeName( OUStringToOString( pCompTypeDescr->aBase.pTypeName, RTL_TEXTENCODING_ASCII_US ) );

    void* pRTTI = RTTIHolder::getRTTI_UnoName( aCompTypeName );
    if( pRTTI )
        return pRTTI;

    // uh oh. Compiler did not generate RTTI for this type
    // but that means that this was neither thrown nor caught explicitly
    // then let's downcast it until we find real RTTI
    // since the object will be destructed by deleteException which does
    // this with the correct type description all should be well

    if( ! pCompTypeDescr->pBaseTypeDescription )
        // this is a base type (and a bad case, we should not need to generate any RTTI)
        return RTTIHolder::insertRTTI_UnoName( aCompTypeName );

    // get base class RTTI
    void* pSuperRTTI = generateRTTI( pCompTypeDescr->pBaseTypeDescription );
    OSL_ENSHURE( pSuperRTTI, "could not generate RTTI for supertype!" );

    return pSuperRTTI;
}

//--------------------------------------------------------------------------------------------------

static void deleteException( void* pExc )
{
     typelib_TypeDescription* pType = (typelib_TypeDescription*)((void**)pExc)[-2];
     uno_destructData( pExc, pType, cpp_release );
     typelib_typedescription_release( pType );
}

//__________________________________________________________________________________________________

//##################################################################################################
//#### exported ####################################################################################
//##################################################################################################

void cc50_solaris_sparc_raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    typelib_TypeDescription * pTypeDescr = 0;
    // will be released by deleteException
    typelib_typedescriptionreference_getDescription( &pTypeDescr, pUnoExc->pType );

    void* pRTTI = generateRTTI( (typelib_CompoundTypeDescription *)pTypeDescr );

    // a must be
    OSL_ENSHURE( sizeof(sal_Int32) == sizeof(void *), "### pointer size differs from sal_Int32!" );

    void** pExcSpace = (void**)__Crun::ex_alloc( pTypeDescr->nSize + 8 );
    void * pCppExc = (void*)(((char*)pExcSpace)+8);
    // will be released in generated dtor
    // alignment to 8
    pExcSpace[0] = pTypeDescr;
    uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // destruct uno exception
    uno_any_destruct( pUnoExc, 0 );

    __Crun::ex_throw( pCppExc, (const __Crun::static_type_info*)pRTTI, deleteException );
}

void cc50_solaris_sparc_fillUnoException(
    void* pCppExc,
    const char* pInfo,
    uno_Any* pExc,
    uno_Mapping * pCpp2Uno )
{
    OUString aName( OStringToOUString( toUNOname( pInfo ), RTL_TEXTENCODING_ASCII_US ) );
    typelib_TypeDescription * pExcTypeDescr = 0;
    typelib_typedescription_getByName(
        &pExcTypeDescr,
        aName.pData );
    if (pExcTypeDescr)
    {
        // construct cpp exception any
        Any aAny( pCppExc, pExcTypeDescr ); // const_cast
        typelib_typedescription_release( pExcTypeDescr );
        // construct uno exception any
        typelib_TypeDescription* pAnyDescr = 0;
        getCppuType( (const Any *)0 ).getDescription( &pAnyDescr );
        uno_copyAndConvertData( pExc, &aAny, pAnyDescr, pCpp2Uno );
        typelib_typedescription_release( pAnyDescr );
    }
}

}

