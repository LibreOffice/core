/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string>
#include <sstream>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>
#include <Wuerror.h>
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

// Show a warning message box. This will be automatically suppressed in unattended installation.
void ShowWarning(MSIHANDLE hInst, const std::wstring& sKBNo, const char* sMessage)
{
    // Error table's message #25000: "Installing a pre-requisite [2] failed.
    // You might need to manually install it from Microsoft site to be able to run the product.[3]"
    PMSIHANDLE hRec = MsiCreateRecord(3);
    // To show a message from Error table, record's Field 0 must be null
    MsiRecordSetInteger(hRec, 1, 25000);
    MsiRecordSetStringW(hRec, 2, sKBNo.c_str());
    std::string s("\n");
    s += sMessage;
    MsiRecordSetStringA(hRec, 3, s.c_str());
    MsiProcessMessage(hInst, INSTALLMESSAGE_WARNING, hRec);
}

// Set custom action description visible in progress dialog
void SetStatusText(MSIHANDLE hInst, const std::wstring& actName, const std::wstring& actDesc)
{
    PMSIHANDLE hRec = MsiCreateRecord(3);
    // For INSTALLMESSAGE_ACTIONSTART, record's Field 0 must be null
    // Field 1: Action name - must be non-null
    MsiRecordSetStringW(hRec, 1, actName.c_str());
    // Field 2: Action description - displayed in dialog
    MsiRecordSetStringW(hRec, 2, actDesc.c_str());
    // Let Field 3 stay null - no action template
    MsiProcessMessage(hInst, INSTALLMESSAGE_ACTIONSTART, hRec);
}

typedef std::unique_ptr<void, decltype(&CloseHandle)> CloseHandleGuard;
CloseHandleGuard Guard(HANDLE h) { return CloseHandleGuard(h, CloseHandle); }

typedef std::unique_ptr<const wchar_t, decltype(&DeleteFileW)> DeleteFileGuard;
DeleteFileGuard Guard(const wchar_t* sFileName) { return DeleteFileGuard(sFileName, DeleteFileW); }

typedef std::unique_ptr<SC_HANDLE__, decltype(&CloseServiceHandle)> CloseServiceHandleGuard;
CloseServiceHandleGuard Guard(SC_HANDLE h)
{
    return CloseServiceHandleGuard(h, CloseServiceHandle);
}

std::wstring GetTempFile()
{
    wchar_t sPath[MAX_PATH + 1];
    DWORD nResult = GetTempPathW(sizeof(sPath) / sizeof(*sPath), sPath);
    if (!nResult)
        ThrowLastError("GetTempPathW");

    wchar_t sFile[MAX_PATH + 1];
    nResult = GetTempFileNameW(sPath, L"TMP", 0, sFile);
    if (!nResult)
        ThrowLastError("GetTempFileNameW");
    return sFile;
}

bool IsWow64Process()
{
#if !defined _WIN64
    BOOL bResult = FALSE;
    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(
        GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));

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

// This class uses MsiProcessMessage to check for user input: it returns IDCANCEL when user cancels
// installation. It throws a special exception, to be intercepted in main action function to return
// corresponding exit code.
class UserInputChecker
{
public:
    class eUserCancelled
    {
    };

    UserInputChecker(MSIHANDLE hInstall)
        : m_hInstall(hInstall)
        , m_hProgressRec(MsiCreateRecord(3))
    {
        // Use explicit progress messages
        MsiRecordSetInteger(m_hProgressRec, 1, 1);
        MsiRecordSetInteger(m_hProgressRec, 2, 1);
        MsiRecordSetInteger(m_hProgressRec, 3, 0);
        int nResult = MsiProcessMessage(m_hInstall, INSTALLMESSAGE_PROGRESS, m_hProgressRec);
        if (nResult == IDCANCEL)
            throw eUserCancelled();
        // Prepare the record to following progress update calls
        MsiRecordSetInteger(m_hProgressRec, 1, 2);
        MsiRecordSetInteger(m_hProgressRec, 2, 0); // step by 0 - don't move progress
        MsiRecordSetInteger(m_hProgressRec, 3, 0);
    }

    void ThrowIfUserCancelled()
    {
        // Check if user has cancelled
        int nResult = MsiProcessMessage(m_hInstall, INSTALLMESSAGE_PROGRESS, m_hProgressRec);
        if (nResult == IDCANCEL)
            throw eUserCancelled();
    }

private:
    MSIHANDLE m_hInstall;
    PMSIHANDLE m_hProgressRec;
};

