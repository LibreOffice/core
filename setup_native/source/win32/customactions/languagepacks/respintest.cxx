/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: respintest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:32:49 $
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

#undef UNICODE
#undef _UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <malloc.h>
#include <assert.h>

#include <tchar.h>
#include <string>
#include <systools/win32/uwinapi.h>

#ifdef _WIN32_WINNT
#error YES
#endif

using namespace std;

namespace
{
    string GetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        string  result;
        TCHAR   szDummy[1] = TEXT("");
        DWORD   nChars = 0;

        if (MsiGetProperty(handle, sProperty.c_str(), szDummy, &nChars) == ERROR_MORE_DATA)
        {
            DWORD nBytes = ++nChars * sizeof(TCHAR);
            LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
            ZeroMemory( buffer, nBytes );
            MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
            result = buffer;
        }
        return result;
    }

    inline bool IsSetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        return (GetMsiProperty(handle, sProperty).length() > 0);
    }

    inline void UnsetMsiProperty(MSIHANDLE handle, const string& sProperty)
    {
        MsiSetProperty(handle, sProperty.c_str(), NULL);
    }

    inline void SetMsiProperty(MSIHANDLE handle, const string& sProperty, const string& sValue)
    {
        MsiSetProperty(handle, sProperty.c_str(), TEXT("1"));
    }

} // namespace

extern "C" UINT __stdcall GetUserInstallMode(MSIHANDLE handle)
{
    string sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    // MessageBox(NULL, sInstallPath.c_str(), "DEBUG", MB_OK);

    UnsetMsiProperty( handle, TEXT("INVALIDDIRECTORY") );
    UnsetMsiProperty( handle, TEXT("ISWRONGPRODUCT") );
    UnsetMsiProperty( handle, TEXT("PATCHISOLDER") );
    UnsetMsiProperty( handle, TEXT("ALLUSERS") );

    string sSetupiniPath = sInstallPath + TEXT("program\\setup.ini");

    TCHAR szValue[32767];

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ProductCode"),
        TEXT("INVALIDDIRECTORY"),
        szValue,
        elementsof(szValue),
        sSetupiniPath.c_str()
        );

    if ( !_tcsicmp( szValue, TEXT("INVALIDDIRECTORY") ) )
    {
        SetMsiProperty( handle, TEXT("INVALIDDIRECTORY"), TEXT("YES") );
        // MessageBox(NULL, "INVALIDDIRECTORY set", "DEBUG", MB_OK);
        return ERROR_SUCCESS;
    }

    string sBootstrapPath = sInstallPath + TEXT("program\\bootstrap.ini");

    szValue[0] = '\0';

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("buildid"),
        TEXT("ISWRONGPRODUCT"),
        szValue,
        elementsof(szValue),
        sBootstrapPath.c_str()
        );

    if ( !_tcsicmp( szValue, TEXT("ISWRONGPRODUCT") ) )
    {
        SetMsiProperty( handle, TEXT("ISWRONGPRODUCT"), TEXT("YES") );
        // MessageBox(NULL, "ISWRONGPRODUCT 1 set", "DEBUG", MB_OK);
        return ERROR_SUCCESS;
    }

    string ProductMajor = GetMsiProperty(handle, TEXT("PRODUCTMAJOR"));

    // Comparing the first three characters, for example "680"
    // If not equal, this version is not suited for patch or language pack

    if (_tcsnicmp(ProductMajor.c_str(), szValue, 3))
    {
        SetMsiProperty( handle, TEXT("ISWRONGPRODUCT"), TEXT("YES") );
        // MessageBox(NULL, "ISWRONGPRODUCT 2 set", "DEBUG", MB_OK);
        return ERROR_SUCCESS;
    }

    string isPatch = GetMsiProperty(handle, TEXT("ISPATCH"));

    if (isPatch=="1")
    {
        string ProductMinor = GetMsiProperty(handle, TEXT("PRODUCTMINOR"));
        int PatchProductMinor = atoi(ProductMinor.c_str());

        szValue[0] = '\0';

        GetPrivateProfileString(
            TEXT("Bootstrap"),
            TEXT("ProductMinor"),
            TEXT("106"),
            szValue,
            elementsof(szValue),
            sBootstrapPath.c_str()
            );

        int InstalledProductMinor = atoi(szValue);

        if ( InstalledProductMinor >= PatchProductMinor )
        {
            SetMsiProperty( handle, TEXT("PATCHISOLDER"), TEXT("YES") );
            // MessageBox(NULL, "PATCHISOLDER set", "DEBUG", MB_OK);
            return ERROR_SUCCESS;
        }
    }

    szValue[0] = '\0';

    GetPrivateProfileString(
        TEXT("Bootstrap"),
        TEXT("ALLUSERS"),
        TEXT(""),
        szValue,
        elementsof(szValue),
        sSetupiniPath.c_str()
        );

    if ( szValue[0] )
    {
        SetMsiProperty( handle, TEXT("ALLUSERS"), szValue );
        // MessageBox(NULL, "ALLUSERS set", "DEBUG", MB_OK);
    }

    return ERROR_SUCCESS;
}
