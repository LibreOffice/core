/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/Gradient2.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>

#include <basegfx/utils/gradienttools.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/propertyvalue.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fmtpdsc.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentSettingAccess.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <formatflysplit.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testTdf155663, "piccrop.rtf")
{
    auto const xShape(getShape(1));
    if (!isExported())
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Height);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Width);
    }
    else // bit of rounding loss?
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Height);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Width);
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123), getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Top);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Bottom);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Left);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                         getProperty<text::GraphicCrop>(xShape, "GraphicCrop").Right);
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0, "tdf158586_pageBreak0.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty paragraph at the start
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored"_ostr, 1);
    assertXPathContent(pLayout, "/root/page[1]/body//txt"_ostr, "First page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_0B, "tdf158586_pageBreak0B.rtf")
{
    // The specified page break must be lost because it is in a text frame
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

    // There should be no empty paragraph at the start
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored"_ostr, 1);
    assertXPathContent(pLayout, "/root/page[1]/body//txt"_ostr, "First page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1, "tdf158586_pageBreak1.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    const auto& pLayout = parseLayoutDump();
    // on import there is a section on page 2; on reimport there is no section
    // (probably not an important difference?)
    assertXPathContent(pLayout, "/root/page[2]/body//txt"_ostr, "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_1header, "tdf158586_pageBreak1_header.rtf")
{
    // None of the specified text frame settings initiates a real text frame - page break not lost
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    // There should be no empty carriage return at the start of the second page
    const auto& pLayout = parseLayoutDump();
    // on import there is a section on page 2; on reimport there is no section
    // (probably not an important difference?)
    assertXPathContent(pLayout, "/root/page[2]/body//txt"_ostr, "Second page");
}

DECLARE_RTFEXPORT_TEST(testTdf158586_lostFrame, "tdf158586_lostFrame.rtf")
{
    // The anchor and align properties are sufficient to define a frame
    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "//anchored"_ostr, 1);
    assertXPathContent(pLayout, "//page[1]/body//txt"_ostr, "1st page");
    assertXPathContent(pLayout, "//page[2]/body//txt"_ostr, "2nd page");

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf158983, "fdo55504-1-min.rtf")
{
    // the problem was that the page break was missing and the shapes were
    // all anchored to the same node

    const auto& pLayout = parseLayoutDump();
    assertXPath(pLayout, "/root/page[1]/body/section/txt"_ostr, 1);
    assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/fly"_ostr, 1);
    // Word shows these shapes anchored in the fly, not body, but at least they are not lost
    assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/SwAnchoredDrawObject"_ostr, 2);
    // page break, paragraph break, section break.
    assertXPath(pLayout, "/root/page[2]/body/section[1]/txt"_ostr, 1);
    assertXPath(pLayout, "/root/page[2]/body/section[1]/txt/anchored"_ostr, 0);
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt"_ostr, 1);
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/fly"_ostr, 1);
    // Word shows these shapes anchored in the fly, not body, but at least they are not lost
    assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/SwAnchoredDrawObject"_ostr, 2);

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testAnnotationPar, "tdf136445-1-min.rtf")
{
    // the problem was that the paragraph break following annotation was missing
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL(
        OUString("Annotation"),
        getProperty<OUString>(
            getRun(getParagraph(1, "I ax xoixx xuxixx xxe xixxx. (Xaxxexx 1989 x.x. xaxax a)"), 2),
            "TextPortionType"));
    CPPUNIT_ASSERT(
        !getProperty<OUString>(getParagraph(2, "Xix\txaxa\tx-a\t\t\txix\tx xi = xa."), "ListId")
             .isEmpty());
}

DECLARE_RTFEXPORT_TEST(testTdf158826_extraCR, "tdf158826_extraCR.rtf")
{
    // Note: this is a hand-minimized sample, and very likely doesn't follow RTF { } rules...

    // The page break defined before the document content should not cause a page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // There is a two-column floating table
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_axialGradient)
{
    // given a frame with an axial gradient (white - green - white)
    loadAndReload("tdf159824_axialGradient.odt");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    const Color aColA(0x127622); // green
    const Color aColB(0xffffff); // white

    // MCGR: Use the completely imported transparency gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    // expected: a 3-color linear gradient (or better yet a 2-color AXIAL gradient)
    CPPUNIT_ASSERT_EQUAL(size_t(3), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[0].getStopOffset(), 0.0));
    CPPUNIT_ASSERT_EQUAL(aColB, Color(aColorStops[0].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[1].getStopOffset(), 0.5));
    CPPUNIT_ASSERT_EQUAL(aColA, Color(aColorStops[1].getStopColor()));
    CPPUNIT_ASSERT(basegfx::fTools::equal(aColorStops[2].getStopOffset(), 1.0));
    CPPUNIT_ASSERT_EQUAL(aColB, Color(aColorStops[2].getStopColor()));
}

DECLARE_RTFEXPORT_TEST(testTdf159824_gradientAngle1, "tdf159824_gradientAngle1.rtf")
{
    // given a frame with a white (top) to lime (bottom) linear gradient at an RTF 1° angle
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    // MCGR: Use the completely imported transparency gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.EndColor);
    // RTF 1° angle (in 1/60,000 degree units = 60,000) == LO 1° (in 1/10 degree units)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aGradient.Angle);
}

DECLARE_RTFEXPORT_TEST(testTdf159824_gradientAngle2, "tdf159824_gradientAngle2.rtf")
{
    // This file is identical to gradientAngle1 except that the fillAngle is -2° instead of 1°.

    // given a frame with a lime (top) to white (bottom) linear gradient at an RTF -2° angle.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    // MCGR: Use the completely imported transparency gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    // because of the negative angle, the colors were swapped compared to gradientAngle1.rtf
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.EndColor);
    // RTF -2° angle (in 1/60,000 degree units = -120,000) == LO 358° (in 1/10 degree units)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(3580), aGradient.Angle);
}

DECLARE_RTFEXPORT_TEST(testTdf159824_gradientAngle3, "tdf159824_gradientAngle3.rtf")
{
    // This file is identical to gradientAngle1 except that the fillAngle is 181° instead of 1°.

    // given a frame with a lime (top) to white (bottom) linear gradient at an RTF 181° angle.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    // MCGR: Use the completely imported transparency gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.EndColor);
    // RTF 181° angle (in 1/60,000 degree units) == LO 181° (in 1/10 degree units)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1810), aGradient.Angle);
}

DECLARE_RTFEXPORT_TEST(testTdf159824_gradientAngle4, "tdf159824_gradientAngle4.rtf")
{
    // This file is identical to gradientAngle1 except that the fillAngle is -90° instead of 1°.

    // given a frame with a white (left) to lime (right) linear gradient at an RTF -90° angle
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                         getProperty<drawing::FillStyle>(xFrame, "FillStyle"));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, "FillGradient");

    // MCGR: Use the completely imported transparency gradient to check for correctness
    basegfx::BColorStops aColorStops = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
    CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
    // because of the negative angle, the colors were swapped compared to gradientAngle1.rtf
    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.StartColor);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.EndColor);
    // RTF -90° angle (in 1/60,000 degree units) == LO 270° (in 1/10 degree units)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2700), aGradient.Angle);
}