// Checks if Windows Update service is disabled, and if it is, enables it temporarily.
// Also stops the service if it's currently running, because it seems that wusa.exe
// does not freeze when it starts the service itself.
class WUServiceEnabler
{
public:
    WUServiceEnabler(MSIHANDLE hInstall)
        : mhInstall(hInstall)
        , mhService(EnableWUService(hInstall))
    {
    }

    ~WUServiceEnabler()
    {
        try
        {
            if (mhService)
            {
                EnsureServiceEnabled(mhInstall, mhService.get(), false);
                StopService(mhInstall, mhService.get(), false);
            }
        }
        catch (std::exception& e)
        {
            WriteLog(mhInstall, e.what());
        }
    }

private:
    static CloseServiceHandleGuard EnableWUService(MSIHANDLE hInstall)
    {
        auto hSCM = Guard(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
        if (!hSCM)
            ThrowLastError("OpenSCManagerW");
        WriteLog(hInstall, "Opened service control manager");

        auto hService = Guard(OpenServiceW(hSCM.get(), L"wuauserv",
                                           SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG
                                               | SERVICE_QUERY_STATUS | SERVICE_STOP));
        if (!hService)
            ThrowLastError("OpenServiceW");
        WriteLog(hInstall, "Obtained WU service handle");

        const DWORD nCurrentStatus = ServiceStatus(hInstall, hService.get());
        // Stop currently running service to prevent wusa.exe from hanging trying to detect if the
        // update is applicable (sometimes this freezes it ~indefinitely; it seems that it doesn't
        // happen if wusa.exe starts the service itself: https://superuser.com/questions/1044528/).
        // tdf#124794: Wait for service to stop.
        if (nCurrentStatus == SERVICE_RUNNING)
            StopService(hInstall, hService.get(), true);

        if (nCurrentStatus == SERVICE_RUNNING
            || !EnsureServiceEnabled(hInstall, hService.get(), true))
        {
            // No need to restore anything back, since we didn't change config
            hService.reset();
            WriteLog(hInstall, "Service configuration is unchanged");
        }

        return hService;
    }

    // Returns if the service configuration was actually changed
    static bool EnsureServiceEnabled(MSIHANDLE hInstall, SC_HANDLE hService, bool bEnabled)
    {
        bool bConfigChanged = false;

        DWORD nCbRequired = 0;
        if (!QueryServiceConfigW(hService, nullptr, 0, &nCbRequired))
        {
            if (DWORD nError = GetLastError(); nError != ERROR_INSUFFICIENT_BUFFER)
                ThrowWin32Error("QueryServiceConfigW", nError);
        }
        std::unique_ptr<char[]> pBuf(new char[nCbRequired]);
        LPQUERY_SERVICE_CONFIGW pConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIGW>(pBuf.get());
        if (!QueryServiceConfigW(hService, pConfig, nCbRequired, &nCbRequired))
            ThrowLastError("QueryServiceConfigW");
        WriteLog(hInstall, "Obtained service config");

        DWORD eNewStartType = 0;
        if (bEnabled && pConfig->dwStartType == SERVICE_DISABLED)
        {
            bConfigChanged = true;
            eNewStartType = SERVICE_DEMAND_START;
            WriteLog(hInstall, "Service is disabled, and requested to enable");
        }
        else if (!bEnabled && pConfig->dwStartType != SERVICE_DISABLED)
        {
            bConfigChanged = true;
            eNewStartType = SERVICE_DISABLED;
            WriteLog(hInstall, "Service is enabled, and requested to disable");
        }

        if (bConfigChanged)
        {
            if (!ChangeServiceConfigW(hService, SERVICE_NO_CHANGE, eNewStartType, SERVICE_NO_CHANGE,
                                      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                      nullptr))
                ThrowLastError("ChangeServiceConfigW");
            WriteLog(hInstall, "WU service config successfully changed");
        }
        else
            WriteLog(hInstall, "No need to modify service config");

        return bConfigChanged;
    }

    static DWORD ServiceStatus(MSIHANDLE hInstall, SC_HANDLE hService)
    {
        SERVICE_STATUS aServiceStatus{};
        if (!QueryServiceStatus(hService, &aServiceStatus))
            ThrowLastError("QueryServiceStatus");

        std::string sStatus;
        switch (aServiceStatus.dwCurrentState)
        {
            case SERVICE_STOPPED:
                sStatus = "SERVICE_STOPPED";
                break;
            case SERVICE_START_PENDING:
                sStatus = "SERVICE_START_PENDING";
                break;
            case SERVICE_STOP_PENDING:
                sStatus = "SERVICE_STOP_PENDING";
                break;
            case SERVICE_RUNNING:
                sStatus = "SERVICE_RUNNING";
                break;
            case SERVICE_CONTINUE_PENDING:
                sStatus = "SERVICE_CONTINUE_PENDING";
                break;
            case SERVICE_PAUSE_PENDING:
                sStatus = "SERVICE_PAUSE_PENDING";
                break;
            case SERVICE_PAUSED:
                sStatus = "SERVICE_PAUSED";
                break;
            default:
                sStatus = Num2Hex(aServiceStatus.dwCurrentState);
        }
        WriteLog(hInstall, "Service status is", sStatus);

        return aServiceStatus.dwCurrentState;
    }

    static void StopService(MSIHANDLE hInstall, SC_HANDLE hService, bool bWait)
    {
        try
        {
            if (ServiceStatus(hInstall, hService) != SERVICE_STOPPED)
            {
                SERVICE_STATUS aServiceStatus{};
                if (!ControlService(hService, SERVICE_CONTROL_STOP, &aServiceStatus))
                    ThrowLastError("ControlService");
                WriteLog(hInstall,
                         "Successfully sent SERVICE_CONTROL_STOP code to Windows Update service");
                if (aServiceStatus.dwCurrentState != SERVICE_STOPPED && bWait)
                {
                    // Let user cancel too long wait
                    UserInputChecker aInputChecker(hInstall);
                    // aServiceStatus.dwWaitHint is unreasonably high for Windows Update (30000),
                    // so don't use it, but simply poll service status each second
                    for (int nWait = 0; nWait < 30; ++nWait) // arbitrary limit of 30 s
                    {
                        for (int i = 0; i < 2; ++i) // check user input twice a second
                        {
                            Sleep(500);
                            aInputChecker.ThrowIfUserCancelled();
                        }

                        if (!QueryServiceStatus(hService, &aServiceStatus))
                            ThrowLastError("QueryServiceStatus");

                        if (aServiceStatus.dwCurrentState == SERVICE_STOPPED)
                            break;
                    }
                }
                if (aServiceStatus.dwCurrentState == SERVICE_STOPPED)
                    WriteLog(hInstall, "Successfully stopped Windows Update service");
                else if (bWait)
                    WriteLog(hInstall, "Wait for Windows Update stop timed out - proceeding");
            }
            else
                WriteLog(hInstall, "Windows Update service is not running");
        }
        catch (std::exception& e)
        {
            WriteLog(hInstall, e.what());
        }
    }

    MSIHANDLE mhInstall;
    CloseServiceHandleGuard mhService;
};
}

