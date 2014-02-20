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




#include "system.h"

#include <osl/module.h>
#include <osl/diagnose.h>
#include <osl/file.h>
#include <osl/thread.h>

#include <stdlib.h>
#include <dlfcn.h>

int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

// static data for holding SAL dll module and full path
static HMODULE hModSal;
static char szSalDir[ _MAX_PATH];
static char szSalDrive[ _MAX_PATH];

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/

ULONG APIENTRY _DosLoadModule (PSZ pszObject, ULONG uObjectLen, PCSZ pszModule,
    PHMODULE phmod)
{
    APIRET  rc;
    rc = DosLoadModule( pszObject, uObjectLen, pszModule, phmod);
    // YD 22/05/06 issue again if first call fails (why?)
    if (rc == ERROR_INVALID_PARAMETER)
        rc = DosLoadModule( pszObject, uObjectLen, pszModule, phmod);
    return rc;
}

oslModule SAL_CALL osl_loadAsciiModule( const sal_Char* pModuleName, sal_Int32 nRtldMode )
{
    rtl_uString* pUniName = NULL;
    rtl_uString_newFromAscii( &pUniName, pModuleName );
    oslModule aModule = osl_loadModule( pUniName, nRtldMode );
    rtl_uString_release( pUniName );
    return aModule;
}

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    HMODULE hModule;
    BYTE szErrorMessage[256];
    APIRET rc;
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
        {
            char drive[_MAX_DRIVE], dir[_MAX_DIR];
            char fname[_MAX_FNAME], ext[_MAX_EXT];
            char* dot;
            // 21/02/2006 YD dll names must be 8.3: since .uno.dll files
            // have hardcoded names, I'm truncating names here and also in
            // the build system
            _splitpath (buffer, drive, dir, fname, ext);
            if (strlen(fname)>8)
                fname[8] = 0;   // truncate to 8.3
            dot = strchr( fname, '.');
            if (dot)
                *dot = '\0';    // truncate on dot
            // if drive is not specified, remove starting \ from dir name
            // so dll is loaded from LIBPATH
            if (drive[0] == 0 && dir[0] == '\\' && dir[1] == '\\') {
                while( dir[0] == '\\')
                    strcpy( dir, dir+1);
            }
            _makepath( buffer, drive, dir, fname, ext);

#if OSL_DEBUG_LEVEL>10
            debug_printf("osl_loadModule module %s", buffer);
#endif
            hModule = dlopen( buffer, RTLD_LOCAL);
            if (hModule != NULL )
                pModule = (oslModule)hModule;
            else
            {
                    sal_Char szError[ PATH_MAX*2 ];
                    sprintf( szError, "Module: %s; errno: %d;\n"
                            "Please contact technical support and report above informations.\n\n",
                            buffer, errno );
#if OSL_DEBUG_LEVEL>0
                    fprintf( stderr, szError);
#endif
                    debug_printf("osl_loadModule error %s", szError);

#if (OSL_DEBUG_LEVEL==0) || !defined(OSL_DEBUG_LEVEL)
                    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
                        szError, "Critical error: DosLoadModule failed",
                        0, MB_ERROR | MB_OK | MB_MOVEABLE);
#endif
            }
        }
    }

    rtl_uString_release( ustrTmp );

    return pModule;
}

/*****************************************************************************/
/* osl_getModuleHandle */
/*****************************************************************************/

sal_Bool SAL_CALL
osl_getModuleHandle(rtl_uString *pModuleName, oslModule *pResult)
{
    HMODULE hmod;
    APIRET  rc;
    rc = DosQueryModuleHandle(pModuleName->buffer, &hmod);
    if( rc == NO_ERROR)
    {
        *pResult = (oslModule) hmod;
        return sal_True;
    }

    return sal_False;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule Module)
{
#if OSL_DEBUG_LEVEL>0
    if (!Module)
       fprintf( stderr, "osl_unloadModule NULL HANDLE.\n");
#endif

    DosFreeModule((HMODULE)Module);
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
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *strSymbolName )
{
    rtl_String *symbolName = NULL;
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

/*****************************************************************************/
/* osl_getAsciiFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL
osl_getAsciiFunctionSymbol( oslModule Module, const sal_Char *pSymbol )
{
    PFN  pFunction;
    APIRET rc;
    void* pHandle=0;

    OSL_ENSURE(Module,"osl_getSymbol : module handle is not valid");
    OSL_ENSURE(Module,"osl_getSymbol : ustrSymbolName");

    if ( Module!= 0 && pSymbol != 0 )
    {

        rc = DosQueryProcAddr( (HMODULE) Module, 0, (PCSZ)pSymbol, &pFunction );
        if( rc == NO_ERROR )
        {
            pHandle = (void*)pFunction;
        }
        else
        {
            // YD try again adding the '_' prefix
            char _pszSymbolName[255];
            strcpy( _pszSymbolName, "_");
            strcat( _pszSymbolName, pSymbol);
            rc = DosQueryProcAddr( (HMODULE) Module, 0, (PCSZ)_pszSymbolName, &pFunction );
            if( rc == NO_ERROR )
                pHandle = (void*)pFunction;
        }

    }

    return pHandle;
}

/*****************************************************************************/
/* osl_getModuleURLFromAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromAddress(void * addr, rtl_uString ** ppLibraryUrl)
{
    //APIRET APIENTRY DosQueryModFromEIP (HMODULE *phMod, ULONG *pObjNum,
    //          ULONG BuffLen, PCHAR pBuff, ULONG *pOffset, ULONG Address)
    HMODULE hMod;
    ULONG   ObjNum;
    CHAR    Buff[2*_MAX_PATH];
    ULONG   Offset;
    APIRET  rc;

    // get module handle (and name)
    rc = DosQueryModFromEIP( &hMod, &ObjNum, sizeof( Buff), Buff, &Offset, (ULONG)addr);
    if (rc)
        return sal_False;

    // get module full path
    rc = DosQueryModuleName( hMod, sizeof( Buff), Buff);
    if (rc)
        return sal_False;

#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE("module.c::osl_getModuleURLFromAddress - %s\n", Buff);
#endif

    // convert to URL
    rtl_uString *ustrSysPath = NULL;
    rtl_string2UString( &ustrSysPath, Buff, strlen(Buff), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
    OSL_ASSERT(ustrSysPath != NULL);
    osl_getFileURLFromSystemPath( ustrSysPath, ppLibraryUrl );
    rtl_uString_release( ustrSysPath );

    return sal_True;
}

/*****************************************************************************/
/* osl_getModuleURLFromFunctionAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress( oslGenericFunction addr, rtl_uString ** ppLibraryUrl )
{
    return osl_getModuleURLFromAddress( ( void * )addr, ppLibraryUrl );
}

/*****************************************************************************/

