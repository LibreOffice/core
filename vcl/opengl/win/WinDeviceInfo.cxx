/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opengl/win/WinDeviceInfo.hxx"

#include "opengl/win/blocklist_parser.hxx"
#include <config_folders.h>

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <objbase.h>
#include <setupapi.h>
#include <algorithm>
#include <cstdint>
#include <memory>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>

#include <desktop/crashreport.hxx>

OUString* WinOpenGLDeviceInfo::mpDeviceVendors[wgl::DeviceVendorMax];
std::vector<wgl::DriverInfo> WinOpenGLDeviceInfo::maDriverInfo;

namespace {

/*
 * Compute the length of an array with constant length.  (Use of this method
 * with a non-array pointer will not compile.)
 *
 * Beware of the implicit trailing '\0' when using this with string constants.
*/
template<typename T, size_t N>
size_t ArrayLength(T (&)[N])
{
    return N;
}

#define GFX_DRIVER_VERSION(a,b,c,d) \
((uint64_t(a)<<48) | (uint64_t(b)<<32) | (uint64_t(c)<<16) | uint64_t(d))

bool GetKeyValue(const WCHAR* keyLocation, const WCHAR* keyName, OUString& destString, int type)
{
    HKEY key;
    DWORD dwcbData;
    DWORD dValue;
    DWORD resultType;
    LONG result;
    bool retval = true;

    result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyLocation, 0, KEY_QUERY_VALUE, &key);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }

    switch (type)
    {
        case REG_DWORD:
            {
                // We only use this for vram size
                dwcbData = sizeof(dValue);
                result = RegQueryValueExW(key, keyName, nullptr, &resultType,
                        reinterpret_cast<LPBYTE>(&dValue), &dwcbData);
                if (result == ERROR_SUCCESS && resultType == REG_DWORD)
                {
                    dValue = dValue / 1024 / 1024;
                    destString += OUString::number(int32_t(dValue));
                }
                else
                {
                    retval = false;
                }
                break;
            }
        case REG_MULTI_SZ:
            {
                // A chain of null-separated strings; we convert the nulls to spaces
                WCHAR wCharValue[1024];
                dwcbData = sizeof(wCharValue);

                result = RegQueryValueExW(key, keyName, nullptr, &resultType,
                        reinterpret_cast<LPBYTE>(wCharValue), &dwcbData);
                if (result == ERROR_SUCCESS && resultType == REG_MULTI_SZ)
                {
                    // This bit here could probably be cleaner.
                    bool isValid = false;

                    DWORD strLen = dwcbData/sizeof(wCharValue[0]);
                    for (DWORD i = 0; i < strLen; i++)
                    {
                        if (wCharValue[i] == '\0')
                        {
                            if (i < strLen - 1 && wCharValue[i + 1] == '\0')
                            {
                                isValid = true;
                                break;
                            }
                            else
                            {
                                wCharValue[i] = ' ';
                            }
                        }
                    }

                    // ensure wCharValue is null terminated
                    wCharValue[strLen-1] = '\0';

                    if (isValid)
                        destString = OUString(SAL_U(wCharValue));

                }
                else
                {
                    retval = false;
                }

                break;
            }
    }
    RegCloseKey(key);

    return retval;
}

// The device ID is a string like PCI\VEN_15AD&DEV_0405&SUBSYS_040515AD
// this function is used to extract the id's out of it
uint32_t ParseIDFromDeviceID(const OUString &key, const char *prefix, int length)
{
    OUString id = key.toAsciiUpperCase();
    OUString aPrefix = OUString::fromUtf8(prefix);
    int32_t start = id.indexOf(aPrefix);
    if (start != -1)
    {
        id = id.copy(start + aPrefix.getLength(), length);
    }
    return id.toUInt32(16);
}

// OS version in 16.16 major/minor form
// based on http://msdn.microsoft.com/en-us/library/ms724834(VS.85).aspx
enum {
    kWindowsUnknown    = 0,
    kWindowsXP         = 0x00050001,
    kWindowsServer2003 = 0x00050002,
    kWindowsVista      = 0x00060000,
    kWindows7          = 0x00060001,
    kWindows8          = 0x00060002,
    kWindows8_1        = 0x00060003,
    kWindows10         = 0x000A0000  // Major 10 Minor 0
};


