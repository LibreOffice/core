/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>

#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <xmloff/odffields.hxx>

#include <wrtsh.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IMark.hxx>
#include <sortedobjs.hxx>
#include <anchoredobject.hxx>
#include <fmtftn.hxx>
#include <ftnidx.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <frame.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlimport/data/", "Office Open XML Text")
    {
    }
};

DECLARE_OOXMLIMPORT_TEST(testTdf108545_embeddedDocxIcon, "tdf108545_embeddedDocxIcon.docx")
{
    uno::Reference<document::XEmbeddedObjectSupplier2> xSupplier(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(embed::Aspects::MSOLE_ICON, xSupplier->getAspect());
}

DECLARE_OOXMLIMPORT_TEST(testTdf121203, "tdf121203.docx")
{
    // We imported the date field
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Custom sdt date content is imported correctly
    ::sw::mark::IDateFieldmark* pFieldmark
        = dynamic_cast<::sw::mark::IDateFieldmark*>(*pMarkAccess->getAllMarksBegin());
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    OUString sDateFormat;
    auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sDateFormat;
    }

    OUString sLang;
    pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sLang;
    }

    OUString sCurrentDate = pFieldmark->GetContent();
    CPPUNIT_ASSERT_EQUAL(OUString("dd-MMM-yy"), sDateFormat);
    CPPUNIT_ASSERT_EQUAL(OUString("en-GB"), sLang);
    CPPUNIT_ASSERT_EQUAL(OUString("17-Oct-2018 09:00"), sCurrentDate);
}

DECLARE_OOXMLIMPORT_TEST(testTdf109053, "tdf109053.docx")
{
    // Table was imported into a text frame which led to a one page document
    // Originally the table takes two pages, so Writer should import it accordingly.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLIMPORT_TEST(testTdf121664, "tdf121664.docx")
{
    uno::Reference<text::XLineNumberingProperties> xLineNumbering(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xLineNumbering.is());
    // Without the accompanying fix in place, numbering did not restart on the
    // second page.
    CPPUNIT_ASSERT(
        getProperty<bool>(xLineNumbering->getLineNumberingProperties(), "RestartAtEachPage"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf108849, "tdf108849.docx")
{
    // sectPr element that is child element of body must be the last child. However, Word accepts it
    // in wrong places, and we should do the same (bug-to-bug compatibility) without creating extra sections.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Misplaced body-level sectPr's create extra sections!", 2,
                                 getPages());
}

DECLARE_OOXMLIMPORT_TEST(testTdf97038, "tdf97038.docx")
{
    // Without the accompanying fix in place, this test would have failed, as the importer lost the
    // fLayoutInCell shape property for wrap-though shapes.
    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Kep2"), "IsFollowingTextFlow"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf114212, "tdf114212.docx")
{
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1427
    // - Actual  : 387
    OUString aTop = parseDump("//anchored/fly[1]/infos/bounds", "top");
    CPPUNIT_ASSERT_EQUAL(OUString("1427"), aTop);
}

DECLARE_OOXMLIMPORT_TEST(testTdf109524, "tdf109524.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // The table should have a small width (just to hold the short text in its single cell).
    // Until it's correctly implemented, we assign it 100% relative width.
    // Previously, the table (without explicitly set width) had huge actual width
    // and extended far outside of page's right border.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), "IsWidthRelative"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100),
                         getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf120547, "tdf120547.docx")
{
    uno::Reference<drawing::XShape> xGroupShape = getShape(1);
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup->getCount());

    awt::Point aPosGroup = xGroupShape->getPosition();
    awt::Size aSizeGroup = xGroupShape->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosGroup.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosGroup.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9091), aSizeGroup.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27940), aSizeGroup.Height);

    // Without the fix in place, this test would have failed at many places
    // as the three shapes in the group would have had an incorrect position,
    // an incorrect width or an incorrect height.

    uno::Reference<drawing::XShape> xShape1(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    awt::Point aPosShape1 = xShape1->getPosition();
    awt::Size aSizeShape1 = xShape1->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape1.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape1.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9066), aSizeShape1.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(27905), aSizeShape1.Height);

    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    awt::Point aPosShape2 = xShape2->getPosition();
    awt::Size aSizeShape2 = xShape2->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape2.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20745), aPosShape2.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9066), aSizeShape2.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7195), aSizeShape2.Height);

    // The second shape is a group of 3 shapes
    uno::Reference<container::XIndexAccess> xGroup2(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xGroup2->getCount());

    uno::Reference<drawing::XShape> xShape3(xGroup->getByIndex(2), uno::UNO_QUERY_THROW);
    awt::Point aPosShape3 = xShape3->getPosition();
    awt::Size aSizeShape3 = xShape3->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape3.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), aPosShape3.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(9091), aSizeShape3.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8073), aSizeShape3.Height);
}

