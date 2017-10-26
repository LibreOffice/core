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
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/file.h>
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include "winreg.hxx"
#include "writemodfile.hxx"

#define MAX_KEY_LENGTH 255

namespace configmgr {

namespace {
// This is not a generic registry reader. We assume the following structure:
// Last element of Key becomes prop, first part is the path and optionally nodes,
// when the node has oor:op attribute.
// Values can be the following: Value (string), Type (string, optional),
// Final (dword, optional), External (dword, optional)
//
// For example the following registry setting:
// [HKEY_LOCAL_MACHINE\SOFTWARE\Policies\LibreOffice\org.openoffice.UserProfile\Data\o]
// "Value"="Example Corp."
// "Final"=dword:00000001
// becomes the following in configuration:
// <!-- set the Company name -->
// <item oor:path="/org.openoffice.UserProfile/Data">
//     <prop oor:name="o" oor:finalized="true">
//         <value>Example Corp.</value>
//     </prop>
// </item>
//
// Another example:
// [HKEY_LOCAL_MACHINE\SOFTWARE\Policies\LibreOffice\org.openoffice.Office.OptionsDialog\OptionsDialogGroups\ProductName/#fuse\Pages\Java/#fuse\Hide]
// "Value"="true"
// becomes the following in configuration:
// <!-- Hide Tools - Options - LibreOffice - Advanced panel -->
// <item oor:path="/org.openoffice.Office.OptionsDialog/OptionsDialogGroups">
//     <node oor:name="ProductName" oor:op="fuse">
//         <node oor:name="Pages">
//             <node oor:name="Java" oor:op="fuse">
//                 <prop oor:name="Hide">
//                     <value>true</value>
//                 </prop>
//             </node>
//         </node>
//     </node>
// </item>
//
// Third example (property of an extensible group -> needs type):
// [HKEY_LOCAL_MACHINE\SOFTWARE\Policies\LibreOffice\org.openoffice.Office.Jobs\Jobs\org.openoffice.Office.Jobs:Job['UpdateCheck']\Arguments\AutoCheckEnabled]
// "Value"="false"
// "Final"=dword:00000001
// "Type"="xs:boolean"
// becomes the following in configuration:
// <item oor:path="/org.openoffice.Office.Jobs/Jobs/org.openoffice.Office.Jobs:Job['UpdateCheck']/Arguments">
//     <prop oor:name="AutoCheckEnabled" oor:type="xs:boolean" oor:finalized="true">
//         <value>false</value>
//     </prop>
// </item>
//
// External (component data) example:
// [HKEY_CURRENT_USER\Software\Policies\LibreOffice\org.openoffice.UserProfile\Data\o]
// "Value"="com.sun.star.configuration.backend.LdapUserProfileBe company"
// "Final"=dword:00000001
// "External"=dword:00000001
// becomes the following in configuration:
// <item oor:path="/org.openoffice.UserProfile/Data">
//     <prop oor:name="o" oor:finalized="true">
//         <value oor:external="com.sun.star.configuration.backend.LdapUserProfileBe company"/>
//     </prop>
// </item>

void dumpWindowsRegistryKey(HKEY hKey, OUString const & aKeyName, TempFile &aFileHandle)
{
    HKEY hCurKey;

    if(RegOpenKeyExW(
           hKey, o3tl::toW(aKeyName.getStr()), 0,
           KEY_READ, &hCurKey)
       == ERROR_SUCCESS)
    {
        DWORD nSubKeys = 0;
        DWORD nValues = 0;
        DWORD nLongestValueNameLen, nLongestValueLen;
        // Query the number of subkeys
        RegQueryInfoKeyW(hCurKey, nullptr, nullptr, nullptr, &nSubKeys, nullptr, nullptr, &nValues, &nLongestValueNameLen, &nLongestValueLen, nullptr, nullptr);
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
                RegEnumKeyExW(hCurKey, i, buffKeyName, &buffSize, nullptr, nullptr, nullptr, nullptr);

                //Make up full key name
                if(aKeyName.isEmpty())
                    aSubkeyName = aKeyName + OUString(o3tl::toU(buffKeyName));
                else
                    aSubkeyName = aKeyName + "\\" + OUString(o3tl::toU(buffKeyName));

                //Recursion, until no more subkeys are found
                dumpWindowsRegistryKey(hKey, aSubkeyName, aFileHandle);
            }
        }
        else if(nValues)
        {
            // No more subkeys, we are at a leaf
            auto pValueName = std::unique_ptr<wchar_t[]>(
                new wchar_t[nLongestValueNameLen + 1]);
            auto pValue = std::unique_ptr<wchar_t[]>(
                new wchar_t[nLongestValueLen/sizeof(wchar_t) + 1]);

            bool bFinal = false;
            bool bExternal = false;
            OUString aValue;
            OUString aType;

            for(DWORD i = 0; i < nValues; ++i)
            {
                DWORD nValueNameLen = nLongestValueNameLen + 1;
                DWORD nValueLen = nLongestValueLen + 1;

                RegEnumValueW(hCurKey, i, pValueName.get(), &nValueNameLen, nullptr, nullptr, reinterpret_cast<LPBYTE>(pValue.get()), &nValueLen);

                if (!wcscmp(pValueName.get(), L"Value"))
                    aValue = o3tl::toU(pValue.get());
                else if (!wcscmp(pValueName.get(), L"Type"))
                    aType = o3tl::toU(pValue.get());
                else if (!wcscmp(pValueName.get(), L"Final"))
                {
                    if (*reinterpret_cast<DWORD*>(pValue.get()) == 1)
                        bFinal = true;
                }
                else if (!wcscmp(pValueName.get(), L"External"))
                {
                    if (*reinterpret_cast<DWORD*>(pValue.get()) == 1)
                        bExternal = true;
                }
            }
            // type and external are mutually exclusive
            if (bExternal)
                aType.clear();

            sal_Int32 aLastSeparator = aKeyName.lastIndexOf('\\');
            OUString aPathAndNodes = aKeyName.copy(0, aLastSeparator);
            OUString aProp = aKeyName.copy(aLastSeparator + 1);
            bool bHasNode = false;
            sal_Int32 nCloseNode = 0;

            aFileHandle.writeString("<item oor:path=\"");
            for(sal_Int32 nIndex = 0;; ++nIndex)
            {
                OUString aNextPathPart = aPathAndNodes.getToken(nIndex, '\\');

                if(!aNextPathPart.isEmpty())
                {
                    if((aNextPathPart.lastIndexOf("/#") != -1) || bHasNode)
                    {
                        bHasNode = true;
                        nCloseNode++;
                        aFileHandle.writeString("\"><node oor:name=\"");
                        sal_Int32 nCommandSeparator = aNextPathPart.lastIndexOf('#');
                        if(nCommandSeparator != -1)
                        {
                            OUString aNodeOp = aNextPathPart.copy(nCommandSeparator + 1);
                            writeAttributeValue(aFileHandle, aNextPathPart.copy(0, nCommandSeparator - 1));
                            aFileHandle.writeString("\" oor:op=\"");
                            writeAttributeValue(aFileHandle, aNodeOp);
                        }
                        else
                        {
                            writeAttributeValue(aFileHandle, aNextPathPart);
                        }
                    }
                    else
                    {
                        writeAttributeValue(
                            aFileHandle, OUString("/" + aNextPathPart));
                    }
                }
                else
                {
                    aFileHandle.writeString("\">");
                    break;
                }
            }

            aFileHandle.writeString("<prop oor:name=\"");
            writeAttributeValue(aFileHandle, aProp);
            aFileHandle.writeString("\"");
            if(!aType.isEmpty())
            {
                aFileHandle.writeString(" oor:type=\"");
                writeAttributeValue(aFileHandle, aType);
                aFileHandle.writeString("\"");
            }
            if(bFinal)
                aFileHandle.writeString(" oor:finalized=\"true\"");
            aFileHandle.writeString("><value");
            if (bExternal)
            {
                aFileHandle.writeString(" oor:external=\"");
                writeAttributeValue(aFileHandle, aValue);
                aFileHandle.writeString("\"/");
            }
            else
            {
                aFileHandle.writeString(">");
                writeValueContent(aFileHandle, aValue);
                aFileHandle.writeString("</value");
            }
            aFileHandle.writeString("></prop>");
            for(; nCloseNode > 0; nCloseNode--)
                aFileHandle.writeString("</node>");
            aFileHandle.writeString("</item>\n");
        }
        RegCloseKey(hCurKey);
    }
}
}

