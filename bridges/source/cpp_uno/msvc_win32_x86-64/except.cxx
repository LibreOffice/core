/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

















#if 0

/* This information until the corresponding '#endif 
 * by ONTL's license, which is said to be the "zlib/libgng license"
 * below, which as far as I see is permissive enough to allow this
 * information to be included here in this source file. Note that no
 * actual code from ONTL below gets compiled into the object file.
 */

/*
 * Copyright (c) 2011 <copyright holders> (The ONTL main
 * developer(s) don't tell their real name(s) on the ONTL site.)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *
 *     3. This notice may not be removed or altered from any source
 *     distribution.
 *
 */

typedef uint32_t rva_t;


typedef void generic_function_t();

struct ptrtomember 
{
  typedef __w64 int32_t  mdiff_t;
  mdiff_t member_offset;
  mdiff_t vbtable_offset; 
  mdiff_t vdisp_offset;   

  template<typename T>
  T * operator()(T * const thisptr) const
  {
    uintptr_t tp = reinterpret_cast<uintptr_t>(thisptr);
    uintptr_t ptr = tp + member_offset;
    if ( vbtable_offset != -1 ) 
    {
      ptr += *reinterpret_cast<mdiff_t*>( static_cast<intptr_t>(vdisp_offset + *reinterpret_cast<mdiff_t*>(tp + vbtable_offset)) )
        + vbtable_offset;
    }
    return reinterpret_cast<T*>(ptr);
  }
};

struct eobject
{
  typedef void (* dtor_ptr )(eobject*);
  typedef void (* ctor_ptr )(eobject*, eobject*);
  typedef void (* ctor_ptr2)(eobject*, eobject*, int);
};

struct catchabletype
{
  /** is simple type */
  uint32_t    memmoveable : 1;
  /** catchable as reference only */
  uint32_t    refonly     : 1;
  /** class with virtual base */
  uint32_t    hasvirtbase : 1;
  /** offset to the type descriptor */
  rva_t       typeinfo;

  /** catch type instance location within a thrown object  */
  ptrtomember thiscast;
  /** size of the simple type or offset into buffer of \c this pointer for catch object */
  uint32_t    object_size;

  union
  {
    rva_t               copyctor;
    rva_t               copyctor2;
  };
};

#pragma pack(push, 4)
struct catchabletypearray
{
  uint32_t        size;
  rva_t           type[1];
};
#pragma pack(pop)

#pragma pack(push, 4)
struct throwinfo
{
  typedef exception_disposition __cdecl forwardcompathandler_t(...);

  /* 0x00 */  uint32_t  econst    : 1;
  /* 0x00 */  uint32_t  evolatile : 1;
  /* 0x00 */  uint32_t            : 1;
  /* 0x00 */  uint32_t  e8        : 1;
  /* 0x04 */  rva_t     exception_dtor;
  /* 0x08 */  rva_t     forwardcompathandler;
  /* 0x0C */  rva_t     catchabletypearray; 
};
#pragma pack(pop)


struct ehandler
{
  
  uint32_t isconst      : 1;
  uint32_t isvolatile   : 1;
  uint32_t isunaligned  : 1;
  uint32_t isreference  : 1;

  uint32_t              :27;
  uint32_t ishz         : 1;

  /** offset to the type descriptor of this catch object */
  /*0x04*/ rva_t        typeinfo;         
  /*0x08*/ int          eobject_bpoffset; 
  /** offset to the catch clause funclet */
  /*0x0C*/ rva_t        handler;          
  /*0x10*/ uint32_t     frame;            
}