DECLARE_OOXMLIMPORT_TEST(testTdf118693, "tdf118693.docx")
{
    uno::Reference<drawing::XShape> xGroupShape = getShape(1);
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());

    awt::Point aPosGroup = xGroupShape->getPosition();
    awt::Size aSizeGroup = xGroupShape->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10162), aPosGroup.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(118), aPosGroup.Y);
    // As of LO7.2 width by 1 too small, height by 2 too small. Reason unclear.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6368), aSizeGroup.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4981), aSizeGroup.Height);

    // Without the fix in place, this test would have failed at many places
    // as the first shape in the group would have had an incorrect position,
    // an incorrect width or an incorrect height.

    uno::Reference<drawing::XShape> xShape1(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    awt::Point aPosShape1 = xShape1->getPosition();
    awt::Size aSizeShape1 = xShape1->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(12861), aPosShape1.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(146), aPosShape1.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3669), aSizeShape1.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4912), aSizeShape1.Height);

    uno::Reference<drawing::XShape> xShape2(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    awt::Point aPosShape2 = xShape2->getPosition();
    awt::Size aSizeShape2 = xShape2->getSize();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(10162), aPosShape2.X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(118), aPosShape2.Y);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4595), aSizeShape2.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4981), aSizeShape2.Height);
}

