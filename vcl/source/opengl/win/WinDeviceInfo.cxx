/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <opengl/win/WinDeviceInfo.hxx>

#include <driverblocklist.hxx>
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
#include <string_view>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <desktop/crashreport.hxx>

namespace {

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
                        destString = OUString(o3tl::toU(wCharValue));

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

WinOpenGLDeviceInfo::WinOpenGLDeviceInfo():
    mbHasDualGPU(false),
    mbRDP(false)
{
    GetData();
}

WinOpenGLDeviceInfo::~WinOpenGLDeviceInfo()
{
}

static OUString getDenylistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/opengl/opengl_denylist_windows.xml";
}

bool WinOpenGLDeviceInfo::FindBlocklistedDeviceInList()
{
    return DriverBlocklist::IsDeviceBlocked( getDenylistFile(), DriverBlocklist::VersionType::OpenGL,
        maDriverVersion, maAdapterVendorID, maAdapterDeviceID);
}

namespace {

OUString getCacheFolder()
{
    OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    rtl::Bootstrap::expandMacros(url);

    osl::Directory::create(url);

    return url;
}

void writeToLog(SvStream& rStrm, const char* pKey, std::u16string_view rVal)
{
    rStrm.WriteCharPtr(pKey);
    rStrm.WriteCharPtr(": ");
    rStrm.WriteOString(OUStringToOString(rVal, RTL_TEXTENCODING_UTF8));
    rStrm.WriteChar('\n');
}

}

bool WinOpenGLDeviceInfo::isDeviceBlocked()
{
    CrashReporter::addKeyValue("OpenGLVendor", maAdapterVendorID, CrashReporter::AddItem);
    CrashReporter::addKeyValue("OpenGLDevice", maAdapterDeviceID, CrashReporter::AddItem);
    CrashReporter::addKeyValue("OpenGLDriver", maDriverVersion, CrashReporter::Write);

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
    SvFileStream aOpenGLLogFile(aCacheFile, StreamMode::WRITE|StreamMode::TRUNC);

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
    if (wcsnlen(displayDevice.DeviceKey, SAL_N_ELEMENTS(displayDevice.DeviceKey))
            == SAL_N_ELEMENTS(displayDevice.DeviceKey))
    {
        // we did not find a null
        SAL_WARN("vcl.opengl", "string not null terminated");
        return;
    }

    /* DeviceKey is "reserved" according to MSDN so we'll be careful with it */
    /* check that DeviceKey begins with DEVICE_KEY_PREFIX */
    /* some systems have a DeviceKey starting with \REGISTRY\Machine\ so we need to compare case insensitively */
    if (_wcsnicmp(displayDevice.DeviceKey, DEVICE_KEY_PREFIX, SAL_N_ELEMENTS(DEVICE_KEY_PREFIX)-1) != 0)
    {
        SAL_WARN("vcl.opengl", "incorrect DeviceKey");
        return;
    }

    // chop off DEVICE_KEY_PREFIX
    maDeviceKey = o3tl::toU(displayDevice.DeviceKey) + SAL_N_ELEMENTS(DEVICE_KEY_PREFIX)-1;

    maDeviceID = o3tl::toU(displayDevice.DeviceID);
    maDeviceString = o3tl::toU(displayDevice.DeviceString);

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
    HDEVINFO devinfo = SetupDiGetClassDevsW(nullptr, o3tl::toW(maDeviceID.getStr()), nullptr,
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
                OUString  driverKey(OUString::Concat("System\\CurrentControlSet\\Control\\Class\\") + o3tl::toU(value));
                result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, o3tl::toW(driverKey.getStr()), 0, KEY_QUERY_VALUE, &key);
                if (result == ERROR_SUCCESS)
                {
                    /* we've found the driver we're looking for */
                    dwcbData = sizeof(value);
                    result = RegQueryValueExW(key, L"DriverVersion", nullptr, nullptr,
                            reinterpret_cast<LPBYTE>(value), &dwcbData);
                    if (result == ERROR_SUCCESS)
                    {
                        maDriverVersion = OUString(o3tl::toU(value));
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
                        maDriverDate = o3tl::toU(value);
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
                    OUString driverKey2(OUString::Concat("System\\CurrentControlSet\\Control\\Class\\") + o3tl::toU(value));
                    result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, o3tl::toW(driverKey2.getStr()), 0, KEY_QUERY_VALUE, &key);
                    if (result == ERROR_SUCCESS)
                    {
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"MatchingDeviceId", nullptr,
                                nullptr, reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            continue;
                        }
                        aDeviceID2 = o3tl::toU(value);
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
                        if (!GetKeyValue(o3tl::toW(driverKey2.getStr()), L"InstalledDisplayDrivers",
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
                        aDriverVersion2 = o3tl::toU(value);
                        dwcbData = sizeof(value);
                        result = RegQueryValueExW(key, L"DriverDate", nullptr, nullptr,
                                reinterpret_cast<LPBYTE>(value), &dwcbData);
                        if (result != ERROR_SUCCESS)
                        {
                            RegCloseKey(key);
                            continue;
                        }
                        aDriverDate2 = o3tl::toU(value);
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
                            maDeviceString2 = o3tl::toU(value);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
