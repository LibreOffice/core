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

#include <sal/config.h>

#if defined(_WIN32)
#include <prewin.h>
#include <postwin.h>
#endif

#include <algorithm>
#include <cstddef>
#include <map>
#include <vector>

#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <runtime.hxx>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#undef max

#include "dllmgr.hxx"

using namespace css;

/* Open issues:

   Missing support for functions returning structs (see TODO in call()).

   Missing support for additional data types (64 bit integers, Any, ...; would
   trigger assert(false) in various switches).

   It is assumed that the variables passed into SbiDllMgr::Call to represent
   the arguments and return value have types that exactly match the Declare
   statement; it would be better if this code had access to the function
   signature from the Declare statement, so that it could convert the passed
   variables accordingly.
*/

namespace {

char * address(std::vector< char > & blob) {
    return blob.empty() ? nullptr : blob.data();
}

ErrCode convert(OUString const & source, OString * target) {
    return
        source.convertToString(
            target, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR |
             RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR))
        ? ERRCODE_NONE : ERRCODE_BASIC_BAD_ARGUMENT;
        //TODO: more specific errcode?
}

ErrCode convert(char const * source, sal_Int32 length, OUString * target) {
    return
        rtl_convertStringToUString(
            &target->pData, source, length, osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
             RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR))
        ? ERRCODE_NONE : ERRCODE_BASIC_BAD_ARGUMENT;
        //TODO: more specific errcode?
}

struct UnmarshalData {
    UnmarshalData(SbxVariable * theVariable, void * theBuffer):
        variable(theVariable), buffer(theBuffer) {}

    SbxVariable * variable;
    void * buffer;
};

struct StringData: public UnmarshalData {
    StringData(SbxVariable * theVariable, void * theBuffer, bool theSpecial):
        UnmarshalData(theVariable, theBuffer), special(theSpecial) {}

    bool special;
};

class MarshalData {
public:
    MarshalData() = default;
    MarshalData(const MarshalData&) = delete;
    const MarshalData& operator=(const MarshalData&) = delete;

    std::vector< char > * newBlob() {
        blobs_.push_back(std::vector< char >());
        return &blobs_.back();
    }

    std::vector< UnmarshalData > unmarshal;

    std::vector< StringData > unmarshalStrings;

private:
    std::vector< std::vector< char > > blobs_;
};

std::size_t align(std::size_t address, std::size_t alignment) {
    // alignment = 2^k for some k >= 0
    return (address + (alignment - 1)) & ~(alignment - 1);
}

char * align(
    std::vector< char > & blob, std::size_t alignment, std::size_t offset,
    std::size_t add)
{
    std::vector< char >::size_type n = blob.size();
    n = align(n - offset, alignment) + offset; //TODO: overflow in align()
    blob.resize(n + add); //TODO: overflow
    return address(blob) + n;
}

template< typename T > void add(
    std::vector< char > & blob, T const & data, std::size_t alignment,
    std::size_t offset)
{
    *reinterpret_cast< T * >(align(blob, alignment, offset, sizeof (T))) = data;
}

std::size_t alignment(SbxVariable const * variable) {
    assert(variable != nullptr);
    if ((variable->GetType() & SbxARRAY) == 0) {
        switch (variable->GetType()) {
        case SbxINTEGER:
            return 2;
        case SbxLONG:
        case SbxSINGLE:
        case SbxSTRING:
            return 4;
        case SbxDOUBLE:
            return 8;
        case SbxOBJECT:
            {
                std::size_t n = 1;
                SbxObject* pobj = dynamic_cast<SbxObject*>(variable->GetObject());
                assert(pobj);
                SbxArray* props = pobj->GetProperties();
                for (sal_uInt32 i = 0; i < props->Count(); ++i)
                {
                    n = std::max(n, alignment(props->Get(i)));
                }
                return n;
            }
        case SbxBOOL:
        case SbxBYTE:
            return 1;
        default:
            assert(false);
            return 1;
        }
    } else {
        SbxDimArray * arr = dynamic_cast<SbxDimArray*>( variable->GetObject() );
        assert(arr);
        sal_Int32 dims = arr->GetDims();
        std::vector< sal_Int32 > low(dims);
        for (sal_Int32 i = 0; i < dims; ++i) {
            sal_Int32 up;
            arr->GetDim(i + 1, low[i], up);
        }
        return alignment(arr->Get(low.data()));
    }
}

ErrCode marshal(
    bool outer, SbxVariable * variable, bool special,
    std::vector< char > & blob, std::size_t offset, MarshalData & data);