void
  constructcatchobject(
  cxxregistration *             cxxreg,
  const ehandler *        const catchblock,
  catchabletype *         const convertible,
  const dispatcher_context* const dispatch
  )
  const
{
  _EH_TRACE_ENTER();
  
  __try {
    struct typeinfo_t { void* vtbl; void* spare; char name[1]; };
    enum catchable_info { cidefault, cicomplex, civirtual } cinfo = cidefault;

    const typeinfo_t* ti = catchblock->typeinfo ? dispatch->va<typeinfo_t*>(catchblock->typeinfo) : NULL;
    if(ti && *ti->name && (catchblock->eobject_bpoffset || catchblock->ishz)){
      eobject** objplace = catchblock->ishz
        ? reinterpret_cast<eobject**>(cxxreg)
        : reinterpret_cast<eobject**>(catchblock->eobject_bpoffset + cxxreg->fp.FramePointers);
      if(catchblock->isreference){
        
        *objplace = adjust_pointer(get_object(), convertible);
      }else if(convertible->memmoveable){
        
        std::memcpy(objplace, get_object(), convertible->object_size);
        if(convertible->object_size == sizeof(void*) && *objplace)
          *objplace = adjust_pointer((void*)*objplace, convertible);
      }else{
        
        if(convertible->copyctor){
          cinfo = convertible->hasvirtbase ? civirtual : cicomplex;
        }else{
          std::memcpy(objplace, (const void*)adjust_pointer(get_object(), convertible), convertible->object_size);
        }
      }
    }
    
    if(cinfo != cidefault){
      eobject* objthis = catchblock->ishz
        ? reinterpret_cast<eobject*>(cxxreg)
        : reinterpret_cast<eobject*>(catchblock->eobject_bpoffset + cxxreg->fp.FramePointers);
      void* copyctor = thrown_va(convertible->copyctor);
      eobject* copyarg = adjust_pointer(get_object(), convertible);
      if(cinfo == cicomplex)
        (eobject::ctor_ptr (copyctor))(objthis, copyarg);
      else
        (eobject::ctor_ptr2(copyctor))(objthis, copyarg, 1);
    }
  }
  __except(cxxregistration::unwindfilter(static_cast<nt::ntstatus>(_exception_code())))
  {
    nt::exception::inconsistency();
  }
  _EH_TRACE_LEAVE();
}

#endif 


#pragma warning( disable : 4237 )
#include <boost/unordered_map.hpp>
#include <sal/config.h>
#include <malloc.h>
#include <new.h>
#include <typeinfo.h>
#include <signal.h>

#include "rtl/alloc.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"

#include "com/sun/star/uno/Any.hxx"

#include "mscx.hxx"

#pragma pack(push, 8)

using namespace ::com::sun::star::uno;
using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace CPPU_CURRENT_NAMESPACE
{

static inline OUString toUNOname(
    OUString const & rRTTIname )
    throw ()
{
    OUStringBuffer aRet( 64 );
    OUString aStr( rRTTIname.copy( 4, rRTTIname.getLength()-4-2 ) ); 
    sal_Int32 nPos = aStr.getLength();
    while (nPos > 0)
    {
        sal_Int32 n = aStr.lastIndexOf( '@', nPos );
        aRet.append( aStr.copy( n +1, nPos -n -1 ) );
        if (n >= 0)
        {
            aRet.append( '.' );
        }
        nPos = n;
    }
    return aRet.makeStringAndClear();
}

static inline OUString toRTTIname(
    OUString const & rUNOname )
    throw ()
{
    OUStringBuffer aRet( 64 );
    aRet.append( ".?AV" ); 
    sal_Int32 nPos = rUNOname.getLength();
    while (nPos > 0)
    {
        sal_Int32 n = rUNOname.lastIndexOf( '.', nPos );
        aRet.append( rUNOname.copy( n +1, nPos -n -1 ) );
        aRet.append( '@' );
        nPos = n;
    }
    aRet.append( '@' );
    return aRet.makeStringAndClear();
}



typedef boost::unordered_map< OUString, void *, OUStringHash, equal_to< OUString > > t_string2PtrMap;

class RTTInfos
{
    Mutex               _aMutex;
    t_string2PtrMap     _allRTTI;

    static OUString toRawName( OUString const & rUNOname ) throw ();
public:
    type_info * getRTTI( OUString const & rUNOname ) throw ();

    RTTInfos();
    ~RTTInfos();
};

class __type_info
{
    friend type_info * RTTInfos::getRTTI( OUString const & ) throw ();
    friend int mscx_filterCppException(
        LPEXCEPTION_POINTERS, uno_Any *, uno_Mapping * );

public:
    virtual ~__type_info() throw ();

    inline __type_info( void * m_data, const char * m_d_name ) throw ()
        : _m_data( m_data )
        { ::strcpy( _m_d_name, m_d_name ); } 

private:
    void * _m_data;
    char _m_d_name[1];
};

__type_info::~__type_info() throw ()
{
}

type_info * RTTInfos::getRTTI( OUString const & rUNOname ) throw ()
{
    
    OSL_ENSURE( sizeof(__type_info) == sizeof(type_info), "### type info structure size differ!" );

    MutexGuard aGuard( _aMutex );
    t_string2PtrMap::const_iterator const iFind( _allRTTI.find( rUNOname ) );

    
    if (iFind == _allRTTI.end())
    {
        
        OString aRawName( OUStringToOString( toRTTIname( rUNOname ), RTL_TEXTENCODING_ASCII_US ) );
        __type_info * pRTTI = new( ::rtl_allocateMemory( sizeof(__type_info) + aRawName.getLength() ) )
            __type_info( NULL, aRawName.getStr() );

        
        pair< t_string2PtrMap::iterator, bool > insertion(
            _allRTTI.insert( t_string2PtrMap::value_type( rUNOname, pRTTI ) ) );
        OSL_ENSURE( insertion.second, "### rtti insertion failed?!" );

        return (type_info *)pRTTI;
    }
    else
    {
        return (type_info *)iFind->second;
    }
}

RTTInfos::RTTInfos() throw ()
{
}

RTTInfos::~RTTInfos() throw ()
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "> freeing generated RTTI infos... <" );
#endif

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRTTI.begin() );
          iPos != _allRTTI.end(); ++iPos )
    {
        __type_info * pType = (__type_info *)iPos->second;
        pType->~__type_info(); 
        ::rtl_freeMemory( pType );
    }
}

