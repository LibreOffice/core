/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_X11_SALVD_H
#define INCLUDED_VCL_INC_SKIA_X11_SALVD_H

#include <salvd.hxx>

class X11SkiaSalVirtualDevice final : public SalVirtualDevice
{
    SalDisplay* mpDisplay;
    std::unique_ptr<X11SalGraphics> mpGraphics;
    bool mbGraphics; // is Graphics used
    SalX11Screen mnXScreen;
    int mnWidth;
    int mnHeight;

public:
    X11SkiaSalVirtualDevice(const SalGraphics& rGraphics, sal_Int32 nDX, sal_Int32 nDY,
                            const SystemGraphicsData* pData,
                            std::unique_ptr<X11SalGraphics> pNewGraphics);
    virtual ~X11SkiaSalVirtualDevice() override;

    // SalGeometryProvider
    virtual sal_Int32 GetSgpMetric(vcl::SGPmetric eMetric) const override;

    SalDisplay* GetDisplay() const { return mpDisplay; }
    const SalX11Screen& GetXScreenNumber() const { return mnXScreen; }

    virtual SalGraphics* AcquireGraphics() override;
    virtual void ReleaseGraphics(SalGraphics* pGraphics) override;

    // Set new size, without saving the old contents
    virtual bool SetSizeUsingBuffer(sal_Int32 nNewDX, sal_Int32 nNewDY, sal_uInt8*,
                                    sal_Int32 = 100) override;
};

#endif // INCLUDED_VCL_INC_SKIA_X11_SALVD_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
