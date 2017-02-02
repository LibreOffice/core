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

// Interesting info can be found in:

// MSDN, obviously

// http://www.osronline.com/article.cfm?article=469

// ONTL, "Open NT Native Template Library", a C++ STL-like library
// that can be used even when writing Windows drivers. This is some
// quite badass code. The author has done impressive heavy spelunking
// of MSVCR structures. http://code.google.com/p/ontl/

// Geoff Chappell's pages:
// http://www.geoffchappell.com/studies/msvc/language/index.htm

// The below is from ONTL's ntl/nt/exception.hxx, cleaned up to keep just the _M_X64 parts:

#if 0

/* This information until the corresponding '#endif // 0' is covered
 * by ONTL's license, which is said to be the "zlib/libpng license"
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

///\note the calling convention should not matter since this has no arguments
typedef void generic_function_t();

struct ptrtomember // _PMD
{
  typedef __w64 int32_t  mdiff_t;
  mdiff_t member_offset;
  mdiff_t vbtable_offset; // -1 if not a virtual base
  mdiff_t vdisp_offset;   // offset to the displacement value inside the vbtable

  template<typename T>
  T * operator()(T * const thisptr) const
  {
    uintptr_t tp = reinterpret_cast<uintptr_t>(thisptr);
    uintptr_t ptr = tp + member_offset;
    if ( vbtable_offset != -1 ) // !(vbtable_offset < 0)
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
  /* 0x0C */  rva_t     catchabletypearray; ///< types able to catch the exception.
};
#pragma pack(pop)

/// This type represents the catch clause
struct ehandler
{
  //  union { uint32_t  adjectives; void * ptr; };
  uint32_t isconst      : 1;
  uint32_t isvolatile   : 1;
  uint32_t isunaligned  : 1;// guess it is not used on x86
  uint32_t isreference  : 1;

  uint32_t              :27;
  uint32_t ishz         : 1;

  /** offset to the type descriptor of this catch object */
  /*0x04*/ rva_t        typeinfo;         // dispType
  /*0x08*/ int          eobject_bpoffset; // dispCatchObj
  /** offset to the catch clause funclet */
  /*0x0C*/ rva_t        handler;          // dispOfHandler
  /*0x10*/ uint32_t     frame;            // dispFrame
}

