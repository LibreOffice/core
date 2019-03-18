/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

#ifdef MACOSX
#define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#include <premac.h>
#include <AppKit/AppKit.h>
#include <postmac.h>
#endif

#include <swmodeltestbase.hxx>
#include <wrtsh.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/embed/Aspects.hpp>

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
    // Make sure that the date SDT's content is imported as plain text, as it
    // has no ISO date, so we have no idea how to represent that with our date
    // control.
    CPPUNIT_ASSERT_EQUAL(OUString("17-Oct-2018 09:00"), getRun(getParagraph(1), 1)->getString());
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

DECLARE_OOXMLIMPORT_TEST(testGroupShapeFontName, "groupshape-fontname.docx")
{
    // Font names inside a group shape were not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();

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
    // 'Expected: 436, Actual  : -2542'.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(436), nHoriOrientPosition);
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

DECLARE_OOXMLIMPORT_TEST(testTdf43017, "tdf43017.docx")
{
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    uno::Reference<text::XTextRange> xText = getRun(xParagraph, 2, "kick the bucket");

    // Ensure that hyperlink text color is not blue (0x0000ff), but default (-1)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Hyperlink color should be black!", sal_Int32(-1),
                                 getProperty<sal_Int32>(xText, "CharColor"));
}

DECLARE_OOXMLIMPORT_TEST(testTdf112443, "tdf112443.docx")
{
    // the position of the flying text frame should be off page
    // 30624 below its anchor
    OUString aTop = parseDump("//fly[1]/infos/bounds", "top");
    CPPUNIT_ASSERT_EQUAL(OUString("30624"), aTop);
}

// DOCX: Textbox wrap differs in MSO and LO
// Both should layout text regardless of existing text box
// and as result only one page should be generated.
DECLARE_OOXMLIMPORT_TEST(testTdf113182, "tdf113182.docx") { CPPUNIT_ASSERT_EQUAL(1, getPages()); }

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
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLIMPORT_TEST(testTdf116486, "tdf116486.docx")
{
    OUString aTop = parseDump("/root/page/body/txt/Special", "nHeight");
    CPPUNIT_ASSERT_EQUAL(OUString("4006"), aTop);
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
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(xDrawPageSupplier->getDrawPage(),
                                                      uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xShape(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText1(xShape->getAnchor()->getText(), uno::UNO_QUERY);

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
    // Introduce new attribute "layoutInCell"

    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName("Grafik 18"), "IsLayoutInCell"));
    CPPUNIT_ASSERT(getProperty<bool>(getShapeByName("Grafik 19"), "IsLayoutInCell"));
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

// tests should only be added to ooxmlIMPORT *if* they fail round-tripping in ooxmlEXPORT

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