wgl::OperatingSystem WindowsVersionToOperatingSystem(int32_t aWindowsVersion)
{
    switch(aWindowsVersion)
    {
        case kWindowsXP:
            return wgl::DRIVER_OS_WINDOWS_XP;
        case kWindowsServer2003:
            return wgl::DRIVER_OS_WINDOWS_SERVER_2003;
        case kWindowsVista:
            return wgl::DRIVER_OS_WINDOWS_VISTA;
        case kWindows7:
            return wgl::DRIVER_OS_WINDOWS_7;
        case kWindows8:
            return wgl::DRIVER_OS_WINDOWS_8;
        case kWindows8_1:
            return wgl::DRIVER_OS_WINDOWS_8_1;
        case kWindows10:
            return wgl::DRIVER_OS_WINDOWS_10;
        case kWindowsUnknown:
        default:
            return wgl::DRIVER_OS_UNKNOWN;
    };
}


int32_t WindowsOSVersion()
{
    static int32_t winVersion = kWindowsUnknown;

    if (winVersion == kWindowsUnknown)
    {
        // GetVersion(Ex) and VersionHelpers (based on VerifyVersionInfo) API are
        // subject to manifest-based behavior since Windows 8.1, so give wrong results.
        // Another approach would be to use NetWkstaGetInfo, but that has some small
        // reported delays (some milliseconds), and might get slower in domains with
        // poor network connections.
        // So go with a solution described at https://msdn.microsoft.com/en-us/library/ms724429
        HINSTANCE hLibrary = LoadLibraryW(L"kernel32.dll");
        if (hLibrary != nullptr)
        {
            wchar_t szPath[MAX_PATH];
            DWORD dwCount = GetModuleFileNameW(hLibrary, szPath, SAL_N_ELEMENTS(szPath));
            FreeLibrary(hLibrary);
            if (dwCount != 0 && dwCount < SAL_N_ELEMENTS(szPath))
            {
                dwCount = GetFileVersionInfoSizeW(szPath, nullptr);
                if (dwCount != 0)
                {
                    std::unique_ptr<char> ver(new char[dwCount]);
                    if (GetFileVersionInfoW(szPath, 0, dwCount, ver.get()) != FALSE)
                    {
                        void* pBlock = nullptr;
                        UINT dwBlockSz = 0;
                        if (VerQueryValueW(ver.get(), L"\\", &pBlock, &dwBlockSz) != FALSE && dwBlockSz >= sizeof(VS_FIXEDFILEINFO))
                        {
                            VS_FIXEDFILEINFO *vinfo = static_cast<VS_FIXEDFILEINFO *>(pBlock);
                            winVersion = int32_t(vinfo->dwProductVersionMS);
                        }
                    }
                }
            }
        }
    }

    return winVersion;
}

// This allows us to pad driver version 'substrings' with 0s, this
// effectively allows us to treat the version numbers as 'decimals'. This is
// a little strange but this method seems to do the right thing for all
// different vendor's driver strings. i.e. .98 will become 9800, which is
// larger than .978 which would become 9780.
void PadDriverDecimal(char *aString)
{
    for (int i = 0; i < 4; i++)
    {
        if (!aString[i])
        {
            for (int c = i; c < 4; c++)
            {
                aString[c] = '0';
            }
            break;
        }
    }
    aString[4] = 0;
}

// All destination string storage needs to have at least 5 bytes available.
bool SplitDriverVersion(const char *aSource, char *aAStr, char *aBStr, char *aCStr, char *aDStr)
{
    // sscanf doesn't do what we want here to we parse this manually.
    int len = strlen(aSource);
    char *dest[4] = { aAStr, aBStr, aCStr, aDStr };
    unsigned destIdx = 0;
    unsigned destPos = 0;

    for (int i = 0; i < len; i++)
    {
        if (destIdx >= ArrayLength(dest))
        {
            // Invalid format found. Ensure we don't access dest beyond bounds.
            return false;
        }

        if (aSource[i] == '.')
        {
            dest[destIdx++][destPos] = 0;
            destPos = 0;
            continue;
        }

        if (destPos > 3)
        {
            // Ignore more than 4 chars. Ensure we never access dest[destIdx]
            // beyond its bounds.
            continue;
        }

        dest[destIdx][destPos++] = aSource[i];
    }

    // Add last terminator.
    dest[destIdx][destPos] = 0;

    if (destIdx != ArrayLength(dest) - 1)
    {
        return false;
    }
    return true;
}

