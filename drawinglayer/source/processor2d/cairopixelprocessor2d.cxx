/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <drawinglayer/processor2d/cairopixelprocessor2d.hxx>
#include <drawinglayer/processor2d/SDPRProcessor2dTools.hxx>
#include <sal/log.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/cairo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/markerarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/pointarrayprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/Tools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonGradientPrimitive2D.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <basegfx/utils/bgradient.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <unordered_map>
#include <dlfcn.h>

using namespace com::sun::star;

namespace
{
void impl_cairo_set_hairline(cairo_t* pRT,
                             const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 18, 0)
    void* addr(dlsym(nullptr, "cairo_set_hairline"));
    if (nullptr != addr)
    {
        cairo_set_hairline(pRT, true);
        return;
    }
#endif
    // avoid cairo_device_to_user_distance, see note on that below
    const double fPx(
        (rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0))
            .getLength());
    cairo_set_line_width(pRT, fPx);
}

void addB2DPolygonToPathGeometry(cairo_t* pRT, const basegfx::B2DPolygon& rPolygon)
{
    // short circuit if there is nothing to do
    const sal_uInt32 nPointCount(rPolygon.count());

    const bool bHasCurves(rPolygon.areControlPointsUsed());
    const bool bClosePath(rPolygon.isClosed());
    const basegfx::B2DPoint* pLast(nullptr);

    for (sal_uInt32 nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++)
    {
        int nClosedIdx = nPointIdx;
        if (nPointIdx >= nPointCount)
        {
            // prepare to close last curve segment if needed
            if (bClosePath && (nPointIdx == nPointCount))
            {
                nClosedIdx = 0;
            }
            else
            {
                break;
            }
        }

        const basegfx::B2DPoint& rPoint(rPolygon.getB2DPoint(nClosedIdx));

        if (!nPointIdx)
        {
            // first point => just move there
            cairo_move_to(pRT, rPoint.getX(), rPoint.getY());
            pLast = &rPoint;
            continue;
        }

        bool bPendingCurve(false);

        if (bHasCurves)
        {
            bPendingCurve = rPolygon.isNextControlPointUsed(nPrevIdx);
            bPendingCurve |= rPolygon.isPrevControlPointUsed(nClosedIdx);
        }

        if (!bPendingCurve) // line segment
        {
            cairo_line_to(pRT, rPoint.getX(), rPoint.getY());
        }
        else // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint(nPrevIdx);
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint(nClosedIdx);

            // tdf#99165 if the control points are 'empty', create the mathematical
            // correct replacement ones to avoid problems with the graphical sub-system
            // tdf#101026 The 1st attempt to create a mathematically correct replacement control
            // vector was wrong. Best alternative is one as close as possible which means short.
            if (aCP1.equal(*pLast))
            {
                aCP1 = *pLast + ((aCP2 - *pLast) * 0.0005);
            }

            if (aCP2.equal(rPoint))
            {
                aCP2 = rPoint + ((aCP1 - rPoint) * 0.0005);
            }

            cairo_curve_to(pRT, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(), rPoint.getX(),
                           rPoint.getY());
        }

        pLast = &rPoint;
    }

    if (bClosePath)
    {
        cairo_close_path(pRT);
    }
}

// needed as helper, see below. It guarantees clean
// construction/cleanup using destructor
// NOTE: maybe mpSurface can be constructed even simpler,
// not sure about that. It is only used to construct
// and hold path data
struct CairoContextHolder
{
    cairo_surface_t* mpSurface;
    cairo_t* mpRenderContext;

    CairoContextHolder()
        : mpSurface(cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1))
        , mpRenderContext(cairo_create(mpSurface))
    {
    }

    ~CairoContextHolder()
    {
        cairo_destroy(mpRenderContext);
        cairo_surface_destroy(mpSurface);
    }

    cairo_t* getContext() const { return mpRenderContext; }
};

// global static helper instance
CairoContextHolder globalStaticCairoContext;

// it shows that re-using and buffering path geometry data using
// cairo is more complicated than initially thought: when adding
// a path to a cairo_t render context it already *uses* the set
// transformation, also usually consumes the path when painting.
// The (only available) method cairo_copy_path to preserve that
// data *also* transforms the path - if not already created in
// transformed form - using the current transformation set at the
// cairo context.
// This is not what we want to have a re-usable path that is
// buffered at the Poly(poly)gon: we explicitly want *exactly*
// the coordinates in the polygon preserved *at* the polygon to
// be able to re-use that data independent from any set
// transformation at any cairo context.
// Thus, create paths using a helper (CairoPathHelper) using a
// helper cairo context (CairoContextHolder) that never gets
// transformed. This removes the need to feed it the cairo context,
// but also does not immediately add the path data to the target
// context, that needs to be done using cairo_append_path at the
// target cairo context. That works since all geometry is designed
// to use exactly that coordinate system the polygon is already
// designed for anyways, and it transforms as needed inside the
// target cairo context as needed (if transform is set)
class CairoPathHelper
{
    // the created CairoPath
    cairo_path_t* mpCairoPath;

public:
    CairoPathHelper(const basegfx::B2DPolygon& rPolygon)
        : mpCairoPath(nullptr)
    {
        cairo_new_path(globalStaticCairoContext.getContext());
        addB2DPolygonToPathGeometry(globalStaticCairoContext.getContext(), rPolygon);
        mpCairoPath = cairo_copy_path(globalStaticCairoContext.getContext());
        cairo_new_path(globalStaticCairoContext.getContext());
    }

    CairoPathHelper(const basegfx::B2DPolyPolygon& rPolyPolygon)
        : mpCairoPath(nullptr)
    {
        cairo_new_path(globalStaticCairoContext.getContext());
        for (const auto& rPolygon : rPolyPolygon)
            addB2DPolygonToPathGeometry(globalStaticCairoContext.getContext(), rPolygon);
        mpCairoPath = cairo_copy_path(globalStaticCairoContext.getContext());
        cairo_new_path(globalStaticCairoContext.getContext());
    }

    ~CairoPathHelper()
    {
        // need to cleanup instance
        cairo_path_destroy(mpCairoPath);
    }

    // read access
    cairo_path_t* getCairoPath() const { return mpCairoPath; }

    sal_Int64 getEstimatedSize() const
    {
        if (nullptr == mpCairoPath)
            return 0;

        // per node:
        // - num_data incarnations of
        // - sizeof(cairo_path_data_t) which is a union of defines and point data
        //   thus may 2 x sizeof(double)
        return mpCairoPath->num_data * sizeof(cairo_path_data_t);
    }
};

class SystemDependentData_CairoPathGeometry : public basegfx::SystemDependentData
{
    // the CairoPath holder
    std::shared_ptr<CairoPathHelper> mpCairoPathHelper;

public:
    SystemDependentData_CairoPathGeometry(const std::shared_ptr<CairoPathHelper>& pCairoPathHelper)
        : basegfx::SystemDependentData(Application::GetSystemDependentDataManager(),
                                       basegfx::SDD_Type::SDDType_CairoPathGeometry)
        , mpCairoPathHelper(pCairoPathHelper)
    {
    }

    // read access
    const std::shared_ptr<CairoPathHelper>& getCairoPathHelper() const { return mpCairoPathHelper; }

    virtual sal_Int64 estimateUsageInBytes() const override
    {
        return (nullptr != mpCairoPathHelper) ? mpCairoPathHelper->getEstimatedSize() : 0;
    }
};

constexpr unsigned long nMinimalPointsPath(4);
constexpr unsigned long nMinimalPointsFill(12);

void checkAndDoPixelSnap(cairo_t* pRT,
                         const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    const bool bPixelSnap(rViewInformation.getPixelSnapHairline()
                          && rViewInformation.getUseAntiAliasing());

    if (!bPixelSnap)
    {
        // no pixel snap, done
        return;
    }

    // with the comments above at CairoPathHelper we cannot do PixelSnap
    // at path construction time, so it needs to be done *after* the path
    // data is added to the cairo context. ADvantage is that all general
    // path data can be buffered, though, but needs view-dependent manipulation
    // here after being added.
    // For now, just snap all points - no real need to identify hor/ver lines
    // when you think about it

    // get helper path
    cairo_path_t* path(cairo_copy_path(pRT));

    if (0 == path->num_data)
    {
        // path is empty, done
        cairo_path_destroy(path);
        return;
    }

    for (int a(0); a < path->num_data; a += path->data[a].header.length)
    {
        cairo_path_data_t* data(&path->data[a]);

        switch (data->header.type)
        {
            case CAIRO_PATH_MOVE_TO:
            case CAIRO_PATH_LINE_TO:
            case CAIRO_PATH_CURVE_TO:
            {
                // NOTE: for CAIRO_PATH_CURVE_TO we would also have the control
                // points, but these do not really need correction. If that may
                // change a correction using the deltas in x and y could be added

                // get pointers to double data
                double* pX(&data[1].point.x);
                double* pY(&data[1].point.y);

                // transform to discrete pixels
                cairo_user_to_device(pRT, pX, pY);

                // round them, also subtract 0.5 which will be as transform in
                // the paint method to move to 'inside' pixels when AA used.
                // remember: this is only done when AA is active (see bPixelSnap
                // above)
                *pX = basegfx::fround(*pX) - 0.5;
                *pY = basegfx::fround(*pY) - 0.5;

                // transform back to former transformed state
                cairo_device_to_user(pRT, pX, pY);
                break;
            }
            case CAIRO_PATH_CLOSE_PATH:
            {
                break;
            }
        }
    }

    // set changed path back at cairo context
    cairo_new_path(pRT);
    cairo_append_path(pRT, path);

    // destroy helper path
    cairo_path_destroy(path);
}

void getOrCreatePathGeometry(cairo_t* pRT, const basegfx::B2DPolygon& rPolygon,
                             const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    // try to access buffered data
    std::shared_ptr<SystemDependentData_CairoPathGeometry> pSystemDependentData_CairoPathGeometry(
        rPolygon.getSystemDependentData<SystemDependentData_CairoPathGeometry>(
            basegfx::SDD_Type::SDDType_CairoPathGeometry));

    if (pSystemDependentData_CairoPathGeometry)
    {
        // re-use data and do evtl. needed pixel snap after adding on cairo path data
        cairo_append_path(
            pRT, pSystemDependentData_CairoPathGeometry->getCairoPathHelper()->getCairoPath());
        checkAndDoPixelSnap(pRT, rViewInformation);
        return;
    }

    // create new data and add path data to pRT and do evtl. needed pixel snap after adding on cairo path data
    std::shared_ptr<CairoPathHelper> pCairoPathHelper(std::make_shared<CairoPathHelper>(rPolygon));
    cairo_append_path(pRT, pCairoPathHelper->getCairoPath());
    checkAndDoPixelSnap(pRT, rViewInformation);

    // add to buffering mechanism if not trivial
    if (rPolygon.count() > nMinimalPointsPath)
        rPolygon.addOrReplaceSystemDependentData<SystemDependentData_CairoPathGeometry>(
            pCairoPathHelper);
}

void getOrCreateFillGeometry(cairo_t* pRT, const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // try to access buffered data
    std::shared_ptr<SystemDependentData_CairoPathGeometry> pSystemDependentData_CairoPathGeometry(
        rPolyPolygon.getSystemDependentData<SystemDependentData_CairoPathGeometry>(
            basegfx::SDD_Type::SDDType_CairoPathGeometry));

    if (pSystemDependentData_CairoPathGeometry)
    {
        // re-use data
        cairo_append_path(
            pRT, pSystemDependentData_CairoPathGeometry->getCairoPathHelper()->getCairoPath());
        return;
    }

    // create new data and add path data to pRT
    std::shared_ptr<CairoPathHelper> pCairoPathHelper(
        std::make_shared<CairoPathHelper>(rPolyPolygon));
    cairo_append_path(pRT, pCairoPathHelper->getCairoPath());

    // get all PointCount to detect non-trivial
    sal_uInt32 nAllPointCount(0);
    for (const auto& rPolygon : rPolyPolygon)
        nAllPointCount += rPolygon.count();

    // add to buffering mechanism when no PixelSnapHairline (see above) and not trivial
    if (nAllPointCount > nMinimalPointsFill)
        rPolyPolygon.addOrReplaceSystemDependentData<SystemDependentData_CairoPathGeometry>(
            pCairoPathHelper);
}

