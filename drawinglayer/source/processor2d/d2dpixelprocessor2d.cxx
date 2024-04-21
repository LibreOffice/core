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

#include <sal/config.h>

// win-specific
#include <prewin.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <postwin.h>

#include <drawinglayer/processor2d/d2dpixelprocessor2d.hxx>
#include <drawinglayer/processor2d/SDPRProcessor2dTools.hxx>
#include <sal/log.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
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
#include <drawinglayer/primitive2d/invertprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star;

namespace
{
class ID2D1GlobalFactoryProvider
{
    sal::systools::COMReference<ID2D1Factory> mpD2DFactory;

public:
    ID2D1GlobalFactoryProvider()
        : mpD2DFactory(nullptr)
    {
        const HRESULT hr(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                           __uuidof(ID2D1Factory), nullptr,
                                           reinterpret_cast<void**>(&mpD2DFactory)));

        if (!SUCCEEDED(hr))
            mpD2DFactory.clear();
    }

    sal::systools::COMReference<ID2D1Factory>& getID2D1Factory() { return mpD2DFactory; }
};

ID2D1GlobalFactoryProvider aID2D1GlobalFactoryProvider;

class ID2D1GlobalRenderTargetProvider
{
    sal::systools::COMReference<ID2D1DCRenderTarget> mpID2D1DCRenderTarget;

public:
    ID2D1GlobalRenderTargetProvider()
        : mpID2D1DCRenderTarget()
    {
    }

    sal::systools::COMReference<ID2D1DCRenderTarget>& getID2D1DCRenderTarget()
    {
        if (!mpID2D1DCRenderTarget && aID2D1GlobalFactoryProvider.getID2D1Factory())
        {
            const D2D1_RENDER_TARGET_PROPERTIES aRTProps(D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                                  D2D1_ALPHA_MODE_IGNORE), //D2D1_ALPHA_MODE_PREMULTIPLIED),
                0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT));

            const HRESULT hr(aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateDCRenderTarget(
                &aRTProps, &mpID2D1DCRenderTarget));

            // interestingly this ID2D1DCRenderTarget already works and can hold
            // created ID2D1Bitmap(s) in RenderTarget-specific form, *without*
            // any call to "BindDC", thus *without* the need of a real HDC - nice :-)
            // When that would be needed, Application::GetDefaultDevice() would need
            // to have a HDC that is valid during LO's lifetime.

            if (!SUCCEEDED(hr))
                mpID2D1DCRenderTarget.clear();
        }

        return mpID2D1DCRenderTarget;
    }
};

ID2D1GlobalRenderTargetProvider aID2D1GlobalRenderTargetProvider;

class SystemDependentData_ID2D1PathGeometry : public basegfx::SystemDependentData
{
private:
    sal::systools::COMReference<ID2D1PathGeometry> mpID2D1PathGeometry;

public:
    SystemDependentData_ID2D1PathGeometry(
        sal::systools::COMReference<ID2D1PathGeometry>& rID2D1PathGeometry)
        : basegfx::SystemDependentData(Application::GetSystemDependentDataManager())
        , mpID2D1PathGeometry(rID2D1PathGeometry)
    {
    }

    const sal::systools::COMReference<ID2D1PathGeometry>& getID2D1PathGeometry() const
    {
        return mpID2D1PathGeometry;
    }
    virtual sal_Int64 estimateUsageInBytes() const override;
};

sal_Int64 SystemDependentData_ID2D1PathGeometry::estimateUsageInBytes() const
{
    sal_Int64 aRetval(0);

    if (getID2D1PathGeometry())
    {
        UINT32 nCount(0);
        const HRESULT hr(getID2D1PathGeometry()->GetSegmentCount(&nCount));

        if (SUCCEEDED(hr))
        {
            // without completely receiving and tracing the GeometrySink
            // do a rough estimation - each segment is 2D, so has two doubles.
            // Some are beziers, so add some guessed buffer for two additional
            // control points
            aRetval = static_cast<sal_Int64>(nCount) * (6 * sizeof(double));
        }
    }

    return aRetval;
}

basegfx::B2DPoint impPixelSnap(const basegfx::B2DPolygon& rPolygon,
                               const drawinglayer::geometry::ViewInformation2D& rViewInformation,
                               sal_uInt32 nIndex)
{
    const sal_uInt32 nCount(rPolygon.count());

    // get the data
    const basegfx::B2ITuple aPrevTuple(
        basegfx::fround(rViewInformation.getObjectToViewTransformation()
                        * rPolygon.getB2DPoint((nIndex + nCount - 1) % nCount)));
    const basegfx::B2DPoint aCurrPoint(rViewInformation.getObjectToViewTransformation()
                                       * rPolygon.getB2DPoint(nIndex));
    const basegfx::B2ITuple aCurrTuple(basegfx::fround(aCurrPoint));
    const basegfx::B2ITuple aNextTuple(
        basegfx::fround(rViewInformation.getObjectToViewTransformation()
                        * rPolygon.getB2DPoint((nIndex + 1) % nCount)));

    // get the states
    const bool bPrevVertical(aPrevTuple.getX() == aCurrTuple.getX());
    const bool bNextVertical(aNextTuple.getX() == aCurrTuple.getX());
    const bool bPrevHorizontal(aPrevTuple.getY() == aCurrTuple.getY());
    const bool bNextHorizontal(aNextTuple.getY() == aCurrTuple.getY());
    const bool bSnapX(bPrevVertical || bNextVertical);
    const bool bSnapY(bPrevHorizontal || bNextHorizontal);

    if (bSnapX || bSnapY)
    {
        basegfx::B2DPoint aSnappedPoint(bSnapX ? aCurrTuple.getX() : aCurrPoint.getX(),
                                        bSnapY ? aCurrTuple.getY() : aCurrPoint.getY());

        aSnappedPoint *= rViewInformation.getInverseObjectToViewTransformation();

        return aSnappedPoint;
    }

    return rPolygon.getB2DPoint(nIndex);
}

void addB2DPolygonToPathGeometry(sal::systools::COMReference<ID2D1GeometrySink>& rSink,
                                 const basegfx::B2DPolygon& rPolygon,
                                 const drawinglayer::geometry::ViewInformation2D* pViewInformation)
{
    const sal_uInt32 nPointCount(rPolygon.count());
    const sal_uInt32 nEdgeCount(rPolygon.isClosed() ? nPointCount : nPointCount - 1);
    basegfx::B2DCubicBezier aEdge;

    for (sal_uInt32 a(0); a < nEdgeCount; a++)
    {
        rPolygon.getBezierSegment(a, aEdge);

        const basegfx::B2DPoint aEndPoint(
            nullptr == pViewInformation
                ? aEdge.getEndPoint()
                : impPixelSnap(rPolygon, *pViewInformation, (a + 1) % nPointCount));

        if (aEdge.isBezier())
        {
            rSink->AddBezier(
                D2D1::BezierSegment(D2D1::Point2F(aEdge.getControlPointA().getX(),
                                                  aEdge.getControlPointA().getY()), //C1
                                    D2D1::Point2F(aEdge.getControlPointB().getX(),
                                                  aEdge.getControlPointB().getY()), //c2
                                    D2D1::Point2F(aEndPoint.getX(), aEndPoint.getY()))); //end
        }
        else
        {
            rSink->AddLine(D2D1::Point2F(aEndPoint.getX(), aEndPoint.getY()));
        }
    }
}

