/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-29 08:32:35 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

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

#ifdef MACOSX
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#include <mach-o/dyld.h>
extern oslProcessError SAL_CALL osl_searchPath(const sal_Char* pszName, const sal_Char* pszPath, sal_Char Separator, sal_Char *pszBuffer, sal_uInt32 Max);
#else /* MACOSX */
extern int _dlclose(void *handle);
#endif /* MACOSX */

/* implemented in file.c */
extern int UnicodeToText( char *, size_t, const sal_Unicode *, sal_Int32 );

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);
void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName);


/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    oslModule pModule=0;
    rtl_uString* ustrTmp = NULL;

    OSL_ENSURE(ustrModuleName,"osl_loadModule : string is not valid");

    /* ensure ustrTmp hold valid string */
    if( osl_File_E_None != osl_getSystemPathFromFileURL( ustrModuleName, &ustrTmp ) )
        rtl_uString_assign( &ustrTmp, ustrModuleName );

    if( ustrTmp )
    {
        char buffer[PATH_MAX];

        if( UnicodeToText( buffer, PATH_MAX, ustrTmp->buffer, ustrTmp->length ) )
            pModule = osl_psz_loadModule( buffer, nRtldMode );
    }

    rtl_uString_release( ustrTmp );

    return pModule;
}

/*****************************************************************************/
/* osl_psz_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode)
{
#ifdef MACOSX
        // GrP use dyld APIs so dylibs don't have to be in framework bundles

        int                     len;
        const struct mach_header      *pLib = NULL;
        oslModule               pModule;
        sal_Char        buf[PATH_MAX + 1];

    OSL_ASSERT(pszModuleName);

    if ( ! pszModuleName || *pszModuleName == '\0' )
        return NULL;

#ifndef NO_DL_FUNCTIONS

        // Check if module is already loaded
        strncpy(buf, pszModuleName, sizeof(buf));
        buf[sizeof(buf)-1] = '\0';
        pLib = NSAddImage(buf, NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED |
                          NSADDIMAGE_OPTION_RETURN_ON_ERROR);

        if (!pLib) {
                // Module not already loaded. Try to load the module using
                // the name as given (search includes DYLD_LIBRARY_PATH)
                strncpy(buf, pszModuleName, sizeof(buf));
                buf[sizeof(buf)-1] = '\0';
                pLib = NSAddImage(buf, NSADDIMAGE_OPTION_WITH_SEARCHING |
                                  NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        }

        if (!pLib  &&  pszModuleName[0] != '/') {
                // Didn't find module in DYLD_LIBRARY_PATH. Try looking
                // in application's bundle.
                // But don't bother if the name is an absolute path.
                strncpy(buf, "@executable_path/", sizeof(buf));
                strncat(buf, pszModuleName, sizeof(buf) - strlen(buf));
                buf[sizeof(buf)-1] = '\0';
                pLib = NSAddImage(buf, NSADDIMAGE_OPTION_WITH_SEARCHING |
                                  NSADDIMAGE_OPTION_RETURN_ON_ERROR);
        }

        if (!pLib) {
                // Still couldn't find it - give up
#if OSL_DEBUG_LEVEL > 1
                // fixme use NSLinkEditError() to get a better error message
                fprintf( stderr,
                         "osl_loadModule: cannot load module %s for reason: %s\n",
                         pszModuleName, "file does not exist or is not a library (tried DYLD_LIBRARY_PATH and @executable_path)" );
#endif
                return NULL;
        }

        pModule = (oslModule)malloc( sizeof( struct _oslModule ) );
        if (!pModule) {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr,
                         "osl_loadModule: cannot load module %s for reason: %s\n",
                         pszModuleName, "out of memory!" );
#endif
                return NULL;
        }

        // Calculate module name = file name without trailing .dylib*
        for ( len = strlen(pszModuleName) - 6; len > 0 ; len-- )
        {
                if ( !strncmp( pszModuleName + len, ".dylib", 6 ) )
                        break;
        }
        pModule->pModuleName = (sal_Char *)malloc( len + 1 );
        strncpy( pModule->pModuleName, pszModuleName, len + 1 );
        pModule->pModuleName[len] = '\0';
        pModule->pModule = pLib;
        return pModule;

#else  /* NO_DL_FUNCTIONS */
        fprintf( stderr,
                 "osl_loadModule: cannot load module %s for reason: %s\n",
                 pszModuleName, "not compiled with dynamic library support" );
        return NULL;
