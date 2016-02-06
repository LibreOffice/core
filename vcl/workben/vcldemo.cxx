/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <math.h>
#include <rtl/math.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <osl/time.h>
#include <vcl/vclmain.hxx>
#include <vcl/layout.hxx>
#include <salhelper/thread.hxx>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/pngread.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/help.hxx>
#include <vcl/menu.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vcldemo-debug.hxx>
#include <opengl/zone.hxx>

// internal headers for OpenGLTests class.
#include "salgdi.hxx"
#include "salframe.hxx"
#include "openglgdiimpl.hxx"
#include "opengl/texture.hxx"
#include "opengl/framebuffer.hxx"
#include <vcl/opengl/OpenGLHelper.hxx>

#include <rtl/math.hxx>

#define FIXME_SELF_INTERSECTING_WORKING 0
#define FIXME_BOUNCE_BUTTON 0
#define THUMB_REPEAT_FACTOR 10

using namespace com::sun::star;

namespace {
    double getTimeNow()
    {
        TimeValue aValue;
        osl_getSystemTime(&aValue);
        return (double)aValue.Seconds * 1000 +
            (double)aValue.Nanosec / (1000*1000);
    }

}

using namespace css;

enum RenderStyle {
    RENDER_THUMB,    // small view <n> to a page
    RENDER_EXPANDED, // expanded view of this renderer
};

class DemoRenderer
{
    Bitmap   maIntroBW;
    BitmapEx maIntro;

    int mnSegmentsX;
    int mnSegmentsY;

    struct RenderContext {
        RenderStyle   meStyle;
        bool          mbVDev;
        DemoRenderer *mpDemoRenderer;
        Size          maSize;
    };
    struct RegionRenderer {
    public:
        RegionRenderer() :
            sumTime(0),
            countTime(0)
        { }
        virtual ~RegionRenderer() {}
        virtual OUString getName() = 0;
        virtual sal_uInt16 getAccelerator() = 0;
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) = 0;
        // repeating count for profiling (to exceed the poor time resolution on Windows)
        virtual sal_uInt16 getTestRepeatCount() = 0;
#define RENDER_DETAILS(name,key,repeat) \
        virtual OUString getName() override \
            { return OUString(SAL_STRINGIFY(name)); } \
        virtual sal_uInt16 getAccelerator() override \
            { return key; } \
        virtual sal_uInt16 getTestRepeatCount() override \
            { return repeat; }

        double sumTime;
        int countTime;
    };

    std::vector< RegionRenderer * > maRenderers;
    sal_Int32  mnSelectedRenderer;
    sal_Int32  iterCount;

    void     InitRenderers();

public:
    DemoRenderer() : mnSegmentsX(0)
                   , mnSegmentsY(0)
                   , mnSelectedRenderer(-1)
                   , iterCount(0)
#if FIXME_BOUNCE_BUTTON
                   , mpButton(NULL)
                   , mpButtonWin(NULL)
                   , mnBounceX(1)
                   , mnBounceY(1)
#endif
    {
        if (!Application::LoadBrandBitmap("intro", maIntro))
            Application::Abort("Failed to load intro image");

        maIntroBW = maIntro.GetBitmap();
        maIntroBW.Filter(BMP_FILTER_EMBOSS_GREY);

        InitRenderers();
        mnSegmentsX = rtl::math::round(std::sqrt(maRenderers.size()), 0,
                                       rtl_math_RoundingMode_Up);
        mnSegmentsY = rtl::math::round(std::sqrt(maRenderers.size()), 0,
                                       rtl_math_RoundingMode_Down);
        mnSegmentsY = floor(std::sqrt(maRenderers.size()));
    }

    OUString getRendererList();
    double   getAndResetBenchmark(RenderStyle style);
    void     selectRenderer(const OUString &rName);
    int      selectNextRenderer();
    void     setIterCount(sal_Int32 iterCount);
    sal_Int32 getIterCount();
    void     addTime(int i, double t);

    Size maSize;
    void SetSizePixel(const Size &rSize) { maSize = rSize; }
    Size GetSizePixel() const            { return maSize;  }


// more of a 'Window' concept - push upwards ?
#if FIXME_BOUNCE_BUTTON
    // Bouncing windows on click ...
    PushButton     *mpButton;
    FloatingWindow *mpButtonWin;
    AutoTimer       maBounce;
    int             mnBounceX, mnBounceY;
    DECL_LINK_TYPED(BounceTimerCb, Timer*, void);
