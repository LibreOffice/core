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

#include <sallayout.hxx>
#include <salgeom.hxx>
#include <salgdi.hxx>
#include <fontinstance.hxx>
#include <fontattributes.hxx>
#include <font/PhysicalFontFace.hxx>
#include <impfont.hxx>
#include <vcl/fontcapabilities.hxx>
#include <vcl/fontcharmap.hxx>

#include <memory>
#include <unordered_set>

#ifndef INCLUDED_PRE_POST_WIN_H
#define INCLUDED_PRE_POST_WIN_H
#  include <prewin.h>
#  include <postwin.h>
#endif

#include <hb-ot.h>
#include <dwrite.h>

namespace vcl::font
{
class PhysicalFontCollection;
class FontSelectPattern;
}
class WinFontInstance;
class ImplFontAttrCache;
class SalGraphicsImpl;
class WinSalGraphicsImplBase;
class ImplFontMetricData;

#define RGB_TO_PALRGB(nRGB)         ((nRGB)|0x02000000)
#define PALRGB_TO_RGB(nPalRGB)      ((nPalRGB)&0x00ffffff)

// win32 specific physically available font face
class WinFontFace : public vcl::font::PhysicalFontFace
{
public:
    explicit                WinFontFace( const FontAttributes&,
                                BYTE eWinCharSet,
                                BYTE nPitchAndFamily  );
    virtual                 ~WinFontFace() override;

    virtual rtl::Reference<LogicalFontInstance> CreateFontInstance( const vcl::font::FontSelectPattern& ) const override;
    virtual sal_IntPtr      GetFontId() const override;
    void                    SetFontId( sal_IntPtr nId ) { mnId = nId; }
    void                    UpdateFromHDC( HDC ) const;

    bool                    HasChar( sal_uInt32 cChar ) const;

    BYTE                    GetCharSet() const          { return meWinCharSet; }
    BYTE                    GetPitchAndFamily() const   { return mnPitchAndFamily; }

    FontCharMapRef GetFontCharMap() const override;
    bool GetFontCapabilities(vcl::FontCapabilities&) const override;

private:
    sal_IntPtr              mnId;

    // some members that are initialized lazily when the font gets selected into a HDC
    mutable bool                    mbFontCapabilitiesRead;
    mutable FontCharMapRef          mxUnicodeMap;
    mutable vcl::FontCapabilities   maFontCapabilities;

    BYTE                    meWinCharSet;
    BYTE                    mnPitchAndFamily;
    bool                    mbAliasSymbolsHigh;
    bool                    mbAliasSymbolsLow;

    void                    ReadCmapTable( HDC ) const;
    void                    GetFontCapabilities( HDC hDC ) const;
};

/** Class that creates (and destroys) a compatible Device Context.

This is to be used for GDI drawing into a DIB that we later use for a different
drawing method, such as a texture for OpenGL drawing or surface for Skia drawing.
*/
class CompatibleDC
{
protected:
    /// The compatible DC that we create for our purposes.
    HDC mhCompatibleDC;

    /// DIBSection that we use for the GDI drawing, and later obtain.
    HBITMAP mhBitmap;

    /// Return the previous bitmap to undo the SelectObject.
    HBITMAP mhOrigBitmap;

    /// DIBSection data.
    sal_uInt32 *mpData;

    /// Mapping between the GDI position and OpenGL, to use for OpenGL drawing.
    SalTwoRect maRects;

    /// The SalGraphicsImpl where we will draw.  If null, we ignore the drawing, it means it happened directly to the DC...
    WinSalGraphicsImplBase *mpImpl;

    // If 'disable' is true, this class is a simple wrapper for drawing directly. Subclasses should use true.
    CompatibleDC(SalGraphics &rGraphics, int x, int y, int width, int height, bool disable=true);

public:
    static std::unique_ptr< CompatibleDC > create(SalGraphics &rGraphics, int x, int y, int width, int height);

    virtual ~CompatibleDC();

    HDC getCompatibleHDC() { return mhCompatibleDC; }

    SalTwoRect getTwoRect() const { return maRects; }

    tools::Long getBitmapWidth() const { return maRects.mnSrcWidth; }
    tools::Long getBitmapHeight() const { return maRects.mnSrcHeight; }

    /// Reset the DC with the defined color.
    void fill(sal_uInt32 color);
};

/**
 * WinSalGraphics never owns the HDC it uses to draw, because the HDC can have
 * various origins with different ways to correctly free it. And WinSalGraphics
 * stores all default values (mhDef*) of the HDC, which must be restored when
 * the HDC changes (setHDC) or the SalGraphics is destructed. So think of the
 * HDC in terms of Rust's Borrowing semantics.
 */
class WinSalGraphics : public SalGraphicsAutoDelegateToImpl
{
    friend class WinSalGraphicsImpl;
    friend class ScopedFont;

protected:
    std::unique_ptr<SalGraphicsImpl> mpImpl;

private:
    HDC                     mhLocalDC;              // HDC
    bool                    mbPrinter : 1;          // is Printer
    bool                    mbVirDev : 1;           // is VirDev
    bool                    mbWindow : 1;           // is Window
    bool                    mbScreen : 1;           // is Screen compatible
    HWND                    mhWnd;              // Window-Handle, when Window-Graphics

    rtl::Reference<WinFontInstance>
                            mpWinFontEntry[ MAX_FALLBACK ]; // pointer to the most recent font instance
    HRGN                    mhRegion;           // vcl::Region Handle
    HPEN                    mhDefPen;           // DefaultPen
    HBRUSH                  mhDefBrush;         // DefaultBrush
    HFONT                   mhDefFont;          // DefaultFont
    HPALETTE                mhDefPal;           // DefaultPalette
    COLORREF                mnTextColor;        // TextColor
    RGNDATA*                mpClipRgnData;      // ClipRegion-Data
    RGNDATA*                mpStdClipRgnData;   // Cache Standard-ClipRegion-Data
    int                     mnPenWidth;         // line width