std::shared_ptr<SystemDependentData_ID2D1PathGeometry>
getOrCreatePathGeometry(const basegfx::B2DPolygon& rPolygon,
                        const drawinglayer::geometry::ViewInformation2D& rViewInformation)
{
    // try to access buffered data
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1PathGeometry(
        rPolygon.getSystemDependentData<SystemDependentData_ID2D1PathGeometry>());

    if (pSystemDependentData_ID2D1PathGeometry)
    {
        if (rViewInformation.getPixelSnapHairline())
        {
            // do not buffer when PixelSnap is active
            pSystemDependentData_ID2D1PathGeometry.reset();
        }
        else
        {
            // use and return buffered data
            return pSystemDependentData_ID2D1PathGeometry;
        }
    }

    sal::systools::COMReference<ID2D1PathGeometry> pID2D1PathGeometry;
    HRESULT hr(
        aID2D1GlobalFactoryProvider.getID2D1Factory()->CreatePathGeometry(&pID2D1PathGeometry));
    const sal_uInt32 nPointCount(rPolygon.count());

    if (SUCCEEDED(hr) && nPointCount)
    {
        sal::systools::COMReference<ID2D1GeometrySink> pSink;
        hr = pID2D1PathGeometry->Open(&pSink);

        if (SUCCEEDED(hr) && pSink)
        {
            const basegfx::B2DPoint aStart(rViewInformation.getPixelSnapHairline()
                                               ? rPolygon.getB2DPoint(0)
                                               : impPixelSnap(rPolygon, rViewInformation, 0));

            pSink->BeginFigure(D2D1::Point2F(aStart.getX(), aStart.getY()),
                               D2D1_FIGURE_BEGIN_HOLLOW);
            addB2DPolygonToPathGeometry(pSink, rPolygon, &rViewInformation);
            pSink->EndFigure(rPolygon.isClosed() ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
            pSink->Close();
        }
    }

    // add to buffering mechanism
    if (pID2D1PathGeometry)
    {
        if (rViewInformation.getPixelSnapHairline() || nPointCount <= 4)
        {
            // do not buffer when PixelSnap is active or small polygon
            return std::make_shared<SystemDependentData_ID2D1PathGeometry>(pID2D1PathGeometry);
        }
        else
        {
            return rPolygon.addOrReplaceSystemDependentData<SystemDependentData_ID2D1PathGeometry>(
                pID2D1PathGeometry);
        }
    }

    return std::shared_ptr<SystemDependentData_ID2D1PathGeometry>();
}

std::shared_ptr<SystemDependentData_ID2D1PathGeometry>
getOrCreateFillGeometry(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // try to access buffered data
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1PathGeometry(
        rPolyPolygon.getSystemDependentData<SystemDependentData_ID2D1PathGeometry>());

    if (pSystemDependentData_ID2D1PathGeometry)
    {
        // use and return buffered data
        return pSystemDependentData_ID2D1PathGeometry;
    }

    sal::systools::COMReference<ID2D1PathGeometry> pID2D1PathGeometry;
    HRESULT hr(
        aID2D1GlobalFactoryProvider.getID2D1Factory()->CreatePathGeometry(&pID2D1PathGeometry));
    const sal_uInt32 nCount(rPolyPolygon.count());

    if (SUCCEEDED(hr) && nCount)
    {
        sal::systools::COMReference<ID2D1GeometrySink> pSink;
        hr = pID2D1PathGeometry->Open(&pSink);

        if (SUCCEEDED(hr) && pSink)
        {
            for (sal_uInt32 a(0); a < nCount; a++)
            {
                const basegfx::B2DPolygon& rPolygon(rPolyPolygon.getB2DPolygon(a));
                const sal_uInt32 nPointCount(rPolygon.count());

                if (nPointCount)
                {
                    const basegfx::B2DPoint aStart(rPolygon.getB2DPoint(0));

                    pSink->BeginFigure(D2D1::Point2F(aStart.getX(), aStart.getY()),
                                       D2D1_FIGURE_BEGIN_FILLED);
                    addB2DPolygonToPathGeometry(pSink, rPolygon, nullptr);
                    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
                }
            }

            pSink->Close();
        }
    }

    // add to buffering mechanism
    if (pID2D1PathGeometry)
    {
        return rPolyPolygon.addOrReplaceSystemDependentData<SystemDependentData_ID2D1PathGeometry>(
            pID2D1PathGeometry);
    }

    return std::shared_ptr<SystemDependentData_ID2D1PathGeometry>();
}

class SystemDependentData_ID2D1Bitmap : public basegfx::SystemDependentData
{
private:
    sal::systools::COMReference<ID2D1Bitmap> mpD2DBitmap;
    const std::shared_ptr<SalBitmap> maAssociatedAlpha;

public:
    SystemDependentData_ID2D1Bitmap(sal::systools::COMReference<ID2D1Bitmap>& rD2DBitmap,
                                    const std::shared_ptr<SalBitmap>& rAssociatedAlpha)
        : basegfx::SystemDependentData(Application::GetSystemDependentDataManager())
        , mpD2DBitmap(rD2DBitmap)
        , maAssociatedAlpha(rAssociatedAlpha)
    {
    }

    const sal::systools::COMReference<ID2D1Bitmap>& getID2D1Bitmap() const { return mpD2DBitmap; }
    const std::shared_ptr<SalBitmap>& getAssociatedAlpha() const { return maAssociatedAlpha; }

    virtual sal_Int64 estimateUsageInBytes() const override;
};

sal_Int64 SystemDependentData_ID2D1Bitmap::estimateUsageInBytes() const
{
    sal_Int64 aRetval(0);

    if (getID2D1Bitmap())
    {
        // use factor 4 for RGBA_8 as estimation
        const D2D1_SIZE_U aSizePixel(getID2D1Bitmap()->GetPixelSize());
        aRetval = static_cast<sal_Int64>(aSizePixel.width)
                  * static_cast<sal_Int64>(aSizePixel.height) * 4;
    }

    return aRetval;
}

sal::systools::COMReference<ID2D1Bitmap> createB2DBitmap(const BitmapEx& rBitmapEx)
{
    const Size& rSizePixel(rBitmapEx.GetSizePixel());
    const bool bAlpha(rBitmapEx.IsAlpha());
    const sal_uInt32 nPixelCount(rSizePixel.Width() * rSizePixel.Height());
    std::unique_ptr<sal_uInt32[]> aData(new sal_uInt32[nPixelCount]);
    sal_uInt32* pTarget = aData.get();

    if (bAlpha)
    {
        Bitmap aSrcAlpha(rBitmapEx.GetAlphaMask().GetBitmap());
        BitmapScopedReadAccess pReadAccess(rBitmapEx.GetBitmap());
        BitmapScopedReadAccess pAlphaReadAccess(aSrcAlpha);
        const tools::Long nHeight(pReadAccess->Height());
        const tools::Long nWidth(pReadAccess->Width());

        for (tools::Long y = 0; y < nHeight; ++y)
        {
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                const BitmapColor aColor(pReadAccess->GetColor(y, x));
                const BitmapColor aAlpha(pAlphaReadAccess->GetColor(y, x));
                const sal_uInt16 nAlpha(aAlpha.GetRed());

                *pTarget++ = sal_uInt32(BitmapColor(
                    ColorAlpha, sal_uInt8((sal_uInt16(aColor.GetRed()) * nAlpha) >> 8),
                    sal_uInt8((sal_uInt16(aColor.GetGreen()) * nAlpha) >> 8),
                    sal_uInt8((sal_uInt16(aColor.GetBlue()) * nAlpha) >> 8), aAlpha.GetRed()));
            }
        }
    }
    else
    {
        BitmapScopedReadAccess pReadAccess(const_cast<Bitmap&>(rBitmapEx.GetBitmap()));
        const tools::Long nHeight(pReadAccess->Height());
        const tools::Long nWidth(pReadAccess->Width());

        for (tools::Long y = 0; y < nHeight; ++y)
        {
            for (tools::Long x = 0; x < nWidth; ++x)
            {
                const BitmapColor aColor(pReadAccess->GetColor(y, x));
                *pTarget++ = sal_uInt32(aColor);
            }
        }
    }

    // use GlobalRenderTarget to allow usage combined with
    // the Direct2D CreateSharedBitmap-mechanism. This is needed
    // since ID2D1Bitmap is a ID2D1RenderTarget-dependent resource
    // and thus - in principle - would have to be re-created for
    // *each* new ID2D1RenderTarget, that means for *each* new
    // target HDC, resp. OutputDevice
    sal::systools::COMReference<ID2D1Bitmap> pID2D1Bitmap;

    if (aID2D1GlobalRenderTargetProvider.getID2D1DCRenderTarget())
    {
        const HRESULT hr(aID2D1GlobalRenderTargetProvider.getID2D1DCRenderTarget()->CreateBitmap(
            D2D1::SizeU(rSizePixel.Width(), rSizePixel.Height()), &aData[0],
            rSizePixel.Width() * sizeof(sal_uInt32),
            D2D1::BitmapProperties(
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, // DXGI_FORMAT
                                  bAlpha ? D2D1_ALPHA_MODE_PREMULTIPLIED
                                         : D2D1_ALPHA_MODE_IGNORE)), // D2D1_ALPHA_MODE
            &pID2D1Bitmap));

        if (!SUCCEEDED(hr))
            pID2D1Bitmap.clear();
    }

    return pID2D1Bitmap;
}

sal::systools::COMReference<ID2D1Bitmap>
getOrCreateB2DBitmap(sal::systools::COMReference<ID2D1RenderTarget>& rRT, const BitmapEx& rBitmapEx)
{
    const basegfx::SystemDependentDataHolder* pHolder(
        rBitmapEx.GetBitmap().accessSystemDependentDataHolder());
    std::shared_ptr<SystemDependentData_ID2D1Bitmap> pSystemDependentData_ID2D1Bitmap;

    if (nullptr != pHolder)
    {
        // try to access SystemDependentDataHolder and buffered data
        pSystemDependentData_ID2D1Bitmap
            = std::static_pointer_cast<SystemDependentData_ID2D1Bitmap>(
                pHolder->getSystemDependentData(
                    typeid(SystemDependentData_ID2D1Bitmap).hash_code()));

        // check data validity for associated Alpha
        if (pSystemDependentData_ID2D1Bitmap
            && pSystemDependentData_ID2D1Bitmap->getAssociatedAlpha()
                   != rBitmapEx.GetAlphaMask().GetBitmap().ImplGetSalBitmap())
        {
            // AssociatedAlpha did change, data invalid
            pSystemDependentData_ID2D1Bitmap.reset();
        }
    }

    if (!pSystemDependentData_ID2D1Bitmap)
    {
        // have to create newly
        sal::systools::COMReference<ID2D1Bitmap> pID2D1Bitmap(createB2DBitmap(rBitmapEx));

        if (pID2D1Bitmap)
        {
            // creation worked, create SystemDependentData_ID2D1Bitmap
            pSystemDependentData_ID2D1Bitmap = std::make_shared<SystemDependentData_ID2D1Bitmap>(
                pID2D1Bitmap, rBitmapEx.GetAlphaMask().GetBitmap().ImplGetSalBitmap());

            // only add if feasible
            if (nullptr != pHolder
                && pSystemDependentData_ID2D1Bitmap->calculateCombinedHoldCyclesInSeconds() > 0)
            {
                basegfx::SystemDependentData_SharedPtr r2(pSystemDependentData_ID2D1Bitmap);
                const_cast<basegfx::SystemDependentDataHolder*>(pHolder)
                    ->addOrReplaceSystemDependentData(r2);
            }
        }
    }

    sal::systools::COMReference<ID2D1Bitmap> pWrappedD2DBitmap;

    if (pSystemDependentData_ID2D1Bitmap)
    {
        // embed to CreateSharedBitmap, that makes it usable on
        // the specified RenderTarget
        const HRESULT hr(rRT->CreateSharedBitmap(
            __uuidof(ID2D1Bitmap),
            static_cast<void*>(pSystemDependentData_ID2D1Bitmap->getID2D1Bitmap()), nullptr,
            &pWrappedD2DBitmap));

        if (!SUCCEEDED(hr))
            pWrappedD2DBitmap.clear();
    }

    return pWrappedD2DBitmap;
}

