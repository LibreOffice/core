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

#ifndef INCLUDED_VCL_INC_QUARTZ_SALVD_H
#define INCLUDED_VCL_INC_QUARTZ_SALVD_H

#include <premac.h>
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#endif
#include <postmac.h>

#include <tools/long.hxx>

#include <quartz/salgdi.h>

#include <salvd.hxx>

class AquaSalGraphics;

class AquaSalVirtualDevice : public SalVirtualDevice
{
private:
    bool mbGraphicsUsed;             // is Graphics used
    bool mbForeignContext;           // is mxContext from outside VCL
    CGContextHolder maBitmapContext;
    int mnBitmapDepth;
    CGLayerHolder maLayer;
    AquaSalGraphics* mpGraphics;     // current VirDev graphics

    tools::Long mnWidth;
    tools::Long mnHeight;

    void Destroy();

public:
    AquaSalVirtualDevice( AquaSalGraphics* pGraphic, tools::Long nDX, tools::Long nDY, DeviceFormat eFormat );
    AquaSalVirtualDevice( AquaSalGraphics* pGraphic, tools::Long &nDX, tools::Long &nDY, DeviceFormat eFormat, const SystemGraphicsData& rData );
    virtual ~AquaSalVirtualDevice() override;

    virtual SalGraphics*            AcquireGraphics() override;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool                    SetSize( tools::Long nNewDX, tools::Long nNewDY ) override;

    tools::Long GetWidth() const override
    {
        return mnWidth;
    }

    tools::Long GetHeight() const override
    {
        return mnHeight;
    }
};

#endif // INCLUDED_VCL_INC_QUARTZ_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