void * __cdecl copyConstruct(
    void * pExcThis,
    void * pSource,
    typelib_TypeDescription * pTD ) throw ()
{
    ::uno_copyData( pExcThis, pSource, pTD, cpp_acquire );
    return pExcThis;
}

void * __cdecl destruct(
    void * pExcThis,
    typelib_TypeDescription * pTD ) throw ()
{
    ::uno_destructData( pExcThis, pTD, cpp_release );
    return pExcThis;
}

const int codeSnippetSize = 40;

void GenerateConstructorTrampoline(
    unsigned char * code,
    typelib_TypeDescription * pTD ) throw ()
{
    unsigned char *p = code;

    
    *p++ = 0x49; *p++ = 0xB8;
    *((void **)p) = pTD; p += 8;

    
    *p++ = 0x49; *p++ = 0xBB;
    *((void **)p) = &copyConstruct; p += 8;

    
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    OSL_ASSERT( p < code + codeSnippetSize );
}

void GenerateDestructorTrampoline(
    unsigned char * code,
    typelib_TypeDescription * pTD ) throw ()
{
    unsigned char *p = code;

    
    *p++ = 0x48; *p++ = 0xBA;
    *((void **)p) = pTD; p += 8;

    
    *p++ = 0x49; *p++ = 0xBB;
    *((void **)p) = &destruct; p += 8;

    
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    OSL_ASSERT( p < code + codeSnippetSize );
}



struct ExceptionType
{
    sal_Int32   _n0;            
    sal_uInt32  _pTypeInfo;     
    sal_Int32   _n1, _n2, _n3;  
    sal_Int32   _n4;            
    sal_uInt32  _pCopyCtor;     

    inline ExceptionType(
        unsigned char * pCode,
        sal_uInt64 pCodeBase,
        typelib_TypeDescription * pTD ) throw ()
        : _n0( 0 )
        , _n1( 0 )
        , _n2( -1 )
        , _n3( 0 )
        , _n4( pTD->nSize )
        {
            
            
            
            _pTypeInfo = (sal_uInt32) ((sal_uInt64) mscx_getRTTI( pTD->pTypeName ) - pCodeBase);
            GenerateConstructorTrampoline( pCode, pTD );
            _pCopyCtor = (sal_uInt32) ((sal_uInt64) pCode - pCodeBase);
        }
    inline ~ExceptionType() throw ()
        {
        }
};

struct RaiseInfo;

class ExceptionInfos
{
    Mutex           _aMutex;
    t_string2PtrMap _allRaiseInfos;

public:
    static RaiseInfo * getRaiseInfo( typelib_TypeDescription * pTD ) throw ();

    static DWORD allocationGranularity;

    ExceptionInfos() throw ();
    ~ExceptionInfos() throw ();
};

DWORD ExceptionInfos::allocationGranularity = 0;



struct RaiseInfo
{
    sal_Int32           _n0;
    sal_uInt32          _pDtor;
    sal_Int32           _n2;
    sal_uInt32          _types;

    
    typelib_TypeDescription * _pTD;
    unsigned char *        _code;
    sal_uInt64         _codeBase;

    RaiseInfo( typelib_TypeDescription * pTD ) throw ();

    ~RaiseInfo() throw ();
};

