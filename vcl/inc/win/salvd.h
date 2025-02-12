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

#pragma once

#include <sal/config.h>
#include <win/scoped_gdi.hxx>

#include <memory>

#include <salvd.hxx>

class WinSalGraphics;


class WinSalVirtualDevice : public SalVirtualDevice
{
private:
    HDC                     mhLocalDC;              // HDC or 0 for Cache Device
    ScopedHBITMAP           mhBmp;                  // Memory Bitmap
    HBITMAP                 mhDefBmp;               // Default Bitmap
    std::unique_ptr<WinSalGraphics> mpGraphics;     // current VirDev graphics
    WinSalVirtualDevice*    mpNext;                 // next VirDev
    sal_uInt16              mnBitCount;             // BitCount (0 or 1)
    bool                    mbGraphics;             // is Graphics used
    bool                    mbForeignDC;            // uses a foreign DC instead of a bitmap
    tools::Long                    mnWidth;
    tools::Long                    mnHeight;

public:
    HDC getHDC() const { return mhLocalDC; }
    WinSalGraphics* getGraphics() const { return mpGraphics.get(); }
    void setGraphics(WinSalGraphics* pVirGraphics) { mpGraphics.reset(pVirGraphics); }
    WinSalVirtualDevice* getNext() const { return mpNext; }

    WinSalVirtualDevice(HDC hDC, HBITMAP hBMP, sal_uInt16 nBitCount, bool bForeignDC, tools::Long nWidth, tools::Long nHeight, bool bIsScreen);
    virtual ~WinSalVirtualDevice() override;

    virtual SalGraphics*    AcquireGraphics() override;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool            SetSize( tools::Long nNewDX, tools::Long nNewDY ) override;

    static HBITMAP ImplCreateVirDevBitmap(HDC hDC, tools::Long nDX, tools::Long nDY, sal_uInt16 nBitCount, void **ppDummy);

    // SalGeometryProvider
    virtual tools::Long GetWidth() const override { return mnWidth; }
    virtual tools::Long GetHeight() const override { return mnHeight; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
