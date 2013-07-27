/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>
#include <drawinglayer/animation/animationtiming.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/animatedprimitive2d.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

//////////////////////////////////////////////////////////////////////////////
// helper class for animated graphics

#include <vcl/animate.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>

//////////////////////////////////////////////////////////////////////////////
// includes for testing MetafilePrimitive2D::create2DDecomposition

// this switch defines if the test code is included or not
#undef USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE

#ifdef USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
#include <vcl/gradient.hxx>
#include <vcl/pngread.hxx>
#include <vcl/lineinfo.hxx>
#endif // USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE

//////////////////////////////////////////////////////////////////////////////

namespace
{
    struct animationStep
    {
        BitmapEx                                maBitmapEx;
        sal_uInt32                              mnTime;
    };

    class animatedBitmapExPreparator
    {
        ::Animation                             maAnimation;
        ::std::vector< animationStep >          maSteps;

        sal_uInt32 generateStepTime(sal_uInt32 nIndex) const;

    public:
        animatedBitmapExPreparator(const Graphic& rGraphic);

        sal_uInt32 count() const { return maSteps.size(); }
        sal_uInt32 loopCount() const { return (sal_uInt32)maAnimation.GetLoopCount(); }
        sal_uInt32 stepTime(sal_uInt32 a) const { return maSteps[a].mnTime; }
        const BitmapEx& stepBitmapEx(sal_uInt32 a) const { return maSteps[a].maBitmapEx; }
    };

    sal_uInt32 animatedBitmapExPreparator::generateStepTime(sal_uInt32 nIndex) const
    {
        const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(nIndex));
        sal_uInt32 nWaitTime(rAnimBitmap.nWait * 10);

        // #115934#
        // Take care of special value for MultiPage TIFFs. ATM these shall just
        // show their first page. Later we will offer some switching when object
        // is selected.
        if(ANIMATION_TIMEOUT_ON_CLICK == rAnimBitmap.nWait)
        {
            // ATM the huge value would block the timer, so
            // use a long time to show first page (whole day)
            nWaitTime = 100 * 60 * 60 * 24;
        }

        // Bad trap: There are animated gifs with no set WaitTime (!).
        // In that case use a default value.
        if(0L == nWaitTime)
        {
            nWaitTime = 100L;
        }