RaiseInfo::RaiseInfo( typelib_TypeDescription * pTD )throw ()
    : _n0( 0 )
    , _n2( 0 )
    , _pTD( pTD )
{
    typelib_CompoundTypeDescription * pCompTD;

    
    int codeSize = codeSnippetSize;

    
    int nLen = 0;
    for ( pCompTD = (typelib_CompoundTypeDescription*)pTD;
          pCompTD; pCompTD = pCompTD->pBaseTypeDescription )
    {
        ++nLen;
        codeSize += codeSnippetSize;
    }

    unsigned char * pCode = _code = (unsigned char *)::rtl_allocateMemory( codeSize );

    _codeBase = (sal_uInt64)pCode & ~(ExceptionInfos::allocationGranularity-1);

    DWORD old_protect;
#if OSL_DEBUG_LEVEL > 0
    BOOL success =
#endif
        VirtualProtect( pCode, codeSize, PAGE_EXECUTE_READWRITE, &old_protect );
        OSL_ENSURE( success, "VirtualProtect() failed!" );

    ::typelib_typedescription_acquire( pTD );

    GenerateDestructorTrampoline( pCode, pTD );
    _pDtor = (sal_Int32)((sal_uInt64)pCode - _codeBase);
    pCode += codeSnippetSize;

    
    _types = (sal_Int32)((sal_uInt64)::rtl_allocateMemory( 4 + 4* nLen) - _codeBase);
    *(sal_Int32 *)_types = nLen;

    ExceptionType ** ppTypes = (ExceptionType **)((sal_Int32 *)_types + 1);

    int nPos = 0;
    for ( pCompTD = (typelib_CompoundTypeDescription*)pTD;
          pCompTD; pCompTD = pCompTD->pBaseTypeDescription )
    {
        ppTypes[nPos++] =
            new ExceptionType( pCode, _codeBase,
                               (typelib_TypeDescription *)pCompTD );
        pCode += codeSnippetSize;
    }
}

RaiseInfo::~RaiseInfo() throw ()
{
    sal_uInt32 * pTypes =
        (sal_uInt32 *)(_codeBase + _types) + 1;

    for ( int nTypes = *(sal_uInt32 *)(_codeBase + _types); nTypes--; )
    {
        delete (ExceptionType *) (_codeBase + pTypes[nTypes]);
    }
    ::rtl_freeMemory( (void*)(_codeBase +_types) );
    ::rtl_freeMemory( _code );

    ::typelib_typedescription_release( _pTD );
}

ExceptionInfos::ExceptionInfos() throw ()
{
}

ExceptionInfos::~ExceptionInfos() throw ()
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "> freeing exception infos... <" );
#endif

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRaiseInfos.begin() );
          iPos != _allRaiseInfos.end(); ++iPos )
    {
        delete (RaiseInfo *)iPos->second;
    }
}

RaiseInfo * ExceptionInfos::getRaiseInfo( typelib_TypeDescription * pTD ) throw ()
{
    static ExceptionInfos * s_pInfos = 0;
    if (! s_pInfos)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pInfos)
        {
            SYSTEM_INFO systemInfo;
            GetSystemInfo( &systemInfo );
            allocationGranularity = systemInfo.dwAllocationGranularity;

#ifdef LEAK_STATIC_DATA
            s_pInfos = new ExceptionInfos();
#else
            static ExceptionInfos s_allExceptionInfos;
            s_pInfos = &s_allExceptionInfos;
#endif
        }
    }

    OSL_ASSERT( pTD &&
                (pTD->eTypeClass == typelib_TypeClass_STRUCT ||
                 pTD->eTypeClass == typelib_TypeClass_EXCEPTION) );

    RaiseInfo * pRaiseInfo;

    OUString const & rTypeName = *reinterpret_cast< OUString * >( &pTD->pTypeName );
    MutexGuard aGuard( s_pInfos->_aMutex );
    t_string2PtrMap::const_iterator const iFind(
        s_pInfos->_allRaiseInfos.find( rTypeName ) );
    if (iFind == s_pInfos->_allRaiseInfos.end())
    {
        pRaiseInfo = new RaiseInfo( pTD );

        
        pair< t_string2PtrMap::iterator, bool > insertion(
            s_pInfos->_allRaiseInfos.insert( t_string2PtrMap::value_type( rTypeName, (void *)pRaiseInfo ) ) );
        OSL_ENSURE( insertion.second, "### raise info insertion failed?!" );
    }
    else
    {
        
        pRaiseInfo = (RaiseInfo *)iFind->second;
    }

    return pRaiseInfo;
}

type_info * mscx_getRTTI(
    OUString const & rUNOname )
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