// check for env var that decides for using downscale pattern
const char* pDisableDownScale(getenv("SAL_DISABLE_CAIRO_DOWNSCALE"));
const bool bDisableDownScale(nullptr != pDisableDownScale);
constexpr unsigned long nMinimalDiscreteSize(15);
constexpr unsigned long nHalfMDSize((nMinimalDiscreteSize + 1) / 2);
constexpr unsigned long
nMinimalDiscreteSquareSizeToBuffer(nMinimalDiscreteSize* nMinimalDiscreteSize);

class CairoSurfaceHelper
{
    // the buffered CairoSurface (bitmap data)
    cairo_surface_t* mpCairoSurface;

    // evtl. MipMapped data (pre-scale to reduce data processing load)
    mutable std::unordered_map<sal_uInt64, cairo_surface_t*> maDownscaled;

    // create 32bit RGBA data for given BitmapEx
    void createRGBA(const BitmapEx& rBitmapEx)
    {
        Bitmap aSrcAlpha(rBitmapEx.GetAlphaMask().GetBitmap());
        BitmapScopedReadAccess pReadAccess(rBitmapEx.GetBitmap());
        BitmapScopedReadAccess pAlphaReadAccess(aSrcAlpha);
        const tools::Long nHeight(pReadAccess->Height());
        const tools::Long nWidth(pReadAccess->Width());
        mpCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
        const sal_uInt32 nStride(cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, nWidth));
        unsigned char* surfaceData(cairo_image_surface_get_data(mpCairoSurface));

        for (tools::Long y(0); y < nHeight; ++y)
        {
            unsigned char* pPixelData(surfaceData + (nStride * y));

            for (tools::Long x(0); x < nWidth; ++x)
            {
                const BitmapColor aColor(pReadAccess->GetColor(y, x));
                const BitmapColor aAlpha(pAlphaReadAccess->GetColor(y, x));
                const sal_uInt16 nAlpha(aAlpha.GetRed());

                pPixelData[SVP_CAIRO_RED] = vcl::bitmap::premultiply(nAlpha, aColor.GetRed());
                pPixelData[SVP_CAIRO_GREEN] = vcl::bitmap::premultiply(nAlpha, aColor.GetGreen());
                pPixelData[SVP_CAIRO_BLUE] = vcl::bitmap::premultiply(nAlpha, aColor.GetBlue());
                pPixelData[SVP_CAIRO_ALPHA] = nAlpha;
                pPixelData += 4;
            }
        }

        cairo_surface_mark_dirty(mpCairoSurface);
    }

    // create 32bit RGB data for given BitmapEx
    void createRGB(const BitmapEx& rBitmapEx)
    {
        BitmapScopedReadAccess pReadAccess(rBitmapEx.GetBitmap());
        const tools::Long nHeight(pReadAccess->Height());
        const tools::Long nWidth(pReadAccess->Width());
        mpCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, nWidth, nHeight);
        sal_uInt32 nStride(cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, nWidth));
        unsigned char* surfaceData(cairo_image_surface_get_data(mpCairoSurface));

        for (tools::Long y(0); y < nHeight; ++y)
        {
            unsigned char* pPixelData(surfaceData + (nStride * y));

            for (tools::Long x(0); x < nWidth; ++x)
            {
                const BitmapColor aColor(pReadAccess->GetColor(y, x));

                pPixelData[SVP_CAIRO_RED] = aColor.GetRed();
                pPixelData[SVP_CAIRO_GREEN] = aColor.GetGreen();
                pPixelData[SVP_CAIRO_BLUE] = aColor.GetBlue();
                pPixelData[SVP_CAIRO_ALPHA] = 255; // not really needed
                pPixelData += 4;
            }
        }

        cairo_surface_mark_dirty(mpCairoSurface);
    }

// #define TEST_RGB16
#ifdef TEST_RGB16
    // experimental: create 16bit RGB data for given BitmapEx
    void createRGB16(const BitmapEx& rBitmapEx)
    {
        BitmapScopedReadAccess pReadAccess(rBitmapEx.GetBitmap());
        const tools::Long nHeight(pReadAccess->Height());
        const tools::Long nWidth(pReadAccess->Width());
        mpCairoSurface = cairo_image_surface_create(CAIRO_FORMAT_RGB16_565, nWidth, nHeight);
        sal_uInt32 nStride(cairo_format_stride_for_width(CAIRO_FORMAT_RGB16_565, nWidth));
        unsigned char* surfaceData(cairo_image_surface_get_data(mpCairoSurface));

        for (tools::Long y(0); y < nHeight; ++y)
        {
            unsigned char* pPixelData(surfaceData + (nStride * y));

            for (tools::Long x(0); x < nWidth; ++x)
            {
                const BitmapColor aColor(pReadAccess->GetColor(y, x));
                const sal_uInt8 aLeft((aColor.GetBlue() >> 3) | ((aColor.GetGreen() << 3) & 0xe0));
                const sal_uInt8 aRight((aColor.GetRed() & 0xf8) | (aColor.GetGreen() >> 5));
#ifdef OSL_BIGENDIAN
                pPixelData[1] = aRight;
                pPixelData[0] = aLeft;
#else
                pPixelData[0] = aLeft;
                pPixelData[1] = aRight;
#endif
                pPixelData += 2;
            }
        }

        cairo_surface_mark_dirty(mpCairoSurface);
    }
#endif

public:
    CairoSurfaceHelper(const BitmapEx& rBitmapEx)
        : mpCairoSurface(nullptr)
        , maDownscaled()
    {
        if (rBitmapEx.IsAlpha())
            createRGBA(rBitmapEx);
        else
#ifdef TEST_RGB16
            createRGB16(rBitmapEx);
#else
            createRGB(rBitmapEx);
#endif
    }

    ~CairoSurfaceHelper()
    {
        // cleanup surface
        cairo_surface_destroy(mpCairoSurface);

        // cleanup MipMap surfaces
        for (auto& candidate : maDownscaled)
            cairo_surface_destroy(candidate.second);
    }

    cairo_surface_t* getCairoSurface(sal_uInt32 nTargetWidth = 0,
                                     sal_uInt32 nTargetHeight = 0) const
    {
        // in simple cases just return the single created surface
        if (bDisableDownScale || nullptr == mpCairoSurface || 0 == nTargetWidth
            || 0 == nTargetHeight)
            return mpCairoSurface;

        // get width/height of original surface
        const sal_uInt32 nSourceWidth(cairo_image_surface_get_width(mpCairoSurface));
        const sal_uInt32 nSourceHeight(cairo_image_surface_get_height(mpCairoSurface));

        // zoomed in, need to stretch at paint, no pre-scale useful
        if (nTargetWidth >= nSourceWidth || nTargetHeight >= nSourceHeight)
            return mpCairoSurface;

        // calculate downscale factor. Only use ONE factor to get the diagonal
        // MipMap, NOT the full MipMap field in X/Y for uneven factors in both dimensions
        sal_uInt32 nFactor(1);
        sal_uInt32 nW((nSourceWidth + 1) / 2);
        sal_uInt32 nH((nSourceHeight + 1) / 2);

        while (nW > nTargetWidth && nW > nHalfMDSize && nH > nTargetHeight && nH > nHalfMDSize)
        {
            nW = (nW + 1) / 2;
            nH = (nH + 1) / 2;
            nFactor *= 2;
        }

        if (1 == nFactor)
        {
            // original size *is* best binary size, use it
            return mpCairoSurface;
        }

        // go up one scale again
        nW *= 2;
        nH *= 2;

        // bail out if the multiplication for the key would overflow
        if (nW >= SAL_MAX_UINT32 || nH >= SAL_MAX_UINT32)
            return mpCairoSurface;

        // check if we have a downscaled version of required size
        const sal_uInt64 key((nW * static_cast<sal_uInt64>(SAL_MAX_UINT32)) + nH);
        auto isHit(maDownscaled.find(key));

        // found -> return it
        if (isHit != maDownscaled.end())
            return isHit->second;

        // create new surface in the targeted size
        cairo_surface_t* pSurfaceTarget(cairo_surface_create_similar(
            mpCairoSurface, cairo_surface_get_content(mpCairoSurface), nW, nH));

        // made a version to scale self first with direct memory access.
        // That worked well, but would've been hard to support
        // CAIRO_FORMAT_A1 and similar (including bit shifting), so
        // I decided to go with cairo itself - use CAIRO_FILTER_FAST or
        // CAIRO_FILTER_GOOD though. Please modify as needed for
        // performance/quality
        cairo_t* cr = cairo_create(pSurfaceTarget);
        const double fScaleX(static_cast<double>(nW) / static_cast<double>(nSourceWidth));
        const double fScaleY(static_cast<double>(nH) / static_cast<double>(nSourceHeight));

        cairo_scale(cr, fScaleX, fScaleY);
        cairo_set_source_surface(cr, mpCairoSurface, 0.0, 0.0);
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_GOOD);
        cairo_paint(cr);
        cairo_destroy(cr);

        // NOTE: Took out, until now not really needed
        // need to set device_scale for downscale surfaces to get
        // them handled correctly
        // cairo_surface_set_device_scale(pSurfaceTarget, fScaleX, fScaleY);

        // add entry to cached entries
        maDownscaled[key] = pSurfaceTarget;

        return pSurfaceTarget;
    }

    bool isTrivial() const
    {
        if (nullptr == mpCairoSurface)
            return true;

        const sal_uInt32 nSourceWidth(cairo_image_surface_get_width(mpCairoSurface));
        const sal_uInt32 nSourceHeight(cairo_image_surface_get_height(mpCairoSurface));

        return nSourceWidth * nSourceHeight < nMinimalDiscreteSquareSizeToBuffer;
    }
};

class SystemDependentData_CairoSurface : public basegfx::SystemDependentData
{
    // the CairoSurface holder
    std::shared_ptr<CairoSurfaceHelper> mpCairoSurfaceHelper;

    // need to remember alpha source for combined BitmapEx to detect/
    // react on that changing
    std::shared_ptr<SalBitmap> maAssociatedAlpha;

public:
    SystemDependentData_CairoSurface(const BitmapEx& rBitmapEx)
        : basegfx::SystemDependentData(Application::GetSystemDependentDataManager(),
                                       basegfx::SDD_Type::SDDType_CairoSurface)
        , mpCairoSurfaceHelper(std::make_shared<CairoSurfaceHelper>(rBitmapEx))
        , maAssociatedAlpha()
    {
        if (rBitmapEx.IsAlpha())
            maAssociatedAlpha = rBitmapEx.GetAlphaMask().GetBitmap().ImplGetSalBitmap();
    }