DECLARE_OOXMLIMPORT_TEST(testGroupShapeFontName, "groupshape-fontname.docx")
{
    // Font names inside a group shape were not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();

    CPPUNIT_ASSERT_EQUAL(
        OUString("Calibri"),
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("Calibri"),
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(
        OUString(""),
        getProperty<OUString>(getRun(getParagraphOfText(1, xText), 1), "CharFontNameAsian"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf124600, "tdf124600.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 318
    // i.e. the shape had an unexpected left margin, but not in Word.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape, "HoriOrientPosition"));

    // Make sure that "Shape 1 text" (anchored in the header) has the same left margin as the body
    // text.
    OUString aShapeTextLeft = parseDump("/root/page/header/txt/anchored/fly/infos/bounds", "left");
    OUString aBodyTextLeft = parseDump("/root/page/body/txt/infos/bounds", "left");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1701
    // - Actual  : 1815
    // i.e. there was a >0 left margin on the text of the shape, resulting in incorrect horizontal
    // position.
    CPPUNIT_ASSERT_EQUAL(aBodyTextLeft, aShapeTextLeft);
}

DECLARE_OOXMLIMPORT_TEST(testTdf120548, "tdf120548.docx")
{
    // Without the accompanying fix in place, this test would have failed with 'Expected: 00ff0000;
    // Actual: ffffffff', i.e. the numbering portion was black, not red.
    CPPUNIT_ASSERT_EQUAL(OUString("00ff0000"),
                         parseDump("//Special[@nType='PortionType::Number']/SwFont", "color"));
}

DECLARE_OOXMLIMPORT_TEST(test120551, "tdf120551.docx")
{
    auto nHoriOrientPosition = getProperty<sal_Int32>(getShape(1), "HoriOrientPosition");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 430, Actual  : -2542'.
    // CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(430), nHoriOrientPosition);
    // File 140335EMU = 389,8Hmm
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(392), nHoriOrientPosition);
}

DECLARE_OOXMLIMPORT_TEST(testTdf111550, "tdf111550.docx")
{
    // The test document has following ill-formed structure:
    //
    //    <w:tbl>
    //        ...
    //        <w:tr>
    //            <w:tc>
    //                <w:p>
    //                    <w:r>
    //                        <w:t>[outer:A2]</w:t>
    //                        <w:br w:type="textWrapping"/>
    //                    </w:r>
    //                    <w:tbl>
    //                        <w:tr>
    //                            <w:tc>
    //                                <w:p>
    //                                    <w:r>
    //                                        <w:t>[inner:A1]</w:t>
    //                                    </w:r>
    //                                </w:p>
    //                            </w:tc>
    //                        </w:tr>
    //                    </w:tbl>
    //                </w:p>
    //            </w:tc>
    //        </w:tr>
    //    </w:tbl>
    //
    // i.e., a <w:tbl> as direct child of <w:p> inside another table.
    // Word accepts that illegal OOXML, and treats it as equal to
    //
    //    <w:tbl>
    //        ...
    //        <w:tr>
    //            <w:tc>
    //                <w:tbl>
    //                    <w:tr>
    //                        <w:tc>
    //                            <w:p>
    //                                <w:r>
    //                                    <w:t>[outer:A2]</w:t>
    //                                    <w:br w:type="textWrapping"/>
    //                                </w:r>
    //                                <w:r>
    //                                    <w:t>[inner:A1]</w:t>
    //                                </w:r>
    //                            </w:p>
    //                        </w:tc>
    //                    </w:tr>
    //                </w:tbl>
    //            </w:tc>
    //        </w:tr>
    //    </w:tbl>
    //
    // i.e., moves all contents of the outer paragraph into the inner table's first paragraph.

    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());

    uno::Reference<text::XTextContent> outerTable = getParagraphOrTable(1);
    getCell(outerTable, "A1", "[outer:A1]");
    uno::Reference<text::XText> cellA2(getCell(outerTable, "A2"), uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextContent> innerTable = getParagraphOrTable(1, cellA2);
    getCell(innerTable, "A1", "[outer:A2]\n[inner:A1]");
}

DECLARE_OOXMLIMPORT_TEST(testTdf117843, "tdf117843.docx")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> xPageStyle(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XText> xHeaderText
        = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
    // This was 4025, increased top paragraph margin was unexpected.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xHeaderText), "ParaTopMargin"));
}

// related tdf#124754
DECLARE_OOXMLIMPORT_TEST(testTdf43017, "tdf43017.docx")
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2, "kick the bucket");

    // Ensure that hyperlink text color is not blue (0x0000ff), but default (-1)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hyperlink color should be black!", sal_Int32(-1),
                                 getProperty<sal_Int32>(xText, "CharColor"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127778)
{
    load(mpTestDocumentPath, "tdf127778.docx");
    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed with:
    // equality assertion failed
    // - Expected: 0
    // - Actual  : 1
    // i.e. the 2nd page had an unexpected header.
    assertXPath(pLayout, "//page[2]/header", 0);
}

// related tdf#43017
DECLARE_OOXMLIMPORT_TEST(testTdf124754, "tdf124754.docx")
{
    uno::Reference<text::XText> textbox(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs(textbox));

    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, textbox);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2);

    // Ensure that hyperlink text color is not black
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hyperlink color should be not black!", sal_Int32(353217),
                                 getProperty<sal_Int32>(xText, "CharColor"));
}

