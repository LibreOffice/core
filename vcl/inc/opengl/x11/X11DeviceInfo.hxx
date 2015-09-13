/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_OPENGL_X11_X11DEVICEINFO_HXX
#define INCLUDED_VCL_INC_OPENGL_X11_X11DEVICEINFO_HXX

#include "opengl/DeviceInfo.hxx"

#include <rtl/string.hxx>

class X11OpenGLDeviceInfo : public OpenGLDeviceInfo
{
private:
    bool mbIsMesa;
    bool mbIsNVIDIA;
    bool mbIsFGLRX;
    bool mbIsNouveau;
    bool mbIsIntel;
    bool mbIsOldSwrast;
    bool mbIsLlvmpipe;
    bool mbHasTextureFromPixmap;

    OString maVendor;
    OString maRenderer;
    OString maVersion;
    OString maOS;
    OString maOSRelease;

    size_t mnGLMajorVersion;
    size_t mnMajorVersion;
    size_t mnMinorVersion;
    size_t mnRevisionVersion;

    void GetData();

public:
    X11OpenGLDeviceInfo();
    virtual ~X11OpenGLDeviceInfo();

    virtual bool isDeviceBlocked() SAL_OVERRIDE;

    const OString& GetVendor() const
    {
        return maVendor;
    }

    const OString& GetRenderer() const
    {
        return maRenderer;
    }

    const OString& GetVersion() const
    {
        return maVersion;
    }

    const OString& GetOS() const
    {
        return maOS;
    }

    const OString& GetOSRelease() const
    {
        return maOSRelease;
    }

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