    // read access
    const std::shared_ptr<CairoSurfaceHelper>& getCairoSurfaceHelper() const
    {
        return mpCairoSurfaceHelper;
    }
    const std::shared_ptr<SalBitmap>& getAssociatedAlpha() const { return maAssociatedAlpha; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

sal_Int64 SystemDependentData_CairoSurface::estimateUsageInBytes() const
{
    sal_Int64 nRetval(0);

    if (mpCairoSurfaceHelper)
    {
        cairo_surface_t* pSurface(mpCairoSurfaceHelper->getCairoSurface());
        const tools::Long nStride(cairo_image_surface_get_stride(pSurface));
        const tools::Long nHeight(cairo_image_surface_get_height(pSurface));

        nRetval = nStride * nHeight;

        // if we do downscale, size will grow by 1/4 + 1/16 + 1/32 + ...,
        // rough estimation just multiplies by 1.25 .. 1.33, should be good enough
        // for estimation of buffer survival time
        if (!bDisableDownScale)
        {
            nRetval = (nRetval * 5) / 4;
        }
    }

    return nRetval;
}

std::shared_ptr<CairoSurfaceHelper> getOrCreateCairoSurfaceHelper(const BitmapEx& rBitmapEx)
{
    const basegfx::SystemDependentDataHolder* pHolder(
        rBitmapEx.GetBitmap().accessSystemDependentDataHolder());
    std::shared_ptr<SystemDependentData_CairoSurface> pSystemDependentData_CairoSurface;

    if (nullptr != pHolder)
    {
        // try to access SystemDependentDataHolder and buffered data
        pSystemDependentData_CairoSurface
            = std::static_pointer_cast<SystemDependentData_CairoSurface>(
                pHolder->getSystemDependentData(basegfx::SDD_Type::SDDType_CairoSurface));

        // check data validity for associated Alpha
        if (pSystemDependentData_CairoSurface && rBitmapEx.IsAlpha()
            && pSystemDependentData_CairoSurface->getAssociatedAlpha()
                   != rBitmapEx.GetAlphaMask().GetBitmap().ImplGetSalBitmap())
        {
            // AssociatedAlpha did change, data invalid
            pSystemDependentData_CairoSurface.reset();
        }
    }

    if (!pSystemDependentData_CairoSurface)
    {
        // create new SystemDependentData_CairoSurface
        pSystemDependentData_CairoSurface
            = std::make_shared<SystemDependentData_CairoSurface>(rBitmapEx);

        // only add if feasible
        if (nullptr != pHolder
            && !pSystemDependentData_CairoSurface->getCairoSurfaceHelper()->isTrivial()
            && pSystemDependentData_CairoSurface->calculateCombinedHoldCyclesInSeconds() > 0)
        {
            basegfx::SystemDependentData_SharedPtr r2(pSystemDependentData_CairoSurface);
            const_cast<basegfx::SystemDependentDataHolder*>(pHolder)
                ->addOrReplaceSystemDependentData(r2);
        }
    }

    return pSystemDependentData_CairoSurface->getCairoSurfaceHelper();
}

// This bit-tweaking looping is unpleasant and unfortunate
void LuminanceToAlpha(cairo_surface_t* pMask)
{
    cairo_surface_flush(pMask);

    const sal_uInt32 nWidth(cairo_image_surface_get_width(pMask));
    const sal_uInt32 nHeight(cairo_image_surface_get_height(pMask));
    const sal_uInt32 nStride(cairo_image_surface_get_stride(pMask));

    if (0 == nWidth || 0 == nHeight)
        return;

    unsigned char* mask_surface_data(cairo_image_surface_get_data(pMask));

    // include/basegfx/color/bcolormodifier.hxx
    constexpr double nRedMul(0.2125 / 255.0);
    constexpr double nGreenMul(0.7154 / 255.0);
    constexpr double nBlueMul(0.0721 / 255.0);

    // Only this alpha channel is taken into account by cairo_mask_surface
    // so reuse this surface for the alpha result
    for (sal_uInt32 y(0); y < nHeight; ++y)
    {
        unsigned char* pMaskPixelData = mask_surface_data + (nStride * y);

        for (sal_uInt32 x(0); x < nWidth; ++x)
        {
            // do not forget that we have pre-multiplied alpha
            const sal_uInt8 nAlpha(pMaskPixelData[SVP_CAIRO_ALPHA]);

            if (0 != nAlpha)
            {
                const double fLuminance = pMaskPixelData[SVP_CAIRO_RED] * nRedMul
                                          + pMaskPixelData[SVP_CAIRO_GREEN] * nGreenMul
                                          + pMaskPixelData[SVP_CAIRO_BLUE] * nBlueMul;

                if (255 != nAlpha)
                    pMaskPixelData[SVP_CAIRO_ALPHA] = fLuminance / nAlpha;
                else
                    pMaskPixelData[SVP_CAIRO_ALPHA] = 255.0 * fLuminance;
            }

            pMaskPixelData += 4;
        }
    }

    cairo_surface_mark_dirty(pMask);
}
}

namespace drawinglayer::processor2d
{
CairoPixelProcessor2D::CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                             cairo_surface_t* pTarget)
    : BaseProcessor2D(rViewInformation)
    , maBColorModifierStack()
    , mpRT(nullptr)
{
    if (pTarget)
    {
        cairo_t* pRT = cairo_create(pTarget);
        cairo_set_antialias(pRT, rViewInformation.getUseAntiAliasing() ? CAIRO_ANTIALIAS_DEFAULT
                                                                       : CAIRO_ANTIALIAS_NONE);
        cairo_set_fill_rule(pRT, CAIRO_FILL_RULE_EVEN_ODD);
        setRenderTarget(pRT);
    }
}

CairoPixelProcessor2D::~CairoPixelProcessor2D()
{
    if (mpRT)
        cairo_destroy(mpRT);
}

void CairoPixelProcessor2D::processBitmapPrimitive2D(
    const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
{
    // check if graphic content is inside discrete local ViewPort
    const basegfx::B2DRange& rDiscreteViewPort(getViewInformation2D().getDiscreteViewport());
    const basegfx::B2DHomMatrix aLocalTransform(
        getViewInformation2D().getObjectToViewTransformation() * rBitmapCandidate.getTransform());

    if (!rDiscreteViewPort.isEmpty())
    {
        basegfx::B2DRange aUnitRange(0.0, 0.0, 1.0, 1.0);

        aUnitRange.transform(aLocalTransform);

        if (!aUnitRange.overlaps(rDiscreteViewPort))
        {
            // content is outside discrete local ViewPort
            return;
        }
    }

    BitmapEx aBitmapEx(rBitmapCandidate.getBitmap());

    if (aBitmapEx.IsEmpty() || aBitmapEx.GetSizePixel().IsEmpty())
    {
        // no pixel data, done
        return;
    }

    if (maBColorModifierStack.count())
    {
        // need to apply ColorModifier to Bitmap data
        aBitmapEx = aBitmapEx.ModifyBitmapEx(maBColorModifierStack);

        if (aBitmapEx.IsEmpty())
        {
            // color gets completely replaced, get it
            const basegfx::BColor aModifiedColor(
                maBColorModifierStack.getModifiedColor(basegfx::BColor()));

            // use unit geometry as fallback object geometry. Do *not*
            // transform, the below used method will use the already
            // correctly initialized local ViewInformation
            basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());

            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D> xTemp(
                new primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon),
                                                             aModifiedColor));

            // draw as Polygon, done
            processPolyPolygonColorPrimitive2D(*xTemp);
            return;
        }
    }

    // access or create cairo bitmap data
    std::shared_ptr<CairoSurfaceHelper> aCairoSurfaceHelper(
        getOrCreateCairoSurfaceHelper(aBitmapEx));
    if (!aCairoSurfaceHelper)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No SurfaceHelper from BitmapEx (!)");
        return;
    }

    // work with dimensions in discrete target pixels to use evtl. MipMap pre-scale
    const tools::Long nDestWidth((aLocalTransform * basegfx::B2DVector(1.0, 0.0)).getLength());
    const tools::Long nDestHeight((aLocalTransform * basegfx::B2DVector(0.0, 1.0)).getLength());

    cairo_surface_t* pTarget(aCairoSurfaceHelper->getCairoSurface(nDestWidth, nDestHeight));
    if (nullptr == pTarget)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No CairoSurface from BitmapEx SurfaceHelper (!)");
        return;
    }

    cairo_save(mpRT);

    // set linear transformation - no fAAOffset for bitmap data
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(),
                      aLocalTransform.d(), aLocalTransform.e(), aLocalTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);

    static bool bRenderTransformationBounds(false);
    if (bRenderTransformationBounds)
    {
        cairo_set_source_rgba(mpRT, 1, 0, 0, 0.8);
        impl_cairo_set_hairline(mpRT, getViewInformation2D());
        cairo_rectangle(mpRT, 0, 0, 1, 1);
        cairo_stroke(mpRT);
    }

    const sal_uInt32 nWidth(cairo_image_surface_get_width(pTarget));
    const sal_uInt32 nHeight(cairo_image_surface_get_height(pTarget));

    cairo_set_source_surface(mpRT, pTarget, 0, 0);

    // get the pattern created by cairo_set_source_surface and
    // it's transformation
    cairo_pattern_t* sourcepattern = cairo_get_source(mpRT);
    cairo_pattern_get_matrix(sourcepattern, &aMatrix);

    // RGBA sources overlap the unit geometry range, slightly,
    // to see that activate bRenderTransformationBounds and
    // insert a ARGB image, zoom to the borders. Seems to be half
    // a pixel. Very good to demonstrate: 8x1 pixel, some
    // transparent.
    // This effect is also visible in the left/right/bottom/top
    // page shadows, these DO use 8x1/1x8 images which led me to
    // that problem. I see two solutions:
    static bool bRenderMasked(true);
    if (bRenderMasked)
    {
        // Consequence is that these need clipping. That again is
        // simple (we are in unit coordinates). Only do for RGBA,
        // for RGB this effect does not happen
        if (CAIRO_FORMAT_ARGB32 == cairo_image_surface_get_format(pTarget))
        {
            cairo_rectangle(mpRT, 0, 0, 1, 1);
            cairo_clip(mpRT);
        }

        cairo_matrix_scale(&aMatrix, nWidth, nHeight);
    }
    else
    {
        // Alternative: for RGBA, resize/scale SLIGHTLY so that
        // half pixel overlap is forced to be inside the unit range.
        // That makes the error disappear, so no clip needed, but
        // SLIGHTLY smaller.
        if (CAIRO_FORMAT_ARGB32 == cairo_image_surface_get_format(pTarget))
        {
            cairo_matrix_init_scale(&aMatrix, nWidth + 1, nHeight + 1);
            cairo_matrix_translate(&aMatrix, -0.5 / (nWidth + 1), -0.5 / (nHeight + 1));
        }
        else
        {
            cairo_matrix_scale(&aMatrix, nWidth, nHeight);
        }
    }

    cairo_pattern_set_matrix(sourcepattern, &aMatrix);

    // paint bitmap data
    cairo_paint(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPointArrayPrimitive2D(
    const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
{
    const std::vector<basegfx::B2DPoint>& rPositions(rPointArrayCandidate.getPositions());

    if (rPositions.empty())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    // determine & set color
    const basegfx::BColor aPointColor(
        maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
    cairo_set_source_rgb(mpRT, aPointColor.getRed(), aPointColor.getGreen(), aPointColor.getBlue());

    // To really paint a single pixel I found nothing better than
    // switch off AA and draw a pixel-aligned rectangle
    const cairo_antialias_t eOldAAMode(cairo_get_antialias(mpRT));
    cairo_set_antialias(mpRT, CAIRO_ANTIALIAS_NONE);

    for (auto const& pos : rPositions)
    {
        const basegfx::B2DPoint aDiscretePos(getViewInformation2D().getObjectToViewTransformation()
                                             * pos);
        const double fX(ceil(aDiscretePos.getX()));
        const double fY(ceil(aDiscretePos.getY()));

        cairo_rectangle(mpRT, fX, fY, 1, 1);
        cairo_fill(mpRT);
    }

    cairo_set_antialias(mpRT, eOldAAMode);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPolygonHairlinePrimitive2D(
    const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D)
{
    const basegfx::B2DPolygon& rPolygon(rPolygonHairlinePrimitive2D.getB2DPolygon());

    if (!rPolygon.count())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    // set linear transformation
    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);
    cairo_set_matrix(mpRT, &aMatrix);

    // determine & set color
    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rPolygonHairlinePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    // set LineWidth, use cairo special cairo_set_hairline
    impl_cairo_set_hairline(mpRT, getViewInformation2D());

    // get PathGeometry & paint it
    cairo_new_path(mpRT);
    getOrCreatePathGeometry(mpRT, rPolygon, getViewInformation2D());
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
{
    const basegfx::B2DPolyPolygon& rPolyPolygon(rPolyPolygonColorPrimitive2D.getB2DPolyPolygon());
    const sal_uInt32 nCount(rPolyPolygon.count());

    if (!nCount)
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    // set linear transformation
    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);
    cairo_set_matrix(mpRT, &aMatrix);

    // determine & set color
    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rPolyPolygonColorPrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());

    // get PathGeometry & paint it
    cairo_new_path(mpRT);
    getOrCreateFillGeometry(mpRT, rPolyPolygon);
    cairo_fill(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processTransparencePrimitive2D(
    const primitive2d::TransparencePrimitive2D& rTransCandidate)
{
    if (rTransCandidate.getChildren().empty())
    {
        // no content, done
        return;
    }

    if (rTransCandidate.getTransparence().empty())
    {
        // no mask (so nothing visible), done
        return;
    }

    // calculate visible range, create only for that range
    basegfx::B2DRange aDiscreteRange(
        rTransCandidate.getChildren().getB2DRange(getViewInformation2D()));
    aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());
    basegfx::B2DRange aVisibleRange(aDiscreteRange);
    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));
    aVisibleRange.intersect(aViewRange);

    if (aVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    cairo_save(mpRT);

    // create embedding transformation for sub-surface
    const basegfx::B2DHomMatrix aEmbedTransform(basegfx::utils::createTranslateB2DHomMatrix(
        -aVisibleRange.getMinX(), -aVisibleRange.getMinY()));
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setViewTransformation(aEmbedTransform
                                             * getViewInformation2D().getViewTransformation());

    // draw mask to temporary surface
    cairo_surface_t* pTarget(cairo_get_target(mpRT));
    const double fContainedWidth(ceil(aVisibleRange.getWidth()));
    const double fContainedHeight(ceil(aVisibleRange.getHeight()));
    cairo_surface_t* pMask(cairo_surface_create_similar_image(pTarget, CAIRO_FORMAT_ARGB32,
                                                              fContainedWidth, fContainedHeight));
    CairoPixelProcessor2D aMaskRenderer(aViewInformation2D, pMask);
    aMaskRenderer.process(rTransCandidate.getTransparence());

    // convert mask to something cairo can use
    LuminanceToAlpha(pMask);

    // draw content to temporary surface
    cairo_surface_t* pContent(cairo_surface_create_similar(
        pTarget, cairo_surface_get_content(pTarget), fContainedWidth, fContainedHeight));
    CairoPixelProcessor2D aContent(aViewInformation2D, pContent);
    aContent.process(rTransCandidate.getChildren());

    // munge the temporary surfaces to our target surface
    cairo_set_source_surface(mpRT, pContent, aVisibleRange.getMinX(), aVisibleRange.getMinY());
    cairo_mask_surface(mpRT, pMask, aVisibleRange.getMinX(), aVisibleRange.getMinY());

    // cleanup temporary surfaces
    cairo_surface_destroy(pContent);
    cairo_surface_destroy(pMask);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processInvertPrimitive2D(
    const primitive2d::InvertPrimitive2D& rInvertCandidate)
{
    if (rInvertCandidate.getChildren().empty())
    {
        // no content, done
        return;
    }

    // calculate visible range, create only for that range
    basegfx::B2DRange aDiscreteRange(
        rInvertCandidate.getChildren().getB2DRange(getViewInformation2D()));
    aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());
    basegfx::B2DRange aVisibleRange(aDiscreteRange);
    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));
    aVisibleRange.intersect(aViewRange);

    if (aVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    cairo_save(mpRT);

    // create embedding transformation for sub-surface
    const basegfx::B2DHomMatrix aEmbedTransform(basegfx::utils::createTranslateB2DHomMatrix(
        -aVisibleRange.getMinX(), -aVisibleRange.getMinY()));
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setViewTransformation(aEmbedTransform
                                             * getViewInformation2D().getViewTransformation());

    // draw sub-content to temporary surface
    cairo_surface_t* pTarget(cairo_get_target(mpRT));
    const double fContainedWidth(ceil(aVisibleRange.getWidth()));
    const double fContainedHeight(ceil(aVisibleRange.getHeight()));
    cairo_surface_t* pContent(cairo_surface_create_similar_image(
        pTarget, CAIRO_FORMAT_ARGB32, fContainedWidth, fContainedHeight));
    CairoPixelProcessor2D aContent(aViewInformation2D, pContent);
    aContent.process(rInvertCandidate.getChildren());
    cairo_surface_flush(pContent);

    // get read access to target - XOR unfortunately needs that
    cairo_surface_t* pRenderTarget(pTarget);

    if (CAIRO_SURFACE_TYPE_IMAGE != cairo_surface_get_type(pRenderTarget))
    {
        pRenderTarget = cairo_surface_map_to_image(pRenderTarget, nullptr);
    }

    // iterate over pre-rendered pContent, call Dst due to being changed
    const sal_uInt32 nDstWidth(cairo_image_surface_get_width(pContent));
    const sal_uInt32 nDstHeight(cairo_image_surface_get_height(pContent));
    const sal_uInt32 nDstStride(cairo_image_surface_get_stride(pContent));
    unsigned char* pDstDataRoot(cairo_image_surface_get_data(pContent));

    // in parallel, iterate over Src data
    const sal_uInt32 nSrcOffX(floor(aVisibleRange.getMinX()));
    const sal_uInt32 nSrcOffY(floor(aVisibleRange.getMinY()));
    const sal_uInt32 nSrcStride(cairo_image_surface_get_stride(pRenderTarget));
    unsigned char* pSrcDataRoot(cairo_image_surface_get_data(pRenderTarget));

    if (nullptr != pDstDataRoot && nullptr != pSrcDataRoot)
    {
        for (sal_uInt32 y(0); y < nDstHeight; ++y)
        {
            // get mem locations
            unsigned char* pDstData(pDstDataRoot + (nDstStride * y));
            unsigned char* pSrcData(pSrcDataRoot + (nSrcStride * (y + nSrcOffY)) + (nSrcOffX * 4));

            for (sal_uInt32 x(0); x < nDstWidth; ++x)
            {
                // do not forget pre-multiply -> need to get both alphas
                sal_uInt8 nSrcAlpha(pSrcData[SVP_CAIRO_ALPHA]);
                sal_uInt8 nDstAlpha(pDstData[SVP_CAIRO_ALPHA]);

                // create XOR r,g,b
                const sal_uInt8 b(
                    vcl::bitmap::unpremultiply(nDstAlpha, pDstData[SVP_CAIRO_BLUE])
                    ^ vcl::bitmap::unpremultiply(nSrcAlpha, pSrcData[SVP_CAIRO_BLUE]));
                const sal_uInt8 g(
                    vcl::bitmap::unpremultiply(nDstAlpha, pDstData[SVP_CAIRO_GREEN])
                    ^ vcl::bitmap::unpremultiply(nSrcAlpha, pSrcData[SVP_CAIRO_GREEN]));
                const sal_uInt8 r(vcl::bitmap::unpremultiply(nDstAlpha, pDstData[SVP_CAIRO_RED])
                                  ^ vcl::bitmap::unpremultiply(nSrcAlpha, pSrcData[SVP_CAIRO_RED]));

                // write back
                pDstData[SVP_CAIRO_BLUE] = vcl::bitmap::premultiply(nDstAlpha, b);
                pDstData[SVP_CAIRO_GREEN] = vcl::bitmap::premultiply(nDstAlpha, g);
                pDstData[SVP_CAIRO_RED] = vcl::bitmap::premultiply(nDstAlpha, r);

                // advance memory
                pSrcData += 4;
                pDstData += 4;
            }
        }

        cairo_surface_mark_dirty(pContent);
    }

    if (pRenderTarget != pTarget)
    {
        // cleanup mapping for read access to target
        cairo_surface_unmap_image(pTarget, pRenderTarget);
    }

    // draw created XOR to target
    cairo_set_source_surface(mpRT, pContent, aVisibleRange.getMinX(), aVisibleRange.getMinY());
    cairo_rectangle(mpRT, aVisibleRange.getMinX(), aVisibleRange.getMinY(),
                    aVisibleRange.getWidth(), aVisibleRange.getHeight());
    cairo_fill(mpRT);

    // cleanup temporary surface
    cairo_surface_destroy(pContent);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processMaskPrimitive2D(
    const primitive2d::MaskPrimitive2D& rMaskCandidate,
    const primitive2d::FillGradientPrimitive2D* pFillGradientPrimitive2D,
    const attribute::FillGradientAttribute* pFillGradientAlpha)
{
    if (rMaskCandidate.getChildren().empty())
    {
        // no content, done
        return;
    }

    basegfx::B2DPolyPolygon aMask(rMaskCandidate.getMask());

    if (!aMask.count())
    {
        // no mask (so nothing inside), done
        return;
    }

    // calculate visible range
    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    basegfx::B2DRange aMaskRange(aMask.getB2DRange());
    aMaskRange.transform(getViewInformation2D().getObjectToViewTransformation());
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));

    if (!aViewRange.overlaps(aMaskRange))
    {
        // not visible, done
        return;
    }

    cairo_save(mpRT);

    // set linear transformation
    cairo_matrix_t aMatrix;
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);
    cairo_set_matrix(mpRT, &aMatrix);

    // create path geometry and put mask as path
    cairo_new_path(mpRT);
    getOrCreateFillGeometry(mpRT, aMask);

    // clip to this mask (also reset path, cairo_clip does not consume it)
    cairo_clip(mpRT);
    cairo_new_path(mpRT);

    if (nullptr != pFillGradientPrimitive2D && nullptr != pFillGradientAlpha)
    {
        // special case: render given FillGradientPrimitive2D using
        // FillGradientAlpha as RGBA gradient directly
        // note that calling this method with nullptr != pFillGradientAlpha
        // is only allowed internal from
        // CairoPixelProcessor2D::processPolyPolygonRGBAGradientPrimitive2D
        processFillGradientPrimitive2D(*pFillGradientPrimitive2D, pFillGradientAlpha);
    }
    else
    {
        // process sub-content (that shall be masked)
        process(rMaskCandidate.getChildren());
    }

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processModifiedColorPrimitive2D(
    const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate)
{
    // standard implementation
    if (!rModifiedCandidate.getChildren().empty())
    {
        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
        process(rModifiedCandidate.getChildren());
        maBColorModifierStack.pop();
    }
}