bool dumpWindowsRegistry(OUString* pFileURL, WinRegType eType)
{
    HKEY hKey;
    HKEY hDomain = eType == LOCAL_MACHINE ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
    if(RegOpenKeyExW(hDomain, L"SOFTWARE\\Policies\\LibreOffice", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        SAL_INFO(
            "configmgr",
            ("Windows registry settings do not exist in HKLM\\SOFTWARE\\Policies\\LibreOffice"));
        return false;
    }

    TempFile aFileHandle;
    switch (osl::FileBase::createTempFile(nullptr, &aFileHandle.handle, pFileURL)) {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_ACCES:
        SAL_INFO(
            "configmgr",
            ("cannot create temp Windows registry dump (E_ACCES)"));
        return false;
    default:
        throw css::uno::RuntimeException(
            "cannot create temporary file");
    }
    aFileHandle.url = *pFileURL;
    aFileHandle.writeString(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<oor:items"
        " xmlns:oor=\"http://openoffice.org/2001/registry\""
        " xmlns:xs=\"http://www.w3.org/2001/XMLSchema\""
        " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    dumpWindowsRegistryKey(hKey, "", aFileHandle);
    aFileHandle.writeString("</oor:items>");
    oslFileError e = aFileHandle.closeWithoutUnlink();
    if (e != osl_File_E_None)
        SAL_WARN("configmgr", "osl_closeFile failed with " << +e);
    RegCloseKey(hKey);
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
