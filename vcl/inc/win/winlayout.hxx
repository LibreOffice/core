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

class WinFontInstance;
struct VisualItem;

class WinLayout : public SalLayout
{
public:
                        WinLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);
    virtual             ~WinLayout();
    virtual void        InitFont() const override;
    void                SetFontScale( float f ) { mfFontScale = f; }
    HFONT               DisableFontScaling() const;

    SCRIPT_CACHE&       GetScriptCache() const;

    /// In the non-OpenGL case, call the DrawTextImpl directly, otherwise make
    /// sure we draw to an interim texture.
    virtual void        DrawText(SalGraphics&) const override;

    /// Draw to the provided HDC.
    virtual bool        DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const = 0;

    virtual bool        CacheGlyphs(SalGraphics& rGraphics) const = 0;
    virtual bool        DrawCachedGlyphs(SalGraphics& rGraphics) const = 0;

    HDC                 mhDC;               // WIN32 device handle
    HFONT               mhFont;             // WIN32 font handle
    int                 mnBaseAdv;          // x-offset relative to Layout origin
    float               mfFontScale;        // allows metrics emulation of huge font sizes
    bool                mbUseOpenGL;        ///< We need to render via OpenGL

    const WinFontFace& mrWinFontData;
    WinFontInstance&   mrWinFontEntry;
};

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);
    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( ImplLayoutArgs& ) override;
    virtual void    AdjustLayout( ImplLayoutArgs& ) override;
    virtual bool    DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;

    virtual bool    CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool    DrawCachedGlyphs(SalGraphics& rGraphics) const override;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances, int* pCharIndexes,
                                   const PhysicalFontFace** pFallbackFonts = NULL ) const override;

    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;

protected:
    void            Justify( DeviceCoordinate nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

private:
    int             mnGlyphCount;
    int             mnCharCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;    // if possible this is shared with mpGlyphAdvances[]
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;       // map rel char pos to char width
    int*            mpChars2Glyphs;     // map rel char pos to abs glyph pos
    int*            mpGlyphs2Chars;     // map abs glyph pos to abs char pos
    bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true=>RTL
    mutable long    mnWidth;

    int             mnNotdefWidth;
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout(HDC, const WinFontFace&, WinFontInstance&, bool bUseOpenGL);

    virtual bool    LayoutText( ImplLayoutArgs& ) override;
    virtual void    AdjustLayout( ImplLayoutArgs& ) override;
    virtual bool    DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;
    virtual bool    CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool    DrawCachedGlyphs(SalGraphics& rGraphics) const override;
    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                                   DeviceCoordinate* pGlyphAdvances, int* pCharPosAry,
                                   const PhysicalFontFace** pFallbackFonts = NULL ) const override;

    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const override;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;
    virtual bool    IsKashidaPosValid ( int nCharPos ) const override;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;
    virtual void    DisableGlyphInjection( bool bDisable ) override { mbDisableGlyphInjection = bDisable; }

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
    OUString        msTheString;        // Sadly we need it in GetNextGlyphs(), to be able to call GetVerticalFlags()
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
    bool           mbUseGLyphy;
    bool           DrawCachedGlyphsUsingGLyphy(SalGraphics& rGraphics) const;
    bool           DrawCachedGlyphsUsingTextures(SalGraphics& rGraphics) const;
};

#if ENABLE_GRAPHITE

class GraphiteLayoutWinImpl : public GraphiteLayout
{
public:
    GraphiteLayoutWinImpl(const gr_face * pFace, WinFontInstance & rFont)
        throw()
    : GraphiteLayout(pFace), mrFont(rFont) {};
    virtual ~GraphiteLayoutWinImpl() throw() {};
    virtual sal_GlyphId getKashidaGlyph(int & rWidth) override;
private:
    WinFontInstance & mrFont;
};

/// This class uses the SIL Graphite engine to provide complex text layout services to the VCL
class GraphiteWinLayout : public WinLayout
{
private:
    gr_font * mpFont;
    grutils::GrFeatureParser * mpFeatures;
    mutable GraphiteLayoutWinImpl maImpl;
public:
    GraphiteWinLayout(HDC hDC, const WinFontFace& rWFD, WinFontInstance& rWFE, bool bUseOpenGL) throw();
    virtual ~GraphiteWinLayout();

    // used by upper layers
    virtual bool  LayoutText( ImplLayoutArgs& ) override;    // first step of layout
    virtual void  AdjustLayout( ImplLayoutArgs& ) override;  // adjusting after fallback etc.
    virtual bool  DrawTextImpl(HDC hDC, const Rectangle* pRectToErase, Point* pPos, int* pGetNextGlypInfo) const override;
    virtual bool  CacheGlyphs(SalGraphics& rGraphics) const override;
    virtual bool  DrawCachedGlyphs(SalGraphics& rGraphics) const override;

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra=0, int nFactor=1) const override;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const override;

    virtual void  GetCaretPositions( int nArraySize, long* pCaretXArray ) const override;

    // methods using glyph indexing
    virtual int   GetNextGlyphs(int nLen, sal_GlyphId* pGlyphIdxAry, ::Point & rPos, int&,
                                DeviceCoordinate* pGlyphAdvAry = NULL, int* pCharPosAry = NULL,
                                const PhysicalFontFace** pFallbackFonts = NULL ) const override;

    // used by glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos ) override;
    virtual void    DropGlyph( int nStart ) override;
    virtual void    Simplify( bool bIsBase ) override;
};

#endif // ENABLE_GRAPHITE

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