// ___BuildCatchObject
/// 15.3/16 When the exception-declaration specifies a class type, a copy
///         constructor is used to initialize either the object declared
///         in the exception-declaration or,
///         if the exception-declaration does not specify a name,
///         a temporary object of that type.
///\note    This is the question may we optimize out the last case.
///\warning If the copy constructor throws an exception, std::unexpected would be called
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
  // build helper
  __try {
    struct typeinfo_t { void* vtbl; void* spare; char name[1]; };
    enum catchable_info { cidefault, cicomplex, civirtual } cinfo = cidefault;

    const typeinfo_t* ti = catchblock->typeinfo ? dispatch->va<typeinfo_t*>(catchblock->typeinfo) : NULL;
    if(ti && *ti->name && (catchblock->eobject_bpoffset || catchblock->ishz)){
      eobject** objplace = catchblock->ishz
        ? reinterpret_cast<eobject**>(cxxreg)
        : reinterpret_cast<eobject**>(catchblock->eobject_bpoffset + cxxreg->fp.FramePointers);
      if(catchblock->isreference){
        // just ref/pointer
        *objplace = adjust_pointer(get_object(), convertible);
      }else if(convertible->memmoveable){
        // POD
        std::memcpy(objplace, get_object(), convertible->object_size);
        if(convertible->object_size == sizeof(void*) && *objplace)
          *objplace = adjust_pointer((void*)*objplace, convertible);
      }else{
        // if copy ctor exists, call it; binary copy otherwise
        if(convertible->copyctor){
          cinfo = convertible->hasvirtbase ? civirtual : cicomplex;
        }else{
          std::memcpy(objplace, (const void*)adjust_pointer(get_object(), convertible), convertible->object_size);
        }
      }
    }
    // end of build helper
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

#endif // 0


#pragma warning( disable : 4237 )
#include <sal/config.h>

#include <memory>

#include <malloc.h>
#include <new.h>
#include <typeinfo.h>
#include <signal.h>

#include "rtl/alloc.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"
#include <sal/log.hxx>
#include <osl/mutex.hxx>

#include "com/sun/star/uno/Any.hxx"
#include <unordered_map>
#include "mscx.hxx"
#include "except.hxx"

#pragma pack(push, 8)

using namespace ::com::sun::star::uno;
using namespace ::std;
using namespace ::osl;

namespace CPPU_CURRENT_NAMESPACE
{
    int mscx_getRTTI_len(OUString const & rUNOname);


static inline OUString toUNOname(
    OUString const & rRTTIname )
    throw ()
{
    OUStringBuffer aRet( 64 );
    OUString aStr( rRTTIname.copy( 4, rRTTIname.getLength()-4-2 ) ); // filter .?AUzzz@yyy@xxx@@
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
    aRet.append( ".?AV" ); // class ".?AV"; struct ".?AU"
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

//RTTI simulation

typedef std::unordered_map< OUString, void *, OUStringHash > t_string2PtrMap;
class type_info_descriptor;

class RTTInfos
{
    Mutex               _aMutex;
    t_string2PtrMap     _allRTTI;

public:
    type_info * getRTTI( OUString const & rUNOname ) throw ();
    int getRTTI_len(OUString const & rUNOname) throw ();
    type_info_descriptor * insert_new_type_info_descriptor(OUString const & rUNOname);

    RTTInfos() throw ();
#if !defined LEAK_STATIC_DATA
    ~RTTInfos() throw ();
#endif
};
class type_info_
{
    friend type_info * RTTInfos::getRTTI( OUString const & ) throw ();
    friend int mscx_filterCppException(
        LPEXCEPTION_POINTERS, uno_Any *, uno_Mapping * );

public:
    virtual ~type_info_() throw ();

    inline type_info_( void * m_data, const char * m_d_name ) throw ()
        : _m_data( m_data )
        { ::strcpy( _m_d_name, m_d_name ); } // #100211# - checked

private:
    void * _m_data;
    char _m_d_name[1];
};

type_info_::~type_info_() throw ()
{
    (void)_m_data;
}

class type_info_descriptor
{
private:
    int type_info_size;
    type_info_ info;

public:

    inline type_info_descriptor(void * m_data, const char * m_d_name) throw ()
        : info(m_data, m_d_name)
    {
        type_info_size = sizeof(type_info_) + strlen(m_d_name);
    }

    type_info * get_type_info()
    {
        return reinterpret_cast<type_info *>(&info);
    }
    int get_type_info_size()
    {
        return type_info_size;
    }
};

type_info_descriptor * RTTInfos::insert_new_type_info_descriptor(OUString const & rUNOname) {

    // insert new type_info
    OString aRawName(OUStringToOString(toRTTIname(rUNOname), RTL_TEXTENCODING_ASCII_US));
    type_info_descriptor * pRTTI = new(::rtl_allocateMemory(sizeof(type_info_descriptor) + aRawName.getLength()))
        type_info_descriptor(nullptr, aRawName.getStr());

    // put into map
    pair< t_string2PtrMap::iterator, bool > insertion(
        _allRTTI.insert(t_string2PtrMap::value_type(rUNOname, pRTTI)));
    assert(insertion.second && "### rtti insertion failed?!");

    return pRTTI;
}
type_info * RTTInfos::getRTTI( OUString const & rUNOname ) throw ()
{
    // a must be
    static_assert(sizeof(type_info_) == sizeof(type_info), "### type info structure size differ!");

    MutexGuard aGuard( _aMutex );
    t_string2PtrMap::const_iterator const iFind( _allRTTI.find( rUNOname ) );

    // check if type is already available
    if (iFind == _allRTTI.end())
    {
        // Wrap new type_info_ in type_info_descriptor to preserve length info
        type_info_descriptor * pRTTI = insert_new_type_info_descriptor(rUNOname);
        return pRTTI->get_type_info();
    }
    else
    {
        return static_cast<type_info_descriptor *>(iFind->second)->get_type_info();
    }
}

int RTTInfos::getRTTI_len(OUString const & rUNOname) throw ()
{
    MutexGuard aGuard(_aMutex);
    t_string2PtrMap::const_iterator const iFind(_allRTTI.find(rUNOname));

    // Wrap new type_info_ in type_info_descriptor to preserve length info
    // check if type is already available
    if (iFind == _allRTTI.end())
    {
        // Wrap new type_info_ in type_info_descriptor to preserve length info
        type_info_descriptor * pRTTI = insert_new_type_info_descriptor(rUNOname);
        return pRTTI->get_type_info_size();
    }
    else
    {
        return static_cast<type_info_descriptor *>(iFind->second)->get_type_info_size();
    }
}

RTTInfos::RTTInfos() throw ()
{
}

#if !defined LEAK_STATIC_DATA
RTTInfos::~RTTInfos() throw ()
{
    SAL_INFO("bridges", "> freeing generated RTTI infos... <");

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRTTI.begin() );
          iPos != _allRTTI.end(); ++iPos )
    {
        type_info_ * pType = static_cast<type_info_ *>(iPos->second);
        pType->~type_info_(); // obsolete, but good style...
        ::rtl_freeMemory( pType );
    }
}
#endif

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

    // mov r8, pTD
    *p++ = 0x49; *p++ = 0xB8;
    *reinterpret_cast<void **>(p) = pTD; p += 8;

    // mov r11, copyConstruct
    *p++ = 0x49; *p++ = 0xBB;
    *reinterpret_cast<void **>(p) = reinterpret_cast<void *>(&copyConstruct); p += 8;

    // jmp r11
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    assert( p < code + codeSnippetSize );
}

