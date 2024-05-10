/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <osl/process.h>
#include <test/bootstrapfixture.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>

#include <vcl/BitmapReadAccess.hxx>
#include <comphelper/errcode.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>

#include <ImplLayoutArgs.hxx>
#include <TextLayoutCache.hxx>
#include <salgdi.hxx>

class VclTextTest : public test::BootstrapFixture
{
    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/vcl_text_test.test.core/"
    static constexpr const bool mbExportBitmap = false;

public:
    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        if (mbExportBitmap)
        {
            BitmapEx aBitmapEx(device->GetBitmapEx(Point(0, 0), device->GetOutputSizePixel()));
            OUString cwd;
            CPPUNIT_ASSERT_EQUAL(osl_Process_E_None, osl_getProcessWorkingDir(&cwd.pData));
            OUString url;
            CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                                 osl::FileBase::getAbsoluteFileURL(cwd, filename, url));
            SvFileStream aStream(url, StreamMode::WRITE | StreamMode::TRUNC);
            CPPUNIT_ASSERT_EQUAL(
                ERRCODE_NONE, GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream));
        }
    }

    VclTextTest()
        : BootstrapFixture(true, false)
    {
    }
};

// Avoid issues when colorized antialiasing generates slightly tinted rather than truly black
// pixels:
static bool isBlack(Color col)
{
    return col.GetRed() < 25 && col.GetGreen() < 25 && col.GetBlue() < 25;
}

// Return pixel width of the base of the given character located above
// the starting position.
// In other words, go up in y direction until a black pixel is found,
// then return the horizontal width of the area of those pixels.
// For 'L' this gives the width of the base of the character.
static tools::Long getCharacterBaseWidth(VirtualDevice* device, const Point& start)
{
    Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
    BitmapScopedReadAccess access(bitmap);
    tools::Long y = start.Y();
    while (y >= 0 && !isBlack(access->GetColor(y, start.X())))
        --y;
    if (y < 0)
        return -1;
    tools::Long xmin = start.X();
    while (xmin >= 0 && access->GetColor(y, xmin) != COL_WHITE)
        --xmin;
    tools::Long xmax = start.X();
    while (xmax < bitmap.GetSizePixel().Width() && access->GetColor(y, xmax) != COL_WHITE)
        ++xmax;
    return xmax - xmin + 1;
}

// Similar to above but this time from the top, for U+30E8 (it's straight at the top, not at the bottom).
static tools::Long getCharacterTopWidth(VirtualDevice* device, const Point& start)
{
    Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
    BitmapScopedReadAccess access(bitmap);
    tools::Long y = start.Y();
    while (y < bitmap.GetSizePixel().Height() && !isBlack(access->GetColor(y, start.X())))
        ++y;
    if (y >= bitmap.GetSizePixel().Height())
        return -1;
    tools::Long xmin = start.X();
    while (xmin >= 0 && access->GetColor(y, xmin) != COL_WHITE)
        --xmin;
    tools::Long xmax = start.X();
    while (xmax < bitmap.GetSizePixel().Width() && access->GetColor(y, xmax) != COL_WHITE)
        ++xmax;
    return xmax - xmin + 1;
}

// Similar to above, but this time return the pixel height of the left-most
// line of the character, going right from the starting point.
// For 'L' this gives the height of the left line.
static tools::Long getCharacterLeftSideHeight(VirtualDevice* device, const Point& start)
{
    Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
    BitmapScopedReadAccess access(bitmap);
    tools::Long x = start.X();
    while (x < bitmap.GetSizePixel().Width() && !isBlack(access->GetColor(start.Y(), x)))
        ++x;
    if (x >= bitmap.GetSizePixel().Width())
        return -1;
    tools::Long ymin = start.Y();
    while (ymin >= 0 && access->GetColor(ymin, x) != COL_WHITE)
        --ymin;
    tools::Long ymax = start.Y();
    while (ymax < bitmap.GetSizePixel().Width() && access->GetColor(ymax, x) != COL_WHITE)
        ++ymax;
    return ymax - ymin + 1;
}

