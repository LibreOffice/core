/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <config_folders.h>

#include "sal/config.h"

#include "nsp_func.hxx"

#ifdef UNIX
#include <sys/types.h>
#include <strings.h>
#ifdef LINUX
#include <dlfcn.h>
#endif
#include <stdarg.h>
#endif // End UNIX

#ifdef WNT

#ifdef _MSC_VER
#pragma warning (push,1)
#pragma warning (disable:4668)
#pragma warning (disable:4917)
#endif

#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <shlobj.h>
#include <objidl.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif
#endif // End WNT

#include <sys/stat.h>
#include <errno.h>
#include "so_env.hxx"
#include "ns_debug.hxx"
#include <sal/config.h>

// Tranform all strings like %20 in pPath to one char like space
/*int retoreUTF8(char* pPath)
{
    // Prepare buf
    int len = strlen(pPath) + 1;
    char* pBuf = (char*)malloc(len);
    memset(pBuf, 0, len);

    // Store the original pBuf and pPath
    char* pBufCur = pBuf;
    char* pPathCur = pPath;
    // ie, for %20, UTF8Numbers[0][0] = 2, UTF8Numbers[1][0] = 0
    char UTF8Numbers[2][2] = {{0, 0}, {0,0}};
    int temp;

    while (*pPathCur) {
        if (('%' == *pPathCur) && (0 != *(pPathCur + 1))
            && (0 != *(pPathCur + 2)))
        {
            UTF8Numbers[0][0] = *(pPathCur + 1);
            UTF8Numbers[1][0] = *(pPathCur + 2);
            temp = 0;
            temp = atoi(UTF8Numbers[0])*16 + atoi(UTF8Numbers[1]);
            *pBufCur = (char)temp;
            pBufCur++;
            pPathCur += 3;
        } else {
            *pBufCur++ = *pPathCur++;
        }
    }

    *pBufCur = 0;
    strcpy(pPath, pBuf);
    free(pBuf);
    return 0;
}*/

int
restoreUTF8(char *pPath)
{
    unsigned char *s, *d;

#define XDIGIT(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)

    s = d = (unsigned char *)pPath;
    do {
        if (*s == '%' && s[1] && s[2]) {
            *d++ = (XDIGIT (s[1]) << 4) + XDIGIT (s[2]);
            s += 2;
        } else
            *d++ = *s;
    } while (*s++);
    debug_fprintf(NSP_LOG_APPEND, "after restoreUTF8, pPath is %s\n", pPath);
    return 0;
}

#ifdef LINUX
extern int nspluginOOoModuleHook (void** aResult);
int nspluginOOoModuleHook (void** aResult)
{
    void *dl_handle;

    dl_handle = dlopen(NULL, RTLD_NOW);
    if (!dl_handle)
    {
        fprintf (stderr, "Can't open myself '%s'\n", dlerror());
        return 1;
    }
    dlclose(dl_handle);

    Dl_info dl_info = { 0,0,0,0 };
    if(!dladdr((void *)nspluginOOoModuleHook, &dl_info))
    {
        fprintf (stderr, "Can't find my own address '%s'\n", dlerror());
        return 1;
    }

    if (!dl_info.dli_fname)
    {
        fprintf (stderr, "Can't find my own file name\n");
        return 1;
    }

    char cwdstr[NPP_PATH_MAX];
    if (!getcwd (cwdstr, sizeof(cwdstr)))
    {
        fprintf (stderr, "Can't get cwd\n");
        return 1;
    }

    char libFileName[NPP_PATH_MAX];

    if (dl_info.dli_fname[0] != '/')
    {
        if ((strlen(cwdstr) + 1 + strlen(dl_info.dli_fname)) >= NPP_PATH_MAX)
        {
            fprintf (stderr, "Plugin path too long\n");
            return 1;
        }
        strcpy (libFileName, cwdstr);
        strcat (libFileName, "/");
        strcat (libFileName, dl_info.dli_fname);
    }
    else
    {
        if (strlen(dl_info.dli_fname) >= NPP_PATH_MAX)
        {
            fprintf (stderr, "Plugin path too long\n");
            return 1;
        }
        strcpy (libFileName, dl_info.dli_fname);
    }

    char *clobber;
    static char realFileName[NPP_PATH_MAX] = {0};
#   define SEARCH_SUFFIX "/" LIBO_LIB_FOLDER "/libnpsoplug"

    if (!(clobber = strstr (libFileName, SEARCH_SUFFIX)))
    {
        ssize_t len = readlink(libFileName, realFileName, NPP_PATH_MAX-1);
        if (len == -1)
        {
            fprintf (stderr, "Couldn't read link '%s'\n", libFileName);
            return 1;
        }
        realFileName[len] = '\0';
        if (!(clobber = strstr (realFileName, SEARCH_SUFFIX)))
        {
                fprintf (stderr, "Couldn't find suffix in '%s'\n", realFileName);
            return 1;
        }
        *clobber = '\0';
    }
    else
    {
        *clobber = '\0';
        strcpy (realFileName, libFileName);
    }

#if OSL_DEBUG_LEVEL > 0
    fprintf (stderr, "LibreOffice path before fixup is '%s'\n", realFileName);
#endif

    if (realFileName[0] != '/') {
        /* a relative sym-link and we need to get an absolute path */
        char scratch[NPP_PATH_MAX] = {0};
        if (strlen (realFileName) + strlen (libFileName) + 2 >= NPP_PATH_MAX - 1)
        {
            fprintf (stderr, "Paths too long to fix up.\n");
            return 1;
        }
        strcpy (scratch, libFileName);
        if (strrchr (scratch, '/')) /* remove the last element */
            *(strrchr (scratch, '/') + 1) = '\0';
        strcat (scratch, realFileName);
        strcpy (realFileName, scratch);
    }

    *aResult = realFileName;

#if OSL_DEBUG_LEVEL > 0
    fprintf (stderr, "LibreOffice path is '%s'\n", realFileName);
#endif

    return 0;
}
#endif

