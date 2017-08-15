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

#ifndef INCLUDED_VCL_INC_WIN_SALVD_H
#define INCLUDED_VCL_INC_WIN_SALVD_H

#include <salvd.hxx>

class WinSalGraphics;


class WinSalVirtualDevice : public SalVirtualDevice
{
private:
    HDC                     mhLocalDC;              // HDC or 0 for Cache Device
    HBITMAP                 mhBmp;                  // Memory Bitmap
    HBITMAP                 mhDefBmp;               // Default Bitmap
    WinSalGraphics*         mpGraphics;             // current VirDev graphics
    WinSalVirtualDevice*    mpNext;                 // next VirDev
    sal_uInt16              mnBitCount;             // BitCount (0 or 1)
    bool                    mbGraphics;             // is Graphics used
    bool                    mbForeignDC;            // uses a foreign DC instead of a bitmap
    sal_Int32               mnWidth;
    sal_Int32               mnHeight;

public:
    HDC getHDC() const { return mhLocalDC; }
    WinSalGraphics* getGraphics() const { return mpGraphics; }
    void setGraphics(WinSalGraphics* pVirGraphics) { mpGraphics = pVirGraphics; }
    WinSalVirtualDevice* getNext() const { return mpNext; }

    WinSalVirtualDevice(HDC hDC = nullptr, HBITMAP hBMP = nullptr, sal_uInt16 nBitCount = 0, bool bForeignDC = false, sal_Int32 nWidth = 0, sal_Int32 nHeight = 0);
    virtual ~WinSalVirtualDevice() override;

    virtual SalGraphics*    AcquireGraphics() override;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) override;
    virtual bool            SetSize( sal_Int32 nNewDX, sal_Int32 nNewDY ) override;

    static HBITMAP ImplCreateVirDevBitmap(HDC hDC, sal_Int32 nDX, sal_Int32 nDY, sal_uInt16 nBitCount, void **ppDummy);

    // SalGeometryProvider
    virtual sal_Int32 GetWidth() const override { return mnWidth; }
    virtual sal_Int32 GetHeight() const override { return mnHeight; }
};


#endif // INCLUDED_VCL_INC_WIN_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