// Immediate action "unpack_msu" that has access to installation database and properties; checks
// "InstMSUBinary" property and unpacks the binary with that name to a temporary file; sets
// "cleanup_msu" and "inst_msu" properties to the full name of the extracted temporary file. These
// properties will become "CustomActionData" property inside relevant deferred actions.
extern "C" __declspec(dllexport) UINT __stdcall UnpackMSUForInstall(MSIHANDLE hInstall)
{
    try
    {
        sLogPrefix = "UnpackMSUForInstall:";
        WriteLog(hInstall, "started");

        WriteLog(hInstall, "Checking value of InstMSUBinary");
        wchar_t sInstMSUBinary[MAX_PATH + 10];
        DWORD nCCh = sizeof(sInstMSUBinary) / sizeof(*sInstMSUBinary);
        CheckWin32Error("MsiGetPropertyW",
                        MsiGetPropertyW(hInstall, L"InstMSUBinary", sInstMSUBinary, &nCCh));
        WriteLog(hInstall, "Got InstMSUBinary value:",
                 sInstMSUBinary); // KB2999226|Windows61_KB2999226_x64msu
        const wchar_t* sBinaryName = wcschr(sInstMSUBinary, L'|');
        if (!sBinaryName)
            throw std::exception("No KB number in InstMSUBinary!");
        // "KB2999226"
        const std::wstring sKBNo(sInstMSUBinary, sBinaryName - sInstMSUBinary);
        ++sBinaryName;

        PMSIHANDLE hDatabase = MsiGetActiveDatabase(hInstall);
        if (!hDatabase)
            ThrowLastError("MsiGetActiveDatabase");
        WriteLog(hInstall, "MsiGetActiveDatabase succeeded");

        std::wstringstream sQuery;
        sQuery << "SELECT `Data` FROM `Binary` WHERE `Name`='" << sBinaryName << "'";

        PMSIHANDLE hBinaryView;
        CheckWin32Error("MsiDatabaseOpenViewW",
                        MsiDatabaseOpenViewW(hDatabase, sQuery.str().c_str(), &hBinaryView));
        WriteLog(hInstall, "MsiDatabaseOpenViewW succeeded");

        CheckWin32Error("MsiViewExecute", MsiViewExecute(hBinaryView, 0));
        WriteLog(hInstall, "MsiViewExecute succeeded");

        PMSIHANDLE hBinaryRecord;
        CheckWin32Error("MsiViewFetch", MsiViewFetch(hBinaryView, &hBinaryRecord));
        WriteLog(hInstall, "MsiViewFetch succeeded");

        const std::wstring sBinary = GetTempFile();
        auto aDeleteFileGuard(Guard(sBinary.c_str()));
        WriteLog(hInstall, "Temp file path:", sBinary.c_str());

        CheckWin32Error("MsiSetPropertyW",
                        MsiSetPropertyW(hInstall, L"cleanup_msu", sBinary.c_str()));

        {
            HANDLE hFile = CreateFileW(sBinary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL, nullptr);
            if (hFile == INVALID_HANDLE_VALUE)
                ThrowLastError("CreateFileW");
            auto aFileHandleGuard(Guard(hFile));

            const DWORD nBufSize = 1024 * 1024;
            std::unique_ptr<char[]> buf(new char[nBufSize]);
            DWORD nTotal = 0;
            DWORD nRead;
            do
            {
                nRead = nBufSize;
                CheckWin32Error("MsiRecordReadStream",
                                MsiRecordReadStream(hBinaryRecord, 1, buf.get(), &nRead));

                if (nRead > 0)
                {
                    DWORD nWritten;
                    if (!WriteFile(hFile, buf.get(), nRead, &nWritten, nullptr))
                        ThrowLastError("WriteFile");
                    nTotal += nWritten;
                }
            } while (nRead == nBufSize);

            WriteLog(hInstall, "Successfully wrote", Num2Dec(nTotal), "bytes");
        }
        const std::wstring s_inst_msu = sKBNo + L"|" + sBinary;
        CheckWin32Error("MsiSetPropertyW",
                        MsiSetPropertyW(hInstall, L"inst_msu", s_inst_msu.c_str()));

        // Don't delete the file: it will be done by following actions (inst_msu or cleanup_msu)
        (void)aDeleteFileGuard.release();
        return ERROR_SUCCESS;
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
    }
    return ERROR_INSTALL_FAILURE;
}

