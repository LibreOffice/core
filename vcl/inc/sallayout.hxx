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
#include <list>
#include <memory>
#include <vector>

#include <hb.h>

#include <com/sun/star/i18n/XBreakIterator.hpp>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/gen.hxx>
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
class VCL_PLUGIN_PUBLIC ImplLayoutRuns
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

class ImplLayoutArgs
{
public:
    // string related inputs
    LanguageTag const   maLanguageTag;
    SalLayoutFlags      mnFlags;
    const OUString&     mrStr;
    int const           mnMinCharPos;
    int const           mnEndCharPos;

    // performance hack
    vcl::TextLayoutCache const* m_pTextLayoutCache;

    // positioning related inputs
    const DeviceCoordinate* mpDXArray;     // in pixel units
    DeviceCoordinate    mnLayoutWidth;      // in pixel units
    int                 mnOrientation;      // in 0-3600 system

    // data for bidi and glyph+script fallback
    ImplLayoutRuns      maRuns;
    ImplLayoutRuns      maFallbackRuns;

                ImplLayoutArgs( const OUString& rStr,
                                int nMinCharPos, int nEndCharPos, SalLayoutFlags nFlags,
                                const LanguageTag& rLanguageTag,
                                vcl::TextLayoutCache const* pLayoutCache);

    void        SetLayoutWidth( DeviceCoordinate nWidth )       { mnLayoutWidth = nWidth; }
    void        SetDXArray( const DeviceCoordinate* pDXArray )  { mpDXArray = pDXArray; }
    void        SetOrientation( int nOrientation )  { mnOrientation = nOrientation; }

    void        ResetPos()
                    { maRuns.ResetPos(); }
    bool        GetNextPos( int* nCharPos, bool* bRTL )
                    { return maRuns.GetNextPos( nCharPos, bRTL ); }
    bool        GetNextRun( int* nMinRunPos, int* nEndRunPos, bool* bRTL );
    void        NeedFallback( int nMinRunPos, int nEndRunPos, bool bRTL )
                    { maFallbackRuns.AddRun( nMinRunPos, nEndRunPos, bRTL ); }
    // methods used by BiDi and glyph fallback
    bool        NeedFallback() const
                    { return !maFallbackRuns.IsEmpty(); }
    bool        PrepareFallback();

private:
    void        AddRun( int nMinCharPos, int nEndCharPos, bool bRTL );
};

// For nice SAL_INFO logging of ImplLayoutArgs values
std::ostream &operator <<(std::ostream& s, ImplLayoutArgs const &rArgs);

class VCL_PLUGIN_PUBLIC MultiSalLayout final : public SalLayout
{
public:
    void            DrawText(SalGraphics&) const override;
    sal_Int32       GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    DeviceCoordinate FillDXArray(DeviceCoordinate* pDXArray) const override;
    void            GetCaretPositions(int nArraySize, long* pCaretXArray) const override;
    bool            GetNextGlyph(const GlyphItem** pGlyph, Point& rPos, int& nStart,
                                 const PhysicalFontFace** pFallbackFont = nullptr,
                                 int* const pFallbackLevel = nullptr) const override;
    bool            GetOutline(basegfx::B2DPolyPolygonVector&) const override;
    bool            IsKashidaPosValid(int nCharPos) const override;

    // used only by OutputDevice::ImplLayout, TODO: make friend
    explicit        MultiSalLayout( std::unique_ptr<SalLayout> pBaseLayout );
    void            AddFallback(std::unique_ptr<SalLayout> pFallbackLayout, ImplLayoutRuns const &);
    bool            LayoutText(ImplLayoutArgs&, const SalLayoutGlyphs*) override;
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

class VCL_PLUGIN_PUBLIC GenericSalLayout : public SalLayout
{
    friend void MultiSalLayout::AdjustLayout(ImplLayoutArgs&);

public:
                    GenericSalLayout(LogicalFontInstance&);
                    ~GenericSalLayout() override;

    void            AdjustLayout(ImplLayoutArgs&) final override;
    bool            LayoutText(ImplLayoutArgs&, const SalLayoutGlyphs*) final override;
    void            DrawText(SalGraphics&) const final override;
    static std::shared_ptr<vcl::TextLayoutCache> CreateTextLayoutCache(OUString const&);
    const SalLayoutGlyphs* GetGlyphs() const final override;

    bool            IsKashidaPosValid(int nCharPos) const final override;

    // used by upper layers
    DeviceCoordinate GetTextWidth() const final override;
    DeviceCoordinate FillDXArray(DeviceCoordinate* pDXArray) const final override;
    sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const final override;
    void            GetCaretPositions(int nArraySize, long* pCaretXArray) const final override;

    // used by display layers
    LogicalFontInstance& GetFont() const
        { return m_GlyphItems.Impl()->GetFont(); }

    bool            GetNextGlyph(const GlyphItem** pGlyph, Point& rPos, int& nStart,
                                 const PhysicalFontFace** pFallbackFont = nullptr,
                                 int* const pFallbackLevel = nullptr) const override;

private:
    // for glyph+font+script fallback
    void            MoveGlyph(int nStart, long nNewXPos);
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

    SalLayoutGlyphs m_GlyphItems;

    OString         msLanguage;
    std::vector<hb_feature_t> maFeatures;

    hb_set_t*       mpVertGlyphs;
    const bool      mbFuzzing;
};

#undef SalGraphics

#endif // INCLUDED_VCL_INC_SALLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