#endif

    bool MouseButtonDown(const MouseEvent& rMEvt);
    void KeyInput(const KeyEvent& rKEvt);

    static std::vector<Rectangle> partition(const RenderContext &rCtx, int nX, int nY)
    {
        return partition(rCtx.maSize, nX, nY);
    }

    static std::vector<Rectangle> partition(Size aSize, int nX, int nY)
    {
        Rectangle r;
        std::vector<Rectangle> aRegions;

        // Make small cleared area for these guys
        long nBorderSize = aSize.Width() / 32;
        long nBoxWidth = (aSize.Width() - nBorderSize*(nX+1)) / nX;
        long nBoxHeight = (aSize.Height() - nBorderSize*(nY+1)) / nY;
        for (int y = 0; y < nY; y++)
        {
            for (int x = 0; x < nX; x++)
            {
                r.SetPos(Point(nBorderSize + (nBorderSize + nBoxWidth) * x,
                               nBorderSize + (nBorderSize + nBoxHeight) * y));
                r.SetSize(Size(nBoxWidth, nBoxHeight));
                aRegions.push_back(r);
            }
        }

        return aRegions;
    }

    static void clearRects(OutputDevice &rDev, std::vector<Rectangle> &rRects)
    {
        for (size_t i = 0; i < rRects.size(); i++)
        {
            // knock up a nice little border
            rDev.SetLineColor(COL_GRAY);
            rDev.SetFillColor(COL_LIGHTGRAY);
            if (i % 2)
            {
                int nBorderSize = rRects[i].GetWidth() / 5;
                rDev.DrawRect(rRects[i], nBorderSize, nBorderSize);
            }
            else
                rDev.DrawRect(rRects[i]);
        }
    }

    static void drawBackground(OutputDevice &rDev, const Rectangle& r)
    {
        rDev.Erase();
        Gradient aGradient;
        aGradient.SetStartColor(COL_BLUE);
        aGradient.SetEndColor(COL_GREEN);
        aGradient.SetStyle(GradientStyle_LINEAR);
        rDev.DrawGradient(r, aGradient);
    }

    struct DrawLines : public RegionRenderer
    {
        RENDER_DETAILS(lines,KEY_L,100)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                AntialiasingFlags nOldAA = rDev.GetAntialiasing();
                rDev.SetAntialiasing(AntialiasingFlags::EnableB2dDraw);

                std::vector<Rectangle> aRegions(DemoRenderer::partition(rCtx, 4, 4));
                DemoRenderer::clearRects(rDev, aRegions);

#if 0 // FIXME: get this through to the backend ...
                double nTransparency[] = {
                    1.0, 1.0, 1.0, 1.0,
                    0.8, 0.8, 0.8, 0.8,
                    0.5, 0.5, 0.5, 0.5,
                    0.1, 0.1, 0.1, 0.1
                };
#endif
                drawing::LineCap eLineCaps[] = {
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT
                };
                ::basegfx::B2DLineJoin eJoins[] = {
                    basegfx::B2DLineJoin::NONE,  basegfx::B2DLineJoin::Middle, basegfx::B2DLineJoin::Bevel,  basegfx::B2DLineJoin::Miter,
                    basegfx::B2DLineJoin::Round, basegfx::B2DLineJoin::NONE,   basegfx::B2DLineJoin::Middle, basegfx::B2DLineJoin::Bevel,
                    basegfx::B2DLineJoin::Miter, basegfx::B2DLineJoin::Round,  basegfx::B2DLineJoin::NONE,   basegfx::B2DLineJoin::Middle,
                    basegfx::B2DLineJoin::Bevel, basegfx::B2DLineJoin::Miter,  basegfx::B2DLineJoin::Round,  basegfx::B2DLineJoin::NONE
                };
                double aLineWidths[] = {
                    10.0, 15.0, 20.0, 10.0,
                    10.0, 15.0, 20.0, 10.0,
                    10.0, 15.0, 20.0, 10.0,
                     0.1,  1.0, 10.0, 50.0
                };
                for (size_t i = 0; i < aRegions.size(); i++)
                {
                    // Half of them not-anti-aliased ..
                    if (i >= aRegions.size()/2)
                        rDev.SetAntialiasing(nOldAA);

                    static const struct {
                        double nX, nY;
                    } aPoints[] = {
                        { 0.2, 0.2 }, { 0.8, 0.3 }, { 0.7, 0.8 }
                    };
                    rDev.SetLineColor(Color(COL_BLACK));
                    basegfx::B2DPolygon aPoly;
                    Rectangle aSub(aRegions[i]);
                    for (size_t j = 0; j < SAL_N_ELEMENTS(aPoints); j++)
                    {
                        aPoly.append(basegfx::B2DPoint(aSub.Left() + aSub.GetWidth() * aPoints[j].nX,
                                                       aSub.Top()  + aSub.GetHeight() * aPoints[j].nY));
                    }
                    rDev.DrawPolyLine(aPoly, aLineWidths[i], eJoins[i], eLineCaps[i]);
                }
            }
            else
            {
                rDev.SetFillColor(Color(COL_LIGHTRED));
                rDev.SetLineColor(Color(COL_BLACK));
                rDev.DrawRect(r);

                for(long i=0; i<r.GetHeight(); i+=15)
                    rDev.DrawLine(Point(r.Left(), r.Top()+i), Point(r.Right(), r.Bottom()-i));
                for(long i=0; i<r.GetWidth(); i+=15)
                    rDev.DrawLine(Point(r.Left()+i, r.Bottom()), Point(r.Right()-i, r.Top()));

                // Should draw a white-line across the middle
                Color aLastPixel(COL_WHITE);
                Point aCenter((r.Left() + r.Right())/2 - 4,
                              (r.Top() + r.Bottom())/2 - 4);
                for(int i=0; i<8; i++)
                {
                    rDev.DrawPixel(aCenter, aLastPixel);
                    aLastPixel = rDev.GetPixel(aCenter);
                    aCenter.Move(1,1);
                }
            }
        }
    };

    struct DrawText : public RegionRenderer
    {
        RENDER_DETAILS(text,KEY_T,1)

        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<Rectangle> aRegions(DemoRenderer::partition(rCtx, 4, 2));
                DemoRenderer::clearRects(rDev, aRegions);

                bool bClip=true, bArabicText=true, bRotate=true;

                int nRegions=0;

                for (int nClipRow=0; nClipRow < 2; nClipRow++)
                {
                    if (!bArabicText)
                        bArabicText=true;

                    for (int nArabicRow=0; nArabicRow < 2; nArabicRow++)
                    {
                        if (!bRotate)
                            bRotate=true;

                        for (int nRotateRow=0; nRotateRow < 2; nRotateRow++)
                        {
                            drawText( rDev, aRegions[nRegions], bClip, bArabicText, bRotate );

                            nRegions++;
                            bRotate=false;
                        }

                        bArabicText=false;
                    }

                    bClip=false;
                }
            }
            else
            {
                drawText(rDev, r, false, false, false);
            }
        }

        static void drawText (OutputDevice &rDev, Rectangle r, bool bClip, bool bArabicText, bool bRotate)
        {
            rDev.SetClipRegion( vcl::Region(r) );

            OUString aLatinText("Click any rect to zoom!!!!");

            const unsigned char pTextUTF8[] = {
                0xd9, 0x88, 0xd8, 0xa7, 0xd8, 0xad, 0xd9, 0x90,
                0xd8, 0xaf, 0xd9, 0x92, 0x20, 0xd8, 0xa5, 0xd8,
                0xab, 0xd9, 0x8d, 0xd9, 0x86, 0xd9, 0x8a, 0xd9,
                0x86, 0x20, 0xd8, 0xab, 0xd9, 0x84, 0xd8, 0xa7,
                0xd8, 0xab, 0xd8, 0xa9, 0xd9, 0x8c, 0x00
            };
            OUString aArabicText( reinterpret_cast<char const *>(pTextUTF8),
                            SAL_N_ELEMENTS( pTextUTF8 ) - 1,
                            RTL_TEXTENCODING_UTF8 );

            OUString aText;

            // To have more text displayed one after the other (overlapping, and in different colours), then
            // change this value
            const int nPrintNumCopies=1;

            if (bArabicText)
                aText = aArabicText;
            else
                aText = aLatinText;

            std::vector<OUString> maFontNames;

            sal_uInt32 nCols[] = {
                COL_BLACK, COL_BLUE, COL_GREEN, COL_CYAN, COL_RED, COL_MAGENTA,
                COL_BROWN, COL_GRAY, COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTGREEN,
                COL_LIGHTCYAN, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_YELLOW, COL_WHITE
            };

            // a few fonts to start with
            const char *pNames[] = {
                "Times", "Liberation Sans", "Arial", "Linux Biolinum G", "Linux Libertine Display G"
              };

            size_t nNumFontNames = SAL_N_ELEMENTS(pNames);

            for (size_t i = 0; i < nNumFontNames; i++)
                maFontNames.push_back(OUString::createFromAscii(pNames[i]));

            if (bClip && !bRotate)
            {
                // only show the first quarter of the text
                Rectangle aRect( r.TopLeft(), Size( r.GetWidth()/2, r.GetHeight()/2 ) );
                rDev.SetClipRegion( vcl::Region( aRect ) );
            }

            for (int i = 1; i < nPrintNumCopies+1; i++)
            {
                int nFontHeight=0, nFontIndex=0, nFontColorIndex=0;

                if (nPrintNumCopies == 1)
                {
                    float nFontMagnitude = 0.25f;
                    // random font size to avoid buffering
                    nFontHeight = 1 + nFontMagnitude * (0.9 + comphelper::rng::uniform_real_distribution(0.0, std::nextafter(0.1, DBL_MAX))) * (r.Bottom() - r.Top());
                    nFontIndex=0;
                    nFontColorIndex=0;
                }
                else
                {
                    // random font size to avoid buffering
                    nFontHeight = 1 + i * (0.9 + comphelper::rng::uniform_real_distribution(0.0, std::nextafter(0.1, DBL_MAX))) * (r.Top() - r.Bottom()) / nPrintNumCopies;
                    nFontIndex = (i % maFontNames.size());
                    nFontColorIndex=(i % maFontNames.size());
                }

                rDev.SetTextColor(Color(nCols[nFontColorIndex]));
                vcl::Font aFont( maFontNames[nFontIndex], Size(0, nFontHeight ));

                if (bRotate)
                {
                    Rectangle aFontRect = r;

                    int nHeight = r.GetHeight();

                    // move the text to the bottom of the bounding rect before rotating
                    aFontRect.Top() += nHeight;
                    aFontRect.Bottom() += nHeight;

                    int nDegrees = 45;

                    aFont.SetOrientation(nDegrees * 10);

                    rDev.SetFont(aFont);
                    rDev.DrawText(aFontRect, aText);

                    if (bClip)
                    {
                        Rectangle aClipRect( Point( r.Left(), r.Top() + ( r.GetHeight()/2 ) ) , Size( r.GetWidth()/2, r.GetHeight()/2 ) );
                        rDev.SetClipRegion( vcl::Region( aClipRect ) );
                    }
                    else
                        rDev.SetClipRegion( vcl::Region(r) );
                }
                else
                {
                    rDev.SetFont(aFont);
                    rDev.DrawText(r, aText);
                }
            }

            rDev.SetClipRegion();
        }
    };

    struct DrawCheckered : public RegionRenderer
    {
        RENDER_DETAILS(checks,KEY_C,20)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<Rectangle> aRegions(DemoRenderer::partition(rCtx, 2, 2));
                for (size_t i = 0; i < aRegions.size(); i++)
                {
                    vcl::Region aRegion;
                    Rectangle aSub(aRegions[i]);
                    Rectangle aSmaller(aSub);
                    aSmaller.Move(10,10);
                    aSmaller.setWidth(aSmaller.getWidth()-20);
                    aSmaller.setHeight(aSmaller.getHeight()-24);
                    switch (i) {
                    case 0:
                        aRegion = vcl::Region(aSub);
                        break;
                    case 1:
                        aRegion = vcl::Region(aSmaller);
                        aRegion.XOr(aSub);
                        break;
                    case 2:
                    {
                        tools::Polygon aPoly(aSub);
                        aPoly.Rotate(aSub.Center(), 450);
                        aPoly.Clip(aSmaller);
                        aRegion = vcl::Region(aPoly);
                        break;
                    }
                    case 3:
                    {
                        tools::PolyPolygon aPolyPoly;
                        sal_Int32 nTW = aSub.GetWidth()/6;
                        sal_Int32 nTH = aSub.GetHeight()/6;
                        Rectangle aTiny(Point(4, 4), Size(nTW*2, nTH*2));
                        aPolyPoly.Insert( tools::Polygon(aTiny));
                        aTiny.Move(nTW*3, nTH*3);
                        aPolyPoly.Insert( tools::Polygon(aTiny));
                        aTiny.Move(nTW, nTH);
                        aPolyPoly.Insert( tools::Polygon(aTiny));

                        aRegion = vcl::Region(aPolyPoly);
                        break;
                    }
                    } // switch
                    rDev.SetClipRegion(aRegion);
                    rDev.DrawCheckered(aSub.TopLeft(), aSub.GetSize());
                    rDev.SetClipRegion();
                }
            }
            else
            {
                rDev.DrawCheckered(r.TopLeft(), r.GetSize());
            }
        }
    };

    struct DrawPoly : public RegionRenderer
    {
        RENDER_DETAILS(poly,KEY_P,20)
        DrawCheckered maCheckered;
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            maCheckered.RenderRegion(rDev, r, rCtx);

            long nDx = r.GetWidth()/20;
            long nDy = r.GetHeight()/20;
            Rectangle aShrunk(r);
            aShrunk.Move(nDx, nDy);
            aShrunk.SetSize(Size(r.GetWidth()-nDx*2,
                                 r.GetHeight()-nDy*2));
            tools::Polygon aPoly(aShrunk);
            tools::PolyPolygon aPPoly(aPoly);
            rDev.SetLineColor(Color(COL_RED));
            rDev.SetFillColor(Color(COL_RED));
            // This hits the optional 'drawPolyPolygon' code-path
            rDev.DrawTransparent(aPPoly, 64);
        }
    };

    struct DrawEllipse : public RegionRenderer
    {
        RENDER_DETAILS(ellipse,KEY_E,5000)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            rDev.SetLineColor(Color(COL_RED));
            rDev.SetFillColor(Color(COL_GREEN));
            rDev.DrawEllipse(r);

            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                auto aRegions = partition(rCtx, 2, 2);
                rDev.Invert(aRegions[0]);
                rDev.Invert(aRegions[1], InvertFlags::N50);
                rDev.Invert(aRegions[2], InvertFlags::Highlight);
                rDev.Invert(aRegions[3], (InvertFlags)0xffff);
            }
        }
    };

    struct DrawGradient : public RegionRenderer
    {
        RENDER_DETAILS(gradient,KEY_G,50)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<Rectangle> aRegions(DemoRenderer::partition(rCtx,5, 4));
                sal_uInt32 nStartCols[] = {
                    COL_RED, COL_RED, COL_RED, COL_GREEN, COL_GREEN,
                    COL_BLUE, COL_BLUE, COL_BLUE, COL_CYAN, COL_CYAN,
                    COL_BLACK, COL_LIGHTGRAY, COL_WHITE, COL_BLUE, COL_CYAN,
                    COL_WHITE, COL_WHITE, COL_WHITE, COL_BLACK, COL_BLACK
                };
                sal_uInt32 nEndCols[] = {
                    COL_WHITE, COL_WHITE, COL_WHITE, COL_BLACK, COL_BLACK,
                    COL_RED, COL_RED, COL_RED, COL_GREEN, COL_GREEN,
                    COL_GRAY, COL_GRAY, COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTCYAN,
                    COL_BLUE, COL_BLUE, COL_BLUE, COL_CYAN, COL_CYAN
                };
                GradientStyle eStyles[] = {
                    GradientStyle_LINEAR, GradientStyle_AXIAL, GradientStyle_RADIAL, GradientStyle_ELLIPTICAL, GradientStyle_SQUARE,
                    GradientStyle_RECT, GradientStyle_FORCE_EQUAL_SIZE, GradientStyle_LINEAR, GradientStyle_RADIAL, GradientStyle_LINEAR,
                    GradientStyle_LINEAR, GradientStyle_AXIAL, GradientStyle_RADIAL, GradientStyle_ELLIPTICAL, GradientStyle_SQUARE,
                    GradientStyle_RECT, GradientStyle_FORCE_EQUAL_SIZE, GradientStyle_LINEAR, GradientStyle_RADIAL, GradientStyle_LINEAR
                };
                sal_uInt16 nAngles[] = {
                    0, 0, 0, 0, 0,
                    15, 30, 45, 60, 75,
                    90, 120, 135, 160, 180,
                    0, 0, 0, 0, 0
                };
                sal_uInt16 nBorders[] = {
                    0, 0, 0, 0, 0,
                    1, 10, 100, 10, 1,
                    0, 0, 0, 0, 0,
                    1, 10, 20, 10, 1,
                    0, 0, 0, 0, 0
                };
                DemoRenderer::clearRects(rDev, aRegions);
                assert(aRegions.size() <= SAL_N_ELEMENTS(nStartCols));
                assert(aRegions.size() <= SAL_N_ELEMENTS(nEndCols));
                assert(aRegions.size() <= SAL_N_ELEMENTS(eStyles));
                assert(aRegions.size() <= SAL_N_ELEMENTS(nAngles));
                assert(aRegions.size() <= SAL_N_ELEMENTS(nBorders));
                for (size_t i = 0; i < aRegions.size(); i++)
                {
                    Rectangle aSub = aRegions[i];
                    Gradient aGradient;
                    aGradient.SetStartColor(Color(nStartCols[i]));
                    aGradient.SetEndColor(Color(nEndCols[i]));
                    aGradient.SetStyle(eStyles[i]);
                    aGradient.SetAngle(nAngles[i]);
                    aGradient.SetBorder(nBorders[i]);
                    rDev.DrawGradient(aSub, aGradient);
                }
            }
            else
            {
                Gradient aGradient;
                aGradient.SetStartColor(COL_YELLOW);
                aGradient.SetEndColor(COL_RED);
                aGradient.SetStyle(GradientStyle_RECT);
                aGradient.SetBorder(r.GetSize().Width()/20);
                rDev.DrawGradient(r, aGradient);
            }
        }
    };

    struct DrawBitmap : public RegionRenderer
    {
        RENDER_DETAILS(bitmap,KEY_B,10)

        // Simulate Page Borders rendering - which ultimately should
        // be done with a shader / gradient
        static void SimulateBorderStretch(OutputDevice &rDev, const Rectangle& r)
        {
            BitmapEx aPageShadowMask("sw/res/page-shadow-mask.png");

            BitmapEx aRight(aPageShadowMask);
            sal_Int32 nSlice = (aPageShadowMask.GetSizePixel().Width() - 3) / 4;
            // a width x 1 slice
            aRight.Crop(Rectangle(Point((nSlice * 3) + 3, (nSlice * 2) + 1),
                                  Size(nSlice, 1)));
            AlphaMask aAlphaMask(aRight.GetBitmap());
            Bitmap aBlockColor = Bitmap(aAlphaMask.GetSizePixel(), 24);
            aBlockColor.Erase(COL_RED);
            BitmapEx aShadowStretch = BitmapEx(aBlockColor, aAlphaMask);

            Point aRenderPt(r.TopLeft());

            long aSizes[] = { 200, 100, 200, 100, 50, 5, 2 };

            // and yes - we really do this in the page border rendering code ...
            for (size_t i = 0; i < SAL_N_ELEMENTS(aSizes); i++)
            {
                aShadowStretch.Scale(Size(aShadowStretch.GetSizePixel().Width(), aSizes[i]),
                                     BmpScaleFlag::Fast);

                rDev.DrawBitmapEx(aRenderPt, aShadowStretch);
                aRenderPt.Move(aShadowStretch.GetSizePixel().Width() + 4, 0);
            }

            AlphaMask aWholeMask(aPageShadowMask.GetBitmap());
            aBlockColor = Bitmap(aPageShadowMask.GetSizePixel(), 24);
            aBlockColor.Erase(COL_GREEN);
            BitmapEx aWhole(aBlockColor, aWholeMask);

            aRenderPt = Point(r.Center());
            aRenderPt.Move(nSlice+1, 0);

            // An offset background for alpha rendering
            rDev.SetFillColor(COL_BLUE);
            Rectangle aSurround(r.Center(), Size(aPageShadowMask.GetSizePixel()));
            rDev.DrawRect(aSurround);
            rDev.DrawBitmapEx(aRenderPt, aWhole);
        }

        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            Bitmap aBitmap(rCtx.mpDemoRenderer->maIntroBW);
            aBitmap.Scale(r.GetSize(), BmpScaleFlag::BestQuality);
            rDev.DrawBitmap(r.TopLeft(), aBitmap);

            SimulateBorderStretch(rDev, r);
        }
    };

    struct DrawBitmapEx : public RegionRenderer
    {
        RENDER_DETAILS(bitmapex,KEY_X,2)
        DrawCheckered maCheckered;
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            maCheckered.RenderRegion(rDev, r, rCtx);

            BitmapEx aBitmap(rCtx.mpDemoRenderer->maIntro);
            aBitmap.Scale(r.GetSize(), BmpScaleFlag::BestQuality);
            AlphaMask aSemiTransp(aBitmap.GetSizePixel());
            aSemiTransp.Erase(64);
            rDev.DrawBitmapEx(r.TopLeft(), BitmapEx(aBitmap.GetBitmap(),
                                                    aSemiTransp));
        }
    };

    struct DrawPolyPolygons : public RegionRenderer
    {
        RENDER_DETAILS(polypoly,KEY_N,100)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &) override
        {
            struct {
                double nX, nY;
            } aPoints[] = { { 0.1, 0.1 }, { 0.9, 0.9 },
#if FIXME_SELF_INTERSECTING_WORKING
                            { 0.9, 0.1 }, { 0.1, 0.9 },
                            { 0.1, 0.1 }
#else
                            { 0.1, 0.9 }, { 0.5, 0.5 },
                            { 0.9, 0.1 }, { 0.1, 0.1 }
#endif
            };

            tools::PolyPolygon aPolyPoly;
            // Render 4x polygons & aggregate into another PolyPolygon
            for (int x = 0; x < 2; x++)
            {
                for (int y = 0; y < 2; y++)
                {
                    Rectangle aSubRect(r);
                    aSubRect.Move(x * r.GetWidth()/3, y * r.GetHeight()/3);
                    aSubRect.SetSize(Size(r.GetWidth()/2, r.GetHeight()/4));
                    tools::Polygon aPoly(SAL_N_ELEMENTS(aPoints));
                    for (size_t v = 0; v < SAL_N_ELEMENTS(aPoints); v++)
                    {
                        aPoly.SetPoint(Point(aSubRect.Left() +
                                             aSubRect.GetWidth() * aPoints[v].nX,
                                             aSubRect.Top() +
                                             aSubRect.GetHeight() * aPoints[v].nY),
                                       v);
                    }
                    rDev.SetLineColor(Color(COL_YELLOW));
                    rDev.SetFillColor(Color(COL_BLACK));
                    rDev.DrawPolygon(aPoly);

                    // now move and add to the polypolygon
                    aPoly.Move(0, r.GetHeight()/2);
                    aPolyPoly.Insert(aPoly);
                }
            }
            rDev.SetLineColor(Color(COL_LIGHTRED));
            rDev.SetFillColor(Color(COL_GREEN));
            rDev.DrawTransparent(aPolyPoly, 50);
        }
    };

    struct DrawToVirtualDevice : public RegionRenderer
    {
        RENDER_DETAILS(vdev,KEY_V,1)
        enum RenderType {
            RENDER_AS_BITMAP,
            RENDER_AS_OUTDEV,
            RENDER_AS_BITMAPEX,
            RENDER_AS_ALPHA_OUTDEV
        };

        static void SizeAndRender(OutputDevice &rDev, const Rectangle& r, RenderType eType,
                           const RenderContext &rCtx)
        {
            ScopedVclPtr<VirtualDevice> pNested;

            if ((int)eType < RENDER_AS_BITMAPEX)
                pNested = VclPtr<VirtualDevice>::Create(rDev).get();
            else
                pNested = VclPtr<VirtualDevice>::Create(rDev,DeviceFormat::DEFAULT,DeviceFormat::DEFAULT).get();

            pNested->SetOutputSizePixel(r.GetSize());
            Rectangle aWhole(Point(0,0), r.GetSize());

            // mini me
            rCtx.mpDemoRenderer->drawToDevice(*pNested, r.GetSize(), true);

            if (eType == RENDER_AS_BITMAP)
            {
                Bitmap aBitmap(pNested->GetBitmap(Point(0,0),aWhole.GetSize()));
                rDev.DrawBitmap(r.TopLeft(), aBitmap);
            }
            else if (eType == RENDER_AS_BITMAPEX)
            {
                BitmapEx aBitmapEx(pNested->GetBitmapEx(Point(0,0),aWhole.GetSize()));
                rDev.DrawBitmapEx(r.TopLeft(), aBitmapEx);
            }
            else if (eType == RENDER_AS_OUTDEV ||
                     eType == RENDER_AS_ALPHA_OUTDEV)
            {
                rDev.DrawOutDev(r.TopLeft(), r.GetSize(),
                                aWhole.TopLeft(), aWhole.GetSize(),
                                *pNested);
            }
        }
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            // avoid infinite recursion
            if (rCtx.mbVDev)
                return;

            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<Rectangle> aRegions(DemoRenderer::partition(rCtx,2, 2));
                DemoRenderer::clearRects(rDev, aRegions);

                RenderType eRenderTypes[] = { RENDER_AS_BITMAP, RENDER_AS_OUTDEV,
                                              RENDER_AS_BITMAPEX, RENDER_AS_ALPHA_OUTDEV };
                for (size_t i = 0; i < aRegions.size(); i++)
                    SizeAndRender(rDev, aRegions[i], eRenderTypes[i], rCtx);
            }
            else
                SizeAndRender(rDev, r, RENDER_AS_BITMAP, rCtx);
        }
    };

    struct DrawIcons : public RegionRenderer
    {
        RENDER_DETAILS(icons,KEY_I,1)

        std::vector<OUString> maIconNames;
        std::vector<BitmapEx> maIcons;
        bool bHasLoadedAll;
        DrawIcons() : bHasLoadedAll(false)
        {
            // a few icons to start with
            const char *pNames[] = {
                "cmd/lc_openurl.png",
                "cmd/lc_newdoc.png",
                "cmd/lc_choosemacro.png",
                "cmd/lc_save.png",
                "cmd/lc_saveas.png",
                "cmd/lc_importdialog.png",
                "cmd/lc_sendmail.png",
                "cmd/lc_editdoc.png",
                "cmd/lc_print.png",
                "cmd/lc_combobox.png",
                "cmd/lc_insertformcombo.png",
                "cmd/lc_printpreview.png",
                "cmd/lc_cut.png",
                "cmd/lc_copy.png",
                "cmd/lc_paste.png",
                "cmd/sc_autopilotmenu.png",
                "cmd/lc_formatpaintbrush.png",
                "cmd/lc_undo.png",
                "cmd/lc_redo.png",
                "cmd/lc_marks.png",
                "cmd/lc_fieldnames.png",
                "cmd/lc_hyperlinkdialog.png",
                "cmd/lc_basicshapes.rectangle.png",
                "cmd/lc_basicshapes.round-rectangle.png"
            };
            for (size_t i = 0; i < SAL_N_ELEMENTS(pNames); i++)
            {
                maIconNames.push_back(OUString::createFromAscii(pNames[i]));
                maIcons.push_back(BitmapEx(maIconNames[i]));
            }
        }

        void LoadAllImages()
        {
            if (bHasLoadedAll)
                return;
            bHasLoadedAll = true;

            css::uno::Sequence< OUString > aAllIcons = ImageTree_getAllImageNames();
            for (sal_Int32 i = 0; i < aAllIcons.getLength(); i++)
            {
                if (aAllIcons[i].endsWithIgnoreAsciiCase("svg"))
                    continue; // too slow to load.
                maIconNames.push_back(aAllIcons[i]);
                maIcons.push_back(BitmapEx(aAllIcons[i]));
            }
        }

        void doDrawIcons(OutputDevice &rDev, Rectangle r, bool bExpanded)
        {
            long nMaxH = 0;
            Point p(r.LeftCenter());
            size_t nToRender = maIcons.size();

            if (!bExpanded && maIcons.size() > 64)
                nToRender = 64;
            for (size_t i = 0; i < nToRender; i++)
            {
                Size aSize(maIcons[i].GetSizePixel());
//              sAL_DEBUG("Draw icon '" << maIconNames[i] << "'");

                if (!(i % 4))
                    rDev.DrawBitmapEx(p, maIcons[i]);
                else
                {
                    basegfx::B2DHomMatrix aTransform;
                    aTransform.scale(aSize.Width(), aSize.Height());
                    switch (i % 4)
                    {
                    case 2:
                        aTransform.shearX((double)((i >> 2) % 8) / 8);
                        aTransform.shearY((double)((i >> 4) % 8) / 8);
                        break;
                    case 3:
                        aTransform.translate(-aSize.Width()/2, -aSize.Height()/2);
                        aTransform.rotate(i);
                        if (i & 0x100)
                        {
                            aTransform.shearX((double)((i >> 2) % 8) / 8);
                            aTransform.shearY((double)((i >> 4) % 8) / 8);
                        }
                        aTransform.translate(aSize.Width()/2,  aSize.Height()/2);
                        break;
                    default:
                        aTransform.translate(-aSize.Width()/2, -aSize.Height()/2);
                        aTransform.rotate(2 * F_2PI * i / nToRender);
                        aTransform.translate(aSize.Width()/2,  aSize.Height()/2);
                        break;
                    }
                    aTransform.translate(p.X(), p.Y());
                    rDev.DrawTransformedBitmapEx(aTransform, maIcons[i]);
                }

                // next position
                p.Move(aSize.Width(), 0);
                if (aSize.Height() > nMaxH)
                    nMaxH = aSize.Height();
                if (p.X() >= r.Right()) // wrap to next line
                {
                    p = Point(r.Left(), p.Y() + nMaxH);
                    nMaxH = 0;
                }
                if (p.Y() >= r.Bottom()) // re-start at middle
                    p = r.LeftCenter();
            }
        }

        static BitmapEx AlphaRecovery(OutputDevice &rDev, Point aPt, BitmapEx &aSrc)
        {
            // Compositing onto 2x colors beyond our control
            ScopedVclPtrInstance< VirtualDevice > aWhite;
            ScopedVclPtrInstance< VirtualDevice > aBlack;
            aWhite->SetOutputSizePixel(aSrc.GetSizePixel());
            aWhite->SetBackground(Wallpaper(COL_WHITE));
            aWhite->Erase();
            aBlack->SetOutputSizePixel(aSrc.GetSizePixel());
            aBlack->SetBackground(Wallpaper(COL_BLACK));
            aBlack->Erase();
            aWhite->DrawBitmapEx(Point(), aSrc);
            aBlack->DrawBitmapEx(Point(), aSrc);

            // Now recover that alpha...
            Bitmap aWhiteBmp = aWhite->GetBitmap(Point(),aSrc.GetSizePixel());
            Bitmap aBlackBmp = aBlack->GetBitmap(Point(),aSrc.GetSizePixel());
            AlphaMask aMask(aSrc.GetSizePixel());
            Bitmap aRecovered(aSrc.GetSizePixel(), 24);
            {
                AlphaMask::ScopedWriteAccess pMaskAcc(aMask);
                Bitmap::ScopedWriteAccess pRecAcc(aRecovered);
                Bitmap::ScopedReadAccess pAccW(aWhiteBmp); // a * pix + (1-a)
                Bitmap::ScopedReadAccess pAccB(aBlackBmp); // a * pix + 0
                int nSizeX = aSrc.GetSizePixel().Width();
                int nSizeY = aSrc.GetSizePixel().Height();
                for (int y = 0; y < nSizeY; y++)
                {
                    for (int x = 0; x < nSizeX; x++)
                    {
                        BitmapColor aColW = pAccW->GetPixel(y,x);
                        BitmapColor aColB = pAccB->GetPixel(y,x);
                        long nAR = (long)(aColW.GetRed() - aColB.GetRed()); // (1-a)
                        long nAG = (long)(aColW.GetGreen() - aColB.GetGreen()); // (1-a)
                        long nAB = (long)(aColW.GetBlue() - aColB.GetBlue()); // (1-a)

#define CLAMP(a,b,c) (((a)<=(b))?(b):(((a)>=(c))?(c):(a)))

                        // we get the most precision from the largest delta
                        long nInverseAlpha = std::max(nAR, std::max(nAG, nAB)); // (1-a)
                        nInverseAlpha = CLAMP(nInverseAlpha, 0, 255);
                        long nAlpha = 255 - nInverseAlpha;

                        pMaskAcc->SetPixel(y,x,BitmapColor((sal_Int8)CLAMP(nInverseAlpha,0,255)));
                        // now recover the pixels
                        long nR = (aColW.GetRed() + aColB.GetRed() - nInverseAlpha) * 128;
                        long nG = (aColW.GetGreen() + aColB.GetGreen() - nInverseAlpha) * 128;
                        long nB = (aColW.GetBlue() + aColB.GetBlue() - nInverseAlpha) * 128;
                        if (nAlpha == 0)
                        { // doesn't matter what's behind transparency
                            nR = nG = nB = 0;
                        }
                        else
                        {
                            nR /= nAlpha; nG /= nAlpha; nB /= nAlpha;
                        }
                        pRecAcc->SetPixel(y,x,BitmapColor(
                                                (sal_uInt8)CLAMP(nR,0,255),
                                                (sal_uInt8)CLAMP(nG,0,255),
                                                (sal_uInt8)CLAMP(nB,0,255)));
#undef CLAMP
                    }
                }
            }
            rDev.DrawBitmap(aPt, aWhiteBmp);
            aPt.Move(aSrc.GetSizePixel().Width(), 0);
            rDev.DrawBitmap(aPt, aBlackBmp);
            aPt.Move(aSrc.GetSizePixel().Width(), 0);
            rDev.DrawBitmap(aPt, aRecovered);
            aPt.Move(aSrc.GetSizePixel().Width(), 0);
            rDev.DrawBitmap(aPt, aMask.GetBitmap());
            aPt.Move(aSrc.GetSizePixel().Width(), 0);

            return BitmapEx(aRecovered, aMask);
        }

        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                LoadAllImages();

                Point aLocation(0,maIcons[0].GetSizePixel().Height() + 8);
                for (size_t i = 0; i < 100; i++)
                {
                    BitmapEx aSrc = maIcons[i];

                    // original above
                    Point aAbove(aLocation);
                    aAbove.Move(0,-aSrc.GetSizePixel().Height() - 4);
                    rDev.DrawBitmapEx(aAbove, aSrc);
                    aAbove.Move(aSrc.GetSizePixel().Width(),0);
                    aAbove.Move(aSrc.GetSizePixel().Width(),0);
                    rDev.DrawBitmap(aAbove, aSrc.GetBitmap());
                    aAbove.Move(aSrc.GetSizePixel().Width(),0);
                    rDev.DrawBitmap(aAbove, aSrc.GetMask());

                    // intermediates middle
                    BitmapEx aResult = AlphaRecovery(rDev, aLocation, aSrc);

                    // result below
                    Point aBelow(aLocation);
                    aBelow.Move(0,aResult.GetSizePixel().Height());
                    rDev.DrawBitmapEx(aBelow, aResult);

                    // mini convert test.
                    aBelow.Move(aResult.GetSizePixel().Width()+4,0);
                    rDev.DrawBitmapEx(aBelow, aResult);

                    Bitmap aGrey = aSrc.GetBitmap();
                    aGrey.Convert(BMP_CONVERSION_8BIT_GREYS);
                    rDev.DrawBitmap(aBelow, aGrey);

                    aBelow.Move(aGrey.GetSizePixel().Width(),0);
                    BitmapEx aGreyMask(aSrc.GetBitmap(),
                                       AlphaMask(aSrc.GetMask()));
                    rDev.DrawBitmapEx(aBelow, aGreyMask);

                    aLocation.Move(aSrc.GetSizePixel().Width()*6,0);
                    if (aLocation.X() > r.Right())
                        aLocation = Point(0,aLocation.Y()+aSrc.GetSizePixel().Height()*3+4);
                }

                // now go crazy with random foo
                doDrawIcons(rDev, r, true);
            }
            else
            {
                doDrawIcons(rDev, r, false);
            }
        }
    };

    struct FetchDrawBitmap : public RegionRenderer
    {
        RENDER_DETAILS(fetchdraw,KEY_F,50)
        virtual void RenderRegion(OutputDevice &rDev, Rectangle r,
                                  const RenderContext &) override
        {
            Bitmap aBitmap(rDev.GetBitmap(Point(0,0),rDev.GetOutputSizePixel()));
            aBitmap.Scale(r.GetSize(), BmpScaleFlag::BestQuality);
            rDev.DrawBitmap(r.TopLeft(), aBitmap);
        }
    };

    void drawToDevice(vcl::RenderContext& rDev, Size aSize, bool bVDev)
    {
        RenderContext aCtx;
        double mnStartTime;
        aCtx.mbVDev = bVDev;
        aCtx.mpDemoRenderer = this;
        aCtx.maSize = aSize;
        Rectangle aWholeWin(Point(0,0), rDev.GetOutputSizePixel());

        drawBackground(rDev, aWholeWin);

        if (!bVDev /* want everything in the vdev */ &&
            mnSelectedRenderer >= 0)
        {
            aCtx.meStyle = RENDER_EXPANDED;
            RegionRenderer * r = maRenderers[mnSelectedRenderer];
            // profiling?
            if (getIterCount() > 0)
            {
                mnStartTime = getTimeNow();
                for (int i = 0; i < r->getTestRepeatCount(); i++)
                    r->RenderRegion(rDev, aWholeWin, aCtx);
                addTime(mnSelectedRenderer, getTimeNow() - mnStartTime);
            } else
                r->RenderRegion(rDev, aWholeWin, aCtx);
        }
        else
        {
            aCtx.meStyle = RENDER_THUMB;
            std::vector<Rectangle> aRegions(partition(aSize, mnSegmentsX, mnSegmentsY));
            DemoRenderer::clearRects(rDev, aRegions);
            for (size_t i = 0; i < maRenderers.size(); i++)
            {
                RegionRenderer * r = maRenderers[i];

                rDev.SetClipRegion( vcl::Region( aRegions[i] ) );

                // profiling?
                if (getIterCount() > 0)
                {
                    if (!bVDev)
                    {
                        mnStartTime = getTimeNow();
                        for (int j = 0; j < r->getTestRepeatCount() * THUMB_REPEAT_FACTOR; j++)
                            r->RenderRegion(rDev, aRegions[i], aCtx);
                        addTime(i, (getTimeNow() - mnStartTime) / THUMB_REPEAT_FACTOR);
                    } else
                        for (int j = 0; j < r->getTestRepeatCount(); j++)
                            r->RenderRegion(rDev, aRegions[i], aCtx);
                }
                else
                {
                    r->RenderRegion(rDev, aRegions[i], aCtx);
                }

                rDev.SetClipRegion();
            }
        }
    }
    std::vector<VclPtr<vcl::Window> > maInvalidates;
    void addInvalidate(vcl::Window *pWindow) { maInvalidates.push_back(pWindow); };
    void removeInvalidate(vcl::Window *pWindow)
    {
        for (auto aIt = maInvalidates.begin(); aIt != maInvalidates.end(); ++aIt)
        {
            if (*aIt == pWindow)
            {
                maInvalidates.erase(aIt);
                return;
            }
        }
    }
    void Invalidate()
    {
        for (size_t i = 0; i < maInvalidates.size(); ++i)
            maInvalidates[i]->Invalidate();
    }
};