// Deferred action "inst_msu" that must be run from system account. Receives the tempfile name from
// "CustomActionData" property, and runs wusa.exe to install it. Waits for it and checks exit code.
extern "C" __declspec(dllexport) UINT __stdcall InstallMSU(MSIHANDLE hInstall)
{
    std::wstring sKBNo; // "KB2999226"
    try
    {
        sLogPrefix = "InstallMSU:";
        WriteLog(hInstall, "started");

        WriteLog(hInstall, "Checking value of CustomActionData");
        wchar_t sCustomActionData[MAX_PATH + 10]; // "KB2999226|C:\Temp\binary.tmp"
        DWORD nCCh = sizeof(sCustomActionData) / sizeof(*sCustomActionData);
        CheckWin32Error("MsiGetPropertyW",
                        MsiGetPropertyW(hInstall, L"CustomActionData", sCustomActionData, &nCCh));
        WriteLog(hInstall, "Got CustomActionData value:", sCustomActionData);
        const wchar_t* sBinaryName = wcschr(sCustomActionData, L'|');
        if (!sBinaryName)
            throw std::exception("No KB number in CustomActionData!");
        sKBNo = std::wstring(sCustomActionData, sBinaryName - sCustomActionData);
        ++sBinaryName;
        auto aDeleteFileGuard(Guard(sBinaryName));

        SetStatusText(hInstall, L"WU service state check",
                      L"Checking Windows Update service state");

        // In case the Windows Update service is disabled, we temporarily enable it here. We also
        // stop running WU service, to avoid wusa.exe freeze (see comment in EnableWUService).
        WUServiceEnabler aWUServiceEnabler(hInstall);

        SetStatusText(hInstall, sKBNo + L" installation", L"Installing " + sKBNo);

        const bool bWow64Process = IsWow64Process();
        WriteLog(hInstall, "Is Wow64 Process:", bWow64Process ? "YES" : "NO");
        std::wstring sWUSAPath = bWow64Process ? GetKnownFolder(FOLDERID_Windows) + L"\\SysNative"
                                               : GetKnownFolder(FOLDERID_System);
        sWUSAPath += L"\\wusa.exe";
        WriteLog(hInstall, "Prepared wusa path:", sWUSAPath);

        std::wstring sWUSACmd
            = L"\"" + sWUSAPath + L"\" \"" + sBinaryName + L"\" /quiet /norestart";
        WriteLog(hInstall, "Prepared wusa command:", sWUSACmd);

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        if (!CreateProcessW(sWUSAPath.c_str(), const_cast<LPWSTR>(sWUSACmd.c_str()), nullptr,
                            nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
            ThrowLastError("CreateProcessW");
        CloseHandle(pi.hThread);
        auto aCloseProcHandleGuard(Guard(pi.hProcess));
        WriteLog(hInstall, "CreateProcessW succeeded");

        {
            // This block waits when the started wusa.exe process finishes. Since it's possible
            // for wusa.exe in some circumstances to wait really long (indefinitely?), we check
            // for user input here.
            UserInputChecker aInputChecker(hInstall);
            for (;;)
            {
                DWORD nWaitResult = WaitForSingleObject(pi.hProcess, 500);
                if (nWaitResult == WAIT_OBJECT_0)
                    break; // wusa.exe finished
                else if (nWaitResult == WAIT_TIMEOUT)
                    aInputChecker.ThrowIfUserCancelled();
                else
                    ThrowWin32Error("WaitForSingleObject", nWaitResult);
            }
        }

        DWORD nExitCode = 0;
        if (!GetExitCodeProcess(pi.hProcess, &nExitCode))
            ThrowLastError("GetExitCodeProcess");

        HRESULT hr = static_cast<HRESULT>(nExitCode);

        // HRESULT_FROM_WIN32 is defined as an inline function in SDK 8.1 without the constexpr
        // And it won't work to place it inside the switch statement.
        if (HRESULT_FROM_WIN32(ERROR_SUCCESS_REBOOT_REQUIRED) == hr)
        {
            hr = ERROR_SUCCESS_REBOOT_REQUIRED;
        }

        switch (hr)
        {
            case S_OK:
            case WU_S_ALREADY_INSTALLED:
            case WU_E_NOT_APPLICABLE: // Windows could lie us about its version, etc.
            case ERROR_SUCCESS_REBOOT_REQUIRED:
            case WU_S_REBOOT_REQUIRED:
                WriteLog(hInstall, "wusa.exe succeeded with exit code", Num2Hex(nExitCode));
                return ERROR_SUCCESS;

            default:
                ThrowHResult("Execution of wusa.exe", hr);
        }
    }
    catch (const UserInputChecker::eUserCancelled&)
    {
        return ERROR_INSTALL_USEREXIT;
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
        ShowWarning(hInstall, sKBNo, e.what());
    }
    return ERROR_SUCCESS; // Do not break on MSU installation errors
}

// Rollback deferred action "cleanup_msu" that is executed on error or cancel.
// It removes the temporary file created by UnpackMSUForInstall action.
// MUST be placed IMMEDIATELY AFTER "unpack_msu" in execute sequence.
extern "C" __declspec(dllexport) UINT __stdcall CleanupMSU(MSIHANDLE hInstall)
{
    try
    {
        sLogPrefix = "CleanupMSU:";
        WriteLog(hInstall, "started");

        WriteLog(hInstall, "Checking value of CustomActionData");
        wchar_t sBinaryName[MAX_PATH + 1];
        DWORD nCCh = sizeof(sBinaryName) / sizeof(*sBinaryName);
        CheckWin32Error("MsiGetPropertyW",
                        MsiGetPropertyW(hInstall, L"CustomActionData", sBinaryName, &nCCh));
        WriteLog(hInstall, "Got CustomActionData value:", sBinaryName);

        if (!DeleteFileW(sBinaryName))
        {
            if (DWORD nError = GetLastError(); nError != ERROR_FILE_NOT_FOUND)
                ThrowWin32Error("DeleteFileW", nError);
        }
        WriteLog(hInstall, "File successfully removed");
    }
    catch (std::exception& e)
    {
        WriteLog(hInstall, e.what());
    }
    // Always return success - we don't want rollback to fail.
    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
