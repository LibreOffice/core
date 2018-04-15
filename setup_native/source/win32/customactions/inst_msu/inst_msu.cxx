/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/*
    After installation of the OOo filter for the indexing service
    it is necessary to restart the indexing service in order to
    activate the filter. This is the most reliable way to get the
    indexing service working. We only restart the service if it is
    already running. If we have insufficient privileges to restart
    the service we do nothing.
*/

#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj_core.h>
#include <Wuerror.h>
#include <msiquery.h>

namespace {

template <typename IntType>
std::string Num2Hex(IntType n)
{
    std::stringstream sMsg;
    sMsg << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(n) * 2) << std::hex << n;
    return sMsg.str();
}

template <typename IntType>
std::string Num2Dec(IntType n)
{
    std::stringstream sMsg;
    sMsg << n;
    return sMsg.str();
}

void CheckHResult(const char* sFunc, HRESULT hr)
{
    if (FAILED(hr))
    {
        std::stringstream sMsg;
        sMsg << sFunc << " failed (HRESULT = " << Num2Hex(hr) << ")!";

        throw std::exception(sMsg.str().c_str());
    }
}

void ThrowWin32Error(const char* sFunc, DWORD nWin32Error)
{
    std::stringstream sMsg;
    sMsg << sFunc << " failed with Win32 error code " << Num2Hex(nWin32Error) << "!";

    throw std::exception(sMsg.str().c_str());
}

void ThrowLastError(const char* sFunc)
{
    ThrowWin32Error(sFunc, GetLastError());
}

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

void RemoveFileAndDeleteString(const std::wstring* pFileName)
{
    DeleteFileW(pFileName->c_str());
    delete pFileName;
}

typedef std::unique_ptr<std::wstring, decltype(&RemoveFileAndDeleteString)> AutoRemovedFile;

AutoRemovedFile GetTempFile()
{
    wchar_t sPath[MAX_PATH + 1];
    DWORD nResult = GetTempPathW(sizeof(sPath)/sizeof(*sPath), sPath);
    if (!nResult)
        ThrowLastError("GetTempPathW");

    wchar_t sFile[MAX_PATH + 1];
    nResult = GetTempFileNameW(sPath, L"TMP", 0, sFile);
    if (!nResult)
        ThrowLastError("GetTempFileNameW");
    return AutoRemovedFile(new std::wstring(sFile), RemoveFileAndDeleteString);
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

static std::string sLogPrefix;

template <class... StrType>
void WriteLog(MSIHANDLE hInst, const StrType&... elements)
{
    PMSIHANDLE hRec = MsiCreateRecord(sizeof...(elements));
    if (!hRec)
        return;

    std::ostringstream sTemplate;
    sTemplate << sLogPrefix;
    WriteLogElem(hInst, hRec, sTemplate, 1, elements...);
}

typedef std::unique_ptr<void, decltype(&CloseHandle)> CloseHandleGuard;

constexpr wchar_t sNamedPipeName[] = L"\\\\.\\pipe\\LibreOfficeMSIInstMSUPipe";
constexpr wchar_t sEventCancelName[] = L"Global\\LibreOfficeMSICancelInstallUCRT";

AutoRemovedFile RecieveBinary(MSIHANDLE hInstall)
{
    WriteLog(hInstall, "RecieveBinary started");

    HANDLE hPipe = CreateFileW(sNamedPipeName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hPipe == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        // The server side of the pipe has not yet been established; wait 0.5 s
        Sleep(500);
        hPipe = CreateFileW(sNamedPipeName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    }
    if (hPipe == INVALID_HANDLE_VALUE)
        ThrowLastError("CreateFileW (while opening pipe)");
    CloseHandleGuard aCloseHandleGuard(hPipe, CloseHandle);

    AutoRemovedFile aBinary = GetTempFile();
    WriteLog(hInstall, "Temp file path:", aBinary->c_str());

    HANDLE hFile = CreateFileW(aBinary->c_str(), GENERIC_WRITE, 0, 0,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        ThrowLastError("CreateFileW");
    CloseHandleGuard aFileHandleGuard(hFile, CloseHandle);

    const DWORD nBufSize = 1024 * 1024;
    std::unique_ptr<char[]> buf(new char[nBufSize]);
    DWORD nTotal = 0;
    bool bFinished = false;
    do
    {
        DWORD nRead = nBufSize;
        if (!ReadFile(hPipe, buf.get(), nRead, &nRead, nullptr))
        {
            DWORD nResult = GetLastError();
            if (nResult == ERROR_BROKEN_PIPE || nResult == ERROR_NO_DATA)
//            if (nResult == ERROR_NO_DATA)
                bFinished = true;
            else
                ThrowLastError("ReadFile");
        }

        if (nRead > 0)
        {
            DWORD nWritten;
            if (!WriteFile(hFile, buf.get(), nRead, &nWritten, nullptr))
                ThrowLastError("WriteFile");
            nTotal += nWritten;
        }
    } while (!bFinished);

    WriteLog(hInstall, "RecieveBinary succeeded; received", Num2Dec(nTotal), "bytes");
    return aBinary;
}

bool CancellableWait(MSIHANDLE hInstall, HANDLE hWaitObject, HANDLE hEventCancel)
{
    HANDLE hWait[2] = { hWaitObject, hEventCancel };
    DWORD nResult = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);
    if (nResult != WAIT_OBJECT_0)
    {
        if (nResult != WAIT_OBJECT_0 + 1)
            ThrowWin32Error("WaitForSingleObject", nResult);

        WriteLog(hInstall, "Recieved cancel event");
        return false;
    }
    return true;
}

bool IsWow64Process()
{
#if !defined _WIN64
    BOOL bResult = FALSE;
    typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));

    if (fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bResult))
            ThrowLastError("IsWow64Process");
    }

    return bResult;
