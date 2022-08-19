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

#ifndef INCLUDED_VCL_INC_SALLAYOUT_HXX
#define INCLUDED_VCL_INC_SALLAYOUT_HXX

#include <sal/config.h>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <tools/gen.hxx>
#include <tools/degree.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <vcl/dllapi.h>
#include <vcl/devicecoordinate.hxx>
#include <vcl/vclenum.hxx> // for typedef sal_UCS4
#include <vcl/vcllayout.hxx>

#include "ImplLayoutRuns.hxx"
#include "impglyphitem.hxx"

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <hb.h>

#include <iostream>
#include <memory>
#include <vector>

#define MAX_FALLBACK 16

class GenericSalLayout;
class SalGraphics;
enum class SalLayoutFlags;

namespace vcl::font {
    class PhysicalFontFace;
}

namespace vcl::text {
    class TextLayoutCache;
}

class MultiSalLayout final : public SalLayout
{
public:
    void            DrawText(SalGraphics&) const override;
    sal_Int32       GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    DeviceCoordinate FillDXArray(std::vector<DeviceCoordinate>* pDXArray) const override;
    void            GetCaretPositions(int nArraySize, sal_Int32* pCaretXArray) const override;
    bool            GetNextGlyph(const GlyphItem** pGlyph, DevicePoint& rPos, int& nStart,
                                 const LogicalFontInstance** ppGlyphFont = nullptr,
                                 const vcl::font::PhysicalFontFace** pFallbackFont = nullptr) const override;
    bool            GetOutline(basegfx::B2DPolyPolygonVector&) const override;
    bool            IsKashidaPosValid(int nCharPos, int nNextCharPos) const override;
    SalLayoutGlyphs GetGlyphs() const final override;

    // used only by OutputDevice::ImplLayout, TODO: make friend
    explicit        MultiSalLayout( std::unique_ptr<SalLayout> pBaseLayout );
    void            AddFallback(std::unique_ptr<SalLayout> pFallbackLayout, ImplLayoutRuns const &);
    // give up ownership of the initial pBaseLayout taken by the ctor
    std::unique_ptr<SalLayout>  ReleaseBaseLayout();
    bool            LayoutText(vcl::text::ImplLayoutArgs&, const SalLayoutGlyphsImpl*) override;
    void            AdjustLayout(vcl::text::ImplLayoutArgs&) override;
    void            InitFont() const override;

    void SetIncomplete(bool bIncomplete);

    void            ImplAdjustMultiLayout(vcl::text::ImplLayoutArgs& rArgs,
                                          vcl::text::ImplLayoutArgs& rMultiArgs,
                                          const double* pMultiDXArray);

    virtual         ~MultiSalLayout() override;

private:
                    MultiSalLayout( const MultiSalLayout& ) = delete;
                    MultiSalLayout& operator=( const MultiSalLayout& ) = delete;

    std::unique_ptr<GenericSalLayout> mpLayouts[ MAX_FALLBACK ];
    ImplLayoutRuns  maFallbackRuns[ MAX_FALLBACK ];
    int             mnLevel;
    bool            mbIncomplete;
};

class VCL_DLLPUBLIC GenericSalLayout : public SalLayout
{
    friend void MultiSalLayout::ImplAdjustMultiLayout(
            vcl::text::ImplLayoutArgs& rArgs,
            vcl::text::ImplLayoutArgs& rMultiArgs,
            const double* pMultiDXArray);

public:
                    GenericSalLayout(LogicalFontInstance&);
                    ~GenericSalLayout() override;

    void            AdjustLayout(vcl::text::ImplLayoutArgs&) final override;
    bool            LayoutText(vcl::text::ImplLayoutArgs&, const SalLayoutGlyphsImpl*) final override;
    void            DrawText(SalGraphics&) const final override;
    SalLayoutGlyphs GetGlyphs() const final override;

    bool            IsKashidaPosValid(int nCharPos, int nNextCharPos) const final override;

    // used by upper layers
    DeviceCoordinate GetTextWidth() const final override;
    DeviceCoordinate FillDXArray(std::vector<DeviceCoordinate>* pDXArray) const final override;
    sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const final override;
    void            GetCaretPositions(int nArraySize, sal_Int32* pCaretXArray) const final override;

    // used by display layers
    LogicalFontInstance& GetFont() const
        { return *m_GlyphItems.GetFont(); }

    bool            GetNextGlyph(const GlyphItem** pGlyph, DevicePoint& rPos, int& nStart,
                                 const LogicalFontInstance** ppGlyphFont = nullptr,
                                 const vcl::font::PhysicalFontFace** pFallbackFont = nullptr) const override;

    const SalLayoutGlyphsImpl& GlyphsImpl() const { return m_GlyphItems; }

private:
    // for glyph+font+script fallback
    void            MoveGlyph(int nStart, tools::Long nNewXPos);
    void            DropGlyph(int nStart);
    void            Simplify(bool bIsBase);

                    GenericSalLayout( const GenericSalLayout& ) = delete;
                    GenericSalLayout& operator=( const GenericSalLayout& ) = delete;

    void            ApplyDXArray(const double*, const sal_Bool*);
    void            Justify(DeviceCoordinate nNewWidth);
    void            ApplyAsianKerning(const OUString& rStr);

    void            GetCharWidths(std::vector<DeviceCoordinate>& rCharWidths) const;

    void            SetNeedFallback(vcl::text::ImplLayoutArgs&, sal_Int32, bool);

    bool            HasVerticalAlternate(sal_UCS4 aChar, sal_UCS4 aNextChar);

    void            ParseFeatures(std::u16string_view name);

    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;

    SalLayoutGlyphsImpl m_GlyphItems;

    OString         msLanguage;
    std::vector<hb_feature_t> maFeatures;

    hb_set_t*       mpVertGlyphs;
    const bool      mbFuzzing;
};

#endif // INCLUDED_VCL_INC_SALLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
