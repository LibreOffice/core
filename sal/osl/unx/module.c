/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-20 10:35:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_TYPES_H_
#include <osl/types.h>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#include "system.h"
#include <stdio.h>

#ifdef MACOSX
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
extern oslProcessError SAL_CALL osl_searchPath(const sal_Char* pszName, const sal_Char* pszPath, sal_Char Separator, sal_Char *pszBuffer, sal_uInt32 Max);
#else /* MACOSX */
extern int _dlclose(void *handle);
#endif /* MACOSX */

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);
void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName);

#if defined(SOLARIS)

#include <sys/utsname.h>

/* osl_getSolarisRelease
 * make a single integer from solaris release string */

sal_Int32 SAL_CALL osl_getSolarisRelease()
{
    struct utsname aRelease;
    static int nRelease = 0;

    if ( nRelease != 0 )
    {
        /* computed once, returned many */
        return nRelease;
    }
    else
    if ( uname( &aRelease ) > -1 )
    {
        int nTokens;
        int nMajor, nMinor, nMMinor;

        /* release will be something like 5.5.1 or 5.6 */
        nTokens = sscanf(aRelease.release, "%i.%i.%i\n",
                    &nMajor, &nMinor, &nMMinor );

        switch ( nTokens )
        {
            case 0: nMajor  = 0;
                    /* fall thru */
            case 1: nMinor  = 0;
                    /* fall thru */
            case 2: nMMinor = 0;
                    break;
            case 3:
            default:
                    /* 3 tokens cannot match more than 3 times */
                    break;
        }

        /* will be something like 551 or 560, dont expect a minor release
         * number larger than 9 */
        nRelease = nMajor * 100 + nMinor * 10 + nMMinor;

        /* okay, the paranoic case */
        if ( nRelease == 0 )
            nRelease = -1;
    }
    else
    {
        /* never saw uname fail, but just in case
         * (must be very old solaris) */
        nRelease = -1;
    }

    return nRelease;
}

#endif

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/
#if defined(SOLARIS)
/*
 *  mfe: #65566# and #68661#
 *       Under 5.5.1 we can't use dlclose due to relocation error with libsk
 *       The First fix was to dlopen all libs with RTLD_GLOBAL but we did
 *       encounter problems with Java then (zip symbols)
 *       The error occurs when an implicit dlclose was called
 *       We catch dlclose here and leave all libs opened via dlopen unclosed
 */