#endif  /* NO_DL_FUNCTIONS */



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
        if ((pLib == 0) && (strchr (pszModuleName, '/') == 0))
        {
            /* module w/o pathname not found, try cwd as last chance */
            char buffer[PATH_MAX];

            buffer[0] = '.', buffer[1] = '/', buffer[2] = '\0';
            strncat (buffer, pszModuleName, sizeof(buffer) - 2);

            pLib = dlopen(buffer, rtld_mode);
        }

#if OSL_DEBUG_LEVEL > 1
        if (pLib == 0)
        {
            /* module not found, give up */
            fprintf (stderr,
                     "osl_loadModule: cannot load module %s for reason: %s\n",
                     pszModuleName, dlerror());
        }
#endif  /* OSL_DEBUG_LEVEL */

        return ((oslModule)(pLib));

#else   /* NO_DL_FUNCTIONS */
        printf("No DL Functions\n");
#endif  /* NO_DL_FUNCTIONS */
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

        // Unloading dylibs is not yet supported on Mac OS X.

#else /* MACOSX */

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

        int nRet = 0;

        nRet = dlclose(hModule);
#if OSL_DEBUG_LEVEL > 1
        if ( nRet != 0 )
        {
            fprintf( stderr, "osl_getsymbol: cannot close lib for reason: %s\n", dlerror() );
        }
#endif /* if OSL_DEBUG_LEVEL */
#endif /* ifndef GCC */

#endif /* ifndef NO_DL_FUNCTIONS */
    }

#endif /* MACOSX */
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString* ustrSymbolName)
{
    void* pHandle=0;

    OSL_ENSURE(Module,"osl_getSymbol : module handle is not valid");
    OSL_ENSURE(Module,"osl_getSymbol : ustrSymbolName");

    if ( Module!= 0 && ustrSymbolName != 0 )
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

    OSL_ASSERT(hModule);
    OSL_ASSERT(pszSymbolName);

    if (hModule && hModule->pModule && pszSymbolName)
    {
#ifndef NO_DL_FUNCTIONS
                struct mach_header *pLib;
                NSSymbol pSymbol;

                // Prefix symbol name with '_'
                char *name = malloc(1+strlen(pszSymbolName)+1);
                strcpy(name, "_");
                strcat(name, pszSymbolName);

                pLib = (struct mach_header *)hModule->pModule;
                // fixme need to BIND_FULLY or BIND_NOW?
                pSymbol = NSLookupSymbolInImage(pLib, name, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND);
                free(name);
                if (pSymbol) {
                        return NSAddressOfSymbol(pSymbol);
                }
#endif
    }
    return NULL;

#else /* MACOSX */

    OSL_ASSERT(hModule);
    OSL_ASSERT(pszSymbolName);

    if (hModule && pszSymbolName)
    {
#ifndef NO_DL_FUNCTIONS
        void* pSym = 0;

        pSym = dlsym(hModule, pszSymbolName);

#if OSL_DEBUG_LEVEL > 1
        if( ! pSym )
            fprintf( stderr, "osl_getsymbol: cannot get Symbol %s for reason: %s\n",
                     pszSymbolName, dlerror() );
        else
            fprintf( stderr, "osl_getsymbol: got Symbol %s \n", pszSymbolName);
#endif

        return pSym;

#endif
    }
    return NULL;

#endif /* MACOSX */
}

