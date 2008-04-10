/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: register.cxx,v $
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

#include "register.hxx"
#include "registryexception.hxx"
#include "registrationcontextinformation.hxx"
#include "userregistrar.hxx"
#include "windowsregistry.hxx"
#include "stringconverter.hxx"
#include "msihelper.hxx"

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#pragma warning(disable: 4917)
#endif
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


#include <assert.h>
#ifdef _MSC_VER
#pragma warning(disable: 4350)
#endif

typedef std::auto_ptr<Registrar> RegistrarPtr;

namespace /* private */
{
    RegistrarPtr CreateRegistrar(bool InstallForAllUser, const RegistrationContextInformation& RegCtx)
    {
        RegistrarPtr RegPtr;

        if (InstallForAllUser)
            RegPtr = RegistrarPtr(new Registrar(RegCtx));
        else
            RegPtr = RegistrarPtr(new UserRegistrar(RegCtx));

        return RegPtr;
    }
} // namespace private

bool query_preselect_registration_for_ms_application(MSIHANDLE handle, int Register)
{
    bool preselect = false;

    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        if (Register & MSWORD)
            preselect = CurrentRegistrar->QueryPreselectMsWordRegistration();
        else if (Register & MSEXCEL)
            preselect = CurrentRegistrar->QueryPreselectMsExcelRegistration();
        else if (Register & MSPOWERPOINT)
            preselect = CurrentRegistrar->QueryPreselectMsPowerPointRegistration();
    }
    catch(RegistryException&)
    {
        assert(false);
    }
    return preselect;
}

//-----------------------------------------
// registers StarOffice for MS document
// types and as default HTML editor if
// specified
//-----------------------------------------

void Register4MsDoc(MSIHANDLE handle, int Register)
{
    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        if ((Register & MSWORD))
            CurrentRegistrar->RegisterForMsWord();

        if ((Register & MSEXCEL))
            CurrentRegistrar->RegisterForMsExcel();

        if ((Register & MSPOWERPOINT))
            CurrentRegistrar->RegisterForMsPowerPoint();

        if ((Register & HTML_EDITOR))
            CurrentRegistrar->RegisterAsHtmlEditorForInternetExplorer();

        if ((Register & DEFAULT_SHELL_HTML_EDITOR))
        {
            CurrentRegistrar->RegisterAsDefaultHtmlEditorForInternetExplorer();
            CurrentRegistrar->RegisterAsDefaultShellHtmlEditor();
        }
    }
    catch(RegistryException&)
    {
        assert(false);
    }

    if (Register)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
}

void Unregister4MsDoc(MSIHANDLE handle, int Unregister)
{
    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        if ((Unregister & MSWORD) && CurrentRegistrar->IsRegisteredFor(MSWORD))
            CurrentRegistrar->UnregisterForMsWord();

        if ((Unregister & HTML_EDITOR) && CurrentRegistrar->IsRegisteredFor(HTML_EDITOR))
            CurrentRegistrar->UnregisterAsHtmlEditorForInternetExplorer();

        if ((Unregister & MSEXCEL) && CurrentRegistrar->IsRegisteredFor(MSEXCEL))
            CurrentRegistrar->UnregisterForMsExcel();

        if ((Unregister & MSPOWERPOINT) && CurrentRegistrar->IsRegisteredFor(MSPOWERPOINT))
            CurrentRegistrar->UnregisterForMsPowerPoint();

        if ((Unregister & DEFAULT_HTML_EDITOR_FOR_IE) && CurrentRegistrar->IsRegisteredFor(DEFAULT_HTML_EDITOR_FOR_IE))
            CurrentRegistrar->UnregisterAsDefaultHtmlEditorForInternetExplorer();

        if ((Unregister & DEFAULT_SHELL_HTML_EDITOR) && CurrentRegistrar->IsRegisteredFor(DEFAULT_SHELL_HTML_EDITOR))
            CurrentRegistrar->UnregisterAsDefaultShellHtmlEditor();
    }
    catch(RegistryException&)
    {
        assert(false);
    }

    if (Unregister)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
}

//-----------------------------------------
// restores the entries for the selected
// registry entries
// Algorithm:
//
// 1.
// Target key exist (e.g. '.doc')
// Default value == soffice.?
// Backup key != empty
// Action: Replace Default value with backup
// key
//
// 2.
// Target key exist
// Default value == soffice.?
// Backup key == empty
// Action: delete default value
//
// 3.
// Target key exist
// Default value != soffice.?
// Action: nop
//
// 4.
// Target key does not exist
// Action: nop
//-----------------------------------------