ErrCode marshalString(
    SbxVariable * variable, bool special, MarshalData & data, void ** buffer)
{
    assert(variable != nullptr && buffer != nullptr);
    OString str;
    ErrCode e = convert(variable->GetOUString(), &str);
    if (e != ERRCODE_NONE) {
        return e;
    }
    std::vector< char > * blob = data.newBlob();
    blob->insert(blob->begin(), str.getStr(), str.getStr() + str.getLength() + 1);
    *buffer = address(*blob);
    data.unmarshalStrings.push_back(StringData(variable, *buffer, special));
    return ERRCODE_NONE;
}

ErrCode marshalStruct(
    SbxVariable const * variable, std::vector< char > & blob, std::size_t offset,
    MarshalData & data)
{
    assert(variable != nullptr);
    SbxObject* pobj = dynamic_cast<SbxObject*>(variable->GetObject());
    assert(pobj);
    SbxArray* props = pobj->GetProperties();
    for (sal_uInt32 i = 0; i < props->Count(); ++i)
    {
        ErrCode e = marshal(false, props->Get(i), false, blob, offset, data);
        if (e != ERRCODE_NONE) {
            return e;
        }
    }
    return ERRCODE_NONE;
}

ErrCode marshalArray(
    SbxVariable const * variable, std::vector< char > & blob, std::size_t offset,
    MarshalData & data)
{
    assert(variable != nullptr);
    SbxDimArray * arr = dynamic_cast<SbxDimArray*>( variable->GetObject() );
    assert(arr);
    sal_Int32 dims = arr->GetDims();
    std::vector< sal_Int32 > low(dims);
    std::vector< sal_Int32 > up(dims);
    for (sal_Int32 i = 0; i < dims; ++i) {
        arr->GetDim(i + 1, low[i], up[i]);
    }
    for (std::vector< sal_Int32 > idx = low;;) {
        ErrCode e = marshal(false, arr->Get(idx.data()), false, blob, offset, data);
        if (e != ERRCODE_NONE) {
            return e;
        }
        sal_Int32 i = dims - 1;
        while (idx[i] == up[i]) {
            idx[i] = low[i];
            if (i == 0) {
                return ERRCODE_NONE;
            }
            --i;
        }
        ++idx[i];
    }
}

// 8-aligned structs are only 4-aligned on stack, so alignment of members in
// such structs must take that into account via "offset"
ErrCode marshal(
    bool outer, SbxVariable * variable, bool special,
    std::vector< char > & blob, std::size_t offset, MarshalData & data)
{
    assert(variable != nullptr);

    SbxDataType eVarType = variable->GetType();
    bool bByVal = !(variable->GetFlags() & SbxFlagBits::Reference);
    if( !bByVal && !SbiRuntime::isVBAEnabled() && eVarType == SbxSTRING )
        bByVal = true;

    if (bByVal) {
        if ((eVarType & SbxARRAY) == 0) {
            switch (eVarType) {
            case SbxINTEGER:
                add(blob, variable->GetInteger(), outer ? 8 : 2, offset);
                break;
            case SbxLONG:
                add(blob, variable->GetLong(), outer ? 8 : 4, offset);
                break;
            case SbxSINGLE:
                add(blob, variable->GetSingle(), outer ? 8 : 4, offset);
                break;
            case SbxDOUBLE:
                add(blob, variable->GetDouble(), 8, offset);
                break;
            case SbxSTRING:
                {
                    void * p;
                    ErrCode e = marshalString(variable, special, data, &p);
                    if (e != ERRCODE_NONE) {
                        return e;
                    }
                    add(blob, p, 8, offset);
                    break;
                }
            case SbxOBJECT:
                {
                    align(blob, outer ? 8 : alignment(variable), offset, 0);
                    ErrCode e = marshalStruct(variable, blob, offset, data);
                    if (e != ERRCODE_NONE) {
                        return e;
                    }
                    break;
                }
            case SbxBOOL:
                add(blob, variable->GetBool(), outer ? 8 : 1, offset);
                break;
            case SbxBYTE:
                add(blob, variable->GetByte(), outer ? 8 : 1, offset);
                break;
            default:
                assert(false);
                break;
            }
        } else {
            ErrCode e = marshalArray(variable, blob, offset, data);
            if (e != ERRCODE_NONE) {
                return e;
            }
        }
    } else {
        if ((eVarType & SbxARRAY) == 0) {
            switch (eVarType) {
            case SbxINTEGER:
            case SbxLONG:
            case SbxSINGLE:
            case SbxDOUBLE:
            case SbxBOOL:
            case SbxBYTE:
                add(blob, variable->data(), 8, offset);
                break;
            case SbxSTRING:
                {
                    void * p;
                    ErrCode e = marshalString(variable, special, data, &p);
                    if (e != ERRCODE_NONE) {
                        return e;
                    }
                    std::vector< char >* blob2 = data.newBlob();
                    add(*blob2, p, 8, 0);
                    add(blob, address(*blob2), 8, offset);
                    break;
                }
            case SbxOBJECT:
                {
                    std::vector< char > * blob2 = data.newBlob();
                    ErrCode e = marshalStruct(variable, *blob2, 0, data);
                    if (e != ERRCODE_NONE) {
                        return e;
                    }
                    void * p = address(*blob2);
                    if (outer) {
                        data.unmarshal.push_back(UnmarshalData(variable, p));
                    }
                    add(blob, p, 8, offset);
                    break;
                }
            default:
                assert(false);
                break;
            }
        } else {
            std::vector< char > * blob2 = data.newBlob();
            ErrCode e = marshalArray(variable, *blob2, 0, data);
            if (e != ERRCODE_NONE) {
                return e;
            }
            void * p = address(*blob2);
            if (outer) {
                data.unmarshal.push_back(UnmarshalData(variable, p));
            }
            add(blob, p, 8, offset);
        }
    }
    return ERRCODE_NONE;
}

