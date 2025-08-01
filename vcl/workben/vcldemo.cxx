/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <thread>

#include <rtl/math.hxx>
#include <sal/log.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <o3tl/safeint.hxx>
#include <osl/time.h>
#include <vcl/gradient.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/layout.hxx>
#include <vcl/ptrstyle.hxx>
#include <salhelper/thread.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/help.hxx>
#include <vcl/kernarray.hxx>
#include <vcl/menu.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/bitmap/BitmapEmbossGreyFilter.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>

#include <framework/desktop.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#define FIXME_SELF_INTERSECTING_WORKING 0
#define FIXME_BOUNCE_BUTTON 0
#define THUMB_REPEAT_FACTOR 10

using namespace com::sun::star;

namespace {
    double getTimeNow()
    {
        TimeValue aValue;
        osl_getSystemTime(&aValue);
        return static_cast<double>(aValue.Seconds) * 1000 +
            static_cast<double>(aValue.Nanosec) / (1000*1000);
    }

}

namespace {

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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) = 0;
        // repeating count for profiling (to exceed the poor time resolution on Windows)
        virtual sal_uInt16 getTestRepeatCount() = 0;
#define RENDER_DETAILS(name,key,repeat) \
        virtual OUString getName() override \
            { return u"" SAL_STRINGIFY(name) ""_ustr; } \
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
        if (!Application::LoadBrandBitmap(u"intro", maIntro))
            Application::Abort(u"Failed to load intro image"_ustr);

        maIntroBW = maIntro.GetBitmap();

        BitmapEx aTmpBmpEx(maIntroBW);
        BitmapFilter::Filter(aTmpBmpEx, BitmapEmbossGreyFilter(0_deg100, 0_deg100));
        maIntroBW = aTmpBmpEx.GetBitmap();

        InitRenderers();
        mnSegmentsY = rtl::math::round(std::sqrt(maRenderers.size()), 0,
                                       rtl_math_RoundingMode_Down);
        mnSegmentsX = (maRenderers.size() + mnSegmentsY - 1)/mnSegmentsY;
    }

    OUString getRendererList();
    double   getAndResetBenchmark(RenderStyle style);
    void     selectRenderer(std::u16string_view rName);
    int      selectNextRenderer();
    void     setIterCount(sal_Int32 iterCount);
    sal_Int32 getIterCount() const;
    void     addTime(int i, double t);

    Size maSize;
    void SetSizePixel(const Size &rSize) { maSize = rSize; }
    const Size& GetSizePixel() const            { return maSize;  }


// more of a 'Window' concept - push upwards ?
#if FIXME_BOUNCE_BUTTON
    // Bouncing windows on click ...
    PushButton     *mpButton;
    FloatingWindow *mpButtonWin;
    AutoTimer       maBounce;
    int             mnBounceX, mnBounceY;
    DECL_LINK(BounceTimerCb, Timer*, void);