/* Other interesting places for info:
 *   IDXGIAdapter::GetDesc()
 *   IDirectDraw7::GetAvailableVidMem()
 *   e->GetAvailableTextureMem()
 * */

template<typename T> void appendIntegerWithPadding(OUString& rString, T value, sal_uInt32 nChars)
{
    rString += "0x";
    OUString aValue = OUString::number(value, 16);
    sal_Int32 nLength = aValue.getLength();
    sal_uInt32 nPadLength = nChars - nLength;
    assert(nPadLength >= 0);
    OUStringBuffer aBuffer;
    for (sal_uInt32 i = 0; i < nPadLength; ++i)
    {
        aBuffer.append("0");
    }
    rString += aBuffer.makeStringAndClear() + aValue;
}

#define DEVICE_KEY_PREFIX L"\\Registry\\Machine\\"
}

namespace wgl {

bool ParseDriverVersion(const OUString& aVersion, uint64_t& rNumericVersion)
{
    rNumericVersion = 0;

#if defined(_WIN32)
    int a, b, c, d;
    char aStr[8], bStr[8], cStr[8], dStr[8];
    /* honestly, why do I even bother */
    OString aOVersion = OUStringToOString(aVersion, RTL_TEXTENCODING_UTF8);
    if (!SplitDriverVersion(aOVersion.getStr(), aStr, bStr, cStr, dStr))
        return false;

    PadDriverDecimal(bStr);
    PadDriverDecimal(cStr);
    PadDriverDecimal(dStr);

    a = atoi(aStr);
    b = atoi(bStr);
    c = atoi(cStr);
    d = atoi(dStr);

    if (a < 0 || a > 0xffff) return false;
    if (b < 0 || b > 0xffff) return false;
    if (c < 0 || c > 0xffff) return false;
    if (d < 0 || d > 0xffff) return false;

    rNumericVersion = GFX_DRIVER_VERSION(a, b, c, d);
    return true;
#else
    return false;
#endif
}

uint64_t DriverInfo::allDriverVersions = ~(uint64_t(0));

DriverInfo::DriverInfo()
    : meOperatingSystem(wgl::DRIVER_OS_UNKNOWN),
    mnOperatingSystemVersion(0),
    maAdapterVendor(WinOpenGLDeviceInfo::GetDeviceVendor(VendorAll)),
    mbWhitelisted(false),
    meComparisonOp(DRIVER_COMPARISON_IGNORED),
    mnDriverVersion(0),
    mnDriverVersionMax(0)
{}

DriverInfo::DriverInfo(OperatingSystem os, const OUString& vendor,
        VersionComparisonOp op,
        uint64_t driverVersion,
        bool bWhitelisted,
        const char *suggestedVersion /* = nullptr */)
    : meOperatingSystem(os),
    mnOperatingSystemVersion(0),
    maAdapterVendor(vendor),
    mbWhitelisted(bWhitelisted),
    meComparisonOp(op),
    mnDriverVersion(driverVersion),
    mnDriverVersionMax(0)
{
    if (suggestedVersion)
        maSuggestedVersion = OStringToOUString(OString(suggestedVersion), RTL_TEXTENCODING_UTF8);
}

DriverInfo::~DriverInfo()
{
}

}

WinOpenGLDeviceInfo::WinOpenGLDeviceInfo():
    mbHasDualGPU(false),
    mbRDP(false)
{
    GetData();
    FillBlacklist();
}

WinOpenGLDeviceInfo::~WinOpenGLDeviceInfo()
{
}

namespace {

struct compareIgnoreAsciiCase
{
    explicit compareIgnoreAsciiCase(const OUString& rString)
        : maString(rString)
    {
    }

    bool operator()(const OUString& rCompare)
    {
        return maString.equalsIgnoreAsciiCase(rCompare);
    }

private:
    OUString maString;
};

}