DECLARE_OOXMLIMPORT_TEST(testTextCopy, "text-copy.docx")
{
    // The document has a header on the second page that is copied as part of the import process.
    // The header has a single paragraph: make sure shapes anchored to it are not lost.
    // Note that the single paragraph itself has no text portions.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    while (xParaEnum->hasMoreElements())
    {
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    }
    auto aPageStyleName = getProperty<OUString>(xPara, "PageStyleName");
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles("PageStyles")->getByName(aPageStyleName), uno::UNO_QUERY);
    auto xHeaderText = getProperty<uno::Reference<text::XText>>(xPageStyle, "HeaderText");
    uno::Reference<container::XContentEnumerationAccess> xHeaderPara(
        getParagraphOfText(1, xHeaderText), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xHeaderShapes
        = xHeaderPara->createContentEnumeration("com.sun.star.text.TextContent");
    // Without the accompanying fix in place, this test would have failed with:
    // assertion failed
    // - Expression: xHeaderShapes->hasMoreElements()
    // i.e. the second page's header had no anchored shapes.
    CPPUNIT_ASSERT(xHeaderShapes->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf112443, "tdf112443.docx")
{
    // the position of the flying text frame should be off page
    // 30624 below its anchor
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwRootFrame* pRootFrame = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwRect aPageRect = pRootFrame->getFrameArea();
    const SwRect aShapeRect(getShape(1)->getPosition().X, getShape(1)->getPosition().Y,
                            getShape(1)->getSize().Width, getShape(1)->getSize().Height);
    CPPUNIT_ASSERT_MESSAGE("The textframe must be off-page!", !aPageRect.IsInside(aShapeRect));

    //OUString aTop = parseDump("//anchored/fly[1]/infos/bounds", "top");
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(30624), aTop.toInt32() );
}

// DOCX: Textbox wrap differs in MSO and LO
// Both should layout text regardless of existing text box
// and as result only one page should be generated.
DECLARE_OOXMLIMPORT_TEST(testTdf113182, "tdf113182.docx") { CPPUNIT_ASSERT_EQUAL(1, getPages()); }

DECLARE_OOXMLIMPORT_TEST(testBtlrFrameVml, "btlr-frame-vml.docx")
{
    uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextFrame.is());

    auto nActual = getProperty<sal_Int16>(xTextFrame, "WritingMode");
    // Without the accompanying fix in place, this test would have failed with 'Expected: 5; Actual:
    // 4', i.e. writing direction was inherited from page, instead of explicit btlr.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, nActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf124398, "tdf124398.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xGroup.is());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 2; Actual:
    // 1', i.e. the chart children of the group shape was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xGroup->getCount());

    uno::Reference<drawing::XShapeDescriptor> xShape(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.OLE2Shape"), xShape->getShapeType());
}

