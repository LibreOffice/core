/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_DRIVERBLOCKLIST_HXX
#define INCLUDED_VCL_DRIVERBLOCKLIST_HXX

#include <vcl/dllapi.h>
#include <xmlreader/xmlreader.hxx>

#include <string_view>
#include <vector>

namespace DriverBlocklist
{
VCL_DLLPUBLIC bool IsDeviceBlocked(const OUString& blocklistURL, const OUString& driverVersion,
                                   const OUString& vendorId, const OUString& deviceId);

#ifdef _WIN32
VCL_DLLPUBLIC int32_t GetWindowsVersion();
#endif

enum DeviceVendor
{
    VendorAll,
    VendorIntel,
    VendorNVIDIA,
    VendorAMD,
    VendorMicrosoft,
};
const int DeviceVendorMax = VendorMicrosoft + 1;

/// Returns vendor for the given vendor ID, or VendorAll if not known.
VCL_DLLPUBLIC DeviceVendor GetVendorFromId(uint32_t id);

VCL_DLLPUBLIC std::string_view GetVendorNameFromId(uint32_t id);

// The rest should be private (only for the unittest).

struct InvalidFileException
{
};

enum OperatingSystem
{
    DRIVER_OS_UNKNOWN = 0,
    DRIVER_OS_WINDOWS_7,
    DRIVER_OS_WINDOWS_8,
    DRIVER_OS_WINDOWS_8_1,
    DRIVER_OS_WINDOWS_10,
    DRIVER_OS_LINUX,
    DRIVER_OS_OSX_10_5,
    DRIVER_OS_OSX_10_6,
    DRIVER_OS_OSX_10_7,
    DRIVER_OS_OSX_10_8,
    DRIVER_OS_ANDROID,
    DRIVER_OS_ALL
};

enum VersionComparisonOp
{
    DRIVER_LESS_THAN, // driver <  version
    DRIVER_LESS_THAN_OR_EQUAL, // driver <= version
    DRIVER_GREATER_THAN, // driver >  version
    DRIVER_GREATER_THAN_OR_EQUAL, // driver >= version
    DRIVER_EQUAL, // driver == version
    DRIVER_NOT_EQUAL, // driver != version
    DRIVER_BETWEEN_EXCLUSIVE, // driver > version && driver < versionMax
    DRIVER_BETWEEN_INCLUSIVE, // driver >= version && driver <= versionMax
    DRIVER_BETWEEN_INCLUSIVE_START, // driver >= version && driver < versionMax
    DRIVER_COMPARISON_IGNORED
};

struct DriverInfo
{
    DriverInfo(OperatingSystem os, const OUString& vendor, VersionComparisonOp op,
               uint64_t driverVersion, bool bAllowListed = false,
               const char* suggestedVersion = nullptr);

    DriverInfo();

    OperatingSystem meOperatingSystem;
    OUString maAdapterVendor;
    std::vector<OUString> maDevices;

    bool mbAllowlisted;

    VersionComparisonOp meComparisonOp;

    /* versions are assumed to be A.B.C.D packed as 0xAAAABBBBCCCCDDDD */
    uint64_t mnDriverVersion;
    uint64_t mnDriverVersionMax;

    OUString maSuggestedVersion;
    OUString maMsg;
};

class VCL_DLLPUBLIC Parser
{
public:
    Parser(const OUString& rURL, std::vector<DriverInfo>& rDriverList);
    bool parse();

private:
    void handleEntry(DriverInfo& rDriver, xmlreader::XmlReader& rReader);
    void handleList(xmlreader::XmlReader& rReader);
    void handleContent(xmlreader::XmlReader& rReader);
    static void handleDevices(DriverInfo& rDriver, xmlreader::XmlReader& rReader);

    enum class BlockType
    {
        ALLOWLIST,
        DENYLIST,
        UNKNOWN
    };

    BlockType meBlockType;
    std::vector<DriverInfo>& mrDriverList;
    OUString maURL;
};

OUString VCL_DLLPUBLIC GetVendorId(DeviceVendor id);

bool VCL_DLLPUBLIC FindBlocklistedDeviceInList(std::vector<DriverInfo>& aDeviceInfos,
                                               OUString const& sDriverVersion,
                                               OUString const& sAdapterVendorID,
                                               OUString const& sAdapterDeviceID,
                                               OperatingSystem system,
                                               const OUString& blocklistURL = OUString());

#define GFX_DRIVER_VERSION(a, b, c, d)                                                             \
    ((uint64_t(a) << 48) | (uint64_t(b) << 32) | (uint64_t(c) << 16) | uint64_t(d))

inline uint64_t V(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    // We make sure every driver number is padded by 0s, this will allow us the
    // easiest 'compare as if decimals' approach. See ParseDriverVersion for a
    // more extensive explanation of this approach.
    while (b > 0 && b < 1000)
    {
        b *= 10;
    }
    while (c > 0 && c < 1000)
    {
        c *= 10;
    }
    while (d > 0 && d < 1000)
    {
        d *= 10;
    }
    return GFX_DRIVER_VERSION(a, b, c, d);
}

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
