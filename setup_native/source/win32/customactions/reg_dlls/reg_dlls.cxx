/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iomanip>
#include <memory>
#include <string>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>
#include <msiquery.h>

namespace
{
template <typename IntType> std::string Num2Hex(IntType n)
{
    std::stringstream sMsg;
    sMsg << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(n) * 2) << std::hex
         << n;
    return sMsg.str();
}

template <typename IntType> std::string Num2Dec(IntType n)
{
    std::stringstream sMsg;
    sMsg << n;
    return sMsg.str();
}

std::string Win32ErrorMessage(const char* sFunc, DWORD nWin32Error)
{
    std::stringstream sMsg;
    sMsg << sFunc << " failed with Win32 error code " << Num2Hex(nWin32Error) << "!";

    return sMsg.str();
}

void ThrowHResult(const char* sFunc, HRESULT hr)
{
    std::stringstream sMsg;
    sMsg << sFunc << " failed (HRESULT = " << Num2Hex(hr) << ")!";

    throw std::exception(sMsg.str().c_str());
}

void CheckHResult(const char* sFunc, HRESULT hr)
{
    if (FAILED(hr))
        ThrowHResult(sFunc, hr);
}

void ThrowWin32Error(const char* sFunc, DWORD nWin32Error)
{
    throw std::exception(Win32ErrorMessage(sFunc, nWin32Error).c_str());
}

void ThrowLastError(const char* sFunc) { ThrowWin32Error(sFunc, GetLastError()); }

void CheckWin32Error(const char* sFunc, DWORD nWin32Error)
{
    if (nWin32Error != ERROR_SUCCESS)
        ThrowWin32Error(sFunc, nWin32Error);
}

std::wstring GetKnownFolder(const KNOWNFOLDERID& rfid)
{
    PWSTR sPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(rfid, KF_FLAG_DEFAULT, nullptr, &sPath);
    CheckHResult("SHGetKnownFolderPath", hr);
    std::wstring sResult(sPath);
    CoTaskMemFree(sPath);
    return sResult;
}

void WriteLogElem(MSIHANDLE hInst, MSIHANDLE hRecord, std::ostringstream& sTmpl, UINT)
{
    MsiRecordSetStringA(hRecord, 0, sTmpl.str().c_str());
    MsiProcessMessage(hInst, INSTALLMESSAGE_INFO, hRecord);
}

void RecSetString(MSIHANDLE hRec, UINT nField, LPCSTR sVal)
{
    MsiRecordSetStringA(hRec, nField, sVal);
}

void RecSetString(MSIHANDLE hRec, UINT nField, LPCWSTR sVal)
{
    MsiRecordSetStringW(hRec, nField, sVal);
}

template <class S1, class... SOther>
void WriteLogElem(MSIHANDLE hInst, MSIHANDLE hRec, std::ostringstream& sTmpl, UINT nField,
                  const S1& elem, const SOther&... others);

template <class Ch, class... SOther>
void WriteLogElem(MSIHANDLE hInst, MSIHANDLE hRec, std::ostringstream& sTmpl, UINT nField,
                  const Ch* elem, const SOther&... others)
{
    sTmpl << " [" << nField << "]";
    RecSetString(hRec, nField, elem);
    WriteLogElem(hInst, hRec, sTmpl, nField + 1, others...);
}

template <class S1, class... SOther>
void WriteLogElem(MSIHANDLE hInst, MSIHANDLE hRec, std::ostringstream& sTmpl, UINT nField,
                  const S1& elem, const SOther&... others)
{
    WriteLogElem(hInst, hRec, sTmpl, nField, elem.c_str(), others...);
}

std::string sLogPrefix;

template <class... StrType> void WriteLog(MSIHANDLE hInst, const StrType&... elements)
{
    PMSIHANDLE hRec = MsiCreateRecord(sizeof...(elements));
    if (!hRec)
        return;

    std::ostringstream sTemplate;
    sTemplate << sLogPrefix;
    WriteLogElem(hInst, hRec, sTemplate, 1, elements...);
}

