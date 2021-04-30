/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/sysdata.hxx>

#include <unx/salunx.h>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>

#include <skia/x11/salvd.hxx>

void X11SalGraphics::Init(X11SkiaSalVirtualDevice* pDevice)
{
    SalDisplay* pDisplay = pDevice->GetDisplay();

    m_nXScreen = pDevice->GetXScreenNumber();
    m_pColormap = &pDisplay->GetColormap(m_nXScreen);

    m_pVDev = pDevice;
    m_pFrame = nullptr;

    bWindow_ = pDisplay->IsDisplay();
    bVirDev_ = true;

    mxImpl->Init();
}

X11SkiaSalVirtualDevice::X11SkiaSalVirtualDevice(const SalGraphics& rGraphics, tools::Long nDX,
                                                 tools::Long nDY, const SystemGraphicsData* pData,
                                                 std::unique_ptr<X11SalGraphics> pNewGraphics)
    : mpGraphics(std::move(pNewGraphics))
    , mbGraphics(false)
    , mnXScreen(0)
{
    assert(mpGraphics);

    // TODO Check where a VirtualDevice is created from SystemGraphicsData
    assert(pData == nullptr);
    (void)pData;

    mpDisplay = vcl_sal::getSalDisplay(GetGenericUnixSalData());
    mnXScreen = static_cast<const X11SalGraphics&>(rGraphics).GetScreenNumber();
    mnWidth = nDX;
    mnHeight = nDY;
    mpGraphics->Init(this);
}

X11SkiaSalVirtualDevice::~X11SkiaSalVirtualDevice() {}

SalGraphics* X11SkiaSalVirtualDevice::AcquireGraphics()
{
    if (mbGraphics)
        return nullptr;

    if (mpGraphics)
        mbGraphics = true;

    return mpGraphics.get();
}

void X11SkiaSalVirtualDevice::ReleaseGraphics(SalGraphics*) { mbGraphics = false; }

bool X11SkiaSalVirtualDevice::SetSize(tools::Long nDX, tools::Long nDY)
{
    if (!nDX)
        nDX = 1;
    if (!nDY)
        nDY = 1;

    mnWidth = nDX;
    mnHeight = nDY;
    if (mpGraphics)
        mpGraphics->Init(this);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
