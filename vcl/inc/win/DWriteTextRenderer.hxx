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

#include <usp10.h>
#include <d2d1.h>
#include <dwrite.h>

#include <systools/win32/comtools.hxx>

#include <win/winlayout.hxx>

class D2DWriteTextOutRenderer : public TextOutRenderer
{
public:
    using MODE = std::pair<D2D1_TEXT_ANTIALIAS_MODE, DWRITE_RENDERING_MODE>;

    explicit D2DWriteTextOutRenderer(MODE mode);

    bool operator()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) override;

    HRESULT BindDC(HDC hDC, tools::Rectangle const & rRect = tools::Rectangle(0, 0, 1, 1));

    HRESULT CreateRenderTarget();

    bool Ready() const;

    void applyTextAntiAliasMode();

    MODE GetRenderingMode() const { return maRenderingMode; }

    static MODE GetMode(bool bRenderingModeNatural, bool bAntiAlias);

private:
    // This is a singleton object disable copy ctor and assignment operator
    D2DWriteTextOutRenderer(const D2DWriteTextOutRenderer &) = delete;
    D2DWriteTextOutRenderer & operator = (const D2DWriteTextOutRenderer &) = delete;

    IDWriteFontFace* GetDWriteFace(const WinFontInstance& rWinFont, float * lfSize) const;
    bool performRender(GenericSalLayout const &rLayout, SalGraphics &rGraphics, HDC hDC, bool& bRetry);

    sal::systools::COMReference<ID2D1Factory> mpD2DFactory;
    sal::systools::COMReference<ID2D1DCRenderTarget> mpRT;
    const D2D1_RENDER_TARGET_PROPERTIES mRTProps;

    MODE maRenderingMode;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
