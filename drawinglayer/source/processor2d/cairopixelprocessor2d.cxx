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
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/alpha.hxx>
#include <vcl/cairo.hxx>
#include <vcl/CairoFormats.hxx>
#include <vcl/outdev.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
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
#include <drawinglayer/primitive2d/PolyPolygonRGBAPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonAlphaGradientPrimitive2D.hxx>
#include <drawinglayer/primitive2d/BitmapAlphaPrimitive2D.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/controlprimitive2d.hxx>
#include <drawinglayer/converters.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <basegfx/utils/bgradient.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/vcllayout.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/awt/XView.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <unordered_map>
#include <dlfcn.h>

using namespace com::sun::star;

namespace
{
void impl_cairo_set_hairline(cairo_t* pRT,
                             const drawinglayer::geometry::ViewInformation2D& rViewInformation,
                             bool bCairoCoordinateLimitWorkaroundActive)
{
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 18, 0)
    void* addr(dlsym(nullptr, "cairo_set_hairline"));
    if (nullptr != addr)
    {
        cairo_set_hairline(pRT, true);
        return;
    }
#endif
    if (bCairoCoordinateLimitWorkaroundActive)
    {
        // we have to render in view coordinates, set line width to 1.0
        cairo_set_line_width(pRT, 1.0);
    }
    else
    {
        // avoid cairo_device_to_user_distance, see note on that below
        const double fPx(
            (rViewInformation.getInverseObjectToViewTransformation() * basegfx::B2DVector(1.0, 0.0))
                .getLength());
        cairo_set_line_width(pRT, fPx);
    }
}

void addB2DPolygonToPathGeometry(cairo_t* pRT, const basegfx::B2DPolygon& rPolygon)
{
    const sal_uInt32 nPointCount(rPolygon.count());

    if (0 == nPointCount)
        // no points, done
        return;

    // get basic infos
    const bool bClosed(rPolygon.isClosed());
    const sal_uInt32 nEdgeCount(bClosed ? nPointCount : nPointCount - 1);

    // get 1st point and move to it
    basegfx::B2DPoint aCurrent(rPolygon.getB2DPoint(0));
    cairo_move_to(pRT, aCurrent.getX(), aCurrent.getY());

    for (sal_uInt32 nIndex(0); nIndex < nEdgeCount; nIndex++)
    {
        // get index for and next point
        const sal_uInt32 nNextIndex((nIndex + 1) % nPointCount);
        const basegfx::B2DPoint aNext(rPolygon.getB2DPoint(nNextIndex));

        // get and check curve stuff
        basegfx::B2DPoint aCP1(rPolygon.getNextControlPoint(nIndex));
        basegfx::B2DPoint aCP2(rPolygon.getPrevControlPoint(nNextIndex));
        const bool bCP1Equal(aCP1.equal(aCurrent));
        const bool bCP2Equal(aCP2.equal(aNext));

        if (!bCP1Equal || !bCP2Equal)
        {
            // tdf#99165, see other similar changes for more info
            if (bCP1Equal)
                aCP1 = aCurrent + ((aCP2 - aCurrent) * 0.0005);

            if (bCP2Equal)
                aCP2 = aNext + ((aCP1 - aNext) * 0.0005);

            cairo_curve_to(pRT, aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(), aNext.getX(),
                           aNext.getY());
        }
        else
        {
            cairo_line_to(pRT, aNext.getX(), aNext.getY());
        }

        // prepare next step
        aCurrent = aNext;
    }

    if (bClosed)
        cairo_close_path(pRT);
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
    }

    CairoPathHelper(const basegfx::B2DPolyPolygon& rPolyPolygon)
        : mpCairoPath(nullptr)
    {
        cairo_new_path(globalStaticCairoContext.getContext());
        for (const auto& rPolygon : rPolyPolygon)
            addB2DPolygonToPathGeometry(globalStaticCairoContext.getContext(), rPolygon);
        mpCairoPath = cairo_copy_path(globalStaticCairoContext.getContext());
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
        // no pixel snap, done
        return;

    // with the comments above at CairoPathHelper we cannot do PixelSnap
    // at path construction time, so it needs to be done *after* the path
    // data is added to the cairo context. Advantage is that all general
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

    auto doPixelSnap([&pRT](double& rX, double& rY) {
        // transform to discrete pixels
        cairo_user_to_device(pRT, &rX, &rY);

        // round them, also add 0.5 which will be as transform in
        // the paint method to move to 'inside' pixels when AA used.
        // remember: this is only done when AA is active (see bPixelSnap
        // above) and moves the hairline to full-pixel position
        rX = trunc(rX) + 0.5;
        rY = trunc(rY) + 0.5;

        // transform back to former transformed state
        cairo_device_to_user(pRT, &rX, &rY);
    });

    for (int a(0); a < path->num_data; a += path->data[a].header.length)
    {
        cairo_path_data_t* data(&path->data[a]);

        switch (data->header.type)
        {
            case CAIRO_PATH_CURVE_TO:
            {
                // curve: snap all three point positions,
                // thus use fallthrough below
                doPixelSnap(data[2].point.x, data[2].point.y);
                doPixelSnap(data[3].point.x, data[3].point.y);
                [[fallthrough]]; // break;
            }
            case CAIRO_PATH_MOVE_TO:
            case CAIRO_PATH_LINE_TO:
            {
                // path/move: snap first point position
                doPixelSnap(data[1].point.x, data[1].point.y);
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
                             const drawinglayer::geometry::ViewInformation2D& rViewInformation,
                             bool bPixelSnap)
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
        if (bPixelSnap)
            checkAndDoPixelSnap(pRT, rViewInformation);
        return;
    }

    // create new data and add path data to pRT and do evtl. needed pixel snap after adding on cairo path data
    std::shared_ptr<CairoPathHelper> pCairoPathHelper(std::make_shared<CairoPathHelper>(rPolygon));
    cairo_append_path(pRT, pCairoPathHelper->getCairoPath());
    if (bPixelSnap)
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
        if (cairo_surface_status(mpCairoSurface) != CAIRO_STATUS_SUCCESS)
        {
            SAL_WARN("drawinglayer",
                     "cairo_image_surface_create failed for: " << nWidth << " x " << nHeight);
            return;
        }
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

                pPixelData[SVP_CAIRO_RED] = vcl::bitmap::premultiply(aColor.GetRed(), nAlpha);
                pPixelData[SVP_CAIRO_GREEN] = vcl::bitmap::premultiply(aColor.GetGreen(), nAlpha);
                pPixelData[SVP_CAIRO_BLUE] = vcl::bitmap::premultiply(aColor.GetBlue(), nAlpha);
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
        if (cairo_surface_status(mpCairoSurface) != CAIRO_STATUS_SUCCESS)
        {
            SAL_WARN("drawinglayer",
                     "cairo_image_surface_create failed for: " << nWidth << " x " << nHeight);
            return;
        }
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
        if (cairo_surface_status(mpCairoSurface) != CAIRO_STATUS_SUCCESS)
        {
            SAL_WARN("drawinglayer",
                     "cairo_image_surface_create failed for: " << nWidth << " x " << nHeight);
            return;
        }
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

    // change to unsigned 16bit and shifting. This is not much
    // faster on modern processors due to nowadays good double/
    // float HW, but may also be used on smaller HW (ARM, ...).
    // Since source is sal_uInt8 integer using double (see version
    // before) is not required numerically either.
    // scaling values are now put to a 256 entry lookup for R, G and B
    // thus 768 bytes, so no multiplications have to happen. The values
    // used to create these are (54+183+18 == 255):
    //    sal_uInt16 nR(0.2125 * 256.0); // -> 54.4
    //    sal_uInt16 nG(0.7154 * 256.0); // -> 183.1424
    //    sal_uInt16 nB(0.0721 * 256.0); // -> 18.4576
    // and the short loop (for nR, nG and nB resp.) like:
    //    for(unsigned short a(0); a < 256; a++)
    //        std::cout << ((a * nR) / 255) << ", ";
    static constexpr std::array<sal_uInt8, 256> nRArray
        = { 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,
            4,  4,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  8,  9,
            9,  9,  9,  9,  10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13,
            13, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18,
            18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 22, 23,
            23, 23, 23, 23, 24, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 26, 27, 27, 27, 27,
            27, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32,
            32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 36, 37,
            37, 37, 37, 37, 38, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41,
            41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 46, 46,
            46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 49, 50, 50, 50, 50, 51,
            51, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 54 };
    static constexpr std::array<sal_uInt8, 256> nGArray
        = { 0,   0,   1,   2,   2,   3,   4,   5,   5,   6,   7,   7,   8,   9,   10,  10,
            11,  12,  12,  13,  14,  15,  15,  16,  17,  17,  18,  19,  20,  20,  21,  22,
            22,  23,  24,  25,  25,  26,  27,  27,  28,  29,  30,  30,  31,  32,  33,  33,
            34,  35,  35,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,
            45,  46,  47,  48,  48,  49,  50,  50,  51,  52,  53,  53,  54,  55,  55,  56,
            57,  58,  58,  59,  60,  61,  61,  62,  63,  63,  64,  65,  66,  66,  67,  68,
            68,  69,  70,  71,  71,  72,  73,  73,  74,  75,  76,  76,  77,  78,  78,  79,
            80,  81,  81,  82,  83,  83,  84,  85,  86,  86,  87,  88,  88,  89,  90,  91,
            91,  92,  93,  94,  94,  95,  96,  96,  97,  98,  99,  99,  100, 101, 101, 102,
            103, 104, 104, 105, 106, 106, 107, 108, 109, 109, 110, 111, 111, 112, 113, 114,
            114, 115, 116, 116, 117, 118, 119, 119, 120, 121, 122, 122, 123, 124, 124, 125,
            126, 127, 127, 128, 129, 129, 130, 131, 132, 132, 133, 134, 134, 135, 136, 137,
            137, 138, 139, 139, 140, 141, 142, 142, 143, 144, 144, 145, 146, 147, 147, 148,
            149, 149, 150, 151, 152, 152, 153, 154, 155, 155, 156, 157, 157, 158, 159, 160,
            160, 161, 162, 162, 163, 164, 165, 165, 166, 167, 167, 168, 169, 170, 170, 171,
            172, 172, 173, 174, 175, 175, 176, 177, 177, 178, 179, 180, 180, 181, 182, 183 };
    static constexpr std::array<sal_uInt8, 256> nBArray
        = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,
            1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,
            3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,
            4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,
            6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
            7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,
            9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
            10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12,
            12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
            13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17,
            17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18 };

    for (sal_uInt32 y(0); y < nHeight; ++y)
    {
        unsigned char* pMaskPixelData = mask_surface_data + (nStride * y);

        for (sal_uInt32 x(0); x < nWidth; ++x)
        {
            // do not forget that we have pre-multiplied alpha
            sal_uInt8 nAlpha(pMaskPixelData[SVP_CAIRO_ALPHA]);

            if (0 != nAlpha)
            {
                // get Luminance in range [0..255]
                const sal_uInt8 nLum(nRArray[pMaskPixelData[SVP_CAIRO_RED]]
                                     + nGArray[pMaskPixelData[SVP_CAIRO_GREEN]]
                                     + nBArray[pMaskPixelData[SVP_CAIRO_BLUE]]);

                if (255 != nAlpha)
                    // remove pre-multiplied alpha (use existing VCL tooling)
                    nAlpha = vcl::bitmap::unpremultiply(nLum, nAlpha);
                else
                    // already what we need
                    nAlpha = nLum;

                pMaskPixelData[SVP_CAIRO_ALPHA] = 255 - nAlpha;
            }

            pMaskPixelData += 4;
        }
    }

    cairo_surface_mark_dirty(pMask);
}

