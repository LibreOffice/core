/*************************************************************************
 *
 *  $RCSfile: except.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-10-19 13:32:39 $
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

#include <stdio.h>
#include <dlfcn.h>
#include <hash_map>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <bridges/cpp_uno/bridge.hxx>
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

//==================================================================================================
static OUString toUNOname( char const * p ) SAL_THROW( () )
{
char const * start = p;
//      N3com3sun4star4lang24IllegalArgumentExceptionE

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

#ifdef DEBUG
    OUString ret( buf.makeStringAndClear() );
    OString c_ret( OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> toUNOname(): %s => %s\n", start, c_ret.getStr() );
    return ret;
#else
    return buf.makeStringAndClear();
#endif
}
//==================================================================================================
static OString toRTTIname( OUString const & unoName ) SAL_THROW( () )
{
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

#ifdef DEBUG
    OString ret( buf.makeStringAndClear() );
    OString str( OUStringToOString( unoName, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, "> toRTTIname(): %s => %s\n", str.getStr(), ret.getStr() );
    return ret;
#else
    return buf.makeStringAndClear();
#endif
}

//==================================================================================================
class RTTI
{
    typedef hash_map< OUString, type_info *, OUStringHash > t_rtti_map;

    Mutex m_mutex;
    t_rtti_map m_rttis;

public:
    type_info * getRTTI( OUString const & unoName ) SAL_THROW( () );
};
//__________________________________________________________________________________________________
type_info * RTTI::getRTTI( OUString const & unoName ) SAL_THROW( () )
{
    static void * s_hApp = dlopen( 0, RTLD_LAZY );
    type_info * rtti;

    MutexGuard guard( m_mutex );
    t_rtti_map::const_iterator iFind( m_rttis.find( unoName ) );
    if (iFind == m_rttis.end())
    {
        OString rttiName( toRTTIname( unoName ) );
        rtti = (type_info *)dlsym( s_hApp, rttiName.getStr() );
        if (rtti)
        {
            pair< t_rtti_map::iterator, bool > insertion(
                m_rttis.insert( t_rtti_map::value_type( unoName, rtti ) ) );
            OSL_ENSURE( insertion.second, "### inserting new rtti failed?!" );
        }
    }
    else
    {
        rtti = iFind->second;
    }

    return rtti;
}

//--------------------------------------------------------------------------------------------------
static void deleteException( void* pExc )
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

//==================================================================================================
void raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    void * pCppExc;
    type_info * rtti;

    fprintf( stderr, ">1\n" );
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
    OUString const & unoName = *(OUString const *)&pTypeDescr->pTypeName;
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
    rtti = (type_info *)s_rtti->getRTTI( unoName );
    TYPELIB_DANGER_RELEASE( pTypeDescr );
    OSL_ENSURE( rtti, "### no rtti for throwing exception!" );
    if (! rtti)
        terminate();
    }

    __cxa_throw( pCppExc, rtti, deleteException );
}

//==================================================================================================
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

