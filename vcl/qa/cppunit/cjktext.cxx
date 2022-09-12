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

class VclCjkTextTest : public test::BootstrapFixture
{
    // if enabled - check the result images with:
    // "xdg-open ./workdir/CppunitTest/vcl_cjk_test.test.core/"
    static constexpr const bool mbExportBitmap = false;

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

    void testVerticalText();

public:
    VclCjkTextTest()
        : BootstrapFixture(true, false)
    {
    }

    CPPUNIT_TEST_SUITE(VclCjkTextTest);
    CPPUNIT_TEST(testVerticalText);
    CPPUNIT_TEST_SUITE_END();
};

// Similar to getCharacterBaseWidth but this time from the top, for U+30E8 (it's straight at the top, not at the bottom).
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

// Like testSimpleText() but for a vertical character, here namely U+30E8 (katakana letter yo),
// chosen because it's a fairly simple shape (looks like horizontally mirrored E) that should
// have the right and top lines being straight. Well, and also chosen because I actually
// do not have much clue about CJK.
// IMPORTANT: If you modify this, modify also the void VclTextTest::testSimpleText().
void VclCjkTextTest::testVerticalText()
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
        SAL_WARN("vcl",
                 "Could not find a font for VclCjkTextTest::testVerticalText, skipping test.");
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
    tools::Long height36 = getCharacterRightSideHeight(device, Point(99, 22));
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
    tools::Long height36pct50 = getCharacterRightSideHeight(device, Point(99, 16));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(height36 / 2, height36pct50, 2);
    tools::Long width36pct50 = getCharacterTopWidth(device, Point(65, 0));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(width36, width36pct50, 2);
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclCjkTextTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