DECLARE_OOXMLIMPORT_TEST(testTdf104167, "tdf104167.docx")
{
    // Make sure that heading 1 paragraphs start on a new page.
    uno::Any xStyle = getStyles("ParagraphStyles")->getByName("Heading 1");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 0
    // i.e. the <w:pageBreakBefore/> was lost on import.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE,
                         getProperty<style::BreakType>(xStyle, "BreakType"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf113946, "tdf113946.docx")
{
    OUString aTop = parseDump("/root/page/body/txt/anchored/SwAnchoredDrawObject/bounds", "top");
    // tdf#106792 Checked loading of tdf113946.docx. Before the change, the expected
    // value of this test was "1696". Opening the file shows a single short line anchored
    // at the doc start. Only diff is that in 'old' version it is slightly rotated, in 'new'
    // version line is strict horizontal. Checked against MSWord2013, there the line
    // is also not rotated -> the change is to the better, correct the expected result here.
    CPPUNIT_ASSERT_EQUAL(OUString("1695"), aTop);
}

DECLARE_OOXMLIMPORT_TEST(testTdf121804, "tdf121804.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFirstPara = getParagraphOfText(1, xShape->getText());
    uno::Reference<text::XTextRange> xFirstRun = getRun(xFirstPara, 1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xFirstRun, "CharEscapement"));
    // This failed with a NoSuchElementException, super/subscript property was
    // lost on import, so the whole paragraph was a single run.
    uno::Reference<text::XTextRange> xSecondRun = getRun(xFirstPara, 2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(30),
                         getProperty<sal_Int32>(xSecondRun, "CharEscapement"));
    uno::Reference<text::XTextRange> xThirdRun = getRun(xFirstPara, 3);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-25),
                         getProperty<sal_Int32>(xThirdRun, "CharEscapement"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf114217, "tdf114217.docx")
{
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

DECLARE_OOXMLIMPORT_TEST(testTdf119200, "tdf119200.docx")
{
    auto xPara = getParagraph(1);
    // Check that we import MathType functional symbols as symbols, not functions with missing args
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2208 } {}"), getFormula(getRun(xPara, 1)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2209 } {}"), getFormula(getRun(xPara, 2)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2282 } {}"), getFormula(getRun(xPara, 3)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2283 } {}"), getFormula(getRun(xPara, 4)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2284 } {}"), getFormula(getRun(xPara, 5)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2286 } {}"), getFormula(getRun(xPara, 6)));
    CPPUNIT_ASSERT_EQUAL(OUString(u" size 12{ func \u2287 } {}"), getFormula(getRun(xPara, 7)));
}