#else
    return false;
#endif
}

}

extern "C" UINT __stdcall InstallUCRT(MSIHANDLE hInstall)
{
    try {
        sLogPrefix = "InstallUCRT:";
        WriteLog(hInstall, "started");

        WriteLog(hInstall, "Checking value of INST_UCRT");
        wchar_t sBinaryName[MAX_PATH + 1];
        DWORD nCCh = sizeof(sBinaryName) / sizeof(*sBinaryName);
        CheckWin32Error("MsiGetPropertyW", MsiGetPropertyW(hInstall, L"INST_UCRT", sBinaryName, &nCCh));
        WriteLog(hInstall, "Got INST_UCRT value:", sBinaryName);

        PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
        if (!hDatabase)
            ThrowLastError("MsiGetActiveDatabase");
        WriteLog(hInstall, "MsiGetActiveDatabase succeeded");

        std::wstringstream sQuery;
        sQuery << "SELECT `Data` FROM `Binary` WHERE `Name`='" << sBinaryName << "'";

        PMSIHANDLE hBinaryView;
        CheckWin32Error("MsiDatabaseOpenViewW", MsiDatabaseOpenViewW(hDatabase, sQuery.str().c_str(),
            &hBinaryView));
        WriteLog(hInstall, "MsiDatabaseOpenViewW succeeded");

        CheckWin32Error("MsiViewExecute", MsiViewExecute(hBinaryView, 0));
        WriteLog(hInstall, "MsiViewExecute succeeded");

        PMSIHANDLE hBinaryRecord;
        CheckWin32Error("MsiViewFetch", MsiViewFetch(hBinaryView, &hBinaryRecord));
        WriteLog(hInstall, "MsiViewFetch succeeded");

        HANDLE hPipe = CreateNamedPipeW(sNamedPipeName, PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 1024 * 1024, 0, 0, nullptr);
        if (hPipe == INVALID_HANDLE_VALUE)
            ThrowLastError("CreateNamedPipeW");
        CloseHandleGuard aClosePipeHandleGuard(hPipe, CloseHandle);
        WriteLog(hInstall, "Named pipe created");

        HANDLE hEventOverlapped = CreateEventW(nullptr, TRUE, TRUE, nullptr);
        if (!hEventOverlapped)
            ThrowLastError("CreateEventW");
        CloseHandleGuard aCloseEventHandleGuard0(hEventOverlapped, CloseHandle);

        HANDLE hEventCancel = CreateEventW(nullptr, TRUE, FALSE, sEventCancelName);
        if (!hEventCancel)
            ThrowLastError("CreateEventW");
        CloseHandleGuard aCloseEventHandleGuard1(hEventCancel, CloseHandle);

        {
            OVERLAPPED aOverlapped{};
            aOverlapped.hEvent = hEventOverlapped;
            ConnectNamedPipe(hPipe, &aOverlapped);
            DWORD nResult = GetLastError();
            if (nResult != ERROR_PIPE_CONNECTED && nResult != ERROR_IO_PENDING)
                ThrowLastError("ConnectNamedPipe");

            WriteLog(hInstall, "Waiting for named pipe connection");

            if (!CancellableWait(hInstall, hEventOverlapped, hEventCancel))
                return ERROR_FUNCTION_NOT_CALLED;
        }

        const DWORD nBufSize = 64 * 1024;
        std::unique_ptr<char[]> buf(new char[nBufSize]);
        DWORD nRead;
        DWORD nTotal = 0;
        do
        {
            nRead = nBufSize;
            CheckWin32Error("MsiRecordReadStream", MsiRecordReadStream(hBinaryRecord, 1, buf.get(), &nRead));

            if (nRead > 0)
            {
                OVERLAPPED aOverlapped{};
                aOverlapped.hEvent = hEventOverlapped;
                if (!WriteFile(hPipe, buf.get(), nRead, nullptr, &aOverlapped) &&
                    GetLastError() != ERROR_IO_PENDING)
                    ThrowLastError("WriteFile");
                if (!CancellableWait(hInstall, hEventOverlapped, hEventCancel))
                    return ERROR_FUNCTION_NOT_CALLED;
                DWORD nWritten;
                if (!GetOverlappedResult(hPipe, &aOverlapped, &nWritten, FALSE))
                    ThrowLastError("GetOverlappedResult");
                nTotal += nWritten;
            }
        } while (nRead == nBufSize);

        if (!FlushFileBuffers(hPipe))
            ThrowLastError("FlushFileBuffers");

        WriteLog(hInstall, "File is successfully sent over the pipe; sent", Num2Dec(nTotal), "bytes");

        return ERROR_SUCCESS;
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
    }
    return ERROR_INSTALL_FAILURE;
}

