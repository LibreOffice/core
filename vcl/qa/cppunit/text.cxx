/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>

#include <vcl/BitmapReadAccess.hxx>
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

    void exportDevice(const OUString& filename, const VclPtr<VirtualDevice>& device)
    {
        if (mbExportBitmap)
        {
            BitmapEx aBitmapEx(device->GetBitmapEx(Point(0, 0), device->GetOutputSizePixel()));
            SvFileStream aStream(filename, StreamMode::WRITE | StreamMode::TRUNC);
            GraphicFilter::GetGraphicFilter().compressAsPNG(aBitmapEx, aStream);
        }
    }

public:
    VclTextTest()
        : BootstrapFixture(true, false)
    {
    }

    void testSimpleText();
    void testVerticalText();
    void testTextLayoutCache();
    void testImplLayoutRuns_AddPos();
    void testImplLayoutRuns_AddRuns();
    void testImplLayoutRuns_PosIsInRun();
    void testImplLayoutRuns_PosIsInAnyRun();
    void testImplLayoutArgsBiDiStrong();
    void testImplLayoutArgsBiDiRtl();
    void testImplLayoutArgsRightAlign();
    void testImplLayoutArgs_PrepareFallback_precalculatedglyphs();

    CPPUNIT_TEST_SUITE(VclTextTest);
    CPPUNIT_TEST(testSimpleText);
    CPPUNIT_TEST(testVerticalText);
    CPPUNIT_TEST(testTextLayoutCache);
    CPPUNIT_TEST(testImplLayoutRuns_AddPos);
    CPPUNIT_TEST(testImplLayoutRuns_AddRuns);
    CPPUNIT_TEST(testImplLayoutRuns_PosIsInRun);
    CPPUNIT_TEST(testImplLayoutRuns_PosIsInAnyRun);
    CPPUNIT_TEST(testImplLayoutArgsBiDiStrong);
    CPPUNIT_TEST(testImplLayoutArgsBiDiRtl);
    CPPUNIT_TEST(testImplLayoutArgsRightAlign);
    CPPUNIT_TEST(testImplLayoutArgs_PrepareFallback_precalculatedglyphs);
    CPPUNIT_TEST_SUITE_END();
};