#if FIXME_BOUNCE_BUTTON
IMPL_LINK_NOARG_TYPED(DemoRenderer,BounceTimerCb,Timer*,void)
{
    mpButton->Check(mnBounceX>0);
    mpButton->SetPressed(mnBounceY>0);

    Point aCur = mpButtonWin->GetPosPixel();
    static const int nMovePix = 10;
    aCur.Move(mnBounceX * nMovePix, mnBounceX * nMovePix);
    Size aWinSize = GetSizePixel();
    if (aCur.X() <= 0 || aCur.X() >= aWinSize.Width())
        mnBounceX *= -1;
    if (aCur.Y() <= 0 || aCur.Y() >= aWinSize.Height())
        mnBounceX *= -1;
    mpButtonWin->SetPosPixel(aCur);

    // All smoke and mirrors to test sub-region invalidation underneath
    Rectangle aRect(aCur, mpButtonWin->GetSizePixel());
    Invalidate(aRect);
}
#endif

void DemoRenderer::KeyInput(const KeyEvent &rKEvt)
{
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    // click to zoom out
    if (mnSelectedRenderer >= 0)
    {
        if (nCode == KEY_ESCAPE || nCode == KEY_BACKSPACE)
        {
            mnSelectedRenderer = -1;
            Invalidate();
            return;
        }
    }
    else
    {
        for (size_t i = 0; i < maRenderers.size(); i++)
        {
            if (nCode == maRenderers[i]->getAccelerator())
            {
                mnSelectedRenderer = i;
                Invalidate();
                return;
            }
        }
    }
}