// This is a simple local derivation of D2DPixelProcessor2D to be used
// when sub-content needs to be rendered to pixels. Hand over the adapted
// ViewInformation2D, a pixel size and the parent RenderTarget. It will
// locally create and use a ID2D1BitmapRenderTarget to render the stuff
// (you need to call process() with the primitives to be painted of
// course). Then use the local helper getID2D1Bitmap() to access the
// ID2D1Bitmap which was the target of that operation.
class D2DBitmapPixelProcessor2D final : public drawinglayer::processor2d::D2DPixelProcessor2D
{
    // the local ID2D1BitmapRenderTarget
    sal::systools::COMReference<ID2D1BitmapRenderTarget> mpBitmapRenderTarget;

public:
    // helper class to create another instance of D2DPixelProcessor2D for
    // creating helper-ID2D1Bitmap's for a given ID2D1RenderTarget
    D2DBitmapPixelProcessor2D(const drawinglayer::geometry::ViewInformation2D& rViewInformation,
                              sal_uInt32 nWidth, sal_uInt32 nHeight,
                              const sal::systools::COMReference<ID2D1RenderTarget>& rParent)
        : drawinglayer::processor2d::D2DPixelProcessor2D(rViewInformation)
        , mpBitmapRenderTarget()
    {
        if (0 == nWidth || 0 == nHeight)
        {
            // no width/height, done
            increaseError();
        }

        if (!hasError())
        {
            // Allocate compatible RGBA render target
            const D2D1_SIZE_U aRenderTargetSizePixel(D2D1::SizeU(nWidth, nHeight));
            const HRESULT hr(rParent->CreateCompatibleRenderTarget(
                nullptr, &aRenderTargetSizePixel, nullptr,
                D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE, &mpBitmapRenderTarget));

            if (!SUCCEEDED(hr) || nullptr == mpBitmapRenderTarget)
            {
                // did not work, done
                increaseError();
            }
            else
            {
                sal::systools::COMReference<ID2D1RenderTarget> pRT;
                mpBitmapRenderTarget->QueryInterface(__uuidof(ID2D1RenderTarget),
                                                     reinterpret_cast<void**>(&pRT));
                setRenderTarget(pRT);
            }
        }

        if (hasRenderTarget())
        {
            // set Viewort if none was given. We have a fixed pixel target, s we know the
            // exact Viewport to work on
            if (getViewInformation2D().getViewport().isEmpty())
            {
                drawinglayer::geometry::ViewInformation2D aViewInformation(getViewInformation2D());
                basegfx::B2DRange aViewport(0.0, 0.0, nWidth, nHeight);
                basegfx::B2DHomMatrix aInvViewTransform(aViewInformation.getViewTransformation());

                aInvViewTransform.invert();
                aViewport.transform(aInvViewTransform);
                aViewInformation.setViewport(aViewport);
                updateViewInformation(aViewInformation);
            }

            // clear as render preparation
            getRenderTarget()->BeginDraw();
            getRenderTarget()->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
            getRenderTarget()->EndDraw();
        }
    }

    sal::systools::COMReference<ID2D1Bitmap> getID2D1Bitmap() const
    {
        sal::systools::COMReference<ID2D1Bitmap> pResult;

        // access the resulting bitmap if exists
        if (mpBitmapRenderTarget)
        {
            mpBitmapRenderTarget->GetBitmap(&pResult);
        }

        return pResult;
    }
};

bool createBitmapSubContent(sal::systools::COMReference<ID2D1Bitmap>& rResult,
                            basegfx::B2DRange& rDiscreteVisibleRange,
                            const drawinglayer::primitive2d::Primitive2DContainer& rContent,
                            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D,
                            const sal::systools::COMReference<ID2D1RenderTarget>& rRenderTarget)
{
    if (rContent.empty() || !rRenderTarget)
    {
        // no content or no render target, done
        return false;
    }

    drawinglayer::processor2d::calculateDiscreteVisibleRange(
        rDiscreteVisibleRange, rContent.getB2DRange(rViewInformation2D), rViewInformation2D);

    if (rDiscreteVisibleRange.isEmpty())
    {
        // not visible, done
        return false;
    }

    // Use a temporary second instance of a D2DBitmapPixelProcessor2D with adapted
    // ViewInformation2D, it will create the needed ID2D1BitmapRenderTarget
    // locally and Clear() it.
    drawinglayer::geometry::ViewInformation2D aAdaptedViewInformation2D(rViewInformation2D);
    const double fTargetWidth(ceil(rDiscreteVisibleRange.getWidth()));
    const double fTargetHeight(ceil(rDiscreteVisibleRange.getHeight()));

    {
        // create adapted ViewTransform, needs to be offset in discrete coordinates,
        // so multiply from left
        basegfx::B2DHomMatrix aAdapted(
            basegfx::utils::createTranslateB2DHomMatrix(-rDiscreteVisibleRange.getMinX(),
                                                        -rDiscreteVisibleRange.getMinY())
            * rViewInformation2D.getViewTransformation());
        aAdaptedViewInformation2D.setViewTransformation(aAdapted);

        // reset Viewport (world coordinates), so the helper renderer will create it's
        // own based on it's given internal discrete size
        aAdaptedViewInformation2D.setViewport(basegfx::B2DRange());
    }

    D2DBitmapPixelProcessor2D aSubContentRenderer(aAdaptedViewInformation2D, fTargetWidth,
                                                  fTargetHeight, rRenderTarget);

    if (!aSubContentRenderer.valid())
    {
        // did not work, done
        return false;
    }

    // render sub-content recursively
    aSubContentRenderer.process(rContent);

    // grab Bitmap & prepare results from RGBA content rendering
    rResult = aSubContentRenderer.getID2D1Bitmap();
    return true;
}
}

namespace drawinglayer::processor2d
{
D2DPixelProcessor2D::D2DPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation)
    : BaseProcessor2D(rViewInformation)
    , maBColorModifierStack()
    , mpRT()
    , mnRecursionCounter(0)
    , mnErrorCounter(0)
{
}

D2DPixelProcessor2D::D2DPixelProcessor2D(const geometry::ViewInformation2D& rViewInformation,
                                         HDC aHdc)
    : BaseProcessor2D(rViewInformation)
    , maBColorModifierStack()
    , mpRT()
    , mnRecursionCounter(0)
    , mnErrorCounter(0)
{
    sal::systools::COMReference<ID2D1DCRenderTarget> pDCRT;
    tools::Long aOutWidth(0), aOutHeight(0);

    if (aHdc)
    {
        aOutWidth = GetDeviceCaps(aHdc, HORZRES);
        aOutHeight = GetDeviceCaps(aHdc, VERTRES);
    }

    if (aOutWidth > 0 && aOutHeight > 0 && aID2D1GlobalFactoryProvider.getID2D1Factory())
    {
        const D2D1_RENDER_TARGET_PROPERTIES aRTProps(D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                              D2D1_ALPHA_MODE_IGNORE), //D2D1_ALPHA_MODE_PREMULTIPLIED),
            0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT));

        const HRESULT hr(
            aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateDCRenderTarget(&aRTProps, &pDCRT));

        if (!SUCCEEDED(hr))
            pDCRT.clear();
    }

    if (pDCRT)
    {
        const RECT rc(
            { 0, 0, o3tl::narrowing<LONG>(aOutWidth), o3tl::narrowing<LONG>(aOutHeight) });
        const HRESULT hr(pDCRT->BindDC(aHdc, &rc));

        if (!SUCCEEDED(hr))
            pDCRT.clear();
    }

    if (pDCRT)
    {
        if (rViewInformation.getUseAntiAliasing())
        {
            D2D1_ANTIALIAS_MODE eAAMode = D2D1_ANTIALIAS_MODE_PER_PRIMITIVE;
            pDCRT->SetAntialiasMode(eAAMode);
        }
        else
        {
            D2D1_ANTIALIAS_MODE eAAMode = D2D1_ANTIALIAS_MODE_ALIASED;
            pDCRT->SetAntialiasMode(eAAMode);
        }

        // since ID2D1DCRenderTarget depends on the transformation
        // set at hdc, be careful and reset it to identity
        XFORM aXForm;
        aXForm.eM11 = 1.0;
        aXForm.eM12 = 0.0;
        aXForm.eM21 = 0.0;
        aXForm.eM22 = 1.0;
        aXForm.eDx = 0.0;
        aXForm.eDy = 0.0;
        SetWorldTransform(aHdc, &aXForm);
    }

    if (pDCRT)
    {
        sal::systools::COMReference<ID2D1RenderTarget> pRT;
        pDCRT->QueryInterface(__uuidof(ID2D1RenderTarget), reinterpret_cast<void**>(&pRT));
        setRenderTarget(pRT);
    }
    else
    {
        increaseError();
    }
}