void GenerateDestructorTrampoline(
    unsigned char * code,
    typelib_TypeDescription * pTD ) throw ()
{
    unsigned char *p = code;

    // mov rdx, pTD
    *p++ = 0x48; *p++ = 0xBA;
    *reinterpret_cast<void **>(p) = pTD; p += 8;

    // mov r11, destruct
    *p++ = 0x49; *p++ = 0xBB;
    *reinterpret_cast<void **>(p) = reinterpret_cast<void *>(&destruct); p += 8;

    // jmp r11
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    assert( p < code + codeSnippetSize );
}

// This looks like it is the struct catchabletype above

struct ExceptionType
{
    sal_Int32   _n0;            // flags
    sal_uInt32  _pTypeInfo;     // typeinfo
    sal_Int32   _n1, _n2, _n3;  // thiscast
    sal_Int32   _n4;            // object_size
    sal_uInt32  _pCopyCtor;     // copyctor
    type_info_   type_info;


    inline ExceptionType(
        unsigned char * pCode,
        sal_uInt64 pCodeBase,
        typelib_TypeDescription * pTD ) throw ()
        : _n0( 0 )
        , _n1( 0 )
        , _n2( -1 )
        , _n3( 0 )
        , _n4( pTD->nSize)
        , type_info(nullptr, "")
        {
            // As _n0 is always initialized to zero, that means the
            // hasvirtbase flag (see the ONTL catchabletype struct) is
            // off, and thus the copyctor is of the ctor_ptr kind.
            memcpy(static_cast<void *>(&type_info), static_cast<void *>(mscx_getRTTI(pTD->pTypeName)), mscx_getRTTI_len(pTD->pTypeName));
            _pTypeInfo = static_cast<sal_uInt32>(
            reinterpret_cast<sal_uInt64>(&type_info) - pCodeBase);
            GenerateConstructorTrampoline( pCode, pTD );
            assert(
                pCodeBase <= reinterpret_cast<sal_uInt64>(pCode)
                && (reinterpret_cast<sal_uInt64>(pCode) - pCodeBase
                    < 0x100000000));
            _pCopyCtor = static_cast<sal_uInt32>(
                reinterpret_cast<sal_uInt64>(pCode) - pCodeBase);
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
#if !defined LEAK_STATIC_DATA
    ~ExceptionInfos() throw ();
#endif
};

DWORD ExceptionInfos::allocationGranularity = 0;

// This corresponds to the struct throwinfo described above.

struct RaiseInfo
{
    sal_Int32           _n0;
    sal_uInt32          _pDtor;
    sal_Int32           _n2;
    sal_uInt32          _types;

    // Additional fields
    typelib_TypeDescription * _pTD;
    unsigned char *        _code;
    sal_uInt64         _codeBase;

