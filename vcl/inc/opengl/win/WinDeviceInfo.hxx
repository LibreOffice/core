/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_WIN_WINDEVICEINFO_HXX
#define INCLUDED_VCL_OPENGL_WIN_WINDEVICEINFO_HXX

#include "opengl/DeviceInfo.hxx"
#include <rtl/ustring.hxx>

namespace wgl {

enum OperatingSystem {
    DRIVER_OS_UNKNOWN = 0,
    DRIVER_OS_WINDOWS_XP,
    DRIVER_OS_WINDOWS_SERVER_2003,
    DRIVER_OS_WINDOWS_VISTA,
    DRIVER_OS_WINDOWS_7,
    DRIVER_OS_WINDOWS_8,
    DRIVER_OS_WINDOWS_8_1,
    DRIVER_OS_LINUX,
    DRIVER_OS_OS_X_10_5,
    DRIVER_OS_OS_X_10_6,
    DRIVER_OS_OS_X_10_7,
    DRIVER_OS_OS_X_10_8,
    DRIVER_OS_ANDROID,
    DRIVER_OS_ALL
};

enum VersionComparisonOp {
    DRIVER_LESS_THAN,             // driver <  version
    DRIVER_LESS_THAN_OR_EQUAL,    // driver <= version
    DRIVER_GREATER_THAN,          // driver >  version
    DRIVER_GREATER_THAN_OR_EQUAL, // driver >= version
    DRIVER_EQUAL,                 // driver == version
    DRIVER_NOT_EQUAL,             // driver != version
    DRIVER_BETWEEN_EXCLUSIVE,     // driver > version && driver < versionMax
    DRIVER_BETWEEN_INCLUSIVE,     // driver >= version && driver <= versionMax
    DRIVER_BETWEEN_INCLUSIVE_START, // driver >= version && driver < versionMax
    DRIVER_COMPARISON_IGNORED
};

enum DeviceFamily {
    IntelGMA500,
    IntelGMA900,
    IntelGMA950,
    IntelGMA3150,
    IntelGMAX3000,
    IntelGMAX4500HD,
    IntelHD3000,
    IntelMobileHDGraphics,
    NvidiaBlockD3D9Layers,
    RadeonX1000,
    Geforce7300GT,
    Nvidia310M,
    DeviceFamilyMax
};

enum DeviceVendor {
    VendorAll,
    VendorIntel,
    VendorNVIDIA,
    VendorAMD,
    VendorATI,
    VendorMicrosoft,
    DeviceVendorMax
};

}

class WinOpenGLDeviceInfo : public OpenGLDeviceInfo
{
private:
    OUString maDriverVersion;
    OUString maDriverVersion2;

    OUString maDriverDate;
    OUString maDriverDate2;

    OUString maDeviceID;
    OUString maDeviceID2;

    OUString maAdapterVendorID;
    OUString maAdapterDeviceID;
    OUString maAdapterSubsysID;

    OUString maAdapterVendorID2;
    OUString maAdapterDeviceID2;
    OUString maAdapterSubsysID2;

    OUString maDeviceKey;
    OUString maDeviceKey2;

    OUString maDeviceString;
    OUString maDeviceString2;
    uint32_t mnWindowsVersion;

    bool mbHasDualGPU;
    bool mbHasDriverVersionMismatch;
    bool mbRDP;

    void GetData();
    OUString GetDeviceVendor(wgl::DeviceVendor eVendor);

    static OUString* mpDeviceVendors[wgl::DeviceVendorMax];

public:
    WinOpenGLDeviceInfo();
    virtual ~WinOpenGLDeviceInfo();

    virtual bool isDeviceBlocked();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
