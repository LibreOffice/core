/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:23 $
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
#include <osl/types.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.h>
#include <systools/win32/kernel9x.h>


/*
    under WIN32, we use the void* oslModule
    as a WIN32 HANDLE (which is also a 32-bit value)
*/

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/
oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 nRtldMode )
{
    HINSTANCE hInstance;
    UINT errorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    rtl_uString* Module = NULL;

    OSL_ASSERT(strModuleName);

    if (strModuleName->buffer[0] == L'/' && strModuleName->buffer[1] == L'/')
        osl_getSystemPathFromNormalizedPath(strModuleName, &Module);
    else
        rtl_uString_assign(&Module, strModuleName);

    hInstance = LoadLibraryW(Module->buffer);

    if (hInstance <= (HINSTANCE)HINSTANCE_ERROR)
        hInstance = 0;

    rtl_uString_release(Module);
    SetErrorMode(errorMode);
    return (oslModule)hInstance;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule Module)
{
    OSL_ASSERT(Module);
    FreeLibrary((HINSTANCE)Module);
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *strSymbolName)
{
    rtl_String *symbolName = NULL;
    void *address;

    OSL_ASSERT(Module);
    OSL_ASSERT(strSymbolName);

    rtl_uString2String(
        &symbolName,
        strSymbolName->buffer,
        strSymbolName->length,
        RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS
    );

    address=(void*)GetProcAddress((HINSTANCE)Module, rtl_string_getStr(symbolName));
    rtl_string_release(symbolName);

    return address;
}

/*****************************************************************************/
/* osl_getSymbolA */
/*****************************************************************************/

void* SAL_CALL osl_getSymbolA(oslModule Module, const char *pszSymbolName)
{
    OSL_ASSERT(Module);
    OSL_ASSERT(pszSymbolName);

    return (void*) GetProcAddress((HINSTANCE)Module, pszSymbolName);
}

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.11  2000/09/18 14:29:01  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.10  2000/06/18 12:06:35  obr
*    function names have to be ascii
*
*    Revision 1.9  2000/05/22 15:30:35  obr
*    moved some unicode wrapper for 9x to kernel9x.lib
*
*    Revision 1.8  2000/03/31 15:55:14  rs
*    UNICODE-Changes
*
*    Revision 1.7  2000/01/06 11:50:21  mfe
*    #71540# : for incompatible z'en
*
*    Revision 1.6  2000/01/04 14:06:15  mfe
*    osl_loadModule takes 2nd parameter
*
*    Revision 1.5  1999/10/27 15:13:35  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.4  1999/04/13 11:50:24  jsc
*    #62178# Keine ErrorBox, wenn eine dynamisch angezogene DLL eines Modules nicht gefunden wird und das Module durch loadOnCall angezogen wird
*
*    Revision 1.3  1999/01/20 19:09:49  jsc
*    #61011# Typumstellung
*
*    Revision 1.2  1997/07/15 19:02:03  rh
*    system.h inserted
*
*    Revision 1.1  1997/07/03 10:04:32  rh
*    Module handling added
*
*************************************************************************/

