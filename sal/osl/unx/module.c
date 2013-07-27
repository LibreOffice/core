/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/module.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.h>

#include "system.h"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

/* implemented in file.c */
extern int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    oslModule pModule=0;
    rtl_uString* ustrTmp = NULL;

    OSL_ENSURE(ustrModuleName,"osl_loadModule : string is not valid");

    /* ensure ustrTmp hold valid string */
    if (osl_File_E_None != osl_getSystemPathFromFileURL(ustrModuleName, &ustrTmp))
        rtl_uString_assign(&ustrTmp, ustrModuleName);

    if (ustrTmp)
    {
        char buffer[PATH_MAX];

        if (UnicodeToText(buffer, PATH_MAX, ustrTmp->buffer, ustrTmp->length))
            pModule = osl_loadAsciiModule(buffer, nRtldMode);
        rtl_uString_release(ustrTmp);
    }

    return pModule;
}

/*****************************************************************************/
/* osl_loadAsciiModule */
/*****************************************************************************/

oslModule SAL_CALL osl_loadAsciiModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode)
{
    OSL_ASSERT(
        (nRtldMode & SAL_LOADMODULE_LAZY) == 0 ||
        (nRtldMode & SAL_LOADMODULE_NOW) == 0); /* only either LAZY or NOW */
    if (pszModuleName)
    {
#ifndef NO_DL_FUNCTIONS
        int rtld_mode =
            ((nRtldMode & SAL_LOADMODULE_NOW) ? RTLD_NOW : RTLD_LAZY) |
            ((nRtldMode & SAL_LOADMODULE_GLOBAL) ? RTLD_GLOBAL : RTLD_LOCAL);
        void* pLib = dlopen(pszModuleName, rtld_mode);

#if OSL_DEBUG_LEVEL > 1
        if (pLib == 0)
            OSL_TRACE("error: osl_loadModule failed with %s\n", dlerror());
#endif /* OSL_DEBUG_LEVEL */

        return ((oslModule)(pLib));

#else   /* NO_DL_FUNCTIONS */
        printf("No DL Functions\n");
#endif  /* NO_DL_FUNCTIONS */
    }
    return NULL;
}

/*****************************************************************************/
/* osl_getModuleHandle */
/*****************************************************************************/

sal_Bool SAL_CALL
osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult)
{
    (void) pModuleName; /* avoid warning about unused parameter */
    *pResult = (oslModule) RTLD_DEFAULT;
    return sal_True;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule hModule)
{
    if (hModule)
    {
#ifndef NO_DL_FUNCTIONS
        int nRet = dlclose(hModule);

#if OSL_DEBUG_LEVEL > 1
        if (nRet != 0)
        {
            fprintf(stderr, "error: osl_unloadModule failed with %s\n", dlerror());
        }
#else
        (void) nRet;
#endif /* if OSL_DEBUG_LEVEL */

#endif /* ifndef NO_DL_FUNCTIONS */
    }
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL
osl_getSymbol(oslModule Module, rtl_uString* pSymbolName)
{
    return (void *) osl_getFunctionSymbol(Module, pSymbolName);
}


/*****************************************************************************/
/* osl_getAsciiFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getAsciiFunctionSymbol(oslModule Module, const sal_Char *pSymbol)
{
    void *fcnAddr = NULL;

#ifndef NO_DL_FUNCTIONS
    if (pSymbol)
    {
        fcnAddr = dlsym(Module, pSymbol);

        if (!fcnAddr)
            OSL_TRACE("error: osl_getAsciiFunctionSymbol failed with %s\n", dlerror());
    }
#endif

    return (oslGenericFunction) fcnAddr;
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getFunctionSymbol(oslModule module, rtl_uString *puFunctionSymbolName)
{
    oslGenericFunction pSymbol = NULL;

    if( puFunctionSymbolName )
    {
        rtl_String* pSymbolName = NULL;

        rtl_uString2String( &pSymbolName,
            rtl_uString_getStr(puFunctionSymbolName),
            rtl_uString_getLength(puFunctionSymbolName),
            RTL_TEXTENCODING_UTF8,
            OUSTRING_TO_OSTRING_CVTFLAGS );

        if( pSymbolName != NULL )
        {
            pSymbol = osl_getAsciiFunctionSymbol(module, rtl_string_getStr(pSymbolName));
            rtl_string_release(pSymbolName);
        }
    }

    return pSymbol;
}

/*****************************************************************************/
/* osl_getModuleURLFromAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromAddress(void * addr, rtl_uString ** ppLibraryUrl)
{
    sal_Bool result = sal_False;
    Dl_info dl_info;

    if ((result = dladdr(addr, &dl_info)) != 0)
    {
        rtl_uString * workDir = NULL;
        osl_getProcessWorkingDir(&workDir);
        if (workDir)
        {
#if OSL_DEBUG_LEVEL > 1
            OSL_TRACE("module.c::osl_getModuleURLFromAddress - %s\n", dl_info.dli_fname);
#endif
            rtl_string2UString(ppLibraryUrl,
                               dl_info.dli_fname,
                               strlen(dl_info.dli_fname),
                               osl_getThreadTextEncoding(),
                               OSTRING_TO_OUSTRING_CVTFLAGS);

            OSL_ASSERT(*ppLibraryUrl != NULL);
            osl_getFileURLFromSystemPath(*ppLibraryUrl, ppLibraryUrl);
            osl_getAbsoluteFileURL(workDir, *ppLibraryUrl, ppLibraryUrl);

            rtl_uString_release(workDir);
            result = sal_True;
        }
        else
        {
            result = sal_False;
        }
    }
    return result;
}

/*****************************************************************************/
/* osl_getModuleURLFromFunctionAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress(oslGenericFunction addr, rtl_uString ** ppLibraryUrl)
{
    return osl_getModuleURLFromAddress((void*)addr, ppLibraryUrl);
}