// *aResult points the static string holding "/opt/staroffice8"
int findReadSversion(void** aResult, int /*bWnt*/, const char* /*tag*/, const char* /*entry*/)
{
#ifdef UNIX
    // The real space to hold "/opt/staroffice8"
    static char realFileName[NPP_PATH_MAX] = {0};
    memset(realFileName, 0, NPP_PATH_MAX);
    *aResult = realFileName;

    // Filename of lnk file, eg. "soffice"
    char lnkFileName[NPP_PATH_MAX] = {0};
    char* pTempZero = NULL;

#ifdef LINUX
    /* try to fetch a 'self' pointer */
    if (!nspluginOOoModuleHook (aResult))
      return 0;

    /* .. now in $HOME */
#endif // LINUX
    snprintf(lnkFileName, NPP_PATH_MAX - 1, "%s/.mozilla/plugins/libnpsoplugin%s", getenv("HOME"), SAL_DLLEXTENSION);
#ifdef LINUX
    ssize_t len = readlink(lnkFileName, realFileName, NPP_PATH_MAX-1);
    if (-1 == len)
    {
        *realFileName = 0;
        return -1;
    }
    realFileName[len] = '\0';

    if (NULL == (pTempZero = strstr(realFileName, "/" LIBO_LIB_FOLDER "/libnpsoplugin" SAL_DLLEXTENSION)))
#else  // LINUX
    if ((0 > readlink(lnkFileName, realFileName, NPP_PATH_MAX)) ||
        (NULL == (pTempZero = strstr(realFileName, "/" LIBO_LIB_FOLDER "/libnpsoplugin" SAL_DLLEXTENSION))))
#endif // LINUX
    {
        *realFileName = 0;
        return -1;
    }
    *pTempZero = 0;
    return 0;
#elif defined WNT // UNIX
    static char realFileName[NPP_PATH_MAX] = {0};
    *aResult = realFileName;
    HKEY hKey;
    DWORD dwBufLen = NPP_PATH_MAX;
    LONG lRet;

    debug_fprintf(NSP_LOG_APPEND, "1 before before strstr realFileName is %s\n", realFileName);
    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
       "SOFTWARE\\MozillaPlugins\\@sun.com/npsopluginmi;version=1.0",
       0, KEY_QUERY_VALUE, &hKey );
    if (lRet == ERROR_FILE_NOT_FOUND) {
        lRet = RegOpenKeyEx(
            HKEY_CURRENT_USER,
            "SOFTWARE\\MozillaPlugins\\@sun.com/npsopluginmi;version=1.0", 0,
            KEY_QUERY_VALUE, &hKey);
    }
    debug_fprintf(NSP_LOG_APPEND, "2 before before strstr realFileName is %s\n", realFileName);
    if( lRet != ERROR_SUCCESS )
       return FALSE;

    lRet = RegQueryValueEx( hKey, "Path", NULL, NULL,
       (LPBYTE) realFileName, &dwBufLen);
    debug_fprintf(NSP_LOG_APPEND, "3 before before strstr realFileName is %s\n", realFileName);
    if( (lRet != ERROR_SUCCESS) || (dwBufLen > NPP_PATH_MAX) )
       return FALSE;

    RegCloseKey( hKey );
    char* pTempZero = NULL;
    debug_fprintf(NSP_LOG_APPEND, "before strstr realFileName is %s\n", realFileName);
    if (NULL == (pTempZero = strstr(realFileName, "\\program")))
    {
        *realFileName = 0;
        return -1;
    }
    *pTempZero = 0;
    debug_fprintf(NSP_LOG_APPEND, "realFileName is %s\n", realFileName);
    return 0;
#endif // UNIX
}

// Return the install dir path of staroffice, return value like "/home/build/staroffice"
const char* findInstallDir()
{
    static char* pInstall = NULL;
    debug_fprintf(NSP_LOG_APPEND, "start of findInstallDir()\n");
    if (!pInstall)
    {
        findReadSversion((void**)&pInstall, 0, "[" SECTION_NAME "]", SOFFICE_VERSION "=");
        if (!pInstall)
            pInstall = const_cast< char* >( "" );
    }
    return pInstall;
}