bool DemoRenderer::MouseButtonDown(const MouseEvent& rMEvt)
{
    // click to zoom out
    if (mnSelectedRenderer >= 0)
    {
        mnSelectedRenderer = -1;
        Invalidate();
        return true;
    }

    // click on a region to zoom into it
    std::vector<Rectangle> aRegions(partition(GetSizePixel(), mnSegmentsX, mnSegmentsY));
    for (size_t i = 0; i < aRegions.size(); i++)
    {
        if (aRegions[i].IsInside(rMEvt.GetPosPixel()))
        {
            mnSelectedRenderer = i;
            Invalidate();
            return true;
        }
    }

#if FIXME_BOUNCE_BUTTON
    // otherwise bounce floating windows
    if (!mpButton)
    {
        mpButtonWin = VclPtr<FloatingWindow>::Create(this);
        mpButton = VclPtr<PushButton>::Create(mpButtonWin);
        mpButton->SetSymbol(SymbolType::HELP);
        mpButton->SetText("PushButton demo");
        mpButton->SetPosSizePixel(Point(0,0), mpButton->GetOptimalSize());
        mpButton->Show();
        mpButtonWin->SetPosSizePixel(Point(0,0), mpButton->GetOptimalSize());
        mpButtonWin->Show();
        mnBounceX = 1; mnBounceX = 1;
        maBounce.SetTimeoutHdl(LINK(this,DemoRenderer,BounceTimerCb));
        maBounce.SetTimeout(55);
        maBounce.Start();
    }
    else
    {
        maBounce.Stop();
        delete mpButtonWin;
        mpButtonWin = NULL;
        mpButton = NULL;
    }
#endif
    return false;
}

