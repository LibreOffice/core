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
#include <config_features.h>

#include <cairo.h>

#include <vcl/dllapi.h>
#include <vcl/GeometryProvider.hxx>
#include <vcl/region.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/BitmapBuffer.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

#include <basegfx/utils/systemdependentdata.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <unordered_map>

//Using formats that match cairo's formats. For android we patch cairo,
//which is internal in that case, to swap the rgb components so that
//cairo then matches the OpenGL GL_RGBA format so we can use it there
//where we don't have GL_BGRA support.
// SVP_24BIT_FORMAT is used to store 24-bit images in 3-byte pixels to conserve memory.
#if defined(ANDROID) && !HAVE_FEATURE_ANDROID_LOK
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcRgba | ScanlineFormat::TopDown)
#define SVP_CAIRO_BLUE 1
#define SVP_CAIRO_GREEN 2
#define SVP_CAIRO_RED 0
#define SVP_CAIRO_ALPHA 3
#elif defined OSL_BIGENDIAN
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcArgb | ScanlineFormat::TopDown)
#define SVP_CAIRO_BLUE 3
#define SVP_CAIRO_GREEN 2
#define SVP_CAIRO_RED 1
#define SVP_CAIRO_ALPHA 0
#else
#define SVP_24BIT_FORMAT (ScanlineFormat::N24BitTcBgr | ScanlineFormat::TopDown)
#define SVP_CAIRO_FORMAT (ScanlineFormat::N32BitTcBgra | ScanlineFormat::TopDown)
#define SVP_CAIRO_BLUE 0
#define SVP_CAIRO_GREEN 1
#define SVP_CAIRO_RED 2
#define SVP_CAIRO_ALPHA 3
#endif

typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo_user_data_key cairo_user_data_key_t;

VCL_DLLPUBLIC void dl_cairo_surface_set_device_scale(cairo_surface_t* surface, double x_scale,
                                                     double y_scale);
VCL_DLLPUBLIC void dl_cairo_surface_get_device_scale(cairo_surface_t* surface, double* x_scale,
                                                     double* y_scale);