bool WinOpenGLDeviceInfo::FindBlocklistedDeviceInList(std::vector<wgl::DriverInfo>& aDeviceInfos,
                                                      OUString const & sDriverVersion, OUString const & sAdapterVendorID,
                                                      OUString const & sAdapterDeviceID, uint32_t nWindowsVersion)
{
    uint64_t driverVersion;
    wgl::ParseDriverVersion(sDriverVersion, driverVersion);

    wgl::OperatingSystem eOS = WindowsVersionToOperatingSystem(nWindowsVersion);
    bool match = false;
    for (std::vector<wgl::DriverInfo>::size_type i = 0; i < aDeviceInfos.size(); i++)
    {
        if (aDeviceInfos[i].meOperatingSystem != wgl::DRIVER_OS_ALL &&
                aDeviceInfos[i].meOperatingSystem != eOS)
        {
            continue;
        }

        if (aDeviceInfos[i].mnOperatingSystemVersion && aDeviceInfos[i].mnOperatingSystemVersion != nWindowsVersion)
        {
            continue;
        }

        if (!aDeviceInfos[i].maAdapterVendor.equalsIgnoreAsciiCase(GetDeviceVendor(wgl::VendorAll)) &&
                !aDeviceInfos[i].maAdapterVendor.equalsIgnoreAsciiCase(sAdapterVendorID))
        {
            continue;
        }

        if (std::none_of(aDeviceInfos[i].maDevices.begin(), aDeviceInfos[i].maDevices.end(), compareIgnoreAsciiCase("all")) &&
            std::none_of(aDeviceInfos[i].maDevices.begin(), aDeviceInfos[i].maDevices.end(), compareIgnoreAsciiCase(sAdapterDeviceID)))
        {
            continue;
        }

        switch (aDeviceInfos[i].meComparisonOp)
        {
            case wgl::DRIVER_LESS_THAN:
                match = driverVersion < aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_LESS_THAN_OR_EQUAL:
                match = driverVersion <= aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_GREATER_THAN:
                match = driverVersion > aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_GREATER_THAN_OR_EQUAL:
                match = driverVersion >= aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_EQUAL:
                match = driverVersion == aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_NOT_EQUAL:
                match = driverVersion != aDeviceInfos[i].mnDriverVersion;
                break;
            case wgl::DRIVER_BETWEEN_EXCLUSIVE:
                match = driverVersion > aDeviceInfos[i].mnDriverVersion && driverVersion < aDeviceInfos[i].mnDriverVersionMax;
                break;
            case wgl::DRIVER_BETWEEN_INCLUSIVE:
                match = driverVersion >= aDeviceInfos[i].mnDriverVersion && driverVersion <= aDeviceInfos[i].mnDriverVersionMax;
                break;
            case wgl::DRIVER_BETWEEN_INCLUSIVE_START:
                match = driverVersion >= aDeviceInfos[i].mnDriverVersion && driverVersion < aDeviceInfos[i].mnDriverVersionMax;
                break;
            case wgl::DRIVER_COMPARISON_IGNORED:
                // We don't have a comparison op, so we match everything.
                match = true;
                break;
            default:
                SAL_WARN("vcl.opengl", "Bogus op in GfxDriverInfo");
                break;
        }

        if (match || aDeviceInfos[i].mnDriverVersion == wgl::DriverInfo::allDriverVersions)
        {
            // white listed drivers
            if (aDeviceInfos[i].mbWhitelisted)
            {
                SAL_WARN("vcl.opengl", "whitelisted driver");
                return false;
            }

            match = true;
            SAL_WARN("vcl.opengl", "use : " << aDeviceInfos[i].maSuggestedVersion);
            break;
        }
    }

    SAL_INFO("vcl.opengl", (match ? "BLACKLISTED" : "not blacklisted"));
    return match;
}

bool WinOpenGLDeviceInfo::FindBlocklistedDeviceInList()
{
    return FindBlocklistedDeviceInList(maDriverInfo, maDriverVersion, maAdapterVendorID, maAdapterDeviceID, mnWindowsVersion);
}

namespace {

OUString getCacheFolder()
{
    OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    rtl::Bootstrap::expandMacros(url);

    osl::Directory::create(url);

    return url;
}

void writeToLog(SvStream& rStrm, const char* pKey, const OUString & rVal)
{
    rStrm.WriteCharPtr(pKey);
    rStrm.WriteCharPtr(": ");
    rStrm.WriteOString(OUStringToOString(rVal, RTL_TEXTENCODING_UTF8));
    rStrm.WriteChar('\n');
}

}

