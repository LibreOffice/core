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

#include "system.h"
#include <psapi.h>

#include "file_url.hxx"
#include "path_helper.hxx"

#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <vector>

/*
    under WIN32, we use the void* oslModule
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 /*nRtldMode*/ )
{
    HMODULE h;
#if OSL_DEBUG_LEVEL < 2
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif
    rtl_uString* Module = nullptr;
    oslModule ret = nullptr;
    oslFileError    nError;

    SAL_INFO( "sal.osl", "osl_loadModule: " << OUString(strModuleName) );
    OSL_ASSERT(strModuleName);

    nError = osl_getSystemPathFromFileURL(strModuleName, &Module);

    if ( osl_File_E_None != nError )
        rtl_uString_assign(&Module, strModuleName);

    h = LoadLibraryW(o3tl::toW(Module->buffer));

    if (h == nullptr)
        h = LoadLibraryExW(o3tl::toW(Module->buffer), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    // In case of long path names (\\?\c:\...) try to shorten the filename.
    // LoadLibrary cannot handle file names which exceed 260 letters.
    // In case the path is too long, the function will fail. However, the error
    // code can be different. For example, it returned  ERROR_FILENAME_EXCED_RANGE
    // on Windows XP and ERROR_INSUFFICIENT_BUFFER on Windows 7 (64bit)
    if (h == nullptr && Module->length > 260)
    {
        std::vector<WCHAR> vec(Module->length + 1);
        DWORD len = GetShortPathNameW(o3tl::toW(Module->buffer), vec.data(), Module->length + 1);
        if (len )
        {
            h = LoadLibraryW(vec.data());

            if (h == nullptr)
                h = LoadLibraryExW(vec.data(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
    }

    ret = static_cast<oslModule>(h);
    rtl_uString_release(Module);
#if OSL_DEBUG_LEVEL < 2
    SetErrorMode(errorMode);
#endif

    return ret;
}

oslModule SAL_CALL osl_loadModuleAscii(const char *pModuleName, sal_Int32 )
{
    HMODULE h;
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    oslModule ret = nullptr;

    SAL_INFO( "sal.osl", "osl_loadModule: " << pModuleName );
    OSL_ASSERT(pModuleName);

    h = LoadLibraryA(pModuleName);
    if (h == nullptr)
        h = LoadLibraryExA(pModuleName, nullptr,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);

    ret = static_cast<oslModule>(h);
    SetErrorMode(errorMode);

    return ret;
}

oslModule osl_loadModuleRelativeAscii(
    oslGenericFunction, char const * relativePath, sal_Int32 mode)
{
    return osl_loadModuleAscii(relativePath, mode); //TODO: FIXME
}

sal_Bool SAL_CALL
osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult)
{
    LPCWSTR pName = pModuleName ? o3tl::toW(pModuleName->buffer) : nullptr;
    HMODULE h = GetModuleHandleW(pName);
    if( h )
    {
        *pResult = static_cast<oslModule>(h);
        return true;
    }

    return false;
}

void SAL_CALL osl_unloadModule(oslModule Module)
{
    FreeLibrary(static_cast<HMODULE>(Module));
}

void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *strSymbolName)
{
    /* casting from a function pointer to a data pointer is invalid
       be in this case unavoidable because the API has to stay
       compatible. We need to keep this function which returns a
       void* by definition */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4054)
#endif
    return reinterpret_cast<void*>(osl_getFunctionSymbol(Module, strSymbolName));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *strSymbolName )
{
    rtl_String *symbolName = nullptr;
    oslGenericFunction address;

    OSL_ASSERT(Module);
    OSL_ASSERT(strSymbolName);

    rtl_uString2String(
        &symbolName,
        strSymbolName->buffer,
        strSymbolName->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS
    );

    address=osl_getAsciiFunctionSymbol(Module, rtl_string_getStr(symbolName));
    rtl_string_release(symbolName);

    return address;
}

oslGenericFunction SAL_CALL
osl_getAsciiFunctionSymbol( oslModule Module, const char *pSymbol )
{
    oslGenericFunction fncAddr = nullptr;

    if( pSymbol )
        fncAddr=reinterpret_cast<oslGenericFunction>(GetProcAddress(static_cast<HMODULE>(Module), pSymbol));

    return fncAddr;
}

sal_Bool SAL_CALL osl_getModuleURLFromAddress( void *pv, rtl_uString **pustrURL )
{
    static HMODULE hModPsapi = LoadLibraryW( L"PSAPI.DLL" );
    static auto lpfnEnumProcessModules = reinterpret_cast<decltype(EnumProcessModules)*>(
        hModPsapi ? GetProcAddress(hModPsapi, "EnumProcessModules") : nullptr);
    static auto lpfnGetModuleInformation = reinterpret_cast<decltype(GetModuleInformation)*>(
        hModPsapi ? GetProcAddress(hModPsapi, "GetModuleInformation") : nullptr);

    if (!lpfnEnumProcessModules || !lpfnGetModuleInformation)
        return false;

    bool bSuccess    = false;    /* Assume failure */
    DWORD cbNeeded = 0;
    HMODULE* lpModules = nullptr;
    DWORD nModules = 0;
    UINT iModule = 0;
    MODULEINFO modinfo;

    lpfnEnumProcessModules(GetCurrentProcess(), nullptr, 0, &cbNeeded);

    lpModules = static_cast<HMODULE*>(_alloca(cbNeeded));
    lpfnEnumProcessModules(GetCurrentProcess(), lpModules, cbNeeded, &cbNeeded);

    nModules = cbNeeded / sizeof(HMODULE);

    for (iModule = 0; !bSuccess && iModule < nModules; iModule++)
    {
        lpfnGetModuleInformation(GetCurrentProcess(), lpModules[iModule], &modinfo,
                                 sizeof(modinfo));

        if (static_cast<BYTE*>(pv) >= static_cast<BYTE*>(modinfo.lpBaseOfDll)
            && static_cast<BYTE*>(pv)
                   < static_cast<BYTE*>(modinfo.lpBaseOfDll) + modinfo.SizeOfImage)
        {
            ::osl::LongPathBuffer<sal_Unicode> aBuffer(MAX_LONG_PATH);
            rtl_uString* ustrSysPath = nullptr;

            GetModuleFileNameW(lpModules[iModule], o3tl::toW(aBuffer),
                               aBuffer.getBufSizeInSymbols());

            rtl_uString_newFromStr(&ustrSysPath, aBuffer);
            osl_getFileURLFromSystemPath(ustrSysPath, pustrURL);
            rtl_uString_release(ustrSysPath);

            bSuccess = true;
        }
    }

    return bSuccess;
}

sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress( oslGenericFunction addr, rtl_uString ** ppLibraryUrl )
{
    /* casting a function pointer to a data pointer (void*) is
       not allowed according to the C/C++ standards. In this case
       it is unavoidable because we have to stay compatible we
       cannot remove any function. */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4054)
#endif
    return osl_getModuleURLFromAddress(reinterpret_cast<void*>(addr), ppLibraryUrl);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
