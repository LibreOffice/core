/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 15:11:52 $
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


#include "system.h"

#include <osl/module.h>
#include <osl/diagnose.h>

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <stdlib.h>

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

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    HMODULE hModule;
    BYTE szErrorMessage[256];
    APIRET rc;
    oslModule pModule=0;
    rtl_uString* ustrTmp = NULL;

    OSL_ENSURE(ustrModuleName,"osl_loadModule : string is not valid");

    // to avoid registering dll with full path at every install inside
    // services.rbd, the dll are registere only once with empty path
    // then if drive is not specified, the SAL module path is used
    // to load components.
    if (hModSal == NULL) {
        ULONG   ObjNum;
        CHAR    Buff[2*_MAX_PATH];
        ULONG   Offset;
        char    drive[_MAX_DRIVE], dir[_MAX_DIR];
        char    fname[_MAX_FNAME], ext[_MAX_EXT];
        // get module handle (and name)
        rc = DosQueryModFromEIP( &hModSal, &ObjNum, sizeof( Buff), Buff,
                                &Offset, (ULONG)osl_loadModule);
        if (rc) {
            sal_Char szError[ 120 ];
            sprintf( szError, "DosQueryModFromEIP failed (%s) rc=%d\n", Buff, rc);
            OSL_TRACE(szError);
            return NULL;
        }
        DosQueryModuleName(hModSal, sizeof(Buff), Buff);
        // extract path info
        _splitpath( Buff, szSalDrive, szSalDir, NULL, NULL);
        // update BeginLIBPATH, otherwise a rc=2 can be returned if someone
        // changes the current directory
        CHAR    ExtLIBPATH[1024];
        strcpy( ExtLIBPATH, szSalDrive);
        strcat( ExtLIBPATH, szSalDir);
        strcat( ExtLIBPATH, ";%BeginLIBPATH%");
        rc = DosSetExtLIBPATH( (PCSZ)ExtLIBPATH, BEGIN_LIBPATH);
    }

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
            _makepath( buffer, drive, dir, fname, ext);

            rc = _DosLoadModule( szErrorMessage, sizeof( szErrorMessage), (PCSZ)buffer, &hModule);
            if (rc == NO_ERROR )
                pModule = (oslModule)hModule;
            else
            {
                // try again using SAL path
                _makepath( buffer, szSalDrive, szSalDir, fname, ext);
                rc = _DosLoadModule( szErrorMessage, sizeof( szErrorMessage), (PCSZ)buffer, &hModule);

                if (rc == NO_ERROR )
                    pModule = (oslModule)hModule;
                else
                {
                    sal_Char szError[ 120 ];
                    sprintf( szError, "Module: %s; rc: %d;\nReason: %s;\n"
                            "Please contact technical support and report above informations.\n\n",
                            buffer, rc, szErrorMessage );
#if OSL_DEBUG_LEVEL>0
                    fprintf( stderr, szError);
#endif
                    //OSL_TRACE(szError);
#ifndef OSL_DEBUG_LEVEL
                    WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
                        szError, "Critical error: DosLoadModule failed",
                        0, MB_ERROR | MB_OK | MB_MOVEABLE);
#endif
                }
            }
        }
    }

    rtl_uString_release( ustrTmp );

    return pModule;
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
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *ustrSymbolName)
{
    PFN  pFunction;
    APIRET rc;
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

        rc = DosQueryProcAddr( (HMODULE) Module, 0, (PCSZ)pszSymbolName, &pFunction );
        if( rc == NO_ERROR )
        {
            pHandle = (void*)pFunction;
        }
        else
        {
            // YD try again adding the '_' prefix
            char _pszSymbolName[255];
            strcpy( _pszSymbolName, "_");
            strcat( _pszSymbolName, pszSymbolName);
            rc = DosQueryProcAddr( (HMODULE) Module, 0, (PCSZ)_pszSymbolName, &pFunction );
            if( rc == NO_ERROR )
                pHandle = (void*)pFunction;
        }

        if ( strSymbolName != 0 )
        {
            rtl_string_release(strSymbolName);
        }
    }


    return pHandle;
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *ustrFunctionSymbolName )
{
    return ( oslGenericFunction )osl_getSymbol( Module, ustrFunctionSymbolName );
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