#endif

    bool MouseButtonDown(const MouseEvent& rMEvt);
    void KeyInput(const KeyEvent& rKEvt);

    static std::vector<tools::Rectangle> partition(const tools::Rectangle &rRect, int nX, int nY)
    {
        std::vector<tools::Rectangle> aRegions = partition(rRect.GetSize(), nX, nY);
        for (auto & region : aRegions)
            region.Move(rRect.Left(), rRect.Top());

        return aRegions;
    }

    static std::vector<tools::Rectangle> partition(const RenderContext &rCtx, int nX, int nY)
    {
        return partition(rCtx.maSize, nX, nY);
    }

    static std::vector<tools::Rectangle> partition(Size aSize, int nX, int nY)
    {
        tools::Rectangle r;
        std::vector<tools::Rectangle> aRegions;

        // Make small cleared area for these guys
        tools::Long nBorderSize = std::min(aSize.Height() / 32, aSize.Width() / 32);
        tools::Long nBoxWidth = (aSize.Width() - nBorderSize*(nX+1)) / nX;
        tools::Long nBoxHeight = (aSize.Height() - nBorderSize*(nY+1)) / nY;
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

    static void clearRects(OutputDevice &rDev, std::vector<tools::Rectangle> &rRects)
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

    static void drawBackground(OutputDevice &rDev, const tools::Rectangle& r)
    {
        rDev.Erase();
        Gradient aGradient;
        aGradient.SetStartColor(COL_BLUE);
        aGradient.SetEndColor(COL_GREEN);
        aGradient.SetStyle(css::awt::GradientStyle_LINEAR);
        rDev.DrawGradient(r, aGradient);
    }

    struct DrawLines : public RegionRenderer
    {
        RENDER_DETAILS(lines,KEY_L,100)
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                AntialiasingFlags nOldAA = rDev.GetAntialiasing();
                rDev.SetAntialiasing(AntialiasingFlags::Enable);

                std::vector<tools::Rectangle> aRegions(DemoRenderer::partition(rCtx, 4, 4));
                DemoRenderer::clearRects(rDev, aRegions);

#if 0 // FIXME: get this through to the backend ...
                double nTransparency[] = {
                    1.0, 1.0, 1.0, 1.0,
                    0.8, 0.8, 0.8, 0.8,
                    0.5, 0.5, 0.5, 0.5,
                    0.1, 0.1, 0.1, 0.1
                };
#endif
                drawing::LineCap const eLineCaps[] = {
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT,
                    drawing::LineCap_BUTT, drawing::LineCap_ROUND, drawing::LineCap_SQUARE, drawing::LineCap_BUTT
                };
                basegfx::B2DLineJoin const eJoins[] = {
                    basegfx::B2DLineJoin::NONE, basegfx::B2DLineJoin::Bevel, basegfx::B2DLineJoin::Miter, basegfx::B2DLineJoin::Round,
                    basegfx::B2DLineJoin::NONE, basegfx::B2DLineJoin::Bevel, basegfx::B2DLineJoin::Miter, basegfx::B2DLineJoin::Round,
                    basegfx::B2DLineJoin::NONE, basegfx::B2DLineJoin::Bevel, basegfx::B2DLineJoin::Miter, basegfx::B2DLineJoin::Round,
                    basegfx::B2DLineJoin::NONE, basegfx::B2DLineJoin::Bevel, basegfx::B2DLineJoin::Miter, basegfx::B2DLineJoin::Round
                };
                double const aLineWidths[] = {
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
                    rDev.SetLineColor(COL_BLACK);
                    basegfx::B2DPolygon aPoly;
                    tools::Rectangle aSub(aRegions[i]);
                    for (const auto& rPoint : aPoints)
                    {
                        aPoly.append(basegfx::B2DPoint(aSub.Left() + aSub.GetWidth() * rPoint.nX,
                                                       aSub.Top()  + aSub.GetHeight() * rPoint.nY));
                    }
                    rDev.DrawPolyLine(aPoly, aLineWidths[i], eJoins[i], eLineCaps[i]);
                }
            }
            else
            {
                rDev.SetFillColor(COL_LIGHTRED);
                rDev.SetLineColor(COL_BLACK);
                rDev.DrawRect(r);

                for(tools::Long i=0; i<r.GetHeight(); i+=15)
                    rDev.DrawLine(Point(r.Left(), r.Top()+i), Point(r.Right(), r.Bottom()-i));
                for(tools::Long i=0; i<r.GetWidth(); i+=15)
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

        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<tools::Rectangle> aToplevelRegions(
                    DemoRenderer::partition(rCtx, 1, 3));
                std::vector<tools::Rectangle> aSubRegions(
                    DemoRenderer::partition(aToplevelRegions[0], 4, 2));
                tools::Rectangle aBottom(aToplevelRegions[1].TopLeft(),
                                  aToplevelRegions[2].BottomRight());
                DemoRenderer::clearRects(rDev,aSubRegions);
                static struct {
                    bool mbClip;
                    bool mbArabicText;
                    bool mbRotate;
                } const aRenderData[] = {
                    { false, false, false },
                    { false, true,  false },
                    { false, true,  true },
                    { false, false, true },
                    { true,  false, true },
                    { true,  true,  true },
                    { true,  true,  false },
                    { true,  false, false },
                };

                size_t i = 0;
                for (int y = 0; y < 2; y++)
                {
                    for (int x = 0; x < 4; x++)
                    {
                        assert(i < std::size(aRenderData));
                        drawText(rDev, aSubRegions[i], aRenderData[i].mbClip,
                                 aRenderData[i].mbArabicText, aRenderData[i].mbRotate);
                        i++;
                    }
                }

                drawComplex(rDev, aBottom);
            }
            else
            {
                drawText(rDev, r, false, false, false);
            }
        }

        static void drawText (OutputDevice &rDev, tools::Rectangle r, bool bClip, bool bArabicText, bool bRotate)
        {
            rDev.SetClipRegion( vcl::Region(r) );

            const OUString aText = bArabicText?u"واحِدٌ اثْنَانِ ثَلاثَةٌ"_ustr
                                              :u"Click any rect to zoom!"_ustr;

            // To have more text displayed one after the other (overlapping, and in different colours), then
            // change this value
            const int nPrintNumCopies=1;

            std::vector<OUString> aFontNames =
            {
                u"Times"_ustr,
                u"Liberation Sans"_ustr,
                u"Arial"_ustr,
                u"Linux Biolinum G"_ustr,
                u"Linux Libertine Display G"_ustr
            };

            static Color const nCols[] = {
                COL_BLACK, COL_BLUE, COL_GREEN, COL_CYAN, COL_RED, COL_MAGENTA,
                COL_BROWN, COL_GRAY, COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTGREEN,
                COL_LIGHTCYAN, COL_LIGHTRED, COL_LIGHTMAGENTA, COL_YELLOW, COL_WHITE
            };

            if (bClip && !bRotate)
            {
                // only show the first quarter of the text
                tools::Rectangle aRect( r.TopLeft(), Size( r.GetWidth()/2, r.GetHeight()/2 ) );
                rDev.SetClipRegion( vcl::Region( aRect ) );
            }

            for (int i = 1; i < nPrintNumCopies+1; i++)
            {
                int nFontHeight=0, nFontIndex=0, nFontColorIndex=0;

                if (nPrintNumCopies == 1)
                {
                    float const nFontMagnitude = 0.25f;
                    // random font size to avoid buffering
                    nFontHeight = 1 + nFontMagnitude * (0.9 + comphelper::rng::uniform_real_distribution(0.0, std::nextafter(0.1, DBL_MAX))) * (r.Bottom() - r.Top());
                    nFontIndex=0;
                    nFontColorIndex=0;
                }
                else
                {
                    // random font size to avoid buffering
                    nFontHeight = 1 + i * (0.9 + comphelper::rng::uniform_real_distribution(0.0, std::nextafter(0.1, DBL_MAX))) * (r.Top() - r.Bottom()) / nPrintNumCopies;
                    nFontIndex = (i % aFontNames.size());
                    nFontColorIndex=(i % aFontNames.size());
                }

                rDev.SetTextColor(nCols[nFontColorIndex]);
                vcl::Font aFont( aFontNames[nFontIndex], Size(0, nFontHeight ));

                if (bRotate)
                {
                    tools::Rectangle aFontRect = r;

                    int nHeight = r.GetHeight();

                    // move the text to the bottom of the bounding rect before rotating
                    aFontRect.AdjustTop(nHeight/2 );
                    aFontRect.AdjustBottom(nHeight );

                    aFont.SetOrientation(450_deg10); // 45 degrees

                    rDev.SetFont(aFont);
                    rDev.DrawText(aFontRect, aText);

                    if (bClip)
                    {
                        tools::Rectangle aClipRect( Point( r.Left(), r.Top() + ( r.GetHeight()/2 ) ) , Size( r.GetWidth()/2, r.GetHeight()/2 ) );
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

        static void drawComplex (OutputDevice &rDev, tools::Rectangle r)
        {
            const unsigned char pInvalid[] = { 0xfe, 0x1f, 0 };
            const unsigned char pDiacritic1[] = { 0x61, 0xcc, 0x8a, 0xcc, 0x8c, 0 };
            const unsigned char pDiacritic2[] = { 0x61, 0xcc, 0x88, 0xcc, 0x86, 0 };
            const unsigned char pDiacritic3[] = { 0x61, 0xcc, 0x8b, 0xcc, 0x87, 0 };
            const unsigned char pJustification[] = {
                0x64, 0x20, 0xc3, 0xa1, 0xc3, 0xa9, 0x77, 0xc4, 0x8d,
                0xc5, 0xa1, 0xc3, 0xbd, 0xc5, 0x99, 0x20, 0xc4, 0x9b, 0
            };
            const unsigned char pEmojis[] = {
                0xf0, 0x9f, 0x8d, 0x80, 0xf0, 0x9f, 0x91, 0x98,
                0xf0, 0x9f, 0x92, 0x8a, 0xf0, 0x9f, 0x92, 0x99,
                0xf0, 0x9f, 0x92, 0xa4, 0xf0, 0x9f, 0x94, 0x90, 0
            };
            const unsigned char pThreeBowlG[] = {
                0xe2, 0x9a, 0x82, 0xe2, 0x99, 0xa8, 0xc4, 0x9e, 0
            };
            const unsigned char pWavesAndDomino[] = {
                0xe2, 0x99, 0x92, 0xf0, 0x9f, 0x81, 0xa0,
                0xf0, 0x9f, 0x82, 0x93, 0
            };
            const unsigned char pSpadesAndBits[] = {
                0xf0, 0x9f, 0x82, 0xa1, 0xc2, 0xa2, 0xc2, 0xa2, 0
            };

            static struct {
                const char *mpFont;
                const char *mpString;
            } const aRuns[] = {
#define SET(font,string) { font, reinterpret_cast<const char *>(string) }
                {"sans", "a"},           // logical font - no 'sans' font.
                {"opensymbol", "#$%"},   // font fallback - $ is missing.
                SET("sans", pInvalid),      // unicode invalid character
                // tdf#96266 - stacking diacritics
                SET("carlito", pDiacritic1),
                SET("carlito", pDiacritic2),
                SET("carlito", pDiacritic3),
                SET("liberation sans", pDiacritic1),
                SET("liberation sans", pDiacritic2),
                SET("liberation sans", pDiacritic3),
                SET("liberation sans", pDiacritic3),

                // tdf#95222 - justification issue
                // - FIXME: replicate justification
                SET("gentium basic", pJustification),

                // tdf#97319 - Unicode beyond BMP; SMP & Plane 2
                SET("symbola", pEmojis),
                SET("symbola", pThreeBowlG),
                SET("symbola", pWavesAndDomino),
                SET("symbola", pSpadesAndBits),
            };

            // Nice clean white background
            rDev.DrawWallpaper(r, Wallpaper(COL_WHITE));
            rDev.SetClipRegion(vcl::Region(r));

            Point aPos(r.Left(), r.Top()+20);

            tools::Long nMaxTextHeight = 0;
            for (size_t i = 0; i < std::size(aRuns); ++i)
            {
                // Legend
                vcl::Font aIndexFont(u"sans"_ustr, Size(0,20));
                aIndexFont.SetColor( COL_BLACK);
                tools::Rectangle aTextRect;
                rDev.SetFont(aIndexFont);
                OUString aText = OUString::number(i) + ".";
                rDev.DrawText(aPos, aText);
                if (rDev.GetTextBoundRect(aTextRect, aText))
                    aPos.Move(aTextRect.GetWidth() + 8, 0);

                // Text
                FontWeight aWeights[] = { WEIGHT_NORMAL,
                                          WEIGHT_BOLD,
                                          WEIGHT_NORMAL };
                FontItalic const aItalics[] = { ITALIC_NONE,
                                          ITALIC_NONE,
                                          ITALIC_NORMAL };
                vcl::Font aFont(OUString::createFromAscii(
                                    aRuns[i].mpFont),
                                Size(0,42));
                aFont.SetColor(COL_BLACK);
                for (size_t j = 0; j < std::size(aWeights); ++j)
                {
                    aFont.SetItalic(aItalics[j]);
                    aFont.SetWeight(aWeights[j]);
                    rDev.SetFont(aFont);

                    OUString aString(aRuns[i].mpString,
                                     strlen(aRuns[i].mpString),
                                     RTL_TEXTENCODING_UTF8);
                    tools::Long nNewX = drawStringBox(rDev, aPos, aString,
                                               nMaxTextHeight);

                    aPos.setX( nNewX );

                    if (aPos.X() >= r.Right())
                    {
                        aPos = Point(r.Left(), aPos.Y() + nMaxTextHeight + 15);
                        nMaxTextHeight = 0;
                        if(j>0)
                            j--; // re-render the last point.
                    }
                    if (aPos.Y() > r.Bottom())
                        break;
                }
                if (aPos.Y() > r.Bottom())
                    break;
            }

            rDev.SetClipRegion();
        }
        // render text, bbox, DX arrays etc.
        static tools::Long drawStringBox(OutputDevice &rDev, Point aPos,
                           const OUString &aText,
                           tools::Long &nMaxTextHeight)
        {
            rDev.Push();
            {
                tools::Rectangle aTextRect;

                rDev.DrawText(aPos,aText);

                if (rDev.GetTextBoundRect(aTextRect, aText))
                {
                    aTextRect.Move(aPos.X(), aPos.Y());
                    rDev.SetFillColor();
                    rDev.SetLineColor(COL_BLACK);
                    rDev.DrawRect(aTextRect);
                    if (aTextRect.GetHeight() > nMaxTextHeight)
                        nMaxTextHeight = aTextRect.GetHeight();
                    // This should intersect with the text
                    tools::Rectangle aInnerRect(
                        aTextRect.Left()+1, aTextRect.Top()+1,
                        aTextRect.Right()-1, aTextRect.Bottom()-1);
                    rDev.SetLineColor(COL_WHITE);
                    rDev.SetRasterOp(RasterOp::Xor);
                    rDev.DrawRect(aInnerRect);
                    rDev.SetRasterOp(RasterOp::OverPaint);
                }

                // DX array rendering
                KernArray aItems;
                rDev.GetTextArray(aText, &aItems);
                for (tools::Long j = 0; j < aText.getLength(); ++j)
                {
                    Point aTop = aTextRect.TopLeft();
                    Point aBottom = aTop;
                    aTop.Move(aItems[j], 0);
                    aBottom.Move(aItems[j], aTextRect.GetHeight());
                    rDev.SetLineColor(COL_RED);
                    rDev.SetRasterOp(RasterOp::Xor);
                    rDev.DrawLine(aTop,aBottom);
                    rDev.SetRasterOp(RasterOp::OverPaint);
                }

                aPos.Move(aTextRect.GetWidth() + 16, 0);
            }
            rDev.Pop();
            return aPos.X();
        }
    };

    struct DrawCheckered : public RegionRenderer
    {
        RENDER_DETAILS(checks,KEY_C,20)
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<tools::Rectangle> aRegions(DemoRenderer::partition(rCtx, 2, 2));
                for (size_t i = 0; i < aRegions.size(); i++)
                {
                    vcl::Region aRegion;
                    tools::Rectangle aSub(aRegions[i]);
                    tools::Rectangle aSmaller(aSub);
                    aSmaller.Move(10,10);
                    aSmaller.setWidth(aSmaller.getOpenWidth()-20);
                    aSmaller.setHeight(aSmaller.getOpenHeight()-24);
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
                        aPoly.Rotate(aSub.Center(), 450_deg10);
                        aPoly.Clip(aSmaller);
                        aRegion = vcl::Region(aPoly);
                        break;
                    }
                    case 3:
                    {
                        tools::PolyPolygon aPolyPoly;
                        sal_Int32 nTW = aSub.GetWidth()/6;
                        sal_Int32 nTH = aSub.GetHeight()/6;
                        tools::Rectangle aTiny(Point(4, 4), Size(nTW*2, nTH*2));
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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            maCheckered.RenderRegion(rDev, r, rCtx);

            tools::Long nDx = r.GetWidth()/20;
            tools::Long nDy = r.GetHeight()/20;
            tools::Rectangle aShrunk(r);
            aShrunk.Move(nDx, nDy);
            aShrunk.SetSize(Size(r.GetWidth()-nDx*2,
                                 r.GetHeight()-nDy*2));
            tools::Polygon aPoly(aShrunk);
            tools::PolyPolygon aPPoly(aPoly);
            rDev.SetLineColor(COL_RED);
            rDev.SetFillColor(COL_RED);
            // This hits the optional 'drawPolyPolygon' code-path
            rDev.DrawTransparent(aPPoly, 64);
        }
    };

    struct DrawEllipse : public RegionRenderer
    {
        RENDER_DETAILS(ellipse,KEY_E,500)
        static void doInvert(OutputDevice &rDev, const tools::Rectangle &r,
                      InvertFlags nFlags)
        {
            rDev.Invert(r, nFlags);
            if (r.GetWidth() > 10 && r.GetHeight() > 10)
            {
                tools::Rectangle aSmall(r.Center()-Point(4,4), Size(8,8));
                rDev.Invert(aSmall,nFlags);
            }
        }
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            rDev.SetLineColor(COL_RED);
            rDev.SetFillColor(COL_GREEN);
            rDev.DrawEllipse(r);

            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                auto aRegions = partition(rCtx, 2, 2);
                doInvert(rDev, aRegions[0], InvertFlags::NONE);
                rDev.DrawText(aRegions[0], u"InvertFlags::NONE"_ustr);
                doInvert(rDev, aRegions[1], InvertFlags::N50);
                rDev.DrawText(aRegions[1], u"InvertFlags::N50"_ustr);
                doInvert(rDev, aRegions[3], InvertFlags::TrackFrame);
                rDev.DrawText(aRegions[3], u"InvertFlags::TrackFrame"_ustr);
            }
        }
    };

    struct DrawGradient : public RegionRenderer
    {
        RENDER_DETAILS(gradient,KEY_G,50)
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<tools::Rectangle> aRegions(DemoRenderer::partition(rCtx,5, 4));
                static Color const nStartCols[] = {
                    COL_RED, COL_RED, COL_RED, COL_GREEN, COL_GREEN,
                    COL_BLUE, COL_BLUE, COL_BLUE, COL_CYAN, COL_CYAN,
                    COL_BLACK, COL_LIGHTGRAY, COL_WHITE, COL_BLUE, COL_CYAN,
                    COL_WHITE, COL_WHITE, COL_WHITE, COL_BLACK, COL_BLACK
                };
                static Color const nEndCols[] = {
                    COL_WHITE, COL_WHITE, COL_WHITE, COL_BLACK, COL_BLACK,
                    COL_RED, COL_RED, COL_RED, COL_GREEN, COL_GREEN,
                    COL_GRAY, COL_GRAY, COL_LIGHTGRAY, COL_LIGHTBLUE, COL_LIGHTCYAN,
                    COL_BLUE, COL_BLUE, COL_BLUE, COL_CYAN, COL_CYAN
                };
                css::awt::GradientStyle eStyles[] = {
                    css::awt::GradientStyle_LINEAR, css::awt::GradientStyle_AXIAL, css::awt::GradientStyle_RADIAL, css::awt::GradientStyle_ELLIPTICAL, css::awt::GradientStyle_SQUARE,
                    css::awt::GradientStyle_RECT, css::awt::GradientStyle::GradientStyle_MAKE_FIXED_SIZE, css::awt::GradientStyle_LINEAR, css::awt::GradientStyle_RADIAL, css::awt::GradientStyle_LINEAR,
                    css::awt::GradientStyle_LINEAR, css::awt::GradientStyle_AXIAL, css::awt::GradientStyle_RADIAL, css::awt::GradientStyle_ELLIPTICAL, css::awt::GradientStyle_SQUARE,
                    css::awt::GradientStyle_RECT, css::awt::GradientStyle::GradientStyle_MAKE_FIXED_SIZE, css::awt::GradientStyle_LINEAR, css::awt::GradientStyle_RADIAL, css::awt::GradientStyle_LINEAR
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
                assert(aRegions.size() <= std::size(nStartCols));
                assert(aRegions.size() <= std::size(nEndCols));
                assert(aRegions.size() <= std::size(eStyles));
                assert(aRegions.size() <= std::size(nAngles));
                assert(aRegions.size() <= std::size(nBorders));
                for (size_t i = 0; i < aRegions.size(); i++)
                {
                    tools::Rectangle aSub = aRegions[i];
                    Gradient aGradient;
                    aGradient.SetStartColor(nStartCols[i]);
                    aGradient.SetEndColor(nEndCols[i]);
                    aGradient.SetStyle(eStyles[i]);
                    aGradient.SetAngle(Degree10(nAngles[i]));
                    aGradient.SetBorder(nBorders[i]);
                    rDev.DrawGradient(aSub, aGradient);
                }
            }
            else
            {
                Gradient aGradient;
                aGradient.SetStartColor(COL_YELLOW);
                aGradient.SetEndColor(COL_RED);
                aGradient.SetStyle(css::awt::GradientStyle_RECT);
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
        static void SimulateBorderStretch(OutputDevice &rDev, const tools::Rectangle& r)
        {
            BitmapEx aPageShadowMask(u"sw/res/page-shadow-mask.png"_ustr);

            BitmapEx aRight(aPageShadowMask);
            sal_Int32 nSlice = (aPageShadowMask.GetSizePixel().Width() - 3) / 4;
            // a width x 1 slice
            aRight.Crop(tools::Rectangle(Point((nSlice * 3) + 3, (nSlice * 2) + 1),
                                  Size(nSlice, 1)));
            AlphaMask aAlphaMask(aRight.GetBitmap());
            Bitmap aBlockColor(aAlphaMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
            aBlockColor.Erase(COL_RED);
            BitmapEx aShadowStretch(aBlockColor, aAlphaMask);

            Point aRenderPt(r.TopLeft());

            tools::Long aSizes[] = { 200, 100, 200, 100, 50, 5, 2 };

            // and yes - we really do this in the page border rendering code ...
            for (const auto& rSize : aSizes)
            {
                aShadowStretch.Scale(Size(aShadowStretch.GetSizePixel().Width(), rSize),
                                     BmpScaleFlag::Fast);

                rDev.DrawBitmapEx(aRenderPt, aShadowStretch);
                aRenderPt.Move(aShadowStretch.GetSizePixel().Width() + 4, 0);
            }

            AlphaMask aWholeMask(aPageShadowMask.GetBitmap());
            aBlockColor = Bitmap(aPageShadowMask.GetSizePixel(), vcl::PixelFormat::N24_BPP);
            aBlockColor.Erase(COL_GREEN);
            BitmapEx aWhole(aBlockColor, aWholeMask);

            aRenderPt = r.Center();
            aRenderPt.Move(nSlice+1, 0);

            // An offset background for alpha rendering
            rDev.SetFillColor(COL_BLUE);
            tools::Rectangle aSurround(r.Center(), aPageShadowMask.GetSizePixel());
            rDev.DrawRect(aSurround);
            rDev.DrawBitmapEx(aRenderPt, aWhole);
        }

        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &) override
        {
            static struct {
                double nX, nY;
            } const aPoints[] = { { 0.1, 0.1 }, { 0.9, 0.9 },
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
                    tools::Rectangle aSubRect(r);
                    aSubRect.Move(x * r.GetWidth()/3, y * r.GetHeight()/3);
                    aSubRect.SetSize(Size(r.GetWidth()/2, r.GetHeight()/4));
                    tools::Polygon aPoly(std::size(aPoints));
                    for (size_t v = 0; v < std::size(aPoints); v++)
                    {
                        aPoly.SetPoint(Point(aSubRect.Left() +
                                             aSubRect.GetWidth() * aPoints[v].nX,
                                             aSubRect.Top() +
                                             aSubRect.GetHeight() * aPoints[v].nY),
                                       v);
                    }
                    rDev.SetLineColor(COL_YELLOW);
                    rDev.SetFillColor(COL_BLACK);
                    rDev.DrawPolygon(aPoly);

                    // now move and add to the polypolygon
                    aPoly.Move(0, r.GetHeight()/2);
                    aPolyPoly.Insert(aPoly);
                }
            }
            rDev.SetLineColor(COL_LIGHTRED);
            rDev.SetFillColor(COL_GREEN);
            rDev.DrawTransparent(aPolyPoly, 50);
        }
    };

    struct DrawClipped : public RegionRenderer
    {
        RENDER_DETAILS(clip,KEY_D,10)
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &) override
        {
            std::vector<tools::Rectangle> aRegions(DemoRenderer::partition(r, 2, 2));
            const int nLimits[] = { 4, -100 };
            for (int i = 0; i < 2; ++i)
            {
                sal_uInt16 nHue = 0;
                rDev.Push(vcl::PushFlags::CLIPREGION);
                tools::Rectangle aOuter = aRegions[i];
                tools::Rectangle aInner = aOuter;
                while (aInner.GetWidth() > nLimits[i] && aInner.GetHeight() > nLimits[i])
                {
                    aInner.expand(-1);
                    rDev.SetClipRegion(vcl::Region(aInner));
                    rDev.SetFillColor(Color::HSBtoRGB(nHue, 75, 100));
                    nHue = (nHue + 97) % 360;
                    rDev.DrawRect(aOuter);
                }
                rDev.Pop();
            }

            {
                sal_uInt16 nHue = 0;
                tools::Rectangle aOuter = aRegions[2];
                std::vector<tools::Rectangle> aPieces(DemoRenderer::partition(aOuter, 2, 2));
                for (int j = 0; j < std::min(aOuter.GetWidth(), aOuter.GetHeight())/5; ++j)
                {
                    rDev.Push(vcl::PushFlags::CLIPREGION);

                    vcl::Region aClipRegion;
                    for (int i = 0; i < 4; ++i)
                    {
                        aPieces[i].expand(-1);
                        aPieces[i].Move(2 - i/2, 2 - i/2);
                        aClipRegion.Union(aPieces[i]);
                    }
                    assert (aClipRegion.getRegionBand());
                    rDev.SetClipRegion(aClipRegion);
                    rDev.SetFillColor(Color::HSBtoRGB(nHue, 75, 75));
                    nHue = (nHue + 97) % 360;
                    rDev.DrawRect(aOuter);

                    rDev.Pop();
                }
            }

            {
                sal_uInt16 nHue = 0;
                tools::Rectangle aOuter = aRegions[3];
                std::vector<tools::Rectangle> aPieces(DemoRenderer::partition(aOuter, 2, 2));
                bool bDone = false;
                while (!bDone)
                {
                    rDev.Push(vcl::PushFlags::CLIPREGION);

                    for (int i = 0; i < 4; ++i)
                    {
                        vcl::Region aClipRegion;
                        tools::Polygon aPoly;
                        switch (i) {
                        case 3:
                        case 0: // 45degree rectangle.
                            aPoly = tools::Polygon(aPieces[i]);
                            aPoly.Rotate(aPieces[i].Center(), 450_deg10);
                            break;
                        case 1: // arc
                            aPoly = tools::Polygon(aPieces[i],
                                                   aPieces[i].TopLeft(),
                                                   aPieces[i].BottomRight());
                            break;
                        case 2:
                            aPoly = tools::Polygon(aPieces[i],
                                                   aPieces[i].GetWidth()/5,
                                                   aPieces[i].GetHeight()/5);
                            aPoly.Rotate(aPieces[i].Center(), 450_deg10);
                            break;
                        }
                        aClipRegion = vcl::Region(aPoly);
                        aPieces[i].expand(-1);
                        aPieces[i].Move(2 - i/2, 2 - i/2);

                        bDone = aPieces[i].GetWidth() < 4 ||
                                aPieces[i].GetHeight() < 4;

                        if (!bDone)
                        {
                            assert (!aClipRegion.getRegionBand());

                            rDev.SetClipRegion(aClipRegion);
                            rDev.SetFillColor(Color::HSBtoRGB(nHue, 50, 75));
                            nHue = (nHue + 97) % 360;
                            rDev.DrawRect(aOuter);
                        }
                    }

                    rDev.Pop();
                }
            }
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

        static void SizeAndRender(OutputDevice &rDev, const tools::Rectangle& r, RenderType eType,
                                  const RenderContext &rCtx)
        {
            ScopedVclPtr<VirtualDevice> pNested;

            if (static_cast<int>(eType) < RENDER_AS_BITMAPEX)
                pNested = VclPtr<VirtualDevice>::Create(rDev).get();
            else
                pNested = VclPtr<VirtualDevice>::Create(rDev,DeviceFormat::WITH_ALPHA).get();

            pNested->SetOutputSizePixel(r.GetSize());
            tools::Rectangle aWhole(Point(0,0), r.GetSize());

            // mini me
            rCtx.mpDemoRenderer->drawToDevice(*pNested, r.GetSize(), true);

            if (eType == RENDER_AS_BITMAP)
            {
                Bitmap aBitmap(pNested->GetBitmap(Point(0,0),aWhole.GetSize()));
                rDev.DrawBitmap(r.TopLeft(), aBitmap);
            }
            else if (eType == RENDER_AS_BITMAPEX)
            {
                BitmapEx aBitmapEx(pNested->GetBitmap(Point(0,0),aWhole.GetSize()));
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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            // avoid infinite recursion
            if (rCtx.mbVDev)
                return;

            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                std::vector<tools::Rectangle> aRegions(DemoRenderer::partition(rCtx,2, 2));
                DemoRenderer::clearRects(rDev, aRegions);

                RenderType const eRenderTypes[] { RENDER_AS_BITMAP, RENDER_AS_OUTDEV,
                                                  RENDER_AS_BITMAPEX, RENDER_AS_ALPHA_OUTDEV };
                for (size_t i = 0; i < aRegions.size(); i++)
                    SizeAndRender(rDev, aRegions[i], eRenderTypes[i], rCtx);
            }
            else
                SizeAndRender(rDev, r, RENDER_AS_BITMAP, rCtx);
        }
    };

    struct DrawXOR : public RegionRenderer
    {
        RENDER_DETAILS(xor,KEY_X,1)

        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            // avoid infinite recursion
            if (rCtx.mbVDev)
                return;

            rDev.Push();

            AntialiasingFlags nFlags = rDev.GetAntialiasing();
            rDev.SetAntialiasing(nFlags & ~AntialiasingFlags::Enable);
            rDev.SetRasterOp( RasterOp::Xor );

            rCtx.mpDemoRenderer->drawThumbs(rDev, r, true);

            rDev.Pop();
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
            for (size_t i = 0; i < std::size(pNames); i++)
            {
                maIconNames.push_back(OUString::createFromAscii(pNames[i]));
                maIcons.emplace_back(maIconNames[i]);
            }
        }

        void LoadAllImages()
        {
            if (bHasLoadedAll)
                return;
            bHasLoadedAll = true;

            css::uno::Reference<css::container::XNameAccess> xRef(ImageTree::get().getNameAccess());
            const css::uno::Sequence< OUString > aAllIcons = xRef->getElementNames();

            for (const auto& rIcon : aAllIcons)
            {
                if (rIcon.endsWithIgnoreAsciiCase("svg"))
                    continue; // too slow to load.
                maIconNames.push_back(rIcon);
                maIcons.emplace_back(rIcon);
            }
        }

        void doDrawIcons(OutputDevice &rDev, tools::Rectangle r, bool bExpanded)
        {
            tools::Long nMaxH = 0;
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
                        aTransform.shearX(static_cast<double>((i >> 2) % 8) / 8);
                        aTransform.shearY(static_cast<double>((i >> 4) % 8) / 8);
                        break;
                    case 3:
                        aTransform.translate(-aSize.Width()/2, -aSize.Height()/2);
                        aTransform.rotate(i);
                        if (i & 0x100)
                        {
                            aTransform.shearX(static_cast<double>((i >> 2) % 8) / 8);
                            aTransform.shearY(static_cast<double>((i >> 4) % 8) / 8);
                        }
                        aTransform.translate(aSize.Width()/2,  aSize.Height()/2);
                        break;
                    default:
                        aTransform.translate(-aSize.Width()/2, -aSize.Height()/2);
                        aTransform.rotate(2 * 2 * M_PI * i / nToRender);
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

        static BitmapEx AlphaRecovery(OutputDevice &rDev, Point aPt, BitmapEx const &aSrc)
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
            Bitmap aRecovered(aSrc.GetSizePixel(), vcl::PixelFormat::N24_BPP);
            {
                BitmapScopedWriteAccess pMaskAcc(aMask);
                BitmapScopedWriteAccess pRecAcc(aRecovered);
                BitmapScopedReadAccess pAccW(aWhiteBmp); // a * pix + (1-a)
                BitmapScopedReadAccess pAccB(aBlackBmp); // a * pix + 0
                int nSizeX = aSrc.GetSizePixel().Width();
                int nSizeY = aSrc.GetSizePixel().Height();
                for (int y = 0; y < nSizeY; y++)
                {
                    Scanline pScanlineMask = pMaskAcc->GetScanline( y );
                    Scanline pScanlineRec = pRecAcc->GetScanline( y );
                    Scanline pScanlineW = pAccW->GetScanline( y );
                    Scanline pScanlineB = pAccB->GetScanline( y );
                    for (int x = 0; x < nSizeX; x++)
                    {
                        BitmapColor aColW = pAccW->GetPixelFromData(pScanlineW,x);
                        BitmapColor aColB = pAccB->GetPixelFromData(pScanlineB,x);
                        tools::Long nAR = static_cast<tools::Long>(aColW.GetRed() - aColB.GetRed()); // (1-a)
                        tools::Long nAG = static_cast<tools::Long>(aColW.GetGreen() - aColB.GetGreen()); // (1-a)
                        tools::Long nAB = static_cast<tools::Long>(aColW.GetBlue() - aColB.GetBlue()); // (1-a)

#define CLAMP(a,b,c) (((a)<=(b))?(b):(((a)>=(c))?(c):(a)))

                        // we get the most precision from the largest delta
                        tools::Long nInverseAlpha = std::max(nAR, std::max(nAG, nAB)); // (1-a)
                        nInverseAlpha = CLAMP(nInverseAlpha, 0, 255);
                        tools::Long nAlpha = 255 - nInverseAlpha;

                        pMaskAcc->SetPixelOnData(pScanlineMask,x,BitmapColor(static_cast<sal_Int8>(CLAMP(nInverseAlpha,0,255))));
                        // now recover the pixels
                        tools::Long nR = (aColW.GetRed() + aColB.GetRed() - nInverseAlpha) * 128;
                        tools::Long nG = (aColW.GetGreen() + aColB.GetGreen() - nInverseAlpha) * 128;
                        tools::Long nB = (aColW.GetBlue() + aColB.GetBlue() - nInverseAlpha) * 128;
                        if (nAlpha == 0)
                        { // doesn't matter what's behind transparency
                            nR = nG = nB = 0;
                        }
                        else
                        {
                            nR /= nAlpha; nG /= nAlpha; nB /= nAlpha;
                        }
                        pRecAcc->SetPixelOnData(pScanlineRec,x,BitmapColor(
                                                static_cast<sal_uInt8>(CLAMP(nR,0,255)),
                                                static_cast<sal_uInt8>(CLAMP(nG,0,255)),
                                                static_cast<sal_uInt8>(CLAMP(nB,0,255))));
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

        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &rCtx) override
        {
            if (rCtx.meStyle == RENDER_EXPANDED)
            {
                LoadAllImages();

                Point aLocation(0,maIcons[0].GetSizePixel().Height() + 8);
                for (size_t i = 0; i < maIcons.size(); i++)
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
                    rDev.DrawBitmap(aAbove, aSrc.GetAlphaMask().GetBitmap());

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
                    aGrey.Convert(BmpConversion::N8BitGreys);
                    rDev.DrawBitmap(aBelow, aGrey);

                    aBelow.Move(aGrey.GetSizePixel().Width(),0);
                    const BitmapEx& aGreyMask(aSrc);
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
        virtual void RenderRegion(OutputDevice &rDev, tools::Rectangle r,
                                  const RenderContext &) override
        {
            Bitmap aBitmap(rDev.GetBitmap(Point(0,0),rDev.GetOutputSizePixel()));
            aBitmap.Scale(r.GetSize(), BmpScaleFlag::BestQuality);
            rDev.DrawBitmap(r.TopLeft(), aBitmap);
        }
    };

    void drawThumbs(vcl::RenderContext& rDev, tools::Rectangle aRect, bool bVDev)
    {
        RenderContext aCtx;
        aCtx.meStyle = RENDER_THUMB;
        aCtx.mbVDev = bVDev;
        aCtx.mpDemoRenderer = this;
        aCtx.maSize = aRect.GetSize();
        std::vector<tools::Rectangle> aRegions(partition(aRect, mnSegmentsX, mnSegmentsY));
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
                    double nStartTime = getTimeNow();
                    for (int j = 0; j < r->getTestRepeatCount() * THUMB_REPEAT_FACTOR; j++)
                        r->RenderRegion(rDev, aRegions[i], aCtx);
                    addTime(i, (getTimeNow() - nStartTime) / THUMB_REPEAT_FACTOR);
                } else
                    for (int j = 0; j < r->getTestRepeatCount(); j++)
                        r->RenderRegion(rDev, aRegions[i], aCtx);
            }
            else
                r->RenderRegion(rDev, aRegions[i], aCtx);

            rDev.SetClipRegion();
        }
    }

    void drawToDevice(vcl::RenderContext& rDev, Size aSize, bool bVDev)
    {
        RenderContext aCtx;
        aCtx.mbVDev = bVDev;
        aCtx.mpDemoRenderer = this;
        aCtx.maSize = aSize;
        tools::Rectangle aWholeWin(Point(0,0), rDev.GetOutputSizePixel());

        drawBackground(rDev, aWholeWin);

        if (!bVDev /* want everything in the vdev */ &&
            mnSelectedRenderer >= 0 &&
            o3tl::make_unsigned(mnSelectedRenderer) < maRenderers.size())
        {
            aCtx.meStyle = RENDER_EXPANDED;
            RegionRenderer * r = maRenderers[mnSelectedRenderer];
            // profiling?
            if (getIterCount() > 0)
            {
                double nStartTime = getTimeNow();
                for (int i = 0; i < r->getTestRepeatCount(); i++)
                    r->RenderRegion(rDev, aWholeWin, aCtx);
                addTime(mnSelectedRenderer, getTimeNow() - nStartTime);
            } else
                r->RenderRegion(rDev, aWholeWin, aCtx);
        }
        else
            drawThumbs(rDev, aWholeWin, bVDev);
    }
    std::vector<VclPtr<vcl::Window> > maInvalidates;
    void addInvalidate(vcl::Window *pWindow) { maInvalidates.emplace_back(pWindow); };
    void removeInvalidate(vcl::Window *pWindow)
    {
        auto aIt = std::find(maInvalidates.begin(), maInvalidates.end(), pWindow);
        if (aIt != maInvalidates.end())
            maInvalidates.erase(aIt);
    }
    void Invalidate()
    {
        for (auto const& invalidate : maInvalidates)
            invalidate->Invalidate();
    }
};

}

