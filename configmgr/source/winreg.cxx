/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cwchar>
#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "rtl/ustring.hxx"
#include "osl/file.h"
#include "osl/file.hxx"
#include "winreg.hxx"
#include "writemodfile.hxx"

#define MAX_KEY_LENGTH 255

namespace configmgr {

namespace {
// This is not a generic registry reader. We assume the following structure:
// Last element of Key becomes prop, first part is the path.
// Values can be the following: Value (string) and Final (dword, optional)
// For example the following registry setting:
// [HKEY_LOCAL_MACHINE\SOFTWARE\Policies\LibreOffice\org.openoffice.UserProfile\Data\o]
// "Value"="Example Corp."
// "Final"=dword:00000001
// becomes the following in configuration:
// <item oor:path="/org.openoffice.UserProfile/Data">
//     <prop oor:name="o" oor:finalized="true">
//         <value>Example Corp.</value>
//     </prop>
// </item>
void dumpWindowsRegistryKey(HKEY hKey, OUString aKeyName, oslFileHandle aFileHandle)
{
    HKEY hCurKey;

    if(RegOpenKeyExW(hKey, aKeyName.getStr(), 0, KEY_READ, &hCurKey) == ERROR_SUCCESS)
    {
        DWORD nSubKeys = 0;
        DWORD nValues = 0;
        DWORD nLongestValueNameLen, nLongestValueLen;
        // Query the number of subkeys
        RegQueryInfoKeyW(hCurKey, NULL, NULL, NULL, &nSubKeys, NULL, NULL, &nValues, &nLongestValueNameLen, &nLongestValueLen, NULL, NULL);
        if(nSubKeys)
        {
            //Look for subkeys in this key
            for(DWORD i = 0; i < nSubKeys; i++)
            {
                wchar_t buffKeyName[MAX_KEY_LENGTH];
                buffKeyName[0] = '\0';
                DWORD buffSize=MAX_KEY_LENGTH;
                OUString aSubkeyName;
                //Get subkey name
                RegEnumKeyExW(hCurKey, i, buffKeyName, &buffSize, NULL, NULL, NULL, NULL);

                //Make up full key name
                if(aKeyName.isEmpty())
                    aSubkeyName = aKeyName + OUString(buffKeyName);
                else
                    aSubkeyName = aKeyName + "\\" + OUString(buffKeyName);

                //Recursion, until no more subkeys are found
                dumpWindowsRegistryKey(hKey, aSubkeyName, aFileHandle);
            }
        }
        else if(nValues)
        {
            // No more subkeys, we are at a leaf
            wchar_t* pValueName = new wchar_t[nLongestValueNameLen + 1];
            wchar_t* pValue = new wchar_t[nLongestValueLen + 1];

            if(pValueName && pValue)
            {
                bool bFinal = false;
                OUString aValue;

                for(DWORD i = 0; i < nValues; ++i)
                {
                    DWORD nValueNameLen = nLongestValueNameLen + 1;
                    DWORD nValueLen = nLongestValueLen + 1;

                    RegEnumValueW(hCurKey, i, pValueName, &nValueNameLen, NULL, NULL, (LPBYTE)pValue, &nValueLen);
                    const wchar_t wsValue[] = L"Value";
                    const wchar_t wsFinal[] = L"Final";

                    if(!wcscmp(pValueName, wsValue))
                        aValue = OUString(pValue);
                    if(!wcscmp(pValueName, wsFinal) && *(DWORD*)pValue == 1)
                        bFinal = true;
                }
                sal_Int32 aLastSeparator = aKeyName.lastIndexOf('\\');
                OUString aPath = aKeyName.replaceAll("\\","/").copy(0, aLastSeparator);
                OUString aProp = aKeyName.copy(aLastSeparator + 1);

                writeData(aFileHandle, "<item oor:path=\"/");
                writeAttributeValue(aFileHandle, aPath);
                writeData(aFileHandle, "\"><prop oor:name=\"");
                writeAttributeValue(aFileHandle, aProp);
                writeData(aFileHandle, "\"");
                if(bFinal)
                    writeData(aFileHandle, " oor:finalized=\"true\"");
                writeData(aFileHandle, "><value>");
                writeValueContent(aFileHandle, aValue);
                writeData(aFileHandle, "</value></prop></item>\n");
                delete[] pValueName;
                delete[] pValue;
            }
        }
        RegCloseKey(hCurKey);
    }
}
}

bool dumpWindowsRegistry(OUString* pFileURL)
{
    HKEY hKey;
    if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\LibreOffice", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        SAL_INFO(
            "configmgr",
            ("Windows registry settings do not exist in HKLM\\SOFTWARE\\Policies\\LibreOffice"));
        return false;
    }

    oslFileHandle aFileHandle;
    switch (osl::FileBase::createTempFile(0, &aFileHandle, pFileURL)) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_ACCES:
        SAL_INFO(
            "configmgr",
            ("cannot create temp Windows registry dump (E_ACCES)"));
        return false;
    default:
        throw css::uno::RuntimeException(
            "cannot create temporary file",
            css::uno::Reference< css::uno::XInterface >());
    }
    writeData(
        aFileHandle,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
            " xmlns:oor=\"http://openoffice.org/2001/registry\""
            " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
            " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    dumpWindowsRegistryKey(hKey, "", aFileHandle);
    writeData(aFileHandle, "</oor:items>");
    oslFileError e = osl_closeFile(aFileHandle);
    if (e != osl_File_E_None)
        SAL_WARN("configmgr", "osl_closeFile failed with " << +e);
    RegCloseKey(hKey);
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