bool WinOpenGLDeviceInfo::isDeviceBlocked()
{
    CrashReporter::AddKeyValue("OpenGLVendor", maAdapterVendorID);
    CrashReporter::AddKeyValue("OpenGLDevice", maAdapterDeviceID);
    CrashReporter::AddKeyValue("OpenGLDriver", maDriverVersion);

    SAL_INFO("vcl.opengl", maDriverVersion);
    SAL_INFO("vcl.opengl", maDriverDate);
    SAL_INFO("vcl.opengl", maDeviceID);
    SAL_INFO("vcl.opengl", maAdapterVendorID);
    SAL_INFO("vcl.opengl", maAdapterDeviceID);
    SAL_INFO("vcl.opengl", maAdapterSubsysID);
    SAL_INFO("vcl.opengl", maDeviceKey);
    SAL_INFO("vcl.opengl", maDeviceString);

    OUString aCacheFolder = getCacheFolder();

    OUString aCacheFile(aCacheFolder + "/opengl_device.log");
    SvFileStream aOpenGLLogFile(aCacheFile, StreamMode::WRITE);

    writeToLog(aOpenGLLogFile, "DriverVersion", maDriverVersion);
    writeToLog(aOpenGLLogFile, "DriverDate", maDriverDate);
    writeToLog(aOpenGLLogFile, "DeviceID", maDeviceID);
    writeToLog(aOpenGLLogFile, "AdapterVendorID", maAdapterVendorID);
    writeToLog(aOpenGLLogFile, "AdapterDeviceID", maAdapterDeviceID);
    writeToLog(aOpenGLLogFile, "AdapterSubsysID", maAdapterSubsysID);
    writeToLog(aOpenGLLogFile, "DeviceKey", maDeviceKey);
    writeToLog(aOpenGLLogFile, "DeviceString", maDeviceString);

    // Check if the device is blocked from the downloaded blocklist. If not, check
    // the static list after that. This order is used so that we can later escape
    // out of static blocks (i.e. if we were wrong or something was patched, we
    // can back out our static block without doing a release).
    if (mbRDP)
    {
        SAL_WARN("vcl.opengl", "all OpenGL blocked for RDP sessions");
        return true;
    }

    return FindBlocklistedDeviceInList();
}

