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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>

#include <vcl/salgtype.hxx>
#include <vcl/vclenum.hxx>

#include <unx/saldisp.hxx>
#include <salgdi.hxx>
#include <salgeom.hxx>
#include <sallayout.hxx>
#include <vclpluginapi.h>
#include <ControlCacheKey.hxx>

#include "saltype.h"
#include "saldisp.hxx"

#include <memory>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

class SalBitmap;
class SalColormap;
class SalDisplay;
class SalFrame;
class X11SalVirtualDevice;
class X11SalGraphicsImpl;
class X11SkiaSalVirtualDevice;
namespace vcl::font
{
class PhysicalFontCollection;
class PhysicalFontFace;
}
class SalGraphicsImpl;
class TextRenderImpl;

namespace basegfx {
    class B2DTrapezoid;
}

class X11Common
{
public:
    Drawable m_hDrawable;
    const SalColormap* m_pColormap;
    cairo_surface_t* m_pExternalSurface;

    X11Common();

    cairo_t* getCairoContext();

    static void releaseCairoContext(cairo_t* cr);

    bool SupportsCairo() const;

    const SalColormap& GetColormap() const { return *m_pColormap; }
    const SalDisplay* GetDisplay() const { return GetColormap().GetDisplay(); }
    const SalVisual& GetVisual() const { return GetColormap().GetVisual(); }
    Display* GetXDisplay() const { return GetColormap().GetXDisplay(); }
    Pixel GetPixel(Color nColor) const { return GetColormap().GetPixel(nColor); }
    Drawable GetDrawable() const { return m_hDrawable; }
};

class X11SalGraphics final : public SalGraphicsAutoDelegateToImpl
{
    friend class X11SalGraphicsImpl;
    friend class X11CairoTextRender;

public:
                                    X11SalGraphics();
    virtual                         ~X11SalGraphics() COVERITY_NOEXCEPT_FALSE override;

    void                            Init( SalFrame *pFrame, Drawable aDrawable, SalX11Screen nXScreen );
    void                            Init( X11SalVirtualDevice *pVirtualDevice, cairo_surface_t* pPreExistingTarget = nullptr,
                                          SalColormap* pColormap = nullptr, bool bDeleteColormap = false );
    void                            Init( X11SkiaSalVirtualDevice *pVirtualDevice );
    void                            DeInit();

    virtual SalGraphicsImpl*        GetImpl() const override;
    SalGeometryProvider*            GetGeometryProvider() const;
    void                            SetDrawable(Drawable d, cairo_surface_t* surface, SalX11Screen nXScreen);
    XRenderPictFormat*              GetXRenderFormat() const;
    void                    SetXRenderFormat( XRenderPictFormat* pXRenderFormat ) { m_pXRenderFormat = pXRenderFormat; }

    const SalX11Screen&             GetScreenNumber() const { return m_nXScreen; }

    void                            Flush();

    sal_Int32 GetDPI() const;
    virtual sal_Int32 GetSgpMetric(vcl::SGPmetric eMetric) const override;

    virtual void                    SetTextColor( Color nColor ) override;
    virtual void                    SetFont(LogicalFontInstance*, int nFallbackLevel) override;
    virtual void                    GetFontMetric( ImplFontMetricDataRef&, int nFallbackLevel ) override;
    virtual FontCharMapRef          GetFontCharMap() const override;
    virtual bool                    GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const override;
    virtual void                    GetDevFontList( vcl::font::PhysicalFontCollection* ) override;
    virtual void                    ClearDevFontCache() override;
    virtual bool                    AddTempDevFont( vcl::font::PhysicalFontCollection*, const OUString& rFileURL, const OUString& rFontName ) override;

    virtual bool                    CreateFontSubset(
                                        const OUString& rToFile,
                                        const vcl::font::PhysicalFontFace*,
                                        const sal_GlyphId* pGlyphIDs,
                                        const sal_uInt8* pEncoding,
                                        sal_Int32* pWidths,
                                        int nGlyphs,
                                        FontSubsetInfo& rInfo ) override;

    virtual const void*             GetEmbedFontData(const vcl::font::PhysicalFontFace*, tools::Long* pDataLen) override;
    virtual void                    FreeEmbedFontData( const void* pData, tools::Long nDataLen ) override;

    virtual void                    GetGlyphWidths(
                                        const vcl::font::PhysicalFontFace*,
                                        bool bVertical,
                                        std::vector< sal_Int32 >& rWidths,
                                        Ucs2UIntMap& rUnicodeEnc ) override;

    virtual std::unique_ptr<GenericSalLayout>
                                    GetTextLayout(int nFallbackLevel) override;
    virtual void                    DrawTextLayout( const GenericSalLayout& ) override;

    virtual SystemGraphicsData      GetGraphicsData() const override;

#if ENABLE_CAIRO_CANVAS
    virtual bool                    SupportsCairo() const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const override;
    virtual cairo::SurfaceSharedPtr CreateSurface(const OutputDevice& rRefDevice, int x, int y, int width, int height) const override;
    virtual cairo::SurfaceSharedPtr CreateBitmapSurface(const OutputDevice& rRefDevice, const BitmapSystemData& rData, const Size& rSize) const override;
    virtual css::uno::Any           GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& rSurface, const basegfx::B2ISize& rSize) const override;

    void clipRegion(cairo_t* cr);
#endif // ENABLE_CAIRO_CANVAS

    /*  use to handle GraphicsExpose/NoExpose after XCopyArea & friends
     *  if pFrame is not NULL, corresponding Paint events are generated
     *  and dispatched to pFrame
     *
     *  it is imperative to eat up graphics exposes even in case you don't need
     *  them because the next one using XCopyArea can depend on them
     */
    void                            YieldGraphicsExpose();

    cairo_t* getCairoContext();
    static void releaseCairoContext(cairo_t* cr);


private:
    using SalGraphics::GetPixel;
    void                            SetClipRegion( GC pGC, Region pXReg = nullptr ) const;
    bool                            GetDitherPixmap ( Color nColor );

    void                            freeResources();

    SalFrame*                       m_pFrame; // the SalFrame which created this Graphics or NULL
    SalVirtualDevice*               m_pVDev;  // the SalVirtualDevice which created this Graphics or NULL


    std::unique_ptr<SalColormap>    m_pDeleteColormap;

    SalX11Screen                    m_nXScreen;
    mutable XRenderPictFormat*      m_pXRenderFormat;
    XID                             m_aXRenderPicture;

    Region                          mpClipRegion;
    Pixmap                          hBrush_;        // Dither

    bool                            bWindow_ : 1;       // is Window or compatible virtual device

    std::unique_ptr<SalGraphicsImpl> mxImpl;
    std::unique_ptr<TextRenderImpl> mxTextRenderImpl;
    X11Common maX11Common;

public:
    Drawable GetDrawable() const { return maX11Common.GetDrawable(); }
    const SalColormap& GetColormap() const { return maX11Common.GetColormap(); }
    const SalDisplay* GetDisplay() const { return maX11Common.GetDisplay(); }
    const SalVisual& GetVisual() const { return maX11Common.GetVisual(); }
    Display* GetXDisplay() const { return maX11Common.GetXDisplay(); }
    Pixel GetPixel(Color nColor) const { return maX11Common.GetPixel(nColor); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
