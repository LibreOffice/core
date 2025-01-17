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
#include <officecfg/Office/Common.hxx>

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
#include <fmtftntx.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/rtfexport/data/"_ustr, u"Rich Text Format"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf155663)
{
    auto verify = [this](bool bIsExport = false) {
        auto const xShape(getShape(1));
        if (!bIsExport)
        {
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Height);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2004), xShape->getSize().Width);
        }
        else // bit of rounding loss?
        {
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Height);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2013), xShape->getSize().Width);
        }
        CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                             getProperty<text::GraphicCrop>(xShape, u"GraphicCrop"_ustr).Top);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                             getProperty<text::GraphicCrop>(xShape, u"GraphicCrop"_ustr).Bottom);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                             getProperty<text::GraphicCrop>(xShape, u"GraphicCrop"_ustr).Left);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(123),
                             getProperty<text::GraphicCrop>(xShape, u"GraphicCrop"_ustr).Right);
    };
    createSwDoc("piccrop.rtf");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158586_0)
{
    auto verify = [this]() {
        // The specified page break must be lost because it is in a text frame
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

        // There should be no empty paragraph at the start
        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//anchored", 1);
        assertXPathContent(pLayout, "/root/page[1]/body//txt", u"First page");
    };
    createSwDoc("tdf158586_pageBreak0.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158586_0B)
{
    auto verify = [this]() {
        // The specified page break must be lost because it is in a text frame
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(1, getParagraphs());

        // There should be no empty paragraph at the start
        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//anchored", 1);
        assertXPathContent(pLayout, "/root/page[1]/body//txt", u"First page");
    };
    createSwDoc("tdf158586_pageBreak0B.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158586_1)
{
    auto verify = [this]() {
        // None of the specified text frame settings initiates a real text frame - page break not lost
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

        // There should be no empty carriage return at the start of the second page
        xmlDocUniquePtr pLayout = parseLayoutDump();
        // on import there is a section on page 2; on reimport there is no section
        // (probably not an important difference?)
        assertXPathContent(pLayout, "/root/page[2]/body//txt", u"Second page");
    };
    createSwDoc("tdf158586_pageBreak1.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158586_1header)
{
    auto verify = [this]() {
        // None of the specified text frame settings initiates a real text frame - page break not lost
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

        // There should be no empty carriage return at the start of the second page
        xmlDocUniquePtr pLayout = parseLayoutDump();
        // on import there is a section on page 2; on reimport there is no section
        // (probably not an important difference?)
        assertXPathContent(pLayout, "/root/page[2]/body//txt", u"Second page");
    };
    createSwDoc("tdf158586_pageBreak1_header.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158586_lostFrame)
{
    auto verify = [this]() {
        // The anchor and align properties are sufficient to define a frame
        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//anchored", 1);
        assertXPathContent(pLayout, "//page[1]/body//txt", u"First page textbox 1st page");
        assertXPathContent(pLayout, "//page[2]/body//txt", u"2nd page");

        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("tdf158586_lostFrame.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testEndnotesAtSectEndRTF)
{
    // Given a document, endnotes at collected at section end:
    createSwDoc();
    {
        SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
        pWrtShell->SplitNode();
        pWrtShell->Up(/*bSelect=*/false);
        pWrtShell->Insert(u"x"_ustr);
        pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
        SwSectionData aSection(SectionType::Content, pWrtShell->GetUniqueSectionName());
        pWrtShell->StartAction();
        SfxItemSetFixed<RES_FTN_AT_TXTEND, RES_FRAMEDIR> aSet(pWrtShell->GetAttrPool());
        aSet.Put(SwFormatEndAtTextEnd(FTNEND_ATTXTEND));
        pWrtShell->InsertSection(aSection, &aSet);
        pWrtShell->EndAction();
        pWrtShell->InsertFootnote(OUString(), /*bEndNote=*/true);
    }

    // When saving to DOC:
    saveAndReload(mpFilter);

    // Then make sure the endnote position is section end:
    SwDoc* pDoc = getSwDoc();
    SwSectionFormats& rSections = pDoc->GetSections();
    SwSectionFormat* pFormat = rSections[0];
    // Without the accompanying fix in place, this test would have failed, endnotes were at doc end.
    CPPUNIT_ASSERT(pFormat->GetEndAtTextEnd().IsAtEnd());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158983)
{
    auto verify = [this]() {
        // the problem was that the page break was missing and the shapes were
        // all anchored to the same node

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "/root/page[1]/body/section/txt", 1);
        assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/fly", 1);
        // Word shows these shapes anchored in the fly, not body, but at least they are not lost
        assertXPath(pLayout, "/root/page[1]/body/section/txt/anchored/SwAnchoredDrawObject", 2);
        // page break, paragraph break, section break.
        assertXPath(pLayout, "/root/page[2]/body/section[1]/txt", 1);
        assertXPath(pLayout, "/root/page[2]/body/section[1]/txt/anchored", 0);
        assertXPath(pLayout, "/root/page[2]/body/section[2]/txt", 1);
        assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/fly", 1);
        // Word shows these shapes anchored in the fly, not body, but at least they are not lost
        assertXPath(pLayout, "/root/page[2]/body/section[2]/txt/anchored/SwAnchoredDrawObject", 2);

        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo55504-1-min.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testAnnotationPar)
{
    auto verify = [this]() {
        // the problem was that the paragraph break following annotation was missing
        CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
        CPPUNIT_ASSERT_EQUAL(
            u"Annotation"_ustr,
            getProperty<OUString>(
                getRun(getParagraph(
                           1, u"I ax xoixx xuxixx xxe xixxx. (Xaxxexx 1989 x.x. xaxax a)"_ustr),
                       2),
                u"TextPortionType"_ustr));
        CPPUNIT_ASSERT(
            !getProperty<OUString>(getParagraph(2, u"Xix\txaxa\tx-a\t\t\txix\tx xi = xa."_ustr),
                                   u"ListId"_ustr)
                 .isEmpty());
    };
    createSwDoc("tdf136445-1-min.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158826_extraCR)
{
    auto verify = [this]() {
        // Note: this is a hand-minimized sample, and very likely doesn't follow RTF { } rules...

        // The page break defined before the document content should not cause a page break
        CPPUNIT_ASSERT_EQUAL(1, getPages());

        // There is a two-column floating table
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
    };
    createSwDoc("tdf158826_extraCR.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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
                         getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
    awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

    const Color aColA(0x127622); // green
    const Color aColB(0xffffff); // white

    CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_AXIAL, aGradient.Style);
    CPPUNIT_ASSERT_EQUAL(aColA, Color(ColorTransparency, aGradient.StartColor));
    CPPUNIT_ASSERT_EQUAL(aColB, Color(ColorTransparency, aGradient.EndColor));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_gradientAngle1)
{
    auto verify = [this]() {
        // given a frame with a white (top) to lime (bottom) linear gradient at an RTF 1° angle
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops
            = model::gradient::getColorStopsFromUno(aGradient.ColorStops);

        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.EndColor);
        // RTF 1° angle (in 1/60,000 degree units = 60,000) == LO 1° (in 1/10 degree units)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aGradient.Angle);
    };
    createSwDoc("tdf159824_gradientAngle1.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_gradientAngle2)
{
    auto verify = [this]() {
        // This file is identical to gradientAngle1 except that the fillAngle is -2° instead of 1°.

        // given a frame with a lime (top) to white (bottom) linear gradient at an RTF -2° angle.
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops
            = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        // because of the negative angle, the colors were swapped compared to gradientAngle1.rtf
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.EndColor);
        // RTF -2° angle (in 1/60,000 degree units = -120,000) == LO 358° (in 1/10 degree units)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(3580), aGradient.Angle);
    };
    createSwDoc("tdf159824_gradientAngle2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_gradientAngle3)
{
    auto verify = [this]() {
        // This file is identical to gradientAngle1 except that the fillAngle is 181° instead of 1°.

        // given a frame with a lime (top) to white (bottom) linear gradient at an RTF 181° angle.
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops
            = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.EndColor);
        // RTF 181° angle (in 1/60,000 degree units) == LO 181° (in 1/10 degree units)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1810), aGradient.Angle);
    };
    createSwDoc("tdf159824_gradientAngle3.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf159824_gradientAngle4)
{
    auto verify = [this]() {
        // This file is identical to gradientAngle1 except that the fillAngle is -90° instead of 1°.

        // given a frame with a white (left) to lime (right) linear gradient at an RTF -90° angle
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_GRADIENT,
                             getProperty<drawing::FillStyle>(xFrame, u"FillStyle"_ustr));
        awt::Gradient2 aGradient = getProperty<awt::Gradient2>(xFrame, u"FillGradient"_ustr);

        // MCGR: Use the completely imported transparency gradient to check for correctness
        basegfx::BColorStops aColorStops
            = model::gradient::getColorStopsFromUno(aGradient.ColorStops);
        CPPUNIT_ASSERT_EQUAL(size_t(2), aColorStops.size());
        // because of the negative angle, the colors were swapped compared to gradientAngle1.rtf
        CPPUNIT_ASSERT_EQUAL(awt::GradientStyle_LINEAR, aGradient.Style);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(8508442), aGradient.StartColor);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_WHITE), aGradient.EndColor);
        // RTF -90° angle (in 1/60,000 degree units) == LO 270° (in 1/10 degree units)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2700), aGradient.Angle);
    };
    createSwDoc("tdf159824_gradientAngle4.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf160976_headerFooter)
{
    // given a nasty ODT with first-even-odd page styles, emulate using RTF's abilities
    auto verify = [this](bool bIsExported = false) {
        // Sanity check - always good to test when dealing with page styles and breaks.
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPathContent(pXmlDoc, "/root/page[1]/footer/txt", u"First page first footer");
        assertXPathContent(pXmlDoc, "/root/page[2]/footer/txt", u"First Left");
        if (!bIsExported)
            assertXPathContent(pXmlDoc, "/root/page[3]/footer/txt", u"First Right");
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

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPathContent(pXmlDoc, "/root/page[1]/footer/txt", u"First page first footer");
        assertXPathContent(pXmlDoc, "/root/page[2]/footer/txt", u"First page footer");
        assertXPathContent(pXmlDoc, "/root/page[3]/footer/txt", u"Default footer");
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

        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPathContent(pXmlDoc, "/root/page[1]/footer/txt", u"First page footer");
        assertXPathContent(pXmlDoc, "/root/page[2]/footer/txt", u"Default footer");
        assertXPathContent(pXmlDoc, "/root/page[3]/footer/txt", u"Default footer");
    };
    createSwDoc("tdf160976_headerFooter3.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158830)
{
    auto verify = [this]() {
        //check centered text in table
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
        CPPUNIT_ASSERT_EQUAL(
            style::ParagraphAdjust_CENTER,
            static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
    };
    createSwDoc("tdf158830.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158978)
{
    auto verify = [this]() {
        //check right alignment in table1 or table3
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
        CPPUNIT_ASSERT_EQUAL(
            style::ParagraphAdjust_RIGHT,
            static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
    };
    createSwDoc("tdf158978.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testNotesAuthorDate)
{
    createSwDoc("text-with-comment.rtf");

    auto pBatch(comphelper::ConfigurationChanges::create());
    // Remove all personal info
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    saveAndReload(mpFilter);

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aRtfContent(read_uInt8s_ToOString(*pStream, pStream->TellEnd()));

    // Make sure user name was anonymized
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aRtfContent.indexOf("\\atnauthor Max Mustermann", 0));
    CPPUNIT_ASSERT(aRtfContent.indexOf("\\atnauthor Author1", 0) >= 0);

    // Make sure user initials were anonymized
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aRtfContent.indexOf("\\atnid MM", 0));
    CPPUNIT_ASSERT(aRtfContent.indexOf("\\atnid A1", 0) >= 0);

    // Make sure no date is set
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aRtfContent.indexOf("\\atndate", 0));
}

CPPUNIT_TEST_FIXTURE(Test, testChangesAuthor)
{
    createSwDoc("text-change-tracking.rtf");

    auto pBatch(comphelper::ConfigurationChanges::create());
    // Remove all personal info
    officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(true, pBatch);
    pBatch->commit();
    saveAndReload(mpFilter);

    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aRtfContent(read_uInt8s_ToOString(*pStream, pStream->TellEnd()));

    // Make sure user name was anonymized
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                         aRtfContent.indexOf("\\revtbl {Unknown;}{Max Mustermann;}", 0));
    CPPUNIT_ASSERT(aRtfContent.indexOf("\\revtbl {Author1;}{Author2;}", 0) >= 0);

    // Make sure no date is set
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), aRtfContent.indexOf("\\revdttmdel", 0));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158982)
{
    auto verify = [this]() {
        //check table count == 6
        // check left margin in a cell of the last table
        //
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTables->getCount());
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(5), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(508),
                             getProperty<sal_Int32>(xPara, u"ParaLeftMargin"_ustr));
    };
    createSwDoc("tdf158982.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161864)
{
    auto verify = [this]() {
        //check bullet size at position 0, 22, 45 with 4/11/16 pt
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextDocument->getText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<beans::XPropertySet> xPropSet(xCursor, uno::UNO_QUERY);
        xCursor->goRight(1, true);
        CPPUNIT_ASSERT_EQUAL(uno::Any(float(4)), xPropSet->getPropertyValue(u"CharHeight"_ustr));
        xCursor->goRight(21, false);
        xCursor->goRight(1, true);
        CPPUNIT_ASSERT_EQUAL(uno::Any(float(11)), xPropSet->getPropertyValue(u"CharHeight"_ustr));
        xCursor->goRight(22, false);
        xCursor->goRight(1, true);
        CPPUNIT_ASSERT_EQUAL(uno::Any(float(16)), xPropSet->getPropertyValue(u"CharHeight"_ustr));
    };
    createSwDoc("tdf161864.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf161878)
{
    auto verify = [this]() {
        //check that the IF field is a pos 2
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextDocument->getText();
        uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        uno::Reference<beans::XPropertySet> xPropSet(xCursor, uno::UNO_QUERY);
        xCursor->goRight(2, false);
        uno::Any xField = xPropSet->getPropertyValue("TextField");
        CPPUNIT_ASSERT(xField.hasValue());
    };
    createSwDoc("tdf161878.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