void CairoPixelProcessor2D::processTransformPrimitive2D(
    const primitive2d::TransformPrimitive2D& rTransformCandidate)
{
    // standard implementation
    // remember current transformation and ViewInformation
    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

    // create new transformations for local ViewInformation2D
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setObjectTransformation(getViewInformation2D().getObjectTransformation()
                                               * rTransformCandidate.getTransformation());
    updateViewInformation(aViewInformation2D);

    // process content
    process(rTransformCandidate.getChildren());

    // restore transformations
    updateViewInformation(aLastViewInformation2D);
}

void CairoPixelProcessor2D::processUnifiedTransparencePrimitive2D(
    const primitive2d::UnifiedTransparencePrimitive2D& rTransCandidate)
{
    if (rTransCandidate.getChildren().empty())
    {
        // no content, done
        return;
    }

    if (0.0 == rTransCandidate.getTransparence())
    {
        // not transparent at all, use content
        process(rTransCandidate.getChildren());
        return;
    }

    if (rTransCandidate.getTransparence() < 0.0 || rTransCandidate.getTransparence() > 1.0)
    {
        // invalid transparence, done
        return;
    }

    cairo_save(mpRT);

    // calculate visible range, create only for that range
    basegfx::B2DRange aDiscreteRange(
        rTransCandidate.getChildren().getB2DRange(getViewInformation2D()));
    aDiscreteRange.transform(getViewInformation2D().getObjectToViewTransformation());
    basegfx::B2DRange aVisibleRange(aDiscreteRange);
    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_clip_extents(mpRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    const basegfx::B2DRange aViewRange(basegfx::B2DPoint(clip_x1, clip_y1),
                                       basegfx::B2DPoint(clip_x2, clip_y2));
    aVisibleRange.intersect(aViewRange);

    if (aVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    // create embedding transformation for sub-surface
    const basegfx::B2DHomMatrix aEmbedTransform(basegfx::utils::createTranslateB2DHomMatrix(
        -aVisibleRange.getMinX(), -aVisibleRange.getMinY()));
    geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
    aViewInformation2D.setViewTransformation(aEmbedTransform
                                             * getViewInformation2D().getViewTransformation());

    // draw content to temporary surface
    cairo_surface_t* pTarget(cairo_get_target(mpRT));
    const double fContainedWidth(ceil(aVisibleRange.getWidth()));
    const double fContainedHeight(ceil(aVisibleRange.getHeight()));
    cairo_surface_t* pContent(cairo_surface_create_similar(
        pTarget, cairo_surface_get_content(pTarget), fContainedWidth, fContainedHeight));
    CairoPixelProcessor2D aContent(aViewInformation2D, pContent);
    aContent.process(rTransCandidate.getChildren());

    // paint temporary surface to target with fixed transparence
    cairo_set_source_surface(mpRT, pContent, aVisibleRange.getMinX(), aVisibleRange.getMinY());
    cairo_paint_with_alpha(mpRT, 1.0 - rTransCandidate.getTransparence());

    // cleanup temporary surface
    cairo_surface_destroy(pContent);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processMarkerArrayPrimitive2D(
    const primitive2d::MarkerArrayPrimitive2D& rMarkerArrayCandidate)
{
    const std::vector<basegfx::B2DPoint>& rPositions(rMarkerArrayCandidate.getPositions());

    if (rPositions.empty())
    {
        // no geometry, done
        return;
    }

    const BitmapEx& rMarker(rMarkerArrayCandidate.getMarker());

    if (rMarker.IsEmpty())
    {
        // no marker defined, done
        return;
    }

    // access or create cairo bitmap data
    const BitmapEx& rBitmapEx(rMarkerArrayCandidate.getMarker());
    std::shared_ptr<CairoSurfaceHelper> aCairoSurfaceHelper(
        getOrCreateCairoSurfaceHelper(rBitmapEx));
    if (!aCairoSurfaceHelper)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No SurfaceHelper from BitmapEx (!)");
        return;
    }

    // do not use dimensions, these are usually small instances
    cairo_surface_t* pTarget(aCairoSurfaceHelper->getCairoSurface());
    if (nullptr == pTarget)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No CairoSurface from BitmapEx SurfaceHelper (!)");
        return;
    }

    const sal_uInt32 nWidth(cairo_image_surface_get_width(pTarget));
    const sal_uInt32 nHeight(cairo_image_surface_get_height(pTarget));
    const tools::Long nMiX((nWidth / 2) + 1);
    const tools::Long nMiY((nHeight / 2) + 1);

    cairo_save(mpRT);
    cairo_identity_matrix(mpRT);
    const cairo_antialias_t eOldAAMode(cairo_get_antialias(mpRT));
    cairo_set_antialias(mpRT, CAIRO_ANTIALIAS_NONE);

    for (auto const& pos : rPositions)
    {
        const basegfx::B2DPoint aDiscretePos(getViewInformation2D().getObjectToViewTransformation()
                                             * pos);
        const double fX(ceil(aDiscretePos.getX()));
        const double fY(ceil(aDiscretePos.getY()));

        cairo_set_source_surface(mpRT, pTarget, fX - nMiX, fY - nMiY);
        cairo_paint(mpRT);
    }

    cairo_set_antialias(mpRT, eOldAAMode);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processBackgroundColorPrimitive2D(
    const primitive2d::BackgroundColorPrimitive2D& rBackgroundColorCandidate)
{
    // check for allowed range [0.0 .. 1.0[
    if (rBackgroundColorCandidate.getTransparency() < 0.0
        || rBackgroundColorCandidate.getTransparency() >= 1.0)
        return;

    cairo_save(mpRT);
    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rBackgroundColorCandidate.getBColor()));
    cairo_set_source_rgba(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue(),
                          1.0 - rBackgroundColorCandidate.getTransparency());
    // to also copy alpha part of color, see cairo docu. Will be reset by restore below
    cairo_set_operator(mpRT, CAIRO_OPERATOR_SOURCE);
    cairo_paint(mpRT);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPolygonStrokePrimitive2D(
    const primitive2d::PolygonStrokePrimitive2D& rPolygonStrokeCandidate)
{
    const basegfx::B2DPolygon& rPolygon(rPolygonStrokeCandidate.getB2DPolygon());
    const attribute::LineAttribute& rLineAttribute(rPolygonStrokeCandidate.getLineAttribute());

    if (!rPolygon.count() || rLineAttribute.getWidth() < 0.0)
    {
        // no geometry, done
        return;
    }

    // get some values early that might be used for decisions
    const bool bHairline(0.0 == rLineAttribute.getWidth());
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    const double fDiscreteLineWidth(
        bHairline
            ? 1.0
            : (rObjectToView * basegfx::B2DVector(rLineAttribute.getWidth(), 0.0)).getLength());

    // Here for every combination which the system-specific implementation is not
    // capable of visualizing, use the (for decomposable Primitives always possible)
    // fallback to the decomposition.
    if (basegfx::B2DLineJoin::NONE == rLineAttribute.getLineJoin() && fDiscreteLineWidth > 1.5)
    {
        // basegfx::B2DLineJoin::NONE is special for our office, no other GraphicSystem
        // knows that (so far), so fallback to decomposition. This is only needed if
        // LineJoin will be used, so also check for discrete LineWidth before falling back
        process(rPolygonStrokeCandidate);
        return;
    }

    // This is a method every system-specific implementation of a decomposable Primitive
    // can use to allow simple optical control of paint implementation:
    // Create a copy, e.g. change color to 'red' as here and paint before the system
    // paints it using the decomposition. That way you can - if active - directly
    // optically compare if the system-specific solution is geometrically identical to
    // the decomposition (which defines our interpretation that we need to visualize).
    // Look below in the impl for bRenderDecomposeForCompareInRed to see that in that case
    // we create a half-transparent paint to better support visual control
    static bool bRenderDecomposeForCompareInRed(false);

    if (bRenderDecomposeForCompareInRed)
    {
        const attribute::LineAttribute aRed(
            basegfx::BColor(1.0, 0.0, 0.0), rLineAttribute.getWidth(), rLineAttribute.getLineJoin(),
            rLineAttribute.getLineCap(), rLineAttribute.getMiterMinimumAngle());
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> xCopy(
            new primitive2d::PolygonStrokePrimitive2D(
                rPolygonStrokeCandidate.getB2DPolygon(), aRed,
                rPolygonStrokeCandidate.getStrokeAttribute()));
        process(*xCopy);
    }

    cairo_save(mpRT);

    // set linear transformation
    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);
    cairo_set_matrix(mpRT, &aMatrix);

    // setup line attributes
    cairo_line_join_t eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
    switch (rLineAttribute.getLineJoin())
    {
        case basegfx::B2DLineJoin::Bevel:
            eCairoLineJoin = CAIRO_LINE_JOIN_BEVEL;
            break;
        case basegfx::B2DLineJoin::Round:
            eCairoLineJoin = CAIRO_LINE_JOIN_ROUND;
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            eCairoLineJoin = CAIRO_LINE_JOIN_MITER;
            break;
    }
    cairo_set_line_join(mpRT, eCairoLineJoin);

    // convert miter minimum angle to miter limit
    double fMiterLimit
        = 1.0 / sin(std::max(rLineAttribute.getMiterMinimumAngle(), 0.01 * M_PI) / 2.0);
    cairo_set_miter_limit(mpRT, fMiterLimit);

    // setup cap attribute
    cairo_line_cap_t eCairoLineCap(CAIRO_LINE_CAP_BUTT);
    switch (rLineAttribute.getLineCap())
    {
        default: // css::drawing::LineCap_BUTT:
        {
            eCairoLineCap = CAIRO_LINE_CAP_BUTT;
            break;
        }
        case css::drawing::LineCap_ROUND:
        {
            eCairoLineCap = CAIRO_LINE_CAP_ROUND;
            break;
        }
        case css::drawing::LineCap_SQUARE:
        {
            eCairoLineCap = CAIRO_LINE_CAP_SQUARE;
            break;
        }
    }
    cairo_set_line_cap(mpRT, eCairoLineCap);

    // determine & set color
    basegfx::BColor aLineColor(maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
    if (bRenderDecomposeForCompareInRed)
        aLineColor.setRed(0.5);
    cairo_set_source_rgb(mpRT, aLineColor.getRed(), aLineColor.getGreen(), aLineColor.getBlue());

    // process/set LineWidth
    const double fObjectLineWidth(
        bHairline ? (getViewInformation2D().getInverseObjectToViewTransformation()
                     * basegfx::B2DVector(1.0, 0.0))
                        .getLength()
                  : rLineAttribute.getWidth());
    cairo_set_line_width(mpRT, fObjectLineWidth);

    // check stroke
    const attribute::StrokeAttribute& rStrokeAttribute(
        rPolygonStrokeCandidate.getStrokeAttribute());
    const bool bDashUsed(!rStrokeAttribute.isDefault()
                         && !rStrokeAttribute.getDotDashArray().empty()
                         && 0.0 < rStrokeAttribute.getFullDotDashLen());
    if (bDashUsed)
    {
        const std::vector<double>& rStroke = rStrokeAttribute.getDotDashArray();
        cairo_set_dash(mpRT, rStroke.data(), rStroke.size(), 0.0);
    }

    // create path geometry and put mask as path
    cairo_new_path(mpRT);
    getOrCreatePathGeometry(mpRT, rPolygon, getViewInformation2D());

    // render
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processLineRectanglePrimitive2D(
    const primitive2d::LineRectanglePrimitive2D& rLineRectanglePrimitive2D)
{
    if (rLineRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rLineRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    const double fDiscreteLineWidth((getViewInformation2D().getInverseObjectToViewTransformation()
                                     * basegfx::B2DVector(1.0, 0.0))
                                        .getLength());
    cairo_set_line_width(mpRT, fDiscreteLineWidth);

    const basegfx::B2DRange& rRange(rLineRectanglePrimitive2D.getB2DRange());
    cairo_rectangle(mpRT, rRange.getMinX(), rRange.getMinY(), rRange.getWidth(),
                    rRange.getHeight());
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFilledRectanglePrimitive2D(
    const primitive2d::FilledRectanglePrimitive2D& rFilledRectanglePrimitive2D)
{
    if (rFilledRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    cairo_matrix_t aMatrix;
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                      rObjectToView.d(), rObjectToView.e() + fAAOffset,
                      rObjectToView.f() + fAAOffset);

    // set linear transformation
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rFilledRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());

    const basegfx::B2DRange& rRange(rFilledRectanglePrimitive2D.getB2DRange());
    cairo_rectangle(mpRT, rRange.getMinX(), rRange.getMinY(), rRange.getWidth(),
                    rRange.getHeight());
    cairo_fill(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processSingleLinePrimitive2D(
    const primitive2d::SingleLinePrimitive2D& rSingleLinePrimitive2D)
{
    cairo_save(mpRT);

    const basegfx::BColor aLineColor(
        maBColorModifierStack.getModifiedColor(rSingleLinePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aLineColor.getRed(), aLineColor.getGreen(), aLineColor.getBlue());

    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    const basegfx::B2DHomMatrix& rObjectToView(
        getViewInformation2D().getObjectToViewTransformation());
    const basegfx::B2DPoint aStart(rObjectToView * rSingleLinePrimitive2D.getStart());
    const basegfx::B2DPoint aEnd(rObjectToView * rSingleLinePrimitive2D.getEnd());

    cairo_set_line_width(mpRT, 1.0f);

    cairo_move_to(mpRT, aStart.getX() + fAAOffset, aStart.getY() + fAAOffset);
    cairo_line_to(mpRT, aEnd.getX() + fAAOffset, aEnd.getY() + fAAOffset);
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGraphicPrimitive2D(
    const primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D)
{
    BitmapEx aPreparedBitmap;
    basegfx::B2DRange aFillUnitRange(rFillGraphicPrimitive2D.getFillGraphic().getGraphicRange());
    constexpr double fBigDiscreteArea(300.0 * 300.0);

    // use tooling to do various checks and prepare tiled rendering, see
    // description of method, parameters and return value there
    if (!prepareBitmapForDirectRender(rFillGraphicPrimitive2D, getViewInformation2D(),
                                      aPreparedBitmap, aFillUnitRange, fBigDiscreteArea))
    {
        // no output needed, done
        return;
    }

    if (aPreparedBitmap.IsEmpty())
    {
        // output needed and Bitmap data empty, so no bitmap data based
        // tiled rendering is suggested. Use fallback for paint (decomposition)
        process(rFillGraphicPrimitive2D);
        return;
    }

    // render tiled using the prepared Bitmap data
    if (maBColorModifierStack.count())
    {
        // need to apply ColorModifier to Bitmap data
        aPreparedBitmap = aPreparedBitmap.ModifyBitmapEx(maBColorModifierStack);

        if (aPreparedBitmap.IsEmpty())
        {
            // color gets completely replaced, get it (any input works)
            const basegfx::BColor aModifiedColor(
                maBColorModifierStack.getModifiedColor(basegfx::BColor()));

            // use unit geometry as fallback object geometry. Do *not*
            // transform, the below used method will use the already
            // correctly initialized local ViewInformation
            basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());

            // what we still need to apply is the object transform from the
            // local primitive, that is not part of DisplayInfo yet
            aPolygon.transform(rFillGraphicPrimitive2D.getTransformation());

            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D> aTemp(
                new primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon),
                                                             aModifiedColor));

            // draw as colored Polygon, done
            processPolyPolygonColorPrimitive2D(*aTemp);
            return;
        }
    }

    // access or create cairo bitmap data
    std::shared_ptr<CairoSurfaceHelper> aCairoSurfaceHelper(
        getOrCreateCairoSurfaceHelper(aPreparedBitmap));
    if (!aCairoSurfaceHelper)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No SurfaceHelper from BitmapEx (!)");
        return;
    }

    // work with dimensions in discrete target pixels to use evtl. MipMap pre-scale
    const basegfx::B2DHomMatrix aLocalTransform(
        getViewInformation2D().getObjectToViewTransformation()
        * rFillGraphicPrimitive2D.getTransformation());
    const tools::Long nDestWidth(
        (aLocalTransform * basegfx::B2DVector(aFillUnitRange.getWidth(), 0.0)).getLength());
    const tools::Long nDestHeight(
        (aLocalTransform * basegfx::B2DVector(0.0, aFillUnitRange.getHeight())).getLength());

    cairo_surface_t* pTarget(aCairoSurfaceHelper->getCairoSurface(nDestWidth, nDestHeight));
    if (nullptr == pTarget)
    {
        SAL_WARN("drawinglayer", "SDPRCairo: No CairoSurface from BitmapEx SurfaceHelper (!)");
        return;
    }

    cairo_save(mpRT);

    // set linear transformation - no fAAOffset for bitmap data
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(),
                      aLocalTransform.d(), aLocalTransform.e(), aLocalTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);

    const sal_uInt32 nWidth(cairo_image_surface_get_width(pTarget));
    const sal_uInt32 nHeight(cairo_image_surface_get_height(pTarget));

    cairo_set_source_surface(mpRT, pTarget, 0, 0);

    // get the pattern created by cairo_set_source_surface and
    // it's transformation
    cairo_pattern_t* sourcepattern = cairo_get_source(mpRT);
    cairo_pattern_get_matrix(sourcepattern, &aMatrix);

    // clip for RGBA (see other places)
    if (CAIRO_FORMAT_ARGB32 == cairo_image_surface_get_format(pTarget))
    {
        cairo_rectangle(mpRT, 0, 0, 1, 1);
        cairo_clip(mpRT);
    }

    // create transformation for source pattern (inverse, see
    // cairo docu: uses user space to pattern space transformation)
    cairo_matrix_init_scale(&aMatrix, nWidth / aFillUnitRange.getWidth(),
                            nHeight / aFillUnitRange.getHeight());
    cairo_matrix_translate(&aMatrix, -aFillUnitRange.getMinX(), -aFillUnitRange.getMinY());

    // set source pattern transform & activate pattern repeat
    cairo_pattern_set_matrix(sourcepattern, &aMatrix);
    cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_REPEAT);

    // paint
    cairo_paint(mpRT);

    static bool bRenderTransformationBounds(false);
    if (bRenderTransformationBounds)
    {
        cairo_set_source_rgba(mpRT, 0, 1, 0, 0.8);
        impl_cairo_set_hairline(mpRT, getViewInformation2D());
        // full object
        cairo_rectangle(mpRT, 0, 0, 1, 1);
        // outline of pattern root image
        cairo_rectangle(mpRT, aFillUnitRange.getMinX(), aFillUnitRange.getMinY(),
                        aFillUnitRange.getWidth(), aFillUnitRange.getHeight());
        cairo_stroke(mpRT);
    }

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_drawOutputRange(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    cairo_save(mpRT);

    // fill simple rect with outer color
    const basegfx::BColor aColor(
        maBColorModifierStack.getModifiedColor(rFillGradientPrimitive2D.getOuterColor()));
    cairo_set_source_rgb(mpRT, aColor.getRed(), aColor.getGreen(), aColor.getBlue());

    const basegfx::B2DHomMatrix aTrans(getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aTrans.a(), aTrans.b(), aTrans.c(), aTrans.d(), aTrans.e(),
                      aTrans.f());
    cairo_set_matrix(mpRT, &aMatrix);

    const basegfx::B2DRange& rRange(rFillGradientPrimitive2D.getOutputRange());
    cairo_rectangle(mpRT, rRange.getMinX(), rRange.getMinY(), rRange.getWidth(),
                    rRange.getHeight());
    cairo_fill(mpRT);

    cairo_restore(mpRT);
}

bool CairoPixelProcessor2D::processFillGradientPrimitive2D_isCompletelyBordered(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    const double fBorder(rFillGradient.getBorder());

    // check if completely 'bordered out'. This can be the case for all
    // types of gradients
    if (basegfx::fTools::less(fBorder, 1.0) && basegfx::fTools::moreOrEqual(fBorder, 0.0))
    {
        // no, we have visible content besides border
        return false;
    }

    // draw all-covering polygon using getOuterColor and getOutputRange
    processFillGradientPrimitive2D_drawOutputRange(rFillGradientPrimitive2D);
    return true;
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_linear_axial(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D,
    const attribute::FillGradientAttribute* pFillGradientAlpha)
{
    assert(
        (css::awt::GradientStyle_LINEAR == rFillGradientPrimitive2D.getFillGradient().getStyle()
         || css::awt::GradientStyle_AXIAL == rFillGradientPrimitive2D.getFillGradient().getStyle())
        && "SDPRCairo: Helper allows only SPECIFIED types (!)");
    cairo_save(mpRT);

    // need to do 'antique' stuff adaptions for rotate/transitionStart in object coordinates
    // (DefinitionRange) to have the right 'bending' on rotation
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    basegfx::B2DRange aAdaptedRange(rFillGradientPrimitive2D.getDefinitionRange());
    const double fAngle(basegfx::normalizeToRange((2 * M_PI) - rFillGradient.getAngle(), 2 * M_PI));
    const bool bAngle(!basegfx::fTools::equalZero(fAngle));
    const basegfx::B2DPoint aCenter(aAdaptedRange.getCenter());

    // pack rotation and offset into a transformation that coverts that part
    basegfx::B2DHomMatrix aRotation(basegfx::utils::createRotateAroundPoint(aCenter, fAngle));

    // create local transform to work in object coordinates based on OutputRange,
    // combine with rotation - that way we can then just draw into AdaptedRange
    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectToViewTransformation()
                                          * aRotation);
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(),
                      aLocalTransform.d(), aLocalTransform.e(), aLocalTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);

    if (bAngle)
    {
        // expand Range by rotating
        aAdaptedRange.transform(aRotation);
    }

    // create linear pattern in unit coordinates in y-direction
    cairo_pattern_t* pPattern(
        cairo_pattern_create_linear(aAdaptedRange.getCenterX(), aAdaptedRange.getMinY(),
                                    aAdaptedRange.getCenterX(), aAdaptedRange.getMaxY()));

    // get color stops (make copy, might have to be changed)
    basegfx::BColorStops aBColorStops(rFillGradient.getColorStops());
    basegfx::BColorStops aBColorStopsAlpha;
    const bool bHasAlpha(nullptr != pFillGradientAlpha);
    if (bHasAlpha)
        aBColorStopsAlpha = pFillGradientAlpha->getColorStops();
    const bool bAxial(css::awt::GradientStyle_AXIAL == rFillGradient.getStyle());

    // get and apply border - create soace at start in gradient
    const double fBorder(std::max(std::min(rFillGradient.getBorder(), 1.0), 0.0));
    if (!basegfx::fTools::equalZero(fBorder))
    {
        if (bAxial)
        {
            aBColorStops.reverseColorStops();
            if (bHasAlpha)
                aBColorStopsAlpha.reverseColorStops();
        }

        aBColorStops.createSpaceAtStart(fBorder);
        if (bHasAlpha)
            aBColorStopsAlpha.createSpaceAtStart(fBorder);

        if (bAxial)
        {
            aBColorStops.reverseColorStops();
            if (bHasAlpha)
                aBColorStopsAlpha.reverseColorStops();
        }
    }

    if (bAxial)
    {
        // expand with mirrored ColorStops to create axial
        aBColorStops.doApplyAxial();
        if (bHasAlpha)
            aBColorStopsAlpha.doApplyAxial();
    }

    // Apply steps if used to 'emulate' LO's 'discrete step' feature
    if (rFillGradient.getSteps())
    {
        aBColorStops.doApplySteps(rFillGradient.getSteps());
        if (bHasAlpha)
            aBColorStopsAlpha.doApplySteps(rFillGradient.getSteps());
    }

    // add color stops
    for (size_t a(0); a < aBColorStops.size(); a++)
    {
        const basegfx::BColorStop& rStop(aBColorStops[a]);
        const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rStop.getStopColor()));

        if (bHasAlpha)
        {
            const basegfx::BColor aAlpha(aBColorStopsAlpha[a].getStopColor());
            cairo_pattern_add_color_stop_rgba(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                              aColor.getGreen(), aColor.getBlue(),
                                              aAlpha.luminance());
        }
        else
        {
            cairo_pattern_add_color_stop_rgb(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                             aColor.getGreen(), aColor.getBlue());
        }
    }

    // draw OutRange
    basegfx::B2DRange aOutRange(rFillGradientPrimitive2D.getOutputRange());
    if (bAngle)
    {
        // expand backwards to cover all area needed for OutputRange
        aRotation.invert();
        aOutRange.transform(aRotation);
    }
    cairo_rectangle(mpRT, aOutRange.getMinX(), aOutRange.getMinY(), aOutRange.getWidth(),
                    aOutRange.getHeight());
    cairo_set_source(mpRT, pPattern);
    cairo_fill(mpRT);

    // cleanup
    cairo_pattern_destroy(pPattern);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_square_rect(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    assert(
        (css::awt::GradientStyle_SQUARE == rFillGradientPrimitive2D.getFillGradient().getStyle()
         || css::awt::GradientStyle_RECT == rFillGradientPrimitive2D.getFillGradient().getStyle())
        && "SDPRCairo: Helper allows only SPECIFIED types (!)");
    cairo_save(mpRT);

    // draw all-covering polygon using getOuterColor and getOutputRange,
    // the partial paints below will not fill areas outside automatically
    // as happens in the other gradient paints
    processFillGradientPrimitive2D_drawOutputRange(rFillGradientPrimitive2D);

    // get DefinitionRange and adapt if needed
    basegfx::B2DRange aAdaptedRange(rFillGradientPrimitive2D.getDefinitionRange());
    const bool bSquare(css::awt::GradientStyle_SQUARE
                       == rFillGradientPrimitive2D.getFillGradient().getStyle());
    const basegfx::B2DPoint aCenter(aAdaptedRange.getCenter());
    bool bLandscape(false);
    double fSmallRadius(1.0);

    // get rotation and offset values
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    const double fAngle(basegfx::normalizeToRange((2 * M_PI) - rFillGradient.getAngle(), 2 * M_PI));
    const bool bAngle(!basegfx::fTools::equalZero(fAngle));
    const double fOffxsetX(std::max(std::min(rFillGradient.getOffsetX(), 1.0), 0.0));
    const double fOffxsetY(std::max(std::min(rFillGradient.getOffsetY(), 1.0), 0.0));

    if (bSquare)
    {
        // expand to make width == height
        const basegfx::B2DRange& rDefRange(rFillGradientPrimitive2D.getDefinitionRange());

        if (rDefRange.getWidth() > rDefRange.getHeight())
        {
            // landscape -> square
            const double fRadius(0.5 * rDefRange.getWidth());
            aAdaptedRange.expand(basegfx::B2DPoint(rDefRange.getMinX(), aCenter.getY() - fRadius));
            aAdaptedRange.expand(basegfx::B2DPoint(rDefRange.getMaxX(), aCenter.getY() + fRadius));
        }
        else
        {
            // portrait -> square
            const double fRadius(0.5 * rDefRange.getHeight());
            aAdaptedRange.expand(basegfx::B2DPoint(aCenter.getX() - fRadius, rDefRange.getMinY()));
            aAdaptedRange.expand(basegfx::B2DPoint(aCenter.getX() + fRadius, rDefRange.getMaxY()));
        }

        bLandscape = true;
        fSmallRadius = 0.5 * aAdaptedRange.getWidth();
    }
    else
    {
        if (bAngle)
        {
            // expand range using applied rotation
            aAdaptedRange.transform(basegfx::utils::createRotateAroundPoint(aCenter, fAngle));
        }

        // set local params as needed for non-square
        bLandscape = aAdaptedRange.getWidth() > aAdaptedRange.getHeight();
        fSmallRadius = 0.5 * (bLandscape ? aAdaptedRange.getHeight() : aAdaptedRange.getWidth());
    }

    // pack rotation and offset into a combined transformation that covers that parts
    basegfx::B2DHomMatrix aRotAndTranslate;
    aRotAndTranslate.translate(-aCenter.getX(), -aCenter.getY());
    if (bAngle)
        aRotAndTranslate.rotate(fAngle);
    aRotAndTranslate.translate(aAdaptedRange.getMinX() + (fOffxsetX * aAdaptedRange.getWidth()),
                               aAdaptedRange.getMinY() + (fOffxsetY * aAdaptedRange.getHeight()));

    // create local transform to work in object coordinates based on OutputRange,
    // combine with rotation and offset - that way we can then just draw into
    // AdaptedRange
    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectToViewTransformation()
                                          * aRotAndTranslate);
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(),
                      aLocalTransform.d(), aLocalTransform.e(), aLocalTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);

    // get color stops (make copy, might have to be changed)
    basegfx::BColorStops aBColorStops(rFillGradient.getColorStops());

    // apply BColorModifierStack early - the BColorStops are used multiple
    // times below, so do this only once
    if (0 != maBColorModifierStack.count())
    {
        aBColorStops.tryToApplyBColorModifierStack(maBColorModifierStack);
    }

    // get and apply border - create soace at start in gradient
    const double fBorder(std::max(std::min(rFillGradient.getBorder(), 1.0), 0.0));
    if (!basegfx::fTools::equalZero(fBorder))
    {
        aBColorStops.createSpaceAtStart(fBorder);
    }

    // Apply steps if used to 'emulate' LO's 'discrete step' feature
    if (rFillGradient.getSteps())
    {
        aBColorStops.doApplySteps(rFillGradient.getSteps());
    }

    // get half single pixel size to fill touching 'gaps'
    // NOTE: I formally used cairo_device_to_user_distance, but that
    // can indeed create negative sizes if the transformation e.g.
    // contains rotation(s). could use fabs(), but just rely on
    // linear algebra and use the (always positive) length of a vector
    const double fHalfPx((getViewInformation2D().getInverseObjectToViewTransformation()
                          * basegfx::B2DVector(1.0, 0.0))
                             .getLength());

    // draw top part trapez/triangle
    {
        cairo_move_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMinY());
        cairo_line_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMinY());
        cairo_line_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMinY() + fHalfPx);
        if (!bSquare && bLandscape)
        {
            cairo_line_to(mpRT, aAdaptedRange.getMaxX() - fSmallRadius, aCenter.getY() + fHalfPx);
            cairo_line_to(mpRT, aAdaptedRange.getMinX() + fSmallRadius, aCenter.getY() + fHalfPx);
        }
        else
        {
            cairo_line_to(mpRT, aCenter.getX(), aAdaptedRange.getMinY() + fSmallRadius + fHalfPx);
        }
        cairo_line_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMinY() + fHalfPx);
        cairo_close_path(mpRT);

        // create linear pattern in needed coordinates directly
        // NOTE: I *tried* to create in unit coordinates and adapt modifying and re-using
        // cairo_pattern_set_matrix - that *seems* to work but sometimes runs into
        // numerical problems -> probably cairo implementation. So stay safe and do
        // it the easy way, for the cost of re-creating gradient definitions (still cheap)
        cairo_pattern_t* pPattern(cairo_pattern_create_linear(
            aCenter.getX(), aAdaptedRange.getMinY(), aCenter.getX(),
            aAdaptedRange.getMinY()
                + (bLandscape ? aAdaptedRange.getHeight() * 0.5 : fSmallRadius)));
        for (const auto& aStop : aBColorStops)
        {
            const basegfx::BColor& rColor(aStop.getStopColor());
            cairo_pattern_add_color_stop_rgb(pPattern, aStop.getStopOffset(), rColor.getRed(),
                                             rColor.getGreen(), rColor.getBlue());
        }

        cairo_set_source(mpRT, pPattern);
        cairo_fill(mpRT);
        cairo_pattern_destroy(pPattern);
    }

    {
        // draw right part trapez/triangle
        cairo_move_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMinY());
        cairo_line_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMaxY());
        if (bSquare || bLandscape)
        {
            cairo_line_to(mpRT, aAdaptedRange.getMaxX() - fSmallRadius - fHalfPx, aCenter.getY());
        }
        else
        {
            cairo_line_to(mpRT, aCenter.getX() - fHalfPx, aAdaptedRange.getMaxY() - fSmallRadius);
            cairo_line_to(mpRT, aCenter.getX() - fHalfPx, aAdaptedRange.getMinY() + fSmallRadius);
        }
        cairo_close_path(mpRT);

        // create linear pattern in needed coordinates directly
        cairo_pattern_t* pPattern(cairo_pattern_create_linear(
            aAdaptedRange.getMaxX(), aCenter.getY(),
            aAdaptedRange.getMaxX() - (bLandscape ? fSmallRadius : aAdaptedRange.getWidth() * 0.5),
            aCenter.getY()));
        for (const auto& aStop : aBColorStops)
        {
            const basegfx::BColor& rColor(aStop.getStopColor());
            cairo_pattern_add_color_stop_rgb(pPattern, aStop.getStopOffset(), rColor.getRed(),
                                             rColor.getGreen(), rColor.getBlue());
        }

        cairo_set_source(mpRT, pPattern);
        cairo_fill(mpRT);
        cairo_pattern_destroy(pPattern);
    }

    {
        // draw bottom part trapez/triangle
        cairo_move_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMaxY());
        cairo_line_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMaxY());
        cairo_line_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMaxY() - fHalfPx);
        if (!bSquare && bLandscape)
        {
            cairo_line_to(mpRT, aAdaptedRange.getMinX() + fSmallRadius, aCenter.getY() - fHalfPx);
            cairo_line_to(mpRT, aAdaptedRange.getMaxX() - fSmallRadius, aCenter.getY() - fHalfPx);
        }
        else
        {
            cairo_line_to(mpRT, aCenter.getX(), aAdaptedRange.getMaxY() - fSmallRadius - fHalfPx);
        }
        cairo_line_to(mpRT, aAdaptedRange.getMaxX(), aAdaptedRange.getMaxY() - fHalfPx);
        cairo_close_path(mpRT);

        // create linear pattern in needed coordinates directly
        cairo_pattern_t* pPattern(cairo_pattern_create_linear(
            aCenter.getX(), aAdaptedRange.getMaxY(), aCenter.getX(),
            aAdaptedRange.getMaxY()
                - (bLandscape ? aAdaptedRange.getHeight() * 0.5 : fSmallRadius)));
        for (const auto& aStop : aBColorStops)
        {
            const basegfx::BColor& rColor(aStop.getStopColor());
            cairo_pattern_add_color_stop_rgb(pPattern, aStop.getStopOffset(), rColor.getRed(),
                                             rColor.getGreen(), rColor.getBlue());
        }

        cairo_set_source(mpRT, pPattern);
        cairo_fill(mpRT);
        cairo_pattern_destroy(pPattern);
    }

    {
        // draw left part trapez/triangle
        cairo_move_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMaxY());
        cairo_line_to(mpRT, aAdaptedRange.getMinX(), aAdaptedRange.getMinY());
        if (bSquare || bLandscape)
        {
            cairo_line_to(mpRT, aAdaptedRange.getMinX() + fSmallRadius + fHalfPx, aCenter.getY());
        }
        else
        {
            cairo_line_to(mpRT, aCenter.getX() + fHalfPx, aAdaptedRange.getMinY() + fSmallRadius);
            cairo_line_to(mpRT, aCenter.getX() + fHalfPx, aAdaptedRange.getMaxY() - fSmallRadius);
        }
        cairo_close_path(mpRT);

        // create linear pattern in needed coordinates directly
        cairo_pattern_t* pPattern(cairo_pattern_create_linear(
            aAdaptedRange.getMinX(), aCenter.getY(),
            aAdaptedRange.getMinX() + (bLandscape ? fSmallRadius : aAdaptedRange.getWidth() * 0.5),
            aCenter.getY()));
        for (const auto& aStop : aBColorStops)
        {
            const basegfx::BColor& rColor(aStop.getStopColor());
            cairo_pattern_add_color_stop_rgb(pPattern, aStop.getStopOffset(), rColor.getRed(),
                                             rColor.getGreen(), rColor.getBlue());
        }

        cairo_set_source(mpRT, pPattern);
        cairo_fill(mpRT);
        cairo_pattern_destroy(pPattern);
    }

    // cleanup
    // cairo_pattern_destroy(pPattern);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_radial_elliptical(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D,
    const attribute::FillGradientAttribute* pFillGradientAlpha)
{
    assert((css::awt::GradientStyle_RADIAL == rFillGradientPrimitive2D.getFillGradient().getStyle()
            || css::awt::GradientStyle_ELLIPTICAL
                   == rFillGradientPrimitive2D.getFillGradient().getStyle())
           && "SDPRCairo: Helper allows only SPECIFIED types (!)");
    cairo_save(mpRT);

    // need to do 'antique' stuff adaptions for rotate/transitionStart in object coordinates
    // (DefinitionRange) to have the right 'bending' on rotation
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    const basegfx::B2DRange rDefRange(rFillGradientPrimitive2D.getDefinitionRange());
    const basegfx::B2DPoint aCenter(rDefRange.getCenter());
    double fRadius(1.0);
    double fRatioElliptical(1.0);
    const bool bRadial(css::awt::GradientStyle_RADIAL == rFillGradient.getStyle());

    // use what is done in initEllipticalGradientInfo method to get as close as
    // possible to former stuff, expand AdaptedRange as needed
    if (bRadial)
    {
        const double fHalfOriginalDiag(std::hypot(rDefRange.getWidth(), rDefRange.getHeight())
                                       * 0.5);
        fRadius = fHalfOriginalDiag;
    }
    else
    {
        double fTargetSizeX(M_SQRT2 * rDefRange.getWidth());
        double fTargetSizeY(M_SQRT2 * rDefRange.getHeight());
        fRatioElliptical = fTargetSizeX / fTargetSizeY;
        fRadius = std::max(fTargetSizeX, fTargetSizeY) * 0.5;
    }

    // get rotation and offset values
    const double fAngle(basegfx::normalizeToRange((2 * M_PI) - rFillGradient.getAngle(), 2 * M_PI));
    const bool bAngle(!basegfx::fTools::equalZero(fAngle));
    const double fOffxsetX(std::max(std::min(rFillGradient.getOffsetX(), 1.0), 0.0));
    const double fOffxsetY(std::max(std::min(rFillGradient.getOffsetY(), 1.0), 0.0));

    // pack rotation and offset into a combined transformation that coverts that parts
    basegfx::B2DHomMatrix aRotAndTranslate;
    aRotAndTranslate.translate(-aCenter.getX(), -aCenter.getY());
    if (bAngle)
        aRotAndTranslate.rotate(fAngle);
    aRotAndTranslate.translate(rDefRange.getMinX() + (fOffxsetX * rDefRange.getWidth()),
                               rDefRange.getMinY() + (fOffxsetY * rDefRange.getHeight()));

    // create local transform to work in object coordinates based on OutputRange,
    // combine with rotation and offset - that way we can then just draw into
    // AdaptedRange
    basegfx::B2DHomMatrix aLocalTransform(getViewInformation2D().getObjectToViewTransformation()
                                          * aRotAndTranslate);
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(),
                      aLocalTransform.d(), aLocalTransform.e(), aLocalTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);

    // create linear pattern in unit coordinates in y-direction
    cairo_pattern_t* pPattern(cairo_pattern_create_radial(aCenter.getX(), aCenter.getY(), fRadius,
                                                          aCenter.getX(), aCenter.getY(), 0.0));

    // get color stops (make copy, might have to be changed)
    basegfx::BColorStops aBColorStops(rFillGradient.getColorStops());
    basegfx::BColorStops aBColorStopsAlpha;
    const bool bHasAlpha(nullptr != pFillGradientAlpha);
    if (bHasAlpha)
        aBColorStopsAlpha = pFillGradientAlpha->getColorStops();

    // get and apply border - create soace at start in gradient
    const double fBorder(std::max(std::min(rFillGradient.getBorder(), 1.0), 0.0));
    if (!basegfx::fTools::equalZero(fBorder))
    {
        aBColorStops.createSpaceAtStart(fBorder);
        if (bHasAlpha)
            aBColorStopsAlpha.createSpaceAtStart(fBorder);
    }

    // Apply steps if used to 'emulate' LO's 'discrete step' feature
    if (rFillGradient.getSteps())
    {
        aBColorStops.doApplySteps(rFillGradient.getSteps());
        if (bHasAlpha)
            aBColorStopsAlpha.doApplySteps(rFillGradient.getSteps());
    }

    // add color stops
    for (size_t a(0); a < aBColorStops.size(); a++)
    {
        const basegfx::BColorStop& rStop(aBColorStops[a]);
        const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rStop.getStopColor()));

        if (bHasAlpha)
        {
            const basegfx::BColor aAlpha(aBColorStopsAlpha[a].getStopColor());
            cairo_pattern_add_color_stop_rgba(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                              aColor.getGreen(), aColor.getBlue(),
                                              aAlpha.luminance());
        }
        else
        {
            cairo_pattern_add_color_stop_rgb(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                             aColor.getGreen(), aColor.getBlue());
        }
    }

    cairo_set_source(mpRT, pPattern);

    if (!bRadial) // css::awt::GradientStyle_ELLIPTICAL
    {
        // set cairo matrix at cairo_pattern_t to get needed ratio scale done.
        // this is necessary since cairo_pattern_create_radial does *not*
        // support ellipse resp. radial gradient with non-equidistant
        // ratio directly
        // this uses the transformation 'from user space to pattern space' as
        // cairo docu states. That is the inverse of the intuitive thought
        // model: describe from coordinates in texture, so use B2DHomMatrix
        // and invert at the end to have better control about what has to happen
        basegfx::B2DHomMatrix aTrans;

        // move center to origin to prepare scale/rotate
        aTrans.translate(-aCenter.getX(), -aCenter.getY());

        // get scale factor and apply as needed
        if (fRatioElliptical > 1.0)
            aTrans.scale(1.0, 1.0 / fRatioElliptical);
        else
            aTrans.scale(fRatioElliptical, 1.0);

        // move transformed stuff back to center
        aTrans.translate(aCenter.getX(), aCenter.getY());

        // invert and set at cairo_pattern_t
        aTrans.invert();
        cairo_matrix_init(&aMatrix, aTrans.a(), aTrans.b(), aTrans.c(), aTrans.d(), aTrans.e(),
                          aTrans.f());
        cairo_pattern_set_matrix(pPattern, &aMatrix);
    }

    // draw OutRange. Due to rot and translate being part of the
    // set transform in cairo we need to back-transform (and expand
    // as needed) the OutputRange to paint at the right place and
    // get all OutputRange covered
    basegfx::B2DRange aOutRange(rFillGradientPrimitive2D.getOutputRange());
    aRotAndTranslate.invert();
    aOutRange.transform(aRotAndTranslate);
    cairo_rectangle(mpRT, aOutRange.getMinX(), aOutRange.getMinY(), aOutRange.getWidth(),
                    aOutRange.getHeight());
    cairo_fill(mpRT);

    // cleanup
    cairo_pattern_destroy(pPattern);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_fallback_decompose(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    // this helper draws the given gradient using the decompose fallback,
    // maybe needed in some cases an can/will be handy
    cairo_save(mpRT);

    // draw all-covering initial BG polygon 1st using getOuterColor and getOutputRange
    processFillGradientPrimitive2D_drawOutputRange(rFillGradientPrimitive2D);

    // bet basic form in unit coordinates
    CairoPathHelper aForm(rFillGradientPrimitive2D.getUnitPolygon());

    // paint solid fill steps by providing callback as lambda
    auto aCallback([this, &aForm](const basegfx::B2DHomMatrix& rMatrix,
                                  const basegfx::BColor& rColor) {
        const basegfx::B2DHomMatrix aTrans(getViewInformation2D().getObjectToViewTransformation()
                                           * rMatrix);
        cairo_matrix_t aMatrix;
        cairo_matrix_init(&aMatrix, aTrans.a(), aTrans.b(), aTrans.c(), aTrans.d(), aTrans.e(),
                          aTrans.f());
        cairo_set_matrix(mpRT, &aMatrix);

        const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rColor));
        cairo_set_source_rgb(mpRT, aColor.getRed(), aColor.getGreen(), aColor.getBlue());

        cairo_append_path(mpRT, aForm.getCairoPath());

        cairo_fill(mpRT);
    });

    // call value generator to trigger callbacks
    rFillGradientPrimitive2D.generateMatricesAndColors(aCallback);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D,
    const attribute::FillGradientAttribute* pFillGradientAlpha)
{
    if (rFillGradientPrimitive2D.getDefinitionRange().isEmpty())
    {
        // no definition area, done
        return;
    }

    if (rFillGradientPrimitive2D.getOutputRange().isEmpty())
    {
        // no output area, done
        return;
    }

    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());

    if (rFillGradient.isDefault())
    {
        // no gradient definition, done
        return;
    }

    // check if completely 'bordered out'
    if (processFillGradientPrimitive2D_isCompletelyBordered(rFillGradientPrimitive2D))
    {
        // yes, done, was processed as single filled rectangle (using getOuterColor())
        return;
    }

    // evtl. prefer fallback: cairo does *not* render hard color transitions
    // in gradients anti-aliased which is most visible in 'step'ed gradients,
    // but may also happen in normal ones -> may need to be checked in
    // basegfx::BColorStops (as tooling, like isSymmetrical() or similar).
    // due to the nature of 'step'ing this also means a low number of
    // filled polygons to be drawn (no 'smooth' parts to be replicated),
    // so this is not runtime burner by definition.
    // Making this configurable using static bool, may be moved to settings
    // somewhere later. Do not forget to deactivate when working on 'step'ping
    // stuff in the other helpers (!)
    static bool bPreferAntiAliasedHardColorTransitions(true);

    if (bPreferAntiAliasedHardColorTransitions && rFillGradient.getSteps())
    {
        processFillGradientPrimitive2D_fallback_decompose(rFillGradientPrimitive2D);
        return;
    }

    // for dfirect RGBA gradient render support: assert when the definition
    // is not allowed, it HAS to fulfil the requested preconditions. Note that
    // the form to call this function using nullptr != pFillGradientAlpha is
    // only allowed locally in CairoPixelProcessor2D::processMaskPrimitive2D
    assert(nullptr == pFillGradientAlpha
           || rFillGradient.sameDefinitionThanAlpha(*pFillGradientAlpha));

    switch (rFillGradient.getStyle())
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
        {
            // use specialized renderer for this cases - linear, axial
            processFillGradientPrimitive2D_linear_axial(rFillGradientPrimitive2D,
                                                        pFillGradientAlpha);
            return;
        }
        case css::awt::GradientStyle_RADIAL:
        case css::awt::GradientStyle_ELLIPTICAL:
        {
            // use specialized renderer for this cases - radial, elliptical

            //  NOTE for css::awt::GradientStyle_ELLIPTICAL:
            // The first time ever I will accept slight deviations for the
            // elliptical case here due to it's old chaotic move-two-pixels inside
            // rendering method that cannot be patched into a lineartransformation
            // and is hard/difficult to support in more modern systems. Differences
            // are small and mostly would be visible *if* in steps-mode what is
            // also rare. IF that should make problems reactivation of that case
            // for the default case below is possible. main reason is that speed
            // for direct rendering in cairo is much better.
            processFillGradientPrimitive2D_radial_elliptical(rFillGradientPrimitive2D,
                                                             pFillGradientAlpha);
            return;
        }
        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
        {
            // use specialized renderer for this cases - square, rect
            // NOTE: *NO* support for FillGradientAlpha here. it is anyways
            // hard to map these to direct rendering, but to do so the four
            // trapezoids/sides are 'stitched' together, so painting RGBA
            // directly will make the overlaps look bad and like errors.
            // Anyways, these gradient types are only our internal heritage
            // and rendering them directly is already much faster, will be okay.
            processFillGradientPrimitive2D_square_rect(rFillGradientPrimitive2D);
            return;
        }
        default:
        {
            // NOTE: All cases are covered above, but keep this as fallback,
            // so it is possible anytime to exclude one of the cases above again
            // and go back to decomposed version - just in case...
            processFillGradientPrimitive2D_fallback_decompose(rFillGradientPrimitive2D);
            break;
        }
    }
}