DECLARE_OOXMLIMPORT_TEST(testTdf115094, "tdf115094.docx")
{
    // anchor of graphic has to be the text in the text frame
    // xray ThisComponent.DrawPage(1).Anchor.Text
    uno::Reference<text::XTextContent> xShape(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText1 = xShape->getAnchor()->getText();

    // xray ThisComponent.TextTables(0).getCellByName("A1")
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText2(xTable->getCellByName("A1"), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xText1.get(), xText2.get());
}

DECLARE_OOXMLIMPORT_TEST(testTdf115094v2, "tdf115094v2.docx")
{
    // layoutInCell="1" combined with <wp:wrapNone/>

    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Grafik 18"), "IsFollowingTextFlow"));
    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName(u"Grafik 19"), "IsFollowingTextFlow"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf122224, "tdf122224.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY_THROW);
    // This was "** Expression is faulty **", because of the unnecessary DOCX number format string
    CPPUNIT_ASSERT_EQUAL(OUString("2000"), xCell->getString());
}

DECLARE_OOXMLIMPORT_TEST(testTdf121440, "tdf121440.docx")
{
    // Insert some text in front of footnote
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRootFrame* pLayout(pWrtShell->GetLayout());
    CPPUNIT_ASSERT(!pLayout->IsHideRedlines());
    pWrtShell->Insert("test");

    // Ensure that inserted text is not superscripted
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Inserted text should be not a superscript!", static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharEscapement"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf124670, "tdf124670.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    // We need to take xml:space attribute into account, even in w:document element
    uno::Reference<text::XTextRange> paragraph = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(
        OUString("You won't believe, but that's how it was in markup of original      bugdoc!"),
        paragraph->getString());
}

DECLARE_OOXMLIMPORT_TEST(testTdf126114, "tdf126114.docx")
{
    // The problem was that after the drop-down form field, also the placeholder string
    // was imported as text. Beside the duplication of the field, it also caused a crash.
    CPPUNIT_ASSERT_EQUAL(7, getLength());
}

DECLARE_OOXMLIMPORT_TEST(testTdf127825, "tdf127825.docx")
{
    // The document has a shape with Japanese-style text in it. The shape has relative size and also
    // has automatic height.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    CPPUNIT_ASSERT(pLayout);
    SwFrame* pPage = pLayout->GetLower();
    CPPUNIT_ASSERT(pPage);
    SwFrame* pBody = pPage->GetLower();
    CPPUNIT_ASSERT(pBody);
    SwFrame* pText = pBody->GetLower();
    CPPUNIT_ASSERT(pText);
    CPPUNIT_ASSERT(pText->GetDrawObjs());
    const SwSortedObjs& rDrawObjs = *pText->GetDrawObjs();
    CPPUNIT_ASSERT(rDrawObjs.size());

    // Without the accompanying fix in place, this overlapped the footer area, not the body area.
    CPPUNIT_ASSERT(rDrawObjs[0]->GetObjRect().IsOver(pBody->getFrameArea()));
}

DECLARE_OOXMLIMPORT_TEST(testTdf103345, "numbering-circle.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const& prop : std::as_const(aProps))
    {
        if (prop.Name == "NumberingType")
        {
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::CIRCLE_NUMBER, prop.Value.get<sal_Int16>());
            return;
        }
    }
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038, "tdf125038.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone:...
    // - Actual  : result1result2phone:...
    // i.e. the result if the inner MERGEFIELD fields ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: \t1234567890"), aActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038b, "tdf125038b.docx")
{
    // Load a document with an IF field, where the IF field command contains a paragraph break.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphAccess(xTextDocument->getText(),
                                                                   uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphAccess->createEnumeration();
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    uno::Reference<text::XTextRange> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: phone: 1234
    // - Actual  :
    // i.e. the first paragraph was empty and the second paragraph had the content.
    CPPUNIT_ASSERT_EQUAL(OUString("phone: 1234"), xParagraph->getString());
    CPPUNIT_ASSERT(xParagraphs->hasMoreElements());
    xParagraphs->nextElement();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expression: !xParagraphs->hasMoreElements()
    // i.e. the document had 3 paragraphs, while only 2 was expected.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}

DECLARE_OOXMLIMPORT_TEST(testTdf125038c, "tdf125038c.docx")
{
    OUString aActual = getParagraph(1)->getString();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: email: test@test.test
    // - Actual  : email:
    // I.e. the result of the MERGEFIELD field inside an IF field was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("email: test@test.test"), aActual);
}

DECLARE_OOXMLIMPORT_TEST(testTdf130214, "tdf130214.docx")
{
    // Currently this file imports with errors because of tdf#126435; it must not segfault on load
}

DECLARE_OOXMLIMPORT_TEST(testTdf129659, "tdf129659.docx")
{
    // don't crash on footnote with page break
}

DECLARE_OOXMLIMPORT_TEST(testTdf129912, "tdf129912.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Goto*FootnoteAnchor iterates the footnotes in a ring, so we need the amount of footnotes to stop the loop
    sal_Int32 nCount = pWrtShell->GetDoc()->GetFootnoteIdxs().size();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), nCount);

    // the expected footnote labels
    // TODO: the 5th label is actually wrong (missing the "PR" after the symbol part), but the "b" is there?!
    static constexpr OUStringLiteral pLabel5 = u"\uF0D1\uF031\uF032b";
    const OUString sFootnoteLabels[]
        = { OUString(u'\xF0A7'), "1", "2", OUString(u'\xF020'), pLabel5 };
    CPPUNIT_ASSERT_EQUAL(sal_Int32(SAL_N_ELEMENTS(sFootnoteLabels)), nCount);

    pWrtShell->GotoPrevFootnoteAnchor();
    nCount--;
    while (nCount >= 0)
    {
        SwFormatFootnote aFootnoteNote;
        CPPUNIT_ASSERT(pWrtShell->GetCurFootnote(&aFootnoteNote));
        OUString sNumStr = aFootnoteNote.GetNumStr();
        if (sNumStr.isEmpty())
            sNumStr = OUString::number(aFootnoteNote.GetNumber());
        CPPUNIT_ASSERT_EQUAL(sFootnoteLabels[nCount], sNumStr);
        pWrtShell->GotoPrevFootnoteAnchor();
        nCount--;
    }
}

// tests should only be added to ooxmlIMPORT *if* they fail round-tripping in ooxmlEXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