VCL_DLLPUBLIC basegfx::B2DRange getFillDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClipBox(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClippedFillDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClippedStrokeDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getStrokeDamage(cairo_t* cr);

class SystemDependentData_CairoPath : public basegfx::SystemDependentData
{
private:
    // the path data itself
    cairo_path_t* mpCairoPath;

    // all other values the path data  is based on and
    // need to be compared with to check for data validity
    bool mbNoJoin;
    bool mbAntiAlias;
    std::vector<double> maStroke;

public:
    SystemDependentData_CairoPath(basegfx::SystemDependentDataManager& rSystemDependentDataManager,
                                  size_t nSizeMeasure, cairo_t* cr, bool bNoJoin, bool bAntiAlias,
                                  const std::vector<double>* pStroke); // MM01
    virtual ~SystemDependentData_CairoPath() override;

    // read access
    cairo_path_t* getCairoPath() { return mpCairoPath; }
    bool getNoJoin() const { return mbNoJoin; }
    bool getAntiAlias() const { return mbAntiAlias; }
    const std::vector<double>& getStroke() const { return maStroke; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

VCL_DLLPUBLIC size_t AddPolygonToPath(cairo_t* cr, const basegfx::B2DPolygon& rPolygon,
                                      const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap,
                                      bool bPixelSnapHairline);

VCL_DLLPUBLIC basegfx::B2DPoint impPixelSnap(const basegfx::B2DPolygon& rPolygon,
                                             const basegfx::B2DHomMatrix& rObjectToDevice,
                                             basegfx::B2DHomMatrix& rObjectToDeviceInv,
                                             sal_uInt32 nIndex);

VCL_DLLPUBLIC void add_polygon_path(cairo_t* cr, const basegfx::B2DPolyPolygon& rPolyPolygon,
                                    const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap);

VCL_DLLPUBLIC cairo_format_t getCairoFormat(const BitmapBuffer& rBuffer);

VCL_DLLPUBLIC std::unique_ptr<BitmapBuffer>
FastConvert24BitRgbTo32BitCairo(const BitmapBuffer* pSrc);

VCL_DLLPUBLIC void Toggle1BitTransparency(const BitmapBuffer& rBuf);

enum class PaintMode
{
    Over,
    Xor
};

typedef void (*damageHandler)(void* handle, sal_Int32 nExtentsX, sal_Int32 nExtentsY,
                              sal_Int32 nExtentsWidth, sal_Int32 nExtentsHeight);

struct VCL_DLLPUBLIC DamageHandler
{
    void* handle;
    damageHandler damaged;
};

struct VCL_DLLPUBLIC CairoCommon : public vcl::SalGeometryProvider
{
    cairo_surface_t* m_pSurface;
    vcl::Region m_aClipRegion;
    Color m_aLineColor;
    Color m_aFillColor;
    PaintMode m_ePaintMode;

    CairoCommon()
        : m_pSurface(nullptr)
        , m_aLineColor(Color(0x00, 0x00, 0x00))
        , m_aFillColor(Color(0xFF, 0xFF, 0XFF))
        , m_ePaintMode(PaintMode::Over)
    {
    }

    static cairo_user_data_key_t* getDamageKey();

    cairo_surface_t* getSurface() const { return m_pSurface; }

    cairo_t* getCairoContext(bool bXorModeAllowed, bool bAntiAlias) const;
    void releaseCairoContext(cairo_t* cr, bool bXorModeAllowed,
                             const basegfx::B2DRange& rExtents) const;
    cairo_t* createTmpCompatibleCairoContext() const;

    void applyColor(cairo_t* cr, Color rColor, double fTransparency = 0.0);
    void clipRegion(cairo_t* cr);
    static void clipRegion(cairo_t* cr, const vcl::Region& rClipRegion);

    // need this static version of ::drawPolyLine for usage from
    // vcl/unx/generic/gdi/salgdi.cxx. It gets wrapped by
    // ::drawPolyLine with some added parameters (see there)
    static bool drawPolyLine(cairo_t* cr, basegfx::B2DRange* pExtents, const Color& rLineColor,
                             bool bAntiAlias, const basegfx::B2DHomMatrix& rObjectToDevice,
                             const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                             double fLineWidth, const std::vector<double>* pStroke,
                             basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                             double fMiterMinimumAngle, bool bPixelSnapHairline);

    void copyWithOperator(const SalTwoRect& rTR, cairo_surface_t* source, cairo_operator_t eOp,
                          bool bAntiAlias);

    void copySource(const SalTwoRect& rTR, cairo_surface_t* source, bool bAntiAlias);

    static basegfx::B2DRange renderSource(cairo_t* cr, const SalTwoRect& rTR,
                                          cairo_surface_t* source);

    void copyBitsCairo(const SalTwoRect& rTR, cairo_surface_t* pSourceSurface, bool bAntiAlias);

    void invert(const basegfx::B2DPolygon& rPoly, SalInvert nFlags, bool bAntiAlias);

    static cairo_surface_t* createCairoSurface(const BitmapBuffer* pBuffer);

    static sal_Int32 GetSgpMetricFromSurface(vcl::SGPmetric eMetric, cairo_surface_t& rSurface);
    virtual sal_Int32 GetSgpMetric(vcl::SGPmetric eMetric) const override;
};

class SurfaceHelper
{
private:
    cairo_surface_t* pSurface;
    std::unordered_map<sal_uInt64, cairo_surface_t*> maDownscaled;

    SurfaceHelper(const SurfaceHelper&) = delete;
    SurfaceHelper& operator=(const SurfaceHelper&) = delete;

    cairo_surface_t* implCreateOrReuseDownscale(unsigned long nTargetWidth,
                                                unsigned long nTargetHeight);

protected:
    cairo_surface_t* implGetSurface() const { return pSurface; }
    void implSetSurface(cairo_surface_t* pNew) { pSurface = pNew; }

    bool isTrivial() const;

public:
    explicit SurfaceHelper();
    ~SurfaceHelper();

    cairo_surface_t* getSurface(unsigned long nTargetWidth = 0,
                                unsigned long nTargetHeight = 0) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