sal_Bool SAL_CALL osl_getModuleURLFromAddress(void * addr, rtl_uString ** ppLibraryUrl)
{
    sal_Bool result = sal_False;
#ifdef MACOSX
    struct mach_header      *imageMachHeader;
    struct load_command     *loadCmd;
    struct segment_command  *segCmd;
    unsigned long           imageIndex;
    unsigned long           numLoadedImages;
    unsigned long           mhCmdIndex;
    unsigned long           imageVMAddressSlide;
    unsigned long           imageLowAddress;
    unsigned long           imageHighAddress;
    unsigned char           *imageName = NULL;

    /* Run through all loaded images in the process' address space and
     * test each segment of each image for the address we want.
     * NOTE:  This simply checks to see if the address is in the image's
     *        address space.  That doesn't mean the symbol the address
     *        is associated with is actually bound yet.
     */
    numLoadedImages = _dyld_image_count();
    for( imageIndex = 0; ((imageIndex < numLoadedImages) && (result==sal_False)); imageIndex++ )
    {
        imageMachHeader = _dyld_get_image_header( imageIndex );
        if ( imageMachHeader != NULL )
        {
            /* Get all the load commands from the image, loop through them, and test
             * the segment they load for the address we were passed.
             */
            imageVMAddressSlide = _dyld_get_image_vmaddr_slide( imageIndex );

            loadCmd = (struct load_command *)((char *)imageMachHeader + sizeof(struct mach_header));
            for ( mhCmdIndex = 0; mhCmdIndex < imageMachHeader->ncmds; mhCmdIndex++ )
            {
                if ( loadCmd->cmd == LC_SEGMENT )
                {
                    segCmd = (struct segment_command *)loadCmd;
                    imageLowAddress = segCmd->vmaddr + imageVMAddressSlide;
                    imageHighAddress = segCmd->vmaddr + segCmd->vmsize + imageVMAddressSlide;
                    if( (((unsigned long)(addr))>=imageLowAddress) && (((unsigned long)(addr))<imageHighAddress) )
                    {
                        /* Address passed in is contained in this image. */
                        imageName = _dyld_get_image_name( imageIndex );
                        result = sal_True;
                        break;
                    }
                }
                loadCmd = (struct load_command *)((char *)loadCmd + loadCmd->cmdsize);
            }
        }
        else
        {
            /* Bad index was passed to _dyld_get_image_header() or the image
             * doesn't exist.
             */
            #if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "osl_getModuleURLFromAddress(): bad index passed to _dyld_get_image_header(), mach_header returned was NULL.\n" );
            #endif
            result = sal_False;
        }
    }
    if ( result == sal_True )
    {
        rtl_uString * workDir = NULL;

        osl_getProcessWorkingDir( &workDir );

    #if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( "module.c::osl_getModuleURLFromAddress - %s\n", imageName );
    #endif
        rtl_string2UString( ppLibraryUrl, imageName, strlen(imageName), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        osl_getFileURLFromSystemPath( *ppLibraryUrl, ppLibraryUrl ); // convert it to be a file url
        osl_getAbsoluteFileURL( workDir, *ppLibraryUrl, ppLibraryUrl ); // ensure it is an abosolute file url
    }

#else   /* MACOSX */
    Dl_info dl_info;

    if((result = dladdr(addr, &dl_info)) != 0)
    {
        rtl_uString * workDir = NULL;
        osl_getProcessWorkingDir(&workDir);

#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE("module.c::osl_getModuleURLFromAddress - %s\n", dl_info.dli_fname);
#endif

        rtl_string2UString(ppLibraryUrl, dl_info.dli_fname, strlen(dl_info.dli_fname), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
        osl_getFileURLFromSystemPath(*ppLibraryUrl, ppLibraryUrl); // convert it to be a file url
        osl_getAbsoluteFileURL(workDir, *ppLibraryUrl, ppLibraryUrl); // ensure it is an abosolute file url

        result = sal_True;
    }
#endif  /* MACOSX */
    return result;
}

