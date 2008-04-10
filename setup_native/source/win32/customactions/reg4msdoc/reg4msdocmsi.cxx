/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: reg4msdocmsi.cxx,v $
 * $Revision: 1.7 $
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

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <tchar.h>
#include "register.hxx"
#include "msihelper.hxx"

#include <memory>
#include <string>

#define ELEMENTS_OF_ARRAY(a) (sizeof(a)/sizeof(a[0]))

void DetermineWordPreselectionState(MSIHANDLE handle)
{
    if (query_preselect_registration_for_ms_application(handle, MSWORD))
        SetMsiProp(handle, TEXT("SELECT_WORD"));
}

void DetermineExcelPreselectionState(MSIHANDLE handle)
{
    if (query_preselect_registration_for_ms_application(handle, MSEXCEL))
        SetMsiProp(handle, TEXT("SELECT_EXCEL"));
}

void DeterminePowerPointPreselectionState(MSIHANDLE handle)
{
    if (query_preselect_registration_for_ms_application(handle, MSPOWERPOINT))
        SetMsiProp(handle, TEXT("SELECT_POWERPOINT"));
}

extern "C" UINT __stdcall InstallUiSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("InstallUiSequenceEntry"), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

    if (IsModuleSelectedForInstallation(handle, TEXT("gm_p_Wrt_Bin")))
    {
        DetermineWordPreselectionState(handle);
    }
    else if (IsModuleInstalled(handle, TEXT("gm_p_Wrt_Bin")) &&
             !IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Wrt_Bin")) &&
             IsRegisteredFor(handle, MSWORD))
    {
        SetMsiProp(handle, TEXT("SELECT_WORD"));
    }
    else
    {
        UnsetMsiProp(handle, TEXT("SELECT_WORD"));
    }

    if (IsModuleSelectedForInstallation(handle, TEXT("gm_p_Calc_Bin")))
    {
        DetermineExcelPreselectionState(handle);
    }
    else if (IsModuleInstalled(handle, TEXT("gm_p_Calc_Bin")) &&
             !IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Calc_Bin")) &&
             IsRegisteredFor(handle, MSEXCEL))
    {
        SetMsiProp(handle, TEXT("SELECT_EXCEL"));
    }
    else
    {
        UnsetMsiProp(handle, TEXT("SELECT_EXCEL"));
    }

    if (IsModuleSelectedForInstallation(handle, TEXT("gm_p_Impress_Bin")))
    {
        DeterminePowerPointPreselectionState(handle);
    }
    else if (IsModuleInstalled(handle, TEXT("gm_p_Impress_Bin")) &&
            !IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Impress_Bin")) &&
            IsRegisteredFor(handle, MSPOWERPOINT))
    {
        SetMsiProp(handle, TEXT("SELECT_POWERPOINT"));
    }
    else
    {
        UnsetMsiProp(handle, TEXT("SELECT_POWERPOINT"));
    }

    SetMsiProp(handle, TEXT("UI_SEQUENCE_EXECUTED"));

    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall InstallExecSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("InstallExecSequenceEntry"), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

    // Do nothing in repair mode.
    // Then UI_SEQUENCE_EXECUTED is not set and Installed is set!
    // In silent installation UI_SEQUENCE_EXECUTED is also not set, but Installed is not set.
    if ((!IsSetMsiProp(handle, TEXT("UI_SEQUENCE_EXECUTED"))) && (IsMsiPropNotEmpty(handle, TEXT("Installed")))) { return ERROR_SUCCESS; }

    int reg4 = 0;
    int unreg4 = 0;

    // we always register as html editor for Internet Explorer
    // if writer is installed because there's no harm if we do so
    if (IsModuleSelectedForInstallation(handle, TEXT("gm_p_Wrt_Bin")))
        reg4 |= HTML_EDITOR;

    if (IsSetMsiProp(handle, TEXT("SELECT_WORD")) && !IsRegisteredFor(handle, MSWORD))
        reg4 |= MSWORD;
    else if (!IsSetMsiProp(handle, TEXT("SELECT_WORD")) && IsRegisteredFor(handle, MSWORD))
        unreg4 |= MSWORD;

    if (IsSetMsiProp(handle, TEXT("SELECT_EXCEL")) && !IsRegisteredFor(handle, MSEXCEL))
        reg4 |= MSEXCEL;
    else if (!IsSetMsiProp(handle, TEXT("SELECT_EXCEL")) && IsRegisteredFor(handle, MSEXCEL))
        unreg4 |= MSEXCEL;

    if (IsSetMsiProp(handle, TEXT("SELECT_POWERPOINT")) && !IsRegisteredFor(handle, MSPOWERPOINT))
        reg4 |= MSPOWERPOINT;
    else if (!IsSetMsiProp(handle, TEXT("SELECT_POWERPOINT")) && IsRegisteredFor(handle, MSPOWERPOINT))
        unreg4 |= MSPOWERPOINT;

    if (reg4)
    {
        Register4MsDoc(handle, reg4);
    }

    if (unreg4)
    {
        Unregister4MsDoc(handle, unreg4);
    }
    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallExecSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("DeinstallExecSequenceEntry"), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

    if (IsCompleteDeinstallation(handle))
    {
        Unregister4MsDocAll(handle);
        return ERROR_SUCCESS;
    }

    if (IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Wrt_Bin")))
    {
        Unregister4MsDoc(handle, MSWORD | HTML_EDITOR);
    }

    if (IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Calc_Bin")))
    {
        Unregister4MsDoc(handle, MSEXCEL);
    }

    if (IsModuleSelectedForDeinstallation(handle, TEXT("gm_p_Impress_Bin")))
    {
        Unregister4MsDoc(handle, MSPOWERPOINT);
    }

    return ERROR_SUCCESS;
}
