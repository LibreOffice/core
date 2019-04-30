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

#ifndef INCLUDED_VCL_INC_WIN_DWRITERENDERER_HXX
#define INCLUDED_VCL_INC_WIN_DWRITERENDERER_HXX

#include <usp10.h>
#include <d2d1.h>
#include <dwrite.h>

#include <win/winlayout.hxx>

enum class D2DTextAntiAliasMode
{
    Default,
    Aliased,
    AntiAliased,
    ClearType,
};

class D2DWriteTextOutRenderer : public TextOutRenderer
{
public:
    explicit D2DWriteTextOutRenderer();
    virtual ~D2DWriteTextOutRenderer() override;

    bool operator ()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC) override;

    HRESULT BindDC(HDC hDC, tools::Rectangle const & rRect = tools::Rectangle(0, 0, 1, 1));

    bool BindFont(HDC hDC) /*override*/;
    bool ReleaseFont() /*override*/;

    std::vector<tools::Rectangle>  GetGlyphInkBoxes(uint16_t const * pGid, uint16_t const * pGidEnd) const /*override*/;
    ID2D1RenderTarget * GetRenderTarget() const { return mpRT; }
    IDWriteFontFace   * GetFontFace() const { return mpFontFace; }
    float               GetEmHeight() const { return mlfEmHeight; }

    HRESULT CreateRenderTarget();

    bool Ready() const;

    void applyTextAntiAliasMode();
    void changeTextAntiAliasMode(D2DTextAntiAliasMode eMode);

private:
    // This is a singleton object disable copy ctor and assignment operator
    D2DWriteTextOutRenderer(const D2DWriteTextOutRenderer &) = delete;
    D2DWriteTextOutRenderer & operator = (const D2DWriteTextOutRenderer &) = delete;

    bool GetDWriteFaceFromHDC(HDC hDC, IDWriteFontFace ** ppFontFace, float * lfSize) const;
    bool performRender(GenericSalLayout const &rLayout, SalGraphics &rGraphics, HDC hDC, bool& bRetry);

    ID2D1Factory        * mpD2DFactory;
    IDWriteFactory      * mpDWriteFactory;
    IDWriteGdiInterop   * mpGdiInterop;
    ID2D1DCRenderTarget * mpRT;
    const D2D1_RENDER_TARGET_PROPERTIES mRTProps;

    IDWriteFontFace * mpFontFace;
    float             mlfEmHeight;
    HDC               mhDC;
    D2DTextAntiAliasMode meTextAntiAliasMode;
};

/**
 * Sets and unsets the needed DirectWrite transform to support the font's horizontal scaling and
 * rotation.
 */
class WinFontTransformGuard
{
public:
    WinFontTransformGuard(ID2D1RenderTarget* pRenderTarget, float fHScale, const GenericSalLayout& rLayout, const D2D1_POINT_2F& rBaseline);
    ~WinFontTransformGuard();

private:
    ID2D1RenderTarget* mpRenderTarget;
    D2D1::Matrix3x2F maTransform;
};

#endif // INCLUDED_VCL_INC_WIN_DWRITERENDERER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