extern "C" UINT __stdcall InstallMSU(MSIHANDLE hInstall)
{
    try {
        sLogPrefix = "InstallMSU:";
        WriteLog(hInstall, "started");

        const bool bWow64Process = IsWow64Process();
        WriteLog(hInstall, "Is Wow64 Process:", bWow64Process ? "YES" : "NO");
        std::wstring sWUSAPath = bWow64Process ?
            GetKnownFolder(FOLDERID_Windows) + L"\\SysNative" :
            GetKnownFolder(FOLDERID_System);
        sWUSAPath += L"\\wusa.exe";
        WriteLog(hInstall, "Prepared wusa path:", sWUSAPath);

        AutoRemovedFile aFile = RecieveBinary(hInstall);
        std::wstring sWUSACmd = L"\"" + sWUSAPath + L"\" \"" + *aFile + L"\" /quiet /norestart";
        WriteLog(hInstall, "Prepared wusa command:", sWUSACmd);

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(sWUSAPath.c_str(), const_cast<LPWSTR>(sWUSACmd.c_str()), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
            ThrowLastError("CreateProcessW");
        CloseHandleGuard aCloseProcHandleGuard(pi.hProcess, CloseHandle);
        WriteLog(hInstall, "CreateProcessW succeeded");

        HANDLE hEventCancel = CreateEventW(nullptr, TRUE, FALSE, sEventCancelName);
        if (!hEventCancel)
            ThrowLastError("CreateEventW");
        CloseHandleGuard aCloseEventHandleGuard(hEventCancel, CloseHandle);

        if (!CancellableWait(hInstall, pi.hProcess, hEventCancel))
            return WAIT_TIMEOUT;

        DWORD nExitCode = 0;
        if (!GetExitCodeProcess(pi.hProcess, &nExitCode))
            ThrowLastError("GetExitCodeProcess");

        HRESULT hr = static_cast<HRESULT>(nExitCode);

        switch (hr)
        {
        case S_OK:
        case S_FALSE:
        case WU_S_ALREADY_INSTALLED:
        case WU_E_NOT_APPLICABLE: // Windows could lie us about its version, etc.
        case ERROR_SUCCESS_REBOOT_REQUIRED:
        case HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED):
        case WU_S_REBOOT_REQUIRED:
            WriteLog(hInstall, "wusa.exe succeeded with exit code", Num2Hex(nExitCode));
            return ERROR_SUCCESS;

        default:
            ThrowWin32Error("Execution of wusa.exe", nExitCode);
        }
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
    }
    return ERROR_INSTALL_FAILURE;
}

extern "C" UINT __stdcall EndInstallUCRT(MSIHANDLE)
{
    HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, sEventCancelName);
    return (hEvent && SetEvent(hEvent)) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