template< typename T > T read(void const ** pointer) {
    T const * p = static_cast< T const * >(*pointer);
    *pointer = static_cast< void const * >(p + 1);
    return *p;
}

void const * unmarshal(SbxVariable * variable, void const * data) {
    assert(variable != nullptr);
    if ((variable->GetType() & SbxARRAY) == 0) {
        switch (variable->GetType()) {
        case SbxINTEGER:
            variable->PutInteger(read< sal_Int16 >(&data));
            break;
        case SbxLONG:
            variable->PutLong(read< sal_Int32 >(&data));
            break;
        case SbxSINGLE:
            variable->PutSingle(read< float >(&data));
            break;
        case SbxDOUBLE:
            variable->PutDouble(read< double >(&data));
            break;
        case SbxSTRING:
            read< char * >(&data); // handled by unmarshalString
            break;
        case SbxOBJECT:
            {
                data = reinterpret_cast< void const * >(
                    align(
                        reinterpret_cast< sal_uIntPtr >(data),
                        alignment(variable)));
                SbxObject* pobj = dynamic_cast<SbxObject*>(variable->GetObject());
                assert(pobj);
                SbxArray* props = pobj->GetProperties();
                for (sal_uInt32 i = 0; i < props->Count(); ++i)
                {
                    data = unmarshal(props->Get(i), data);
                }
                break;
            }
        case SbxBOOL:
            variable->PutBool(read< sal_Bool >(&data));
            break;
        case SbxBYTE:
            variable->PutByte(read< sal_uInt8 >(&data));
            break;
        default:
            assert(false);
            break;
        }
    } else {
        SbxDimArray * arr = dynamic_cast<SbxDimArray*>( variable->GetObject() );
        assert(arr);
        sal_Int32 dims = arr->GetDims();
        std::vector< sal_Int32 > low(dims);
        std::vector< sal_Int32 > up(dims);
        for (sal_Int32 i = 0; i < dims; ++i) {
            arr->GetDim(i + 1, low[i], up[i]);
        }
        for (std::vector< sal_Int32 > idx = low;;) {
            data = unmarshal(arr->Get(idx.data()), data);
            sal_Int32 i = dims - 1;
            while (idx[i] == up[i]) {
                idx[i] = low[i];
                if (i == 0) {
                    goto done;
                }
                --i;
            }
            ++idx[i];
        }
    done:;
    }
    return data;
}

ErrCode unmarshalString(StringData const & data, SbxVariable const & result) {
    OUString str;
    if (data.buffer != nullptr) {
        char const * p = static_cast< char const * >(data.buffer);
        sal_Int32 len;
        if (data.special) {
            len = static_cast< sal_Int32 >(result.GetULong());
            if (len < 0) { // i.e., DWORD result >= 2^31
                return ERRCODE_BASIC_BAD_ARGUMENT;
                    //TODO: more specific errcode?
            }
        } else {
            len = rtl_str_getLength(p);
        }
        ErrCode e = convert(p, len, &str);
        if (e != ERRCODE_NONE) {
            return e;
        }
    }
    data.variable->PutString(str);
    return ERRCODE_NONE;
}

struct ProcData {
    OString name;
    FARPROC proc;
};