basegfx::B2DRange getDiscreteViewRange(cairo_t* pRT)
{
    double clip_x1, clip_x2, clip_y1, clip_y2;
    cairo_save(pRT);
    cairo_identity_matrix(pRT);
    cairo_clip_extents(pRT, &clip_x1, &clip_y1, &clip_x2, &clip_y2);
    cairo_restore(pRT);

    return basegfx::B2DRange(basegfx::B2DPoint(clip_x1, clip_y1),
                             basegfx::B2DPoint(clip_x2, clip_y2));
}

bool checkCoordinateLimitWorkaroundNeededForUsedCairo()
{
    // setup surface and render context
    cairo_surface_t* pSurface(cairo_image_surface_create(CAIRO_FORMAT_RGB24, 8, 8));
    if (!pSurface)
    {
        SAL_INFO(
            "drawinglayer",
            "checkCoordinateLimitWorkaroundNeededForUsedCairo: got no surface -> be pessimistic");
        return true;
    }

    cairo_t* pRender(cairo_create(pSurface));
    if (!pRender)
    {
        SAL_INFO(
            "drawinglayer",
            "checkCoordinateLimitWorkaroundNeededForUsedCairo: got no render -> be pessimistic");
        cairo_surface_destroy(pSurface);
        return true;
    }

    // set basic values
    cairo_set_antialias(pRender, CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule(pRender, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(pRender, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb(pRender, 1.0, 0.0, 0.0);

    // create a to-be rendered area centered at the fNumCairoMax
    // spot and 8x8 discrete units in size
    constexpr double fNumCairoMax(1 << 23);
    const basegfx::B2DPoint aCenter(fNumCairoMax, fNumCairoMax);
    const basegfx::B2DPoint aOffset(4, 4);
    const basegfx::B2DRange aObject(aCenter - aOffset, aCenter + aOffset);

    // create transformation to render that to an area with
    // range(0, 0, 8, 8) and set as transformation
    const basegfx::B2DHomMatrix aObjectToView(basegfx::utils::createSourceRangeTargetRangeTransform(
        aObject, basegfx::B2DRange(0, 0, 8, 8)));
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aObjectToView.a(), aObjectToView.b(), aObjectToView.c(),
                      aObjectToView.d(), aObjectToView.e(), aObjectToView.f());
    cairo_set_matrix(pRender, &aMatrix);

    // get/create the path for an object exactly filling that area
    cairo_new_path(pRender);
    basegfx::B2DPolyPolygon aObjectPolygon(basegfx::utils::createPolygonFromRect(aObject));
    CairoPathHelper aPathHelper(aObjectPolygon);
    cairo_append_path(pRender, aPathHelper.getCairoPath());

    // render it and flush since we want to immediately inspect result
    cairo_fill(pRender);
    cairo_surface_flush(pSurface);

    // get access to pixel data
    const sal_uInt32 nStride(cairo_image_surface_get_stride(pSurface));
    sal_uInt8* pStartPixelData(cairo_image_surface_get_data(pSurface));

    // extract red value for pixels at (1,1) and (7,7)
    sal_uInt8 aRedAt_1_1((pStartPixelData + (nStride * 1) + 1)[SVP_CAIRO_RED]);
    sal_uInt8 aRedAt_6_6((pStartPixelData + (nStride * 6) + 6)[SVP_CAIRO_RED]);

    // cleanup
    cairo_destroy(pRender);
    cairo_surface_destroy(pSurface);

    // if cairo works or has no 24.8 internal format all pixels
    // have to be red (255), thus workaround is needed if !=
    auto const needed = aRedAt_1_1 != aRedAt_6_6;
    SAL_INFO("drawinglayer", "checkCoordinateLimitWorkaroundNeededForUsedCairo: " << needed);
    return needed;
}
}

namespace drawinglayer::processor2d
{
CairoPixelProcessor2D::CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                             cairo_surface_t* pTarget)
    : BaseProcessor2D(rViewInformation)
    , mpTargetOutputDevice(nullptr)
    , maBColorModifierStack()
    , mpOwnedSurface(nullptr)
    , mpRT(nullptr)
    , mbRenderSimpleTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderSimpleTextDirect::get())
    , mbRenderDecoratedTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderDecoratedTextDirect::get())
    , mnClipRecursionCount(0)
    , mbCairoCoordinateLimitWorkaroundActive(false)
{
    // no target, nothing to initialize
    if (nullptr == pTarget)
        return;

    // create RenderTarget for full target
    mpRT = cairo_create(pTarget);

    if (nullptr == mpRT)
        // error, invalid
        return;

    // initialize some basic used values/settings
    cairo_set_antialias(mpRT, rViewInformation.getUseAntiAliasing() ? CAIRO_ANTIALIAS_DEFAULT
                                                                    : CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule(mpRT, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(mpRT, CAIRO_OPERATOR_OVER);

    // evaluate if CairoCoordinateLimitWorkaround is needed
    evaluateCairoCoordinateLimitWorkaround();
}

CairoPixelProcessor2D::CairoPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                             tools::Long nWidthPixel, tools::Long nHeightPixel,
                                             bool bUseRGBA)
    : BaseProcessor2D(rViewInformation)
    , mpTargetOutputDevice(nullptr)
    , maBColorModifierStack()
    , mpOwnedSurface(nullptr)
    , mpRT(nullptr)
    , mbRenderSimpleTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderSimpleTextDirect::get())
    , mbRenderDecoratedTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderDecoratedTextDirect::get())
    , mnClipRecursionCount(0)
    , mbCairoCoordinateLimitWorkaroundActive(false)
{
    if (nWidthPixel <= 0 || nHeightPixel <= 0)
        // no size, invalid
        return;

    mpOwnedSurface = cairo_image_surface_create(bUseRGBA ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
                                                nWidthPixel, nHeightPixel);

    if (nullptr == mpOwnedSurface)
        // error, invalid
        return;

    // create RenderTarget for full target
    mpRT = cairo_create(mpOwnedSurface);

    if (nullptr == mpRT)
        // error, invalid
        return;

    // initialize some basic used values/settings
    cairo_set_antialias(mpRT, rViewInformation.getUseAntiAliasing() ? CAIRO_ANTIALIAS_DEFAULT
                                                                    : CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule(mpRT, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(mpRT, CAIRO_OPERATOR_OVER);

    // evaluate if CairoCoordinateLimitWorkaround is needed
    evaluateCairoCoordinateLimitWorkaround();
}

CairoPixelProcessor2D::CairoPixelProcessor2D(OutputDevice& rOutputDevice,
                                             const geometry::ViewInformation2D& rViewInformation)
    : BaseProcessor2D(rViewInformation)
    , mpTargetOutputDevice(&rOutputDevice)
    , maBColorModifierStack()
    , mpOwnedSurface(nullptr)
    , mpRT(nullptr)
    , mbRenderSimpleTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderSimpleTextDirect::get())
    , mbRenderDecoratedTextDirect(
          officecfg::Office::Common::Drawinglayer::RenderDecoratedTextDirect::get())
    , mnClipRecursionCount(0)
    , mbCairoCoordinateLimitWorkaroundActive(false)
{
    SystemGraphicsData aData(mpTargetOutputDevice->GetSystemGfxData());
    cairo_surface_t* pTarget(static_cast<cairo_surface_t*>(aData.pSurface));

    // no target, nothing to initialize
    if (nullptr == pTarget)
        return;

    // get evtl. offsets if OutputDevice is e.g. a OUTDEV_WINDOW
    // to evaluate if initial clip is needed
    const tools::Long nOffsetPixelX(mpTargetOutputDevice->GetOutOffXPixel());
    const tools::Long nOffsetPixelY(mpTargetOutputDevice->GetOutOffYPixel());
    const tools::Long nWidthPixel(mpTargetOutputDevice->GetOutputWidthPixel());
    const tools::Long nHeightPixel(mpTargetOutputDevice->GetOutputHeightPixel());
    bool bClipNeeded(false);

    if (0 != nOffsetPixelX || 0 != nOffsetPixelY || 0 != nWidthPixel || 0 != nHeightPixel)
    {
        if (0 != nOffsetPixelX || 0 != nOffsetPixelY)
        {
            // if offset is used we need initial clip
            bClipNeeded = true;
        }
        else
        {
            // no offset used, compare to real pixel size
            const tools::Long nRealPixelWidth(cairo_image_surface_get_width(pTarget));
            const tools::Long nRealPixelHeight(cairo_image_surface_get_height(pTarget));

            if (nRealPixelWidth != nWidthPixel || nRealPixelHeight != nHeightPixel)
            {
                // if size differs we need initial clip
                bClipNeeded = true;
            }
        }
    }

    if (bClipNeeded)
    {
        // Make use of the possibility to add an initial clip relative
        // to the 'real' pixel dimensions of the target surface. This is e.g.
        // needed here due to the existence of 'virtual' target surfaces that
        // internally use an offset and limited pixel size, mainly used for
        // UI elements.
        // let the CairoPixelProcessor2D do this, it has internal,
        // system-specific possibilities to do that in an elegant and
        // efficient way (using cairo_surface_create_for_rectangle).
        mpOwnedSurface = cairo_surface_create_for_rectangle(pTarget, nOffsetPixelX, nOffsetPixelY,
                                                            nWidthPixel, nHeightPixel);

        if (nullptr == mpOwnedSurface)
            // error, invalid
            return;

        mpRT = cairo_create(mpOwnedSurface);
    }
    else
    {
        // create RenderTarget for full target
        mpRT = cairo_create(pTarget);
    }

    if (nullptr == mpRT)
        // error, invalid
        return;

    // initialize some basic used values/settings
    cairo_set_antialias(mpRT, rViewInformation.getUseAntiAliasing() ? CAIRO_ANTIALIAS_DEFAULT
                                                                    : CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule(mpRT, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(mpRT, CAIRO_OPERATOR_OVER);

    // prepare output directly to pixels
    mpTargetOutputDevice->Push(vcl::PushFlags::MAPMODE);
    mpTargetOutputDevice->SetMapMode();

    // evaluate if CairoCoordinateLimitWorkaround is needed
    evaluateCairoCoordinateLimitWorkaround();
}

CairoPixelProcessor2D::~CairoPixelProcessor2D()
{
    if (nullptr != mpTargetOutputDevice) // restore MapMode
        mpTargetOutputDevice->Pop();
    if (nullptr != mpRT)
        cairo_destroy(mpRT);
    if (nullptr != mpOwnedSurface)
        cairo_surface_destroy(mpOwnedSurface);
}

BitmapEx CairoPixelProcessor2D::extractBitmapEx() const
{
    // default is empty BitmapEx
    BitmapEx aRetval;

    if (nullptr == mpRT)
        // no RenderContext, not valid
        return aRetval;

    cairo_surface_t* pSource(cairo_get_target(mpRT));
    if (nullptr == pSource)
        // no surface, not valid
        return aRetval;

    // check pixel sizes
    const sal_uInt32 nWidth(cairo_image_surface_get_width(pSource));
    const sal_uInt32 nHeight(cairo_image_surface_get_height(pSource));
    if (0 == nWidth || 0 == nHeight)
        // no content, not valid
        return aRetval;

    // check format
    const cairo_format_t aFormat(cairo_image_surface_get_format(pSource));
    if (CAIRO_FORMAT_ARGB32 != aFormat && CAIRO_FORMAT_RGB24 != aFormat)
        // we for now only support ARGB32 and RGB24, format not supported, not valid
        return aRetval;

    // ensure surface read access, wer need CAIRO_SURFACE_TYPE_IMAGE
    cairo_surface_t* pReadSource(pSource);

    if (CAIRO_SURFACE_TYPE_IMAGE != cairo_surface_get_type(pReadSource))
    {
        // create mapping for read access to source
        pReadSource = cairo_surface_map_to_image(pReadSource, nullptr);
    }

    // prepare VCL/Bitmap stuff
    const Size aBitmapSize(nWidth, nHeight);
    Bitmap aBitmap(aBitmapSize, vcl::PixelFormat::N24_BPP);
    BitmapWriteAccess aAccess(aBitmap);

    // prepare VCL/AlphaMask stuff
    const bool bHasAlpha(CAIRO_FORMAT_ARGB32 == aFormat);
    std::optional<AlphaMask> aAlphaMask;
    // NOTE: Tried to use std::optional for pAlphaWrite but
    // BitmapWriteAccess does not have all needed operators
    BitmapWriteAccess* pAlphaWrite(nullptr);
    if (bHasAlpha)
    {
        aAlphaMask = AlphaMask(aBitmapSize);
        pAlphaWrite = new BitmapWriteAccess(*aAlphaMask);
    }

    // prepare cairo stuff
    const sal_uInt32 nStride(cairo_image_surface_get_stride(pReadSource));
    unsigned char* pStartPixelData(cairo_image_surface_get_data(pReadSource));

    // separate loops for bHasAlpha so that we have *no* branch in the
    // loops itself
    if (bHasAlpha)
    {
        for (sal_uInt32 y(0); y < nHeight; ++y)
        {
            // prepare scanline
            unsigned char* pPixelData(pStartPixelData + (nStride * y));
            Scanline pWriteRGB = aAccess.GetScanline(y);
            Scanline pWriteA = pAlphaWrite->GetScanline(y);

            for (sal_uInt32 x(0); x < nWidth; ++x)
            {
                // RGBA: Do not forget: it's pre-multiplied
                sal_uInt8 nAlpha(pPixelData[SVP_CAIRO_ALPHA]);
                aAccess.SetPixelOnData(
                    pWriteRGB, x,
                    BitmapColor(vcl::bitmap::unpremultiply(pPixelData[SVP_CAIRO_RED], nAlpha),
                                vcl::bitmap::unpremultiply(pPixelData[SVP_CAIRO_GREEN], nAlpha),
                                vcl::bitmap::unpremultiply(pPixelData[SVP_CAIRO_BLUE], nAlpha)));
                pAlphaWrite->SetPixelOnData(pWriteA, x, BitmapColor(nAlpha));
                pPixelData += 4;
            }
        }
    }
    else
    {
        for (sal_uInt32 y(0); y < nHeight; ++y)
        {
            // prepare scanline
            unsigned char* pPixelData(pStartPixelData + (nStride * y));
            Scanline pWriteRGB = aAccess.GetScanline(y);

            for (sal_uInt32 x(0); x < nWidth; ++x)
            {
                aAccess.SetPixelOnData(pWriteRGB, x,
                                       BitmapColor(pPixelData[SVP_CAIRO_RED],
                                                   pPixelData[SVP_CAIRO_GREEN],
                                                   pPixelData[SVP_CAIRO_BLUE]));
                pPixelData += 4;
            }
        }
    }

    // cleanup optional BitmapWriteAccess pAlphaWrite
    if (nullptr != pAlphaWrite)
        delete pAlphaWrite;

    if (bHasAlpha)
        // construct and return BitmapEx
        aRetval = BitmapEx(aBitmap, *aAlphaMask);
    else
        // reset BitmapEx to just Bitmap content
        aRetval = aBitmap;

    if (pReadSource != pSource)
    {
        // cleanup mapping for read/write access to source
        cairo_surface_unmap_image(pSource, pReadSource);
    }

    return aRetval;
}

void CairoPixelProcessor2D::processBitmapPrimitive2D(
    const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
{
    paintBitmapAlpha(rBitmapCandidate.getBitmap(), rBitmapCandidate.getTransform());
}

void CairoPixelProcessor2D::paintBitmapAlpha(const BitmapEx& rBitmapEx,
                                             const basegfx::B2DHomMatrix& rTransform,
                                             double fTransparency)
{
    // transparency invalid or completely transparent, done
    if (fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return;
    }

    // check if graphic content is inside discrete local ViewPort
    const basegfx::B2DRange& rDiscreteViewPort(getViewInformation2D().getDiscreteViewport());
    const basegfx::B2DHomMatrix aLocalTransform(
        getViewInformation2D().getObjectToViewTransformation() * rTransform);

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

    BitmapEx aBitmapEx(rBitmapEx);

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
            const basegfx::B2DPolygon& aPolygon(basegfx::utils::createUnitPolygon());

            // draw directly, done
            paintPolyPolygonRGBA(basegfx::B2DPolyPolygon(aPolygon), aModifiedColor, fTransparency);

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
        impl_cairo_set_hairline(mpRT, getViewInformation2D(),
                                isCairoCoordinateLimitWorkaroundActive());
        cairo_rectangle(mpRT, 0, 0, 1, 1);
        cairo_stroke(mpRT);
    }

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
    // Also errors with images 1 pixel wide/high, e.g. insert
    // RGBA 8x1, 1x8 to see (and deactivate fix below). It also
    // depends on the used filter, see comment below at
    // cairo_pattern_set_filter. Found also errors with more
    // than one pixel, so cannot use as criteria.
    // This effect is also visible in the left/right/bottom/top
    // page shadows, these DO use 8x1/1x8 images which led me to
    // that problem. I double-checked that these *are* correctly
    // defined, that is not the problem.
    // Decided now to use clipping always. That again is
    // simple (we are in unit coordinates)
    cairo_rectangle(mpRT, 0, 0, 1, 1);
    cairo_clip(mpRT);
    cairo_matrix_scale(&aMatrix, cairo_image_surface_get_width(pTarget),
                       cairo_image_surface_get_height(pTarget));

    // The alternative wpuld be: resize/scale it SLIGHTLY to force
    // that half pixel overlap to be inside the unit range.
    // That makes the error disappear, so no clip needed, but
    // SLIGHTLY smaller. Keeping this code if someone might have
    // to finetune this later for reference.
    //
    // cairo_matrix_init_scale(&aMatrix, nWidth + 1, nHeight + 1);
    // cairo_matrix_translate(&aMatrix, -0.5 / (nWidth + 1), -0.5 / (nHeight + 1));

    // The error/effect described above also is connected to the
    // filter used, so I checked the filter modes available
    // in Cairo:
    //
    // CAIRO_FILTER_FAST: okay, small errors, sometimes stretching some pixels
    // CAIRO_FILTER_GOOD: stretching error
    // CAIRO_FILTER_BEST: okay, small errors
    // CAIRO_FILTER_NEAREST: similar to CAIRO_FILTER_FAST
    // CAIRO_FILTER_BILINEAR: similar to CAIRO_FILTER_GOOD
    // CAIRO_FILTER_GAUSSIAN: same as CAIRO_FILTER_GOOD/CAIRO_FILTER_BILINEAR, should
    //   not be used anyways (see docs)
    //
    // CAIRO_FILTER_GOOD seems to be the default anyways, but set it
    // to be on the safe side
    cairo_pattern_set_filter(sourcepattern, CAIRO_FILTER_GOOD);

    // also set extend to CAIRO_EXTEND_PAD, else the outside of the
    // bitmap is guessed as COL_BLACK and the filtering would blend
    // against COL_BLACK what might give strange gray lines at borders
    // of white-on-white bitmaps (used e.g. when painting controls).
    // NOTE: CAIRO_EXTEND_REPEAT also works with clipping and might be
    // broader supported by Cairo implementations
    cairo_pattern_set_extend(sourcepattern, CAIRO_EXTEND_PAD);

    cairo_pattern_set_matrix(sourcepattern, &aMatrix);

    // paint bitmap data, evtl. with additional alpha channel
    if (!basegfx::fTools::equalZero(fTransparency))
        cairo_paint_with_alpha(mpRT, 1.0 - fTransparency);
    else
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

    // determine & set color
    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rPolygonHairlinePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    // set LineWidth, use Cairo's special cairo_set_hairline
    impl_cairo_set_hairline(mpRT, getViewInformation2D(), isCairoCoordinateLimitWorkaroundActive());

    if (isCairoCoordinateLimitWorkaroundActive())
    {
        // need to fallback to paint in view coordinates, unfortunately
        // need to transform self (cairo will do it wrong in this coordinate
        // space), so no need to try to buffer
        cairo_new_path(mpRT);
        basegfx::B2DPolygon aAdaptedPolygon(rPolygon);
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        aAdaptedPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(fAAOffset, fAAOffset)
                                  * getViewInformation2D().getObjectToViewTransformation());
        cairo_identity_matrix(mpRT);
        addB2DPolygonToPathGeometry(mpRT, aAdaptedPolygon);
        cairo_stroke(mpRT);
    }
    else
    {
        // set linear transformation. use own, prepared, re-usable
        // ObjectToViewTransformation and PolyPolygon data and let
        // cairo do the transformations
        cairo_matrix_t aMatrix;
        const basegfx::B2DHomMatrix& rObjectToView(
            getViewInformation2D().getObjectToViewTransformation());
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                          rObjectToView.d(), rObjectToView.e() + fAAOffset,
                          rObjectToView.f() + fAAOffset);
        cairo_set_matrix(mpRT, &aMatrix);

        // get PathGeometry & paint it
        cairo_new_path(mpRT);
        getOrCreatePathGeometry(mpRT, rPolygon, getViewInformation2D(),
                                getViewInformation2D().getUseAntiAliasing());
        cairo_stroke(mpRT);
    }

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
{
    paintPolyPolygonRGBA(rPolyPolygonColorPrimitive2D.getB2DPolyPolygon(),
                         rPolyPolygonColorPrimitive2D.getBColor());
}

void CairoPixelProcessor2D::paintPolyPolygonRGBA(const basegfx::B2DPolyPolygon& rPolyPolygon,
                                                 const basegfx::BColor& rColor,
                                                 double fTransparency)
{
    // transparency invalid or completely transparent, done
    if (fTransparency < 0.0 || fTransparency >= 1.0)
    {
        return;
    }

    const sal_uInt32 nCount(rPolyPolygon.count());

    if (!nCount)
    {
        // no geometry, done
        return;
    }

    cairo_save(mpRT);

    // determine & set color
    const basegfx::BColor aFillColor(maBColorModifierStack.getModifiedColor(rColor));

    if (!basegfx::fTools::equalZero(fTransparency))
        cairo_set_source_rgba(mpRT, aFillColor.getRed(), aFillColor.getGreen(),
                              aFillColor.getBlue(), 1.0 - fTransparency);
    else
        cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(),
                             aFillColor.getBlue());

    if (isCairoCoordinateLimitWorkaroundActive())
    {
        // need to fallback to paint in view coordinates, unfortunately
        // need to transform self (cairo will do it wrong in this coordinate
        // space), so no need to try to buffer
        cairo_new_path(mpRT);
        basegfx::B2DPolyPolygon aAdaptedPolyPolygon(rPolyPolygon);
        aAdaptedPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());
        cairo_identity_matrix(mpRT);
        for (const auto& rPolygon : aAdaptedPolyPolygon)
            addB2DPolygonToPathGeometry(mpRT, rPolygon);
        cairo_fill(mpRT);
    }
    else
    {
        // set linear transformation. use own, prepared, re-usable
        // ObjectToViewTransformation and PolyPolygon data and let
        // cairo do the transformations
        cairo_matrix_t aMatrix;
        const basegfx::B2DHomMatrix& rObjectToView(
            getViewInformation2D().getObjectToViewTransformation());
        cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                          rObjectToView.d(), rObjectToView.e(), rObjectToView.f());
        cairo_set_matrix(mpRT, &aMatrix);

        // get PathGeometry & paint it
        cairo_new_path(mpRT);
        getOrCreateFillGeometry(mpRT, rPolyPolygon);
        cairo_fill(mpRT);
    }

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
    aVisibleRange.intersect(getDiscreteViewRange(mpRT));

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

    // important for content rendering: need to take over the ColorModifierStack
    aContent.setBColorModifierStack(getBColorModifierStack());

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
    aVisibleRange.intersect(getDiscreteViewRange(mpRT));

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

    // take over evtl. used ColorModifierStack for content
    aContent.setBColorModifierStack(getBColorModifierStack());

    aContent.process(rInvertCandidate.getChildren());
    cairo_surface_flush(pContent);

    // decide if to use builtin or create XOR yourself
    // NOTE: not using and doing self is closer to what the
    //       current default does, so keep it
    static bool bUseBuiltinXOR(false);

    if (bUseBuiltinXOR)
    {
        // draw XOR to target using Cairo Operator CAIRO_OPERATOR_XOR
        cairo_set_source_surface(mpRT, pContent, aVisibleRange.getMinX(), aVisibleRange.getMinY());
        cairo_rectangle(mpRT, aVisibleRange.getMinX(), aVisibleRange.getMinY(),
                        aVisibleRange.getWidth(), aVisibleRange.getHeight());
        cairo_set_operator(mpRT, CAIRO_OPERATOR_XOR);
        cairo_fill(mpRT);
    }
    else
    {
        // get read/write access to target - XOR unfortunately needs that
        cairo_surface_t* pRenderTarget(pTarget);

        if (CAIRO_SURFACE_TYPE_IMAGE != cairo_surface_get_type(pRenderTarget))
        {
            // create mapping for read/write access to pRenderTarget
            pRenderTarget = cairo_surface_map_to_image(pRenderTarget, nullptr);
        }

        // iterate over pre-rendered pContent (call it Front)
        const sal_uInt32 nFrontWidth(cairo_image_surface_get_width(pContent));
        const sal_uInt32 nFrontHeight(cairo_image_surface_get_height(pContent));
        const sal_uInt32 nFrontStride(cairo_image_surface_get_stride(pContent));
        unsigned char* pFrontDataRoot(cairo_image_surface_get_data(pContent));

        // in parallel, iterate over original data (call it Back)
        const sal_uInt32 nBackOffX(floor(aVisibleRange.getMinX()));
        const sal_uInt32 nBackOffY(floor(aVisibleRange.getMinY()));
        const sal_uInt32 nBackStride(cairo_image_surface_get_stride(pRenderTarget));
        unsigned char* pBackDataRoot(cairo_image_surface_get_data(pRenderTarget));
        const bool bBackPreMultiply(CAIRO_FORMAT_ARGB32
                                    == cairo_image_surface_get_format(pRenderTarget));

        if (nullptr != pFrontDataRoot && nullptr != pBackDataRoot)
        {
            for (sal_uInt32 y(0); y < nFrontHeight; ++y)
            {
                // get mem locations
                unsigned char* pFrontData(pFrontDataRoot + (nFrontStride * y));
                unsigned char* pBackData(pBackDataRoot + (nBackStride * (y + nBackOffY))
                                         + (nBackOffX * 4));

                // added advance mem to for-expression to be able to continue calls inside
                for (sal_uInt32 x(0); x < nFrontWidth; ++x, pBackData += 4, pFrontData += 4)
                {
                    // do not forget pre-multiply. Use 255 for non-premultiplied to
                    // not have to do if not needed
                    const sal_uInt8 nBackAlpha(bBackPreMultiply ? pBackData[SVP_CAIRO_ALPHA] : 255);

                    // change will only be visible in back/target when not fully transparent
                    if (0 == nBackAlpha)
                        continue;

                    // do not forget pre-multiply -> need to get both alphas. Use 255
                    // for non-premultiplied to not have to do if not needed
                    const sal_uInt8 nFrontAlpha(pFrontData[SVP_CAIRO_ALPHA]);

                    // only something to do if source is not fully transparent
                    if (0 == nFrontAlpha)
                        continue;

                    sal_uInt8 nFrontB(pFrontData[SVP_CAIRO_BLUE]);
                    sal_uInt8 nFrontG(pFrontData[SVP_CAIRO_GREEN]);
                    sal_uInt8 nFrontR(pFrontData[SVP_CAIRO_RED]);

                    if (255 != nFrontAlpha)
                    {
                        // get front color (Front is always CAIRO_FORMAT_ARGB32 and
                        // thus pre-multiplied)
                        nFrontB = vcl::bitmap::unpremultiply(nFrontB, nFrontAlpha);
                        nFrontG = vcl::bitmap::unpremultiply(nFrontG, nFrontAlpha);
                        nFrontR = vcl::bitmap::unpremultiply(nFrontR, nFrontAlpha);
                    }

                    sal_uInt8 nBackB(pBackData[SVP_CAIRO_BLUE]);
                    sal_uInt8 nBackG(pBackData[SVP_CAIRO_GREEN]);
                    sal_uInt8 nBackR(pBackData[SVP_CAIRO_RED]);

                    if (255 != nBackAlpha)
                    {
                        // get back color if bBackPreMultiply (aka 255)
                        nBackB = vcl::bitmap::unpremultiply(nBackB, nBackAlpha);
                        nBackG = vcl::bitmap::unpremultiply(nBackG, nBackAlpha);
                        nBackR = vcl::bitmap::unpremultiply(nBackR, nBackAlpha);
                    }

                    // create XOR r,g,b
                    const sal_uInt8 b(nFrontB ^ nBackB);
                    const sal_uInt8 g(nFrontG ^ nBackG);
                    const sal_uInt8 r(nFrontR ^ nBackR);

                    // write back directly to pBackData/target
                    if (255 == nBackAlpha)
                    {
                        pBackData[SVP_CAIRO_BLUE] = b;
                        pBackData[SVP_CAIRO_GREEN] = g;
                        pBackData[SVP_CAIRO_RED] = r;
                    }
                    else
                    {
                        // additionally premultiply if bBackPreMultiply (aka 255)
                        pBackData[SVP_CAIRO_BLUE] = vcl::bitmap::premultiply(b, nBackAlpha);
                        pBackData[SVP_CAIRO_GREEN] = vcl::bitmap::premultiply(g, nBackAlpha);
                        pBackData[SVP_CAIRO_RED] = vcl::bitmap::premultiply(r, nBackAlpha);
                    }
                }
            }

            cairo_surface_mark_dirty(pRenderTarget);
        }

        if (pRenderTarget != pTarget)
        {
            // cleanup mapping for read/write access to target
            cairo_surface_unmap_image(pTarget, pRenderTarget);
        }
    }

    // cleanup temporary surface
    cairo_surface_destroy(pContent);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processMaskPrimitive2D(
    const primitive2d::MaskPrimitive2D& rMaskCandidate)
{
    if (rMaskCandidate.getChildren().empty())
    {
        // no content, done
        return;
    }

    const basegfx::B2DPolyPolygon& rMask(rMaskCandidate.getMask());

    if (!rMask.count())
    {
        // no mask (so nothing inside), done
        return;
    }

    // calculate visible range
    basegfx::B2DRange aMaskRange(rMask.getB2DRange());
    aMaskRange.transform(getViewInformation2D().getObjectToViewTransformation());
    if (!getDiscreteViewRange(mpRT).overlaps(aMaskRange))
    {
        // not visible, done
        return;
    }

    cairo_save(mpRT);

    if (isCairoCoordinateLimitWorkaroundActive())
    {
        // need to fallback to paint in view coordinates, unfortunately
        // need to transform self (cairo will do it wrong in this coordinate
        // space), so no need to try to buffer
        cairo_new_path(mpRT);
        basegfx::B2DPolyPolygon aAdaptedPolyPolygon(rMask);
        aAdaptedPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());
        for (const auto& rPolygon : aAdaptedPolyPolygon)
            addB2DPolygonToPathGeometry(mpRT, rPolygon);

        // clip to this mask
        cairo_clip(mpRT);
    }
    else
    {
        // set linear transformation for applying mask. use no fAAOffset for mask
        cairo_matrix_t aMatrix;
        const basegfx::B2DHomMatrix& rObjectToView(
            getViewInformation2D().getObjectToViewTransformation());
        cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                          rObjectToView.d(), rObjectToView.e(), rObjectToView.f());
        cairo_set_matrix(mpRT, &aMatrix);

        // create path geometry and put mask as path
        cairo_new_path(mpRT);
        getOrCreateFillGeometry(mpRT, rMask);

        // clip to this mask
        cairo_clip(mpRT);

        // reset transformation to not have it set when processing
        // child content below (was only used to set clip path)
        cairo_identity_matrix(mpRT);
    }

    // process sub-content (that shall be masked)
    mnClipRecursionCount++;
    process(rMaskCandidate.getChildren());
    mnClipRecursionCount--;

    cairo_restore(mpRT);

    if (0 == mnClipRecursionCount)
    {
        // for *some* reason Cairo seems to have problems using cairo_clip
        // recursively, in combination with cairo_save/cairo_restore. I think
        // it *should* work as used here, see
        // https://www.cairographics.org/manual/cairo-cairo-t.html#cairo-clip
        // where this combination is explicitly mentioned/explained. It may
        // just be a error in cairo, too (?).
        // The error is that without that for some reason the last clip is not
        // restored but *stays*, so e.g. when having a shape filled with
        // 'tux.svg' and an ellipse overlapping in front, suddenly (but not
        // always?) the ellipse gets 'clipped' against the shape filled with
        // the tux graphic.
        // What helps is to count the clip recursion for each incarnation of
        // CairoPixelProcessor2D/cairo_t used and call/use cairo_reset_clip
        // when last clip is left.
        cairo_reset_clip(mpRT);
    }
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
    aVisibleRange.intersect(getDiscreteViewRange(mpRT));

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

    // take over evtl. used ColorModifierStack for content
    aContent.setBColorModifierStack(getBColorModifierStack());

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

    // check stroke
    const attribute::StrokeAttribute& rStrokeAttribute(
        rPolygonStrokeCandidate.getStrokeAttribute());
    const bool bDashUsed(!rStrokeAttribute.isDefault()
                         && !rStrokeAttribute.getDotDashArray().empty()
                         && 0.0 < rStrokeAttribute.getFullDotDashLen());
    if (isCairoCoordinateLimitWorkaroundActive())
    {
        // need to fallback to paint in view coordinates, unfortunately
        // need to transform self (cairo will do it wrong in this coordinate
        // space), so no need to try to buffer
        cairo_new_path(mpRT);
        basegfx::B2DPolygon aAdaptedPolygon(rPolygon);
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        aAdaptedPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(fAAOffset, fAAOffset)
                                  * getViewInformation2D().getObjectToViewTransformation());
        cairo_identity_matrix(mpRT);
        addB2DPolygonToPathGeometry(mpRT, aAdaptedPolygon);

        // process/set LineWidth
        const double fObjectLineWidth(bHairline
                                          ? 1.0
                                          : (getViewInformation2D().getObjectToViewTransformation()
                                             * basegfx::B2DVector(rLineAttribute.getWidth(), 0.0))
                                                .getLength());
        cairo_set_line_width(mpRT, fObjectLineWidth);

        if (bDashUsed)
        {
            std::vector<double> aStroke(rStrokeAttribute.getDotDashArray());
            for (auto& rCandidate : aStroke)
                rCandidate = (getViewInformation2D().getObjectToViewTransformation()
                              * basegfx::B2DVector(rCandidate, 0.0))
                                 .getLength();
            cairo_set_dash(mpRT, aStroke.data(), aStroke.size(), 0.0);
        }

        cairo_stroke(mpRT);
    }
    else
    {
        // set linear transformation
        cairo_matrix_t aMatrix;
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        cairo_matrix_init(&aMatrix, rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                          rObjectToView.d(), rObjectToView.e() + fAAOffset,
                          rObjectToView.f() + fAAOffset);
        cairo_set_matrix(mpRT, &aMatrix);

        // create path geometry and put mask as path
        cairo_new_path(mpRT);
        getOrCreatePathGeometry(mpRT, rPolygon, getViewInformation2D(),
                                bHairline && getViewInformation2D().getUseAntiAliasing());

        // process/set LineWidth
        const double fObjectLineWidth(
            bHairline ? (getViewInformation2D().getInverseObjectToViewTransformation()
                         * basegfx::B2DVector(1.0, 0.0))
                            .getLength()
                      : rLineAttribute.getWidth());
        cairo_set_line_width(mpRT, fObjectLineWidth);

        if (bDashUsed)
        {
            const std::vector<double>& rStroke = rStrokeAttribute.getDotDashArray();
            cairo_set_dash(mpRT, rStroke.data(), rStroke.size(), 0.0);
        }

        // render
        cairo_stroke(mpRT);
    }

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

    // work in view coordinates
    const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
    basegfx::B2DRange aRange(rLineRectanglePrimitive2D.getB2DRange());
    aRange.transform(getViewInformation2D().getObjectToViewTransformation());
    cairo_identity_matrix(mpRT);

    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rLineRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aHairlineColor.getRed(), aHairlineColor.getGreen(),
                         aHairlineColor.getBlue());

    const double fDiscreteLineWidth((getViewInformation2D().getInverseObjectToViewTransformation()
                                     * basegfx::B2DVector(1.0, 0.0))
                                        .getLength());
    cairo_set_line_width(mpRT, fDiscreteLineWidth);

    cairo_rectangle(mpRT, aRange.getMinX() + fAAOffset, aRange.getMinY() + fAAOffset,
                    aRange.getWidth(), aRange.getHeight());
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

    // work in view coordinates
    basegfx::B2DRange aRange(rFilledRectanglePrimitive2D.getB2DRange());
    aRange.transform(getViewInformation2D().getObjectToViewTransformation());
    cairo_identity_matrix(mpRT);

    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rFilledRectanglePrimitive2D.getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());

    cairo_rectangle(mpRT, aRange.getMinX(), aRange.getMinY(), aRange.getWidth(),
                    aRange.getHeight());
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
    cairo_identity_matrix(mpRT);

    cairo_set_line_width(mpRT, 1.0f);

    cairo_move_to(mpRT, aStart.getX() + fAAOffset, aStart.getY() + fAAOffset);
    cairo_line_to(mpRT, aEnd.getX() + fAAOffset, aEnd.getY() + fAAOffset);
    cairo_stroke(mpRT);

    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGraphicPrimitive2D(
    const primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D)
{
    if (rFillGraphicPrimitive2D.getTransparency() < 0.0
        || rFillGraphicPrimitive2D.getTransparency() > 1.0)
    {
        // invalid transparence, done
        return;
    }

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

            // draw directly
            paintPolyPolygonRGBA(basegfx::B2DPolyPolygon(aPolygon), aModifiedColor,
                                 rFillGraphicPrimitive2D.getTransparency());

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

    // CAIRO_FILTER_GOOD seems to be the default anyways, but set it
    // to be on the safe side
    cairo_pattern_set_filter(sourcepattern, CAIRO_FILTER_GOOD);

    // paint
    if (rFillGraphicPrimitive2D.hasTransparency())
        cairo_paint_with_alpha(mpRT, 1.0 - rFillGraphicPrimitive2D.getTransparency());
    else
        cairo_paint(mpRT);

    static bool bRenderTransformationBounds(false);
    if (bRenderTransformationBounds)
    {
        cairo_set_source_rgba(mpRT, 0, 1, 0, 0.8);
        impl_cairo_set_hairline(mpRT, getViewInformation2D(),
                                isCairoCoordinateLimitWorkaroundActive());
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

    if (rFillGradientPrimitive2D.hasAlphaGradient())
    {
        const attribute::FillGradientAttribute& rAlphaGradient(
            rFillGradientPrimitive2D.getAlphaGradient());
        double fLuminance(0.0);

        if (!rAlphaGradient.getColorStops().empty())
        {
            if (css::awt::GradientStyle_AXIAL == rAlphaGradient.getStyle())
                fLuminance = rAlphaGradient.getColorStops().back().getStopColor().luminance();
            else
                fLuminance = rAlphaGradient.getColorStops().front().getStopColor().luminance();
        }

        cairo_set_source_rgba(mpRT, aColor.getRed(), aColor.getGreen(), aColor.getBlue(),
                              1.0 - fLuminance);
    }
    else
    {
        cairo_set_source_rgb(mpRT, aColor.getRed(), aColor.getGreen(), aColor.getBlue());
    }

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
    if (basegfx::fTools::less(fBorder, 1.0) && fBorder >= 0.0)
    {
        // no, we have visible content besides border
        return false;
    }

    // draw all-covering polygon using getOuterColor and getOutputRange
    processFillGradientPrimitive2D_drawOutputRange(rFillGradientPrimitive2D);
    return true;
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_linear_axial(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    assert(!rFillGradientPrimitive2D.hasAlphaGradient()
           || rFillGradient.sameDefinitionThanAlpha(rFillGradientPrimitive2D.getAlphaGradient()));
    assert(
        (css::awt::GradientStyle_LINEAR == rFillGradientPrimitive2D.getFillGradient().getStyle()
         || css::awt::GradientStyle_AXIAL == rFillGradientPrimitive2D.getFillGradient().getStyle())
        && "SDPRCairo: Helper allows only SPECIFIED types (!)");
    cairo_save(mpRT);

    // need to do 'antique' stuff adaptions for rotate/transitionStart in object coordinates
    // (DefinitionRange) to have the right 'bending' on rotation
    basegfx::B2DRange aAdaptedRange(rFillGradientPrimitive2D.getDefinitionRange());
    const double fAngle(basegfx::normalizeToRange((2 * M_PI) - rFillGradient.getAngle(), 2 * M_PI));
    const bool bAngle(!basegfx::fTools::equalZero(fAngle));
    const basegfx::B2DPoint aCenter(aAdaptedRange.getCenter());

    // pack rotation and offset into a transformation covering that part
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
    const bool bHasAlpha(rFillGradientPrimitive2D.hasAlphaGradient());
    if (bHasAlpha)
        aBColorStopsAlpha = rFillGradientPrimitive2D.getAlphaGradient().getColorStops();
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
                                              1.0 - aAlpha.luminance());
        }
        else
        {
            if (rFillGradientPrimitive2D.hasTransparency())
            {
                cairo_pattern_add_color_stop_rgba(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                                  aColor.getGreen(), aColor.getBlue(),
                                                  1.0 - rFillGradientPrimitive2D.getTransparency());
            }
            else
            {
                cairo_pattern_add_color_stop_rgb(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                                 aColor.getGreen(), aColor.getBlue());
            }
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
    if (rFillGradientPrimitive2D.hasAlphaGradient() || rFillGradientPrimitive2D.hasTransparency())
    {
        // Do not use direct alpha for this: It paints using four trapez that
        // do not add up at edges due to being painted AntiAliased; that means
        // common pixels do not add up, but blend by transparency, so leaving
        // visual traces -> process recursively
        process(rFillGradientPrimitive2D);
        return;
    }

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
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processFillGradientPrimitive2D_radial_elliptical(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    const attribute::FillGradientAttribute& rFillGradient(
        rFillGradientPrimitive2D.getFillGradient());
    assert(!rFillGradientPrimitive2D.hasAlphaGradient()
           || rFillGradient.sameDefinitionThanAlpha(rFillGradientPrimitive2D.getAlphaGradient()));
    assert((css::awt::GradientStyle_RADIAL == rFillGradientPrimitive2D.getFillGradient().getStyle()
            || css::awt::GradientStyle_ELLIPTICAL
                   == rFillGradientPrimitive2D.getFillGradient().getStyle())
           && "SDPRCairo: Helper allows only SPECIFIED types (!)");
    cairo_save(mpRT);

    // need to do 'antique' stuff adaptions for rotate/transitionStart in object coordinates
    // (DefinitionRange) to have the right 'bending' on rotation
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

    // pack rotation and offset into a combined transformation covering that parts
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
    const bool bHasAlpha(rFillGradientPrimitive2D.hasAlphaGradient());
    if (bHasAlpha)
        aBColorStopsAlpha = rFillGradientPrimitive2D.getAlphaGradient().getColorStops();

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
                                              1.0 - aAlpha.luminance());
        }
        else
        {
            if (rFillGradientPrimitive2D.hasTransparency())
            {
                cairo_pattern_add_color_stop_rgba(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                                  aColor.getGreen(), aColor.getBlue(),
                                                  1.0 - rFillGradientPrimitive2D.getTransparency());
            }
            else
            {
                cairo_pattern_add_color_stop_rgb(pPattern, rStop.getStopOffset(), aColor.getRed(),
                                                 aColor.getGreen(), aColor.getBlue());
            }
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
    if (rFillGradientPrimitive2D.hasAlphaGradient())
    {
        // process recursively to eliminate alpha, cannot be used in decompose fallback
        process(rFillGradientPrimitive2D);
        return;
    }

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
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
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
    // so this is no runtime burner by definition.
    // Making this configurable using static bool, may be moved to settings
    // somewhere later. Do not forget to deactivate when working on 'step'ping
    // stuff in the other helpers (!)
    static bool bPreferAntiAliasedHardColorTransitions(true);

    if (bPreferAntiAliasedHardColorTransitions && rFillGradient.getSteps())
    {
        processFillGradientPrimitive2D_fallback_decompose(rFillGradientPrimitive2D);
        return;
    }

    switch (rFillGradient.getStyle())
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
        {
            // use specialized renderer for this cases - linear, axial
            processFillGradientPrimitive2D_linear_axial(rFillGradientPrimitive2D);
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
            processFillGradientPrimitive2D_radial_elliptical(rFillGradientPrimitive2D);
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

void CairoPixelProcessor2D::processPolyPolygonRGBAPrimitive2D(
    const primitive2d::PolyPolygonRGBAPrimitive2D& rPolyPolygonRGBAPrimitive2D)
{
    if (!rPolyPolygonRGBAPrimitive2D.hasTransparency())
    {
        // do what CairoPixelProcessor2D::processPolyPolygonColorPrimitive2D does
        paintPolyPolygonRGBA(rPolyPolygonRGBAPrimitive2D.getB2DPolyPolygon(),
                             rPolyPolygonRGBAPrimitive2D.getBColor());
        return;
    }

    // draw with alpha directly
    paintPolyPolygonRGBA(rPolyPolygonRGBAPrimitive2D.getB2DPolyPolygon(),
                         rPolyPolygonRGBAPrimitive2D.getBColor(),
                         rPolyPolygonRGBAPrimitive2D.getTransparency());
}

void CairoPixelProcessor2D::processPolyPolygonAlphaGradientPrimitive2D(
    const primitive2d::PolyPolygonAlphaGradientPrimitive2D& rPolyPolygonAlphaGradientPrimitive2D)
{
    const basegfx::B2DPolyPolygon& rPolyPolygon(
        rPolyPolygonAlphaGradientPrimitive2D.getB2DPolyPolygon());
    if (0 == rPolyPolygon.count())
    {
        // no geometry, done
        return;
    }

    const basegfx::BColor& rColor(rPolyPolygonAlphaGradientPrimitive2D.getBColor());
    const attribute::FillGradientAttribute& rAlphaGradient(
        rPolyPolygonAlphaGradientPrimitive2D.getAlphaGradient());
    if (rAlphaGradient.isDefault())
    {
        // default is a single ColorStop at 0.0 with black (0, 0, 0). The
        // luminance is then 0.0, too -> not transparent at all
        paintPolyPolygonRGBA(rPolyPolygon, rColor);
        return;
    }

    basegfx::BColor aSingleColor;
    const basegfx::BColorStops& rAlphaStops(rAlphaGradient.getColorStops());
    if (rAlphaStops.isSingleColor(aSingleColor))
    {
        // draw with alpha directly
        paintPolyPolygonRGBA(rPolyPolygon, rColor, aSingleColor.luminance());
        return;
    }

    const css::awt::GradientStyle aStyle(rAlphaGradient.getStyle());
    if (css::awt::GradientStyle_SQUARE == aStyle || css::awt::GradientStyle_RECT == aStyle)
    {
        // direct paint cannot be used for these styles since they get 'stitched'
        // by multiple parts, so *need* single alpha for multiple pieces, go
        // with decompose/recursion
        process(rPolyPolygonAlphaGradientPrimitive2D);
        return;
    }

    // render as FillGradientPrimitive2D. The idea is to create BColorStops
    // with the same number of entries, but all the same color, using the
    // polygon's target fill color, so we can directly paint gradients as
    // RGBA in Cairo
    basegfx::BColorStops aColorStops;

    // create ColorStops at same stops but single color
    aColorStops.reserve(rAlphaStops.size());
    for (const auto& entry : rAlphaStops)
        aColorStops.emplace_back(entry.getStopOffset(), rColor);

    // create FillGradient using that single-color ColorStops
    const attribute::FillGradientAttribute aFillGradient(
        rAlphaGradient.getStyle(), rAlphaGradient.getBorder(), rAlphaGradient.getOffsetX(),
        rAlphaGradient.getOffsetY(), rAlphaGradient.getAngle(), aColorStops,
        rAlphaGradient.getSteps());

    // create temporary FillGradientPrimitive2D, but do not forget
    // to embed to MaskPrimitive2D to get the PolyPolygon form
    const basegfx::B2DRange aRange(basegfx::utils::getRange(rPolyPolygon));
    const primitive2d::Primitive2DContainer aContainerMaskedFillGradient{
        rtl::Reference<primitive2d::MaskPrimitive2D>(new primitive2d::MaskPrimitive2D(
            rPolyPolygon,
            primitive2d::Primitive2DContainer{ rtl::Reference<primitive2d::FillGradientPrimitive2D>(
                new primitive2d::FillGradientPrimitive2D(aRange, // OutputRange
                                                         aRange, // DefinitionRange
                                                         aFillGradient, &rAlphaGradient)) }))
    };

    // render this. Use container to not trigger decompose for temporary content
    process(aContainerMaskedFillGradient);
}

void CairoPixelProcessor2D::processBitmapAlphaPrimitive2D(
    const primitive2d::BitmapAlphaPrimitive2D& rBitmapAlphaPrimitive2D)
{
    if (!rBitmapAlphaPrimitive2D.hasTransparency())
    {
        // do what CairoPixelProcessor2D::processPolyPolygonColorPrimitive2D does
        paintBitmapAlpha(rBitmapAlphaPrimitive2D.getBitmap(),
                         rBitmapAlphaPrimitive2D.getTransform());
        return;
    }

    // draw with alpha directly
    paintBitmapAlpha(rBitmapAlphaPrimitive2D.getBitmap(), rBitmapAlphaPrimitive2D.getTransform(),
                     rBitmapAlphaPrimitive2D.getTransparency());
}

void CairoPixelProcessor2D::processTextSimplePortionPrimitive2D(
    const primitive2d::TextSimplePortionPrimitive2D& rCandidate)
{
    if (SAL_LIKELY(mbRenderSimpleTextDirect))
    {
        renderTextSimpleOrDecoratedPortionPrimitive2D(rCandidate, nullptr);
    }
    else
    {
        process(rCandidate);
    }
}

void CairoPixelProcessor2D::processTextDecoratedPortionPrimitive2D(
    const primitive2d::TextDecoratedPortionPrimitive2D& rCandidate)
{
    if (SAL_LIKELY(mbRenderDecoratedTextDirect))
    {
        if (!rCandidate.getOrCreateBrokenUpText().empty())
        {
            // if BrokenUpText/WordLineMode is used, go into recursion
            // with single snippets
            process(rCandidate.getOrCreateBrokenUpText());
            return;
        }

        renderTextSimpleOrDecoratedPortionPrimitive2D(rCandidate, &rCandidate);
    }
    else
    {
        process(rCandidate);
    }
}

void CairoPixelProcessor2D::renderTextBackground(
    const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate, double fAscent,
    double fDescent, const basegfx::B2DHomMatrix& rTransform, double fTextWidth)
{
    cairo_save(mpRT);
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, rTransform.a(), rTransform.b(), rTransform.c(), rTransform.d(),
                      rTransform.e(), rTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);
    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rTextCandidate.getTextFillColor().getBColor()));
    cairo_set_source_rgb(mpRT, aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());
    cairo_rectangle(mpRT, 0.0, -fAscent, fTextWidth, fAscent + fDescent);
    cairo_fill(mpRT);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::renderSalLayout(const std::unique_ptr<SalLayout>& rSalLayout,
                                            const basegfx::BColor& rTextColor,
                                            const basegfx::B2DHomMatrix& rTransform,
                                            bool bAntiAliase) const
{
    cairo_save(mpRT);
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, rTransform.a(), rTransform.b(), rTransform.c(), rTransform.d(),
                      rTransform.e(), rTransform.f());
    cairo_set_matrix(mpRT, &aMatrix);
    rSalLayout->drawSalLayout(mpRT, rTextColor, bAntiAliase);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::renderTextDecorationWithOptionalTransformAndColor(
    const primitive2d::TextDecoratedPortionPrimitive2D& rDecoratedCandidate,
    const basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans,
    const basegfx::B2DHomMatrix* pOptionalObjectTransform, const basegfx::BColor* pReplacementColor)
{
    // get decorations from Primitive (using original TextTransform),
    // guaranteed the same visualization as a decomposition would create
    const primitive2d::Primitive2DContainer& rDecorationGeometryContent(
        rDecoratedCandidate.getOrCreateDecorationGeometryContent(
            rDecTrans, rDecoratedCandidate.getText(), rDecoratedCandidate.getTextPosition(),
            rDecoratedCandidate.getTextLength(), rDecoratedCandidate.getDXArray()));

    if (rDecorationGeometryContent.empty())
    {
        // no decoration, done
        return;
    }

    // modify ColorStack as needed - if needed
    if (nullptr != pReplacementColor)
        maBColorModifierStack.push(
            std::make_shared<basegfx::BColorModifier_replace>(*pReplacementColor));

    // modify transformation as needed - if needed
    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());
    if (nullptr != pOptionalObjectTransform)
    {
        geometry::ViewInformation2D aViewInformation2D(getViewInformation2D());
        aViewInformation2D.setObjectTransformation(*pOptionalObjectTransform);
        updateViewInformation(aViewInformation2D);
    }

    // render primitives
    process(rDecorationGeometryContent);

    // restore mods
    if (nullptr != pOptionalObjectTransform)
        updateViewInformation(aLastViewInformation2D);
    if (nullptr != pReplacementColor)
        maBColorModifierStack.pop();
}

