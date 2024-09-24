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
#include <config_options.h>
#include <cairo.h>

#include <vcl/dllapi.h>
#include <vcl/region.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/BitmapBuffer.hxx>

#include <com/sun/star/drawing/LineCap.hpp>

#include <basegfx/utils/systemdependentdata.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <optional>
#include <unordered_map>

typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;
typedef struct _cairo_user_data_key cairo_user_data_key_t;

class Gradient;
class SalBitmap;
struct SalGradient;

VCL_DLLPUBLIC void dl_cairo_surface_set_device_scale(cairo_surface_t* surface, double x_scale,
                                                     double y_scale);
VCL_DLLPUBLIC void dl_cairo_surface_get_device_scale(cairo_surface_t* surface, double* x_scale,
                                                     double* y_scale);

VCL_DLLPUBLIC basegfx::B2DRange getFillDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClipBox(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClippedFillDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getClippedStrokeDamage(cairo_t* cr);
VCL_DLLPUBLIC basegfx::B2DRange getStrokeDamage(cairo_t* cr);

class SystemDependentData_CairoPath final : public basegfx::SystemDependentData
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
    SystemDependentData_CairoPath(size_t nSizeMeasure, cairo_t* cr, bool bNoJoin, bool bAntiAlias,
                                  const std::vector<double>* pStroke); // MM01
    virtual ~SystemDependentData_CairoPath() override;

    // read access
    cairo_path_t* getCairoPath() { return mpCairoPath; }
    bool getNoJoin() const { return mbNoJoin; }
    bool getAntiAlias() const { return mbAntiAlias; }
    const std::vector<double>& getStroke() const { return maStroke; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) PixelSnapper
{
public:
    basegfx::B2DPoint snap(const basegfx::B2DPolygon& rPolygon,
                           const basegfx::B2DHomMatrix& rObjectToDevice,
                           basegfx::B2DHomMatrix& rObjectToDeviceInv, sal_uInt32 nIndex);

private:
    basegfx::B2DPoint maPrevPoint, maCurrPoint, maNextPoint;
    basegfx::B2ITuple maPrevTuple, maCurrTuple, maNextTuple;
};

VCL_DLLPUBLIC void add_polygon_path(cairo_t* cr, const basegfx::B2DPolyPolygon& rPolyPolygon,
                                    const basegfx::B2DHomMatrix& rObjectToDevice, bool bPixelSnap);

VCL_DLLPUBLIC cairo_format_t getCairoFormat(const BitmapBuffer& rBuffer);

VCL_DLLPUBLIC std::optional<BitmapBuffer> FastConvert24BitRgbTo32BitCairo(const BitmapBuffer* pSrc);

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

struct VCL_DLLPUBLIC CairoCommon
{
    cairo_surface_t* m_pSurface;
    basegfx::B2IVector m_aFrameSize;
    vcl::Region m_aClipRegion;
    std::optional<Color> m_oLineColor;
    std::optional<Color> m_oFillColor;
    PaintMode m_ePaintMode;
    double m_fScale;

    CairoCommon()
        : m_pSurface(nullptr)
        , m_oLineColor(Color(0x00, 0x00, 0x00))
        , m_oFillColor(Color(0xFF, 0xFF, 0XFF))
        , m_ePaintMode(PaintMode::Over)
        , m_fScale(1.0)
    {
    }

    static cairo_user_data_key_t* getDamageKey();

    cairo_surface_t* getSurface() const { return m_pSurface; }

    sal_uInt16 GetBitCount() const;

    cairo_t* getCairoContext(bool bXorModeAllowed, bool bAntiAlias) const;
    void releaseCairoContext(cairo_t* cr, bool bXorModeAllowed,
                             const basegfx::B2DRange& rExtents) const;
    void applyFullDamage() const;

    cairo_t* createTmpCompatibleCairoContext() const;

    static void applyColor(cairo_t* cr, Color rColor, double fTransparency = 0.0);
    static void applyColor2(cairo_t* cr, Color rColor);
    void clipRegion(cairo_t* cr);
    static void clipRegion(cairo_t* cr, const vcl::Region& rClipRegion);

    void SetXORMode(bool bSet, bool bInvertOnly);
    void SetROPLineColor(SalROPColor nROPColor);
    void SetROPFillColor(SalROPColor nROPColor);

    void drawPixel(const std::optional<Color>& rLineColor, tools::Long nX, tools::Long nY,
                   bool bAntiAlias);

    static Color getPixel(cairo_surface_t* pSurface, tools::Long nX, tools::Long nY);

    void drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2,
                  bool bAntiAlias);

    void drawRect(double nX, double nY, double nWidth, double nHeight, bool bAntiAlias);

    void drawPolygon(sal_uInt32 nPoints, const Point* pPtAry, bool bAntiAlias);

    void drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry,
                         bool bAntiAlias);

    void drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                         const basegfx::B2DPolyPolygon&, double fTransparency, bool bAntiAlias);

    void drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry, bool bAntiAlias);

    bool drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                      const basegfx::B2DPolygon& rPolyLine, double fTransparency, double fLineWidth,
                      const std::vector<double>* pStroke, basegfx::B2DLineJoin eLineJoin,
                      css::drawing::LineCap eLineCap, double fMiterMinimumAngle,
                      bool bPixelSnapHairline, bool bAntiAlias);

    bool drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                       sal_uInt8 nTransparency, bool bAntiAlias);

    bool drawGradient(const tools::PolyPolygon& rPolyPolygon, const Gradient& rGradient,
                      bool bAntiAlias);

    bool implDrawGradient(basegfx::B2DPolyPolygon const& rPolyPolygon, SalGradient const& rGradient,
                          bool bAntiAlias);

    void copyWithOperator(const SalTwoRect& rTR, cairo_surface_t* source, cairo_operator_t eOp,
                          bool bAntiAlias);

    void copySource(const SalTwoRect& rTR, cairo_surface_t* source, bool bAntiAlias);

    static basegfx::B2DRange renderSource(cairo_t* cr, const SalTwoRect& rTR,
                                          cairo_surface_t* source);

    void copyBitsCairo(const SalTwoRect& rTR, cairo_surface_t* pSourceSurface, bool bAntiAlias);

    void invert(const basegfx::B2DPolygon& rPoly, SalInvert nFlags, bool bAntiAlias);

    void invert(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                SalInvert nFlags, bool bAntiAlias);

    void invert(sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags, bool bAntiAlias);

    void drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap, bool bAntiAlias);

    bool drawAlphaBitmap(const SalTwoRect& rTR, const SalBitmap& rSourceBitmap,
                         const SalBitmap& rAlphaBitmap, bool bAntiAlias);

    bool drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                               const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                               const SalBitmap* pAlphaBitmap, double fAlpha, bool bAntiAlias);

    void drawMask(const SalTwoRect& rTR, const SalBitmap& rSalBitmap, Color nMaskColor,
                  bool bAntiAlias);

    std::shared_ptr<SalBitmap> getBitmap(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                         tools::Long nHeight, bool bWithoutAlpha);

    static cairo_surface_t* createCairoSurface(const BitmapBuffer* pBuffer);

    static bool supportsOperation(OutDevSupportType eType);

private:
    void doXorOnRelease(sal_Int32 nExtentsLeft, sal_Int32 nExtentsTop, sal_Int32 nExtentsRight,
                        sal_Int32 nExtentsBottom, cairo_surface_t* const surface,
                        sal_Int32 nWidth) const;
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) SurfaceHelper
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
