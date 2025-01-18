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

#include "file_url.hxx"
#include "path_helper.hxx"

#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <systools/win32/extended_max_path.hxx>
#include <vector>

/*
    under WIN32, we use the void* oslModule
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 /*nRtldMode*/ )
{
#if OSL_DEBUG_LEVEL < 2
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
#endif
    SAL_INFO( "sal.osl", "osl_loadModule: " << OUString(strModuleName) );
    OSL_ASSERT(strModuleName);

    OUString Module;
    oslFileError nError = osl_getSystemPathFromFileURL(strModuleName, &Module.pData);

    if ( osl_File_E_None != nError )
        Module = OUString::unacquired(&strModuleName);

    HMODULE h = LoadLibraryW(o3tl::toW(Module.getStr()));

    if (h == nullptr)
        h = LoadLibraryExW(o3tl::toW(Module.getStr()), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    // In case of long path names (\\?\c:\...) try to shorten the filename.
    // LoadLibrary cannot handle file names which exceed 260 letters.
    // In case the path is too long, the function will fail. However, the error
    // code can be different. For example, it returned  ERROR_FILENAME_EXCED_RANGE
    // on Windows XP and ERROR_INSUFFICIENT_BUFFER on Windows 7 (64bit)
    if (h == nullptr && Module.getLength() > 260)
    {
        std::vector<WCHAR> vec(Module.getLength() + 1);
        DWORD len = GetShortPathNameW(o3tl::toW(Module.getStr()), vec.data(), vec.size());
        if (len )
        {
            h = LoadLibraryW(vec.data());

            if (h == nullptr)
                h = LoadLibraryExW(vec.data(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
    }

#if OSL_DEBUG_LEVEL < 2
    SetErrorMode(errorMode);
#endif

    return static_cast<oslModule>(h);
}

oslModule SAL_CALL osl_loadModuleAscii(const char *pModuleName, sal_Int32 )
{
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);

    SAL_INFO( "sal.osl", "osl_loadModule: " << pModuleName );
    OSL_ASSERT(pModuleName);

    HMODULE h = LoadLibraryA(pModuleName);
    if (h == nullptr)
        h = LoadLibraryExA(pModuleName, nullptr,
                                  LOAD_WITH_ALTERED_SEARCH_PATH);

    SetErrorMode(errorMode);

    return static_cast<oslModule>(h);
}

oslModule osl_loadModuleRelativeAscii(
    oslGenericFunction baseModule, char const * relativePath, sal_Int32 mode)
{
    return osl_loadModuleRelative(baseModule, OUString::createFromAscii(relativePath).pData, mode);
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
    return reinterpret_cast<void*>(osl_getFunctionSymbol(Module, strSymbolName));
}

oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *strSymbolName )
{
    OSL_ASSERT(Module);
    assert(strSymbolName);

    auto symbolName(OUStringToOString(OUString::unacquired(&strSymbolName), RTL_TEXTENCODING_UTF8));

    return osl_getAsciiFunctionSymbol(Module, symbolName.getStr());
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
    HMODULE hModule{};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                           | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       static_cast<LPCWSTR>(pv), &hModule);
    if (!hModule)
        return false;

    osl::LongPathBuffer<sal_Unicode> aBuffer(EXTENDED_MAX_PATH);

    DWORD nch = GetModuleFileNameW(hModule, o3tl::toW(aBuffer), aBuffer.getBufSizeInSymbols());

    OUString ustrSysPath(aBuffer, nch);
    return osl_getFileURLFromSystemPath(ustrSysPath.pData, pustrURL) == osl_File_E_None;
}

sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress( oslGenericFunction addr, rtl_uString ** ppLibraryUrl )
{
    /* casting a function pointer to a data pointer (void*) is
       not allowed according to the C/C++ standards. In this case
       it is unavoidable because we have to stay compatible we
       cannot remove any function. */
    return osl_getModuleURLFromAddress(reinterpret_cast<void*>(addr), ppLibraryUrl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