void CairoPixelProcessor2D::renderTextSimpleOrDecoratedPortionPrimitive2D(
    const primitive2d::TextSimplePortionPrimitive2D& rTextCandidate,
    const primitive2d::TextDecoratedPortionPrimitive2D* pDecoratedCandidate)
{
    primitive2d::TextLayouterDevice aTextLayouter;
    rTextCandidate.createTextLayouter(aTextLayouter);
    std::unique_ptr<SalLayout> pSalLayout(rTextCandidate.createSalLayout(aTextLayouter));

    if (!pSalLayout)
    {
        // got no layout, error. use decompose as fallback
        process(rTextCandidate);
        return;
    }

    // prepare local transformations
    basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(
        rTextCandidate.getTextTransform());
    const basegfx::B2DHomMatrix aObjTransformWithoutScale(
        basegfx::utils::createShearXRotateTranslateB2DHomMatrix(
            aDecTrans.getShearX(), aDecTrans.getRotate(), aDecTrans.getTranslate()));
    const basegfx::B2DHomMatrix aFullTextTransform(
        getViewInformation2D().getObjectToViewTransformation() * aObjTransformWithoutScale);

    if (!rTextCandidate.getTextFillColor().IsTransparent())
    {
        // render TextBackground first -> casts no shadow itself, so do independent of
        // text shadow being activated
        double fAscent(aTextLayouter.getFontAscent());
        double fDescent(aTextLayouter.getFontDescent());

        if (nullptr != pDecoratedCandidate
            && primitive2d::TEXT_FONT_EMPHASIS_MARK_NONE
                   != pDecoratedCandidate->getTextEmphasisMark())
        {
            if (pDecoratedCandidate->getEmphasisMarkAbove())
                fAscent += aTextLayouter.getTextHeight() * (250.0 / 1000.0);
            if (pDecoratedCandidate->getEmphasisMarkBelow())
                fDescent += aTextLayouter.getTextHeight() * (250.0 / 1000.0);
        }

        renderTextBackground(rTextCandidate, fAscent, fDescent, aFullTextTransform,
                             pSalLayout->GetTextWidth());
    }

    if (rTextCandidate.hasShadow())
    {
        // Text shadow is constant, relative to font size, *not* rotated with
        // text (always from top-left!)
        static const double fFactor(1.0 / 24.0);
        const double fTextShadowOffset(aDecTrans.getScale().getY() * fFactor);

        // see ::ImplDrawSpecialText -> no longer simple fixed color
        const basegfx::BColor aBlack(0.0, 0.0, 0.0);
        basegfx::BColor aShadowColor(aBlack);
        if (aBlack == rTextCandidate.getFontColor()
            || rTextCandidate.getFontColor().luminance() < (8.0 / 255.0))
            aShadowColor = COL_LIGHTGRAY.getBColor();
        aShadowColor = maBColorModifierStack.getModifiedColor(aShadowColor);

        // create shadow offset
        const basegfx::B2DHomMatrix aShadowTransform(
            basegfx::utils::createTranslateB2DHomMatrix(fTextShadowOffset, fTextShadowOffset));
        const basegfx::B2DHomMatrix aShadowFullTextTransform(
            // right to left: 1st the ObjTrans, then the shadow offset, last ObjToView. That way
            // the shadow is always from top-left, independent of text rotation. Independent from
            // thinking about if that is wanted (shadow direction *could* rotate with the text)
            // this is what the office currently does -> do *not* change visualization (!)
            getViewInformation2D().getObjectToViewTransformation() * aShadowTransform
            * aObjTransformWithoutScale);

        // render text as shadow
        renderSalLayout(pSalLayout, aShadowColor, aShadowFullTextTransform,
                        getViewInformation2D().getUseAntiAliasing());

        if (rTextCandidate.hasTextDecoration())
        {
            const basegfx::B2DHomMatrix aTransform(getViewInformation2D().getObjectTransformation()
                                                   * aShadowTransform);
            renderTextDecorationWithOptionalTransformAndColor(*pDecoratedCandidate, aDecTrans,
                                                              &aTransform, &aShadowColor);
        }
    }
    // get TextColor early, may have to be modified
    basegfx::BColor aTextColor(rTextCandidate.getFontColor());

    if (rTextCandidate.hasOutline())
    {
        // render as outline
        aTextColor = maBColorModifierStack.getModifiedColor(aTextColor);
        basegfx::B2DHomMatrix aInvViewTransform;

        // discrete offsets defined here to easily allow to change them,
        // e.g. if more 'fat' outline is wanted, it may be increased to 1.5
        constexpr double fZero(0.0);
        constexpr double fPlus(1.0);
        constexpr double fMinus(-1.0);

        static constexpr std::array<std::pair<double, double>, 8> offsets{
            std::pair<double, double>{ fMinus, fMinus }, std::pair<double, double>{ fZero, fMinus },
            std::pair<double, double>{ fPlus, fMinus },  std::pair<double, double>{ fMinus, fZero },
            std::pair<double, double>{ fPlus, fZero },   std::pair<double, double>{ fMinus, fPlus },
            std::pair<double, double>{ fZero, fPlus },   std::pair<double, double>{ fPlus, fPlus }
        };

        if (rTextCandidate.hasTextDecoration())
        {
            // to use discrete offset (pixels) we will need the back-transform from
            // discrete view coordinates to 'world' coordinates (logic view coordinates),
            // this is the inverse ViewTransformation.
            // NOTE: Alternatively we could calculate the lengths for fPlus/fMinus in
            // logic view coordinates, but would need to create another B2DHomMatrix and
            // to do it correct would need to handle two vectors holding the directions,
            // else - if ever someone will rotate/shear that transformation - it would
            // break
            aInvViewTransform = getViewInformation2D().getViewTransformation();
            aInvViewTransform.invert();
        }

        for (const auto& offset : offsets)
        {
            const basegfx::B2DHomMatrix aDiscreteOffset(
                basegfx::utils::createTranslateB2DHomMatrix(offset.first, offset.second));
            renderSalLayout(pSalLayout, aTextColor, aDiscreteOffset * aFullTextTransform,
                            getViewInformation2D().getUseAntiAliasing());
            if (rTextCandidate.hasTextDecoration())
            {
                const basegfx::B2DHomMatrix aTransform(
                    aInvViewTransform * aDiscreteOffset
                    * getViewInformation2D().getObjectToViewTransformation());
                renderTextDecorationWithOptionalTransformAndColor(*pDecoratedCandidate, aDecTrans,
                                                                  &aTransform);
            }
        }

        // at (center, center) paint in COL_WHITE
        aTextColor = maBColorModifierStack.getModifiedColor(COL_WHITE.getBColor());
        renderSalLayout(pSalLayout, aTextColor, aFullTextTransform,
                        getViewInformation2D().getUseAntiAliasing());
        if (rTextCandidate.hasTextDecoration())
        {
            renderTextDecorationWithOptionalTransformAndColor(*pDecoratedCandidate, aDecTrans,
                                                              nullptr, &aTextColor);
        }

        // paint is complete, Outline and TextRelief cannot be combined, return
        return;
    }

    if (rTextCandidate.hasTextRelief())
    {
        // manipulate TextColor for final text paint below (see ::ImplDrawSpecialText)
        if (aTextColor == COL_BLACK.getBColor())
            aTextColor = COL_WHITE.getBColor();

        // relief offset defined here to easily allow to change them
        // see ::ImplDrawSpecialText and the comment @ 'nOff += mnDPIX/300'
        const bool bEmboss(primitive2d::TEXT_RELIEF_EMBOSSED
                           == pDecoratedCandidate->getTextRelief());
        constexpr double fReliefOffset(1.1);
        const double fOffset(bEmboss ? fReliefOffset : -fReliefOffset);
        const basegfx::B2DHomMatrix aDiscreteOffset(
            basegfx::utils::createTranslateB2DHomMatrix(fOffset, fOffset));

        // see aReliefColor in ::ImplDrawSpecialText
        basegfx::BColor aReliefColor(COL_LIGHTGRAY.getBColor());
        if (COL_WHITE.getBColor() == aTextColor)
            aReliefColor = COL_BLACK.getBColor();
        aReliefColor = maBColorModifierStack.getModifiedColor(aReliefColor);

        // render relief text with offset
        renderSalLayout(pSalLayout, aReliefColor, aDiscreteOffset * aFullTextTransform,
                        getViewInformation2D().getUseAntiAliasing());

        if (rTextCandidate.hasTextDecoration())
        {
            basegfx::B2DHomMatrix aInvViewTransform(getViewInformation2D().getViewTransformation());
            aInvViewTransform.invert();
            const basegfx::B2DHomMatrix aTransform(
                aInvViewTransform * aDiscreteOffset
                * getViewInformation2D().getObjectToViewTransformation());
            renderTextDecorationWithOptionalTransformAndColor(*pDecoratedCandidate, aDecTrans,
                                                              &aTransform, &aReliefColor);
        }
    }

    // render text
    aTextColor = maBColorModifierStack.getModifiedColor(aTextColor);
    renderSalLayout(pSalLayout, aTextColor, aFullTextTransform,
                    getViewInformation2D().getUseAntiAliasing());

    if (rTextCandidate.hasTextDecoration())
    {
        // render using same geometry/primitives that a decompose would
        // create -> safe to get the same visualization for both
        renderTextDecorationWithOptionalTransformAndColor(*pDecoratedCandidate, aDecTrans);
    }
}

