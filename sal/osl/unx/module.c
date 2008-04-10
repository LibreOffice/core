/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: module.c,v $
 * $Revision: 1.39 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/module.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.h>

#ifdef IRIX
#ifndef _RLD_INTERFACE_DLFCN_H_DLADDR
#define _RLD_INTERFACE_DLFCN_H_DLADDR
typedef struct DL_INFO {
       const char * dli_fname;
       void       * dli_fbase;
       const char * dli_sname;
       void       * dli_saddr;
       int          dli_version;
       int          dli_reserved1;
       long         dli_reserved[4];
} Dl_info;
#endif
#include <rld_interface.h>
#define _RLD_DLADDR             14
int dladdr(void *address, Dl_info *dl);

int dladdr(void *address, Dl_info *dl)
{
       void *v;
       v = _rld_new_interface(_RLD_DLADDR,address,dl);

       return (int)v;
}
#endif

#include "system.h"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

/* implemented in file.c */
extern int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);

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
            pModule = osl_psz_loadModule(buffer, nRtldMode);
        rtl_uString_release(ustrTmp);
    }

    return pModule;
}

/*****************************************************************************/
/* osl_psz_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode)
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
            OSL_TRACE("Error osl_loadModule: %s\n", dlerror());
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
#ifndef GCC
        /*     gcc (2.9.1 (egcs), 295) registers atexit handlers for
         *     static destructors which obviously cannot
         *     be called after dlclose. A compiler "feature". The workaround for now
         *     is not to dlclose libraries. Since most of them are closed at shutdown
         *     this does not make that much a difference
         */
        int nRet = dlclose(hModule);

#if OSL_DEBUG_LEVEL > 1
        if (nRet != 0)
        {
            fprintf(stderr, "Error osl_unlocadModule: %s\n", dlerror());
        }
#endif /* if OSL_DEBUG_LEVEL */
#endif /* ifndef GCC */

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
            OSL_TRACE("Error osl_getAsciiFunctionSymbol: %s\n", dlerror());
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