#if FIXME_BOUNCE_BUTTON
IMPL_LINK_NOARG(DemoRenderer,BounceTimerCb,Timer*,void)
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
    std::vector<tools::Rectangle> aRegions(partition(GetSizePixel(), mnSegmentsX, mnSegmentsY));
    for (size_t i = 0; i < aRegions.size(); i++)
    {
        if (aRegions[i].Contains(rMEvt.GetPosPixel()))
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
        maBounce.SetInvokeHandler(LINK(this,DemoRenderer,BounceTimerCb));
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
    maRenderers.push_back(new DrawLines);
    maRenderers.push_back(new DrawText);
    maRenderers.push_back(new DrawPoly);
    maRenderers.push_back(new DrawEllipse);
    maRenderers.push_back(new DrawCheckered);
    maRenderers.push_back(new DrawBitmapEx);
    maRenderers.push_back(new DrawBitmap);
    maRenderers.push_back(new DrawGradient);
    maRenderers.push_back(new DrawPolyPolygons);
    maRenderers.push_back(new DrawClipped);
    maRenderers.push_back(new DrawToVirtualDevice);
    maRenderers.push_back(new DrawXOR);
    maRenderers.push_back(new DrawIcons());
    maRenderers.push_back(new FetchDrawBitmap);
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
                OUStringToOString(maRenderers[i]->getName(),
                RTL_TEXTENCODING_UTF8).getStr(), avgtime,
                maRenderers[i]->countTime, maRenderers[i]->getTestRepeatCount(),
                (style == RENDER_THUMB) ? THUMB_REPEAT_FACTOR : 1);
        maRenderers[i]->sumTime = 0;
        maRenderers[i]->countTime = 0;
    }
    geomean = pow(geomean, 1.0/maRenderers.size());
    fprintf(stderr, "GEOMEAN_%s: %f\n", style == RENDER_THUMB ? "THUMB": "EXPANDED", geomean);
    return geomean;
}