ErrCode call(
    OUString const & dll, ProcData const & proc, SbxArray * arguments,
    SbxVariable & result)
{
    if (arguments && arguments->Count() > 20)
        return ERRCODE_BASIC_NOT_IMPLEMENTED;

    std::vector< char > stack;
    MarshalData data;

    // For DWORD GetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer)
    // from kernel32, upon return, filled lpBuffer length is result DWORD, which
    // requires special handling in unmarshalString; other functions might
    // require similar treatment, too:
    bool special =
        dll.equalsIgnoreAsciiCase("KERNEL32.DLL") &&
        (proc.name == OString("GetLogicalDriveStringsA"));
    for (sal_uInt32 i = 1; i < (arguments == nullptr ? 0 : arguments->Count()); ++i)
    {
        ErrCode e = marshal(true, arguments->Get(i), special && i == 2, stack, stack.size(),
            data);
        if (e != ERRCODE_NONE) {
            return e;
        }
        align(stack, 8, 0, 0);
    }

    stack.resize(20*8);

    // We fake all calls as being to a varargs function,
    // as this means any floating-point argument among the first four
    // ones will end up in a XMM register where the callee expects it.
    sal_Int32 (*proc_i)(double d, ...) = reinterpret_cast<sal_Int32 (*)(double, ...)>(proc.proc);
    double (*proc_d)(double d, ...) = reinterpret_cast<double (*)(double, ...)>(proc.proc);

    sal_Int64 iRetVal = 0;
    double dRetVal = 0.0;

    switch (result.GetType()) {
    case SbxEMPTY:
    case SbxINTEGER:
    case SbxLONG:
    case SbxSTRING:
    case SbxOBJECT:
    case SbxBOOL:
    case SbxBYTE:
        {
            auto const st = stack.data();
            iRetVal =
                proc_i(*reinterpret_cast<double *>(st + 0),
                       *reinterpret_cast<double *>(st + 1*8),
                       *reinterpret_cast<double *>(st + 2*8),
                       *reinterpret_cast<double *>(st + 3*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 4*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 5*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 6*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 7*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 8*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 9*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 10*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 11*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 12*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 13*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 14*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 15*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 16*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 17*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 18*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 19*8));
            break;
        }
    case SbxSINGLE:
    case SbxDOUBLE:
        {
            auto const st = stack.data();
            dRetVal =
                proc_d(*reinterpret_cast<double *>(st + 0),
                       *reinterpret_cast<double *>(st + 1*8),
                       *reinterpret_cast<double *>(st + 2*8),
                       *reinterpret_cast<double *>(st + 3*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 4*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 5*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 6*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 7*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 8*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 9*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 10*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 11*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 12*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 13*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 14*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 15*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 16*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 17*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 18*8),
                       *reinterpret_cast<sal_uInt64 *>(st + 19*8));
            break;
        }
    default:
        break;
    }

    switch (result.GetType()) {
    case SbxEMPTY:
        break;
    case SbxINTEGER:
        result.PutInteger(static_cast< sal_Int16 >(iRetVal));
        break;
    case SbxLONG:
        result.PutLong(static_cast< sal_Int32 >(iRetVal));
        break;
    case SbxSINGLE:
        result.PutSingle(static_cast< float >(dRetVal));
        break;
    case SbxDOUBLE:
        result.PutDouble(dRetVal);
        break;
    case SbxSTRING:
        {
            char const * s1 = reinterpret_cast< char const * >(iRetVal);
            OUString s2;
            ErrCode e = convert(s1, rtl_str_getLength(s1), &s2);
            if (e != ERRCODE_NONE) {
                return e;
            }
            result.PutString(s2);
            break;
        }
    case SbxOBJECT:
        //TODO
        break;
    case SbxBOOL:
        result.PutBool(bool(iRetVal));
        break;
    case SbxBYTE:
        result.PutByte(static_cast< sal_uInt8 >(iRetVal));
        break;
    default:
        assert(false);
        break;
    }
    for (sal_uInt32 i = 1; i < (arguments == nullptr ? 0 : arguments->Count()); ++i)
    {
        arguments->Get(i)->ResetFlag(SbxFlagBits::Reference);
            //TODO: skipped for errors?!?
    }
    for (auto const& elem : data.unmarshal)
    {
        unmarshal(elem.variable, elem.buffer);
    }
    for (auto const& elem : data.unmarshalStrings)
    {
        ErrCode e = unmarshalString(elem, result);
        if (e != ERRCODE_NONE) {
            return e;
        }
    }
    return ERRCODE_NONE;
}

