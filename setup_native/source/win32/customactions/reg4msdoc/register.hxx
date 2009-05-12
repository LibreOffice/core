/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: register.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _REGISTER_HXX_
#define _REGISTER_HXX_

#ifndef _CONSTANTS_HXX_
#include "constants.hxx"
#endif

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msi.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>

const int MOD_WRITER  = 0x1;
const int MOD_CALC    = 0x2;
const int MOD_IMPRESS = 0x4;
const int MOD_ACTIVEX = 0x40;

/*  registers StarOffice for MS document
    types and as default HTML editor if
    specified */
void Register4MsDoc(MSIHANDLE handle, int Register);

void Unregister4MsDoc(MSIHANDLE handle, int Unregister);

/* restores the entries for the selected
   registry entries */
void Unregister4MsDocAll(MSIHANDLE handle);

/* restores lost settings formerly made
   with Register4MsDoc */
void RepairRegister4MsDocSettings(MSIHANDLE handle);

/** Returns whether we are registered for */
bool IsRegisteredFor(MSIHANDLE handle, int State);

/** Returns whether we should preselect the
    registration checkbox for a certain
    application type or not */
bool query_preselect_registration_for_ms_application(
    MSIHANDLE handle, int Register);

// StarOffice 6.0 saved the registration
// state in HKEY_LOCAL_MACHINE\Software\
// Microsoft\Windows\CurrentVersion\Uninstall\
// StarOffice 6.0\Reg4MsdocState we move this
// value if available to the new registry
// location
int FixReturnRegistrationState(MSIHANDLE handle);

#endif
