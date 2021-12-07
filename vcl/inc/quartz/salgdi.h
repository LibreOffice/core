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

#include <vector>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <tools/long.hxx>

#include <premac.h>
#ifdef MACOSX
#include <ApplicationServices/ApplicationServices.h>
#include <osx/osxvcltypes.h>
#include <osx/salframe.h>
#else
#include <CoreGraphics/CoreGraphics.h>
#include <CoreText/CoreText.h>
#include "salgeom.hxx"
#endif
#include <postmac.h>

#include <vcl/fontcapabilities.hxx>
#include <vcl/metric.hxx>


#include <fontinstance.hxx>
#include <impfontmetricdata.hxx>
#include <font/PhysicalFontFace.hxx>
#include <salgdi.hxx>

#include <quartz/salgdicommon.hxx>
#include <unordered_map>
#include <hb-ot.h>

#include <quartz/CGHelpers.hxx>

class AquaSalFrame;
class FontAttributes;
class XorEmulation;

// CoreText-specific physically available font face
class CoreTextFontFace : public vcl::font::PhysicalFontFace
{
public:
                                    CoreTextFontFace( const FontAttributes&, sal_IntPtr nFontID );
    virtual                         ~CoreTextFontFace() override;

    sal_IntPtr                      GetFontId() const override;

    int                             GetFontTable( uint32_t nTagCode, unsigned char* ) const;
    int                             GetFontTable( const char pTagName[5], unsigned char* ) const;

    FontCharMapRef GetFontCharMap() const override;
    bool GetFontCapabilities(vcl::FontCapabilities&) const override;
    bool                            HasChar( sal_uInt32 cChar ) const;

    rtl::Reference<LogicalFontInstance> CreateFontInstance(const vcl::font::FontSelectPattern&) const override;

private:
    const sal_IntPtr                mnFontId;
    mutable FontCharMapRef          mxCharMap;
    mutable vcl::FontCapabilities   maFontCapabilities;
    mutable bool                    mbFontCapabilitiesRead;
};

class CoreTextStyle final : public LogicalFontInstance
{
    friend rtl::Reference<LogicalFontInstance> CoreTextFontFace::CreateFontInstance(const vcl::font::FontSelectPattern&) const;

public:
    ~CoreTextStyle() override;

    void       GetFontMetric( ImplFontMetricDataRef const & );
    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override;

    CFMutableDictionaryRef  GetStyleDict( void ) const { return mpStyleDict; }

    /// <1.0: font is squeezed, >1.0 font is stretched, else 1.0
    float mfFontStretch;
    /// text rotation in radian
    float mfFontRotation;
    /// faux bold - true, if font doesn't have proper bold variants
    bool mbFauxBold;

private:
    explicit CoreTextStyle(const vcl::font::PhysicalFontFace&, const vcl::font::FontSelectPattern&);

    hb_font_t* ImplInitHbFont() override;
    bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override;

    /// CoreText text style object
    CFMutableDictionaryRef  mpStyleDict;
};

// TODO: move into cross-platform headers

class SystemFontList
{
public:
    SystemFontList( void );
    ~SystemFontList( void );

    bool        Init( void );
    void        AddFont( CoreTextFontFace* );

    void    AnnounceFonts( vcl::font::PhysicalFontCollection& ) const;
    CoreTextFontFace* GetFontDataFromId( sal_IntPtr nFontId ) const;

    CTFontCollectionRef fontCollection() { return mpCTFontCollection; }

private:
    CTFontCollectionRef mpCTFontCollection;
    CFArrayRef mpCTFontArray;

    std::unordered_map<sal_IntPtr, rtl::Reference<CoreTextFontFace>> maFontContainer;
};

namespace sal::aqua
{
float getWindowScaling();
}

struct AquaSharedAttributes
{
    /// path representing current clip region
    CGMutablePathRef mxClipPath;

    /// Drawing colors
    /// pen color RGBA
    RGBAColor maLineColor;

    /// brush color RGBA
    RGBAColor maFillColor;

    // Graphics types
#ifdef MACOSX
    AquaSalFrame* mpFrame;
    /// is this a window graphics
    bool mbWindow;
#else // IOS
    // mirror AquaSalVirtualDevice::mbForeignContext for SvpSalGraphics objects related to such
    bool mbForeignContext;
#endif
    /// is this a printer graphics
    bool mbPrinter;
    /// is this a virtual device graphics
    bool mbVirDev;