bool CairoPixelProcessor2D::handleSvgGradientHelper(
    const primitive2d::SvgGradientHelper& rCandidate)
{
    // check PolyPolygon to be filled
    const basegfx::B2DPolyPolygon& rPolyPolygon(rCandidate.getPolyPolygon());

    if (!rPolyPolygon.count())
    {
        // no PolyPolygon, done
        return true;
    }

    // calculate visible range
    basegfx::B2DRange aPolyPolygonRange(rPolyPolygon.getB2DRange());
    aPolyPolygonRange.transform(getViewInformation2D().getObjectToViewTransformation());
    if (!getDiscreteViewRange(mpRT).overlaps(aPolyPolygonRange))
    {
        // not visible, done
        return true;
    }

    if (!rCandidate.getCreatesContent())
    {
        // creates no content, done
        return true;
    }

    if (rCandidate.getSingleEntry())
    {
        // only one color entry, fill with last existing color, done
        primitive2d::SvgGradientEntryVector::const_reference aEntry(
            rCandidate.getGradientEntries().back());
        paintPolyPolygonRGBA(rCandidate.getPolyPolygon(), aEntry.getColor(),
                             1.0 - aEntry.getOpacity());

        return true;
    }

    return false;
}

void CairoPixelProcessor2D::processSvgLinearGradientPrimitive2D(
    const primitive2d::SvgLinearGradientPrimitive2D& rCandidate)
{
    // check for simple cases, returns if all necessary is already done
    if (handleSvgGradientHelper(rCandidate))
    {
        // simple case, handled, done
        return;
    }

    cairo_save(mpRT);

    // set ObjectToView as regular transformation at CairoContext
    const basegfx::B2DHomMatrix aTrans(getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aTrans.a(), aTrans.b(), aTrans.c(), aTrans.d(), aTrans.e(),
                      aTrans.f());
    cairo_set_matrix(mpRT, &aMatrix);

    // create pattern using unit coordinates. Unit coordinates here means that
    // the transformation provided by the primitive maps the linear gradient
    // to (0,0) -> (1,0) at the unified object coordinates, along the unified
    // X-Axis
    cairo_pattern_t* pPattern(cairo_pattern_create_linear(0, 0, 1, 0));

    // get pre-defined UnitGradientToObject transformation from primitive
    // and invert to get ObjectToUnitGradient transform
    basegfx::B2DHomMatrix aObjectToUnitGradient(
        rCandidate.createUnitGradientToObjectTransformation());
    aObjectToUnitGradient.invert();

    // set ObjectToUnitGradient as transformation at gradient - patterns
    // need the inverted transformation, see cairo documentation
    cairo_matrix_init(&aMatrix, aObjectToUnitGradient.a(), aObjectToUnitGradient.b(),
                      aObjectToUnitGradient.c(), aObjectToUnitGradient.d(),
                      aObjectToUnitGradient.e(), aObjectToUnitGradient.f());
    cairo_pattern_set_matrix(pPattern, &aMatrix);

    // add color stops
    const primitive2d::SvgGradientEntryVector& rGradientEntries(rCandidate.getGradientEntries());

    for (const auto& entry : rGradientEntries)
    {
        const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(entry.getColor()));
        cairo_pattern_add_color_stop_rgba(pPattern, entry.getOffset(), aColor.getRed(),
                                          aColor.getGreen(), aColor.getBlue(), entry.getOpacity());
    }

    // set SpreadMethod. Note that we have no SpreadMethod::None because the
    // source is SVG and SVG does also not have that (checked that)
    switch (rCandidate.getSpreadMethod())
    {
        case primitive2d::SpreadMethod::Pad:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_PAD);
            break;
        case primitive2d::SpreadMethod::Reflect:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_REFLECT);
            break;
        case primitive2d::SpreadMethod::Repeat:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_REPEAT);
            break;
    }

    // get PathGeometry & paint it filed with gradient
    cairo_new_path(mpRT);
    getOrCreateFillGeometry(mpRT, rCandidate.getPolyPolygon());
    cairo_set_source(mpRT, pPattern);
    cairo_fill(mpRT);

    // cleanup
    cairo_pattern_destroy(pPattern);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processSvgRadialGradientPrimitive2D(
    const primitive2d::SvgRadialGradientPrimitive2D& rCandidate)
{
    // check for simple cases, returns if all necessary is already done
    if (handleSvgGradientHelper(rCandidate))
    {
        // simple case, handled, done
        return;
    }

    cairo_save(mpRT);

    // set ObjectToView as regular transformation at CairoContext
    const basegfx::B2DHomMatrix aTrans(getViewInformation2D().getObjectToViewTransformation());
    cairo_matrix_t aMatrix;
    cairo_matrix_init(&aMatrix, aTrans.a(), aTrans.b(), aTrans.c(), aTrans.d(), aTrans.e(),
                      aTrans.f());
    cairo_set_matrix(mpRT, &aMatrix);

    // get pre-defined UnitGradientToObject transformation from primitive
    // and invert to get ObjectToUnitGradient transform
    basegfx::B2DHomMatrix aObjectToUnitGradient(
        rCandidate.createUnitGradientToObjectTransformation());
    aObjectToUnitGradient.invert();

    // prepare empty FocalVector
    basegfx::B2DVector aFocalVector(0.0, 0.0);

    if (rCandidate.isFocalSet())
    {
        // FocalPoint is used, create ObjectTransform based on polygon range
        const basegfx::B2DRange aPolyRange(rCandidate.getPolyPolygon().getB2DRange());
        const double fPolyWidth(aPolyRange.getWidth());
        const double fPolyHeight(aPolyRange.getHeight());
        const basegfx::B2DHomMatrix aObjectTransform(
            basegfx::utils::createScaleTranslateB2DHomMatrix(
                fPolyWidth, fPolyHeight, aPolyRange.getMinX(), aPolyRange.getMinY()));

        // get vector, then transform to object coordinates, then to
        // UnitGradient coordinates to be in the needed coordinate system
        aFocalVector = basegfx::B2DVector(rCandidate.getStart() - rCandidate.getFocal());
        aFocalVector *= aObjectTransform;
        aFocalVector *= aObjectToUnitGradient;
    }

    // create pattern using unit coordinates. Unit coordinates here means that
    // the transformation provided by the primitive maps the radial gradient
    // to (0,0) as center, 1.0 as radius - which is the unit circle. The
    // FocalPoint (if used) has to be relative to that, so - since unified
    // center is at (0, 0), handling as vector is sufficient
    cairo_pattern_t* pPattern(
        cairo_pattern_create_radial(0, 0, 0, aFocalVector.getX(), aFocalVector.getY(), 1));

    // set ObjectToUnitGradient as transformation at gradient - patterns
    // need the inverted transformation, see cairo documentation
    cairo_matrix_init(&aMatrix, aObjectToUnitGradient.a(), aObjectToUnitGradient.b(),
                      aObjectToUnitGradient.c(), aObjectToUnitGradient.d(),
                      aObjectToUnitGradient.e(), aObjectToUnitGradient.f());
    cairo_pattern_set_matrix(pPattern, &aMatrix);

    // add color stops
    const primitive2d::SvgGradientEntryVector& rGradientEntries(rCandidate.getGradientEntries());

    for (const auto& entry : rGradientEntries)
    {
        const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(entry.getColor()));
        cairo_pattern_add_color_stop_rgba(pPattern, entry.getOffset(), aColor.getRed(),
                                          aColor.getGreen(), aColor.getBlue(), entry.getOpacity());
    }

    // set SpreadMethod
    switch (rCandidate.getSpreadMethod())
    {
        case primitive2d::SpreadMethod::Pad:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_PAD);
            break;
        case primitive2d::SpreadMethod::Reflect:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_REFLECT);
            break;
        case primitive2d::SpreadMethod::Repeat:
            cairo_pattern_set_extend(pPattern, CAIRO_EXTEND_REPEAT);
            break;
    }

    // get PathGeometry & paint it filed with gradient
    cairo_new_path(mpRT);
    getOrCreateFillGeometry(mpRT, rCandidate.getPolyPolygon());
    cairo_set_source(mpRT, pPattern);
    cairo_fill(mpRT);

    // cleanup
    cairo_pattern_destroy(pPattern);
    cairo_restore(mpRT);
}

