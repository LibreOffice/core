/*************************************************************************
 *
 *  $RCSfile: except.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:49 $
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

#define LEAK_STATIC_DATA
#define TRACE(x) OSL_TRACE(x)
//  #define TRACE(x)

#pragma warning( disable : 4237 )
#include <stl/hash_map>
#include <sal/config.h>
#include <malloc.h>
#include <new.h>
#include <typeinfo.h>
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _RTL_STRBUF_HXX_
#include <rtl/strbuf.hxx>
#endif

#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#include <bridges/cpp_uno/bridge.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include "msci.hxx"

#pragma pack(push, 8)

using namespace com::sun::star::uno;
using namespace std;
using namespace osl;
using namespace rtl;

namespace CPPU_CURRENT_NAMESPACE
{

//==================================================================================================
static inline OString toUNOname( const OString & rRTTIname )
{
    OStringBuffer aRet( 64 );
    OString aStr( rRTTIname.copy( 4, rRTTIname.getLength()-4-2 ) ); // filter .?AUzzz@yyy@xxx@@
    for ( sal_Int32 nToken = aStr.getTokenCount( '@' ); nToken--; )
    {
        aRet.append( aStr.getToken( nToken, '@' ) );
        if (nToken)
            aRet.append( '.' );
    }
    return aRet.makeStringAndClear();
}
//==================================================================================================
static inline OString toRTTIname( const OString & rUNOname )
{
    OStringBuffer aRet( 64 );
    aRet.append( RTL_CONSTASCII_STRINGPARAM(".?AV") ); // class ".?AV"; struct ".?AU"
    for ( sal_Int32 nToken = rUNOname.getTokenCount( '.' ); nToken--; )
    {
        aRet.append( rUNOname.getToken( nToken, '.' ) );
        aRet.append( '@' );
    }
    aRet.append( '@' );
    return aRet.makeStringAndClear();
}


//##################################################################################################
//#### RTTI simulation #############################################################################
//##################################################################################################


//==================================================================================================
struct FctOStringHash : public unary_function< const OString &, size_t >
{
    size_t operator()( const OString & rStr ) const
        { return rStr.hashCode(); }
};
typedef hash_map< OString, void *, FctOStringHash, equal_to< OString > > t_string2PtrMap;

//==================================================================================================
class RTTInfos
{
    Mutex               _aMutex;
    t_string2PtrMap     _allRTTI;

    static OString toRawName( const OString & rUNOname );
public:
    type_info * getRTTI( const OString & rUNOname );

    RTTInfos();
    ~RTTInfos();
};

//==================================================================================================
class __type_info
{
    friend type_info * RTTInfos::getRTTI( const OString & );
    friend sal_Int32 msci_filterCppException( LPEXCEPTION_POINTERS, uno_Any *, uno_Mapping * );

public:
    virtual ~__type_info();

    __type_info( void * m_data, const char * m_d_name )
        : _m_data( m_data )
    { ::strcpy( _m_d_name, m_d_name ); }

private:
    void * _m_data;
    char _m_d_name[1];
};
//__________________________________________________________________________________________________
__type_info::~__type_info()
{
}
//__________________________________________________________________________________________________
type_info * RTTInfos::getRTTI( const OString & rUNOname )
{
    // a must be
    OSL_ENSHURE( sizeof(__type_info) == sizeof(type_info), "### type info structure size differ!" );

    MutexGuard aGuard( _aMutex );
    const t_string2PtrMap::const_iterator iFind( _allRTTI.find( rUNOname ) );

    // check if type is already available
    if (iFind == _allRTTI.end())
    {
        // insert new type_info
        OString aRawName( toRTTIname( rUNOname ) );
        __type_info * pRTTI = new(rtl_allocateMemory( sizeof(__type_info) + aRawName.getLength() ))
                              __type_info( NULL, aRawName.getStr() );
        // put into map
        _allRTTI[rUNOname] = pRTTI;

        return (type_info *)pRTTI;
    }
    else
    {
        return (type_info *)(*iFind).second;
    }
}
//__________________________________________________________________________________________________
RTTInfos::RTTInfos()
{
}
//__________________________________________________________________________________________________
RTTInfos::~RTTInfos()
{
    TRACE( "> freeing generated RTTI infos... <\n" );

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRTTI.begin() );
          iPos != _allRTTI.end(); ++iPos )
    {
        __type_info * pType = (__type_info *)(*iPos).second;
        pType->~__type_info(); // obsolete, but good style...
        rtl_freeMemory( pType );
    }
}


//##################################################################################################
//#### Exception raising ###########################################################################
//##################################################################################################


//==================================================================================================
struct ObjectFunction
{
    char somecode[12];
    typelib_TypeDescription * _pTypeDescr; // type of object

    ObjectFunction( typelib_TypeDescription * pTypeDescr, void * fpFunc );
    ~ObjectFunction();
};
//__________________________________________________________________________________________________
ObjectFunction::ObjectFunction( typelib_TypeDescription * pTypeDescr, void * fpFunc )
    : _pTypeDescr( pTypeDescr )
{
    typelib_typedescription_acquire( _pTypeDescr );

    unsigned char * pCode = (unsigned char *)somecode;
    // a must be!
    OSL_ENSHURE( (void *)this == (void *)pCode, "### unexpected!" );

    // push ObjectFunction this
    *pCode++ = 0x68;
    *(void **)pCode = this;
    pCode += sizeof(void *);
    // jmp rel32 fpFunc
    *pCode++ = 0xe9;
    *(sal_Int32 *)pCode = ((unsigned char *)fpFunc) - pCode - sizeof(sal_Int32);
}
//__________________________________________________________________________________________________
ObjectFunction::~ObjectFunction()
{
    typelib_typedescription_release( _pTypeDescr );
}

//==================================================================================================
static void * __cdecl __copyConstruct( void * pExcThis, void * pSource, ObjectFunction * pThis )
{
    uno_copyData( pExcThis, pSource, pThis->_pTypeDescr, cpp_acquire );
    return pExcThis;
}
//==================================================================================================
static void * __cdecl __destruct( void * pExcThis, ObjectFunction * pThis )
{
    uno_destructData( pExcThis, pThis->_pTypeDescr, cpp_release );
    return pExcThis;
}

// these are non virtual object methods; there is no this ptr on stack => ecx supplies _this_ ptr

//==================================================================================================
static __declspec(naked) copyConstruct()
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
//==================================================================================================
static __declspec(naked) destruct()
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

//==================================================================================================
struct ExceptionType
{
    sal_Int32           _n0;
    type_info *         _pTypeInfo;
    sal_Int32           _n1, _n2, _n3, _n4;
    ObjectFunction *    _pCopyCtor;
    sal_Int32           _n5;

    ExceptionType( typelib_TypeDescription * pTypeDescr )
        : _n0( 0 )
        , _n1( 0 )
        , _n2( -1 )
        , _n3( 0 )
        , _n4( pTypeDescr->nSize )
        , _pCopyCtor( new ObjectFunction( pTypeDescr, copyConstruct ) )
        , _n5( 0 )
        { _pTypeInfo = msci_getRTTI( OUStringToOString( pTypeDescr->pTypeName, RTL_TEXTENCODING_ASCII_US ) ); }
    ~ExceptionType()
        { delete _pCopyCtor; }
};
//==================================================================================================
struct RaiseInfo
{
    sal_Int32           _n0;
    ObjectFunction *    _pDtor;
    sal_Int32           _n2;
    void *              _types;
    sal_Int32           _n3, _n4;

    RaiseInfo( typelib_TypeDescription * pTypeDescr );
    ~RaiseInfo();
};
//__________________________________________________________________________________________________
RaiseInfo::RaiseInfo( typelib_TypeDescription * pTypeDescr )
    : _n0( 0 )
    , _pDtor( new ObjectFunction( pTypeDescr, destruct ) )
    , _n2( 0 )
    , _n3( 0 )
    , _n4( 0 )
{
    // a must be
    OSL_ENSHURE( sizeof(sal_Int32) == sizeof(ExceptionType *), "### pointer size differs from sal_Int32!" );

    typelib_CompoundTypeDescription * pCompTypeDescr;

    // info count
    sal_Int32 nLen = 0;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        ++nLen;
    }

    // info count accompanied by type info ptrs: type, base type, base base type, ...
    _types = rtl_allocateMemory( sizeof(sal_Int32) + (sizeof(ExceptionType *) * nLen) );
    *(sal_Int32 *)_types = nLen;

    ExceptionType ** ppTypes = (ExceptionType **)((sal_Int32 *)_types + 1);

    sal_Int32 nPos = 0;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        ppTypes[nPos++] = new ExceptionType( (typelib_TypeDescription *)pCompTypeDescr );
    }
}
//__________________________________________________________________________________________________
RaiseInfo::~RaiseInfo()
{
    ExceptionType ** ppTypes = (ExceptionType **)((sal_Int32 *)_types + 1);
    for ( sal_Int32 nTypes = *(sal_Int32 *)_types; nTypes--; )
        delete ppTypes[nTypes];
    rtl_freeMemory( _types );

    delete _pDtor;
}

//==================================================================================================
class ExceptionInfos
{
    Mutex           _aMutex;
    t_string2PtrMap _allRaiseInfos;
public:
    void            raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp );

    ExceptionInfos();
    ~ExceptionInfos();
};
//__________________________________________________________________________________________________
ExceptionInfos::ExceptionInfos()
{
}
//__________________________________________________________________________________________________
ExceptionInfos::~ExceptionInfos()
{
    TRACE( "> freeing exception infos... <\n" );

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRaiseInfos.begin() );
          iPos != _allRaiseInfos.end(); ++iPos )
    {
        delete (RaiseInfo *)(*iPos).second;
    }
}
//__________________________________________________________________________________________________
void ExceptionInfos::raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = 0;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );

    OSL_ENSHURE( pTypeDescr && (pTypeDescr->eTypeClass == typelib_TypeClass_STRUCT ||
                                pTypeDescr->eTypeClass == typelib_TypeClass_EXCEPTION),
                 "### can only throw types of class exception/ structs" );

    void * pCppExc = alloca( pTypeDescr->nSize );
    uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // concatenate all types of exception for key
    OUStringBuffer aKeyBuf;
    typelib_CompoundTypeDescription * pCompTypeDescr;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription *)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        aKeyBuf.append( ((typelib_TypeDescription *)pCompTypeDescr)->pTypeName );
    }
    OString aKey( OUStringToOString( aKeyBuf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US ) );

    MutexGuard aGuard( _aMutex );
    const t_string2PtrMap::const_iterator iFind( _allRaiseInfos.find( aKey ) );

    // a must be
    OSL_ENSHURE( sizeof(sal_Int32) == sizeof(void *), "### pointer size differs from sal_Int32!" );
    DWORD arFilterArgs[3];
    arFilterArgs[0] = 0x19930520L;
    arFilterArgs[1] = (DWORD)pCppExc;
    arFilterArgs[2] = (DWORD)(iFind != _allRaiseInfos.end()
                              ? (*iFind).second // reuse existing info
                              : _allRaiseInfos[aKey] = new RaiseInfo( pTypeDescr )); // put into map

    // this is the last chance to release anything not affected by stack unwinding:
    // destruct uno exception
    uno_any_destruct( pUnoExc, 0 );

    TYPELIB_DANGER_RELEASE( pTypeDescr );

    RaiseException( MSVC_ExceptionCode, EXCEPTION_NONCONTINUABLE, 3, arFilterArgs );
}


//##################################################################################################
//#### exported ####################################################################################
//##################################################################################################


//##################################################################################################
type_info * msci_getRTTI( const OString & rUNOname )
{
    static RTTInfos * s_pRTTIs = 0;
    if (! s_pRTTIs)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pRTTIs)
        {
#ifdef LEAK_STATIC_DATA
            s_pRTTIs = new RTTInfos();
#else
            static RTTInfos s_aRTTIs;
            s_pRTTIs = &s_aRTTIs;
#endif
        }
    }
    return s_pRTTIs->getRTTI( rUNOname );
}

//##################################################################################################
void msci_raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    static ExceptionInfos * s_pInfos = 0;
    if (! s_pInfos)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pInfos)
        {
#ifdef LEAK_STATIC_DATA
            s_pInfos = new ExceptionInfos();
#else
            static ExceptionInfos s_allExceptionInfos;
            s_pInfos = &s_allExceptionInfos;
#endif
        }
    }
    s_pInfos->raiseException( pUnoExc, pUno2Cpp );
}

//##################################################################################################
sal_Int32 msci_filterCppException(
    LPEXCEPTION_POINTERS pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno )
{
    PEXCEPTION_RECORD pRecord = pPointers->ExceptionRecord;
    if (pRecord->ExceptionCode == MSVC_ExceptionCode &&
        pRecord->ExceptionFlags == EXCEPTION_NONCONTINUABLE &&
        pRecord->NumberParameters == 3 &&
//          pRecord->ExceptionInformation[0] == 0x19930520 &&
        pRecord->ExceptionInformation[1] &&
        pRecord->ExceptionInformation[2])
    {
        void * types = ((RaiseInfo *)pRecord->ExceptionInformation[2])->_types;
        if (types && *(sal_Int32 *)types) // count
        {
            ExceptionType * pType = *(ExceptionType **)((sal_Int32 *)types +1);
            if (pType && pType->_pTypeInfo)
            {
                OUString aUNOname( OStringToOUString( toUNOname(
                    ((__type_info *)pType->_pTypeInfo)->_m_d_name ), RTL_TEXTENCODING_ASCII_US ) );
                typelib_TypeDescription * pExcTypeDescr = 0;
                typelib_typedescription_getByName( &pExcTypeDescr, aUNOname.pData );

                if (pExcTypeDescr)
                {
                    // construct uno exception any
                    uno_any_constructAndConvert( pUnoExc, (void *)pRecord->ExceptionInformation[1],
                                                 pExcTypeDescr, pCpp2Uno );
                    uno_destructData( (void *)pRecord->ExceptionInformation[1],
                                      pExcTypeDescr, cpp_release );
                    typelib_typedescription_release( pExcTypeDescr );
                    return EXCEPTION_EXECUTE_HANDLER;
                }
            }
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

}

#pragma pack(pop)