    CGLayerHolder maLayer; // Quartz graphics layer
    CGContextHolder maContextHolder;  // Quartz drawing context
    CGContextHolder maBGContextHolder;  // Quartz drawing context for CGLayer
    CGContextHolder maCSContextHolder;  // Quartz drawing context considering the color space
    int mnWidth;
    int mnHeight;
    int mnXorMode; // 0: off 1: on 2: invert only
    int mnBitmapDepth;  // zero unless bitmap

    Color maTextColor;
    /// allows text to be rendered without antialiasing
    bool mbNonAntialiasedText;

    std::unique_ptr<XorEmulation> mpXorEmulation;

    AquaSharedAttributes()
        : mxClipPath(nullptr)
        , maLineColor(COL_WHITE)
        , maFillColor(COL_BLACK)
#ifdef MACOSX
        , mpFrame(nullptr)
        , mbWindow(false)
#else
        , mbForeignContext(false)
#endif
        , mbPrinter(false)
        , mbVirDev(false)
        , mnWidth(0)
        , mnHeight(0)
        , mnXorMode(0)
        , mnBitmapDepth(0)
        , maTextColor( COL_BLACK )
        , mbNonAntialiasedText( false )
    {}

    void unsetClipPath()
    {
        if (mxClipPath)
        {
            CGPathRelease(mxClipPath);
            mxClipPath = nullptr;
        }
    }

    void unsetState()
    {
        unsetClipPath();
    }

    bool checkContext();
    void setState();

    bool isPenVisible() const
    {
        return maLineColor.IsVisible();
    }
    bool isBrushVisible() const
    {
        return maFillColor.IsVisible();
    }

    void refreshRect(float lX, float lY, float lWidth, float lHeight)
    {
#ifdef MACOSX
        if (!mbWindow) // view only on Window graphics
            return;

        if (mpFrame)
        {
            // update a little more around the designated rectangle
            // this helps with antialiased rendering
            // Rounding down x and width can accumulate a rounding error of up to 2
            // The decrementing of x, the rounding error and the antialiasing border
            // require that the width and the height need to be increased by four
            const tools::Rectangle aVclRect(
                    Point(tools::Long(lX - 1), tools::Long(lY - 1)),
                    Size(tools::Long(lWidth + 4), tools::Long(lHeight + 4)));

            mpFrame->maInvalidRect.Union(aVclRect);
        }
#else
        (void) lX;
        (void) lY;
        (void) lWidth;
        (void) lHeight;
        return;
#endif
    }

    // apply the XOR mask to the target context if active and dirty
    void applyXorContext()
    {
        if (!mpXorEmulation)
            return;
        if (mpXorEmulation->UpdateTarget())
        {
            refreshRect(0, 0, mnWidth, mnHeight); // TODO: refresh minimal changerect
        }
    }

    // differences between VCL, Quartz and kHiThemeOrientation coordinate systems
    // make some graphics seem to be vertically-mirrored from a VCL perspective
    bool isFlipped() const
    {
    #ifdef MACOSX
        return mbWindow;
    #else
        return false;
    #endif
    }
};

class AquaGraphicsBackendBase
{
public:
    AquaGraphicsBackendBase(AquaSharedAttributes& rShared)
        : mrShared( rShared )
    {}
    virtual ~AquaGraphicsBackendBase() = 0;
    AquaSharedAttributes& GetShared() { return mrShared; }
    SalGraphicsImpl* GetImpl()
    {
        if(mpImpl == nullptr)
            mpImpl = dynamic_cast<SalGraphicsImpl*>(this);
        return mpImpl;
    }
    virtual void UpdateGeometryProvider(SalGeometryProvider*) {};
    virtual bool drawNativeControl(ControlType nType,
                                   ControlPart nPart,
                                   const tools::Rectangle &rControlRegion,
                                   ControlState nState,
                                   const ImplControlValue &aValue) = 0;
    virtual void drawTextLayout(const GenericSalLayout& layout) = 0;
    virtual void Flush() {}
    virtual void Flush( const tools::Rectangle& ) {}
protected:
    static bool performDrawNativeControl(ControlType nType,
                                         ControlPart nPart,
                                         const tools::Rectangle &rControlRegion,
                                         ControlState nState,
                                         const ImplControlValue &aValue,
                                         CGContextRef context,
                                         AquaSalFrame* mpFrame);
    AquaSharedAttributes& mrShared;
private:
    SalGraphicsImpl* mpImpl = nullptr;
};

