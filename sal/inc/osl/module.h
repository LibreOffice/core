/*************************************************************************
 *
 *  $RCSfile: module.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obr $ $Date: 2000-11-01 11:35:48 $
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


#ifndef _OSL_MODULE_H_
#define _OSL_MODULE_H_

#ifndef _RTL_USTRING_H
#   include <rtl/ustring.h>
#endif

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifndef _RTL_TENCINFO_H
#   include <rtl/tencinfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_DLLPREFIX
#define SAL_MODULENAME(name) SAL_DLLPREFIX name SAL_DLLEXTENSION
#else
#define SAL_MODULENAME(name) name SAL_DLLEXTENSION
#endif

#define SAL_LOADMODULE_DEFAULT    0x00000
#define SAL_LOADMODULE_LAZY       0x00001
#define SAL_LOADMODULE_NOW        0x00002
#define SAL_LOADMODULE_GLOBAL     0x00100


typedef void* oslModule;


/** Load a module.<BR>
    @param strModuleName denotes the name of the module to be loaded.
    @return 0 if the module could not be loaded, otherwise a handle to the module.
*/
oslModule SAL_CALL osl_loadModule(rtl_uString *strModuleName, sal_Int32 nRtldMode);

/** Release the Module
*/
void SAL_CALL osl_unloadModule(oslModule Module);

/** lookup the specified symbolname.
    @returns address of the symbol or 0 if lookup failed,
*/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString *strSymbolName);

/** ascii interface for internal use.
    @returns address of the symbol or 0 if lookup failed,
*/
void* SAL_CALL osl_getSymbolA(oslModule Module, const char *pszSymbolName);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_MODULE_H_  */