void DemoRenderer::setIterCount(sal_Int32 i)
{
    iterCount = i;
}

sal_Int32 DemoRenderer::getIterCount() const
{
    return iterCount;
}

void DemoRenderer::addTime(int i, double t)
{
    maRenderers[i]->sumTime += t / maRenderers[i]->getTestRepeatCount();
    maRenderers[i]->countTime++;
}

void DemoRenderer::selectRenderer(std::u16string_view rName )
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
    if (mnSelectedRenderer == static_cast<signed>(maRenderers.size()))
        mnSelectedRenderer = -1;
    Invalidate();
    return mnSelectedRenderer;
}

namespace {

class DemoWin : public WorkWindow
{
    DemoRenderer &mrRenderer;
    bool underTesting;
    bool testThreads;

    class RenderThread final : public salhelper::Thread {
        DemoWin  &mrWin;
        sal_uInt32 const mnDelaySecs = 0;
    public:
        RenderThread(DemoWin &rWin, sal_uInt32 nDelaySecs)
            : Thread("vcldemo render thread")
            , mrWin(rWin)
            , mnDelaySecs(nDelaySecs)
        {
            launch();
        }
        virtual ~RenderThread() override
        {
            join();
        }
        virtual void execute() override
        {
            std::this_thread::sleep_for(std::chrono::seconds(mnDelaySecs));

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
    virtual ~DemoWin() override
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
        if (mrRenderer.MouseButtonDown(rMEvt))
            return;

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
            pNewWin->SetText(u"Another interactive VCL demo window"_ustr);
            pNewWin->Show();
        }
    }
    virtual void KeyInput(const KeyEvent& rKEvt) override
    {
        mrRenderer.SetSizePixel(GetSizePixel());
        mrRenderer.KeyInput(rKEvt);
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override
    {
        mrRenderer.SetSizePixel(GetSizePixel());
        fprintf(stderr, "DemoWin::Paint(%" SAL_PRIdINT64 ",%" SAL_PRIdINT64 ",%" SAL_PRIdINT64 ",%" SAL_PRIdINT64 ")\n", sal_Int64(rRect.Left()), sal_Int64(rRect.Top()), sal_Int64(rRect.getOpenWidth()), sal_Int64(rRect.getOpenHeight()));
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

        fprintf(stderr, "GEOMEAN_TOTAL: %f\n", pow(thumbGEOMEAN * expandedGEOMEAN, 0.5));
        Application::Quit();
    }
};

struct PointerData {
    PointerStyle eStyle;
    const char * name;
};

}

const PointerData gvPointerData [] = {
    { PointerStyle::Null, "Null" },
    { PointerStyle::Magnify, "Magnify" },
    { PointerStyle::Fill, "Fill" },
    { PointerStyle::MoveData, "MoveData" },
    { PointerStyle::CopyData, "CopyData" },
    { PointerStyle::MoveFile, "MoveFile" },
    { PointerStyle::CopyFile, "CopyFile" },
    { PointerStyle::MoveFiles, "MoveFiles" },
    { PointerStyle::CopyFiles, "CopyFiles" },
    { PointerStyle::NotAllowed, "NotAllowed" },
    { PointerStyle::Rotate, "Rotate" },
    { PointerStyle::HShear, "HShear" },
    { PointerStyle::VShear, "VShear" },
    { PointerStyle::DrawLine, "DrawLine" },
    { PointerStyle::DrawRect, "DrawRect" },
    { PointerStyle::DrawPolygon, "DrawPolygon" },
    { PointerStyle::DrawBezier, "DrawBezier" },
    { PointerStyle::DrawArc, "DrawArc" },
    { PointerStyle::DrawPie, "DrawPie" },
    { PointerStyle::DrawCircleCut, "DrawCircleCut" },
    { PointerStyle::DrawEllipse, "DrawEllipse" },
    { PointerStyle::DrawConnect, "DrawConnect" },
    { PointerStyle::DrawText, "DrawText" },
    { PointerStyle::Mirror, "Mirror" },
    { PointerStyle::Crook, "Crook" },
    { PointerStyle::Crop, "Crop" },
    { PointerStyle::MovePoint, "MovePoint" },
    { PointerStyle::MoveBezierWeight, "MoveBezierWeight" },
    { PointerStyle::DrawFreehand, "DrawFreehand" },
    { PointerStyle::DrawCaption, "DrawCaption" },
    { PointerStyle::LinkData, "LinkData" },
    { PointerStyle::MoveDataLink, "MoveDataLink" },
    { PointerStyle::CopyDataLink, "CopyDataLink" },
    { PointerStyle::LinkFile, "LinkFile" },
    { PointerStyle::MoveFileLink, "MoveFileLink" },
    { PointerStyle::CopyFileLink, "CopyFileLink" },
    { PointerStyle::Chart, "Chart" },
    { PointerStyle::Detective, "Detective" },
    { PointerStyle::PivotCol, "PivotCol" },
    { PointerStyle::PivotRow, "PivotRow" },
    { PointerStyle::PivotField, "PivotField" },
    { PointerStyle::PivotDelete, "PivotDelete" },
    { PointerStyle::Chain, "Chain" },
    { PointerStyle::ChainNotAllowed, "ChainNotAllowed" },
    { PointerStyle::AutoScrollN, "AutoScrollN" },
    { PointerStyle::AutoScrollS, "AutoScrollS" },
    { PointerStyle::AutoScrollW, "AutoScrollW" },
    { PointerStyle::AutoScrollE, "AutoScrollE" },
    { PointerStyle::AutoScrollNW, "AutoScrollNW" },
    { PointerStyle::AutoScrollNE, "AutoScrollNE" },
    { PointerStyle::AutoScrollSW, "AutoScrollSW" },
    { PointerStyle::AutoScrollSE, "AutoScrollSE" },
    { PointerStyle::AutoScrollNS, "AutoScrollNS" },
    { PointerStyle::AutoScrollWE, "AutoScrollWE" },
    { PointerStyle::AutoScrollNSWE, "AutoScrollNSWE" },
    { PointerStyle::TextVertical, "TextVertical" },
    { PointerStyle::TabSelectS, "TabSelectS" },
    { PointerStyle::TabSelectE, "TabSelectE" },
    { PointerStyle::TabSelectSE, "TabSelectSE" },
    { PointerStyle::TabSelectW, "TabSelectW" },
    { PointerStyle::TabSelectSW, "TabSelectSW" },
    { PointerStyle::HideWhitespace, "HideWhitespace" },
    { PointerStyle::ShowWhitespace, "ShowWhitespace" },
    { PointerStyle::FatCross, "FatCross" },
};

namespace {

class DemoWidgets : public WorkWindow
{
    VclPtr<MenuBar> mpBar;
    VclPtr<VclBox> mpBox;
    VclPtr<ToolBox> mpToolbox;
    VclPtr<PushButton> mpButton;
    std::vector<VclPtr<VclHBox>> mvCursorBoxes;
    std::vector<VclPtr<PushButton>> mvCursorButtons;