int dlclose(void *handle)
{
    int nRet;
    sal_Int32 release;
    release = osl_getSolarisRelease();
    if ( release >= 560 )
    {
        nRet=_dlclose(handle);
    }
    return nRet;
}
#endif

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    oslModule pModule=0;

    if (  ustrModuleName != 0 )
    {
        rtl_String* strModuleName = 0;
        sal_Char* pszModuleName=0;

        rtl_uString2String( &strModuleName,
                            rtl_uString_getStr(ustrModuleName),
                            rtl_uString_getLength(ustrModuleName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pszModuleName=rtl_string_getStr(strModuleName);

#if defined(DEBUG)
        fprintf(stderr,"lib to load : [%s]\n",pszModuleName);
#endif

        if ( strncmp(pszModuleName,"//./",4) == 0 )
            pszModuleName+=3;

        pModule = osl_psz_loadModule(pszModuleName,nRtldMode);
        rtl_string_release(strModuleName);
    }

    return pModule;
}


oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode)
{
#ifdef MACOSX

    CFStringRef     pPath=0;
    CFURLRef        pURL=0;
    CFBundleRef     pLib=0;
    sal_Char        *searchPath=0;
    sal_Char        path[PATH_MAX + 1];
    sal_Char        *pszModulePath=0;

    OSL_ASSERT(pszModuleName);

    /*
     * Try to construct the absolute path to the module name by searching
     * for the module name in the directories specified in the
     * DYLD_LIBRARY_PATH environment variable.
     */
    if ( osl_searchPath( pszModuleName, "DYLD_LIBRARY_PATH", '\0', path, sizeof(path) ) == osl_Process_E_None )
        pszModulePath = path;

    if ( pszModulePath )
    {
#ifndef NO_DL_FUNCTIONS
        /* Convert path in strModulePath to CFStringRef */
        pPath = CFStringCreateWithCString( NULL, pszModulePath,
            kCFStringEncodingUTF8 );

        /* Get the framework's CFURLRef using its path */
        if ( ! (pURL = CFURLCreateWithFileSystemPath( kCFAllocatorDefault,
            pPath, kCFURLPOSIXPathStyle, true) ) )
        {
#ifdef DEBUG
            fprintf( stderr,
                "osl_loadModule: cannot load module %s for reason: %s\n",
                pszModulePath, "path does not exist" );
#endif
            CFRelease(pPath);
            return NULL;
        }

        /* Load the framework */
        if ( ! ( pLib = CFBundleCreate( NULL, pURL) ) )
        {
#ifdef DEBUG
            fprintf( stderr,
                "osl_loadModule: cannot load module %s for reason: %s\n",
                pszModulePath, "path is not a bundle" );
#endif
            CFRelease( pPath );
            CFRelease( pURL );
            return NULL;
        }

        /* Load the library in the framework */
        if ( CFBundleLoadExecutable( pLib ) )
        {
            oslModule pModule=0;

            CFRelease( pPath );
            CFRelease( pURL );
            pModule = (oslModule)malloc( sizeof( struct _oslModule ) );

            if ( pModule )
            {
                // ignore ".dylib.framework"
                int len = strlen( pszModuleName ) - 16;
                pModule->pModule = pLib;
                pModule->pModuleName = (sal_Char *)malloc( len );
                strncpy( pModule->pModuleName, pszModuleName, len );
                pModule->pModuleName[len] = '\0';
            }

            return pModule;
        }
        else
        {
#ifdef DEBUG
            fprintf( stderr,
                "osl_loadModule: cannot load module %s for reason: %s\n",
                pszModulePath, "bundle does not contain a valid library" );
#endif
            CFRelease( pPath );
            CFRelease( pURL );
            CFRelease( pLib );
            return NULL;
        }

#endif
    }
    return NULL;

#else /* MACOSX */

    sal_Int32 rtld_mode;

    if ( nRtldMode == 0 )
    {
        rtld_mode = RTLD_LAZY;
    }
    else
    {
        rtld_mode=nRtldMode;
    }

    OSL_ASSERT(pszModuleName);

    if (pszModuleName)
    {
#ifndef NO_DL_FUNCTIONS
        void* pLib = dlopen(pszModuleName, rtld_mode );
#ifdef DEBUG
        if( ! pLib )
            fprintf( stderr, "osl_loadModule: cannot load module %s for reason: %s\n",
                     pszModuleName, dlerror() );
#endif
        return ((oslModule)pLib);
#endif
    }
    return NULL;

#endif /* MACOSX */
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule hModule)
{
#ifdef MACOSX

    OSL_ASSERT(hModule);

    if (hModule)
    {
        if ( hModule->pModule )
        {
            CFBundleUnloadExecutable((CFBundleRef)(hModule->pModule));
            CFRelease((CFBundleRef)(hModule->pModule));
        }
        if ( hModule->pModuleName )
            free( hModule->pModuleName );
        free( hModule );
    }

#else /* MACOSX */

    OSL_ASSERT(hModule);

    if (hModule)
    {
#ifndef NO_DL_FUNCTIONS
#ifndef GCC
        /*  gcc (2.9.1 (egcs), 295) registers atexit handlers for
         *  static destructors which obviously cannot
         *  be called after dlclose. A compiler "feature". The workaround for now
         *  is not to dlclose libraries. Since most of them are closed at shutdown
         *  this does not make that much a difference
         */
        dlclose(hModule);

#endif
#endif
    }

#endif /* MACOSX */
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString* ustrSymbolName)
{
    void* pHandle=0;

    if ( ustrSymbolName != 0 )
    {
        rtl_String* strSymbolName=0;
        sal_Char* pszSymbolName=0;

        rtl_uString2String( &strSymbolName,
                            rtl_uString_getStr(ustrSymbolName),
                            rtl_uString_getLength(ustrSymbolName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pszSymbolName = rtl_string_getStr(strSymbolName);


        pHandle=osl_psz_getSymbol(Module,pszSymbolName);

        if ( strSymbolName != 0 )
        {
            rtl_string_release(strSymbolName);
        }
    }


    return pHandle;
}

void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName)
{
#ifdef MACOSX

    CFMutableStringRef pMutSymbolName=0;
    CFStringRef pSymbolName=0;
    void *pSymbol=0;

    OSL_ASSERT(hModule);
    OSL_ASSERT(pszSymbolName);

    if (hModule && hModule->pModule && pszSymbolName)
    {
#ifndef NO_DL_FUNCTIONS
        /* The need to concat the library name and function name is caused
           by a bug in MACOSX's loader */
        if ( hModule->pModuleName )  /* if module name is not null */
        {
            /* Convert char pointer to CFStringRef, make it mutable and
               append the symbol */
            pSymbolName = CFStringCreateWithCString(NULL, hModule->pModuleName, kCFStringEncodingUTF8);
            pMutSymbolName = CFStringCreateMutableCopy(NULL, 0, pSymbolName);
            CFStringAppendCString(pMutSymbolName, pszSymbolName, kCFStringEncodingUTF8);
            /* Try to get the symbol */
            pSymbol = CFBundleGetFunctionPointerForName((CFBundleRef)(hModule->pModule), (CFStringRef)pMutSymbolName);
            /* Release CFStringRef */
            if ( pSymbolName )
                CFRelease(pSymbolName);
            /* Release CFMutableStringRef */
            if ( pMutSymbolName )
                CFRelease(pMutSymbolName);
        }

        /* If a symbol with the module name as a prefix was not found, try to
           load the symbol without any prefix */
        if ( !pSymbol )
        {
            /* Convert char pointer to CFStringRef */
            pSymbolName = CFStringCreateWithCString(NULL, pszSymbolName, kCFStringEncodingUTF8);

            /* Try to get the symbol */
            pSymbol = CFBundleGetFunctionPointerForName((CFBundleRef)(hModule->pModule), pSymbolName);
            /* Release CFStringRef */
            if ( pSymbolName )
                CFRelease(pSymbolName);
        }

        return pSymbol;
#endif
    }
    return NULL;

#else /* MACOSX */

    OSL_ASSERT(hModule);
    OSL_ASSERT(pszSymbolName);

    if (hModule && pszSymbolName)
    {
#ifndef NO_DL_FUNCTIONS
        return dlsym(hModule, pszSymbolName);
#endif
    }
    return NULL;

#endif /* MACOSX */
}

/*****************************************************************************/
/* NYI */
/*****************************************************************************/
#ifdef NOT_IMPL


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef AIX
#include <errno.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include <a.out.h>
#include <ldfcn.h>
#define _H_FCNTL
#define LIBPATH "LIBPATH"
#elif defined HPUX
#include <dl.h>
#include <errno.h>
#define LIBPATH "SHLIB_PATH"
#elif defined S390
#include <errno.h>
#include <dll.h>
#define LIBPATH "LIPPATH"
#else
#include <dlfcn.h>
#define LIBPATH "LD_LIBRARY_PATH"
#endif

#ifdef AIX
struct SymbolEntry
{
    String aSymbolName;
    void *aSymbolAdress;
};
DECLARE_LIST(SymbolTable, SymbolEntry*);
#endif

struct LibraryPrivateData
{
    String aLibname;
    ULONG nRefCount;

#ifdef HPUX
    shl_t aHandle;
#elif S390
    dllhandle *aHandle;
#else
    void *aHandle;
#endif

#ifdef AIX
    SymbolTable aSymbolTable;
#endif

#if defined AIX || defined HPUX
    void (*init)(void);
    void (*exit)(void);
#endif

    LibraryPrivateData( const String & aName )
        : aLibname (aName), nRefCount (1), aHandle (NULL)
#if defined AIX || defined HPUX
            , init (NULL), exit (NULL)
#endif
    {}
};

static String aError;

DECLARE_LIST(LibraryList, LibraryPrivateData*);
static LibraryList aLibraryList;

#if defined AIX || defined HPUX
inline void SetError (const sal_Char* const pError, BOOL bErrno = FALSE)
{
    aError = pError;
    if (bErrno)
    {
        aError += ' ';
        aError += strerror(errno);
    }
    aError += '\n';
#ifdef DBG_UTIL
    DBG_ASSERTWARNING (TRUE, aError.GetStr());
#endif
}
#endif

inline String FindLibrary( const String &rPathname )
{
    if (rPathname.Search('/') != STRING_NOTFOUND)
        return access(rPathname.GetStr(), F_OK | R_OK | X_OK)
                ? String() : (String)rPathname;
    String aLibpath (getenv (LIBPATH));
    String aPathName;
    USHORT nToken = aLibpath.GetTokenCount (':');
    for (USHORT i = 0; i < nToken; i++)
    {
        aPathName = aLibpath.GetToken (i, ':');
        aPathName += '/';
        aPathName += rPathname;
        if (! access(aPathName.GetStr(), F_OK | R_OK | X_OK))
            return aPathName;
    }
    return String();
}

#ifdef HPUX

inline void *FindFunction(LibraryPrivateData *pPriv, const String & rSymbol)
{
    void *pReturn;
    if (shl_findsym( &pPriv->aHandle, rFuncName.GetStr(),
                                        TYPE_PROCEDURE, &pReturn) == -1)
    {
        SetError("FindFunction", TRUE);
        pReturn = NULL;
    }
    return pReturn;
}

inline BOOL Load(LibraryPrivateData *pPriv, const String & rPathname)
{
    pPriv->aHandle = shl_load(rPathname.GetStr(),
                (BIND_DEFERRED | BIND_NONFATAL | DYNAMIC_PATH), 0L);
    BOOL ret = (pPriv->aHandle != NULL);
    if (ret)
    {
        String aSymbol("_GLOBAL__FI_");
        String aSymbol += pPriv->aLibName;
        aSymbol.SearchAndReplace('.', '_');
        pPriv->init = FindFunction(pPriv, Symbol);
        aSymbol.SearchAndReplace('I', 'D');
        pPriv->exit = FindFunction(pPriv, Symbol);
        if (pPriv->init)
            (pPriv->init)();
    }
    else
        SetError("Load:", TRUE);
    return (ret);
}

inline BOOL Unload (LibraryPrivateData *pPriv) {
    BOOL ret = TRUE;
    if (pPriv)
    {
        if (pPriv->exit)
            (pPriv->exit)();
        ret = (shl_unload (pPriv->aHandle) != -1);
        if (! ret)
            SetError("Unload:", TRUE);
    }
    return (ret);
}

#elif defined AIX

inline struct ld_info *GetInfo (void)
{
    for (ULONG size = 4096; TRUE; size += 4096)
    {
        sal_Char *pBuffer = new sal_Char [size];
        if (! pBuffer) {
            SetError("GetInfo: out of memory");
            return NULL;
        }
        if (loadquery (L_GETINFO, pBuffer, size) != -1)
            return ((struct ld_info *) pBuffer);
        delete[] pBuffer;
        if (errno != ENOMEM)
            return NULL;
    }
}

class MainEntry
{
    void *pPoint;
    MainEntry ()
    {
        struct ld_info *pLdInfo = GetInfo ();
        if (pLdInfo)
        {
            pPoint =  pLdInfo->ldinfo_dataorg;
            delete[] pLdInfo;
        }
        else
        {
            pPoint = NULL;
            SetError("MainEntryPoint: cannot find");
        }
    }
}

static MainEntry aMainEntryPoint;

inline void *FindFunction(LibraryPrivateData *pPriv, const String & rSymbol)
{
    SymbolEntry *pSymbolEntry;
    for (pSymbolEntry = pPriv->aSymbolTable.First();
            pSymbolEntry && (pSymbolEntry->aSymbolName != rFuncName);
            pSymbolEntry = pPriv->aSymbolTable.Next())
        ;
    void *ret = pSymbolEntry ? pSymbolEntry->aSymbolAdress : NULL;
    if (ret)
    {
        String aTmp("FindFunction: symbol ");
        aTmp += rFuncName;
        aTmp += " not found";
        SetError(aTmp.GetStr());
    }
    return ret;
}

inline BOOL Load(LibraryPrivateData *pPriv, const String & rPathname)
{
    String aPathName = FindLibrary (rPathname);
    if (!aPathName)
    {
        String aTmp = "Load: ";
        aTmp += rPathname;
        aTmp += " not found";
        SetError (aTmp.GetStr());
        return FALSE;
    }

    pPriv->aHandle = load ((sal_Char*)aPathName.GetStr(), L_NOAUTODEFER, NULL);
    if (! pPriv->aHandle)
    {
        String aTmp = "LOad: load:";
        aTmp += rPathname;
        aTmp += ':';
        SetError (aTmp.GetStr(), TRUE);
        return FALSE;
    }

    if (loadbind(NULL, aMainEntryPoint.aPoint , pPriv->aHandle) == -1)
    {
        unload (pPriv->aHandle);
        SetError("Load: loadbind:", TRUE);
        return FALSE;
    }

    LDFILE *pLdFile = ldopen ((sal_Char*) aPathName.GetStr(), NULL);
    if (! pLdFile)
    {
        struct ld_info *pLdInfo;
        if (!(pLdInfo = GetInfo()));
        {
            SetError("Load: cannot find symbol table");
            return FALSE;
        }
        while (pLdInfo && (! pLdFile))
        {
            if (pLdInfo->ldinfo_dataorg == pPriv->aHandle)
                pLdFile = ldopen (pLdInfo->ldinfo_filename,pLdFile);
            else
                pLdInfo = pLdInfo->ldinfo_next ?
                        (struct ld_info *)((sal_Char *) pLdInfo) +
                                    pLdInfo->ldinfo_next :
                        NULL;
        }
        delete[] pLdInfo;
        if (! pLdFile)
        {
            SetError("Library::Library:");
            return FALSE;
        }
    }
    if (TYPE(pLdFile) != U802TOCMAGIC)
    {
        while (ldclose (pLdFile) == FAILURE)
            ;
        SetError("Library::Library: bad magic");
        return FALSE;
    }

    SCNHDR aSectionHeader;
    if (ldnshread (pLdFile, _LOADER, &aSectionHeader) != SUCCESS)
    {
        while (ldclose (pLdFile) == FAILURE)
            ;
        SetError("Library::Library: cannot read loader section header");
        return FALSE;
    }
    if (FSEEK(pLdFile, aSectionHeader.s_scnptr, BEGINNING) != OKFSEEK) {
        while (ldclose (pLdFile) == FAILURE)
            ;
        SetError("Load: cannot seek to loader section");
        return FALSE;
    }
    sal_Char *pBuffer = new sal_Char [aSectionHeader.s_size];
    if (FREAD(pBuffer, aSectionHeader.s_size, 1, pLdFile) != 1)
    {
        while (ldclose (pLdFile) == FAILURE)
            ;
        SetError("Load: cannot read loader section");
        return FALSE;
    }
    int i = ((LDHDR *)pBuffer)->l_nsyms;
    for (LDSYM *pSymbol = (LDSYM*) (pBuffer + LDHDRSZ); i--; pSymbol++)
    {
        if (LDR_EXPORT (*pSymbol))
        {
            SymbolEntry *pSymbolEntry = new SymbolEntry;
            pSymbolEntry->aSymbolName = pSymbol->l_zeroes ?
                pSymbol->l_name :
                pSymbol->l_offset + ((LDHDR *)pBuffer)->l_stoff + pBuffer;
            pSymbolEntry->aSymbolAdress = (void *)((sal_uInt32)
                                pPriv->aHandle + pSymbol->l_value);
            pPriv->aSymbolTable.Insert(pSymbolEntry);
        }
    }
    delete[] pBuffer;
    while(ldclose(pLdFile) == FAILURE)
        ;

    struct dl_info { void (*init)(void); void (*exit)(void); }
            *pInfo = (struct dl_info *) FindFunction ("dl_info");
    if (pInfo)
    {
        pPriv->exit = pInfo->exit;
        pPriv->init = pInfo->init;
        if (pPriv->init)
            pPriv->init();
    }
    return TRUE;
}

inline BOOL Unload (LibraryPrivateData *pPriv)
{
    if (pPriv->exit)
        pPriv->exit();
    if (unload (pPriv->aHandle) == -1)
        SetError("Unload", TRUE);
    for (SymbolEntry *pSymbolEntry = pPriv->aSymbolTable.First();
                pSymbolEntry; pSymbolEntry = pPriv->aSymbolTable.Next())
        delete pSymbolEntry;
}

#elif S390

inline void *FindFunction(LibraryPrivateData *pPriv, const String & rFuncname)
{
    void *pReturn = (void *)(dllqueryfn(pPriv->aHandle, (sal_Char*) rFuncname.GetStr()));
    if (pReturn == NULL)
        aError = strerror(errno);
    return pReturn;
}

inline BOOL Load(LibraryPrivateData *pPriv, const String & rPathname)
{
    pPriv->aHandle = dllload((sal_Char *) rPathname.GetStr());
    if (pPriv->aHandle == NULL)
        aError = strerror(errno);
    return (pPriv->aHandle != NULL);
}

inline BOOL Unload (LibraryPrivateData *pPriv)
{
    BOOL bRet = (dllfree(pPriv->aHandle) == 0);
    if (!bRet)
        aError = strerror(errno);
    return bRet;
}

#else

inline void *FindFunction(LibraryPrivateData *pPriv, const String & rFuncname)
{
    void *pReturn = dlsym(pPriv->aHandle, rFuncname.GetStr());
    aError = dlerror();
    return pReturn;
}

inline BOOL Load(LibraryPrivateData *pPriv, const String & rPathname)
{
    pPriv->aHandle = dlopen((sal_Char *) rPathname.GetStr(), RTLD_LAZY);
    aError = dlerror();
    return (pPriv->aHandle != NULL);
}

inline BOOL Unload (LibraryPrivateData *pPriv)
{
#ifndef LINUX
    BOOL ret = (dlclose(pPriv->aHandle) != 0);
    aError = dlerror();
    return ret;
#else
    /* gcc 2.9.1 (egcs) registers atexit handlers which obviously cannot
     * be called after dlclose. A compiler "feature". The workaround for now
     * is not to dlclose libraries. Since most of them are closed at shutdown
     * this does not make that much a difference
     */
     return TRUE;
#endif
}

#endif

BOOL Library::ModulFound( const String &rPathname )
{
    return FindLibrary (rPathname).Len();
}

Library::Library(const String & rPathname)
{
    sal_Char *pLibname = strrchr (rPathname.GetStr(), '/');
    String aLibname(pLibname ? String(pLibname) : (String)rPathname);

    for (pPriv = aLibraryList.First();
                pPriv && (pPriv->aLibname != aLibname);
                pPriv = aLibraryList.Next())
        ;
    if (pPriv)
    {
        pPriv->nRefCount++;
        return;
    }
    pPriv = new LibraryPrivateData (aLibname);
    if (Load(pPriv, rPathname))
        aLibraryList.Insert(pPriv);
    else
    {
        delete pPriv;
        pPriv = NULL;
    }
}

Library::~Library()
{
    if (pPriv && --pPriv->nRefCount)
    {
        Unload (pPriv);
        delete pPriv;
        pPriv = NULL;
        aLibraryList.Remove(pPriv);
    }
}

void* Library::GetFunction(const String & rFuncName) const
{
    return pPriv ? FindFunction(pPriv, rFuncName) : NULL;
}

BOOL Library::ModulFound () const {
    return ((pPriv != NULL) && (pPriv->aHandle != NULL));
}

#endif