// Test rendering of the 'L' character (chosen because L is a simple shape).
// Check things like using a double font size doubling the size of the character, correct rotation, etc.
// IMPORTANT: If you modify this, check also the VclCjkTextTest::testVerticalText().
CPPUNIT_TEST_FIXTURE(VclTextTest, testSimpleText)
{
    OUString text(u"L"_ustr);
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(100, 100));
    device->SetBackground(Wallpaper(COL_WHITE));
    // Disable AA, to make all pixels be black or white.
    device->SetAntialiasing(AntialiasingFlags::DisableText);

    // Bail out on all backends that do not work (or I didn't test). Opt-out rather than opt-in
    // to make sure new backends fail initially.
    if (device->GetGraphics()->getRenderBackendName() == "qt5"
        || device->GetGraphics()->getRenderBackendName() == "qt5svp"
        || device->GetGraphics()->getRenderBackendName() == "gtk3svp"
        || device->GetGraphics()->getRenderBackendName() == "aqua"
        || device->GetGraphics()->getRenderBackendName() == "gen"
        || device->GetGraphics()->getRenderBackendName() == "genpsp")
        return;

    // Use Dejavu fonts, they are shipped with LO, so they should be ~always available.
    // Use Sans variant for simpler glyph shapes (no serifs).
    vcl::Font font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 36));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice(u"simple-text-36.png"_ustr, device);
    // Height of 'L' with font 36 size should be roughly 28 pixels.
    // Use the 'doubles' variant of the test, since that one allows
    // a delta, and allow several pixels of delta to account
    // for different rendering methods and whatnot.
    tools::Long height36 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(28), height36, 4);
    tools::Long width36 = getCharacterBaseWidth(device, Point(20, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(19), width36, 4);

    font.SetOrientation(2700_deg10);
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice(u"simple-text-36-270deg.png"_ustr, device);
    // Width and height here should be swapped, again allowing for some imprecisions.
    tools::Long height36Rotated = getCharacterLeftSideHeight(device, Point(0, 20));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, height36Rotated, 2);
    tools::Long width36Rotated = getCharacterTopWidth(device, Point(70, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, width36Rotated, 2);

    font = vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 72));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice(u"simple-text-72.png"_ustr, device);
    // Font size is doubled, so pixel sizes should also roughly double.
    tools::Long height72 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 * 2, height72, 4);
    tools::Long width72 = getCharacterBaseWidth(device, Point(20, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 * 2, width72, 5);

    font.SetOrientation(2700_deg10);
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice(u"simple-text-72-270deg.png"_ustr, device);
    tools::Long height72Rotated = getCharacterLeftSideHeight(device, Point(0, 35));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width72, height72Rotated, 2);
    tools::Long width72Rotated = getCharacterTopWidth(device, Point(50, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height72, width72Rotated, 2);

    // Test width scaled to 200%.
    font = vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(72, 36));
#ifdef _WIN32
    // TODO: What is the proper way to draw 200%-wide text? This is needed on Windows
    // but it breaks Linux.
    font.SetAverageFontWidth(2 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice(u"simple-text-36-200pct.png"_ustr, device);
    tools::Long height36pct200 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, height36pct200, 2);
    tools::Long width36pct200 = getCharacterBaseWidth(device, Point(20, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 * 2, width36pct200, 5);

    // Test width scaled to 50%.
    font = vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(18, 36));