void DemoRenderer::InitRenderers()
{
    maRenderers.push_back(new DrawLines());
    maRenderers.push_back(new DrawText());
    maRenderers.push_back(new DrawPoly());
    maRenderers.push_back(new DrawEllipse());
    maRenderers.push_back(new DrawCheckered());
    maRenderers.push_back(new DrawBitmapEx());
    maRenderers.push_back(new DrawBitmap());
    maRenderers.push_back(new DrawGradient());
    maRenderers.push_back(new DrawPolyPolygons());
    maRenderers.push_back(new DrawToVirtualDevice());
    maRenderers.push_back(new DrawIcons());
    maRenderers.push_back(new FetchDrawBitmap());
}

OUString DemoRenderer::getRendererList()
{
    OUStringBuffer aBuf;
    for (size_t i = 0; i < maRenderers.size(); i++)
    {
        aBuf.append(maRenderers[i]->getName());
        aBuf.append(' ');
    }
    return aBuf.makeStringAndClear();
}

double DemoRenderer::getAndResetBenchmark(const RenderStyle style)
{
    double geomean = 1.0;
    fprintf(stderr, "Rendering: %s, Times (ms):\n", style == RENDER_THUMB ? "THUMB": "EXPANDED");
    for (size_t i = 0; i < maRenderers.size(); i++)
    {
        double avgtime = maRenderers[i]->sumTime / maRenderers[i]->countTime;
        geomean *= avgtime;
        fprintf(stderr, "%s: %f (iteration: %d*%d*%d)\n",
                rtl::OUStringToOString(maRenderers[i]->getName(),
                RTL_TEXTENCODING_UTF8).getStr(), avgtime,
                maRenderers[i]->countTime, maRenderers[i]->getTestRepeatCount(),
                (style == RENDER_THUMB) ? THUMB_REPEAT_FACTOR : 1);
        maRenderers[i]->sumTime = 0;
        maRenderers[i]->countTime = 0;
    }
    geomean = pow(geomean, static_cast<double>(1.0)/maRenderers.size());
    fprintf(stderr, "GEOMEAN_%s: %f\n", style == RENDER_THUMB ? "THUMB": "EXPANDED", geomean);
    return geomean;
}

