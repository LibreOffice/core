/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shell9x.h,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
