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

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <sal/config.h>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>

#include <mutex>
#include <unordered_map>

typedef struct _uno_Any uno_Any;
typedef struct _uno_Mapping uno_Mapping;

int msvc_filterCppException(EXCEPTION_POINTERS*, uno_Any*, uno_Mapping*);
[[noreturn]] void msvc_raiseException(uno_Any*, uno_Mapping*);

constexpr DWORD MSVC_EH_MAGIC_PARAM = 0x19930520;
// The NT Exception code that msvcrt uses ('msc' | 0xE0000000)
constexpr DWORD MSVC_EH_MAGIC_CODE = 0xE06D7363;

#if defined(_M_IX86)
#define MSVC_EH_PARAMETERS 3 // Number of parameters in exception record for x86
#elif defined(_M_AMD64) || defined(_M_ARM64)
#define MSVC_EH_PARAMETERS 4 // Number of parameters in exception record for AMD64
#else
#error "Unsupported machine type"
#endif

class type_info;
struct RaiseInfo;
typedef std::unordered_map<OUString, void*> t_string2PtrMap;
typedef struct _typelib_TypeDescription typelib_TypeDescription;

// fixed class, because sizeof(ExceptionTypeInfo) must be sizeof(type_info)
// this is tested by a static_assert, so can't break.
class ExceptionTypeInfo final
{
    friend int msvc_filterCppException(EXCEPTION_POINTERS*, uno_Any*, uno_Mapping*);

    void* m_data;
    char m_d_name[1];

public:
    explicit ExceptionTypeInfo(void* data, const char* d_name) noexcept
        : m_data(data)
    {
        ::strcpy(m_d_name, d_name); // #100211# - checked
    }
    virtual ~ExceptionTypeInfo() noexcept;
};

class ExceptionTypeInfoWrapper final
{
    int type_info_size;
    ExceptionTypeInfo info;

public:
    explicit ExceptionTypeInfoWrapper(void* m_data, const char* m_d_name) noexcept
        : info(m_data, m_d_name)
    {
        type_info_size = sizeof(ExceptionTypeInfo) + strlen(m_d_name);
    }

    type_info* get_type_info() { return reinterpret_cast<type_info*>(&info); }
    int get_type_info_size() { return type_info_size; }
};

class RTTInfos final
{
    osl::Mutex m_aMutex;
    t_string2PtrMap m_allRTTI;

    RTTInfos() noexcept;
    ExceptionTypeInfoWrapper* getInfo(OUString const& rUNOname) noexcept;

public:
    ~RTTInfos();

    static type_info* get(OUString const& rUNOname, int* len = nullptr) noexcept;
};

class ExceptionInfos final
{
    std::mutex m_aMutex;
    t_string2PtrMap m_allRaiseInfos;

public:
    static RaiseInfo* getRaiseInfo(typelib_TypeDescription* pTD) noexcept;

    static DWORD allocationGranularity;

    ExceptionInfos() noexcept;
    ~ExceptionInfos() noexcept;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
