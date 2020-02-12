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

#include <vcl/dllapi.h>

#include <opengl/DeviceInfo.hxx>
#include <driverblocklist.hxx>

#include <rtl/ustring.hxx>
#include <vector>
#include <cstdint>

class VCL_DLLPUBLIC WinOpenGLDeviceInfo : public OpenGLDeviceInfo
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

    bool mbHasDualGPU;
    bool mbRDP;

    void GetData();
    bool FindBlocklistedDeviceInList();

public:
    WinOpenGLDeviceInfo();

    virtual ~WinOpenGLDeviceInfo() override;

    virtual bool isDeviceBlocked() override;

    const OUString& GetDriverVersion() const
    {
        return maDriverVersion;
    }

    const OUString& GetDriverDate() const
    {
        return maDriverDate;
    }

    const OUString& GetDeviceID() const
    {
        return maDeviceID;
    }

    const OUString& GetAdapterVendorID() const
    {
        return maAdapterVendorID;
    }

    const OUString& GetAdapterDeviceID() const
    {
        return maAdapterDeviceID;
    }

    const OUString& GetAdapterSubsysID() const
    {
        return maAdapterSubsysID;
    }
    const OUString& GetDeviceKey() const
    {
        return maDeviceKey;
    }

    const OUString& GetDeviceString() const
    {
        return maDeviceString;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