// Return the program dir path of staroffice, return value like "/home/build/staroffice/program"
const char* findProgramDir()
{
    static char sProgram[NPP_BUFFER_SIZE] = {0};
    if (!sProgram[0])
    {
        sprintf(sProgram, "%s/" LIBO_BIN_FOLDER, findInstallDir());
#ifdef WNT
        UnixToDosPath(sProgram);
#endif
    }
    return sProgram;
}

#ifdef WNT
// Return SO executable absolute path, like "/home/build/staroffice/program/soffice"
const char* findSofficeExecutable()
{
    static char pSofficeExeccutable[NPP_PATH_MAX] = {0};
    if (!pSofficeExeccutable[0])
    {
        sprintf(pSofficeExeccutable, "%s/%s", findProgramDir(), STAROFFICE_EXE_FILE_NAME);
#ifdef WNT
        UnixToDosPath(pSofficeExeccutable);
#endif
    }

    return pSofficeExeccutable;
}

// Change Dos path such as c:\program\soffice to c:/program/soffice
int DosToUnixPath(char* sPath)
{
    if (!sPath)
        return -1;
    char* p = sPath;
    while (*p)
    {
        if(*p == '\\')
            *p = '/';
        p++;
    }
    return 0;

}
#endif
// Change Unix path such as program/soffice to program\soffice
int UnixToDosPath(char* sPath)
{
    if (!sPath)
        return -1;
    char* p = sPath;
    while (*p)
    {
        if(*p == '/')
            *p = '\\';
        p++;
    }
    return 0;

}

#ifdef UNIX
#define PLUGIN_NAME "LibreOffice"
char productName[128] = {0};
char* NSP_getProductName()
{
    if(productName[0])
        return productName;
    char fullBootstrapIniPath[1024] = {0};
    const char* pFullFilePath = findProgramDir();
    if(0 == *pFullFilePath)
    {
        strcpy(productName, PLUGIN_NAME);
        return productName;
    }
    sprintf(fullBootstrapIniPath, "%s/%s", pFullFilePath,
        "bootstraprc");

    FILE* fp = fopen(fullBootstrapIniPath, "r");

    if (NULL == fp)
    {
        strcpy(productName, PLUGIN_NAME);
        return productName;
    }
    char line[4096] = {0};
    char *pStart = 0;
    char *pEnd = 0;
    while(!feof(fp))
    {
        if (fgets( line, sizeof(line), fp ) == NULL)
            continue;
        if (NULL == (pStart = strstr( line, "ProductKey=" )))
            continue;
        pStart += strlen("ProductKey=");
        if (NULL == (pEnd = strchr( pStart, ' ' )) &&
           (NULL == (pEnd = strchr( pStart, '\r' ))))
            continue;
        *pEnd = 0;
        if (static_cast<size_t>(pEnd - pStart) <= sizeof(productName))
            strcpy(productName, pStart);
    }
    fclose(fp);
    if ((*productName == 0) ||
        (0 != STRNICMP(productName, "StarOffice", sizeof("StarOffice"))))
    {
        strcpy(productName, PLUGIN_NAME);
        return productName;
    }
    memset(productName, 0, sizeof(productName));
    strcat(productName, "StarOffice/StarSuite");
    return productName;
}

char PluginName[1024] = {0};
char* NSP_getPluginName()
{
    if(*PluginName)
        return PluginName;
    sprintf(PluginName, "%s Plug-in", NSP_getProductName());
    return PluginName;
}

char PluginDesc[1024] = {0};
char* NSP_getPluginDesc()
{
    if(*PluginDesc)
        return PluginDesc;

    sprintf(PluginDesc, "%s Plug-in handles all its documents",
        productName);
    return PluginDesc;
}
#endif //end of UNIX

void NSP_WriteLog(int level,  const char* pFormat, ...)
{
    (void)level;
#ifndef DEBUG
    (void)pFormat;
#else
    va_list      ap;
    char         msgBuf[NPP_BUFFER_SIZE];
    static char  logName[NPP_PATH_MAX] = {0};
    FILE *       fp = NULL;

    va_start(ap,pFormat);
    NSP_vsnprintf(msgBuf, NPP_BUFFER_SIZE, pFormat, ap);
    va_end(ap);

    if (!logName[0])
    {
#ifdef UNIX
        const char* homeDir = getenv("HOME");
        sprintf(logName,"%s/%s",homeDir,"nsplugin.log");
#endif // End UNIX
#ifdef WNT
        char szPath[MAX_PATH];
        if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, 0))
        {
             return;
        }
        char* homeDir = szPath;
        sprintf(logName,"%s\\%s", szPath, "nsplugin.log");
#endif // End WNT
    }
    else
        fp = fopen(logName, "a+");

    if (!fp)
        return;
    fputs(msgBuf, fp);
    fclose(fp);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
