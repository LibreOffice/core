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

#ifndef INCLUDED_VCL_WIN_SOURCE_GDI_WINLAYOUT_HXX
#define INCLUDED_VCL_WIN_SOURCE_GDI_WINLAYOUT_HXX

#include <rtl/ustring.hxx>

#include <sallayout.hxx>
#include <svsys.h>
#include <win/salgdi.h>

#include <usp10.h>

// Graphite headers
#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite_layout.hxx>
#endif

class ImplWinFontEntry;
struct VisualItem;

class WinLayout : public SalLayout
{
public:
                        WinLayout(HDC, const ImplWinFontData&, ImplWinFontEntry&, bool bUseOpenGL);
    virtual void        InitFont() const SAL_OVERRIDE;
    void                SetFontScale( float f ) { mfFontScale = f; }
    HFONT               DisableFontScaling() const;

    SCRIPT_CACHE&       GetScriptCache() const;

    /// In the non-OpenGL case, call the DrawTextImpl directly, otherwise make
    /// sure we draw to an interim texture.
    virtual void        DrawText(SalGraphics&) const SAL_OVERRIDE;

    /// Draw to the provided HDC.
    virtual void        DrawTextImpl(HDC hDC) const = 0;

    virtual bool        CacheGlyphs(SalGraphics& rGraphics) const = 0;
    virtual bool        DrawCachedGlyphs(SalGraphics& rGraphics) const = 0;

    HDC                 mhDC;               // WIN32 device handle
    HFONT               mhFont;             // WIN32 font handle
    int                 mnBaseAdv;          // x-offset relative to Layout origin
    float               mfFontScale;        // allows metrics emulation of huge font sizes
    bool                mbUseOpenGL;        ///< We need to render via OpenGL

    const ImplWinFontData& mrWinFontData;
    ImplWinFontEntry&   mrWinFontEntry;
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout(HDC, const ImplWinFontData&, ImplWinFontEntry&, bool bUseOpenGL);

    virtual bool    LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;
    virtual void    DrawTextImpl(HDC hDC) const SAL_OVERRIDE;
    virtual bool    CacheGlyphs(SalGraphics& rGraphics) const SAL_OVERRIDE;
    virtual bool    DrawCachedGlyphs(SalGraphics& rGraphics) const SAL_OVERRIDE;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances, int* pCharPosAry,
                                   const PhysicalFontFace** pFallbackFonts = NULL ) const SAL_OVERRIDE;

    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const SAL_OVERRIDE;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const SAL_OVERRIDE;
    virtual bool    IsKashidaPosValid ( int nCharPos ) const SAL_OVERRIDE;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    virtual void    DropGlyph( int nStart ) SAL_OVERRIDE;
    virtual void    Simplify( bool bIsBase ) SAL_OVERRIDE;
    virtual void    DisableGlyphInjection( bool bDisable ) SAL_OVERRIDE { mbDisableGlyphInjection = bDisable; }

protected:
    virtual         ~UniscribeLayout();

    void            Justify( DeviceCoordinate nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

    bool            GetItemSubrange( const VisualItem&,
                        int& rMinIndex, int& rEndIndex ) const;

private:
    // item specific info
    SCRIPT_ITEM*    mpScriptItems;      // in logical order
    VisualItem*     mpVisualItems;      // in visual order
    int             mnItemCount;        // number of visual items

    // string specific info
    // everything is in logical order
    int             mnCharCapacity;
    WORD*           mpLogClusters;      // map from absolute_char_pos to relative_glyph_pos
    int*            mpCharWidths;       // map from absolute_char_pos to char_width
    int             mnSubStringMin;     // char_pos of first char in context

    // glyph specific info
    // everything is in visual order
    int             mnGlyphCount;
    int             mnGlyphCapacity;
    int*            mpGlyphAdvances;    // glyph advance width before justification
    int*            mpJustifications;   // glyph advance width after justification
    WORD*           mpOutGlyphs;        // glyphids in visual order
    GOFFSET*        mpGlyphOffsets;     // glyph offsets to the "naive" layout
    SCRIPT_VISATTR* mpVisualAttrs;      // glyph visual attributes
    mutable int*    mpGlyphs2Chars;     // map from absolute_glyph_pos to absolute_char_pos

    // kashida stuff
    void InitKashidaHandling();
    void KashidaItemFix( int nMinGlyphPos, int nEndGlyphPos );
    bool KashidaWordFix( int nMinGlyphPos, int nEndGlyphPos, int* pnCurrentPos );

    int            mnMinKashidaWidth;
    int            mnMinKashidaGlyph;
    bool           mbDisableGlyphInjection;
};

#if ENABLE_GRAPHITE

class GraphiteLayoutWinImpl : public GraphiteLayout
{
public:
    GraphiteLayoutWinImpl(const gr_face * pFace, ImplWinFontEntry & rFont)
        throw()
    : GraphiteLayout(pFace), mrFont(rFont) {};
    virtual ~GraphiteLayoutWinImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & rWidth) SAL_OVERRIDE;
private:
    ImplWinFontEntry & mrFont;
};

/// This class uses the SIL Graphite engine to provide complex text layout services to the VCL
class GraphiteWinLayout : public WinLayout
{
private:
    gr_font * mpFont;
    grutils::GrFeatureParser * mpFeatures;
    mutable GraphiteLayoutWinImpl maImpl;
public:
    GraphiteWinLayout(HDC hDC, const ImplWinFontData& rWFD, ImplWinFontEntry& rWFE, bool bUseOpenGL) throw();
    virtual ~GraphiteWinLayout();

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& ) SAL_OVERRIDE;    // first step of layout
    virtual void  AdjustLayout( ImplLayoutArgs& ) SAL_OVERRIDE;  // adjusting after fallback etc.
    virtual void  DrawTextImpl(HDC hDC) const SAL_OVERRIDE;
    virtual bool  CacheGlyphs(SalGraphics& rGraphics) const SAL_OVERRIDE;
    virtual bool  DrawCachedGlyphs(SalGraphics& rGraphics) const SAL_OVERRIDE;

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra=0, int nFactor=1) const SAL_OVERRIDE;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const SAL_OVERRIDE;

    virtual void  GetCaretPositions( int nArraySize, long* pCaretXArray ) const SAL_OVERRIDE;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
                                DeviceCoordinate* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
                                const PhysicalFontFace** pFallbackFonts = NULL ) const SAL_OVERRIDE;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) SAL_OVERRIDE;
    virtual void    DropGlyph( int nStart ) SAL_OVERRIDE;
    virtual void    Simplify( bool bIsBase ) SAL_OVERRIDE;
};

#endif // ENABLE_GRAPHITE

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