void DemoRenderer::setIterCount(sal_Int32 i)
{
    iterCount = i;
}

sal_Int32 DemoRenderer::getIterCount()
{
    return iterCount;
}

void DemoRenderer::addTime(int i, double t)
{
    maRenderers[i]->sumTime += t / maRenderers[i]->getTestRepeatCount();
    maRenderers[i]->countTime++;
}

void DemoRenderer::selectRenderer(const OUString &rName )
{
    for (size_t i = 0; i < maRenderers.size(); i++)
    {
        if (maRenderers[i]->getName() == rName)
        {
            mnSelectedRenderer = i;
            Invalidate();
            return;
        }
    }
}

int DemoRenderer::selectNextRenderer()
{
    mnSelectedRenderer++;
    if (mnSelectedRenderer == (signed) maRenderers.size())
        mnSelectedRenderer = -1;
    Invalidate();
    return mnSelectedRenderer;
}

class DemoWin : public WorkWindow
{
    DemoRenderer &mrRenderer;
    bool underTesting;
    bool testThreads;

    class RenderThread : public salhelper::Thread {
        DemoWin  &mrWin;
        TimeValue maDelay;
    public:
        RenderThread(DemoWin &rWin, sal_uInt32 nDelaySecs)
            : Thread("vcldemo render thread")
            , mrWin(rWin)
        {
            maDelay.Seconds = nDelaySecs;
            maDelay.Nanosec = 0;
            launch();
        }
        virtual ~RenderThread()
        {
            join();
        }
        virtual void execute() override
        {
            osl_waitThread(&maDelay);

            SolarMutexGuard aGuard;
            fprintf (stderr, "render from a different thread\n");
            mrWin.Invalidate();
        }
    };
    rtl::Reference<RenderThread> mxThread;

public:
    DemoWin(DemoRenderer &rRenderer, bool bThreads) :
        WorkWindow(nullptr, WB_APP | WB_STDWORK),
        mrRenderer(rRenderer),
        testThreads(bThreads)
    {
        mrRenderer.addInvalidate(this);
        underTesting = false;
    }
    virtual ~DemoWin()
    {
        disposeOnce();
    }
    virtual void dispose() override
    {
        mxThread.clear();
        mrRenderer.removeInvalidate(this);
        WorkWindow::dispose();
    }
    virtual void MouseButtonDown(const MouseEvent& rMEvt) override
    {
        mrRenderer.SetSizePixel(GetSizePixel());
        if (!mrRenderer.MouseButtonDown(rMEvt))
        {
            if (testThreads)
            { // render this window asynchronously in a new thread
                sal_uInt32 nDelaySecs = 0;
                if (rMEvt.GetButtons() & MOUSE_RIGHT)
                    nDelaySecs = 5;
                mxThread = new RenderThread(*this, nDelaySecs);
            }
            else
            { // spawn another window
                VclPtrInstance<DemoWin> pNewWin(mrRenderer, testThreads);
                pNewWin->SetText("Another interactive VCL demo window");
                pNewWin->Show();
            }
        }
    }
    virtual void KeyInput(const KeyEvent& rKEvt) override
    {
        mrRenderer.SetSizePixel(GetSizePixel());
        mrRenderer.KeyInput(rKEvt);
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override
    {
        mrRenderer.SetSizePixel(GetSizePixel());
        fprintf(stderr, "DemoWin::Paint(%ld,%ld,%ld,%ld)\n", rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight());
        if (mrRenderer.getIterCount() == 0)
            mrRenderer.drawToDevice(rRenderContext, GetSizePixel(), false);
        else
            TestAndQuit(rRenderContext);
    }

    void TestAndQuit(vcl::RenderContext& rRenderContext)
    {
        if (underTesting)
            return;
        underTesting = true;
        for (sal_Int32 i = 0; i < mrRenderer.getIterCount(); i++)
        {
            while (mrRenderer.selectNextRenderer() > -1)
            {
                mrRenderer.drawToDevice(rRenderContext, GetSizePixel(), false);
            }
        }

        double expandedGEOMEAN = mrRenderer.getAndResetBenchmark(RENDER_EXPANDED);

        for (sal_Int32 i = 0; i < mrRenderer.getIterCount(); i++)
            mrRenderer.drawToDevice(rRenderContext, GetSizePixel(), false);

        double thumbGEOMEAN = mrRenderer.getAndResetBenchmark(RENDER_THUMB);

        fprintf(stderr, "GEOMEAN_TOTAL: %f\n", pow(thumbGEOMEAN * expandedGEOMEAN, static_cast<double>(0.5)));
        Application::Quit();
    }
};

class DemoWidgets : public WorkWindow
{
    MenuBar *mpBar;
    PopupMenu *mpPopup;

