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

#include "SplashScreenBase.hxx"
#include "splash_dllapi.h"

#include <cppuhelper/supportsservice.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/introwin.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/virdev.hxx>

class SplashScreen;

class SplashScreenWindow : public IntroWindow
{
public:
    SplashScreen* m_pSplash;
    ScopedVclPtr<VirtualDevice> m_pVDev;
    explicit SplashScreenWindow(SplashScreen*);
    virtual ~SplashScreenWindow() override { disposeOnce(); }
    virtual void dispose() override;
    // workwindow
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    void Redraw();
};

class SPLASH_DLLPUBLIC SplashScreen : public SplashScreenBase
{
    friend class SplashScreenWindow;

private:
    VclPtr<SplashScreenWindow> m_pWindow;

    DECL_LINK(AppEventListenerHdl, VclSimpleEvent&, void);
    void loadConfig();
    void updateStatus();
    void SetScreenBitmap(Bitmap& rBitmap);
    static void determineProgressRatioValues(double& rXRelPos, double& rYRelPos, double& rRelWidth,
                                             double& rRelHeight);

    Bitmap m_aIntroBmp;
    Color m_aProgressFrameColor;
    Color m_aProgressBarColor;
    Color m_aProgressTextColor;
    bool m_bNativeProgress;
    OUString m_sAppName;
    OUString m_sProgressText;

    sal_Int32 m_nMax;
    sal_Int32 m_nProgress;
    bool m_bPaintProgress;
    bool m_bShowLogo;
    bool m_bFullScreenSplash;
    bool m_bProgressEnd;
    tools::Long m_nHeight, m_nWidth, m_nTlx, m_nTly, m_nBarwidth;
    tools::Long m_nBarheight, m_nBarspace, m_nTextBaseline;
    double m_fXPos, m_fYPos;
    double m_fWidth, m_fHeight;
    static constexpr tools::Long _xoffset = 12;
    static constexpr tools::Long _yoffset = 18;

public:
    SplashScreen(const OUString& rAppName);
    virtual ~SplashScreen() override;

    virtual void end() override;
    virtual void reset() override;
    virtual void setText(const OUString& aText) override;
    virtual void setValue(sal_Int32 nValue) override;
    virtual void start(const OUString& aText, sal_Int32 nRange) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
