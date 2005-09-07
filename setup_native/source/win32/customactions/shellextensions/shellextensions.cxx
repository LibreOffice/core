/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shellextensions.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:40:46 $
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

/*
    Windows shell extensions need to be approved in order to be used by the
    Windows shell for clarification read the following section from the
    Microsoft Developers Network Library (MSDN) see
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_extending/extensionhandlers/shell_ext.asp


    <MSDN>
    Shell extension handlers run in the Shell process. Because it is a system process,
    the administrator of a Windows NT system can limit Shell extension handlers to
    those on an approved list by setting the EnforceShellExtensionSecurity value of the
    HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer key to 1
    (one).
    To place a Shell extension handler on the approved list, create a REG_SZ value whose
    name is the string form of the handler's GUID under
    HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved.

    The Shell does not use the value that is assigned to the GUID, but it should be set to
    make inspecting the registry easier.

    Your setup application can add values to the Approved key only if the person installing
    the application has sufficient privileges. If the attempt to add an extension handler
    fails, you should inform the user that administrative privileges are required to fully
    install the application. If the handler is essential to the application, you should fail
    the setup and notify the user to contact an administrator.

    While there is no need to add values to the Approved key on Windows 95 or Windows 98
    systems, there is no harm in doing so. The system will simply ignore them. However, there
    is no guarantee that the key will exist on these systems. Your setup program must be able
    to handle this case.
    </MSDN>

    We add the following entries to the respective registry key
    "{C52AF81D-F7A0-4AAB-8E87-F80A60CCD396}"="OpenOffice.org Column Handler"
    "{087B3AE3-E237-4467-B8DB-5A38AB959AC9}"="OpenOffice.org Infotip Handler"
    "{63542C48-9552-494A-84F7-73AA6A7C99C1}"="OpenOffice.org Property Sheet Handler"
    "{3B092F0C-7696-40E3-A80F-68D74DA84210}"="OpenOffice.org Thumbnail Viewer"

    These shell extensions are implemented in the 'shell' project. We ignore registration
    failures because of insufficient privileges. The reason is: On systems which restrict the
    use of shell extensions by applying the aforementioned policy probably only people with
    sufficient privileges are allowed to install applications anyway. On systems where the
    use of shell extensions is not restricted registration failures because of insufficient
    prviliges have no negative effect because the shell extensions will work anyhow.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#endif
#include <tchar.h>

struct RegistryEntry
{
    TCHAR* Key;
    TCHAR* Value;
};

RegistryEntry ColumnHandler = { TEXT("{C52AF81D-F7A0-4AAB-8E87-F80A60CCD396}"), TEXT("OpenOffice.org Column Handler") };
RegistryEntry InfotipHandler = { TEXT("{087B3AE3-E237-4467-B8DB-5A38AB959AC9}"), TEXT("OpenOffice.org Infotip Handler") };
RegistryEntry PropHandler = { TEXT("{63542C48-9552-494A-84F7-73AA6A7C99C1}"), TEXT("OpenOffice.org Property Sheet Handler") };
RegistryEntry ThumbViewer = { TEXT("{3B092F0C-7696-40E3-A80F-68D74DA84210}"), TEXT("OpenOffice.org Thumbnail Viewer") };

/*
    Called during installation when the module "Windows Explorer Extensions" is
    selected.
*/
extern "C" UINT __stdcall InstallExecSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("InstallExecSequenceEntry"), TEXT("Pythonmsi"), MB_OK | MB_ICONINFORMATION);
    HKEY hKey;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, ColumnHandler.Key, 0, REG_SZ, reinterpret_cast<const BYTE*>(ColumnHandler.Value), _tcslen(ColumnHandler.Value) + 1);
        RegSetValueEx(hKey, InfotipHandler.Key, 0, REG_SZ, reinterpret_cast<const BYTE*>(InfotipHandler.Value), _tcslen(InfotipHandler.Value) + 1);
        RegSetValueEx(hKey, PropHandler.Key, 0, REG_SZ, reinterpret_cast<const BYTE*>(PropHandler.Value), _tcslen(PropHandler.Value) + 1);
        RegSetValueEx(hKey, ThumbViewer.Key, 0, REG_SZ, reinterpret_cast<const BYTE*>(ThumbViewer.Value), _tcslen(ThumbViewer.Value) + 1);

        RegCloseKey(hKey);
    }
    return ERROR_SUCCESS;
}

/*
    Called during deinstallation when the module "Windows Explorer Extensions" has
    been installed.
*/
extern "C" UINT __stdcall DeinstallExecSequenceEntry(MSIHANDLE handle)
{
    //MessageBox(NULL, TEXT("DeinstallExecSequenceEntry"), TEXT("Pythonmsi"), MB_OK | MB_ICONINFORMATION);
    HKEY hKey;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), &hKey) == ERROR_SUCCESS)
    {
        RegDeleteValue(hKey, ColumnHandler.Key);
        RegDeleteValue(hKey, InfotipHandler.Key);
        RegDeleteValue(hKey, PropHandler.Key);
        RegDeleteValue(hKey, ThumbViewer.Key);

        RegCloseKey(hKey);
    }
    return ERROR_SUCCESS;
}
