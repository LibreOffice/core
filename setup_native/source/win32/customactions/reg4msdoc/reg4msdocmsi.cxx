/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reg4msdocmsi.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:35:28 $
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <msiquery.h>

#ifndef _REGISTER_HXX_
#include "register.hxx"
#endif

#ifndef INCLUDED_MSIHELPER_HXX
#include "msihelper.hxx"
#endif

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
    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall InstallExecSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("InstallExecSequenceEntry"), TEXT("Information"), MB_OK | MB_ICONINFORMATION);

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
