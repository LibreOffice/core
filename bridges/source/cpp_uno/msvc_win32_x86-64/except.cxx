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


#include <sal/config.h>

#include <memory>

#include <malloc.h>
#include <new.h>
#include <typeinfo>
#include <signal.h>

#include <rtl/alloc.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <msvc/amd64.hxx>
#include <except.hxx>

#pragma pack(push, 8)

using namespace ::com::sun::star;

static void * __cdecl copyConstruct(
    void * pExcThis,
    void * pSource,
    typelib_TypeDescription * pTD ) throw ()
{
    ::uno_copyData(pExcThis, pSource, pTD, uno::cpp_acquire);
    return pExcThis;
}

static void * __cdecl destruct(
    void * pExcThis,
    typelib_TypeDescription * pTD ) throw ()
{
    ::uno_destructData(pExcThis, pTD, uno::cpp_release);
    return pExcThis;
}

const int codeSnippetSize = 40;

static void GenerateConstructorTrampoline(
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

static void GenerateDestructorTrampoline(
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

ExceptionType::ExceptionType(unsigned char * pCode, sal_uInt64 pCodeBase,
                             typelib_TypeDescription * pTD) throw ()
    : _n0(0)
    , _n1(0)
    , _n2(-1)
    , _n3(0)
    , _n4(pTD->nSize)
    , exc_type_info(nullptr, "")
{
    // As _n0 is always initialized to zero, that means the
    // hasvirtbase flag (see the ONTL catchabletype struct) is
    // off, and thus the copyctor is of the ctor_ptr kind.

    int len;
    type_info* pRTTI = RTTInfos::get(pTD->pTypeName, &len);

    memcpy(static_cast<void*>(&exc_type_info), static_cast<void*>(pRTTI), len);
    _pTypeInfo = static_cast<sal_uInt32>(reinterpret_cast<sal_uInt64>(&exc_type_info) - pCodeBase);
    GenerateConstructorTrampoline(pCode, pTD);

    assert(pCodeBase <= reinterpret_cast<sal_uInt64>(pCode)
           && (reinterpret_cast<sal_uInt64>(pCode) - pCodeBase < 0x100000000));
    _pCopyCtor = static_cast<sal_uInt32>(reinterpret_cast<sal_uInt64>(pCode) - pCodeBase);
}

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
    auto exceptionTypeSizeArray = std::make_unique<int[]>(nLen);

    nLen = 0;
    for (pCompTD = reinterpret_cast<typelib_CompoundTypeDescription*>(pTD);
        pCompTD; pCompTD = pCompTD->pBaseTypeDescription)
    {
        int typeInfoLen;
        RTTInfos::get(pCompTD->aBase.pTypeName, &typeInfoLen);
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
        static_cast<unsigned char *>(std::malloc(totalSize));
    int pCodeOffset = 0;

    // New base of types array, starts after Trampoline D-Tor / C-Tors
    DWORD * types = reinterpret_cast<DWORD *>(pCode + codeSize);

    // New base of ExceptionType array, starts after types array
    unsigned char *etMem = pCode + codeSize + typeInfoArraySize;
    int etMemOffset = 0;

    _codeBase = reinterpret_cast<sal_uInt64>(pCode)
        & ~static_cast<sal_uInt64>(ExceptionInfos::allocationGranularity - 1);

    DWORD old_protect;
    bool success =
        VirtualProtect(pCode, codeSize, PAGE_EXECUTE_READWRITE, &old_protect);
    (void) success;
    assert(success && "VirtualProtect() failed!");

    ::typelib_typedescription_acquire(pTD);

    // Fill pCode with D-Tor code
    GenerateDestructorTrampoline(pCode, pTD);
    _pDtor = static_cast<sal_Int32>(reinterpret_cast<sal_uInt64>(pCode) - _codeBase);
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

#pragma pack(pop)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