    DECL_LINK(CursorButtonClick, Button*, void);

public:
    DemoWidgets() :
        WorkWindow(nullptr, WB_APP | WB_STDWORK),
        mpBox(VclPtrInstance<VclVBox>(this, false, 3)),
        mpToolbox(VclPtrInstance<ToolBox>(mpBox.get())),
        mpButton(VclPtrInstance<PushButton>(mpBox.get()))
    {
        SetText(u"VCL widget demo"_ustr);

        Wallpaper aWallpaper(BitmapEx(u"sfx2/res/128x128_writer_doc-p.png"_ustr));
        aWallpaper.SetStyle(WallpaperStyle::BottomRight);
        aWallpaper.SetColor(COL_RED);

        mpBox->SetBackground(aWallpaper);
        mpBox->Show();

        Help::EnableBalloonHelp();
        mpToolbox->SetHelpText(u"Help text"_ustr);
        mpToolbox->InsertItem(ToolBoxItemId(0), u"Toolbar item"_ustr, OUString());
        mpToolbox->SetQuickHelpText(ToolBoxItemId(0), u"This is a tooltip popup"_ustr);
        mpToolbox->InsertSeparator();
        mpToolbox->Show();

        mpButton->SetText(u"Click me; go on"_ustr);
        mpButton->Show();

        int i = 0;
        VclHBox* pCurrentCursorHBox = nullptr;
        constexpr int numButtonsPerRow = 9;
        for (auto & rData : gvPointerData)
        {
            if (i % numButtonsPerRow == 0)
            {
                mvCursorBoxes.push_back(VclPtrInstance<VclHBox>(mpBox.get(), true, numButtonsPerRow));
                pCurrentCursorHBox = mvCursorBoxes.back().get();
                pCurrentCursorHBox->Show();
            }
            mvCursorButtons.emplace_back(VclPtrInstance<PushButton>(pCurrentCursorHBox));
            PushButton& rButton = *mvCursorButtons.back();
            rButton.SetText(OUString::createFromAscii(rData.name));
            rButton.SetClickHdl(LINK(this,DemoWidgets,CursorButtonClick));
            rButton.Show();
            ++i;
        }

        mpBar = VclPtr<MenuBar>::Create();
        mpBar->InsertItem(0,u"File"_ustr);
        VclPtrInstance<PopupMenu> pPopup;
        pPopup->InsertItem(0,u"Item"_ustr);
        mpBar->SetPopupMenu(0, pPopup);
        SetMenuBar(mpBar);

        Show();
    }
    virtual ~DemoWidgets() override { disposeOnce(); }
    virtual void dispose() override
    {
        for (auto & p : mvCursorButtons)
            p.disposeAndClear();
        mvCursorButtons.clear();
        for (auto & p : mvCursorBoxes)
            p.disposeAndClear();
        mvCursorBoxes.clear();
        mpToolbox.disposeAndClear();
        mpButton.disposeAndClear();
        mpBox.disposeAndClear();
        mpBar.disposeAndClear();
        WorkWindow::dispose();
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override
    {
        tools::Rectangle aWholeSize(Point(0, 0),GetOutputSizePixel());
        vcl::Region aClip(aWholeSize);
        tools::Rectangle aExclude(tools::Rectangle(Point(50,50),Size(100,100)));
        aClip.Exclude(aExclude);

        Wallpaper aWallpaper(COL_GREEN);

        rRenderContext.Push(vcl::PushFlags::CLIPREGION);
        rRenderContext.IntersectClipRegion(aClip);
        rRenderContext.DrawWallpaper(aWholeSize, aWallpaper);
        rRenderContext.Pop();

        ScopedVclPtrInstance< VirtualDevice > pDev(*GetOutDev());
        pDev->EnableRTL(IsRTLEnabled());
        pDev->SetOutputSizePixel(aExclude.GetSize());

        tools::Rectangle aSubRect(aWholeSize);
        aSubRect.Move(-aExclude.Left(), -aExclude.Top());
        pDev->DrawWallpaper(aSubRect, aWallpaper );

        rRenderContext.DrawOutDev(aExclude.TopLeft(), aExclude.GetSize(),
                   Point( 0, 0 ), aExclude.GetSize(), *pDev );
    }
};

}

IMPL_LINK(DemoWidgets, CursorButtonClick, Button*, pButton, void)
{
    for (size_t i=0; i<std::size(gvPointerData); ++i)
    {
        if (mvCursorButtons[i].get() == pButton)
        {
            mpBox->SetPointer( gvPointerData[i].eStyle );
            return;
        }
    }
    assert(false);
}

namespace {

class DemoPopup : public FloatingWindow
{
 public:
    DemoPopup() : FloatingWindow( nullptr, WB_SYSTEMWINDOW|WB_TOOLTIPWIN)
    {
        SetType( WindowType::HELPTEXTWINDOW );

        SetOutputSizePixel( Size( 300, 30 ) );
        SetBackground(Wallpaper(COL_YELLOW));

        Show( true, ShowFlags::NoActivate );
        PaintImmediately();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override
    {
        Size aSize = GetOutputSizePixel();
        tools::Rectangle aTextRect(Point(6, 6), aSize);

        SetTextColor(COL_BLACK);
        SetTextAlign(ALIGN_TOP);
        rRenderContext.DrawText(aTextRect, u"This is a standalone help text test"_ustr,
                 DrawTextFlags::MultiLine|DrawTextFlags::WordBreak|
                 DrawTextFlags::Left|DrawTextFlags::Top);

        rRenderContext.SetLineColor(COL_BLACK);
        rRenderContext.SetFillColor();
        rRenderContext.DrawRect( tools::Rectangle( Point(), aSize ) );
        aSize.AdjustWidth( -2 );
        aSize.AdjustHeight( -2 );
        Color aColor( rRenderContext.GetLineColor() );
        rRenderContext.SetLineColor( COL_GRAY );
        rRenderContext.DrawRect( tools::Rectangle( Point( 1, 1 ), aSize ) );
        rRenderContext.SetLineColor( aColor );
    }

    virtual void MouseButtonDown( const MouseEvent & ) override
    {
        Application::Quit();
    }
};

}

namespace {
    void renderFonts()
    {
        ScopedVclPtrInstance<VirtualDevice> xDevice;
        Size aSize(1024, 1024);
        xDevice->SetOutputSizePixel(aSize);

#if 0
        for (auto & aFontName : aFontNames)
        {
            vcl::Font aFont(aFontName, Size(0,96));

            aFont.Set(COL_BLACK);
            xDevice->SetFont(aFont);
            xDevice->Erase();

            FontMetric aMetric = xDevice->GetFontMetric(aFont);

            FontCharMapRef xMap;
            if (xDevice->GetFontCharMap(xMap))
            {
                ... iterate through glyphs ...
            }


    bool                        GetGlyphBoundRects( const Point& rOrigin, const OUString& rStr, int nIndex,
                                                    int nLen, int nBase, MetricVector& rVector );

include/vcl/outdev.hxx:typedef std::vector< Rectangle > MetricVector;
include/vcl/outdev.hxx:                                          MetricVector* pVector = nullptr, OUString* pDisplayText = nullptr );
include/vcl/outdev.hxx:                                          MetricVector* pVector = nullptr, OUString* pDisplayText = nullptr,
include/vcl/outdev.hxx:                                              MetricVector* pVector, OUString* pDisplayText, vcl::ITextLayout& _rLayout );
include/vcl/outdev.hxx:                                              DrawTextFlags nStyle = DrawTextFlags::Mnemonic, MetricVector* pVector = nullp