inline AquaGraphicsBackendBase::~AquaGraphicsBackendBase() {}

class AquaGraphicsBackend final : public SalGraphicsImpl, public AquaGraphicsBackendBase
{
private:
    void drawPixelImpl( tools::Long nX, tools::Long nY, const RGBAColor& rColor); // helper to draw single pixels

#ifdef MACOSX
    void refreshRect(const NSRect& rRect)
    {
        mrShared.refreshRect(rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height);
    }
#else
    void refreshRect(const CGRect& /*rRect*/)
    {}
#endif

    void pattern50Fill();

#ifdef MACOSX
    void copyScaledArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX, tools::Long nSrcY,
                        tools::Long nSrcWidth, tools::Long nSrcHeight, AquaSharedAttributes* pSrcShared);
#endif

public:
    AquaGraphicsBackend(AquaSharedAttributes & rShared);
    ~AquaGraphicsBackend() override;

    void Init() override;

    void freeResources() override;

    OUString getRenderBackendName() const override
    {
        return "aqua";
    }

    bool setClipRegion(vcl::Region const& rRegion) override;
    void ResetClipRegion() override;

    sal_uInt16 GetBitCount() const override;

    tools::Long GetGraphicsWidth() const override;

    void SetLineColor() override;
    void SetLineColor(Color nColor) override;
    void SetFillColor() override;
    void SetFillColor(Color nColor) override;
    void SetXORMode(bool bSet, bool bInvertOnly) override;
    void SetROPLineColor(SalROPColor nROPColor) override;
    void SetROPFillColor(SalROPColor nROPColor) override;

    void drawPixel(tools::Long nX, tools::Long nY) override;
    void drawPixel(tools::Long nX, tools::Long nY, Color nColor) override;

    void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2) override;
    void drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight) override;
    void drawPolyLine(sal_uInt32 nPoints, const Point* pPointArray) override;
    void drawPolygon(sal_uInt32 nPoints, const Point* pPointArray) override;
    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                         const Point** pPointArray) override;

    bool drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                         const basegfx::B2DPolyPolygon&, double fTransparency) override;

    bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice, const basegfx::B2DPolygon&,
                      double fTransparency, double fLineWidth, const std::vector<double>* pStroke,
                      basegfx::B2DLineJoin, css::drawing::LineCap, double fMiterMinimumAngle,
                      bool bPixelSnapHairline) override;

    bool drawPolyLineBezier(sal_uInt32 nPoints, const Point* pPointArray,
                            const PolyFlags* pFlagArray) override;

    bool drawPolygonBezier(sal_uInt32 nPoints, const Point* pPointArray,
                           const PolyFlags* pFlagArray) override;

    bool drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                               const Point* const* pPointArray,
                               const PolyFlags* const* pFlagArray) override;

    void copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX, tools::Long nSrcY,
                  tools::Long nSrcWidth, tools::Long nSrcHeight, bool bWindowInvalidate) override;

    void copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap) override;

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                    const SalBitmap& rMaskBitmap) override;

    void drawMask(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap,
                  Color nMaskColor) override;

    std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                         tools::Long nHeight) override;

    Color getPixel(tools::Long nX, tools::Long nY) override;

    void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                SalInvert nFlags) override;

    void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags) override;

    bool drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                 void* pPtr, sal_uInt32 nSize) override;

    bool blendBitmap(const SalTwoRect&, const SalBitmap& rBitmap) override;

    bool blendAlphaBitmap(const SalTwoRect&, const SalBitmap& rSrcBitmap,
                          const SalBitmap& rMaskBitmap, const SalBitmap& rAlphaBitmap) override;

    bool drawAlphaBitmap(const SalTwoRect&, const SalBitmap& rSourceBitmap,
                         const SalBitmap& rAlphaBitmap) override;

    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                               const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                               const SalBitmap* pAlphaBitmap, double fAlpha) override;

    bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                       sal_uInt8 nTransparency) override;

    bool drawGradient(const tools::PolyPolygon& rPolygon, const Gradient& rGradient) override;
    bool implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon,
                          SalGradient const& rGradient) override;

    virtual bool drawNativeControl(ControlType nType,
                                   ControlPart nPart,
                                   const tools::Rectangle &rControlRegion,
                                   ControlState nState,
                                   const ImplControlValue &aValue) override;

    virtual void drawTextLayout(const GenericSalLayout& layout) override;

    bool supportsOperation(OutDevSupportType eType) const override;
};