    // just call both from setHDC!
    void InitGraphics();
    void DeInitGraphics();

public:
    // Return HFONT, and whether the font is for vertical writing ( prefixed with '@' )
    // and tmDescent value for adjusting offset in vertical writing mode.
    std::tuple<HFONT,bool,sal_Int32> ImplDoSetFont(HDC hDC, vcl::font::FontSelectPattern const & i_rFont, const vcl::font::PhysicalFontFace * i_pFontFace, HFONT& o_rOldFont);

    HDC getHDC() const { return mhLocalDC; }
    // NOTE: this doesn't transfer ownership! See class comment.
    void setHDC(HDC aNew);

    HPALETTE getDefPal() const;
    // returns the result from RealizePalette, otherwise 0 on success or GDI_ERROR
    UINT setPalette(HPALETTE, BOOL bForceBkgd = TRUE);

    HRGN getRegion() const;


    enum Type
    {
        PRINTER,
        VIRTUAL_DEVICE,
        WINDOW,
        SCREEN
    };

public:
    explicit WinSalGraphics(WinSalGraphics::Type eType, bool bScreen, HWND hWnd,
                            SalGeometryProvider *pProvider);
    virtual ~WinSalGraphics() override;

    SalGraphicsImpl* GetImpl() const override;
    bool isPrinter() const;
    bool isVirtualDevice() const;
    bool isWindow() const;
    bool isScreen() const;

    HWND gethWnd();
    void setHWND(HWND hWnd);
    void Flush();

    // native widget rendering methods that require mirroring
protected:
    virtual bool        isNativeControlSupported( ControlType nType, ControlPart nPart ) override;
    virtual bool        hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                              const Point& aPos, bool& rIsInside ) override;
    virtual bool        drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                           ControlState nState, const ImplControlValue& aValue,
                                           const OUString& aCaption, const Color& rBackgroundColor ) override;
    virtual bool        getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue, const OUString& aCaption,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion ) override;
private:
    // local helpers
    void DrawTextLayout(const GenericSalLayout&, HDC, bool bUseDWrite, bool bRenderingModeNatural);

public:
    // public SalGraphics methods, the interface to the independent vcl part

    // get device resolution
    virtual void            GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY ) override;
    // set the text color to a specific color
    virtual void            SetTextColor( Color nColor ) override;
    // set the font
    virtual void            SetFont( LogicalFontInstance*, int nFallbackLevel ) override;
    // get the current font's metrics
    virtual void            GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    // get the repertoire of the current font
    virtual FontCharMapRef  GetFontCharMap() const override;
    // get the layout capabilities of the current font
    virtual bool GetFontCapabilities(vcl::FontCapabilities &rGetFontCapabilities) const override;
    // graphics must fill supplied font list
    virtual void            GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    // graphics must drop any cached font info
    virtual void            ClearDevFontCache() override;
    virtual bool            AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;
    // CreateFontSubset: a method to get a subset of glyhps of a font
    // inside a new valid font file
    // returns TRUE if creation of subset was successful
    // parameters: rToFile: contains an osl file URL to write the subset to
    //             pFont: describes from which font to create a subset
    //             pGlyphIDs: the glyph ids to be extracted
    //             pEncoding: the character code corresponding to each glyph
    //             pWidths: the advance widths of the corresponding glyphs (in PS font units)
    //             nGlyphs: the number of glyphs
    //             rInfo: additional outgoing information
    // implementation note: encoding 0 with glyph id 0 should be added implicitly
    // as "undefined character"
    virtual bool            CreateFontSubset( const OUString& rToFile,
                                              const vcl::font::PhysicalFontFace*,
                                              const sal_GlyphId* pGlyphIDs,
                                              const sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo // out parameter
                                              ) override;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void* GetEmbedFontData(const vcl::font::PhysicalFontFace*, tools::Long* pDataLen) override;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;
    virtual void            GetGlyphWidths( const vcl::font::PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;

    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;

    virtual SystemGraphicsData GetGraphicsData() const override;

    bool commonDrawEPS(tools::Long nX, tools::Long nY,
                       tools::Long nWidth, tools::Long nHeight,
                       void* pPtr, sal_uInt32 nSize);

    /// Update settings based on the platform values
    static void updateSettingsNative( AllSettings& rSettings );

    /// convert incompatible SalBitmap instances
    static void convertToWinSalBitmap(SalBitmap& rSalBitmap, WinSalBitmap& rWinSalBitmap);
};

// Init/Deinit Graphics
void    ImplUpdateSysColorEntries();
int     ImplIsSysColorEntry( Color nColor );
void    ImplGetLogFontFromFontSelect( const vcl::font::FontSelectPattern&,
            const vcl::font::PhysicalFontFace*, LOGFONTW& );

#define MAX_64KSALPOINTS    ((((sal_uInt16)0xFFFF)-8)/sizeof(POINTS))

// called extremely often from just one spot => inline
inline bool WinFontFace::HasChar( sal_uInt32 cChar ) const
{
    if( mxUnicodeMap->HasChar( cChar ) )
        return true;
    // second chance to allow symbol aliasing
    if( mbAliasSymbolsLow && ((cChar-0xF000) <= 0xFF) )
        cChar -= 0xF000;
    else if( mbAliasSymbolsHigh && (cChar <= 0xFF) )
        cChar += 0xF000;
    else
        return false;
    return mxUnicodeMap->HasChar( cChar );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