    bool                        GetTextBoundRect( Rectangle& rRect,
                                                  const OUString& rStr, sal_Int32 nBase = 0, sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                                  sal_uLong nLayoutWidth = 0, const long* pDXArray = nullptr ) const;


    void                        DrawText( const Point& rStartPt, const OUString& rStr,
                                          sal_Int32 nIndex = 0, sal_Int32 nLen = -1,
                                          MetricVector* pVector = nullptr, OUString* pDisplayText = nullptr );

    void                        DrawText( const Rectangle& rRect,
                                          const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::NONE,
                                          MetricVector* pVector = nullptr, OUString* pDisplayText = nullptr,
                                          vcl::ITextLayout* _pTextLayout = nullptr );

    Rectangle                   GetTextRect( const Rectangle& rRect,
                                             const OUString& rStr, DrawTextFlags nStyle = DrawTextFlags::WordBreak,
                                             TextRectInfo* pInfo = nullptr,
                                             const vcl::ITextLayout* _pTextLayout = nullptr ) const;

        }
#endif

    }
};

namespace {

class DemoApp : public Application
{
    static int showHelp(DemoRenderer &rRenderer)
    {
        fprintf(stderr,"vcldemo - a VCL test app\n");
        fprintf(stderr,"  --help             - print this text\n");
        fprintf(stderr,"  --show <renderer>  - start with a given renderer, options are:\n");
        OUString aRenderers(rRenderer.getRendererList());
        fprintf(stderr,"         %s\n",
                OUStringToOString(aRenderers, RTL_TEXTENCODING_UTF8).getStr());
        fprintf(stderr,"  --test <iterCount> - create benchmark data\n");
        fprintf(stderr,"  --widgets          - launch the widget test.\n");
        fprintf(stderr,"  --popup            - launch the popup test.\n");
        fprintf(stderr,"  --threads          - render from multiple threads.\n");
        fprintf(stderr,"  --font <fontname>  - run the font render test.\n");
        fprintf(stderr, "\n");
        return 0;
    }

public:
    DemoApp() {}