void CairoPixelProcessor2D::processPolyPolygonRGBAGradientPrimitive2D(
    const primitive2d::PolyPolygonRGBAGradientPrimitive2D& rPolyPolygonRGBAGradientPrimitive2D)
{
    const attribute::FillGradientAttribute& rFill(
        rPolyPolygonRGBAGradientPrimitive2D.getFillGradient());
    const attribute::FillGradientAttribute& rAlpha(
        rPolyPolygonRGBAGradientPrimitive2D.getFillGradientAlpha());

    if (rFill.isDefault())
    {
        // no gradient definition, done
        return;
    }

    // assert when the definition is not allowed, it HAS to fulfil the
    // requested preconditions
    assert(rFill.sameDefinitionThanAlpha(rAlpha));

    // the gradient still needs to be masked to getB2DPolyPolygon() at the
    // primitive, see PolyPolygonGradientPrimitive2D::create2DDecomposition.
    // we could repeat here the code inside localprocessMaskPrimitive2D, but
    // it is easier to just locally temporarily create the needed data structure
    // and hand over the needed extra-data
    const basegfx::B2DRange aPolyPolygonRange(
        rPolyPolygonRGBAGradientPrimitive2D.getB2DPolyPolygon().getB2DRange());
    primitive2d::FillGradientPrimitive2D* pFillGradientPrimitive2D(
        new primitive2d::FillGradientPrimitive2D(
            aPolyPolygonRange, rPolyPolygonRGBAGradientPrimitive2D.getDefinitionRange(), rFill));
    primitive2d::Primitive2DContainer aContent{ pFillGradientPrimitive2D };

    // NOTE: I had this like
    //   const primitive2d::MaskPrimitive2D aMaskPrimitive2D(
    //       rPolyPolygonRGBAGradientPrimitive2D.getB2DPolyPolygon(), std::move(aContent));
    // but I got
    //   error: salhelper::SimpleReferenceObject subclass being directly stack managed, should
    //   be managed via rtl::Reference, const primitive2d::MaskPrimitive2D [loplugin:refcounting]
    // thus I have *no choice* and have to use the heap here (?) It is no problem to use the stack
    // and I wanted to do this here by purpose... sigh
    primitive2d::MaskPrimitive2D* pMaskPrimitive2D(new primitive2d::MaskPrimitive2D(
        rPolyPolygonRGBAGradientPrimitive2D.getB2DPolyPolygon(), std::move(aContent)));
    primitive2d::Primitive2DContainer aMask{ pMaskPrimitive2D };
    (void)aMask;

    // render masked RGBA gradient
    processMaskPrimitive2D(*pMaskPrimitive2D, pFillGradientPrimitive2D, &rAlpha);
}

void CairoPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    switch (rCandidate.getPrimitive2DID())
    {
        // geometry that *has* to be processed
        case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D:
        {
            processBitmapPrimitive2D(
                static_cast<const primitive2d::BitmapPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D:
        {
            processPointArrayPrimitive2D(
                static_cast<const primitive2d::PointArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D:
        {
            processPolygonHairlinePrimitive2D(
                static_cast<const primitive2d::PolygonHairlinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D:
        {
            processPolyPolygonColorPrimitive2D(
                static_cast<const primitive2d::PolyPolygonColorPrimitive2D&>(rCandidate));
            break;
        }
        // embedding/groups that *have* to be processed
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            processTransparencePrimitive2D(
                static_cast<const primitive2d::TransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
        {
            processInvertPrimitive2D(
                static_cast<const primitive2d::InvertPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            processMaskPrimitive2D(static_cast<const primitive2d::MaskPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D:
        {
            processModifiedColorPrimitive2D(
                static_cast<const primitive2d::ModifiedColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D:
        {
            processTransformPrimitive2D(
                static_cast<const primitive2d::TransformPrimitive2D&>(rCandidate));
            break;
        }
        // geometry that *may* be processed due to being able to do it better
        // then using the decomposition
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            processUnifiedTransparencePrimitive2D(
                static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        {
            processMarkerArrayPrimitive2D(
                static_cast<const primitive2d::MarkerArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            processBackgroundColorPrimitive2D(
                static_cast<const primitive2d::BackgroundColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            processPolygonStrokePrimitive2D(
                static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
        {
            processLineRectanglePrimitive2D(
                static_cast<const primitive2d::LineRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
        {
            processFilledRectanglePrimitive2D(
                static_cast<const primitive2d::FilledRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
        {
            processSingleLinePrimitive2D(
                static_cast<const primitive2d::SingleLinePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLGRAPHICPRIMITIVE2D:
        {
            processFillGraphicPrimitive2D(
                static_cast<const primitive2d::FillGraphicPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D:
        {
            processFillGradientPrimitive2D(
                static_cast<const primitive2d::FillGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONRGBAGRADIENTPRIMITIVE2D:
        {
            processPolyPolygonRGBAGradientPrimitive2D(
                static_cast<const primitive2d::PolyPolygonRGBAGradientPrimitive2D&>(rCandidate));
            break;
        }

        // continue with decompose
        default:
        {
            SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(
                                         rCandidate.getPrimitive2DID()));
            // process recursively
            process(rCandidate);
            break;
        }
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
