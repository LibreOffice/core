/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mpr9x.h,v $
 * $Revision: 1.3 $
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

#ifndef _WINDOWS_
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C"{
#endif

//------------------------------------------------------------------------
// undefine the macros defined in the winuser.h file in order to avoid
// warnings because of multiple defines
//------------------------------------------------------------------------

#ifdef WNetGetConnectionW
#undef WNetGetConnectionW
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define MPR9X_API extern

//------------------------------------------------------------------------
// declare function pointers to the appropriate user32 functions
//------------------------------------------------------------------------

MPR9X_API DWORD (WINAPI *lpfnWNetGetConnectionW)(
    LPCWSTR lpLocalName,    // pointer to local name
    LPWSTR  lpRemoteName,   // pointer to buffer for remote name
    LPDWORD lpnLength       // pointer to buffer size, in characters
);

//------------------------------------------------------------------------
// redefine the above undefined macros so that the preprocessor replaces
// all occurrences of this macros with our function pointer
//------------------------------------------------------------------------

#define WNetGetConnectionW          lpfnWNetGetConnectionW

#ifdef __cplusplus
}
#endif