    explicit RaiseInfo(typelib_TypeDescription * pTD) throw ();

#if !defined LEAK_STATIC_DATA
    ~RaiseInfo() throw ();
#endif
};

/* Rewrite of 32-Bit-Code to work under 64 Bit:
* To use the 32 Bit offset values in the ExceptionType we have to
* allocate a single allocation block and use it for all code and date
* all offsets inside this area are guaranteed to be in 32 bit address range.
* So we have to calc total memory allocation size for D-tor, C-Tors,
* ExceptionType and type_info. ExceptionType is allocated via placement new
* to locate everything inside our mem block.
* There is one caveat: Struct type_info is kept in
* a map and was referenced from class ExceptionType. Therefore type_info now
* is also member of ExceptionType and can be referenced via 32 bit offset.
*/

RaiseInfo::RaiseInfo(typelib_TypeDescription * pTD)throw ()
    : _n0(0)
    , _n2(0)
    , _pTD(pTD)
{
    typelib_CompoundTypeDescription * pCompTD;

    // Count how many trampolines we need
    int codeSize = codeSnippetSize;
    // Info count
    int nLen = 0;
    for (pCompTD = reinterpret_cast<typelib_CompoundTypeDescription*>(pTD);
        pCompTD; pCompTD = pCompTD->pBaseTypeDescription)
    {
        ++nLen;
        codeSize += codeSnippetSize;
    }

    // Array with size (4) and all _pTypeInfo (4*nLen)
    int typeInfoArraySize = 4 + 4 * nLen;

    // 2.Pass: Get the total needed memory for class ExceptionType
    // (with embedded type_info) and keep the sizes for each instance
    // is stored in allocated int array
    auto exceptionTypeSizeArray = std::unique_ptr<int[]>(new int[nLen]);

    nLen = 0;
    for (pCompTD = reinterpret_cast<typelib_CompoundTypeDescription*>(pTD);
        pCompTD; pCompTD = pCompTD->pBaseTypeDescription)
    {
        int typeInfoLen = mscx_getRTTI_len(pCompTD->aBase.pTypeName);
        // Mem has to be on 4-byte Boundary
        if (typeInfoLen % 4 != 0)
        {
            int n = typeInfoLen / 4;
            n++;
            typeInfoLen = n*4;
        }
        exceptionTypeSizeArray[nLen++] = typeInfoLen + sizeof(ExceptionType);
    }

    // Total ExceptionType related mem
    int excTypeAddLen = 0;
    for (int i = 0; i < nLen; i++)
    {
        excTypeAddLen += exceptionTypeSizeArray[i];
    }

    // Allocate mem for code and all dynamic data in one chunk to guarantee
    // 32 bit offsets
    const int totalSize = codeSize + typeInfoArraySize + excTypeAddLen;
    unsigned char * pCode = _code =
        static_cast<unsigned char *>(::rtl_allocateMemory(totalSize));
    int pCodeOffset = 0;

    // New base of types array, starts after Trampoline D-Tor / C-Tors
    DWORD * types = reinterpret_cast<DWORD *>(pCode + codeSize);

    // New base of ExceptionType array, starts after types array
    unsigned char *etMem = pCode + codeSize + typeInfoArraySize;
    int etMemOffset = 0;

    _codeBase = reinterpret_cast<sal_uInt64>(pCode)
        & ~static_cast<sal_uInt64>(ExceptionInfos::allocationGranularity - 1);

    DWORD old_protect;
    BOOL success =
        VirtualProtect(pCode, codeSize, PAGE_EXECUTE_READWRITE, &old_protect);
    (void) success;
    assert(success && "VirtualProtect() failed!");

    ::typelib_typedescription_acquire(pTD);

    // Fill pCode with D-Tor code
    GenerateDestructorTrampoline(pCode, pTD);
    _pDtor = (sal_Int32)(reinterpret_cast<sal_uInt64>(pCode) - _codeBase);
    pCodeOffset += codeSnippetSize;

    // Info count accompanied by type info ptrs: type, base type, base base type, ...
    // Keep offset of types_array
    _types = static_cast<sal_Int32>(
        reinterpret_cast<sal_uInt64>(types)-_codeBase);
    // Fill types: (nLen, _offset to ExceptionType1, ...ExceptionType2, ...)
    types[0] = nLen;

    int nPos = 1;
    for (pCompTD = reinterpret_cast<typelib_CompoundTypeDescription*>(pTD);
        pCompTD; pCompTD = pCompTD->pBaseTypeDescription)
    {
        // Create instance in mem block with placement new
        ExceptionType * et = new(etMem + etMemOffset)ExceptionType(
            pCode + pCodeOffset, _codeBase, reinterpret_cast<typelib_TypeDescription *>(pCompTD));

        // Next trampoline entry offset
        pCodeOffset += codeSnippetSize;
        // Next ExceptionType placement offset
        etMemOffset += exceptionTypeSizeArray[nPos - 1];

        // Keep offset of addresses of ET for D-Tor call in ~RaiseInfo
        types[nPos++]
            = static_cast<DWORD>(reinterpret_cast<sal_uInt64>(et)-_codeBase);
    }
    // Final check: end of address calculation must be end of mem
    assert(etMem + etMemOffset == pCode + totalSize);
}

#if !defined LEAK_STATIC_DATA
RaiseInfo::~RaiseInfo() throw ()
{
    sal_uInt32 * pTypes = reinterpret_cast<sal_uInt32 *>(_codeBase + _types) + 1;

    // Because of placement new we have to call D.-tor, not delete!
    for ( int nTypes = *reinterpret_cast<sal_uInt32 *>(_codeBase + _types); nTypes--; )
    {
        ExceptionType *et = reinterpret_cast<ExceptionType *>(_codeBase + pTypes[nTypes]);
        et->~ExceptionType();
    }
    // free our single block
    ::rtl_freeMemory( _code );
    ::typelib_typedescription_release( _pTD );
}
#endif

ExceptionInfos::ExceptionInfos() throw ()
{
}

#if !defined LEAK_STATIC_DATA
ExceptionInfos::~ExceptionInfos() throw ()
{
    SAL_INFO("bridges", "> freeing exception infos... <");

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRaiseInfos.begin() );
          iPos != _allRaiseInfos.end(); ++iPos )
    {
        delete static_cast<RaiseInfo *>(iPos->second);
    }
}
#endif