void WinOpenGLDeviceInfo::GetData()
{
    DISPLAY_DEVICEW displayDevice;
    displayDevice.cb = sizeof(displayDevice);

    mnWindowsVersion = WindowsOSVersion();
    int deviceIndex = 0;

    while (EnumDisplayDevicesW(nullptr, deviceIndex, &displayDevice, 0))
    {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            break;
        }
        deviceIndex++;
    }

    // make sure the string is null terminated
    // (using the term "null" here to mean a zero UTF-16 unit)
    if (wcsnlen(displayDevice.DeviceKey, ArrayLength(displayDevice.DeviceKey))
            == ArrayLength(displayDevice.DeviceKey))
    {
        // we did not find a null
        SAL_WARN("vcl.opengl", "string not null terminated");
        return;
    }

    /* DeviceKey is "reserved" according to MSDN so we'll be careful with it */
    /* check that DeviceKey begins with DEVICE_KEY_PREFIX */
    /* some systems have a DeviceKey starting with \REGISTRY\Machine\ so we need to compare case insensitively */
    if (_wcsnicmp(displayDevice.DeviceKey, DEVICE_KEY_PREFIX, ArrayLength(DEVICE_KEY_PREFIX)-1) != 0)
    {
        SAL_WARN("vcl.opengl", "incorrect DeviceKey");
        return;
    }

    // chop off DEVICE_KEY_PREFIX
    maDeviceKey = SAL_U(displayDevice.DeviceKey) + ArrayLength(DEVICE_KEY_PREFIX)-1;

    maDeviceID = SAL_U(displayDevice.DeviceID);
    maDeviceString = SAL_U(displayDevice.DeviceString);

    if (maDeviceID.isEmpty() &&
        (maDeviceString == "RDPDD Chained DD" ||
         (maDeviceString == "RDPUDD Chained DD")))
    {
        // we need to block RDP as it does not provide OpenGL 2.1+
        mbRDP = true;
        SAL_WARN("vcl.opengl", "RDP => blocked");
        return;
    }

    /* create a device information set composed of the current display device */
    HDEVINFO devinfo = SetupDiGetClassDevsW(nullptr, SAL_W(maDeviceID.getStr()), nullptr,
            DIGCF_PRESENT | DIGCF_PROFILE | DIGCF_ALLCLASSES);

    if (devinfo != INVALID_HANDLE_VALUE)
    {
        HKEY key;
        LONG result;
        WCHAR value[255];
        DWORD dwcbData;
        SP_DEVINFO_DATA devinfoData;
        DWORD memberIndex = 0;

        devinfoData.cbSize = sizeof(devinfoData);
        /* enumerate device information elements in the device information set */
        while (SetupDiEnumDeviceInfo(devinfo, memberIndex++, &devinfoData))
        {
            /* get a string that identifies the device's driver key */
            if (SetupDiGetDeviceRegistryPropertyW(devinfo,
                        &devinfoData,
                        SPDRP_DRIVER,
                        nullptr,
                        reinterpret_cast<PBYTE>(value),
                        sizeof(value),
                        nullptr))
            {
                OUString  driverKey("System\\CurrentControlSet\\Control\\Class\\");
                driverKey += SAL_U(value);
                result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SAL_W(driverKey.getStr()), 0, KEY_QUERY_VALUE, &key);
                if (result == ERROR_SUCCESS)
                {
                    /* we've found the driver we're looking for */
                    dwcbData = sizeof(value);
                    result = RegQueryValueExW(key, L"DriverVersion", nullptr, nullptr,
                            reinterpret_cast<LPBYTE>(value), &dwcbData);
                    if (result == ERROR_SUCCESS)
                    {
                        maDriverVersion = OUString(SAL_U(value));
                    }
                    else
                    {
                        // If the entry wasn't found, assume the worst (0.0.0.0).
                        maDriverVersion = OUString("0.0.0.0");
                    }
                    dwcbData = sizeof(value);
                    result = RegQueryValueExW(key, L"DriverDate", nullptr, nullptr,
                            reinterpret_cast<LPBYTE>(value), &dwcbData);
                    if (result == ERROR_SUCCESS)
                    {
                        maDriverDate = SAL_U(value);
                    }
                    else
                    {
                        // Again, assume the worst
                        maDriverDate = OUString("01-01-1970");
                    }
                    RegCloseKey(key);
                    break;
                }
            }
        }

        SetupDiDestroyDeviceInfoList(devinfo);
    }
    else
    {
        SAL_WARN("vcl.opengl", "invalid handle value");
    }

    appendIntegerWithPadding(maAdapterVendorID, ParseIDFromDeviceID(maDeviceID, "VEN_", 4), 4);
    appendIntegerWithPadding(maAdapterDeviceID, ParseIDFromDeviceID(maDeviceID, "&DEV_", 4), 4);
    appendIntegerWithPadding(maAdapterSubsysID, ParseIDFromDeviceID(maDeviceID, "&SUBSYS_", 8), 8);

    // We now check for second display adapter.

    // Device interface class for display adapters.
    CLSID GUID_DISPLAY_DEVICE_ARRIVAL;
    HRESULT hresult = CLSIDFromString(L"{1CA05180-A699-450A-9A0C-DE4FBE3DDD89}",
            &GUID_DISPLAY_DEVICE_ARRIVAL);
    if (hresult == NOERROR)
    {
        devinfo = SetupDiGetClassDevsW(&GUID_DISPLAY_DEVICE_ARRIVAL,
                nullptr, nullptr,
                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

        if (devinfo != INVALID_HANDLE_VALUE)
        {
            HKEY key;
            LONG result;
            WCHAR value[255];
            DWORD dwcbData;
            SP_DEVINFO_DATA devinfoData;
            DWORD memberIndex = 0;
            devinfoData.cbSize = sizeof(devinfoData);

            OUString aAdapterDriver2;
            OUString aDeviceID2;
            OUString aDriverVersion2;
            OUString aDriverDate2;
            uint32_t adapterVendorID2;
            uint32_t adapterDeviceID2;

            /* enumerate device information elements in the device information set */
            while (SetupDiEnumDeviceInfo(devinfo, memberIndex++, &devinfoData))
            {
                /* get a string that identifies the device's driver key */
                if (SetupDiGetDeviceRegistryPropertyW(devinfo,
                            &devinfoData,
                            SPDRP_DRIVER,
                            nullptr,
                            reinterpret_cast<PBYTE>(value),
                            sizeof(value),
                            nullptr))
                {
                    OUString driverKey2("System\\CurrentControlSet\\Control\\Class\\");
                    driverKey2 += SAL_U(value);
                    result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, SAL_W(driverKey2.getStr()), 0, KEY_QUERY_VALUE, &key);
                    if (result == ERROR_SUCCESS)
                    {
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"MatchingDeviceId", nullptr,
                                nullptr, reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            continue;
                        }
                        aDeviceID2 = SAL_U(value);
                        OUString aAdapterVendorID2String;
                        OUString aAdapterDeviceID2String;
                        adapterVendorID2 = ParseIDFromDeviceID(aDeviceID2, "VEN_", 4);
                        appendIntegerWithPadding(aAdapterVendorID2String, adapterVendorID2, 4);
                        adapterDeviceID2 = ParseIDFromDeviceID(aDeviceID2, "&DEV_", 4);
                        appendIntegerWithPadding(aAdapterDeviceID2String, adapterDeviceID2, 4);
                        if (maAdapterVendorID == aAdapterVendorID2String &&
                                maAdapterDeviceID == aAdapterDeviceID2String)
                        {
                            RegCloseKey(key);
                            continue;
                        }

                        // If this device is missing driver information, it is unlikely to
                        // be a real display adapter.
                        if (!GetKeyValue(SAL_W(driverKey2.getStr()), L"InstalledDisplayDrivers",
                                        aAdapterDriver2, REG_MULTI_SZ))
                        {
                            RegCloseKey(key);
                            continue;
                        }
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"DriverVersion", nullptr, nullptr,
                                reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            RegCloseKey(key);
                            continue;
                        }
                        aDriverVersion2 = SAL_U(value);
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"DriverDate", nullptr, nullptr,
                                reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            RegCloseKey(key);
                            continue;
                        }
                        aDriverDate2 = SAL_U(value);
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"Device Description", nullptr,
                                nullptr, reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            dwcbData = sizeof(value);
                            result = RegQueryValueExW(key, L"DriverDesc", nullptr, nullptr,
                                    reinterpret_cast<LPBYTE>(value), &dwcbData);
                        }
                        RegCloseKey(key);
                        if (result == ERROR_SUCCESS)
                        {
                            mbHasDualGPU = true;
                            maDeviceString2 = SAL_U(value);
                            maDeviceID2 = aDeviceID2;
                            maDeviceKey2 = driverKey2;
                            maDriverVersion2 = aDriverVersion2;
                            maDriverDate2 = aDriverDate2;
                            appendIntegerWithPadding(maAdapterVendorID2, adapterVendorID2, 4);
                            appendIntegerWithPadding(maAdapterDeviceID2, adapterDeviceID2, 4);
                            appendIntegerWithPadding(maAdapterSubsysID2, ParseIDFromDeviceID(maDeviceID2, "&SUBSYS_", 8), 8);
                            break;
                        }
                    }
                }
            }

            SetupDiDestroyDeviceInfoList(devinfo);
        }
    }
}