    virtual int Main() override
    {
        try
        {
            bool bWidgets = false;
            bool bThreads = false;
            bool bPopup = false;
            DemoRenderer aRenderer;
            std::vector<OUString> aFontNames;

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
                else if (aArg == "--threads")
                    bThreads = true;
                else if (aArg == "--font" && !bLast)
                    aFontNames.push_back(GetCommandLineParam(++i));
                else if (aArg.startsWith("--"))
                {
                    fprintf(stderr,"Unknown argument '%s'\n",
                            OUStringToOString(aArg, RTL_TEXTENCODING_UTF8).getStr());
                    return showHelp(aRenderer);
                }
            }

            ScopedVclPtrInstance<DemoWin> aMainWin(aRenderer, bThreads);
            VclPtr<DemoWidgets> xWidgets;
            VclPtr<DemoPopup> xPopup;

            aMainWin->SetText(u"Interactive VCL demo #1"_ustr);
            if (bWidgets)
                xWidgets = VclPtr< DemoWidgets >::Create ();
            else if (bPopup)
                xPopup = VclPtrInstance< DemoPopup> ();
            else if (!aFontNames.empty())
                renderFonts();
            else
                aMainWin->Show();

            Application::Execute();

            xWidgets.disposeAndClear();
            xPopup.disposeAndClear();
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("vcl.app", "Fatal");
            return 1;
        }
        catch (const std::exception& e)
        {
            SAL_WARN("vcl.app", "Fatal: " << e.what());
            return 1;
        }
        return 0;
    }

protected:
    void Init() override
    {
        LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());

        try
        {
            uno::Reference<uno::XComponentContext> xComponentContext
                = ::cppu::defaultBootstrap_InitialComponentContext();
            uno::Reference<lang::XMultiServiceFactory> xMSF;
            xMSF.set(xComponentContext->getServiceManager(), uno::UNO_QUERY);
            if(!xMSF.is())
                Application::Abort(u"Bootstrap failure - no service manager"_ustr);

            ::comphelper::setProcessServiceFactory(xMSF);
        }
        catch (const uno::Exception &e)
        {
            Application::Abort("Bootstrap exception " + e.Message);
        }
    }
    void DeInit() override
    {
        framework::getDesktop(::comphelper::getProcessComponentContext())->terminate();
        framework::getDesktop(::comphelper::getProcessComponentContext())->disposing();

        uno::Reference< lang::XComponent >(
            comphelper::getProcessComponentContext(),
        uno::UNO_QUERY_THROW)-> dispose();
        ::comphelper::setProcessServiceFactory(nullptr);
    }
};

}

void vclmain::createApplication()
{
#ifdef _WIN32
    _putenv_s("LIBO_VCL_DEMO", "1");
#else
    setenv("LIBO_VCL_DEMO", "1", 0);
#endif
    static DemoApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
