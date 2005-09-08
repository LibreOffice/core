/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shell9x.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:50:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#pragma once

#ifndef _SHELL9X_H_
#define _SHELL9X_H_

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif

#include <shellapi.h>

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the shlobj.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef CommandLineToArgvW
#undef CommandLineToArgvW
#endif

#ifdef SHBrowseForFolderW
#undef SHBrowseForFolderW
#endif

#ifdef SHGetPathFromIDListW
#undef SHGetPathFromIDListW
#endif

#ifdef ShellExecuteExW
#undef ShellExecuteExW
#endif

//------------------------------------------------------------------------
// set the compiler directives for the function pointer we declare below
// if we build sal or sal will be used as static library we define extern
// else sal exports the function pointers from a dll and we use __declspec
//------------------------------------------------------------------------

#define SHELL9X_API extern

//------------------------------------------------------------------------
// declare function pointers to the appropriate shell functions
//------------------------------------------------------------------------

SHELL9X_API LPWSTR *     ( WINAPI * lpfnCommandLineToArgvW ) ( LPCWSTR lpCmdLine, int *pNumArgs );
SHELL9X_API LPITEMIDLIST ( WINAPI * lpfnSHBrowseForFolderW ) ( LPBROWSEINFOW lpbi );
SHELL9X_API BOOL         ( WINAPI * lpfnSHGetPathFromIDListW ) ( LPCITEMIDLIST pidl, LPWSTR pszPath );

SHELL9X_API BOOL ( WINAPI * lpfnShellExecuteExW ) ( LPSHELLEXECUTEINFOW lpExecInfo );

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define CommandLineToArgvW   lpfnCommandLineToArgvW
#define SHBrowseForFolderW   lpfnSHBrowseForFolderW
#define SHGetPathFromIDListW lpfnSHGetPathFromIDListW

#define ShellExecuteExW lpfnShellExecuteExW

#ifdef __cplusplus
}
#endif

#endif