class AquaSalGraphics : public SalGraphicsAutoDelegateToImpl
{
    AquaSharedAttributes maShared;
    std::unique_ptr<AquaGraphicsBackendBase> mpBackend;

    /// device resolution of this graphics
    sal_Int32                               mnRealDPIX;
    sal_Int32                               mnRealDPIY;

    // Device Font settings
    rtl::Reference<CoreTextStyle>           mpTextStyle[MAX_FALLBACK];

public:
                            AquaSalGraphics();
    virtual                 ~AquaSalGraphics() override;

    void                    SetVirDevGraphics(SalVirtualDevice* pVirDev,CGLayerHolder const &rLayer, CGContextRef, int nBitDepth = 0);
#ifdef MACOSX
    void                    initResolution( NSWindow* );
    void                    copyResolution( AquaSalGraphics& );
    void                    updateResolution();

    void                    SetWindowGraphics( AquaSalFrame* pFrame );
    bool                    IsWindowGraphics() const { return maShared.mbWindow; }
    void                    SetPrinterGraphics(CGContextRef, sal_Int32 nRealDPIX, sal_Int32 nRealDPIY);
    AquaSalFrame*           getGraphicsFrame() const { return maShared.mpFrame; }
    void                    setGraphicsFrame( AquaSalFrame* pFrame ) { maShared.mpFrame = pFrame; }
#endif

#ifdef MACOSX
    void                    UpdateWindow( NSRect& ); // delivered in NSView coordinates
    void                    RefreshRect(const NSRect& rRect)
    {
        maShared.refreshRect(rRect.origin.x, rRect.origin.y, rRect.size.width, rRect.size.height);
    }
#else
    void                    RefreshRect( const CGRect& ) {}
#endif

    void                    Flush();
    void                    Flush( const tools::Rectangle& );

    void                    UnsetState();
    // InvalidateContext does an UnsetState and sets mrContext to 0
    void                    InvalidateContext();

    AquaGraphicsBackendBase* getAquaGraphicsBackend() const
    {
        return mpBackend.get();
    }

    virtual SalGraphicsImpl* GetImpl() const override;

#ifdef MACOSX

protected:

    // native widget rendering methods that require mirroring

    virtual bool            isNativeControlSupported( ControlType nType, ControlPart nPart ) override;

    virtual bool            hitTestNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                                  const Point& aPos, bool& rIsInside ) override;
    virtual bool            drawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                               ControlState nState, const ImplControlValue& aValue,
                                               const OUString& aCaption, const Color& rBackgroundColor ) override;
    virtual bool            getNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState,
                                                    const ImplControlValue& aValue, const OUString& aCaption,
                                                    tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion ) override;
#endif

public:
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
    virtual bool            GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
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
                                                  const vcl::font::PhysicalFontFace* pFont,
                                                  const sal_GlyphId* pGlyphIds,
                                                  const sal_uInt8* pEncoding,
                                                  sal_Int32* pWidths,
                                                  int nGlyphs,
                                                  FontSubsetInfo& rInfo // out parameter
                                                  ) override;

    // GetEmbedFontData: gets the font data for a font marked
    // embeddable by GetDevFontList or NULL in case of error
    // parameters: pFont: describes the font in question
    //             pDataLen: out parameter, contains the byte length of the returned buffer
    virtual const void*     GetEmbedFontData(const vcl::font::PhysicalFontFace*, tools::Long* pDataLen)
        override;
    // frees the font data again
    virtual void            FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;

    virtual void            GetGlyphWidths( const vcl::font::PhysicalFontFace*,
                                            bool bVertical,
                                            std::vector< sal_Int32 >& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc ) override;

    virtual std::unique_ptr<GenericSalLayout>
                            GetTextLayout(int nFallbackLevel) override;
    virtual void            DrawTextLayout( const GenericSalLayout& ) override;

    virtual SystemGraphicsData
                            GetGraphicsData() const override;

private:
    static bool             GetRawFontData( const vcl::font::PhysicalFontFace* pFontData,
                                std::vector<unsigned char>& rBuffer,
                                bool* pJustCFF );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
