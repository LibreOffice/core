/*************************************************************************
 *
 *  $RCSfile: module.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:20 $
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

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/
oslModule SAL_CALL osl_loadModule(const sal_Char* pszModuleName, sal_Int32 nRtldMode )
{
    HMODULE hModule;
    sal_Char szErrorMessage[ 80 ];
    APIRET rc;

    OSL_ASSERT(pszModuleName);

    rc = DosLoadModule( szErrorMessage, sizeof( szErrorMessage ),
                        (sal_Char*)pszModuleName, &hModule );

    if (rc == NO_ERROR )
        return (oslModule)hModule;
    else
    {
        sal_Char szError[ 120 ];
        sprintf( szError, "DosLoadModule (%d): %s\n", rc, szErrorMessage );
        OSL_TRACE(szError);

        return 0;
    }
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule Module)
{
    OSL_ASSERT(Module);

    DosFreeModule((HMODULE)Module);
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, const sal_Char* pszSymbolName)
{
    PFN  pFunction;
    APIRET rc;

    OSL_ASSERT(Module);
    OSL_ASSERT(pszSymbolName);

    rc = DosQueryProcAddr( (HMODULE) Module, 0, (sal_Char*) pszSymbolName, &pFunction );

    if( rc == NO_ERROR )
        return (void*)pFunction;
    else
        return NULL;
}

/*****************************************************************************/