// Macro for assigning a device vendor id to a string.
#define DECLARE_VENDOR_ID(name, deviceId) \
    case name: \
        *mpDeviceVendors[id] = deviceId; \
break;

OUString WinOpenGLDeviceInfo::GetDeviceVendor(wgl::DeviceVendor id)
{
    assert(id >= 0 && id < wgl::DeviceVendorMax);

    if (mpDeviceVendors[id])
        return *mpDeviceVendors[id];

    mpDeviceVendors[id] = new OUString();

    switch (id)
    {
        DECLARE_VENDOR_ID(wgl::VendorAll, "");
        DECLARE_VENDOR_ID(wgl::VendorIntel, "0x8086");
        DECLARE_VENDOR_ID(wgl::VendorNVIDIA, "0x10de");
        DECLARE_VENDOR_ID(wgl::VendorAMD, "0x1022");
        DECLARE_VENDOR_ID(wgl::VendorATI, "0x1002");
        DECLARE_VENDOR_ID(wgl::VendorMicrosoft, "0x1414");
        // Suppress a warning.
        DECLARE_VENDOR_ID(wgl::DeviceVendorMax, "");
    }

    return *mpDeviceVendors[id];
}

namespace {


OUString getBlacklistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/opengl/opengl_blacklist_windows.xml";
}


}

void WinOpenGLDeviceInfo::FillBlacklist()
{
    OUString aURL = getBlacklistFile();
    WinBlocklistParser aParser(aURL, maDriverInfo);
    try {
        aParser.parse();
    }
    catch (...)
    {
        SAL_WARN("vcl.opengl", "error parsing blacklist");
        maDriverInfo.clear();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
