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

#ifndef INCLUDED_OSL_MODULE_H
#define INCLUDED_OSL_MODULE_H

#include "sal/config.h"

#include "rtl/ustring.h"
#include "sal/saldllapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_DLLPREFIX
#define SAL_MODULENAME(name) SAL_DLLPREFIX name SAL_DLLEXTENSION
#else
#define SAL_MODULENAME(name) name SAL_DLLEXTENSION
#endif

#if defined(_WIN32)
#define SAL_MODULENAME_WITH_VERSION(name, version) name version SAL_DLLEXTENSION

#elif defined(SAL_UNX)
#if defined(MACOSX)
#define SAL_MODULENAME_WITH_VERSION(name, version) SAL_DLLPREFIX name ".dylib." version
#else
#define SAL_MODULENAME_WITH_VERSION(name, version) SAL_DLLPREFIX name SAL_DLLEXTENSION "." version
#endif

#endif

#define SAL_LOADMODULE_DEFAULT    0x00000
#define SAL_LOADMODULE_LAZY       0x00001
#define SAL_LOADMODULE_NOW        0x00002
#define SAL_LOADMODULE_GLOBAL     0x00100

typedef void* oslModule;

/** Generic Function pointer type that will be used as symbol address.

    @see osl_getFunctionSymbol.
    @see osl_getModuleURLFromFunctionAddress.
*/
typedef void ( SAL_CALL *oslGenericFunction )( void );

#ifndef DISABLE_DYNLOADING

/** Load a shared library or module.

    @param[in] strModuleName denotes the name of the module to be loaded.
    @param[in] nRtldMode denotes the mode.

    @returns NULL if the module could not be loaded, otherwise a handle to the module.
*/
SAL_DLLPUBLIC oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 nRtldMode);

/** Load a shared library or module.

    @param[in] pModuleName denotes the name of the module to be loaded.
    @param[in] nRtldMode denotes the mode.

    @return NULL if the module could not be loaded, otherwise a handle to the module.

    @since UDK 3.6
*/
SAL_DLLPUBLIC oslModule SAL_CALL osl_loadModuleAscii(const sal_Char *pModuleName, sal_Int32 nRtldMode);

/** Load a module located relative to some other module.

    @param[in] baseModule must point to a function that is part of the code of some loaded module;
                        must not be NULL.
    @param[in] relativePath a relative URL; must not be NULL.
    @param[in] mode the SAL_LOADMODULE_xxx flags.

    @return a non-NULL handle to the loaded module, or NULL if an error occurred.

    @since UDK 3.2.8
*/
SAL_DLLPUBLIC oslModule SAL_CALL osl_loadModuleRelative(
    oslGenericFunction baseModule, rtl_uString * relativePath, sal_Int32 mode);

/** Load a module located relative to some other module.

    @param[in] baseModule must point to a function that is part of the code of some loaded module;
                        must not be NULL.
    @param[in] relativePath a relative URL containing only ASCII (0x01--7F) characters;
                        must not be NULL.
    @param[in] mode     the SAL_LOADMODULE_xxx flags.

    @return a non-NULL handle to the loaded module, or NULL if an error occurred.

    @since LibreOffice 3.5
*/
SAL_DLLPUBLIC oslModule SAL_CALL osl_loadModuleRelativeAscii(
    oslGenericFunction baseModule, char const * relativePath, sal_Int32 mode);
    /* This function is guaranteed not to call into
       FullTextEncodingDataSingleton in sal/textenc/textenc.cxx, so can be used
       in its implementation without running into circles. */

#endif

/** Retrieve the handle of an already loaded module.

    This function can be used to search for a function symbol in the process address space.
    Do not use the returned handle as an argument to osl_unloadModule. On Unix platforms,
    pModuleName gets ignored and the special handle RTLD_DEFAULT is returned.

    @param[in] pModuleName  denotes the name of the module to search for.
                            @attention Ignored on Unix.
    @param[out] pResult     a pointer to a oslModule that is updated with the
                            requested module handle on success.

    @retval sal_True if the module handle could be retrieved and has been copied to *pResult.
    @retval sal_False if the module has not been loaded yet.

    @see osl_getFunctionSymbol
    @see osl_getAsciiFunctionSymbol
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult);

#ifndef DISABLE_DYNLOADING

/** Release the module
*/
SAL_DLLPUBLIC void SAL_CALL osl_unloadModule(oslModule Module);

#endif

/** lookup the specified symbol name.

    @param[in] Module the handle of the Module.
    @param[in] strSymbolName Name of the function that will be looked up.

    @return address of the symbol or NULL if lookup failed.

    @see osl_getFunctionSymbol
*/
SAL_DLLPUBLIC void* SAL_CALL osl_getSymbol( oslModule Module, rtl_uString *strSymbolName);

/** Lookup the specified function symbol name.

    osl_getFunctionSymbol is an alternative function for osl_getSymbol.
    Use Function pointer as symbol address to conceal type conversion.

    @param[in] Module the handle of the Module.
    @param[in] ustrFunctionSymbolName Unicode name of the function that will be looked up.

    @retval function-address on success
    @retval NULL lookup failed or the parameter are invalid

    @see osl_getSymbol
    @see osl_getAsciiFunctionSymbol
*/
SAL_DLLPUBLIC oslGenericFunction SAL_CALL osl_getFunctionSymbol(
        oslModule Module, rtl_uString *ustrFunctionSymbolName );

/** Lookup the specified function symbol name.

    osl_getAsciiFunctionSymbol is an alternative function for osl_getFunctionSymbol.
    It expects the C-style function name string to contain ascii characters only.

    @param Module
    [in] a module handle as returned by osl_loadModule or osl_getModuleHandle

    @param pSymbol
    [in] Name of the function that will be looked up.

    @retval function-address on success
    @retval NULL lookup failed or the parameter are invalid

    @see osl_getModuleHandle
    @see osl_getFunctionSymbol
*/
SAL_DLLPUBLIC oslGenericFunction SAL_CALL osl_getAsciiFunctionSymbol(
        oslModule Module, const sal_Char *pSymbol );

/** Lookup URL of module which is mapped at the specified address.

    @param[in] pv       specifies an address in the process memory space.
    @param[out] pustrURL receives the URL of the module that is mapped at pv.
    @return sal_True on success, sal_False if no module can be found at the specified address.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getModuleURLFromAddress(
        void *pv, rtl_uString **pustrURL );

/** Lookup URL of module which is mapped at the specified function address.

    osl_getModuleURLFromFunctionAddress is an alternative function for osl_getModuleURLFromAddress.
    Use Function pointer as symbol address to conceal type conversion.

    @param[in] pf       function address in oslGenericFunction format.
    @param[out] pustrFunctionURL receives the URL of the module that is mapped at pf.

    @retval sal_True on success
    @retval sal_False no module can be found at the specified function address or parameter is somewhat invalid

    @see osl_getModuleURLFromAddress
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress(
        oslGenericFunction pf, rtl_uString **pustrFunctionURL );

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_MODULE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
