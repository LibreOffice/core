/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <systools/win32/comtools.hxx>

#include <vcl/dllapi.h>
#include <skia/gdiimpl.hxx>
#include <skia/utils.hxx>
#include <win/salgdi.h>
#include <win/wingdiimpl.hxx>
#include <o3tl/lru_map.hxx>
#include <ControlCacheKey.hxx>
#include <svdata.hxx>

#include <SkFont.h>
#include <SkFontMgr.h>

#include <dwrite_3.h>

class SkTypeface;
class ControlCacheKey;

/** Class that creates (and destroys) a compatible Device Context.

This is to be used for GDI drawing into a DIB that we later use for a different
drawing method, such as a texture for OpenGL drawing or surface for Skia drawing.
*/
class SkiaCompatibleDC
{
    /// The compatible DC that we create for our purposes.
    HDC mhCompatibleDC;

    /// Mapping between the GDI position and OpenGL, to use for OpenGL drawing.
    SalTwoRect maRects;

    /// DIBSection that we use for the GDI drawing, and later obtain.
    HBITMAP mhBitmap;

    /// Return the previous bitmap to undo the SelectObject.
    HBITMAP mhOrigBitmap;

    /// DIBSection data.
    sal_uInt32* mpData;

    /// The SalGraphicsImpl where we will draw.  If null, we ignore the drawing, it means it happened directly to the DC...
    WinSalGraphicsImplBase* mpImpl;

public:
    SkiaCompatibleDC(WinSalGraphics& rGraphics, int x, int y, int width, int height);
    ~SkiaCompatibleDC();

    HDC getCompatibleHDC() const { return mhCompatibleDC; }

    /// Reset the DC with the defined color.
    void fill(sal_uInt32 color);

    sk_sp<SkImage> getAsImageDiff(const SkiaCompatibleDC& white) const;
};

class WinSkiaSalGraphicsImpl : public SkiaSalGraphicsImpl, public WinSalGraphicsImplBase
{
private:
    WinSalGraphics& mWinParent;

public:
    WinSkiaSalGraphicsImpl(WinSalGraphics& rGraphics, SalGeometryProvider* mpProvider);

    virtual bool UseRenderNativeControl() const override { return true; }
    virtual bool TryRenderCachedNativeControl(ControlCacheKey const& rControlCacheKey, int nX,
                                              int nY) override;
    virtual bool RenderAndCacheNativeControl(SkiaCompatibleDC& rWhite, SkiaCompatibleDC& rBlack,
                                             int nX, int nY,
                                             ControlCacheKey& aControlCacheKey) override;

    virtual bool DrawTextLayout(const GenericSalLayout& layout) override;
    virtual void ClearDevFontCache() override;
    virtual void ClearNativeControlCache() override;

    virtual void Flush() override;

    static void prepareSkia();

protected:
    virtual void createWindowSurfaceInternal(bool forceRaster = false) override;
    static sk_sp<SkTypeface> createDirectWriteTypeface(const WinFontInstance* pWinFont);
    static void initFontInfo();
    inline static sal::systools::COMReference<IDWriteFontCollection> dwritePrivateCollection;
    inline static sk_sp<SkFontMgr> dwriteFontMgr;
    inline static bool dwriteDone = false;
    static SkFont::Edging fontEdging;
};

typedef std::pair<ControlCacheKey, sk_sp<SkImage>> SkiaControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, sk_sp<SkImage>, ControlCacheHashFunction>
    SkiaControlCacheType;

class SkiaControlsCache
{
    SkiaControlCacheType cache;

    SkiaControlsCache();

public:
    static SkiaControlCacheType& get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
