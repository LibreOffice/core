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

#include <rtl/ustring.hxx>

#include <sallayout.hxx>
#include <svsys.h>
#include <win/salgdi.h>
#include <o3tl/sorted_vector.hxx>

// win32 specific logical font instance
class WinFontInstance : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance> WinFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

public:
    ~WinFontInstance() override;

    void SetGraphics(WinSalGraphics*);
    WinSalGraphics* GetGraphics() const { return m_pGraphics; }

    HFONT GetHFONT() const { return m_hFont; }
    // Return true if the font is for vertical writing.
    // I.e. the font name of the LOGFONT is prefixed with '@'.
    bool  IsCJKVerticalFont() const { return m_bIsCJKVerticalFont; }
    sal_Int32 GetTmDescent() const { return m_nTmDescent; }

    const WinFontFace * GetFontFace() const { return static_cast<const WinFontFace *>(LogicalFontInstance::GetFontFace()); }
    WinFontFace * GetFontFace() { return static_cast<WinFontFace *>(LogicalFontInstance::GetFontFace()); }

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

    IDWriteFontFace* GetDWFontFace() const;

private:
    explicit WinFontInstance(const WinFontFace&, const vcl::font::FontSelectPattern&);

    virtual void ImplInitHbFont(hb_font_t*) override;

    WinSalGraphics *m_pGraphics;
    HFONT m_hFont;
    bool  m_bIsCJKVerticalFont;
    sal_Int32 m_nTmDescent;
    mutable sal::systools::COMReference<IDWriteFontFace> mxDWFontFace;
};

class TextOutRenderer
{
protected:
    explicit TextOutRenderer() = default;
    TextOutRenderer(const TextOutRenderer &) = delete;
    TextOutRenderer & operator = (const TextOutRenderer &) = delete;

public:
    static TextOutRenderer & get(bool bUseDWrite, bool bRenderingModeNatural);

    virtual ~TextOutRenderer() = default;

    virtual bool operator ()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC,
        bool bRenderingModeNatural) = 0;
};

class ExTextOutRenderer : public TextOutRenderer
{
    ExTextOutRenderer(const ExTextOutRenderer &) = delete;
    ExTextOutRenderer & operator = (const ExTextOutRenderer &) = delete;

public:
    explicit ExTextOutRenderer() = default;

    bool operator ()(GenericSalLayout const &rLayout,
        SalGraphics &rGraphics,
        HDC hDC,
        bool bRenderingModeNatural) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
