/*************************************************************************
 *
 *  $RCSfile: comdlg9x.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2000-11-22 13:54:59 $
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

#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef _COMMDLG_H_
#include <commdlg.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the shlobj.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef GetOpenFileNameW
#undef GetOpenFileNameW
#endif

#ifdef GetSaveFileNameW
#undef GetSaveFileNameW
#endif

//------------------------------------------------------------------------
// set the compiler directives for the function pointer we declare below
// if we build sal or sal will be used as static library we define extern
// else sal exports the function pointers from a dll and we use __declspec
//------------------------------------------------------------------------

#if defined(SAL_EXPORT_SYSTOOLS) || defined(USE_SAL_STATIC)
    #define COMDLG9X_API extern
#else
    #define COMDLG9X_API __declspec( dllimport )
#endif

//------------------------------------------------------------------------
// the Comdlg9xInit and Comdlg9xDeInit functions will be used only by
// sal itself and will not be exported
//------------------------------------------------------------------------

#if defined(SAL_EXPORT_SYSTOOLS)
    extern void WINAPI Comdlg9xInit( );
    extern void WINAPI Comdlg9xDeInit( );
#endif

//------------------------------------------------------------------------
// declare function pointers to the appropriate comdlg functions
//------------------------------------------------------------------------

COMDLG9X_API BOOL ( WINAPI * lpfnGetOpenFileNameW ) ( LPOPENFILENAMEW lpofn );
COMDLG9X_API BOOL ( WINAPI * lpfnGetSaveFileNameW ) ( LPOPENFILENAMEW lpofn );

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define GetOpenFileNameW    lpfnGetOpenFileNameW
#define GetSaveFileNameW    lpfnGetSaveFileNameW

#ifdef __cplusplus
}
#endif