    VclPtr<VclBox> mpBox;
    VclPtr<ToolBox> mpToolbox;
    VclPtr<PushButton> mpButton;
    VclPtr<VclHBox> mpHBox;
    VclPtr<CheckBox> mpGLCheck;
    VclPtr<ComboBox> mpGLCombo;
    VclPtr<PushButton> mpGLButton;

    DECL_LINK_TYPED(GLTestClick, Button*, void);

public:
    DemoWidgets() :
        WorkWindow(nullptr, WB_APP | WB_STDWORK),
        mpBox(VclPtrInstance<VclVBox>(this, false, 3)),
        mpToolbox(VclPtrInstance<ToolBox>(mpBox.get())),
        mpButton(VclPtrInstance<PushButton>(mpBox.get())),
        mpHBox(VclPtrInstance<VclHBox>(mpBox.get(), true, 3)),
        mpGLCheck(VclPtrInstance<CheckBox>(mpHBox.get())),
        mpGLCombo(VclPtrInstance<ComboBox>(mpHBox.get())),
        mpGLButton(VclPtrInstance<PushButton>(mpHBox.get()))
    {
        SetText("VCL widget demo");

        Wallpaper aWallpaper(BitmapEx("sfx2/res/startcenter-logo.png"));
        aWallpaper.SetStyle(WallpaperStyle::BottomRight);
        aWallpaper.SetColor(COL_RED);

        mpBox->SetBackground(aWallpaper);
        mpBox->Show();

        Help::EnableBalloonHelp();
        mpToolbox->SetHelpText("Help text");
        mpToolbox->InsertItem(0, "Toolbar item");
        mpToolbox->SetQuickHelpText(0, "This is a tooltip popup");
        mpToolbox->InsertSeparator();
        mpToolbox->Show();

        mpButton->SetText("Click me; go on");
        mpButton->Show();

        mpGLCheck->SetText("Test in OGL zone");
        mpGLCheck->Show();
        mpGLCombo->InsertEntry("sleep 1 second");
        mpGLCombo->InsertEntry("sleep 3 seconds");
        mpGLCombo->InsertEntry("sleep 7 seconds");
        mpGLCombo->SelectEntryPos(2);
        mpGLCombo->Show();
        mpGLButton->SetText("Execute test");
        mpGLButton->SetClickHdl(LINK(this,DemoWidgets,GLTestClick));
        mpGLButton->Show();
        mpHBox->Show();

        mpBar = new MenuBar();
        mpBar->InsertItem(0,"File");
        mpPopup = new PopupMenu();
        mpPopup->InsertItem(0,"Item");
        mpBar->SetPopupMenu(0, mpPopup);
        SetMenuBar(mpBar);

        Show();
    }
    virtual ~DemoWidgets() { disposeOnce(); }
    virtual void dispose() override
    {
        mpGLButton.disposeAndClear();
        mpGLCombo.disposeAndClear();
        mpGLCheck.disposeAndClear();
        mpHBox.disposeAndClear();
        mpToolbox.disposeAndClear();
        mpButton.disposeAndClear();
        mpBox.disposeAndClear();
        delete mpPopup;
        delete mpBar;
        WorkWindow::dispose();
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override
    {
        Rectangle aWholeSize(Point(0, 0),GetOutputSizePixel());
        vcl::Region aClip(aWholeSize);
        Rectangle aExclude(Rectangle(Point(50,50),Size(100,100)));
        aClip.Exclude(aExclude);

        Wallpaper aWallpaper(COL_GREEN);

        rRenderContext.Push(PushFlags::CLIPREGION);
        rRenderContext.IntersectClipRegion(aClip);
        rRenderContext.DrawWallpaper(aWholeSize, aWallpaper);
        rRenderContext.Pop();

        ScopedVclPtrInstance< VirtualDevice > pDev(*this);
        pDev->EnableRTL(IsRTLEnabled());
        pDev->SetOutputSizePixel(aExclude.GetSize());

        Rectangle aSubRect(aWholeSize);
        aSubRect.Move(-aExclude.Left(), -aExclude.Top());
        pDev->DrawWallpaper(aSubRect, aWallpaper );

        rRenderContext.DrawOutDev(aExclude.TopLeft(), aExclude.GetSize(),
                   Point( 0, 0 ), aExclude.GetSize(), *pDev.get() );
    }
};

class OpenGLZoneTest {
public:
    static void enter() { OpenGLZone::enter(); }
    static void leave() { OpenGLZone::leave(); }
};

IMPL_LINK_NOARG_TYPED(DemoWidgets, GLTestClick, Button*, void)
{
    sal_Int32 nSelected = mpGLCombo->GetSelectEntryPos();

    TimeValue aDelay;
    aDelay.Seconds = 0;
    aDelay.Nanosec = 0;
    switch (nSelected)
    {
    case 0:
        aDelay.Seconds = 1;
        break;
    case 1:
        aDelay.Seconds = 3;
        break;
    case 2:
        aDelay.Seconds = 7;
        break;
    default:
        break;
    }

    bool bEnterLeave = mpGLCheck->IsChecked();
    if (bEnterLeave)
        OpenGLZoneTest::enter();

    osl_waitThread(&aDelay);

    if (bEnterLeave)
        OpenGLZoneTest::leave();
}

class DemoPopup : public FloatingWindow
{
 public:
    DemoPopup() : FloatingWindow( nullptr, WB_SYSTEMWINDOW|WB_TOOLTIPWIN)
    {
        SetType( WINDOW_HELPTEXTWINDOW );

        SetOutputSizePixel( Size( 300, 30 ) );
        SetBackground(Wallpaper(COL_YELLOW));

        Show( true, ShowFlags::NoActivate );
        Update();
    }

