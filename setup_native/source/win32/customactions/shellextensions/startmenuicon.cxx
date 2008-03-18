/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: startmenuicon.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:55:01 $
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

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>


std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring   result;
    TCHAR   szDummy[1] = TEXT("");
    DWORD   nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return  result;
}

/*
    Called during installation to customize the start menu folder icon.
    See: http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_basics/shell_basics_extending/custom.asp
*/
extern "C" UINT __stdcall InstallStartmenuFolderIcon( MSIHANDLE handle )
{
    std::_tstring   sOfficeMenuFolder = GetMsiProperty( handle, TEXT("OfficeMenuFolder") );
    std::_tstring sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");

    //    MessageBox(NULL, sDesktopFile.c_str(), TEXT("OfficeMenuFolder"), MB_OK | MB_ICONINFORMATION);

    std::_tstring   sIconFile = GetMsiProperty( handle, TEXT("OFFICEINSTALLLOCATION") ) + TEXT("program\\soffice.exe");

    OSVERSIONINFO   osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &osverinfo );

    if (osverinfo.dwMajorVersion < 6 /* && osverinfo.dwMinorVersion  */ )
    {
        // This icon (18) is a Windows folder until XP Version (number is 0 based)
        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconFile"),
            sIconFile.c_str(),
            sDesktopFile.c_str() );

        // FYI: in tool 'ResHack' this icon can be found on position '19' (number is 1 based)
        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconIndex"),
            TEXT("18"),
            sDesktopFile.c_str() );
    }
    // else
    // {
    //     // at the moment there exists no Vista Icon, so we use the default folder icon.
    //     // add the icon into desktop/util/verinfo.rc
    // }

    // The value '0' is to avoid a message like "You Are Deleting a System Folder" warning when deleting or moving the folder.
    WritePrivateProfileString(
        TEXT(".ShellClassInfo"),
        TEXT("ConfirmFileOp"),
        TEXT("0"),
        sDesktopFile.c_str() );

    /*
      WritePrivateProfileString(
      TEXT(".ShellClassInfo"),
      TEXT("InfoTip"),
      TEXT("StarOffice Productivity Suite"),
      sDesktopFile.c_str() );
    */

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_HIDDEN );
    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_SYSTEM );


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallStartmenuFolderIcon(MSIHANDLE handle)
{
    std::_tstring   sOfficeMenuFolder = GetMsiProperty( handle, TEXT("OfficeMenuFolder") );
    std::_tstring sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_NORMAL );
    DeleteFile( sDesktopFile.c_str() );

    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_NORMAL );

    return ERROR_SUCCESS;
}
