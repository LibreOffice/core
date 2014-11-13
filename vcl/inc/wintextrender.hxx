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

#ifndef INCLUDED_VCL_INC_WINTEXTRENDER_HXX
#define INCLUDED_VCL_INC_WINTEXTRENDER_HXX

#include <tools/rational.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/metric.hxx>

#include "salgdi.hxx"
#include "salglyphid.hxx"
#include "fontsubset.hxx"
#include <textrender.hxx>
#include <win/salgdi.h>

class ImplLayoutArgs;
class ServerFontLayout;
class PhysicalFontCollection;
class PhysicalFontFace;

/// Implementation of the GDI text rendering.
class WinTextRender : public TextRenderImpl
{
    /// Rendering for a printer?
    bool mbPrinter;

    /// Parent WinSalGraphics.
    WinSalGraphics& mrGraphics;

    HFONT                   mhFonts[ MAX_FALLBACK ];        ///< Font + Fallbacks
    const ImplWinFontData*  mpWinFontData[ MAX_FALLBACK ];  ///< pointer to the most recent font face
    ImplWinFontEntry*       mpWinFontEntry[ MAX_FALLBACK ]; ///< pointer to the most recent font instance
    float                   mfFontScale[ MAX_FALLBACK ];    ///< allows metrics emulation of huge font sizes
    float                   mfCurrentFontScale;
    HFONT                   mhDefFont;                      ///< DefaultFont

    LOGFONTA*               mpLogFont;                      ///< LOG-Font which is currently selected (only W9x)
    ImplFontAttrCache*      mpFontAttrCache;                ///< Cache font attributes from files in so/share/fonts
    BYTE*                   mpFontCharSets;                 ///< All Charsets for the current font
    BYTE                    mnFontCharSetCount;             ///< Number of Charsets of the current font; 0 - if not queried
    bool                    mbFontKernInit;                 ///< FALSE: FontKerns must be queried
    KERNINGPAIR*            mpFontKernPairs;                ///< Kerning Pairs of the current Font
    sal_uIntPtr             mnFontKernPairCount;            ///< Number of Kerning Pairs of the current Font

public:
    WinTextRender(bool bPrinter, WinSalGraphics& rGraphics);
    virtual ~WinTextRender();

    virtual void SetTextColor(SalColor nSalColor) SAL_OVERRIDE;

    virtual sal_uInt16 SetFont(FontSelectPattern*, int nFallbackLevel) SAL_OVERRIDE;

    virtual void GetFontMetric(ImplFontMetricData*, int nFallbackLevel) SAL_OVERRIDE;

    virtual const FontCharMapPtr GetFontCharMap() const SAL_OVERRIDE;

    virtual bool GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const SAL_OVERRIDE;

    virtual void GetDevFontList(PhysicalFontCollection*) SAL_OVERRIDE;

    virtual void ClearDevFontCache() SAL_OVERRIDE;

    virtual bool AddTempDevFont(PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName) SAL_OVERRIDE;

    virtual bool CreateFontSubset(const OUString& rToFile,
                                  const PhysicalFontFace*,
                                  sal_GlyphId* pGlyphIDs,
                                  sal_uInt8* pEncoding,
                                  sal_Int32* pWidths,
                                  int nGlyphs,
                                  FontSubsetInfo& rInfo) SAL_OVERRIDE;

    virtual const Ucs2SIntMap* GetFontEncodingVector(const PhysicalFontFace*, const Ucs2OStrMap** ppNonEncoded) SAL_OVERRIDE;

    virtual const void* GetEmbedFontData(const PhysicalFontFace*,
                                         const sal_Ucs* pUnicodes,
                                         sal_Int32* pWidths,
                                         FontSubsetInfo& rInfo,
                                         long* pDataLen) SAL_OVERRIDE;

    virtual void FreeEmbedFontData(const void* pData, long nDataLen) SAL_OVERRIDE;

    virtual void GetGlyphWidths(const PhysicalFontFace*,
                                bool bVertical,
                                Int32Vector& rWidths,
                                Ucs2UIntMap& rUnicodeEnc) SAL_OVERRIDE;

    virtual bool GetGlyphBoundRect(sal_GlyphId nIndex, Rectangle&) SAL_OVERRIDE;

    virtual bool GetGlyphOutline(sal_GlyphId nIndex, ::basegfx::B2DPolyPolygon&) SAL_OVERRIDE;

    virtual SalLayout* GetTextLayout(ImplLayoutArgs&, int nFallbackLevel) SAL_OVERRIDE;

    virtual void DrawServerFontLayout(const ServerFontLayout&) SAL_OVERRIDE;

    virtual SystemFontData GetSysFontData(int nFallbackLevel) const SAL_OVERRIDE;

private:
    HDC getHDC() const;

    /// Get kerning pairs of the current font.
    sal_uLong GetKernPairs();

    int GetMinKashidaWidth();

    class ScopedFont
    {
        public:
            explicit ScopedFont(WinTextRender & rData);

            ~ScopedFont();

        private:
            WinTextRender & m_rData;
            HFONT m_hOrigFont;
    };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
