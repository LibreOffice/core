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

#include "premac.h"
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#endif
#include "postmac.h"

#include "quartz/salgdi.h"

#include "salvd.hxx"

struct SalVirDevData
{
};

typedef struct SalVirDevData   SalVirDevData;

class AquaSalGraphics;

class AquaSalVirtualDevice : public SalVirtualDevice
{
private:
    bool mbGraphicsUsed;             // is Graphics used
    bool mbForeignContext;           // is mxContext from outside VCL
    CGContextRef mxBitmapContext;
    int mnBitmapDepth;
    CGLayerRef mxLayer;              // Quartz layer
    AquaSalGraphics* mpGraphics;     // current VirDev graphics

    void Destroy();

public:
    AquaSalVirtualDevice( AquaSalGraphics* pGraphic, long &nDX, long &nDY, DeviceFormat eFormat, const SystemGraphicsData *pData );
    virtual ~AquaSalVirtualDevice();

    virtual SalGraphics*            AcquireGraphics() override;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool                    SetSize( long nNewDX, long nNewDY ) override;

    virtual long GetWidth() const override;
    virtual long GetHeight() const override;
};

#endif // INCLUDED_VCL_INC_QUARTZ_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
