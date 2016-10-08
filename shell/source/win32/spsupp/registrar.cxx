/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "registrar.hpp"
#include "stdio.h"

namespace {

    HRESULT RegWrite(HKEY hRootKey, const wchar_t* subKey, const wchar_t* keyName, const wchar_t* keyValue, HKEY *hKeyResult = nullptr)
    {
        HKEY hKey;
        long iRetVal = RegCreateKeyExW(
            hRootKey,
            subKey,
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            nullptr,
            &hKey,
            nullptr);
        if (iRetVal != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(iRetVal);

        if (keyValue)
        {
            DWORD cbData = static_cast<DWORD>(wcslen(keyValue)*sizeof(keyValue[0]));
            iRetVal = RegSetValueExW(hKey, keyName, 0, REG_SZ, reinterpret_cast<const BYTE *>(keyValue), cbData);
        }

        if (hKeyResult && (iRetVal == ERROR_SUCCESS))
            *hKeyResult = hKey;
        else
            RegCloseKey(hKey);

        return HRESULT_FROM_WIN32(iRetVal);
    }

    HRESULT RegDel(HKEY hRootKey, const wchar_t* subKey)
    {
        long iRetVal = RegDeleteKeyW(hRootKey, subKey);
        return HRESULT_FROM_WIN32(iRetVal);
    }

    const int nGUIDlen = 40;

}

namespace Registrar {

    // see http://stackoverflow.com/questions/284619
    // see https://msdn.microsoft.com/en-us/library/ms691424
    // see https://msdn.microsoft.com/en-us/library/ms694514

    HRESULT RegisterObject(REFIID riidCLSID,
                           REFIID riidTypeLib,
                           const wchar_t* sProgram,
                           const wchar_t* sComponent,
                           const wchar_t* Path)
    {
        if (!wcslen(sComponent) || !wcslen(sProgram))
            return E_INVALIDARG;

        wchar_t sCLSID[nGUIDlen];
        if (::StringFromGUID2(riidCLSID, sCLSID, nGUIDlen) == 0)
            return E_UNEXPECTED;

        // HKEY_CLASSES_ROOT
        //    \CLSID
        //       \{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
        //          (default) = "MyLibrary MyControl Class"
        //          \InprocServer32
        //             (default) = "c:\foo\control.dll"
        //             ThreadingModel = "Apartment"
        //          \ProgID
        //             (default) = "MyLibrary.MyControl"
        //          \Programmable
        //          \TypeLib
        //             (default) = "{YYYYYYYY-YYYY-YYYY-YYYY-YYYYYYYYYYYY}"
        //    \MyLibrary.MyControl
        //       \CLSID
        //          (default) = "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}"

        wchar_t sBufKey[MAX_PATH];
        wchar_t sBufVal[MAX_PATH];

        // CLSID
        swprintf(sBufKey, MAX_PATH, L"CLSID\\%s", sCLSID);
        swprintf(sBufVal, MAX_PATH, L"%s %s Class", sProgram, sComponent);
        HKEY hKeyCLSID;
        HRESULT hr = RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", sBufVal, &hKeyCLSID);
        if (FAILED(hr))
            return hr;
        {
            class HKeyGuard {
            public:
                HKeyGuard(HKEY aKey) : m_hKey(aKey) {}
                ~HKeyGuard() { RegCloseKey(m_hKey); }
            private:
                HKEY m_hKey;
            };

            HKeyGuard hKeyCLSIDGuard(hKeyCLSID);

            // InprocServer32
            HKEY hKeyInprocServer32;
            hr = RegWrite(hKeyCLSID, L"InprocServer32", L"", Path, &hKeyInprocServer32);
            if (FAILED(hr))
                return hr;
            {
                HKeyGuard hKeyInProcServer32Guard(hKeyInprocServer32);
                hr = RegWrite(hKeyInprocServer32, L"", L"ThreadingModel", L"Apartment");
                if (FAILED(hr))
                    return hr;
            }

            // ProgID
            swprintf(sBufVal, MAX_PATH, L"%s.%s", sProgram, sComponent);
            hr = RegWrite(hKeyCLSID, L"ProgID", L"", sBufVal);
            if (FAILED(hr))
                return hr;

            // Programmable
            hr = RegWrite(hKeyCLSID, L"Programmable", nullptr, nullptr);
            if (FAILED(hr))
                return hr;

            // TypeLib
            if (::StringFromGUID2(riidTypeLib, sBufVal, nGUIDlen) == 0)
                return E_UNEXPECTED;
            hr = RegWrite(hKeyCLSID, L"TypeLib", L"", sBufVal);
            if (FAILED(hr))
                return hr;
        }

        // ProgID
        swprintf(sBufKey, MAX_PATH, L"%s.%s\\CLSID", sProgram, sComponent);
        return RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", sCLSID);
    }

    HRESULT UnRegisterObject(REFIID riidCLSID, const wchar_t* LibId, const wchar_t* ClassId)
    {
        wchar_t sCLSID[nGUIDlen];
        wchar_t sBuf[MAX_PATH];
        if (::StringFromGUID2(riidCLSID, sCLSID, nGUIDlen) == 0)
            return E_UNEXPECTED;
        // ProgID
        swprintf(sBuf, MAX_PATH, L"%s.%s\\CLSID", LibId, ClassId);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        swprintf(sBuf, MAX_PATH, L"%s.%s", LibId, ClassId);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        // CLSID
        swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\InProcServer32", sCLSID);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\ProgId", sCLSID);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\Programmable", sCLSID);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\TypeLib", sCLSID);
        RegDel(HKEY_CLASSES_ROOT, sBuf);
        swprintf(sBuf, MAX_PATH, L"CLSID\\%s", sCLSID);
        return RegDel(HKEY_CLASSES_ROOT, sBuf);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
