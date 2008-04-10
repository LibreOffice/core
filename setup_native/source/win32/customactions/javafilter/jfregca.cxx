/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jfregca.cxx,v $
 * $Revision: 1.5 $
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
//#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#include <windows.h>
#include <msi.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#if defined UNICODE
#define _UNICODE
#endif
#include <tchar.h>

//Simple function prototypes
bool update_activesync_regvalues(bool, bool, char** );
void createKeys(HKEY hKey, char **);
void deleteKeys(HKEY hKey, char **);
bool isMulti(MSIHANDLE);

//Simple data arrays for registry values
TCHAR *pxlData[8]= {
    "{C6AB3E74-9F4F-4370-8120-A8A6FABB7A7C}", //CLSID 1 - key name at InstalledFilters Key
    "{43887C67-4D5D-4127-BAAC-87A288494C7C}", //CLSID 2 - key value for Default Export
    ".pxl",                                   //Registry key for device type - already there if ActiveSync installerd
    ".sxc",                                   //New registry key for SO docs
    "InstalledFilters",                       //Sub-key of device/so doc key
    "DefaultImport",                          //Key name added at device/so level key
    "DefaultExport",                          //Key name added at device/so level key
    "Binary Copy",                            //Key value for DefaultImport
};

TCHAR *pswData[8] = {
    "{BDD611C3-7BAB-460F-8711-5B9AC9EF6020}", //CLSID 1 - key name at InstalledFilters Key
    "{CB43F086-838D-4FA4-B5F6-3406B9A57439}", //CLSID 2 - key value for Default Export
    ".psw",                                   //Registry key for device type - already there if ActiveSync installe
    ".sxw",                                   //New registry key for SO docs
    "InstalledFilters",                       //Sub-key of device/so doc key
    "DefaultImport",                          //Key name added at device/so level key
    "DefaultExport",                          //Key name added at device/so level key
    "Binary Copy",                            //Key value for DefaultImport
};


//index into registry value arrays
#define CLSID1 0
#define CLSID2 1
#define DEVICE_PATH 2
#define SO_PATH 3
#define IF_PATH 4
#define DEFIMPORT_KEY 5
#define DEFEXPORT_KEY 6
#define BC_VALUE 7

//Constants for Registry buffers
const int MAX_KEY_LENGTH=255;
const int MAX_VALUE_NAME=16383;

BOOL APIENTRY DllMain( HANDLE,
                       DWORD  ul_reason,
                       LPVOID
                     )
{
    switch (ul_reason)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

extern "C" UINT install_jf ( MSIHANDLE hModule ) {
    bool bMulti = isMulti(hModule);
#ifdef _JRGREG_DEBUG
    MessageBox(NULL, bMulti ? "Multi" : "Single", "Install", MB_OK);
#endif
    update_activesync_regvalues(bMulti, true, pxlData);
    update_activesync_regvalues(bMulti, true, pswData);

    return ERROR_SUCCESS;
}

extern "C" UINT uninstall_jf ( MSIHANDLE hModule ) {
    bool bMulti = isMulti(hModule);
#ifdef _JRGREG_DEBUG
    MessageBox(NULL, bMulti ? "Multi" : "Single", "Uninstall", MB_OK);
#endif
    update_activesync_regvalues(false, bMulti, pxlData);
    update_activesync_regvalues(false, bMulti, pswData);

    return ERROR_SUCCESS;
}

/**
    Determines if this is being installed on a per user or a machine wide basis
    @param hModule
    [in] a valid msi handle.


    @returns
    <TRUE/>if this is a multi-user install.
*/
bool isMulti( MSIHANDLE hModule ) {
    TCHAR* szValueBuf = NULL;
    DWORD cchValueBuf = 0;
    bool bRet = false;
    UINT uiStat =  MsiGetProperty(hModule, TEXT("ALLUSERS"), TEXT(""), &cchValueBuf);
    if (ERROR_MORE_DATA == uiStat)
    {
        ++cchValueBuf; // on output does not include terminating null, so add 1
        szValueBuf = new TCHAR[cchValueBuf];
        if (szValueBuf)
        {
            uiStat = MsiGetProperty(hModule, TEXT("ALLUSERS"), szValueBuf, &cchValueBuf);
        }
    }
    if (ERROR_SUCCESS != uiStat)
    {
        return false;
    }
    bRet = _tcscmp(szValueBuf, TEXT("1")) == 0;
    delete [] szValueBuf;

    return bRet;
}

/**
    Add or remove ActiveSync integration entries from the registry
    @param bMultiUser
    [in] <TRUE/>if this is a multiuser install (<FALSE/> for single user install)

    @param bInstall
    [in] <TRUE/>if installing

    @param data
    [in] an array of string containing names of registry keys and values


    @returns
    <TRUE/>if this is a multi-user install.
*/

bool update_activesync_regvalues(bool bMultiUser, bool bInstall, char **data) {
    bool bReturn = false;
    CHAR SUKey[]    = "Software\\Microsoft\\Windows CE Services\\Partners";
    CHAR MUKey[]    = "Software\\Microsoft\\Windows CE Services\\Filters";
    HKEY hKey;

    if (bMultiUser) {
        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)MUKey, 0, KEY_ALL_ACCESS, &hKey)) {
            return false;
        }
        if (bInstall) {
            createKeys(hKey, data);
        } else {
            deleteKeys(hKey, data);
        }
        bReturn = true;
    } else {
        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, (LPCSTR)SUKey, 0, KEY_ALL_ACCESS, &hKey)) {
            return false;
        }

        CHAR     achKey[MAX_KEY_LENGTH];   // buffer for subkey name
        DWORD    cbName;                   // size of name string
        CHAR     achClass[MAX_PATH] = "";  // buffer for class name
        DWORD    cchClassName = MAX_PATH;  // size of class string
        DWORD    cSubKeys=0;               // number of subkeys
        DWORD    cbMaxSubKey;              // longest subkey size
        DWORD    cchMaxClass;              // longest class string
        DWORD    cValues;              // number of values for key
        DWORD    cchMaxValue;          // longest value name
        DWORD    cbMaxValueData;       // longest value data
        DWORD    cbSecurityDescriptor; // size of security descriptor
        FILETIME ftLastWriteTime;      // last write time

        // Get the class name and the value count.
        if (ERROR_SUCCESS == RegQueryInfoKey(
            hKey,                    // key handle
            achClass,                // buffer for class name
            &cchClassName,           // size of class string
            NULL,                    // reserved
            &cSubKeys,               // number of subkeys
            &cbMaxSubKey,            // longest subkey size
            &cchMaxClass,            // longest class string
            &cValues,                // number of values for this key
            &cchMaxValue,            // longest value name
            &cbMaxValueData,         // longest value data
            &cbSecurityDescriptor,   // security descriptor
            &ftLastWriteTime)) {       // last write time

            if (cSubKeys) {
                for (DWORD i=0; i<cSubKeys; i++) {
                    cbName = 1024;
                    if (ERROR_SUCCESS == RegEnumKeyEx(hKey,i,achKey,&cbName,NULL,NULL,NULL,&ftLastWriteTime)) {
                        HKEY subKey;
                        if (ERROR_SUCCESS == RegOpenKeyEx(hKey, achKey, 0, KEY_ALL_ACCESS, &subKey)) {
                            if (ERROR_SUCCESS == RegOpenKeyEx(subKey, "Filters", 0, KEY_ALL_ACCESS, &subKey)) {
                                if (bInstall) {
                                    createKeys(subKey, data);
                                } else {
                                    deleteKeys(subKey, data);
                                }
                                RegCloseKey(subKey);
                            }
                        }
                    }
                }
            }

            bReturn = true;
        }
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return bReturn;
}