RaiseInfo * ExceptionInfos::getRaiseInfo( typelib_TypeDescription * pTD ) throw ()
{
    static ExceptionInfos * s_pInfos = nullptr;
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

    assert( pTD &&
                (pTD->eTypeClass == typelib_TypeClass_STRUCT ||
                 pTD->eTypeClass == typelib_TypeClass_EXCEPTION) );

    RaiseInfo * pRaiseInfo;

    OUString const & rTypeName = OUString::unacquired( &pTD->pTypeName );
    MutexGuard aGuard( s_pInfos->_aMutex );
    t_string2PtrMap::const_iterator const iFind(
        s_pInfos->_allRaiseInfos.find( rTypeName ) );
    if (iFind == s_pInfos->_allRaiseInfos.end())
    {
        pRaiseInfo = new RaiseInfo( pTD );

        // Put into map
        pair< t_string2PtrMap::iterator, bool > insertion(
            s_pInfos->_allRaiseInfos.insert( t_string2PtrMap::value_type( rTypeName, static_cast<void *>(pRaiseInfo) ) ) );
        assert(insertion.second && "### raise info insertion failed?!");
    }
    else
    {
        // Reuse existing info
        pRaiseInfo = static_cast<RaiseInfo *>(iFind->second);
    }

    return pRaiseInfo;
}