void D2DPixelProcessor2D::processPolygonHairlinePrimitive2D(
    const primitive2d::PolygonHairlinePrimitive2D& rPolygonHairlinePrimitive2D)
{
    const basegfx::B2DPolygon& rPolygon(rPolygonHairlinePrimitive2D.getB2DPolygon());

    if (!rPolygon.count())
    {
        // no geometry, done
        return;
    }

    bool bDone(false);
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1PathGeometry(
        getOrCreatePathGeometry(rPolygon, getViewInformation2D()));

    if (pSystemDependentData_ID2D1PathGeometry)
    {
        sal::systools::COMReference<ID2D1TransformedGeometry> pTransformedGeometry;
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        const basegfx::B2DHomMatrix& rObjectToView(
            getViewInformation2D().getObjectToViewTransformation());
        HRESULT hr(aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateTransformedGeometry(
            pSystemDependentData_ID2D1PathGeometry->getID2D1PathGeometry(),
            D2D1::Matrix3x2F(rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                             rObjectToView.d(), rObjectToView.e() + fAAOffset,
                             rObjectToView.f() + fAAOffset),
            &pTransformedGeometry));

        if (SUCCEEDED(hr) && pTransformedGeometry)
        {
            const basegfx::BColor aHairlineColor(
                maBColorModifierStack.getModifiedColor(rPolygonHairlinePrimitive2D.getBColor()));
            const D2D1::ColorF aD2DColor(aHairlineColor.getRed(), aHairlineColor.getGreen(),
                                         aHairlineColor.getBlue());
            sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
            hr = getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush);

            if (SUCCEEDED(hr) && pColorBrush)
            {
                getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
                // TODO: Unfortunately Direct2D paint of one pixel wide lines does not
                // correctly and completely blend 100% over the background. Experimenting
                // shows that a value around/slightly below 2.0 is needed which hints that
                // alpha blending the half-shifted lines (see fAAOffset above) is involved.
                // To get correct blending I try to use just wider hairlines for now. This
                // may need to be improved - or balanced (trying sqrt(2) now...)
                getRenderTarget()->DrawGeometry(pTransformedGeometry, pColorBrush, 1.44f);
                bDone = true;
            }
        }
    }

    if (!bDone)
        increaseError();
}

bool D2DPixelProcessor2D::drawPolyPolygonColorTransformed(
    const basegfx::B2DHomMatrix& rTansformation, const basegfx::B2DPolyPolygon& rPolyPolygon,
    const basegfx::BColor& rColor)
{
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1PathGeometry(
        getOrCreateFillGeometry(rPolyPolygon));

    if (pSystemDependentData_ID2D1PathGeometry)
    {
        sal::systools::COMReference<ID2D1TransformedGeometry> pTransformedGeometry;
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        basegfx::B2DHomMatrix aTansformation(getViewInformation2D().getObjectToViewTransformation()
                                             * rTansformation);
        HRESULT hr(aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateTransformedGeometry(
            pSystemDependentData_ID2D1PathGeometry->getID2D1PathGeometry(),
            D2D1::Matrix3x2F(aTansformation.a(), aTansformation.b(), aTansformation.c(),
                             aTansformation.d(), aTansformation.e() + fAAOffset,
                             aTansformation.f() + fAAOffset),
            &pTransformedGeometry));

        if (SUCCEEDED(hr) && pTransformedGeometry)
        {
            const basegfx::BColor aFillColor(maBColorModifierStack.getModifiedColor(rColor));
            const D2D1::ColorF aD2DColor(aFillColor.getRed(), aFillColor.getGreen(),
                                         aFillColor.getBlue());

            sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
            hr = getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush);

            if (SUCCEEDED(hr) && pColorBrush)
            {
                getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
                getRenderTarget()->FillGeometry(pTransformedGeometry, pColorBrush);
                return true;
            }
        }
    }

    return false;
}

void D2DPixelProcessor2D::processPolyPolygonColorPrimitive2D(
    const primitive2d::PolyPolygonColorPrimitive2D& rPolyPolygonColorPrimitive2D)
{
    const basegfx::B2DPolyPolygon& rPolyPolygon(rPolyPolygonColorPrimitive2D.getB2DPolyPolygon());
    const sal_uInt32 nCount(rPolyPolygon.count());

    if (!nCount)
    {
        // no geometry, done
        return;
    }

    const bool bDone(drawPolyPolygonColorTransformed(basegfx::B2DHomMatrix(), rPolyPolygon,
                                                     rPolyPolygonColorPrimitive2D.getBColor()));

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processBitmapPrimitive2D(
    const primitive2d::BitmapPrimitive2D& rBitmapCandidate)
{
    // check if graphic content is inside discrete local ViewPort
    if (!getViewInformation2D().getDiscreteViewport().isEmpty())
    {
        // calculate logic object range, remember: the helper below will
        // transform using getObjectToViewTransformation, so the bitmap-local
        // transform would be missing
        basegfx::B2DRange aDiscreteVisibleRange(basegfx::B2DRange::getUnitB2DRange());
        aDiscreteVisibleRange.transform(rBitmapCandidate.getTransform());

        // calculate visible range
        calculateDiscreteVisibleRange(aDiscreteVisibleRange, aDiscreteVisibleRange,
                                      getViewInformation2D());

        if (aDiscreteVisibleRange.isEmpty())
        {
            // not visible, done
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
            // color gets completely replaced, get it (any input works)
            const basegfx::BColor aModifiedColor(
                maBColorModifierStack.getModifiedColor(basegfx::BColor()));

            // use unit geometry as fallback object geometry. Do *not*
            // transform, the below used method will use the already
            // correctly initialized local ViewInformation
            basegfx::B2DPolygon aPolygon(basegfx::utils::createUnitPolygon());

            rtl::Reference<primitive2d::PolyPolygonColorPrimitive2D> aTemp(
                new primitive2d::PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(aPolygon),
                                                             aModifiedColor));

            // draw as Polygon, done
            processPolyPolygonColorPrimitive2D(*aTemp);
            return;
        }
    }

    bool bDone(false);
    sal::systools::COMReference<ID2D1Bitmap> pD2DBitmap(
        getOrCreateB2DBitmap(getRenderTarget(), aBitmapEx));

    if (pD2DBitmap)
    {
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        const basegfx::B2DHomMatrix aLocalTransform(
            getViewInformation2D().getObjectToViewTransformation()
            * rBitmapCandidate.getTransform());
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F(
            aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(), aLocalTransform.d(),
            aLocalTransform.e() + fAAOffset, aLocalTransform.f() + fAAOffset));

        // destinationRectangle is part of transformation above, so use UnitRange
        getRenderTarget()->DrawBitmap(pD2DBitmap, D2D1::RectF(0.0, 0.0, 1.0, 1.0));
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

sal::systools::COMReference<ID2D1Bitmap> D2DPixelProcessor2D::implCreateAlpha_Direct(
    const primitive2d::TransparencePrimitive2D& rTransCandidate)
{
    // Try if we can use ID2D1DeviceContext/d2d1_1 by querying for interface.
    // Only then can we use ID2D1Effect/CLSID_D2D1LuminanceToAlpha and it makes
    // sense to try to do it this way in this implementation
    sal::systools::COMReference<ID2D1DeviceContext> pID2D1DeviceContext;
    getRenderTarget()->QueryInterface(__uuidof(ID2D1DeviceContext),
                                      reinterpret_cast<void**>(&pID2D1DeviceContext));
    sal::systools::COMReference<ID2D1Bitmap> pRetval;

    if (!pID2D1DeviceContext)
    {
        // no, done - tell caller to use fallback by returning empty - we have
        // not the preconditions for this
        return pRetval;
    }

    // Release early
    pID2D1DeviceContext.clear();
    basegfx::B2DRange aDiscreteVisibleRange;

    if (!createBitmapSubContent(pRetval, aDiscreteVisibleRange, rTransCandidate.getTransparence(),
                                getViewInformation2D(), getRenderTarget())
        || !pRetval)
    {
        // return of false means no display needed, return
        return pRetval;
    }

    // Now we need a target to render this to, using the ID2D1Effect tooling.
    // We can directly apply the effect to an alpha-only 8bit target here,
    // so create one (no RGBA needed for this).
    // We need another render target: I tried to render pInBetweenResult
    // to pContent again, but that does not work due to the bitmap
    // fetched being probably only an internal reference to the
    // ID2D1BitmapRenderTarget, thus it would draw onto itself -> chaos
    sal::systools::COMReference<ID2D1BitmapRenderTarget> pContent;
    const D2D1_PIXEL_FORMAT aAlphaFormat(
        D2D1::PixelFormat(DXGI_FORMAT_A8_UNORM, D2D1_ALPHA_MODE_STRAIGHT));
    const D2D1_SIZE_U aRenderTargetSizePixel(D2D1::SizeU(ceil(aDiscreteVisibleRange.getWidth()),
                                                         ceil(aDiscreteVisibleRange.getHeight())));
    const HRESULT hr(getRenderTarget()->CreateCompatibleRenderTarget(
        nullptr, &aRenderTargetSizePixel, &aAlphaFormat, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
        &pContent));

    if (SUCCEEDED(hr) && pContent)
    {
        // try to access ID2D1DeviceContext of that target, we need that *now*
        pContent->QueryInterface(__uuidof(ID2D1DeviceContext),
                                 reinterpret_cast<void**>(&pID2D1DeviceContext));

        if (pID2D1DeviceContext)
        {
            // create the effect
            sal::systools::COMReference<ID2D1Effect> pLuminanceToAlpha;
            pID2D1DeviceContext->CreateEffect(CLSID_D2D1LuminanceToAlpha, &pLuminanceToAlpha);

            if (pLuminanceToAlpha)
            {
                // chain effect stuff together & paint it
                pLuminanceToAlpha->SetInput(0, pRetval);

                pID2D1DeviceContext->BeginDraw();
                pID2D1DeviceContext->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
                pID2D1DeviceContext->DrawImage(pLuminanceToAlpha);
                pID2D1DeviceContext->EndDraw();

                // grab result
                pContent->GetBitmap(&pRetval);
            }
        }
    }

    return pRetval;
}

sal::systools::COMReference<ID2D1Bitmap> D2DPixelProcessor2D::implCreateAlpha_B2DBitmap(
    const primitive2d::TransparencePrimitive2D& rTransCandidate,
    const basegfx::B2DRange& rVisibleRange, D2D1_MATRIX_3X2_F& rMaskScale)
{
    // Use this fallback that will also use a pixel processor indirectly,
    // but allows to get the AlphaMask as vcl Bitmap using existing tooling
    const sal_uInt32 nDiscreteClippedWidth(ceil(rVisibleRange.getWidth()));
    const sal_uInt32 nDiscreteClippedHeight(ceil(rVisibleRange.getHeight()));
    const sal_uInt32 nMaximumQuadraticPixels(250000);

    // Embed content graphics to TransformPrimitive2D
    const basegfx::B2DHomMatrix aAlphaEmbedding(
        basegfx::utils::createTranslateB2DHomMatrix(-rVisibleRange.getMinX(),
                                                    -rVisibleRange.getMinY())
        * getViewInformation2D().getObjectToViewTransformation());
    const primitive2d::Primitive2DReference xAlphaEmbedRef(new primitive2d::TransformPrimitive2D(
        aAlphaEmbedding,
        drawinglayer::primitive2d::Primitive2DContainer(rTransCandidate.getTransparence())));
    drawinglayer::primitive2d::Primitive2DContainer xEmbedSeq{ xAlphaEmbedRef };

    // use empty ViewInformation to have neutral transformation
    const geometry::ViewInformation2D aEmptyViewInformation2D;

    // use new mode to create AlphaChannel (not just AlphaMask) for transparency channel
    const AlphaMask aAlpha(::drawinglayer::createAlphaMask(
        std::move(xEmbedSeq), aEmptyViewInformation2D,
        basegfx::B2DRange(0, 0, nDiscreteClippedWidth, nDiscreteClippedHeight),
        nMaximumQuadraticPixels, true));
    sal::systools::COMReference<ID2D1Bitmap> pRetval;

    if (aAlpha.IsEmpty())
    {
        // if we have no content we are done
        return pRetval;
    }

    // use alpha data to create the ID2D1Bitmap
    const Size& rSizePixel(aAlpha.GetSizePixel());
    const sal_uInt32 nPixelCount(rSizePixel.Width() * rSizePixel.Height());
    std::unique_ptr<sal_uInt8[]> aData(new sal_uInt8[nPixelCount]);
    sal_uInt8* pTarget = aData.get();
    Bitmap aSrcAlpha(aAlpha.GetBitmap());
    BitmapScopedReadAccess pReadAccess(aSrcAlpha);
    const tools::Long nHeight(pReadAccess->Height());
    const tools::Long nWidth(pReadAccess->Width());

    for (tools::Long y = 0; y < nHeight; ++y)
    {
        for (tools::Long x = 0; x < nWidth; ++x)
        {
            const BitmapColor aColor(pReadAccess->GetColor(y, x));
            *pTarget++ = aColor.GetLuminance();
        }
    }

    const D2D1_BITMAP_PROPERTIES aBmProps(D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)));
    const HRESULT hr(getRenderTarget()->CreateBitmap(
        D2D1::SizeU(rSizePixel.Width(), rSizePixel.Height()), &aData[0],
        rSizePixel.Width() * sizeof(sal_uInt8), &aBmProps, &pRetval));

    if (!SUCCEEDED(hr) || !pRetval)
    {
        // did not work, done
        return pRetval;
    }

    // create needed adapted transformation for alpha brush.
    // We may have to take a corrective scaling into account when the
    // MaximumQuadraticPixel limit was used/triggered
    const Size& rBitmapExSizePixel(aAlpha.GetSizePixel());

    if (static_cast<sal_uInt32>(rBitmapExSizePixel.Width()) != nDiscreteClippedWidth
        || static_cast<sal_uInt32>(rBitmapExSizePixel.Height()) != nDiscreteClippedHeight)
    {
        // scale in X and Y should be the same (see fReduceFactor in createAlphaMask),
        // so adapt numerically to a single scale value, they are integer rounded values
        const double fScaleX(static_cast<double>(rBitmapExSizePixel.Width())
                             / static_cast<double>(nDiscreteClippedWidth));
        const double fScaleY(static_cast<double>(rBitmapExSizePixel.Height())
                             / static_cast<double>(nDiscreteClippedHeight));

        const double fScale(1.0 / ((fScaleX + fScaleY) * 0.5));
        rMaskScale = D2D1::Matrix3x2F::Scale(fScale, fScale);
    }

    return pRetval;
}