/**
    Create Registry Keys

    @param hKey
    [in] Handle to the parent registry key

    @param data
    [in] an array of string containing names of registry keys and values
*/

void createKeys(HKEY hKey, char **data) {

    LPCSTR clsid1     = data[CLSID1];
    LPCSTR clsid2     = data[CLSID2];
    LPCSTR devicePath = data[DEVICE_PATH];
    LPCSTR soPath     = data[SO_PATH];
    LPCSTR defImport  = data[DEFIMPORT_KEY];
    LPCSTR defExport  = data[DEFEXPORT_KEY];
    LPCSTR binaryCopy = data[BC_VALUE];
    LPCSTR IFPath     = data[IF_PATH];

    HKEY deviceKey, deviceIFKey, soKey, soIFKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(hKey,devicePath,0,KEY_ALL_ACCESS, &deviceKey)) {
        if (ERROR_SUCCESS == RegOpenKeyEx(deviceKey,IFPath,0,KEY_ALL_ACCESS, &deviceIFKey)) {
            RegSetValueEx(deviceIFKey, clsid1, 0, REG_SZ, NULL, NULL);
        }
    }

    if (ERROR_SUCCESS == RegCreateKeyEx(hKey, soPath, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &soKey, NULL)) {
        RegSetValueEx(soKey, defExport, 0, REG_SZ, (LPBYTE)binaryCopy, strlen(binaryCopy));
        RegSetValueEx(soKey, defImport, 0, REG_SZ, (LPBYTE)clsid2, strlen(clsid2));


        if (ERROR_SUCCESS == RegCreateKeyEx(soKey, IFPath, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &soIFKey, NULL)) {
            RegSetValueEx(soIFKey, clsid2, 0, REG_SZ, NULL, NULL);
        }
    }
}

/**
    Delete registry keys

    @param hKey
    [in] Handle to the parent registry key
*/
void deleteKeys(HKEY hKey, TCHAR **data) {
    LPCSTR clsid1     = data[CLSID1];
    LPCSTR clsid2     = data[CLSID2];
    LPCSTR devicePath = data[DEVICE_PATH];
    LPCSTR soPath     = data[SO_PATH];
    LPCSTR defImport  = data[DEFIMPORT_KEY];
    LPCSTR defExport  = data[DEFEXPORT_KEY];
    LPCSTR IFPath     = data[IF_PATH];

    HKEY deviceKey, deviceIFKey, soKey, soIFKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(hKey,devicePath,0,KEY_ALL_ACCESS, &deviceKey)) {
        if (ERROR_SUCCESS == RegOpenKeyEx(deviceKey,IFPath,0,KEY_ALL_ACCESS, &deviceIFKey)) {
            RegDeleteValue(deviceIFKey, clsid1);
        }
    }

    if (ERROR_SUCCESS == RegOpenKeyEx(hKey, soPath, 0, KEY_ALL_ACCESS, &soKey)) {
        RegDeleteValue(soKey, defExport);
        RegDeleteValue(soKey, defImport);

        if (ERROR_SUCCESS == RegOpenKeyEx(soKey, IFPath, 0, KEY_ALL_ACCESS, &soIFKey)) {
            RegDeleteValue(soIFKey, clsid2);
            RegCloseKey(soIFKey);
            RegDeleteKey(soKey, IFPath);
        }
        RegCloseKey(soKey);
        RegDeleteKey(hKey, soPath);
    }
}