void CairoPixelProcessor2D::processControlPrimitive2D(
    const primitive2d::ControlPrimitive2D& rControlPrimitive)
{
    // find out if the control is already visualized as a VCL-ChildWindow
    bool bControlIsVisibleAsChildWindow(rControlPrimitive.isVisibleAsChildWindow());

    // tdf#131281 FormControl rendering for Tiled Rendering
    if (bControlIsVisibleAsChildWindow && comphelper::LibreOfficeKit::isActive())
    {
        // Do force paint when we are in Tiled Renderer and FormControl is 'visible'
        bControlIsVisibleAsChildWindow = false;
    }

    if (bControlIsVisibleAsChildWindow)
    {
        // f the control is already visualized as a VCL-ChildWindow it
        // does not need to be painted at all
        return;
    }

    bool bDone(false);

    try
    {
        if (nullptr != mpTargetOutputDevice)
        {
            const uno::Reference<awt::XGraphics> xTargetGraphics(
                mpTargetOutputDevice->CreateUnoGraphics());

            if (xTargetGraphics.is())
            {
                // Needs to be drawn. Link new graphics and view
                const uno::Reference<awt::XControl>& rXControl(rControlPrimitive.getXControl());
                uno::Reference<awt::XView> xControlView(rXControl, uno::UNO_QUERY_THROW);
                const uno::Reference<awt::XGraphics> xOriginalGraphics(xControlView->getGraphics());
                xControlView->setGraphics(xTargetGraphics);

                // get position
                const basegfx::B2DHomMatrix aObjectToPixel(
                    getViewInformation2D().getObjectToViewTransformation()
                    * rControlPrimitive.getTransform());
                const basegfx::B2DPoint aTopLeftPixel(aObjectToPixel * basegfx::B2DPoint(0.0, 0.0));

                xControlView->draw(basegfx::fround(aTopLeftPixel.getX()),
                                   basegfx::fround(aTopLeftPixel.getY()));

                // restore original graphics
                xControlView->setGraphics(xOriginalGraphics);
                bDone = true;
            }
        }
    }
    catch (const uno::Exception&)
    {
        // #i116763# removing since there is a good alternative when the xControlView
        // is not found and it is allowed to happen
        // DBG_UNHANDLED_EXCEPTION();
    }

    if (!bDone)
    {
        // process recursively and use the decomposition as Bitmap
        process(rControlPrimitive);
    }
}