void D2DPixelProcessor2D::processTransparencePrimitive2D(
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
    basegfx::B2DRange aDiscreteVisibleRange;
    calculateDiscreteVisibleRange(aDiscreteVisibleRange,
                                  rTransCandidate.getChildren().getB2DRange(getViewInformation2D()),
                                  getViewInformation2D());

    if (aDiscreteVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    // try to create directly, this needs the current mpRT to be a ID2D1DeviceContext/d2d1_1
    // what is not guaranteed but usually works for more modern windows (after 7)
    sal::systools::COMReference<ID2D1Bitmap> pAlphaBitmap(implCreateAlpha_Direct(rTransCandidate));
    D2D1_MATRIX_3X2_F aMaskScale(D2D1::Matrix3x2F::Identity());

    if (!pAlphaBitmap)
    {
        // did not work, use more expensive fallback to existing tooling
        pAlphaBitmap
            = implCreateAlpha_B2DBitmap(rTransCandidate, aDiscreteVisibleRange, aMaskScale);
    }

    if (!pAlphaBitmap)
    {
        // could not create alpha channel, error
        increaseError();
        return;
    }

    sal::systools::COMReference<ID2D1Layer> pLayer;
    HRESULT hr(getRenderTarget()->CreateLayer(nullptr, &pLayer));
    bool bDone(false);

    if (SUCCEEDED(hr) && pLayer)
    {
        sal::systools::COMReference<ID2D1BitmapBrush> pBitmapBrush;
        hr = getRenderTarget()->CreateBitmapBrush(pAlphaBitmap, &pBitmapBrush);

        if (SUCCEEDED(hr) && pBitmapBrush)
        {
            // apply MaskScale to Brush, maybe used if implCreateAlpha_B2DBitmap was needed
            pBitmapBrush->SetTransform(aMaskScale);

            // need to set transform offset for Layer initialization, we work
            // in discrete device coordinates
            getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Translation(
                floor(aDiscreteVisibleRange.getMinX()), floor(aDiscreteVisibleRange.getMinY())));

            getRenderTarget()->PushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), nullptr,
                                                               D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                                                               D2D1::Matrix3x2F::Identity(), 1.0,
                                                               pBitmapBrush),
                                         pLayer);

            // ... but need to reset to paint content unchanged
            getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());

            // draw content recursively
            process(rTransCandidate.getChildren());

            getRenderTarget()->PopLayer();
            bDone = true;
        }
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processUnifiedTransparencePrimitive2D(
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

    // calculate visible range
    basegfx::B2DRange aDiscreteVisibleRange;
    calculateDiscreteVisibleRange(aDiscreteVisibleRange,
                                  rTransCandidate.getChildren().getB2DRange(getViewInformation2D()),
                                  getViewInformation2D());

    if (aDiscreteVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    bool bDone(false);
    sal::systools::COMReference<ID2D1Layer> pLayer;
    const HRESULT hr(getRenderTarget()->CreateLayer(nullptr, &pLayer));

    if (SUCCEEDED(hr) && pLayer)
    {
        // need to set correct transform for Layer initialization, we work
        // in discrete device coordinates
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
        getRenderTarget()->PushLayer(
            D2D1::LayerParameters(D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE,
                                  D2D1::IdentityMatrix(),
                                  1.0 - rTransCandidate.getTransparence()), // opacity
            pLayer);
        process(rTransCandidate.getChildren());
        getRenderTarget()->PopLayer();
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processMaskPrimitive2DPixel(
    const primitive2d::MaskPrimitive2D& rMaskCandidate)
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
    basegfx::B2DRange aDiscreteVisibleRange;
    calculateDiscreteVisibleRange(aDiscreteVisibleRange, aMask.getB2DRange(),
                                  getViewInformation2D());

    if (aDiscreteVisibleRange.isEmpty())
    {
        // not visible, done
        return;
    }

    bool bDone(false);
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1MaskGeometry(
        getOrCreateFillGeometry(rMaskCandidate.getMask()));

    if (pSystemDependentData_ID2D1MaskGeometry)
    {
        sal::systools::COMReference<ID2D1TransformedGeometry> pTransformedMaskGeometry;
        const basegfx::B2DHomMatrix& rObjectToView(
            getViewInformation2D().getObjectToViewTransformation());
        HRESULT hr(aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateTransformedGeometry(
            pSystemDependentData_ID2D1MaskGeometry->getID2D1PathGeometry(),
            D2D1::Matrix3x2F(rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                             rObjectToView.d(), rObjectToView.e(), rObjectToView.f()),
            &pTransformedMaskGeometry));

        if (SUCCEEDED(hr) && pTransformedMaskGeometry)
        {
            sal::systools::COMReference<ID2D1Layer> pLayer;
            hr = getRenderTarget()->CreateLayer(nullptr, &pLayer);

            if (SUCCEEDED(hr) && pLayer)
            {
                // need to set correct transform for Layer initialization, we work
                // in discrete device coordinates
                getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
                getRenderTarget()->PushLayer(
                    D2D1::LayerParameters(D2D1::InfiniteRect(), pTransformedMaskGeometry), pLayer);
                process(rMaskCandidate.getChildren());
                getRenderTarget()->PopLayer();
                bDone = true;
            }
        }
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processPointArrayPrimitive2D(
    const primitive2d::PointArrayPrimitive2D& rPointArrayCandidate)
{
    const std::vector<basegfx::B2DPoint>& rPositions(rPointArrayCandidate.getPositions());

    if (rPositions.empty())
    {
        // no geometry, done
        return;
    }

    const basegfx::BColor aPointColor(
        maBColorModifierStack.getModifiedColor(rPointArrayCandidate.getRGBColor()));
    sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
    D2D1::ColorF aD2DColor(aPointColor.getRed(), aPointColor.getGreen(), aPointColor.getBlue());
    const HRESULT hr(getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush));
    bool bDone(false);

    if (SUCCEEDED(hr) && pColorBrush)
    {
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());

        // To really paint a single pixel I found nothing better than
        // switch off AA and draw a pixel-aligned rectangle
        const D2D1_ANTIALIAS_MODE aOldAAMode(getRenderTarget()->GetAntialiasMode());
        getRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

        for (auto const& pos : rPositions)
        {
            const basegfx::B2DPoint aDiscretePos(
                getViewInformation2D().getObjectToViewTransformation() * pos);
            const double fX(ceil(aDiscretePos.getX()));
            const double fY(ceil(aDiscretePos.getY()));
            const D2D1_RECT_F rect = { FLOAT(fX), FLOAT(fY), FLOAT(fX), FLOAT(fY) };

            getRenderTarget()->DrawRectangle(&rect, pColorBrush);
        }

        getRenderTarget()->SetAntialiasMode(aOldAAMode);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processMarkerArrayPrimitive2D(
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

    sal::systools::COMReference<ID2D1Bitmap> pD2DBitmap(
        getOrCreateB2DBitmap(getRenderTarget(), rMarker));
    bool bDone(false);

    if (pD2DBitmap)
    {
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
        const Size& rSizePixel(rMarker.GetSizePixel());
        const tools::Long nMiX((rSizePixel.Width() / 2) + 1);
        const tools::Long nMiY((rSizePixel.Height() / 2) + 1);
        const tools::Long nPlX(rSizePixel.Width() - nMiX);
        const tools::Long nPlY(rSizePixel.Height() - nMiY);

        // draw with non-AA to show unhampered, clear, non-scaled marker
        const D2D1_ANTIALIAS_MODE aOldAAMode(getRenderTarget()->GetAntialiasMode());
        getRenderTarget()->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

        for (auto const& pos : rPositions)
        {
            const basegfx::B2DPoint aDiscretePos(
                getViewInformation2D().getObjectToViewTransformation() * pos);
            const double fX(ceil(aDiscretePos.getX()));
            const double fY(ceil(aDiscretePos.getY()));
            const D2D1_RECT_F rect
                = { FLOAT(fX - nMiX), FLOAT(fY - nMiY), FLOAT(fX + nPlX), FLOAT(fY + nPlY) };

            getRenderTarget()->DrawBitmap(pD2DBitmap, &rect);
        }

        getRenderTarget()->SetAntialiasMode(aOldAAMode);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processBackgroundColorPrimitive2D(
    const primitive2d::BackgroundColorPrimitive2D& rBackgroundColorCandidate)
{
    // check for allowed range [0.0 .. 1.0[
    if (rBackgroundColorCandidate.getTransparency() < 0.0
        || rBackgroundColorCandidate.getTransparency() >= 1.0)
        return;

    const D2D1::ColorF aD2DColor(rBackgroundColorCandidate.getBColor().getRed(),
                                 rBackgroundColorCandidate.getBColor().getGreen(),
                                 rBackgroundColorCandidate.getBColor().getBlue(),
                                 1.0 - rBackgroundColorCandidate.getTransparency());

    getRenderTarget()->Clear(aD2DColor);
}

void D2DPixelProcessor2D::processModifiedColorPrimitive2D(
    const primitive2d::ModifiedColorPrimitive2D& rModifiedCandidate)
{
    if (!rModifiedCandidate.getChildren().empty())
    {
        maBColorModifierStack.push(rModifiedCandidate.getColorModifier());
        process(rModifiedCandidate.getChildren());
        maBColorModifierStack.pop();
    }
}

void D2DPixelProcessor2D::processTransformPrimitive2D(
    const primitive2d::TransformPrimitive2D& rTransformCandidate)
{
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

void D2DPixelProcessor2D::processPolygonStrokePrimitive2D(
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
        rtl::Reference<primitive2d::PolygonStrokePrimitive2D> aCopy(
            new primitive2d::PolygonStrokePrimitive2D(
                rPolygonStrokeCandidate.getB2DPolygon(), aRed,
                rPolygonStrokeCandidate.getStrokeAttribute()));
        process(*aCopy);
    }

    bool bDone(false);
    std::shared_ptr<SystemDependentData_ID2D1PathGeometry> pSystemDependentData_ID2D1PathGeometry(
        getOrCreatePathGeometry(rPolygon, getViewInformation2D()));

    if (pSystemDependentData_ID2D1PathGeometry)
    {
        sal::systools::COMReference<ID2D1TransformedGeometry> pTransformedGeometry;
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        HRESULT hr(aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateTransformedGeometry(
            pSystemDependentData_ID2D1PathGeometry->getID2D1PathGeometry(),
            D2D1::Matrix3x2F(rObjectToView.a(), rObjectToView.b(), rObjectToView.c(),
                             rObjectToView.d(), rObjectToView.e() + fAAOffset,
                             rObjectToView.f() + fAAOffset),
            &pTransformedGeometry));

        if (SUCCEEDED(hr) && pTransformedGeometry)
        {
            const basegfx::BColor aLineColor(
                maBColorModifierStack.getModifiedColor(rLineAttribute.getColor()));
            D2D1::ColorF aD2DColor(aLineColor.getRed(), aLineColor.getGreen(),
                                   aLineColor.getBlue());

            if (bRenderDecomposeForCompareInRed)
            {
                aD2DColor.a = 0.5;
            }

            sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
            hr = getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush);

            if (SUCCEEDED(hr) && pColorBrush)
            {
                sal::systools::COMReference<ID2D1StrokeStyle> pStrokeStyle;
                D2D1_CAP_STYLE aCapStyle(D2D1_CAP_STYLE_FLAT);
                D2D1_LINE_JOIN aLineJoin(D2D1_LINE_JOIN_MITER);
                const attribute::StrokeAttribute& rStrokeAttribute(
                    rPolygonStrokeCandidate.getStrokeAttribute());
                const bool bDashUsed(!rStrokeAttribute.isDefault()
                                     && !rStrokeAttribute.getDotDashArray().empty()
                                     && 0.0 < rStrokeAttribute.getFullDotDashLen());
                D2D1_DASH_STYLE aDashStyle(bDashUsed ? D2D1_DASH_STYLE_CUSTOM
                                                     : D2D1_DASH_STYLE_SOLID);
                std::vector<float> dashes;
                float miterLimit(1.0);

                switch (rLineAttribute.getLineCap())
                {
                    case css::drawing::LineCap_ROUND:
                        aCapStyle = D2D1_CAP_STYLE_ROUND;
                        break;
                    case css::drawing::LineCap_SQUARE:
                        aCapStyle = D2D1_CAP_STYLE_SQUARE;
                        break;
                    default:
                        break;
                }

                switch (rLineAttribute.getLineJoin())
                {
                    case basegfx::B2DLineJoin::NONE:
                        break;
                    case basegfx::B2DLineJoin::Bevel:
                        aLineJoin = D2D1_LINE_JOIN_BEVEL;
                        break;
                    case basegfx::B2DLineJoin::Miter:
                    {
                        // for basegfx::B2DLineJoin::Miter there are two problems:
                        // (1) MS uses D2D1_LINE_JOIN_MITER which handles the cut-off when MiterLimit is hit not by
                        //     fallback to Bevel, but by cutting miter geometry at the defined distance. That is
                        //     nice, but not what we need or is the standard for other graphic systems. Luckily there
                        //     is also D2D1_LINE_JOIN_MITER_OR_BEVEL and (after some search) the page
                        //     https://learn.microsoft.com/en-us/windows/win32/api/d2d1/ne-d2d1-d2d1_line_join
                        //     which gives some explanation, so that is what we need to use here.
                        // (2) Instead of using an angle in radians (15 deg default) MS uses
                        //     "miterLimit is relative to 1/2 LineWidth", so a length. After some experimenting
                        //     it shows that the (better understandable) angle has to be converted to the length
                        //     that a miter prolongation would have at that angle, so use some trigonometry.
                        //     Unfortunately there is also some'precision' problem (probably), so I had to
                        //     experimentally come to a correction value around 0.9925. Since that seems to
                        //     be no obvious numerical value involved somehow (and as long as I find no other
                        //     explanation) I will have to use that.
                        // NOTE: To find that correction value I usd that handy bRenderDecomposeForCompareInRed
                        //       and changes in debugger - as work tipp
                        // With both done I can use Direct2D for Miter completely - what is good for speed.
                        aLineJoin = D2D1_LINE_JOIN_MITER_OR_BEVEL;

                        // snap absolute value of angle in radians to [0.0 .. PI]
                        double fVal(::basegfx::snapToZeroRange(
                            fabs(rLineAttribute.getMiterMinimumAngle()), M_PI));

                        // cut at 0.0 and PI since sin would be zero ('endless' miter)
                        const double fSmallValue(M_PI * 0.0000001);
                        fVal = std::max(fSmallValue, fVal);
                        fVal = std::min(M_PI - fSmallValue, fVal);

                        // get relative length
                        fVal = 1.0 / sin(fVal);

                        // use for miterLimit, we need factor 2.0 (relative to double LineWidth)
                        // and the correction mentioned in (2) above
                        const double fCorrector(2.0 * 0.9925);

                        miterLimit = fVal * fCorrector;
                        break;
                    }
                    case basegfx::B2DLineJoin::Round:
                        aLineJoin = D2D1_LINE_JOIN_ROUND;
                        break;
                    default:
                        break;
                }

                if (bDashUsed)
                {
                    // dashes need to be discrete and relative to LineWidth
                    for (auto& value : rStrokeAttribute.getDotDashArray())
                    {
                        dashes.push_back(
                            (rObjectToView * basegfx::B2DVector(value, 0.0)).getLength()
                            / fDiscreteLineWidth);
                    }
                }

                hr = aID2D1GlobalFactoryProvider.getID2D1Factory()->CreateStrokeStyle(
                    D2D1::StrokeStyleProperties(aCapStyle, // startCap
                                                aCapStyle, // endCap
                                                aCapStyle, // dashCap
                                                aLineJoin, // lineJoin
                                                miterLimit, // miterLimit
                                                aDashStyle, // dashStyle
                                                0.0f), // dashOffset
                    bDashUsed ? dashes.data() : nullptr, bDashUsed ? dashes.size() : 0,
                    &pStrokeStyle);

                if (SUCCEEDED(hr) && pStrokeStyle)
                {
                    getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
                    getRenderTarget()->DrawGeometry(
                        pTransformedGeometry, pColorBrush,
                        // TODO: Hairline LineWidth, see comment at processPolygonHairlinePrimitive2D
                        bHairline ? 1.44 : fDiscreteLineWidth, pStrokeStyle);
                    bDone = true;
                }
            }
        }
    }

    if (!bDone)
    {
        // fallback to decomposition
        process(rPolygonStrokeCandidate);
    }
}

void D2DPixelProcessor2D::processLineRectanglePrimitive2D(
    const primitive2d::LineRectanglePrimitive2D& rLineRectanglePrimitive2D)
{
    if (rLineRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    const basegfx::BColor aHairlineColor(
        maBColorModifierStack.getModifiedColor(rLineRectanglePrimitive2D.getBColor()));
    const D2D1::ColorF aD2DColor(aHairlineColor.getRed(), aHairlineColor.getGreen(),
                                 aHairlineColor.getBlue());
    sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
    const HRESULT hr(getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush));
    bool bDone(false);

    if (SUCCEEDED(hr) && pColorBrush)
    {
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        const basegfx::B2DHomMatrix aLocalTransform(
            getViewInformation2D().getObjectToViewTransformation());
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F(
            aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(), aLocalTransform.d(),
            aLocalTransform.e() + fAAOffset, aLocalTransform.f() + fAAOffset));
        const basegfx::B2DRange& rRange(rLineRectanglePrimitive2D.getB2DRange());
        const D2D1_RECT_F rect = { FLOAT(rRange.getMinX()), FLOAT(rRange.getMinY()),
                                   FLOAT(rRange.getMaxX()), FLOAT(rRange.getMaxY()) };
        const double fDiscreteLineWidth(
            (getViewInformation2D().getInverseObjectToViewTransformation()
             * basegfx::B2DVector(1.44, 0.0))
                .getLength());

        getRenderTarget()->DrawRectangle(&rect, pColorBrush, fDiscreteLineWidth);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processFilledRectanglePrimitive2D(
    const primitive2d::FilledRectanglePrimitive2D& rFilledRectanglePrimitive2D)
{
    if (rFilledRectanglePrimitive2D.getB2DRange().isEmpty())
    {
        // no geometry, done
        return;
    }

    const basegfx::BColor aFillColor(
        maBColorModifierStack.getModifiedColor(rFilledRectanglePrimitive2D.getBColor()));
    const D2D1::ColorF aD2DColor(aFillColor.getRed(), aFillColor.getGreen(), aFillColor.getBlue());
    sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
    const HRESULT hr(getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush));
    bool bDone(false);

    if (SUCCEEDED(hr) && pColorBrush)
    {
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        const basegfx::B2DHomMatrix aLocalTransform(
            getViewInformation2D().getObjectToViewTransformation());
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F(
            aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(), aLocalTransform.d(),
            aLocalTransform.e() + fAAOffset, aLocalTransform.f() + fAAOffset));
        const basegfx::B2DRange& rRange(rFilledRectanglePrimitive2D.getB2DRange());
        const D2D1_RECT_F rect = { FLOAT(rRange.getMinX()), FLOAT(rRange.getMinY()),
                                   FLOAT(rRange.getMaxX()), FLOAT(rRange.getMaxY()) };

        getRenderTarget()->FillRectangle(&rect, pColorBrush);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processSingleLinePrimitive2D(
    const primitive2d::SingleLinePrimitive2D& rSingleLinePrimitive2D)
{
    const basegfx::BColor aLineColor(
        maBColorModifierStack.getModifiedColor(rSingleLinePrimitive2D.getBColor()));
    const D2D1::ColorF aD2DColor(aLineColor.getRed(), aLineColor.getGreen(), aLineColor.getBlue());
    sal::systools::COMReference<ID2D1SolidColorBrush> pColorBrush;
    const HRESULT hr(getRenderTarget()->CreateSolidColorBrush(aD2DColor, &pColorBrush));
    bool bDone(false);

    if (SUCCEEDED(hr) && pColorBrush)
    {
        const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
        basegfx::B2DHomMatrix aLocalTransform(
            getViewInformation2D().getObjectToViewTransformation());
        const basegfx::B2DPoint aStart(aLocalTransform * rSingleLinePrimitive2D.getStart());
        const basegfx::B2DPoint aEnd(aLocalTransform * rSingleLinePrimitive2D.getEnd());

        getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
        const D2D1_POINT_2F aD2D1Start
            = { FLOAT(aStart.getX() + fAAOffset), FLOAT(aStart.getY() + fAAOffset) };
        const D2D1_POINT_2F aD2D1End
            = { FLOAT(aEnd.getX() + fAAOffset), FLOAT(aEnd.getY() + fAAOffset) };

        getRenderTarget()->DrawLine(aD2D1Start, aD2D1End, pColorBrush, 1.44f);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processFillGraphicPrimitive2D(
    const primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D)
{
    BitmapEx aPreparedBitmap;
    basegfx::B2DRange aFillUnitRange(rFillGraphicPrimitive2D.getFillGraphic().getGraphicRange());
    static double fBigDiscreteArea(300.0 * 300.0);

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

    bool bDone(false);
    sal::systools::COMReference<ID2D1Bitmap> pD2DBitmap(
        getOrCreateB2DBitmap(getRenderTarget(), aPreparedBitmap));

    if (pD2DBitmap)
    {
        sal::systools::COMReference<ID2D1BitmapBrush> pBitmapBrush;
        const HRESULT hr(getRenderTarget()->CreateBitmapBrush(pD2DBitmap, &pBitmapBrush));

        if (SUCCEEDED(hr) && pBitmapBrush)
        {
            // set extended to repeat/wrap AKA tiling
            pBitmapBrush->SetExtendModeX(D2D1_EXTEND_MODE_WRAP);
            pBitmapBrush->SetExtendModeY(D2D1_EXTEND_MODE_WRAP);

            // set interpolation mode
            // NOTE: This uses D2D1_BITMAP_INTERPOLATION_MODE, but there seem to be
            //       advanced modes when using D2D1_INTERPOLATION_MODE, but that needs
            //       D2D1_BITMAP_BRUSH_PROPERTIES1 and ID2D1BitmapBrush1
            sal::systools::COMReference<ID2D1BitmapBrush1> pBrush1;
            pBitmapBrush->QueryInterface(__uuidof(ID2D1BitmapBrush1),
                                         reinterpret_cast<void**>(&pBrush1));

            if (pBrush1)
            {
                pBrush1->SetInterpolationMode1(D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR);
            }
            else
            {
                pBitmapBrush->SetInterpolationMode(D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
            }

            // set BitmapBrush transformation relative to it's PixelSize and
            // the used FillUnitRange. Since we use unit coordinates here this
            // is pretty simple
            const D2D1_SIZE_U aBMSPixel(pD2DBitmap->GetPixelSize());
            const double fScaleX((aFillUnitRange.getMaxX() - aFillUnitRange.getMinX())
                                 / aBMSPixel.width);
            const double fScaleY((aFillUnitRange.getMaxY() - aFillUnitRange.getMinY())
                                 / aBMSPixel.height);
            const D2D1_MATRIX_3X2_F aBTrans(D2D1::Matrix3x2F(
                fScaleX, 0.0, 0.0, fScaleY, aFillUnitRange.getMinX(), aFillUnitRange.getMinY()));
            pBitmapBrush->SetTransform(&aBTrans);

            // set transform to ObjectToWorld to be able to paint in unit coordinates, so
            // evtl. shear/rotate in that transform is used and does not influence the
            // orthogonal and unit-oriented brush handling
            const double fAAOffset(getViewInformation2D().getUseAntiAliasing() ? 0.5 : 0.0);
            const basegfx::B2DHomMatrix aLocalTransform(
                getViewInformation2D().getObjectToViewTransformation()
                * rFillGraphicPrimitive2D.getTransformation());
            getRenderTarget()->SetTransform(D2D1::Matrix3x2F(
                aLocalTransform.a(), aLocalTransform.b(), aLocalTransform.c(), aLocalTransform.d(),
                aLocalTransform.e() + fAAOffset, aLocalTransform.f() + fAAOffset));

            // use unit rectangle, transformation is already set to include ObjectToWorld
            const D2D1_RECT_F rect = { FLOAT(0.0), FLOAT(0.0), FLOAT(1.0), FLOAT(1.0) };

            // draw as unit rectangle as brush filled rectangle
            getRenderTarget()->FillRectangle(&rect, pBitmapBrush);
            bDone = true;
        }
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processFillGradientPrimitive2D(
    const primitive2d::FillGradientPrimitive2D& rFillGradientPrimitive2D)
{
    // draw all-covering initial BG polygon 1st
    bool bDone(drawPolyPolygonColorTransformed(
        basegfx::B2DHomMatrix(),
        basegfx::B2DPolyPolygon(
            basegfx::utils::createPolygonFromRect(rFillGradientPrimitive2D.getOutputRange())),
        rFillGradientPrimitive2D.getOuterColor()));

    if (bDone)
    {
        const basegfx::B2DPolyPolygon aForm(rFillGradientPrimitive2D.getUnitPolygon());

        // paint solid fill steps by providing callback as lambda
        auto aCallback([&aForm, &bDone, this](const basegfx::B2DHomMatrix& rMatrix,
                                              const basegfx::BColor& rColor) {
            if (bDone)
            {
                bDone = drawPolyPolygonColorTransformed(rMatrix, aForm, rColor);
            }
        });

        // call value generator to trigger callbacks
        rFillGradientPrimitive2D.generateMatricesAndColors(aCallback);
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processInvertPrimitive2D(
    const primitive2d::InvertPrimitive2D& rInvertPrimitive2D)
{
    if (rInvertPrimitive2D.getChildren().empty())
    {
        // no content, done
        return;
    }

    // Try if we can use ID2D1DeviceContext/d2d1_1 by querying for interface.
    // Only with ID2D1DeviceContext we can use ::DrawImage which supports
    // D2D1_COMPOSITE_MODE_XOR
    sal::systools::COMReference<ID2D1DeviceContext> pID2D1DeviceContext;
    getRenderTarget()->QueryInterface(__uuidof(ID2D1DeviceContext),
                                      reinterpret_cast<void**>(&pID2D1DeviceContext));

    if (!pID2D1DeviceContext)
    {
        // TODO: We have *no* ID2D1DeviceContext and cannot use D2D1_COMPOSITE_MODE_XOR,
        // so there is currently no (simple?) way to solve this, there is no 'Invert' method.
        // It may be possible to convert to a WICBitmap (gets read access) and do the invert
        // there, but that needs experimenting and is probably not performant - but doable.
        increaseError();
        return;
    }

    sal::systools::COMReference<ID2D1Bitmap> pInBetweenResult;
    basegfx::B2DRange aDiscreteVisibleRange;

    // create in-between result in discrete coordinates, clipped against visible
    // part of ViewInformation (if available)
    if (!createBitmapSubContent(pInBetweenResult, aDiscreteVisibleRange,
                                rInvertPrimitive2D.getChildren(), getViewInformation2D(),
                                getRenderTarget()))
    {
        // return of false means no display needed, return
        return;
    }

    bool bDone(false);

    if (pInBetweenResult)
    {
        getRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
        const D2D1_POINT_2F aTopLeft = { FLOAT(floor(aDiscreteVisibleRange.getMinX())),
                                         FLOAT(floor(aDiscreteVisibleRange.getMinY())) };

        pID2D1DeviceContext->DrawImage(pInBetweenResult, aTopLeft, D2D1_INTERPOLATION_MODE_LINEAR,
                                       D2D1_COMPOSITE_MODE_XOR);
        bDone = true;
    }

    if (!bDone)
        increaseError();
}

void D2DPixelProcessor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
{
    if (0 == mnRecursionCounter)
        getRenderTarget()->BeginDraw();
    mnRecursionCounter++;

    switch (rCandidate.getPrimitive2DID())
    {
        // Geometry that *has* to be processed
        //
        // These Primitives have *no* decompose implementation, so these are the basic ones
        // Just four to go to make a processor work completely (but not optimized)
        // NOTE: This *could* theoretically be reduced to one and all could implement
        //       a decompose to pixel data, but that seemed not to make sense to me when
        //       I designed this. Thus these four are the lowest-level best representation
        //       from my POV
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

        // Embedding/groups that *have* to be processed
        //
        // These represent qualifiers for freely defined content, e.g. making
        // any combination of primitives freely transformed or transparent
        // NOTE: PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D and
        //       PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D are pretty much default-
        //       implementations that can and are re-used in all processors.
        // So - with these and PRIMITIVE2D_ID_INVERTPRIMITIVE2D marked to
        // be removed in the future - just three really to be implemented
        // locally specifically
        case PRIMITIVE2D_ID_TRANSPARENCEPRIMITIVE2D:
        {
            processTransparencePrimitive2D(
                static_cast<const primitive2d::TransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_INVERTPRIMITIVE2D:
        {
            // We urgently should get rid of XOR paint, modern graphic systems
            // allow no read access to the pixel targets, but that's naturally
            // a precondition for XOR. While we can do that for the office's
            // visualization, we can in principle *not* fully avoid getting
            // stuff that needs/defines XOR paint, e.g. EMF/WMF imports, so
            // we *have* to support it (for now - sigh)...
            processInvertPrimitive2D(
                static_cast<const primitive2d::InvertPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MASKPRIMITIVE2D:
        {
            processMaskPrimitive2DPixel(
                static_cast<const primitive2d::MaskPrimitive2D&>(rCandidate));
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

        // Geometry that *may* be processed due to being able to do it better
        // then using the decomposition.
        // NOTE: In these implementations you could always call what the default
        //       case below does - call process(rCandidate) to use the decomposition.
        //       So these impls should only do something if they can do it better/
        //       faster that the decomposition. So some of them check if they could
        //       - and if not - use exactly that.
        case PRIMITIVE2D_ID_UNIFIEDTRANSPARENCEPRIMITIVE2D:
        {
            // transparence with a fixed alpha for all content, can be done
            // significally faster
            processUnifiedTransparencePrimitive2D(
                static_cast<const primitive2d::UnifiedTransparencePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D:
        {
            // can be done simpler and without AA better locally
            processMarkerArrayPrimitive2D(
                static_cast<const primitive2d::MarkerArrayPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_BACKGROUNDCOLORPRIMITIVE2D:
        {
            // reset to a color, can be done more effectively locally, would
            // else decompose to a polygon fill
            processBackgroundColorPrimitive2D(
                static_cast<const primitive2d::BackgroundColorPrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_POLYGONSTROKEPRIMITIVE2D:
        {
            // fat and stroked lines - much better doable locally, would decompose
            // to the full line geometry creation (tessellation)
            processPolygonStrokePrimitive2D(
                static_cast<const primitive2d::PolygonStrokePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_LINERECTANGLEPRIMITIVE2D:
        {
            // simple primitive to support future fast callbacks from OutputDevice
            // (see 'Example POC' in Gerrit), decomposes to polygon primitive
            processLineRectanglePrimitive2D(
                static_cast<const primitive2d::LineRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_FILLEDRECTANGLEPRIMITIVE2D:
        {
            // simple primitive to support future fast callbacks from OutputDevice
            // (see 'Example POC' in Gerrit), decomposes to filled polygon primitive
            processFilledRectanglePrimitive2D(
                static_cast<const primitive2d::FilledRectanglePrimitive2D&>(rCandidate));
            break;
        }
        case PRIMITIVE2D_ID_SINGLELINEPRIMITIVE2D:
        {
            // simple primitive to support future fast callbacks from OutputDevice
            // (see 'Example POC' in Gerrit), decomposes to polygon primitive
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

        // continue with decompose as fallback
        default:
        {
            SAL_INFO("drawinglayer", "default case for " << drawinglayer::primitive2d::idToString(
                                         rCandidate.getPrimitive2DID()));
            // process recursively
            process(rCandidate);
            break;
        }
    }

    mnRecursionCounter--;
    if (0 == mnRecursionCounter)
        getRenderTarget()->EndDraw();
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