type_info * mscx_getRTTI(
    OUString const & rUNOname )
{
    static RTTInfos * s_pRTTIs = nullptr;
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
int mscx_getRTTI_len(
    OUString const & rUNOname)
{
    static RTTInfos * s_pRTTIs = nullptr;
    if (!s_pRTTIs)
    {
        MutexGuard aGuard(Mutex::getGlobalMutex());
        if (!s_pRTTIs)
        {
#ifdef LEAK_STATIC_DATA
            s_pRTTIs = new RTTInfos();
#else
            static RTTInfos s_aRTTIs;
            s_pRTTIs = &s_aRTTIs;
#endif
        }
    }
    return s_pRTTIs->getRTTI_len(rUNOname);
}


void mscx_raiseException(
    uno_Any * pUnoExc,
    uno_Mapping * pUno2Cpp )
{
    // no ctor/dtor in here: this leads to dtors called twice upon RaiseException()!
    // thus this obj file will be compiled without opt, so no inlining of
    // ExceptionInfos::getRaiseInfo()

    // construct cpp exception object
    typelib_TypeDescription * pTD = nullptr;
    TYPELIB_DANGER_GET( &pTD, pUnoExc->pType );

    void * pCppExc = alloca( pTD->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTD, pUno2Cpp );

    ULONG_PTR arFilterArgs[4];
    arFilterArgs[0] = MSVC_magic_number;
    arFilterArgs[1] = reinterpret_cast<ULONG_PTR>(pCppExc);
    arFilterArgs[2] = reinterpret_cast<ULONG_PTR>(ExceptionInfos::getRaiseInfo( pTD ));
    arFilterArgs[3] = reinterpret_cast<RaiseInfo *>(arFilterArgs[2])->_codeBase;

    // Destruct uno exception
    ::uno_any_destruct( pUnoExc, nullptr );
    TYPELIB_DANGER_RELEASE( pTD );

    // last point to release anything not affected by stack unwinding
    RaiseException( MSVC_ExceptionCode, EXCEPTION_NONCONTINUABLE, 4, arFilterArgs);
}

int mscx_filterCppException(
    EXCEPTION_POINTERS * pPointers,
    uno_Any * pUnoExc,
    uno_Mapping * pCpp2Uno )
{
    if (pPointers == nullptr)
        return EXCEPTION_CONTINUE_SEARCH;

    EXCEPTION_RECORD * pRecord = pPointers->ExceptionRecord;

    // Handle only C++ exceptions:
    if (pRecord == nullptr || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    bool rethrow = __CxxDetectRethrow( &pRecord );
    assert(pRecord == pPointers->ExceptionRecord);

    if (rethrow && pRecord == pPointers->ExceptionRecord)
    {
        pRecord = *reinterpret_cast< EXCEPTION_RECORD ** >(
#if _MSC_VER >= 1900 // VC 2015 (and later?)
            __current_exception()
#else
            // Hack to get msvcrt internal _curexception field
            reinterpret_cast< char * >( __pxcptinfoptrs() ) +
            // As long as we don't demand MSVCR source as build prerequisite,
            // we have to code those offsets here.
            //
            // MSVS9/crt/src/mtdll.h:
            // offsetof (_tiddata, _curexception) -
            // offsetof (_tiddata, _tpxcptinfoptrs):
            0x48
#endif
            );
    }

    // Rethrow: handle only C++ exceptions:
    if (pRecord == nullptr || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    if (pRecord->NumberParameters == 4 &&
        pRecord->ExceptionInformation[0] == MSVC_magic_number &&
        pRecord->ExceptionInformation[1] != 0 &&
        pRecord->ExceptionInformation[2] != 0 &&
        pRecord->ExceptionInformation[3] != 0)
    {
        // ExceptionInformation[1] is the address of the thrown object
        // (or the address of a pointer to it, in most cases when it
        // is a C++ class, obviously).

        // [2] is the throwinfo pointer

        // [3] is the image base address which is added the 32-bit
        // rva_t fields in throwinfo to get actual 64-bit addresses
        ULONG_PTR base = pRecord->ExceptionInformation[3];
        DWORD * types = reinterpret_cast<DWORD *>(
            base
            + (reinterpret_cast<RaiseInfo *>(pRecord->ExceptionInformation[2])
               ->_types));
        if (types != nullptr && types[0] != 0)
        {
            DWORD pType = types[1];
            ExceptionType * et
                = reinterpret_cast<ExceptionType *>(base + pType);
            if (pType != 0 && et->_pTypeInfo != 0)
            {
                OUString aRTTIname(
                    OStringToOUString(
                        (reinterpret_cast<type_info_ *>(base + et->_pTypeInfo)
                         ->_m_d_name),
                        RTL_TEXTENCODING_ASCII_US));
                OUString aUNOname( toUNOname( aRTTIname ) );

                typelib_TypeDescription * pExcTD = nullptr;
                typelib_typedescription_getByName(
                    &pExcTD, aUNOname.pData );
                if (pExcTD == nullptr)
                {
                    OUStringBuffer buf;
                    buf.append(
                            "[mscx_uno bridge error] UNO type of "
                            "C++ exception unknown: \"" );
                    buf.append( aUNOname );
                    buf.append( "\", RTTI-name=\"" );
                    buf.append( aRTTIname );
                    buf.append( "\"!" );
                    RuntimeException exc( buf.makeStringAndClear() );
                    uno_type_any_constructAndConvert(
                        pUnoExc, &exc,
                        cppu::UnoType<decltype(exc)>::get().getTypeLibType(), pCpp2Uno );
                }
                else
                {
                    // construct uno exception any
                    uno_any_constructAndConvert(
                        pUnoExc, reinterpret_cast<void *>(pRecord->ExceptionInformation[1]),
                        pExcTD, pCpp2Uno );
                    typelib_typedescription_release( pExcTD );
                }

                return EXCEPTION_EXECUTE_HANDLER;
            }
        }
    }
    // though this unknown exception leaks now, no user-defined exception
    // is ever thrown through the binary C-UNO dispatcher call stack.
    RuntimeException exc( "[mscx_uno bridge error] unexpected "
                  "C++ exception occurred!" );
    uno_type_any_constructAndConvert(
        pUnoExc, &exc, cppu::UnoType<decltype(exc)>::get().getTypeLibType(), pCpp2Uno );
    return EXCEPTION_EXECUTE_HANDLER;
}

}

#pragma pack(pop)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
