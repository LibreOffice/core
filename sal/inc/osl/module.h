/*************************************************************************
 *
 *  $RCSfile: module.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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


/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.8  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.7  2000/05/17 14:30:14  mfe
*    comments upgraded
*
*    Revision 1.6  2000/03/31 15:55:56  rs
*    UNICODE-Changes
*
*    Revision 1.5  2000/03/16 16:43:52  obr
*    Unicode API changes
*
*    Revision 1.4  2000/01/06 11:46:32  mfe
*    #71540# : for incompatible z'en
*
*    Revision 1.3  2000/01/04 14:07:19  mfe
*    osl_loadModule takes 2nd parameter
*
*    Revision 1.2  1999/10/27 15:02:07  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:19  jsc
*    verschoben aus osl
*
*    Revision 1.6  1999/08/05 11:14:35  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.5  1999/01/20 18:53:40  jsc
*    #61011# Typumstellung
*
*    Revision 1.4  1998/02/16 19:34:51  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.3  1997/07/31 15:28:38  ts
*    *** empty log message ***
*
*    Revision 1.2  1997/07/17 11:02:27  rh
*    Header adapted and profile added
*
*    Revision 1.1  1997/07/03 10:04:31  rh
*    Module handling added
*
*************************************************************************/