ErrCode getProcData(HMODULE handle, OUString const & name, ProcData * proc)
{
    assert(proc != nullptr);
    if (name.getLength() != 0 && name[0] == '@') { //TODO: "@" vs. "#"???
        sal_Int32 n = name.copy(1).toInt32(); //TODO: handle bad input
        if (n <= 0 || n > 0xFFFF) {
            return ERRCODE_BASIC_BAD_ARGUMENT; //TODO: more specific errcode?
        }
        FARPROC p = GetProcAddress(handle, reinterpret_cast< LPCSTR >(n));
        if (p != nullptr) {
            proc->name = "#" + OString::number(n);
            proc->proc = p;
            return ERRCODE_NONE;
        }
    } else {
        OString name8;
        ErrCode e = convert(name, &name8);
        if (e != ERRCODE_NONE) {
            return e;
        }
        FARPROC p = GetProcAddress(handle, name8.getStr());
        if (p != nullptr) {
            proc->name = name8;
            proc->proc = p;
            return ERRCODE_NONE;
        }
        sal_Int32 i = name8.indexOf('#');
        if (i != -1) {
            name8 = name8.copy(0, i);
            p = GetProcAddress(handle, name8.getStr());
            if (p != nullptr) {
                proc->name = name8;
                proc->proc = p;
                return ERRCODE_NONE;
            }
        }
        OString real("_" + name8);
        p = GetProcAddress(handle, real.getStr());
        if (p != nullptr) {
            proc->name = real;
            proc->proc = p;
            return ERRCODE_NONE;
        }
        real = name8 + "A";
        p = GetProcAddress(handle, real.getStr());
        if (p != nullptr) {
            proc->name = real;
            proc->proc = p;
            return ERRCODE_NONE;
        }
    }
    return ERRCODE_BASIC_PROC_UNDEFINED;
}

struct Dll: public salhelper::SimpleReferenceObject {
private:
    typedef std::map< OUString, ProcData > Procs;

    virtual ~Dll() override;

public:
    Dll(): handle(nullptr) {}

    ErrCode getProc(OUString const & name, ProcData * proc);

    HMODULE handle;
    Procs procs;
};

Dll::~Dll() {
    if (handle != nullptr && !FreeLibrary(handle)) {
        SAL_WARN("basic", "FreeLibrary(" << handle << ") failed with " << GetLastError());
    }
}

ErrCode Dll::getProc(OUString const & name, ProcData * proc) {
    Procs::iterator i(procs.find(name));
    if (i != procs.end()) {
        *proc = i->second;
        return ERRCODE_NONE;
    }
    ErrCode e = getProcData(handle, name, proc);
    if (e == ERRCODE_NONE) {
        procs.emplace(name, *proc);
    }
    return e;
}

OUString fullDllName(OUString const & name) {
    OUString full(name);
    if (full.indexOf('.') == -1) {
        full += ".DLL";
    }
    return full;
}

}

struct SbiDllMgr::Impl{
private:
    typedef std::map< OUString, ::rtl::Reference< Dll > > Dlls;

public:
    Impl() = default;
    Impl(const Impl&) = delete;
    const Impl& operator=(const Impl&) = delete;
    
    Dll * getDll(OUString const & name);

    Dlls dlls;
};

Dll * SbiDllMgr::Impl::getDll(OUString const & name) {
    Dlls::iterator i(dlls.find(name));
    if (i == dlls.end()) {
        i = dlls.emplace(name, new Dll).first;
        HMODULE h = LoadLibraryW(o3tl::toW(name.getStr()));
        if (h == nullptr) {
            dlls.erase(i);
            return nullptr;
        }
        i->second->handle = h;
    }
    return i->second.get();
}

ErrCode SbiDllMgr::Call(
    std::u16string_view function, std::u16string_view library,
    SbxArray * arguments, SbxVariable & result, bool cdeclConvention)
{
    if (cdeclConvention) {
        return ERRCODE_BASIC_NOT_IMPLEMENTED;
    }
    OUString dllName(fullDllName(OUString(library)));
    Dll * dll = impl_->getDll(dllName);
    if (dll == nullptr) {
        return ERRCODE_BASIC_BAD_DLL_LOAD;
    }
    ProcData proc;
    ErrCode e = dll->getProc(OUString(function), &proc);
    if (e != ERRCODE_NONE) {
        return e;
    }
    return call(dllName, proc, arguments, result);
}

void SbiDllMgr::FreeDll(OUString const & library) {
    impl_->dlls.erase(library);
}

SbiDllMgr::SbiDllMgr(): impl_(new Impl) {}

SbiDllMgr::~SbiDllMgr() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
