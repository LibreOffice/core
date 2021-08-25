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

#include <iostream>
#include <memory>
#include <vector>

#include <hb.h>

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/gen.hxx>
#include <tools/degree.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx> // for typedef sal_UCS4
#include <vcl/devicecoordinate.hxx>
#include <vcl/vcllayout.hxx>

#include "impglyphitem.hxx"

#define MAX_FALLBACK 16


class SalGraphics;
class PhysicalFontFace;
class GenericSalLayout;
enum class SalLayoutFlags;
namespace vcl {
    class TextLayoutCache;
}

// used for managing runs e.g. for BiDi, glyph and script fallback
class ImplLayoutRuns
{
private:
    int                 mnRunIndex;
    std::vector<int>    maRuns;

public:
            ImplLayoutRuns() { mnRunIndex = 0; maRuns.reserve(8); }

    void    Clear()             { maRuns.clear(); }
    void    AddPos( int nCharPos, bool bRTL );
    void    AddRun( int nMinRunPos, int nEndRunPos, bool bRTL );

    bool    IsEmpty() const     { return maRuns.empty(); }
    void    ResetPos()          { mnRunIndex = 0; }
    void    NextRun()           { mnRunIndex += 2; }
    bool    GetRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL ) const;
    bool    GetNextPos( int* nCharPos, bool* bRTL );
    bool    PosIsInRun( int nCharPos ) const;
    bool    PosIsInAnyRun( int nCharPos ) const;
};

class MultiSalLayout final : public SalLayout
{
public:
    void            DrawText(SalGraphics&) const override;
    sal_Int32       GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    DeviceCoordinate FillDXArray(DeviceCoordinate* pDXArray) const override;
    void            GetCaretPositions(int nArraySize, tools::Long* pCaretXArray) const override;
    bool            GetNextGlyph(const GlyphItem** pGlyph, Point& rPos, int& nStart,
                                 const PhysicalFontFace** pFallbackFont = nullptr) const override;
    bool            GetOutline(basegfx::B2DPolyPolygonVector&) const override;
    bool            IsKashidaPosValid(int nCharPos) const override;
    SalLayoutGlyphs GetGlyphs() const final override;

    // used only by OutputDevice::ImplLayout, TODO: make friend
    explicit        MultiSalLayout( std::unique_ptr<SalLayout> pBaseLayout );
    void            AddFallback(std::unique_ptr<SalLayout> pFallbackLayout, ImplLayoutRuns const &);
    // give up ownership of the initial pBaseLayout taken by the ctor
    std::unique_ptr<SalLayout>  ReleaseBaseLayout();
    bool            LayoutText(ImplLayoutArgs&, const SalLayoutGlyphsImpl*) override;
    void            AdjustLayout(ImplLayoutArgs&) override;
    void            InitFont() const override;

    void SetIncomplete(bool bIncomplete);

public:
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
    friend void MultiSalLayout::AdjustLayout(ImplLayoutArgs&);

public:
                    GenericSalLayout(LogicalFontInstance&);
                    ~GenericSalLayout() override;

    void            AdjustLayout(ImplLayoutArgs&) final override;
    bool            LayoutText(ImplLayoutArgs&, const SalLayoutGlyphsImpl*) final override;
    void            DrawText(SalGraphics&) const final override;
    static std::shared_ptr<vcl::TextLayoutCache> CreateTextLayoutCache(OUString const&);
    SalLayoutGlyphs GetGlyphs() const final override;

    bool            IsKashidaPosValid(int nCharPos) const final override;

    // used by upper layers
    DeviceCoordinate GetTextWidth() const final override;
    DeviceCoordinate FillDXArray(DeviceCoordinate* pDXArray) const final override;
    sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const final override;
    void            GetCaretPositions(int nArraySize, tools::Long* pCaretXArray) const final override;

    // used by display layers
    LogicalFontInstance& GetFont() const
        { return *m_GlyphItems.GetFont(); }

    bool            GetNextGlyph(const GlyphItem** pGlyph, Point& rPos, int& nStart,
                                 const PhysicalFontFace** pFallbackFont = nullptr) const override;

    const SalLayoutGlyphsImpl& GlyphsImpl() const { return m_GlyphItems; }

private:
    // for glyph+font+script fallback
    void            MoveGlyph(int nStart, tools::Long nNewXPos);
    void            DropGlyph(int nStart);
    void            Simplify(bool bIsBase);

                    GenericSalLayout( const GenericSalLayout& ) = delete;
                    GenericSalLayout& operator=( const GenericSalLayout& ) = delete;

    void            ApplyDXArray(const ImplLayoutArgs&);
    void            Justify(DeviceCoordinate nNewWidth);
    void            ApplyAsianKerning(const OUString& rStr);

    void            GetCharWidths(DeviceCoordinate* pCharWidths) const;

    void            SetNeedFallback(ImplLayoutArgs&, sal_Int32, bool);

    bool            HasVerticalAlternate(sal_UCS4 aChar, sal_UCS4 aNextChar);

    void            ParseFeatures(const OUString& name);

    css::uno::Reference<css::i18n::XBreakIterator> mxBreak;

    SalLayoutGlyphsImpl m_GlyphItems;

    OString         msLanguage;
    std::vector<hb_feature_t> maFeatures;

    hb_set_t*       mpVertGlyphs;
    const bool      mbFuzzing;
};

#undef SalGraphics

#endif // INCLUDED_VCL_INC_SALLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