        return nWaitTime;
    }

    animatedBitmapExPreparator::animatedBitmapExPreparator(const Graphic& rGraphic)
    :   maAnimation(rGraphic.GetAnimation())
    {
        OSL_ENSURE(GRAPHIC_BITMAP == rGraphic.GetType() && rGraphic.IsAnimated(), "animatedBitmapExPreparator: graphic is not animated (!)");

        // #128539# secure access to Animation, looks like there exist animated GIFs out there
        // with a step count of zero
        if(maAnimation.Count())
        {
            VirtualDevice aVirtualDevice(*Application::GetDefaultDevice());
            VirtualDevice aVirtualDeviceMask(*Application::GetDefaultDevice(), 1L);

            // Prepare VirtualDevices and their states
            aVirtualDevice.EnableMapMode(sal_False);
            aVirtualDeviceMask.EnableMapMode(sal_False);
            aVirtualDevice.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDeviceMask.SetOutputSizePixel(maAnimation.GetDisplaySizePixel());
            aVirtualDevice.Erase();
            aVirtualDeviceMask.Erase();

            for(sal_uInt16 a(0L); a < maAnimation.Count(); a++)
            {
                animationStep aNextStep;
                aNextStep.mnTime = generateStepTime(a);

                // prepare step
                const AnimationBitmap& rAnimBitmap = maAnimation.Get(sal_uInt16(a));

                switch(rAnimBitmap.eDisposal)
                {
                    case DISPOSE_NOT:
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        Bitmap aMask = rAnimBitmap.aBmpEx.GetMask();

                        if(aMask.IsEmpty())
                        {
                            const Point aEmpty;
                            const Rectangle aRect(aEmpty, aVirtualDeviceMask.GetOutputSizePixel());
                            const Wallpaper aWallpaper(COL_BLACK);
                            aVirtualDeviceMask.DrawWallpaper(aRect, aWallpaper);
                        }
                        else
                        {
                            BitmapEx aExpandVisibilityMask = BitmapEx(aMask, aMask);
                            aVirtualDeviceMask.DrawBitmapEx(rAnimBitmap.aPosPix, aExpandVisibilityMask);
                        }

                        break;
                    }
                    case DISPOSE_BACK:
                    {
                        // #i70772# react on no mask, for primitives, too.
                        const Bitmap aMask(rAnimBitmap.aBmpEx.GetMask());
                        const Bitmap aContent(rAnimBitmap.aBmpEx.GetBitmap());

                        aVirtualDeviceMask.Erase();
                        aVirtualDevice.DrawBitmap(rAnimBitmap.aPosPix, aContent);

                        if(aMask.IsEmpty())
                        {
                            const Rectangle aRect(rAnimBitmap.aPosPix, aContent.GetSizePixel());
                            aVirtualDeviceMask.SetFillColor(COL_BLACK);
                            aVirtualDeviceMask.SetLineColor();
                            aVirtualDeviceMask.DrawRect(aRect);
                        }
                        else
                        {
                            aVirtualDeviceMask.DrawBitmap(rAnimBitmap.aPosPix, aMask);
                        }

                        break;
                    }
                    case DISPOSE_FULL:
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        break;
                    }
                    case DISPOSE_PREVIOUS :
                    {
                        aVirtualDevice.DrawBitmapEx(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx);
                        aVirtualDeviceMask.DrawBitmap(rAnimBitmap.aPosPix, rAnimBitmap.aBmpEx.GetMask());
                        break;
                    }
                }

                // create BitmapEx
                Bitmap aMainBitmap = aVirtualDevice.GetBitmap(Point(), aVirtualDevice.GetOutputSizePixel());
#if defined(MACOSX)
                AlphaMask aMaskBitmap( aVirtualDeviceMask.GetBitmap( Point(), aVirtualDeviceMask.GetOutputSizePixel()));
#else
                Bitmap aMaskBitmap = aVirtualDeviceMask.GetBitmap( Point(), aVirtualDeviceMask.GetOutputSizePixel());
#endif
                aNextStep.maBitmapEx = BitmapEx(aMainBitmap, aMaskBitmap);

                // add to vector
                maSteps.push_back(aNextStep);
            }
        }
    }
} // end of anonymous namespace

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence create2DDecompositionOfGraphic(
            const Graphic& rGraphic,
            const basegfx::B2DHomMatrix& rTransform)
        {
            Primitive2DSequence aRetval;

            switch(rGraphic.GetType())
            {
                case GRAPHIC_BITMAP :
                {
                    if(rGraphic.IsAnimated())
                    {
                        // prepare animation data
                        animatedBitmapExPreparator aData(rGraphic);

                        if(aData.count())
                        {
                            // create sub-primitives for animated bitmap and the needed animation loop
                            animation::AnimationEntryLoop aAnimationLoop(aData.loopCount() ? aData.loopCount() : 0xffff);
                            Primitive2DSequence aBitmapPrimitives(aData.count());

                            for(sal_uInt32 a(0); a < aData.count(); a++)
                            {
                                animation::AnimationEntryFixed aTime((double)aData.stepTime(a), (double)a / (double)aData.count());
                                aAnimationLoop.append(aTime);
                                aBitmapPrimitives[a] = new BitmapPrimitive2D(
                                    aData.stepBitmapEx(a),
                                    rTransform);
                            }

                            // prepare animation list
                            animation::AnimationEntryList aAnimationList;
                            aAnimationList.append(aAnimationLoop);

                            // create and add animated switch primitive
                            aRetval.realloc(1);
                            aRetval[0] = new AnimatedSwitchPrimitive2D(
                                aAnimationList,
                                aBitmapPrimitives,
                                false);
                        }
                    }
                    else if(rGraphic.getSvgData().get())
                    {
                        // embedded Svg fill, create embed transform
                        const basegfx::B2DRange& rSvgRange(rGraphic.getSvgData()->getRange());

                        if(basegfx::fTools::more(rSvgRange.getWidth(), 0.0) && basegfx::fTools::more(rSvgRange.getHeight(), 0.0))
                        {
                            // translate back to origin, scale to unit coordinates
                            basegfx::B2DHomMatrix aEmbedSvg(
                                basegfx::tools::createTranslateB2DHomMatrix(
                                    -rSvgRange.getMinX(),
                                    -rSvgRange.getMinY()));

                            aEmbedSvg.scale(
                                1.0 / rSvgRange.getWidth(),
                                1.0 / rSvgRange.getHeight());

                            // apply created object transformation
                            aEmbedSvg = rTransform * aEmbedSvg;

                            // add Svg primitives embedded
                            aRetval.realloc(1);
                            aRetval[0] = new TransformPrimitive2D(
                                aEmbedSvg,
                                rGraphic.getSvgData()->getPrimitive2DSequence());
                        }
                    }
                    else
                    {
                        aRetval.realloc(1);
                        aRetval[0] = new BitmapPrimitive2D(
                            rGraphic.GetBitmapEx(),
                            rTransform);
                    }

                    break;
                }

                case GRAPHIC_GDIMETAFILE :
                {
#ifdef USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
                    static bool bDoTest(false);

                    if(bDoTest)
                    {
                        // All this is/was test code for testing MetafilePrimitive2D::create2DDecomposition
                        // extensively. It may be needed again when diverse actions need debugging, so i leave
                        // it in here, but take it out using USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE.
                        // Use it by compiling with the code, insert any DrawObject, convert to Metafile. The
                        // debugger will then stop here (when breakpoint set, of course). You may enter single
                        // parts of actions and/or change to true what You want to check.
                        GDIMetaFile aMtf;
                        VirtualDevice aOut;
                        const basegfx::B2DRange aRange(getB2DRange(rViewInformation));
                        const Rectangle aRectangle(
                            basegfx::fround(aRange.getMinX()), basegfx::fround(aRange.getMinY()),
                            basegfx::fround(aRange.getMaxX()), basegfx::fround(aRange.getMaxY()));
                        const Point aOrigin(aRectangle.TopLeft());
                        const Fraction aScaleX(aRectangle.getWidth());
                        const Fraction aScaleY(aRectangle.getHeight());
                        MapMode aMapMode(MAP_100TH_MM, aOrigin, aScaleX, aScaleY);

                        Size aDummySize(2, 2);
                        aOut.SetOutputSizePixel(aDummySize);
                        aOut.EnableOutput(FALSE);
                        aOut.SetMapMode(aMapMode);

                        aMtf.Clear();
                        aMtf.Record(&aOut);

                        const Fraction aNeutralFraction(1, 1);
                        const MapMode aRelativeMapMode(
                            MAP_RELATIVE,
                            Point(-aRectangle.Left(), -aRectangle.Top()),
                            aNeutralFraction, aNeutralFraction);
                        aOut.SetMapMode(aRelativeMapMode);

                        if(false)
                        {
                            const sal_Int32 nHor(aRectangle.getWidth() / 4);
                            const sal_Int32 nVer(aRectangle.getHeight() / 4);
                            const Rectangle aCenteredRectangle(
                                aRectangle.Left() + nHor, aRectangle.Top() + nVer,
                                aRectangle.Right() - nHor, aRectangle.Bottom() - nVer);
                            aOut.SetClipRegion(aCenteredRectangle);
                        }

                        if(false)
                        {
                            const Rectangle aRightRectangle(aRectangle.TopCenter(), aRectangle.BottomRight());
                            aOut.IntersectClipRegion(aRightRectangle);
                        }

                        if(false)
                        {
                            const Rectangle aRightRectangle(aRectangle.TopCenter(), aRectangle.BottomRight());
                            const Rectangle aBottomRectangle(aRectangle.LeftCenter(), aRectangle.BottomRight());
                            Region aRegion(aRightRectangle);
                            aRegion.Intersect(aBottomRectangle);
                            aOut.IntersectClipRegion(aRegion);
                        }

                        if(false)
                        {
                            const sal_Int32 nHor(aRectangle.getWidth() / 10);
                            const sal_Int32 nVer(aRectangle.getHeight() / 10);
                            aOut.MoveClipRegion(nHor, nVer);
                        }

                        if(false)
                        {
                            Wallpaper aWallpaper(Color(COL_BLACK));
                            aOut.DrawWallpaper(aRectangle, aWallpaper);
                        }

                        if(false)
                        {
                            Wallpaper aWallpaper(Gradient(GRADIENT_LINEAR, Color(COL_RED), Color(COL_GREEN)));
                            aOut.DrawWallpaper(aRectangle, aWallpaper);
                        }

                        if(false)
                        {
                            SvFileStream aRead((const String&)String(ByteString( "c:\\test.png" ), RTL_TEXTENCODING_UTF8), STREAM_READ);
                            vcl::PNGReader aPNGReader(aRead);
                            BitmapEx aBitmapEx(aPNGReader.Read());
                            Wallpaper aWallpaper(aBitmapEx);
                            aOut.DrawWallpaper(aRectangle, aWallpaper);
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());
                            Color aColor(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0));

                            for(sal_uInt32 a(0); a < 5000; a++)
                            {
                                const Point aPoint(
                                    aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                    aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));

                                if(!(a % 3))
                                {
                                    aColor = Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0));
                                }

                                aOut.DrawPixel(aPoint, aColor);
                            }
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());

                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor();

                            for(sal_uInt32 a(0); a < 5000; a++)
                            {
                                const Point aPoint(
                                    aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                    aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                aOut.DrawPixel(aPoint);
                            }
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());

                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor();

                            Point aStart(
                                aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                            Point aStop(
                                aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));

                            LineInfo aLineInfo(LINE_SOLID, basegfx::fround(fHor / 50.0));
                            bool bUseLineInfo(false);

                            for(sal_uInt32 a(0); a < 20; a++)
                            {
                                if(!(a%6))
                                {
                                    bUseLineInfo = !bUseLineInfo;
                                }

                                if(!(a%4))
                                {
                                    aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                                }

                                if(a%3)
                                {
                                    aStart = aStop;
                                    aStop = Point(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                }
                                else
                                {
                                    aStart = Point(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                    aStop = Point(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                }

                                if(bUseLineInfo)
                                {
                                    aOut.DrawLine(aStart, aStop, aLineInfo);
                                }
                                else
                                {
                                    aOut.DrawLine(aStart, aStop);
                                }
                            }
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawRect(aRectangle);
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            const sal_uInt32 nHor(aRectangle.getWidth() / 10);
                            const sal_uInt32 nVer(aRectangle.getHeight() / 10);
                            aOut.DrawRect(aRectangle, nHor, nVer);
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawEllipse(aRectangle);
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawArc(aRectangle, aRectangle.TopLeft(), aRectangle.BottomCenter());
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawPie(aRectangle, aRectangle.TopLeft(), aRectangle.BottomCenter());
                        }

                        if(false)
                        {
                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawChord(aRectangle, aRectangle.TopLeft(), aRectangle.BottomCenter());
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());

                            for(sal_uInt32 b(0); b < 5; b++)
                            {
                                const sal_uInt32 nCount(basegfx::fround(rand() * (20 / 32767.0)));
                                const bool bClose(basegfx::fround(rand() / 32767.0));
                                Polygon aPolygon(nCount + (bClose ? 1 : 0));

                                for(sal_uInt32 a(0); a < nCount; a++)
                                {
                                    const Point aPoint(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                    aPolygon[a] = aPoint;
                                }

                                if(bClose)
                                {
                                    aPolygon[aPolygon.GetSize() - 1] = aPolygon[0];
                                }

                                aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                                aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));

                                if(!(b%2))
                                {
                                    const LineInfo aLineInfo(LINE_SOLID, basegfx::fround(fHor / 50.0));
                                    aOut.DrawPolyLine(aPolygon, aLineInfo);
                                }
                                else
                                {
                                    aOut.DrawPolyLine(aPolygon);
                                }
                            }
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());

                            for(sal_uInt32 b(0); b < 5; b++)
                            {
                                const sal_uInt32 nCount(basegfx::fround(rand() * (20 / 32767.0)));
                                const bool bClose(basegfx::fround(rand() / 32767.0));
                                Polygon aPolygon(nCount + (bClose ? 1 : 0));

                                for(sal_uInt32 a(0); a < nCount; a++)
                                {
                                    const Point aPoint(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                    aPolygon[a] = aPoint;
                                }

                                if(bClose)
                                {
                                    aPolygon[aPolygon.GetSize() - 1] = aPolygon[0];
                                }

                                aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                                aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                                aOut.DrawPolygon(aPolygon);
                            }
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());
                            PolyPolygon aPolyPolygon;

                            for(sal_uInt32 b(0); b < 3; b++)
                            {
                                const sal_uInt32 nCount(basegfx::fround(rand() * (6 / 32767.0)));
                                const bool bClose(basegfx::fround(rand() / 32767.0));
                                Polygon aPolygon(nCount + (bClose ? 1 : 0));

                                for(sal_uInt32 a(0); a < nCount; a++)
                                {
                                    const Point aPoint(
                                        aRectangle.Left() + basegfx::fround(rand() * (fHor / 32767.0)),
                                        aRectangle.Top() + basegfx::fround(rand() * (fVer / 32767.0)));
                                    aPolygon[a] = aPoint;
                                }

                                if(bClose)
                                {
                                    aPolygon[aPolygon.GetSize() - 1] = aPolygon[0];
                                }

                                aPolyPolygon.Insert(aPolygon);
                            }

                            aOut.SetLineColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.SetFillColor(Color(basegfx::BColor(rand() / 32767.0, rand() / 32767.0, rand() / 32767.0)));
                            aOut.DrawPolyPolygon(aPolyPolygon);
                        }

                        if(false)
                        {
                            SvFileStream aRead((const String&)String(ByteString( "c:\\test.png" ), RTL_TEXTENCODING_UTF8), STREAM_READ);
                            vcl::PNGReader aPNGReader(aRead);
                            BitmapEx aBitmapEx(aPNGReader.Read());
                            aOut.DrawBitmapEx(aRectangle.TopLeft(), aBitmapEx);
                        }

                        if(false)
                        {
                            SvFileStream aRead((const String&)String(ByteString( "c:\\test.png" ), RTL_TEXTENCODING_UTF8), STREAM_READ);
                            vcl::PNGReader aPNGReader(aRead);
                            BitmapEx aBitmapEx(aPNGReader.Read());
                            aOut.DrawBitmapEx(aRectangle.TopLeft(), aRectangle.GetSize(), aBitmapEx);
                        }

                        if(false)
                        {
                            SvFileStream aRead((const String&)String(ByteString( "c:\\test.png" ), RTL_TEXTENCODING_UTF8), STREAM_READ);
                            vcl::PNGReader aPNGReader(aRead);
                            BitmapEx aBitmapEx(aPNGReader.Read());
                            const Size aSizePixel(aBitmapEx.GetSizePixel());
                            aOut.DrawBitmapEx(
                                aRectangle.TopLeft(),
                                aRectangle.GetSize(),
                                Point(0, 0),
                                Size(aSizePixel.Width() /2, aSizePixel.Height() / 2),
                                aBitmapEx);
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());
                            const Point aPointA(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.3));
                            const Point aPointB(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.5));
                            const Point aPointC(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.7));
                            const String aText(ByteString("Hello, World!"), RTL_TEXTENCODING_UTF8);

                            const String aFontName(ByteString("Comic Sans MS"), RTL_TEXTENCODING_UTF8);
                            Font aFont(aFontName, Size(0, 1000));
                            aFont.SetAlign(ALIGN_BASELINE);
                            aFont.SetColor(COL_RED);
                            //sal_Int32* pDXArray = new sal_Int32[aText.Len()];

                            aFont.SetOutline(true);
                            aOut.SetFont(aFont);
                            aOut.DrawText(aPointA, aText, 0, aText.Len());

                            aFont.SetShadow(true);
                            aOut.SetFont(aFont);
                            aOut.DrawText(aPointB, aText, 0, aText.Len());

                            aFont.SetRelief(RELIEF_EMBOSSED);
                            aOut.SetFont(aFont);
                            aOut.DrawText(aPointC, aText, 0, aText.Len());

                            //delete pDXArray;
                        }

                        if(false)
                        {
                            const double fHor(aRectangle.getWidth());
                            const double fVer(aRectangle.getHeight());
                            const Point aPointA(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.3));
                            const Point aPointB(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.5));
                            const Point aPointC(
                                aRectangle.Left() + basegfx::fround(fHor * 0.2),
                                aRectangle.Top() + basegfx::fround(fVer * 0.7));
                            const String aText(ByteString("Hello, World!"), RTL_TEXTENCODING_UTF8);

                            const String aFontName(ByteString("Comic Sans MS"), RTL_TEXTENCODING_UTF8);
                            Font aFont(aFontName, Size(0, 1000));
                            aFont.SetAlign(ALIGN_BASELINE);
                            aFont.SetColor(COL_RED);

                            aOut.SetFont(aFont);
                            const sal_Int32 nWidth(aOut.GetTextWidth(aText, 0, aText.Len()));
                            aOut.DrawText(aPointA, aText, 0, aText.Len());
                            aOut.DrawTextLine(aPointA, nWidth, STRIKEOUT_SINGLE, UNDERLINE_SINGLE, UNDERLINE_SMALLWAVE);
                            aOut.DrawTextLine(aPointB, nWidth, STRIKEOUT_SINGLE, UNDERLINE_SINGLE, UNDERLINE_SMALLWAVE);
                            aOut.DrawTextLine(aPointC, nWidth, STRIKEOUT_SINGLE, UNDERLINE_SINGLE, UNDERLINE_SMALLWAVE);
                        }

                        aMtf.Stop();
                        aMtf.WindStart();
                        aMtf.SetPrefMapMode(MapMode(MAP_100TH_MM));
                        aMtf.SetPrefSize(Size(aRectangle.getWidth(), aRectangle.getHeight()));

                        aRetval.realloc(1);
                        aRetval[0] = new MetafilePrimitive2D(
                            rTransform,
                            aMtf);
                    }
                    else
                    {
#endif // USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
                        // create MetafilePrimitive2D
                        const GDIMetaFile& rMetafile = rGraphic.GetGDIMetaFile();

                        aRetval.realloc(1);
                        aRetval[0] = new MetafilePrimitive2D(
                            rTransform,
                            rMetafile);

                        // #i100357# find out if clipping is needed for this primitive. Unfortunately,
                        // there exist Metafiles who's content is bigger than the proposed PrefSize set
                        // at them. This is an error, but we need to work around this
                        const Size aMetaFilePrefSize(rMetafile.GetPrefSize());
                        const Size aMetaFileRealSize(
                            const_cast< GDIMetaFile& >(rMetafile).GetBoundRect(
                                *Application::GetDefaultDevice()).GetSize());

                        if(aMetaFileRealSize.getWidth() > aMetaFilePrefSize.getWidth()
                            || aMetaFileRealSize.getHeight() > aMetaFilePrefSize.getHeight())
                        {
                            // clipping needed. Embed to MaskPrimitive2D. Create childs and mask polygon
                            basegfx::B2DPolygon aMaskPolygon(basegfx::tools::createUnitPolygon());
                            aMaskPolygon.transform(rTransform);

                            aRetval[0] = new MaskPrimitive2D(
                                basegfx::B2DPolyPolygon(aMaskPolygon),
                                aRetval);
                        }
#ifdef USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
                    }
#endif // USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE

                    break;
                }

                default:
                {
                    // nothing to create
                    break;
                }
            }

            return aRetval;
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