void CairoPixelProcessor2D::evaluateCairoCoordinateLimitWorkaround()
{
    static bool bAlreadyCheckedIfNeeded(false);
    static bool bIsNeeded(false);

    if (!bAlreadyCheckedIfNeeded)
    {
        // check once for office runtime: is workaround needed?
        bAlreadyCheckedIfNeeded = true;
        bIsNeeded = checkCoordinateLimitWorkaroundNeededForUsedCairo();
    }

    if (!bIsNeeded)
    {
        // we have a working cairo, so workaround is not needed
        // and mbCairoCoordinateLimitWorkaroundActive can stay false
        return;
    }

    // get discrete size (pixels)
    basegfx::B2DRange aLogicViewRange(getDiscreteViewRange(mpRT));

    // transform to world coordinates -> logic view range
    basegfx::B2DHomMatrix aInvViewTrans(getViewInformation2D().getViewTransformation());
    aInvViewTrans.invert();
    aLogicViewRange.transform(aInvViewTrans);

    // create 1<<23 CairoCoordinate limit from 24.8 internal format
    // and a range fitting to it (just once, this is static)
    constexpr double fNumCairoMax(1 << 23);
    static const basegfx::B2DRange aNumericalCairoLimit(-fNumCairoMax, -fNumCairoMax,
                                                        fNumCairoMax - 1.0, fNumCairoMax - 1.0);

    if (!aLogicViewRange.isEmpty() && !aNumericalCairoLimit.isInside(aLogicViewRange))
    {
        // aLogicViewRange is not completely inside region covered by
        // 24.8 cairo format, thus workaround is needed, set flag
        mbCairoCoordinateLimitWorkaroundActive = true;
    }
}

void CairoPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    const cairo_status_t aStart(cairo_status(mpRT));

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
        case PRIMITIVE2D_ID_POLYPOLYGONRGBAPRIMITIVE2D:
        {
            processPolyPolygonRGBAPrimitive2D(
                static_cast<const primitive2d::PolyPolygonRGBAPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BITMAPALPHAPRIMITIVE2D:
        {
            processBitmapAlphaPrimitive2D(
                static_cast<const primitive2d::BitmapAlphaPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYPOLYGONALPHAGRADIENTPRIMITIVE2D:
        {
            processPolyPolygonAlphaGradientPrimitive2D(
                static_cast<const primitive2d::PolyPolygonAlphaGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D:
        {
            processTextSimplePortionPrimitive2D(
                static_cast<const primitive2d::TextSimplePortionPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D:
        {
            processTextDecoratedPortionPrimitive2D(
                static_cast<const primitive2d::TextDecoratedPortionPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D:
        {
            processSvgLinearGradientPrimitive2D(
                static_cast<const primitive2d::SvgLinearGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D:
        {
            processSvgRadialGradientPrimitive2D(
                static_cast<const primitive2d::SvgRadialGradientPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_CONTROLPRIMITIVE2D:
        {
            processControlPrimitive2D(
                static_cast<const primitive2d::ControlPrimitive2D&>(rCandidate));
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

    const cairo_status_t aEnd(cairo_status(mpRT));

    if (aStart != aEnd)
    {
        SAL_WARN("drawinglayer", "CairoSDPR: Cairo status problem (!)");
    }
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
