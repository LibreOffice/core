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

#include <vcl/outdev.hxx>

namespace vcl
{
class WindowOutputDevice final : public ::OutputDevice
{
public:
    WindowOutputDevice(vcl::Window& rOwnerWindow);
    virtual ~WindowOutputDevice() override;
    virtual void dispose() override;

    size_t GetSyncCount() const override { return 0x000000ff; }
    virtual void EnableRTL(bool bEnable = true) override;

    void Flush() override;

    void SaveBackground(VirtualDevice& rSaveDevice, const Point& rPos, const Size& rSize,
                        const Size&) const override;

    css::awt::DeviceInfo GetDeviceInfo() const override;

    virtual vcl::Region GetActiveClipRegion() const override;
    virtual vcl::Region GetOutputBoundsClipRegion() const override;

    virtual bool AcquireGraphics() const override;
    virtual void ReleaseGraphics(bool bRelease = true) override;

    Color GetBackgroundColor() const override;

    bool CanAnimate() const override { return true; }

    using ::OutputDevice::SetSettings;
    virtual void SetSettings(const AllSettings& rSettings) override;
    void SetSettings(const AllSettings& rSettings, bool bChild);

    bool CanEnableNativeWidget() const override;

    /** Get the vcl::Window that this OutputDevice belongs to, if any */
    virtual vcl::Window* GetOwnerWindow() const override { return mxOwnerWindow.get(); }

    virtual css::uno::Reference<css::rendering::XCanvas>
    ImplGetCanvas(bool bSpriteCanvas) const override;

private:
    virtual void InitClipRegion() override;

    void ImplClearFontData(bool bNewFontLists) override;
    void ImplRefreshFontData(bool bNewFontLists) override;
    void ImplInitMapModeObjects() override;

    virtual void CopyDeviceArea(SalTwoRect& aPosAry, bool bWindowInvalidate) override;
    virtual const OutputDevice* DrawOutDevDirectCheck(const OutputDevice& rSrcDev) const override;
    virtual void DrawOutDevDirectProcess(const OutputDevice& rSrcDev, SalTwoRect& rPosAry,
                                         SalGraphics* pSrcGraphics) override;
    virtual void ClipToPaintRegion(tools::Rectangle& rDstRect) override;
    virtual bool UsePolyPolygonForComplexGradient() override;

    VclPtr<vcl::Window> mxOwnerWindow;
};

}; // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