std::wstring MsiGetPropertyW(MSIHANDLE hInst, LPCWSTR szName)
{
    std::wstring sResult;
    DWORD nSz = 0;
    UINT nRet = ::MsiGetPropertyW(hInst, szName, const_cast<wchar_t*>(L""), &nSz);
    if (nRet == ERROR_MORE_DATA)
    {
        ++nSz;
        auto buf = std::make_unique<wchar_t[]>(nSz);
        CheckWin32Error("MsiGetPropertyW", ::MsiGetPropertyW(hInst, szName, buf.get(), &nSz));
        sResult = buf.get();
        WriteLog(hInst, "Property", szName, "=", sResult);
    }
    else
        CheckWin32Error("MsiGetPropertyW", nRet);

    return sResult;
}

typedef std::unique_ptr<void, decltype(&CloseHandle)> CloseHandleGuard;
CloseHandleGuard Guard(HANDLE h) { return CloseHandleGuard(h, CloseHandle); }

void RegDLL(MSIHANDLE hInst, const std::wstring& sArgs, bool bUnreg)
{
    static std::wstring sRegSvr32 = GetKnownFolder(FOLDERID_System) + L"\\regsvr32.exe";

    try
    {
        std::wstring sCmd = L"\"" + sRegSvr32 + L"\" /s ";
        if (bUnreg)
            sCmd += L"/u ";
        sCmd += sArgs;
        WriteLog(hInst, "Prepared regsvr32 command:", sCmd);

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(sRegSvr32.c_str(), const_cast<LPWSTR>(sCmd.c_str()), nullptr, nullptr,
                            FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
            ThrowLastError("CreateProcessW");
        auto aCloseProcHandleGuard(Guard(pi.hProcess));
        WriteLog(hInst, "CreateProcessW succeeded");

        DWORD nWaitResult = WaitForSingleObject(pi.hProcess, INFINITE);
        if (nWaitResult != WAIT_OBJECT_0)
            ThrowWin32Error("WaitForSingleObject", nWaitResult);

        DWORD nExitCode = 0;
        if (!GetExitCodeProcess(pi.hProcess, &nExitCode))
            ThrowLastError("GetExitCodeProcess");

        WriteLog(hInst, "regsvr32 returned:", Num2Dec(nExitCode));
    }
    catch (std::exception& e)
    {
        WriteLog(hInst, e.what());
    }
}

void ProcessCustomActionData(MSIHANDLE hInst, bool bUnreg)
{
    WriteLog(hInst, "Checking value of CustomActionData");
    std::wstring sCustomActionData = MsiGetPropertyW(hInst, L"CustomActionData");
    WriteLog(hInst, "Got CustomActionData value:", sCustomActionData);
    std::wstringstream ss(sCustomActionData);
    std::wstring sToken;
    while (std::getline(ss, sToken, L'|'))
    {
        if (!sToken.empty())
        {
            RegDLL(hInst, sToken, bUnreg);
        }
    }
}
} // namespace

// Deferred action "reg_dlls" that must be run from system account. Receives a list of regsvr32
// arguments: DLLs which need registering, and possibly /i argument with its parameter.
extern "C" __declspec(dllexport) UINT __stdcall RegDLLs(MSIHANDLE hInstall)
{
    sLogPrefix = "RegDLLs:";
    WriteLog(hInstall, "started");

    ProcessCustomActionData(hInstall, false);
    return ERROR_SUCCESS;
}

// Deferred action "unreg_dlls" that must be run from system account. Receives a list of regsvr32
// arguments: DLLs which need registering, and possibly /i argument with its parameter.
extern "C" __declspec(dllexport) UINT __stdcall UnregDLLs(MSIHANDLE hInstall)
{
    sLogPrefix = "UnregDLLs:";
    WriteLog(hInstall, "started");

    ProcessCustomActionData(hInstall, true);
    return ERROR_SUCCESS;
}

