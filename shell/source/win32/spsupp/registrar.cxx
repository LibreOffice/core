/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <registrar.hpp>
#include <wchar.h>
#include <objbase.h>

namespace {

    HRESULT RegRead(HKEY hRootKey, const wchar_t* subKey, const wchar_t* valName, wchar_t* valData, size_t cchData)
    {
        HKEY hKey;
        long iRetVal = RegCreateKeyExW(
            hRootKey,
            subKey,
            0,
            nullptr,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,
            nullptr,
            &hKey,
            nullptr);
        if (iRetVal != ERROR_SUCCESS)
            return HRESULT_FROM_WIN32(iRetVal);

        DWORD cbData = cchData * sizeof(valData[0]);
        DWORD dwType;
        iRetVal = RegQueryValueExW(hKey, valName, nullptr, &dwType, reinterpret_cast<LPBYTE>(valData), &cbData);
        RegCloseKey(hKey);
        if ((iRetVal == ERROR_SUCCESS) && (dwType != REG_SZ))
        {
            return E_FAIL;
        }
        return HRESULT_FROM_WIN32(iRetVal);
    }

    HRESULT RegWrite(HKEY hRootKey, const wchar_t* subKey, const wchar_t* valName, const wchar_t* valData, HKEY *hKeyResult = nullptr)
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