#ifdef _WIN32
    font.SetAverageFontWidth(0.5 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice(u"simple-text-36-50pct.png"_ustr, device);
    tools::Long height36pct50 = getCharacterLeftSideHeight(device, Point(0, 40));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, height36pct50, 2);
    tools::Long width36pct50 = getCharacterBaseWidth(device, Point(15, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 / 2, width36pct50, 2);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testSimpleTextFontSpecificKerning)
{
    OUString aAV(u"AV"_ustr);

    vcl::Font aFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 2048));

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // absolute character widths AKA text array.
    tools::Long nRefTextWidth = 2671;
    std::vector<sal_Int32> aRefCharWidths = { 1270, 2671 };
    KernArray aCharWidths;
    tools::Long nTextWidth = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aAV, &aCharWidths));

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths[0], aCharWidths.get_subunit_array()[0]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths[1], aCharWidths.get_subunit_array()[1]);
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, pOutDev->GetTextWidth(aAV));
    CPPUNIT_ASSERT_EQUAL(tools::Long(2384), pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
    tools::Rectangle aBoundRect;
    pOutDev->GetTextBoundRect(aBoundRect, aAV);
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aBoundRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(408), aBoundRect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2639), aBoundRect.GetWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1493), aBoundRect.getOpenHeight());

    // normal orientation
    tools::Rectangle aInput;
    tools::Rectangle aRect = pOutDev->GetTextRect(aInput, aAV);

    // now rotate 270 degrees
    vcl::Font aRotated(aFont);
    aRotated.SetOrientation(2700_deg10);
    pOutDev->SetFont(aRotated);
    tools::Rectangle aRectRot = pOutDev->GetTextRect(aInput, aAV);

    // Check that we did do the rotation...
    CPPUNIT_ASSERT_EQUAL(aRectRot.GetWidth(), aRect.GetHeight());
    CPPUNIT_ASSERT_EQUAL(aRectRot.GetHeight(), aRect.GetWidth());
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testSimpleTextNoKerning)
{
    OUString aAV(u"AV"_ustr);

    vcl::Font aFont(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 2048));
    aFont.SetKerning(FontKerning::NONE);

    ScopedVclPtrInstance<VirtualDevice> pOutDev;
    pOutDev->SetFont(aFont);

    // absolute character widths AKA text array.
    tools::Long nRefTextWidth = 2802;
    std::vector<sal_Int32> aRefCharWidths = { 1401, 2802 };
    KernArray aCharWidths;
    tools::Long nTextWidth = basegfx::fround<tools::Long>(pOutDev->GetTextArray(aAV, &aCharWidths));

    CPPUNIT_ASSERT_EQUAL(aRefCharWidths[0], aCharWidths.get_subunit_array()[0]);
    CPPUNIT_ASSERT_EQUAL(aRefCharWidths[1], aCharWidths.get_subunit_array()[1]);
    // this sporadically returns 75 or 74 on some of the windows tinderboxes eg. tb73
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, nTextWidth);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(nTextWidth), aCharWidths.back());

    // text advance width and line height
    CPPUNIT_ASSERT_EQUAL(nRefTextWidth, pOutDev->GetTextWidth(aAV));
    CPPUNIT_ASSERT_EQUAL(tools::Long(2384), pOutDev->GetTextHeight());

    // exact bounding rectangle, not essentially the same as text width/height
    tools::Rectangle aBoundRect;
    pOutDev->GetTextBoundRect(aBoundRect, aAV);
    CPPUNIT_ASSERT_EQUAL(tools::Long(16), aBoundRect.Left());
    CPPUNIT_ASSERT_EQUAL(tools::Long(408), aBoundRect.Top());
    CPPUNIT_ASSERT_EQUAL(tools::Long(2770), aBoundRect.GetWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1493), aBoundRect.getOpenHeight());

    // normal orientation
    tools::Rectangle aInput;
    tools::Rectangle aRect = pOutDev->GetTextRect(aInput, aAV);

    // now rotate 270 degrees
    vcl::Font aRotated(aFont);
    aRotated.SetOrientation(2700_deg10);
    pOutDev->SetFont(aRotated);
    tools::Rectangle aRectRot = pOutDev->GetTextRect(aInput, aAV);

    // Check that we did do the rotation...
    CPPUNIT_ASSERT_EQUAL(aRectRot.GetWidth(), aRect.GetHeight());
    CPPUNIT_ASSERT_EQUAL(aRectRot.GetHeight(), aRect.GetWidth());
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testTextLayoutCache)
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dogالعاشر"_ustr;
    vcl::text::TextLayoutCache cache(sTestString.getStr(), sTestString.getLength());

    vcl::text::Run run1 = cache.runs[0];
    vcl::text::Run run2 = cache.runs[1];

    bool bCorrectRuns = (cache.runs.size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of runs", bCorrectRuns);
    CPPUNIT_ASSERT_EQUAL(USCRIPT_LATIN, run1.nCode);
    CPPUNIT_ASSERT_EQUAL(0, run1.nStart);
    CPPUNIT_ASSERT_EQUAL(45, run1.nEnd);
    CPPUNIT_ASSERT_EQUAL(USCRIPT_ARABIC, run2.nCode);
    CPPUNIT_ASSERT_EQUAL(45, run2.nStart);
    CPPUNIT_ASSERT_EQUAL(51, run2.nEnd);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutRuns_AddPos)
{
    ImplLayoutRuns aRuns;
    aRuns.AddPos(1, false);
    aRuns.AddPos(2, false);
    aRuns.AddPos(3, false);
    aRuns.AddPos(4, true); // add RTL marker glyph
    aRuns.AddPos(5, false);
    aRuns.AddPos(6, true); // add RTL marker glyph
    aRuns.AddPos(7, false);

    int nCharPos(0);
    bool bRightToLeftMarker(false);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(1, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(2, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(3, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(4, nCharPos);
    CPPUNIT_ASSERT(bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(5, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(6, nCharPos);
    CPPUNIT_ASSERT(bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(7, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    // no next position, we are running off the end
    CPPUNIT_ASSERT(!aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));

    aRuns.ResetPos();

    int nMinRunPos, nEndRunPos;
    bool bRightToLeft(false);

    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(1, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(4, nEndRunPos);
    CPPUNIT_ASSERT(!bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(4, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(5, nEndRunPos);
    CPPUNIT_ASSERT(bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(5, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(6, nEndRunPos);
    CPPUNIT_ASSERT(!bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(6, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(7, nEndRunPos);
    CPPUNIT_ASSERT(bRightToLeft);

    // test clear
    aRuns.Clear();
    CPPUNIT_ASSERT(aRuns.IsEmpty());
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutRuns_AddRuns)
{
    ImplLayoutRuns aRuns;
    aRuns.AddRun(1, 4, false);
    aRuns.AddRun(5, 4, true);
    aRuns.AddRun(5, 6, false);
    aRuns.AddRun(6, 7, true);

    int nCharPos(0);
    bool bRightToLeftMarker(false);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(1, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(2, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(3, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(4, nCharPos);
    CPPUNIT_ASSERT(bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(5, nCharPos);
    CPPUNIT_ASSERT(!bRightToLeftMarker);

    CPPUNIT_ASSERT(aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));
    CPPUNIT_ASSERT_EQUAL(6, nCharPos);
    CPPUNIT_ASSERT(bRightToLeftMarker);

    // no next position, we are running off the end
    CPPUNIT_ASSERT(!aRuns.GetNextPos(&nCharPos, &bRightToLeftMarker));

    aRuns.ResetPos();

    int nMinRunPos, nEndRunPos;
    bool bRightToLeft(false);

    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(1, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(4, nEndRunPos);
    CPPUNIT_ASSERT(!bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(4, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(5, nEndRunPos);
    CPPUNIT_ASSERT(bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(5, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(6, nEndRunPos);
    CPPUNIT_ASSERT(!bRightToLeft);

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.GetRun(&nMinRunPos, &nEndRunPos, &bRightToLeft));
    CPPUNIT_ASSERT_EQUAL(6, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(7, nEndRunPos);
    CPPUNIT_ASSERT(bRightToLeft);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutRuns_PosIsInRun)
{
    ImplLayoutRuns aRuns;
    aRuns.AddRun(1, 4, false);
    aRuns.AddRun(4, 5, true);
    aRuns.AddRun(5, 6, false);
    aRuns.AddRun(6, 7, true);

    CPPUNIT_ASSERT(aRuns.PosIsInRun(1));
    CPPUNIT_ASSERT(aRuns.PosIsInRun(2));
    CPPUNIT_ASSERT(aRuns.PosIsInRun(3));

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.PosIsInRun(4));

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.PosIsInRun(5));

    aRuns.NextRun();
    CPPUNIT_ASSERT(aRuns.PosIsInRun(6));

    CPPUNIT_ASSERT(!aRuns.PosIsInRun(7));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutRuns_PosIsInAnyRun)
{
    ImplLayoutRuns aRuns;
    aRuns.AddRun(1, 4, false);
    aRuns.AddRun(4, 5, true);
    aRuns.AddRun(5, 6, false);
    aRuns.AddRun(6, 7, true);

    CPPUNIT_ASSERT(aRuns.PosIsInAnyRun(1));
    CPPUNIT_ASSERT(!aRuns.PosIsInAnyRun(7));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutArgsBiDiStrong)
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر"_ustr;
    vcl::text::ImplLayoutArgs aArgs(sTestString, 0, sTestString.getLength(),
                                    SalLayoutFlags::BiDiStrong, LanguageTag(LANGUAGE_NONE),
                                    nullptr);

    int nMinRunPos(0);
    int nEndRunPos(0);
    bool bRTL(false);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(0, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(19, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(20, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(51, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(20, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(51, nEndRunPos);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutArgsBiDiRtl)
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر"_ustr;
    vcl::text::ImplLayoutArgs aArgs(sTestString, 0, sTestString.getLength(),
                                    SalLayoutFlags::BiDiRtl, LanguageTag(LANGUAGE_NONE), nullptr);

    int nMinRunPos(0);
    int nEndRunPos(0);
    bool bRTL(false);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(45, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(51, nEndRunPos);
    CPPUNIT_ASSERT(&bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(21, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(45, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(20, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(21, nEndRunPos);
    CPPUNIT_ASSERT(bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(0, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(19, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutArgsRightAlign)
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر"_ustr;
    vcl::text::ImplLayoutArgs aArgs(sTestString, 0, sTestString.getLength(),
                                    SalLayoutFlags::RightAlign, LanguageTag(LANGUAGE_NONE),
                                    nullptr);

    int nMinRunPos(0);
    int nEndRunPos(0);
    bool bRTL(false);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(0, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(19, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(20, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(45, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(45, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(51, nEndRunPos);
    CPPUNIT_ASSERT(bRTL);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testImplLayoutArgs_PrepareFallback_precalculatedglyphs)
{
    // this font has no Cyrillic characters and thus needs fallback
    const vcl::Font aFont(u"Amiri"_ustr, Size(0, 36));

    ScopedVclPtrInstance<VirtualDevice> pVirDev;
    pVirDev->SetFont(aFont);

    static constexpr OStringLiteral sUTF8String(u8"Тхе яуицк\n ыумпед овер");
    const OUString sTestString(OUString::fromUtf8(sUTF8String));
    std::unique_ptr<SalLayout> pLayout
        = pVirDev->ImplLayout(sTestString, 0, sTestString.getLength(), Point(0, 0), 0, {}, {},
                              SalLayoutFlags::GlyphItemsOnly);
    SalLayoutGlyphs aGlyphs = pLayout->GetGlyphs();
    SalLayoutGlyphsImpl* pGlyphsImpl = aGlyphs.Impl(1);

    vcl::text::ImplLayoutArgs aArgs(sTestString, 0, sTestString.getLength(),
                                    SalLayoutFlags::BiDiRtl, LanguageTag(LANGUAGE_RUSSIAN),
                                    nullptr);

    aArgs.PrepareFallback(pGlyphsImpl);

    int nMinRunPos(0);
    int nEndRunPos(0);
    bool bRTL(false);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(0, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(3, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(4, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(9, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(11, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(17, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);

    aArgs.GetNextRun(&nMinRunPos, &nEndRunPos, &bRTL);
    CPPUNIT_ASSERT_EQUAL(18, nMinRunPos);
    CPPUNIT_ASSERT_EQUAL(22, nEndRunPos);
    CPPUNIT_ASSERT(!bRTL);
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetStringWithCenterEllpsis)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(
        u"a b c d ...v w x y z"_ustr,
        device->GetEllipsisString(u"a b c d e f g h i j k l m n o p q r s t u v w x y z"_ustr, 100,
                                  DrawTextFlags::CenterEllipsis));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetStringWithEndEllpsis)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, device->GetEllipsisString(u"abcde. f g h i j ..."_ustr, 10,
                                                              DrawTextFlags::EndEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a b c d e f g h i j ..."_ustr,
        device->GetEllipsisString(u"a b c d e f g h i j k l m n o p q r s t u v w x y z"_ustr, 100,
                                  DrawTextFlags::EndEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a"_ustr, device->GetEllipsisString(u"abcde. f g h i j ..."_ustr, 1,
                                             DrawTextFlags::EndEllipsis | DrawTextFlags::Clip));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetStringWithNewsEllpsis)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, device->GetEllipsisString(u"abcde. f g h i j ..."_ustr, 10,
                                                              DrawTextFlags::NewsEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a b .... x y z"_ustr,
        device->GetEllipsisString(u"a b c d. e f g. h i j k l m n o p q r s t u v w. x y z"_ustr,
                                  100, DrawTextFlags::NewsEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a b .... x y z"_ustr,
        device->GetEllipsisString(u"a b c d. e f g h i j k l m n o p q r s t u v w. x y z"_ustr,
                                  100, DrawTextFlags::NewsEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a b c d e f g h i j ..."_ustr,
        device->GetEllipsisString(u"a b c d e f g h i j k l m n o p q r s t u v w. x y z"_ustr, 100,
                                  DrawTextFlags::NewsEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"a..... x y z"_ustr,
        device->GetEllipsisString(u"a. b c d e f g h i j k l m n o p q r s t u v w. x y z"_ustr,
                                  100, DrawTextFlags::NewsEllipsis));

    CPPUNIT_ASSERT_EQUAL(
        u"ab. cde..."_ustr,
        device->GetEllipsisString(u"ab. cde. x y z"_ustr, 50, DrawTextFlags::NewsEllipsis));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetTextBreak_invalid_index)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    const OUString sTestStr(u"textline_ text_"_ustr);
    const auto nLen = sTestStr.getLength();
    const auto nTextWidth = device->GetTextWidth(u"text"_ustr);
    const auto nInvalidIndex = sTestStr.getLength() + 2;

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         device->GetTextBreak(sTestStr, nTextWidth, nInvalidIndex, nLen));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetTextBreak)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"DejaVu Sans"_ustr, u"Book"_ustr, Size(0, 11)));

    const OUString sTestStr(u"textline_ text_"_ustr);
    const auto nLen = sTestStr.getLength();
    const auto nTextWidth = device->GetTextWidth(u"text"_ustr);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4),
                         device->GetTextBreak(sTestStr, nTextWidth, 0, nLen));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(7),
                         device->GetTextBreak(sTestStr, nTextWidth, 3, nLen));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9),
                         device->GetTextBreak(sTestStr, nTextWidth, 6, nLen));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(12),
                         device->GetTextBreak(sTestStr, nTextWidth, 8, nLen));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14),
                         device->GetTextBreak(sTestStr, nTextWidth, 11, nLen));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         device->GetTextBreak(sTestStr, nTextWidth, 13, nLen));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetSingleLineTextRect)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(
        tools::Rectangle(Point(), Size(75, 12)),
        device->GetTextRect(tools::Rectangle(Point(), Point(100, 100)), u"This is test text"_ustr));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetSingleLineTextRectWithEndEllipsis)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(
        tools::Rectangle(Point(), Size(52, 12)),
        device->GetTextRect(tools::Rectangle(Point(), Point(50, 50)), u"This is test text"_ustr,
                            DrawTextFlags::WordBreak | DrawTextFlags::EndEllipsis));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetRightBottomAlignedSingleLineTextRect)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(926, 989), Size(75, 12)),
                         device->GetTextRect(tools::Rectangle(Point(), Point(1000, 1000)),
                                             u"This is test text"_ustr,
                                             DrawTextFlags::Right | DrawTextFlags::Bottom));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetRotatedSingleLineTextRect)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    vcl::Font aFont(device->GetFont());
    aFont.SetOrientation(45_deg10);
    device->SetFont(aFont);

    CPPUNIT_ASSERT_EQUAL(
        tools::Rectangle(Point(0, -3), Size(75, 18)),
        device->GetTextRect(tools::Rectangle(Point(), Point(100, 100)), u"This is test text"_ustr));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetMultiLineTextRect)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(), Size(75, 12)),
                         device->GetTextRect(tools::Rectangle(Point(), Point(100, 100)),
                                             u"This is test text"_ustr,
                                             DrawTextFlags::WordBreak | DrawTextFlags::MultiLine));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetMultiLineTextRectWithEndEllipsis)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(), Size(52, 48)),
                         device->GetTextRect(tools::Rectangle(Point(), Point(50, 50)),
                                             u"This is test text xyzabc123abcdefghijk"_ustr,
                                             DrawTextFlags::WordBreak | DrawTextFlags::EndEllipsis
                                                 | DrawTextFlags::MultiLine));
}

CPPUNIT_TEST_FIXTURE(VclTextTest, testGetRightBottomAlignedMultiLineTextRect)
{
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    device->SetOutputSizePixel(Size(1000, 1000));
    device->SetFont(vcl::Font(u"Liberation Sans"_ustr, Size(0, 11)));

    CPPUNIT_ASSERT_EQUAL(tools::Rectangle(Point(926, 989), Size(75, 12)),
                         device->GetTextRect(tools::Rectangle(Point(), Point(1000, 1000)),
                                             u"This is test text"_ustr,
                                             DrawTextFlags::Right | DrawTextFlags::Bottom
                                                 | DrawTextFlags::MultiLine));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
