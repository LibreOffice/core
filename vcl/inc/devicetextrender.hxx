/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_DEVICETEXTRENDER_HXX
#define INCLUDED_VCL_INC_DEVICETEXTRENDER_HXX

#include <basebmp/bitmapdevice.hxx>
#include "textrender.hxx"

class VCL_DLLPUBLIC DeviceTextRenderImpl : public TextRenderImpl
{
public:
    virtual void setDevice(basebmp::BitmapDeviceSharedPtr& rDevice) = 0;
};

#endif

/* vim:set tabstop=4 shiftwidth=4 softtabstop=4 expandtab: */