    virtual void Paint(vcl::RenderContext& /*rRenderContext*/, const Rectangle&) override
    {
        // Interestingly in GL mode on Windows, this doesn't render.

        Size aSize = GetOutputSizePixel();
        Rectangle aTextRect(Point(6, 6), aSize);

        SetTextColor(COL_BLACK);
        SetTextAlign(ALIGN_TOP);
        DrawText(aTextRect, "This is a standalone help text test",
                 DrawTextFlags::MultiLine|DrawTextFlags::WordBreak|
                 DrawTextFlags::Left|DrawTextFlags::Top);

        SetLineColor(COL_BLACK);
        SetFillColor();
        DrawRect( Rectangle( Point(), aSize ) );
        aSize.Width() -= 2;
        aSize.Height() -= 2;
        Color aColor( GetLineColor() );
        SetLineColor( ( COL_GRAY ) );
        DrawRect( Rectangle( Point( 1, 1 ), aSize ) );
        SetLineColor( aColor );
    }

    virtual void MouseButtonDown( const MouseEvent & ) override
    {
        Application::Quit();
    }
};

class OpenGLTests
{
    VclPtr<WorkWindow> mxWinA;
    VclPtr<WorkWindow> mxWinB;
    OpenGLSalGraphicsImpl *mpImplA;
    OpenGLSalGraphicsImpl *mpImplB;
    rtl::Reference<OpenGLContext> mpA;
    rtl::Reference<OpenGLContext> mpB;

    static OpenGLSalGraphicsImpl *getImpl(const VclPtr<OutputDevice> &xOut)
    {
        SalGraphics *pGraphics = xOut->GetGraphics();
        return dynamic_cast<OpenGLSalGraphicsImpl *>(pGraphics->GetImpl());
    }
public:
    OpenGLTests() :
        mxWinA(VclPtr<WorkWindow>::Create(nullptr, WB_APP | WB_STDWORK)),
        mxWinB(VclPtr<WorkWindow>::Create(nullptr, WB_APP | WB_STDWORK))
    {
        if (!OpenGLHelper::isVCLOpenGLEnabled())
        {
            mpImplA = mpImplB = nullptr;
            fprintf (stderr, "OpenGL is not enabled: try SAL_FORCEGL=1\n");
            return;
        }

        mpImplA = getImpl(mxWinA);
        mpImplB = getImpl(mxWinB);
        assert (mpImplA && mpImplB);
        mpA = mpImplA->GetOpenGLContext();
        mpB = mpImplB->GetOpenGLContext();

        assert (mpA.is() && mpB.is());
        assert (mpA != mpB);
    }
    ~OpenGLTests()
    {
        mxWinB.disposeAndClear();
        mxWinA.disposeAndClear();
    }

    void testCurrentFramebuffer()
    {
        fprintf(stderr,"test OpenGLContext's framebuffer association.\n");
        mpA->makeCurrent();
        OpenGLFramebuffer *pBuffer;
        {
            OpenGLTexture aTexture(256,128);
            pBuffer = mpA->AcquireFramebuffer(aTexture);
        }
        assert (pBuffer->IsFree()); (void)pBuffer;
        mpB->makeCurrent();
        assert (mpA->mpCurrentFramebuffer == nullptr);
    }

    void testVirtualDevice()
    {
        fprintf(stderr, "test sharing OpenGLContexts with virtual-devices reference counting\n");
        VclPtrInstance<WorkWindow> xTempWin(nullptr, WB_STDWORK);
        xTempWin->Show();
        // forcibly make this context current by rendering
        xTempWin->DrawPixel(Point(0, 0), COL_RED);

        // get some other guys to leach off this context
        VclPtrInstance<VirtualDevice> xVDev;
        rtl::Reference<OpenGLContext> pContext = getImpl(xVDev)->GetOpenGLContext();
        VclPtrInstance<VirtualDevice> xVDev2;
        rtl::Reference<OpenGLContext> pContext2 = getImpl(xVDev)->GetOpenGLContext();

        // sharing the same off-screen context.
        assert(pContext == pContext2);
        assert(pContext == getImpl(xTempWin)->GetOpenGLContext());
        assert(pContext != mpA && pContext != mpB);
        (void)pContext; (void)pContext2;

        // Kill the parent we free-ride on ...
        xTempWin.disposeAndClear();

        // This appears to continue working; fun.
        Point aPt(0, 0);
        xVDev->DrawPixel(aPt, COL_GREEN);
        assert(xVDev->GetPixel(aPt) == COL_GREEN);
        xVDev.disposeAndClear();

        // Switch context to see if we can switch back.
        mxWinA->DrawPixel(aPt, COL_WHITE);

        // Now try switching back to this guy ...
        xVDev2->DrawPixel(aPt, COL_BLUE);
        assert(xVDev2->GetPixel(aPt) == COL_BLUE);
        xVDev2.disposeAndClear();
    }

    int execute()
    {
        if (!OpenGLHelper::isVCLOpenGLEnabled())
            return 1;

        testCurrentFramebuffer();
        testVirtualDevice();

        return 0;
    }
};

class DemoApp : public Application
{
    static int showHelp(DemoRenderer &rRenderer)
    {
        fprintf(stderr,"vcldemo - a VCL test app\n");
        fprintf(stderr,"  --help             - print this text\n");
        fprintf(stderr,"  --show <renderer>  - start with a given renderer, options are:\n");
        OUString aRenderers(rRenderer.getRendererList());
        fprintf(stderr,"         %s\n",
                rtl::OUStringToOString(aRenderers, RTL_TEXTENCODING_UTF8).getStr());
        fprintf(stderr,"  --test <iterCount> - create benchmark data\n");
        fprintf(stderr,"  --widgets          - launch the widget test.\n");
        fprintf(stderr,"  --threads          - render from multiple threads.\n");
        fprintf(stderr,"  --gltest           - run openGL regression tests.\n");
        fprintf(stderr, "\n");
        return 0;
    }

public:
    DemoApp() {}

    virtual int Main() override
    {
        try
        {
            bool bWidgets = false, bThreads = false;
            bool bPopup = false, bGLTest = false;
            DemoRenderer aRenderer;

            for (sal_uInt16 i = 0; i < GetCommandLineParamCount(); ++i)
            {
                bool bLast = i == GetCommandLineParamCount() - 1;
                OUString aArg = GetCommandLineParam(i);
                if (aArg == "--help" || aArg == "-h")
                    return showHelp(aRenderer);
                if (aArg == "--show")
                {
                    if (bLast)
                        return showHelp(aRenderer);
                    else
                        aRenderer.selectRenderer(GetCommandLineParam(++i));
                }
                else if (aArg == "--test")
                {
                    if (bLast)
                        return showHelp(aRenderer);
                    else
                        aRenderer.setIterCount(GetCommandLineParam(++i).toInt32());
                }
                else if (aArg == "--widgets")
                    bWidgets = true;
                else if (aArg == "--popup")
                    bPopup = true;
                else if (aArg == "--gltest")
                    bGLTest = true;
                else if (aArg == "--threads")
                    bThreads = true;
                else if (aArg.startsWith("--"))
                {
                    fprintf(stderr,"Unknown argument '%s'\n",
                            rtl::OUStringToOString(aArg, RTL_TEXTENCODING_UTF8).getStr());
                    return showHelp(aRenderer);
                }
            }

            ScopedVclPtrInstance<DemoWin> aMainWin(aRenderer, bThreads);
            VclPtr<DemoWidgets> xWidgets;
            VclPtr<DemoPopup> xPopup;

            aMainWin->SetText("Interactive VCL demo #1");

            if (bGLTest)
            {
                OpenGLTests aTests;
                return aTests.execute();
            }
            else if (bWidgets)
                xWidgets = VclPtr< DemoWidgets >::Create ();
            else if (bPopup)
                xPopup = VclPtrInstance< DemoPopup> ();
            else
                aMainWin->Show();

            Application::Execute();

            xWidgets.disposeAndClear();
            xPopup.disposeAndClear();
        }
        catch (const css::uno::Exception& e)
        {
            SAL_WARN("vcl.app", "Fatal exception: " << e.Message);
            return 1;
        }
        catch (const std::exception& e)
        {
            SAL_WARN("vcl.app", "Fatal exception: " << e.what());
            return 1;
        }
        return 0;
    }

protected:
    uno::Reference<lang::XMultiServiceFactory> xMSF;
    void Init() override
    {
        try
        {
            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            xMSF.set(xComponentContext->getServiceManager(), uno::UNO_QUERY);
            if(!xMSF.is())
                Application::Abort("Bootstrap failure - no service manager");

            ::comphelper::setProcessServiceFactory(xMSF);
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }
    void DeInit() override
    {
        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW)-> dispose();
        ::comphelper::setProcessServiceFactory(nullptr);
    }
};

void vclmain::createApplication()
{
    static DemoApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