void mscx_raiseException(
    uno_Any * pUnoExc,
    uno_Mapping * pUno2Cpp )
{
    
    
    

    
    typelib_TypeDescription * pTD = NULL;
    TYPELIB_DANGER_GET( &pTD, pUnoExc->pType );

    void * pCppExc = alloca( pTD->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTD, pUno2Cpp );

    ULONG_PTR arFilterArgs[4];
    arFilterArgs[0] = MSVC_magic_number;
    arFilterArgs[1] = (ULONG_PTR)pCppExc;
    arFilterArgs[2] = (ULONG_PTR)ExceptionInfos::getRaiseInfo( pTD );
    arFilterArgs[3] = ((RaiseInfo *)arFilterArgs[2])->_codeBase;

    
    ::uno_any_destruct( pUnoExc, 0 );
    TYPELIB_DANGER_RELEASE( pTD );

    
    RaiseException( MSVC_ExceptionCode, EXCEPTION_NONCONTINUABLE, 3, arFilterArgs );
}

int mscx_filterCppException(
    EXCEPTION_POINTERS * pPointers,
    uno_Any * pUnoExc,
    uno_Mapping * pCpp2Uno )
{
    if (pPointers == 0)
        return EXCEPTION_CONTINUE_SEARCH;

    EXCEPTION_RECORD * pRecord = pPointers->ExceptionRecord;

    
    if (pRecord == 0 || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    bool rethrow = __CxxDetectRethrow( &pRecord );
    OSL_ASSERT( pRecord == pPointers->ExceptionRecord );

    if (rethrow && pRecord == pPointers->ExceptionRecord)
    {
        
        pRecord = *reinterpret_cast< EXCEPTION_RECORD ** >(
            reinterpret_cast< char * >( __pxcptinfoptrs() ) +
            
            
            //
            
            
            
#if _MSC_VER < 1600
            0x48 
#else
            error, please find value for this compiler version
#endif
            );
    }

    
    if (pRecord == 0 || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    if (pRecord->NumberParameters == 4 &&
        pRecord->ExceptionInformation[0] == MSVC_magic_number &&
        pRecord->ExceptionInformation[1] != 0 &&
        pRecord->ExceptionInformation[2] != 0 &&
        pRecord->ExceptionInformation[3] != 0)
    {
        
        
        

        

        
        

        void * types =
            (void *) (pRecord->ExceptionInformation[3] +
                      ((RaiseInfo *)pRecord->ExceptionInformation[2])->_types);

        if (types != 0 && *(DWORD *)types > 0)
        {
            DWORD pType = *((DWORD *)types + 1);
            if (pType != 0 &&
                ((ExceptionType *)(pRecord->ExceptionInformation[3]+pType))->_pTypeInfo != 0)
            {
                OUString aRTTIname(
                    OStringToOUString(
                        reinterpret_cast< __type_info * >(
                            ((ExceptionType *)(pRecord->ExceptionInformation[3]+pType))->_pTypeInfo )->_m_d_name,
                        RTL_TEXTENCODING_ASCII_US ) );
                OUString aUNOname( toUNOname( aRTTIname ) );

                typelib_TypeDescription * pExcTD = 0;
                typelib_typedescription_getByName(
                    &pExcTD, aUNOname.pData );
                if (pExcTD == NULL)
                {
                    OUStringBuffer buf;
                    buf.append(
                            "[mscx_uno bridge error] UNO type of "
                            "C++ exception unknown: \"" );
                    buf.append( aUNOname );
                    buf.append( "\", RTTI-name=\"" );
                    buf.append( aRTTIname );
                    buf.append( "\"!" );
                    RuntimeException exc(
                        buf.makeStringAndClear(), Reference< XInterface >() );
                    uno_type_any_constructAndConvert(
                        pUnoExc, &exc,
                        ::getCppuType( &exc ).getTypeLibType(), pCpp2Uno );
                }
                else
                {
                    
                    uno_any_constructAndConvert(
                        pUnoExc, (void *) pRecord->ExceptionInformation[1],
                        pExcTD, pCpp2Uno );
                    typelib_typedescription_release( pExcTD );
                }

                return EXCEPTION_EXECUTE_HANDLER;
            }
        }
    }
    
    
    RuntimeException exc(
        OUString( "[mscx_uno bridge error] unexpected "
                  "C++ exception occurred!" ),
        Reference< XInterface >() );
    uno_type_any_constructAndConvert(
        pUnoExc, &exc, ::getCppuType( &exc ).getTypeLibType(), pCpp2Uno );
    return EXCEPTION_EXECUTE_HANDLER;
}

}

#pragma pack(pop)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