void Unregister4MsDocAll(MSIHANDLE handle)
{
    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        CurrentRegistrar->UnregisterAllAndCleanUpRegistry();
    }
    catch(RegistryException&)
    {
        assert(false);
    }
}

//-----------------------------------------
// restores lost settings formerly made
// with Register4MsDoc
//-----------------------------------------

void RepairRegister4MsDocSettings(MSIHANDLE handle)
{
    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        CurrentRegistrar->RepairRegistrationState();
    }
    catch(RegistryException&)
    {
        assert(false);
    }
}

bool IsRegisteredFor(MSIHANDLE handle, int State)
{
    bool Registered = false;

    try
    {
        RegistrationContextInformation RegContext(handle, GetOfficeExecutablePath(handle));
        RegistrarPtr CurrentRegistrar = CreateRegistrar(IsAllUserInstallation(handle), RegContext);

        Registered = CurrentRegistrar->IsRegisteredFor(State);
    }
    catch(RegistryException&)
    {
        assert(false);
    }
    return Registered;
}

#define SO60_UNINSTALL_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\StarOffice 6.0"
#define SO_BACKUP_KEY      L"soffice6.bak"
#define REGMSDOCSTATE      L"Reg4MsDocState"
#define SOFTWARE_CLASSES   L"Software\\Classes"

int FixReturnRegistrationState(MSIHANDLE handle)
{
    int registration_state = 0;

    try
    {
        WindowsRegistry registry;

        RegistryValue rv_regmsdocstate = RegistryValue(
            new RegistryValueImpl(REGMSDOCSTATE, 0));

        RegistryKey so_bak_key;

        if (IsAllUserInstallation(handle))
        {
            RegistryKey hkcr_key = registry.GetClassesRootKey();

            if (hkcr_key->HasSubKey(SO_BACKUP_KEY))
                so_bak_key = hkcr_key->OpenSubKey(SO_BACKUP_KEY);
            else
                so_bak_key = hkcr_key->CreateSubKey(SO_BACKUP_KEY);

            if (!so_bak_key->HasValue(REGMSDOCSTATE))
            {
                // set a defined value
                so_bak_key->SetValue(rv_regmsdocstate);

                RegistryKey hklm_key = registry.GetLocalMachineKey();

                if (hklm_key->HasSubKey(SO60_UNINSTALL_KEY))
                {
                    RegistryKey so_uninst_key =
                        hklm_key->OpenSubKey(SO60_UNINSTALL_KEY);

                    if (so_uninst_key->HasValue(REGMSDOCSTATE))
                        so_bak_key->CopyValue(so_uninst_key, REGMSDOCSTATE);
                }
            }
        }
        else
        {
            RegistryKey hkcu_classes_key =
                registry.GetCurrentUserKey()->OpenSubKey(SOFTWARE_CLASSES);

            so_bak_key = hkcu_classes_key->CreateSubKey(SO_BACKUP_KEY);

            if (!so_bak_key->HasValue(REGMSDOCSTATE))
            {
                // set a defined value
                so_bak_key->SetValue(rv_regmsdocstate);

                RegistryKey hklm_sftw_classes =
                    registry.GetLocalMachineKey()->OpenSubKey(SOFTWARE_CLASSES, false);

                RegistryKey so_bak_key_old;

                if (hklm_sftw_classes->HasSubKey(SO_BACKUP_KEY))
                {
                    so_bak_key_old = hklm_sftw_classes->OpenSubKey(SO_BACKUP_KEY, false);

                    if (so_bak_key_old->HasValue(REGMSDOCSTATE))
                        so_bak_key->CopyValue(so_bak_key_old, REGMSDOCSTATE);
                }
                else // try the uninstall key
                {
                    RegistryKey hklm_key = registry.GetLocalMachineKey();

                    if (hklm_key->HasSubKey(SO60_UNINSTALL_KEY))
                    {
                        RegistryKey so_uninst_key =
                            hklm_key->OpenSubKey(SO60_UNINSTALL_KEY);

                        if (so_uninst_key->HasValue(REGMSDOCSTATE))
                            so_bak_key->CopyValue(so_uninst_key, REGMSDOCSTATE);
                    }
                }
            }
        }

        rv_regmsdocstate = so_bak_key->GetValue(REGMSDOCSTATE);
        registration_state = rv_regmsdocstate->GetDataAsInt();
    }
    catch(RegistryException&)
    {
        registration_state = 0;
    }

    return registration_state;
}