        if (valData)
        {
            DWORD cbData = static_cast<DWORD>(wcslen(valData)*sizeof(valData[0]));
            iRetVal = RegSetValueExW(hKey, valName, 0, REG_SZ, reinterpret_cast<const BYTE *>(valData), cbData);
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

} // namespace

// see http://stackoverflow.com/questions/284619
// see https://msdn.microsoft.com/en-us/library/ms691424
// see https://msdn.microsoft.com/en-us/library/ms694514

Registrar::Registrar(REFIID riidCLSID)
{
    m_ConstructionResult = (StringFromGUID2(riidCLSID, m_sCLSID, nGUIDlen) == 0) ?
        E_UNEXPECTED: S_OK;
}

HRESULT Registrar::RegisterObject(REFIID riidTypeLib, const wchar_t* sProgram,
                                  const wchar_t* sComponent, std::initializer_list<int> aVersions,
                                  const wchar_t* Path)
{
    if (!wcslen(sComponent) || !wcslen(sProgram))
        return E_INVALIDARG;

    if (FAILED(m_ConstructionResult))
        return m_ConstructionResult;

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

    wchar_t sBufKey[MAX_PATH];
    wchar_t sBufVal[MAX_PATH];

    // CLSID
    swprintf(sBufKey, MAX_PATH, L"CLSID\\%s", m_sCLSID);
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
    return RegisterProgIDs(sProgram, sComponent, aVersions);
}

HRESULT Registrar::UnRegisterObject(const wchar_t* sProgram, const wchar_t* sComponent,
                                    std::initializer_list<int> aVersions)
{
    if (FAILED(m_ConstructionResult))
        return m_ConstructionResult;
    // ProgID
    UnRegisterProgIDs(sProgram, sComponent, aVersions);
    // CLSID
    wchar_t sBuf[MAX_PATH];
    swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\InProcServer32", m_sCLSID);
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\ProgId", m_sCLSID);
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\Programmable", m_sCLSID);
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    swprintf(sBuf, MAX_PATH, L"CLSID\\%s\\TypeLib", m_sCLSID);
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    swprintf(sBuf, MAX_PATH, L"CLSID\\%s", m_sCLSID);
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    return S_OK;
}

HRESULT Registrar::RegisterProgID(const wchar_t* sProgram, const wchar_t* sComponent, int nVersion, bool bSetDefault)
{
    // HKEY_CLASSES_ROOT
    //    \MyLibrary.MyControl
    //       (default) = "MyLibrary MyControl Class"
    //       \CurVer
    //          (default) = "MyLibrary.MyControl.1"
    //    \MyLibrary.MyControl.1
    //       (default) = "MyLibrary MyControl Class"
    //       \CLSID
    //          (default) = "{XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}"
    if (FAILED(m_ConstructionResult))
        return m_ConstructionResult;
    wchar_t sBufKey[MAX_PATH];
    swprintf(sBufKey, MAX_PATH, L"%s.%s.%d", sProgram, sComponent, nVersion);
    wchar_t sBufVal[MAX_PATH];
    swprintf(sBufVal, MAX_PATH, L"%s %s Class", sProgram, sComponent);
    RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", sBufVal);
    swprintf(sBufKey, MAX_PATH, L"%s.%s.%d\\CLSID", sProgram, sComponent, nVersion);
    HRESULT hr = RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", m_sCLSID);
    if (SUCCEEDED(hr) && bSetDefault)
    {
        swprintf(sBufKey, MAX_PATH, L"%s.%s", sProgram, sComponent);
        swprintf(sBufVal, MAX_PATH, L"%s %s Class", sProgram, sComponent);
        hr = RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", sBufVal);
        swprintf(sBufKey, MAX_PATH, L"%s.%s\\CurVer", sProgram, sComponent);
        swprintf(sBufVal, MAX_PATH, L"%s.%s.%d", sProgram, sComponent, nVersion);
        hr = RegWrite(HKEY_CLASSES_ROOT, sBufKey, L"", sBufVal);
    }
    return hr;
}

HRESULT Registrar::RegisterProgIDs(const wchar_t* sProgram, const wchar_t* sComponent,
                                   std::initializer_list<int> aVersions)
{
    HRESULT hr = S_OK;
    bool bDefaultRegistered = false;
    for (int nVersion : aVersions)
    {
        if (SUCCEEDED(hr))
        {
            hr = RegisterProgID(sProgram, sComponent, nVersion, !bDefaultRegistered);
            bDefaultRegistered = true;
        }
    }
    return hr;
}

HRESULT Registrar::UnRegisterProgID(const wchar_t* sProgram, const wchar_t* sComponent, int nVersion)
{
    if (FAILED(m_ConstructionResult))
        return m_ConstructionResult;
    wchar_t sBuf[MAX_PATH];
    swprintf(sBuf, MAX_PATH, L"%s.%s.%d\\CLSID", sProgram, sComponent, nVersion);
    wchar_t sCurCLSID[nGUIDlen];
    HRESULT hr = RegRead(HKEY_CLASSES_ROOT, sBuf, L"", sCurCLSID, nGUIDlen);
    if (FAILED(hr))
        return hr;
    if (wcsncmp(sCurCLSID, m_sCLSID, nGUIDlen) != 0)
    {
        // The ProgID points to a different CLSID; most probably it's intercepted
        // by a different application, so don't remove it
        return S_FALSE;
    }
    RegDel(HKEY_CLASSES_ROOT, sBuf);
    swprintf(sBuf, MAX_PATH, L"%s.%s.%d", sProgram, sComponent, nVersion);
    hr = RegDel(HKEY_CLASSES_ROOT, sBuf);

    wchar_t sBufKey[MAX_PATH];
    swprintf(sBufKey, MAX_PATH, L"%s.%s\\CurVer", sProgram, sComponent);
    wchar_t sBufVal[MAX_PATH];
    if (SUCCEEDED(RegRead(HKEY_CLASSES_ROOT, sBufKey, L"", sBufVal, MAX_PATH)) && (wcsncmp(sBufVal, sBuf, MAX_PATH) == 0))
    {
        // Only unreg default if this version is current default
        RegDel(HKEY_CLASSES_ROOT, sBufKey);
        swprintf(sBuf, MAX_PATH, L"%s.%s", sProgram, sComponent);
        HRESULT hr1 = RegDel(HKEY_CLASSES_ROOT, sBuf);
        // Always return a failure result if we failed somewhere
        if (FAILED(hr1))
            hr = hr1;
    }
    return hr;
}

HRESULT Registrar::UnRegisterProgIDs(const wchar_t* sProgram, const wchar_t* sComponent,
                                    std::initializer_list<int> aVersions)
{
    HRESULT hr = S_OK;
    // Try all ProgIDs regardless of error, but make sure to return failure result if some failed
    for (int nVersion : aVersions)
    {
        HRESULT hrLast = UnRegisterProgID(sProgram, sComponent, nVersion);
        if (SUCCEEDED(hr))
            hr = hrLast;
    }
    return hr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
