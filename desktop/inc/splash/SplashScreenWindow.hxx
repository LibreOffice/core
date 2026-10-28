/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <vcl/introwin.hxx>
#include <vcl/virdev.hxx>

class SplashScreen;

class SplashScreenWindow : public IntroWindow
{
    SplashScreen* m_pSplash;

    Bitmap m_aIntroBmp;
    Color m_aProgressFrameColor;
    Color m_aProgressBarColor;
    Color m_aProgressTextColor;
    bool m_bNativeProgress;
    OUString m_sAppName;
    bool m_bShowLogo;
    bool m_bFullScreenSplash;
    tools::Long m_nHeight;
    tools::Long m_nWidth;
    tools::Long m_nTlx;
    tools::Long m_nTly;
    tools::Long m_nBarwidth;
    tools::Long m_nBarheight;
    tools::Long m_nBarspace;
    tools::Long m_nTextBaseline;
    double m_fXPos;
    double m_fYPos;
    double m_fWidth;
    double m_fHeight;

    static constexpr tools::Long _xoffset = 12;
    static constexpr tools::Long _yoffset = 18;

    ScopedVclPtr<VirtualDevice> m_pVDev;

public:
    explicit SplashScreenWindow(SplashScreen* pSplash, const OUString& rAppName);
    virtual ~SplashScreenWindow() override { disposeOnce(); }
    virtual void dispose() override;
    virtual void SetOutputSizePixel(const Size& rNewSize) override;
    // workwindow
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    void Redraw();

private:
    void loadConfig();
    void SetScreenBitmap(Bitmap& rBitmap);
    static void determineProgressRatioValues(double& rXRelPos, double& rYRelPos, double& rRelWidth,
                                             double& rRelHeight);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