// Immediate action "prep_reg_unreg_dlls". Checks states of the features to prepare custom action data
// for reg_dlls and unreg_dlls deferred actions.
extern "C" __declspec(dllexport) UINT __stdcall PrepRegUnregDLLs(MSIHANDLE hInstall)
{
    sLogPrefix = "PrepRegUnregDLLs:";
    WriteLog(hInstall, "started");

    try
    {
        INSTALLSTATE current_state_SubstMSO;
        INSTALLSTATE future_state_SubstMSO;
        CheckWin32Error("MsiGetFeatureStateW",
                        MsiGetFeatureStateW(hInstall, L"gm_SharePointSupport_SubstMSO",
                                            &current_state_SubstMSO, &future_state_SubstMSO));

        WriteLog(hInstall, "gm_SharePointSupport_SubstMSO state:", //
                 "current", std::to_string(current_state_SubstMSO), //
                 "future", std::to_string(future_state_SubstMSO)); //

        INSTALLSTATE current_state_Main;
        INSTALLSTATE future_state_Main;
        CheckWin32Error("MsiGetFeatureStateW",
                        MsiGetFeatureStateW(hInstall, L"gm_o_SharePointSupport",
                                            &current_state_Main, &future_state_Main));

        WriteLog(hInstall, "gm_o_SharePointSupport state:", //
                 "current", std::to_string(current_state_Main), //
                 "future", std::to_string(future_state_Main)); //

        const bool bUnregSubstMSO = current_state_SubstMSO == INSTALLSTATE_LOCAL
                                    && future_state_SubstMSO == INSTALLSTATE_ABSENT;
        const bool bUnregMain
            = current_state_Main == INSTALLSTATE_LOCAL && future_state_Main == INSTALLSTATE_ABSENT;
        const bool bRegSubstMSO = current_state_SubstMSO == INSTALLSTATE_ABSENT
                                  && future_state_SubstMSO == INSTALLSTATE_LOCAL;
        // basic registration is needed when either:
        // 1. gm_o_SharePointSupport is installed;
        // 2. gm_SharePointSupport_SubstMSO is uninstalled (and unregisters everything), but
        //    gm_o_SharePointSupport is not, so needs to be re-registered
        const bool bRegMain
            = (current_state_Main == INSTALLSTATE_ABSENT && future_state_Main == INSTALLSTATE_LOCAL)
              || (bUnregSubstMSO && !bUnregMain);

        std::wstring sUnregStr;
        if (bUnregSubstMSO)
        {
            sUnregStr = L"/i:Substitute_OWSSUPP \"[#spsupp_x86.dll]\"|"
                        L"/i:Substitute_OWSSUPP \"[#spsupp_x64.dll]\"";
        }
        else if (bUnregMain)
        {
            sUnregStr = L"\"[#spsupp_x86.dll]\"|\"[#spsupp_x64.dll]\"";
        }

        std::wstring sRegStr;
        if (bRegSubstMSO)
        {
            sRegStr = L"/i:Substitute_OWSSUPP \"[#spsupp_x86.dll]\"|"
                      L"/i:Substitute_OWSSUPP \"[#spsupp_x64.dll]\"";
        }
        else if (bRegMain)
        {
            sRegStr = L"\"[#spsupp_x86.dll]\"|\"[#spsupp_x64.dll]\"";
        }

        auto SetFormattedPropW = [&](LPCWSTR sProp, const std::wstring& sVal) {
            PMSIHANDLE hRec = MsiCreateRecord(0);
            if (!hRec)
                throw std::exception("MsiCreateRecord failed!");
            MsiRecordSetStringW(hRec, 0, sVal.c_str());
            DWORD nSz = 0;
            if (MsiFormatRecordW(hInstall, hRec, const_cast<wchar_t*>(L""), &nSz)
                == ERROR_MORE_DATA)
            {
                ++nSz;
                auto buf = std::make_unique<wchar_t[]>(nSz);
                CheckWin32Error("MsiFormatRecordW",
                                MsiFormatRecordW(hInstall, hRec, buf.get(), &nSz));
                CheckWin32Error("MsiSetPropertyW", MsiSetPropertyW(hInstall, sProp, buf.get()));
            }
        };
        if (!sRegStr.empty())
            SetFormattedPropW(L"reg_dlls", sRegStr);
        if (!sUnregStr.empty())
            SetFormattedPropW(L"unreg_dlls", sUnregStr);
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
    }

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