// Return pixel width of the base of the given character located above
// the starting position.
// In other words, go up in y direction until a black pixel is found,
// then return the horizontal width of the area of those pixels.
// For 'L' this gives the width of the base of the character.
static tools::Long getCharacterBaseWidth(VirtualDevice* device, const Point& start)
{
    Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
    Bitmap::ScopedReadAccess access(bitmap);
    tools::Long y = start.Y();
    while (y >= 0 && access->GetColor(y, start.X()) != COL_BLACK)
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
    Bitmap::ScopedReadAccess access(bitmap);
    tools::Long y = start.Y();
    while (y < bitmap.GetSizePixel().Height() && access->GetColor(y, start.X()) != COL_BLACK)
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
    Bitmap::ScopedReadAccess access(bitmap);
    tools::Long x = start.X();
    while (x < bitmap.GetSizePixel().Width() && access->GetColor(start.Y(), x) != COL_BLACK)
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

// The same, but from the right side, for U+30E8 (it's straight on the right side, not the left one).
static tools::Long getCharacterRightSideHeight(VirtualDevice* device, const Point& start)
{
    Bitmap bitmap = device->GetBitmap(Point(), device->GetOutputSizePixel());
    Bitmap::ScopedReadAccess access(bitmap);
    tools::Long x = start.X();
    while (x >= 0 && access->GetColor(start.Y(), x) != COL_BLACK)
        --x;
    if (x < 0)
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
// IMPORTANT: If you modify this, check also the testVerticalText().
void VclTextTest::testSimpleText()
{
    OUString text("L");
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
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
    vcl::Font font("DejaVu Sans", "Book", Size(0, 36));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("simple-text-36.png", device);
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
    exportDevice("simple-text-36-270deg.png", device);
    // Width and height here should be swapped, again allowing for some imprecisions.
    tools::Long height36Rotated = getCharacterLeftSideHeight(device, Point(0, 20));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, height36Rotated, 2);
    tools::Long width36Rotated = getCharacterTopWidth(device, Point(70, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, width36Rotated, 2);

    font = vcl::Font("DejaVu Sans", "Book", Size(0, 72));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("simple-text-72.png", device);
    // Font size is doubled, so pixel sizes should also roughly double.
    tools::Long height72 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 * 2, height72, 4);
    tools::Long width72 = getCharacterBaseWidth(device, Point(20, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 * 2, width72, 4);

    font.SetOrientation(2700_deg10);
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice("simple-text-72-270deg.png", device);
    tools::Long height72Rotated = getCharacterLeftSideHeight(device, Point(0, 35));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width72, height72Rotated, 2);
    tools::Long width72Rotated = getCharacterTopWidth(device, Point(50, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height72, width72Rotated, 2);

    // Test width scaled to 200%.
    font = vcl::Font("DejaVu Sans", "Book", Size(72, 36));
#ifdef _WIN32
    // TODO: What is the proper way to draw 200%-wide text? This is needed on Windows
    // but it breaks Linux.
    font.SetAverageFontWidth(2 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("simple-text-36-200pct.png", device);
    tools::Long height36pct200 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, height36pct200, 2);
    tools::Long width36pct200 = getCharacterBaseWidth(device, Point(20, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 * 2, width36pct200, 4);

    // Test width scaled to 50%.
    font = vcl::Font("DejaVu Sans", "Book", Size(18, 36));
#ifdef _WIN32
    font.SetAverageFontWidth(0.5 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("simple-text-36-50pct.png", device);
    tools::Long height36pct50 = getCharacterLeftSideHeight(device, Point(0, 30));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, height36pct50, 2);
    tools::Long width36pct50 = getCharacterBaseWidth(device, Point(15, 99));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 / 2, width36pct50, 2);
}

// Like testSimpleText() but for a vertical character, here namely U+30E8 (katakana letter yo),
// chosen because it's a fairly simple shape (looks like horizontally mirrored E) that should
// have the right and top lines being straight. Well, and also chosen because I actually
// do not have much clue about CJK.
// IMPORTANT: If you modify this, modify also the testSimpleText().
void VclTextTest::testVerticalText()
{
    OUString text(u"\x30e8");
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
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
#ifdef MACOSX // vertical fonts are broken on Mac with or without Skia
        || device->GetGraphics()->getRenderBackendName() == "skia"
#endif
        || device->GetGraphics()->getRenderBackendName() == "genpsp")
        return;

    // We do not ship any CJK fonts, so try to find a common one that is usable for the test.
    vcl::Font baseFont;
    vcl::Font font;
    bool fontFound = false;
    for (const char* ptrfontName :
         { "Droid Sans Japanese", "Baekmuk Gulim", "Microsoft JhengHei", "Microsoft YaHei",
           "MS PGothic", "Hiragino Sans", "Arial Unicode MS" })
    {
        OUString fontName = OUString::fromUtf8(ptrfontName);
        if (!device->IsFontAvailable(fontName))
            continue;
        baseFont = vcl::Font(fontName, "Book", Size(0, 36));
        baseFont.SetLanguage(LANGUAGE_JAPANESE);
        baseFont.SetVertical(true);
        baseFont.SetOrientation(2700_deg10);
        if (device->HasGlyphs(baseFont, text) == -1) // -1 means no glyph is missing
        {
            fontFound = true;
            break;
        }
    }
    if (!fontFound)
    {
        SAL_WARN("vcl", "Could not find a font for VclTextTest::testVerticalText, skipping test.");
        return;
    }

    font = baseFont;
    font.SetFontSize(Size(0, 36));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice("vertical-text-36.png", device);
    // Height of U+30E8 with font 36 size should be roughly 28 pixels,
    // but since we don't know which font will be used, allow even more range.
    tools::Long height36 = getCharacterRightSideHeight(device, Point(99, 20));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(28), height36, 6);
    tools::Long width36 = getCharacterTopWidth(device, Point(65, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(25), width36, 6);

    // Horizontal writing of vertical glyphs. For some reason in this case
    // the font is not set to be vertical.
    font.SetOrientation(0_deg10);
    font.SetVertical(false);
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("vertical-text-36-0deg.png", device);
    // Here width and height should be the same, since the glyphs actually
    // not rotated compared to the vertical writing.
    tools::Long height36Rotated = getCharacterRightSideHeight(device, Point(99, 35));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36, height36Rotated, 1);
    tools::Long width36Rotated = getCharacterTopWidth(device, Point(25, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, width36Rotated, 1);

    font = baseFont;
    font.SetFontSize(Size(0, 72));
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice("vertical-text-72.png", device);
    // Font size is doubled, so pixel sizes should also roughly double.
    tools::Long height72 = getCharacterRightSideHeight(device, Point(99, 35));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 * 2, height72, 4);
    tools::Long width72 = getCharacterTopWidth(device, Point(40, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36 * 2, width72, 4);

    font.SetOrientation(0_deg10);
    font.SetVertical(false);
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(10, 10), text);
    exportDevice("vertical-text-72-0deg.png", device);
    tools::Long height72Rotated = getCharacterRightSideHeight(device, Point(99, 60));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height72, height72Rotated, 1);
    tools::Long width72Rotated = getCharacterTopWidth(device, Point(45, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width72, width72Rotated, 1);

    // On Windows scaling of vertical glyphs is broken.
    if (device->GetGraphics()->getRenderBackendName() == "gdi")
        return;

    // Test width scaled to 200%.
    font = baseFont;
    font.SetFontSize(Size(72, 36));
#ifdef _WIN32
    // TODO: What is the proper way to draw 200%-wide text? This is needed on Windows
    // but it breaks Linux.
    font.SetAverageFontWidth(2 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    // Double "width" with vertical text makes the height doubled.
    exportDevice("vertical-text-36-200pct.png", device);
    tools::Long height36pct200 = getCharacterRightSideHeight(device, Point(99, 35));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 * 2, height36pct200, 4);
    tools::Long width36pct200 = getCharacterTopWidth(device, Point(65, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, width36pct200, 2);

    // Test width scaled to 50%.
    font = baseFont;
    font.SetFontSize(Size(18, 36));
#ifdef _WIN32
    font.SetAverageFontWidth(0.5 * font.GetOrCalculateAverageFontWidth());
#endif
    device->Erase();
    device->SetFont(font);
    device->DrawText(Point(90, 10), text);
    exportDevice("vertical-text-36-50pct.png", device);
    tools::Long height36pct50 = getCharacterRightSideHeight(device, Point(99, 15));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 / 2, height36pct50, 2);
    tools::Long width36pct50 = getCharacterTopWidth(device, Point(65, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, width36pct50, 2);
}

void VclTextTest::testTextLayoutCache()
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dogالعاشر";
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

void VclTextTest::testImplLayoutRuns_AddPos()
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

void VclTextTest::testImplLayoutRuns_AddRuns()
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

void VclTextTest::testImplLayoutRuns_PosIsInRun()
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

void VclTextTest::testImplLayoutRuns_PosIsInAnyRun()
{
    ImplLayoutRuns aRuns;
    aRuns.AddRun(1, 4, false);
    aRuns.AddRun(4, 5, true);
    aRuns.AddRun(5, 6, false);
    aRuns.AddRun(6, 7, true);

    CPPUNIT_ASSERT(aRuns.PosIsInAnyRun(1));
    CPPUNIT_ASSERT(!aRuns.PosIsInAnyRun(7));
}

void VclTextTest::testImplLayoutArgsBiDiStrong()
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر";
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

void VclTextTest::testImplLayoutArgsBiDiRtl()
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر";
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

void VclTextTest::testImplLayoutArgsRightAlign()
{
    OUString sTestString = u"The quick brown fox\n jumped over the lazy dog"
                           "العاشر";
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

void VclTextTest::testImplLayoutArgs_PrepareFallback_precalculatedglyphs()
{
    // this font has no latin characters and thus needs fallback
    const vcl::Font aFont("KacstBook", Size(0, 36));

    ScopedVclPtrInstance<VirtualDevice> pVirDev;
    pVirDev->SetFont(aFont);

    const OUString sTestString = "The quick\n jumped over";
    std::unique_ptr<SalLayout> pLayout
        = pVirDev->ImplLayout(sTestString, 0, sTestString.getLength(), Point(0, 0), 0, {},
                              SalLayoutFlags::GlyphItemsOnly);
    SalLayoutGlyphs aGlyphs = pLayout->GetGlyphs();
    SalLayoutGlyphsImpl* pGlyphsImpl = aGlyphs.Impl(1);

    vcl::text::ImplLayoutArgs aArgs(sTestString, 0, sTestString.getLength(),
                                    SalLayoutFlags::BiDiRtl, LanguageTag(LANGUAGE_LATIN), nullptr);

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

CPPUNIT_TEST_SUITE_REGISTRATION(VclTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