DECLARE_RTFEXPORT_TEST(testTdf158830, "tdf158830.rtf")
{
    //check centered text in table
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf158978, "tdf158978.rtf")
{
    //check right alignment in table1 or table3
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_RIGHT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf158982, "tdf158982.rtf")
{
    //check table count == 6
    // check left margin in a cell of the last table
    //
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(5), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(508),
                         getProperty<sal_Int32>(xPara, "ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160976_headerFooter)
{
    // given a nasty ODT with first-even-odd page styles, emulate using RTF's abilities
    auto verify = [this](bool bIsExported = false) {
        // Sanity check - always good to test when dealing with page styles and breaks.
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        CPPUNIT_ASSERT_EQUAL(u"First page first footer"_ustr,
                             parseDump("/root/page[1]/footer/txt"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"First Left"_ustr, parseDump("/root/page[2]/footer/txt"_ostr));
        if (!bIsExported)
            CPPUNIT_ASSERT_EQUAL(u"First Right"_ustr, parseDump("/root/page[3]/footer/txt"_ostr));
    };
    createSwDoc("tdf160976_headerFooter.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*IsExported*/ true);

    //tdf#164748: export must have the all same footer settings as the first page
    auto xStyles = getStyles(u"PageStyles"_ustr);
    auto xPara = getParagraph(2, "2");
    OUString page2StyleName = getProperty<OUString>(xPara, u"PageDescName"_ustr);
    uno::Reference<beans::XPropertySet> page2Style;
    page2Style.set(xStyles->getByName(page2StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT(getProperty<bool>(page2Style, u"FooterDynamicSpacing"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(915),
                         getProperty<sal_Int32>(page2Style, u"FooterBodyDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1016), getProperty<sal_Int32>(page2Style, u"FooterHeight"_ustr));

    // note: an unexpected header surfaces on page 3.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160976_headerFooter2)
{
    // given a typical ODT with first-follow page styles, emulate using RTF's abilities
    auto verify = [this]() {
        // Sanity check - always good to test when dealing with page styles and breaks.
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        CPPUNIT_ASSERT_EQUAL(u"First page first footer"_ustr,
                             parseDump("/root/page[1]/footer/txt"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"First page footer"_ustr, parseDump("/root/page[2]/footer/txt"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"Default footer"_ustr, parseDump("/root/page[3]/footer/txt"_ostr));
    };
    createSwDoc("tdf160976_headerFooter2.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160976_headerFooter3)
{
    // given a simple ODT with typical "different first" on the default page style
    auto verify = [this]() {
        // Sanity check - always good to test when dealing with page styles and breaks.
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        CPPUNIT_ASSERT_EQUAL(u"First page footer"_ustr, parseDump("/root/page[1]/footer/txt"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"Default footer"_ustr, parseDump("/root/page[2]/footer/txt"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"Default footer"_ustr, parseDump("/root/page[3]/footer/txt"_ostr));
    };
    createSwDoc("tdf160976_headerFooter3.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
