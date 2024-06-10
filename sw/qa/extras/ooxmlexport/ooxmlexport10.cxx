/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <tools/UnitConversion.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

DECLARE_OOXMLEXPORT_TEST(testWPGtextboxes, "testWPGtextboxes.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getShapes());

    auto MyShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, MyShape->getShapeType());

    uno::Reference<drawing::XShapes> xGroup(MyShape, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xTriangle(xGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShapes> xEmbedGroup(xGroup->getByIndex(1), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xCircle(xEmbedGroup->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDiamond(xEmbedGroup->getByIndex(1), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The circle lost its textbox", true, xCircle->getPropertyValue(u"TextBox"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The diamond lost its textbox", true, xDiamond->getPropertyValue(u"TextBox"_ustr).get<bool>());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The triangle lost its textbox", true, xTriangle->getPropertyValue(u"TextBox"_ustr).get<bool>());

}

CPPUNIT_TEST_FIXTURE(Test, testSmartart)
{
    // experimental config setting
    bool bOrigSet = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    Resetter resetter(
        [bOrigSet] () {
            std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
                    comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(bOrigSet, pBatch);
            return pBatch->commit();
        });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pBatch);
    pBatch->commit();

    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getShapes());

        uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xGroup->getCount()); // background, 3 rectangles and an arrow in the group

        uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0x4f81bd), getProperty<Color>(xPropertySet, u"FillColor"_ustr)); // If fill color is right, theme import is OK

        uno::Reference<text::XTextRange> xTextRange(xGroup->getByIndex(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Sample"_ustr, xTextRange->getString()); // Shape has text

        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextRange->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_CENTER), getProperty<sal_Int32>( xPropertySet, u"ParaAdjust"_ustr)); // Paragraph properties are imported
    };
    createSwDoc("smartart.docx");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69548)
{
    loadAndReload("fdo69548.docx");
    // The problem was that the last space in target URL was removed
    CPPUNIT_ASSERT_EQUAL(u"#this_is_a_bookmark"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testWpsOnly, "wps-only.docx")
{
    // Document has wp:anchor, not wp:inline, so handle it accordingly.
    uno::Reference<drawing::XShape> xShape = getShapeByName(u"Isosceles Triangle 1");
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(xShape, u"AnchorType"_ustr);
    // Word only as as-char and at-char, so at-char is our only choice.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, eValue);

    // Check position, it was 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(671830), xShape->getPosition().X);

    // Wrap type was PARALLEL.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(xShape, u"Surround"_ustr));
    // Confirm that the deprecated (incorrectly spelled) _THROUGHT also matches
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xShape, u"Surround"_ustr));

    // This should be in front of text.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, u"Opaque"_ustr));
    // And this should be behind the document.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShapeByName(u"Isosceles Triangle 2"), u"Opaque"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableNestedDOCXExport)
{
    // Given a document with nested floating tables:
    createSwDoc("floattable-nested.odt");

    // When exporting to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure both floating table is exported:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 1
    // i.e. the inner floating table was lost.
    assertXPath(pXmlDoc, "//w:tblpPr"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableNestedCellStartDOCXExport)
{
    // Given a document with a nested floating table at cell start:
    createSwDoc("floattable-nested-cell-start.odt");

    // When exporting to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure both floating table is exported at the right position:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with
    // - Expected: 1
    // - Actual  : 0
    // i.e. the inner <w:tbl> was between the two <w:tr>, not inside the C1 cell.
    assertXPath(pXmlDoc, "//w:tc/w:tbl/w:tblPr/w:tblpPr"_ostr, 1);
}

DECLARE_OOXMLEXPORT_TEST(testWpgOnly, "wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Check position, it was nearly 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(548005), xShape->getPosition().X);
}

DECLARE_OOXMLEXPORT_TEST(testWpgNested, "wpg-nested.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to lack of handling of groupshapes inside groupshapes.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, xShapeDescriptor->getShapeType());

    // This failed, the right edge of the shape was outside the page
    // boundaries.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "/root/page[1]/infos/bounds"_ostr, "left"_ostr).toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "/root/page[1]/infos/bounds"_ostr, "width"_ostr).toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/SwAnchoredDrawObject/bounds"_ostr, "left"_ostr)
              .toInt32();
    sal_Int32 nShapeWidth
        = getXPath(pXmlDoc, "/root/page[1]/body/txt/anchored/SwAnchoredDrawObject/bounds"_ostr, "width"_ostr)
              .toInt32();
    // Make sure the shape is within the page bounds.
    CPPUNIT_ASSERT_GREATEREQUAL(nShapeLeft + nShapeWidth, nPageLeft + nPageWidth);
}

DECLARE_OOXMLEXPORT_TEST(textboxWpgOnly, "textbox-wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // The relativeFrom attribute was ignored for groupshapes, i.e. these were text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
    // Make sure the shape is not in the background, as we have behindDoc="0" in the doc.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, u"Opaque"_ustr));

    // The 3 paragraphs on the rectangles inside the groupshape ended up in the
    // body text, make sure we don't have multiple paragraphs there anymore.
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs()); // was 4

    // Character escapement was enabled by default, this was 58.
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY_THROW)->getText();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xText), 1), u"CharEscapementHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf120412_400PercentSubscript, "tdf120412_400PercentSubscript.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    // The word "Base" should not be subscripted.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Base"_ustr), u"CharEscapement"_ustr), 0);
    // The word "Subscript" should be 12pt, subscripted by 400% (48pt).
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -400.f, getProperty<float>(getRun(xPara, 2, u"Subscript"_ustr), u"CharEscapement"_ustr), 0);
}

CPPUNIT_TEST_FIXTURE(Test, testFontEsc)
{
    loadAndSave("test_tdf120412.docx");
    xmlDocUniquePtr pXmlDoc =parseExport(u"word/document.xml"_ustr);
    // don't lose the run with superscript formatting
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r"_ostr, 2);
    // Superscript should be raised by 100% (11pt). Was 110% (12pt)
    // calculated using docDefault with fontsize 10pt (note only w:szCs defined as 11pt, not w:sz)
    // instead of inherited normal paraStyle fontsize 11pt (related to tdf#99602)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:position"_ostr,"val"_ostr, u"22"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testMceWpg, "mce-wpg.docx")
{
    // Make sure that we read the primary branch, if wpg is requested as a feature.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY_THROW)->getText();
    // This was VML1.
    getParagraphOfText(1, xText, u"DML1"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testMceNested, "mce-nested.docx")
{
    // Vertical position of the shape was incorrect due to incorrect nested mce handling.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // positionV's posOffset from the bugdoc, was 0.
    CPPUNIT_ASSERT(6879 <= getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr));
    // This was -1 (default), make sure the background color is set.
    CPPUNIT_ASSERT_EQUAL(Color(0x4f81bd), getProperty<Color>(xShape, u"FillColor"_ustr));

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor = getShape(2);
    // This was a com.sun.star.drawing.CustomShape, due to incorrect handling of wpg elements after a wps textbox.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.GroupShape"_ustr, xShapeDescriptor->getShapeType());

    // Now check the top right textbox.
    uno::Reference<container::XIndexAccess> xGroup(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, u"[Year]"_ustr);
    CPPUNIT_ASSERT_EQUAL(48.f, getProperty<float>(getRun(xParagraph, 1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, getProperty<Color>(getRun(xParagraph, 1), u"CharColor"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xParagraph, 1), u"CharWeight"_ustr));
    //FIXME: CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xGroup->getByIndex(1), "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testMissingPath, "missing-path.docx")
{
    comphelper::SequenceAsHashMap aCustomShapeGeometry(getProperty<beans::PropertyValues>(getShape(1), u"CustomShapeGeometry"_ustr));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry[u"Path"_ustr].get<beans::PropertyValues>());
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates = aPath[u"Coordinates"_ustr].get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    // This was 0, the coordinate list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aCoordinates.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testFdo70457, "fdo70457.docx")
{
    // The document contains a rotated bitmap
    // It must be imported as a XShape object with the proper rotation value

    // Check: there is one shape in the doc
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    // Check: the angle of the shape is 45º
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4500), getProperty<sal_Int32>(getShape(1), u"RotateAngle"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testLOCrash, "file_crash.docx")
{
    //The problem was libreoffice crash while opening the file.
    getParagraph(1,u"Contents"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560, "fdo72560.docx")
{
    // The problem was libreoffice confuse when there RTL default style for paragraph
    uno::Reference<uno::XInterface> xParaLeftRTL(getParagraph( 1, u"RTL LEFT"_ustr));
    uno::Reference<uno::XInterface> xParaRightLTR(getParagraph( 2, u"LTR RIGHT"_ustr));

    // this will test the text direction and alignment for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaLeftRTL, u"WritingMode"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaLeftRTL, u"ParaAdjust"_ustr ));

    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaRightLTR, u"WritingMode"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( xParaRightLTR, u"ParaAdjust"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560b, "fdo72560b.docx")
{
    // The problem was libreoffice confuse when RTL was specified in non-default style
    uno::Reference<uno::XInterface> xParaEndRTL(getParagraph( 2, u"RTL END"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaEndRTL, u"WritingMode"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaEndRTL, u"ParaAdjust"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560c, "fdo72560c.docx")
{
    // The problem was libreoffice confuse when RTL was specified in DocDefaults
    uno::Reference<uno::XInterface> xParaEndRTL(getParagraph( 2, u"RTL END"_ustr));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaEndRTL, u"WritingMode"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaEndRTL, u"ParaAdjust"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560d, "fdo72560d.docx")
{
    // The problem was libreoffice confuse when RTL was specified in "Normal" when not using Normal at all
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( getParagraph(1), u"ParaAdjust"_ustr ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( getParagraph(2), u"ParaAdjust"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560e, "fdo72560e.docx")
{
    // The problem was libreoffice confuse when *locale* is RTL, but w:bidi / w:jc are never defined.
    // This unit test would only be noticed if the testing environment is set to something like an Arabic locale.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( getParagraph(2), u"WritingMode"_ustr ));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_LEFT), getProperty<sal_Int32>( getParagraph(2), u"ParaAdjust"_ustr ));

    // widow/orphan control is on when never specified.
    CPPUNIT_ASSERT_EQUAL(sal_Int8(2), getProperty<sal_Int8>( getParagraph(2), u"ParaWidows"_ustr ));
}

DECLARE_OOXMLEXPORT_TEST(testRPrChangeClosed, "rprchange_closed.docx")
{
    // Redline defined by rPrChanged wasn't removed.
    // First paragraph has an rPrChange element, make sure it doesn't appear in the second paragraph.
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 1), u"RedlineType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65090, "fdo65090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // The first row had two cells, instead of a single horizontally merged one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr).getLength());
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not imported with correct page nos
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes( );
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xTOCIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText( );
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor( );
    xTextCursor->gotoRange(xTextRange->getStart(),false);
    xTextCursor->gotoRange(xTextRange->getEnd(),true);
    OUString aTocString(xTextCursor->getString());

    // heading 15 on the 15th page
    aTocString = aTocString.copy(aTocString.indexOf("Heading 15.1:\t") + strlen("Heading 15.1:\t"));
    CPPUNIT_ASSERT(aTocString.startsWithIgnoreAsciiCase( "15" ) );
}

CPPUNIT_TEST_FIXTURE(Test, testFdo73389)
{
    loadAndSave("fdo73389.docx");
    // The width of the inner table was too large. The first fix still converted
    // the "auto" table width to a fixed one. The second fix used variable width.
    // The recent fix uses fixed width again, according to the fixed width cells.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tblPr/w:tblW"_ostr,"type"_ostr,u"dxa"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tblPr/w:tblW"_ostr,"w"_ostr,u"1611"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133735)
{
    loadAndSave("fdo73389.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tr[2]/w:tc[1]/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    // This was 200
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tr[1]/w:tc[1]/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tr[1]/w:tc[2]/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl/w:tr[1]/w:tc[3]/w:p/w:pPr/w:spacing"_ostr, "after"_ostr, u"0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134569_nestedTable)
{
    loadAndReload("tdf134569_nestedTable.docx");
    // non-overridden w:after spacing in the table was pushing the document to the second page.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf59274)
{
    loadAndSave("tdf59274.docx");
    // Table with "auto" table width and incomplete grid: 11 columns, but only 4 gridCol elements.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW"_ostr, "type"_ostr, u"dxa"_ustr);
    // This was 7349: sum of the cell widths in first row, but the table width is determined by a longer row later.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tblPr/w:tblW"_ostr, "w"_ostr, u"9048"_ustr);
    // This was 1224: too narrow first cell in first row
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:tcPr/w:tcW"_ostr, "w"_ostr, u"4291"_ustr);
    // This was 3674: too wide last cell in first row
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[4]/w:tcPr/w:tcW"_ostr, "w"_ostr, u"1695"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupshapeSdt, "dml-groupshape-sdt.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    // The text in the groupshape was missing due to the w:sdt and w:sdtContent wrapper around it.
    CPPUNIT_ASSERT_EQUAL(u"sdt and sdtContent inside groupshape"_ustr, uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY_THROW)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testDmlCharheightDefault, "dml-charheight-default.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 16: the first run of the second para incorrectly inherited the char height of the first para.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(getRun(getParagraphOfText(2, xShape->getText()), 1), u"CharHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeCapitalization, "dml-groupshape-capitalization.docx")
{
    // Capitalization inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();
    // 2nd line is written with uppercase letters
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), u"CharCaseMap"_ustr));
    // 3rd line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), u"CharCaseMap"_ustr));
    // 4th line has written with small capitals
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::SMALLCAPS, getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), u"CharCaseMap"_ustr));
    // 5th line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), u"CharCaseMap"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testPictureWithSchemeColor, "picture-with-schemecolor.docx")
{
    // At the start of the document, a picture which has a color specified with a color scheme, lost
    // it's color during import.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, u"Graphic"_ustr);
    Graphic aVclGraphic(xGraphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(tools::Long(341), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(181), aBitmap.GetSizePixel().Height());
    CPPUNIT_ASSERT_EQUAL(Color( 0xad, 0xc5, 0xdb ), aBitmap.GetPixelColor(120, 30));
    CPPUNIT_ASSERT_EQUAL(Color( 0xad, 0xc5, 0xdb ), aBitmap.GetPixelColor(260, 130));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8153), getProperty<sal_Int32>(xTables->getByIndex(0), u"Width"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablesAnchor, "floating-tables-anchor.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    // Problem was one of the two text frames was anchored to the other text frame
    // Both frames should be anchored to the body text
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
    CPPUNIT_ASSERT_EQUAL(xBodyText, xRange->getText());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange = xTextContent->getAnchor();
    CPPUNIT_ASSERT_EQUAL(xBodyText, xRange->getText());

    // tdf#149292 pre-emptive test - ensure "First Page" page style
    // TODO - FIRST HEADER CHANGE - Need to find a solution to this
    //CPPUNIT_ASSERT_EQUAL(OUString("First Page"), getProperty<OUString>(getParagraph(1), "PageDescName"));
}

DECLARE_OOXMLEXPORT_TEST(testAnnotationFormatting, "annotation-formatting.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(2), 2), u"TextField"_ustr);
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XText> >(xTextField, u"TextRange"_ustr);
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, u"days"_ustr);
    // Formatting was lost: the second text portion was NONE, not SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(xParagraph, 1), u"CharUnderline"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeRunFonts, "dml-groupshape-runfonts.docx")
{
    // Fonts defined by w:rFonts was not imported and so the font specified by a:fontRef was used.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText    = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xRun, u"CharFontName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Arial Unicode MS"_ustr, getProperty<OUString>(xRun, u"CharFontNameComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"MS Mincho"_ustr, getProperty<OUString>(xRun, u"CharFontNameAsian"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testStrict, "strict.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was only 127, pt suffix was ignored, so this got parsed as twips instead of points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(72 * 20)), getProperty<sal_Int32>(xPageStyle, u"BottomMargin"_ustr));
    // This was only 1397, same issue
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(792 * 20)), getProperty<sal_Int32>(xPageStyle, u"Height"_ustr));
    // Text was missing, due to not handling the strict namespaces.
    getParagraph(1, u"Hello world!"_ustr);

    // Header in the document caused a crash on import.
    uno::Reference<text::XText> xHeaderText(xPageStyle->getPropertyValue(u"HeaderText"_ustr), uno::UNO_QUERY);
    getParagraphOfText(1, xHeaderText, u"This is a header."_ustr);

    // Picture was missing.
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShapeByName(u"Picture 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr));

    // SmartArt was missing.
    xServiceInfo.set(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.drawing.GroupShape"_ustr));

    // Chart was missing.
    xServiceInfo.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextEmbeddedObject"_ustr));

    // Math was missing.
    xServiceInfo.set(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextEmbeddedObject"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testSmartartStrict)
{
    // experimental config setting
    bool bOrigSet = officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::get();
    Resetter resetter(
        [bOrigSet] () {
            std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
                    comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(bOrigSet, pBatch);
            return pBatch->commit();
        });
    std::shared_ptr<comphelper::ConfigurationChanges> pBatch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Filter::Microsoft::Import::SmartArtToShapes::set(true, pBatch);
    pBatch->commit();

    auto verify = [this]() {
        uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
        // This was 0, SmartArt was visually missing.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), xGroup->getCount()); // background, 3 ellipses + 3 arrows
    };
    createSwDoc("strict-smartart.docx");
    verify();
    saveAndReload(mpFilter);
    verify();
}

DECLARE_OOXMLEXPORT_TEST(testLibreOfficeHang, "frame-wrap-auto.docx")
{
    // fdo#72775
    // This was text::WrapTextMode_NONE.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_DYNAMIC, getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));

    // tdf#154703 top/bottom margins should not be duplicated from paragraph(s)
    uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(u"test"_ustr, getParagraphOfText(1, xText)->getString());

    sal_Int32 nFrame = getProperty<sal_Int32>(getShape(1), u"TopBorderDistance"_ustr);
    sal_Int32 nPara = getProperty<sal_Int32>(getParagraphOfText(1, xText), u"TopBorderDistance"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(141), nFrame + nPara);
    // NOTE: left/right are different because of compat flag INVERT_BORDER_SPACING]
    nFrame = getProperty<sal_Int32>(getShape(1), u"LeftBorderDistance"_ustr);
    nPara = getProperty<sal_Int32>(getParagraphOfText(1, xText), u"LeftBorderDistance"_ustr);
    CPPUNIT_ASSERT_EQUAL(nFrame, nPara);
    CPPUNIT_ASSERT(nPara);

    // Additionally, the width of the frame needs to grow by the size of the left/right spacing
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7064), getProperty<sal_Int32>(getShape(1), u"Width"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testI124106, "i124106.docx")
{
    // This was 2.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testLargeTwips, "large-twips.docx" )
{
    // cp#1000043: MSO seems to ignore large twips values, we didn't, which resulted in different
    // layout of broken documents (text not visible in this specific document).
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds"_ostr, "width"_ostr );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLEXPORT_TEST(testNegativeCellMarginTwips, "negative-cell-margin-twips.docx")
{
    // Slightly related to cp#1000043, the twips value was negative, which wrapped around somewhere,
    // while MSO seems to ignore that as well.
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds"_ostr, "width"_ostr );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLEXPORT_TEST(testFdo38414, "fdo38414.docx")
{
    // The cells in the last (4th) column were merged properly and so the result didn't have the same height.
    // (Since w:gridBefore is worked around by faking another cell in the row, so column count is thus 5
    // instead of 4, therefore compare height of cells 4 and 5 rather than 3 and 4.)
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns = xTextTable->getColumns();
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 5 ), xTableColumns->getCount());
    OUString height3 = parseDump("/root/page/body/tab/row[1]/cell[4]/infos/bounds"_ostr, "height"_ostr );
    OUString height4 = parseDump("/root/page/body/tab/row[1]/cell[5]/infos/bounds"_ostr, "height"_ostr );
    CPPUNIT_ASSERT_EQUAL( height3, height4 );
}

DECLARE_OOXMLEXPORT_TEST(test_extra_image, "test_extra_image.docx")
{
    // fdo#74652 Check there is no shape added to the doc during import
    CPPUNIT_ASSERT_EQUAL(0, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testFdo74401, "fdo74401.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // The triangle (second child) was a TextShape before, so it was shown as a rectangle.
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShape->getShapeType());

    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    uno::Reference<text::XTextRange> xCharRun = getRun(getParagraphOfText(1, xText), 1, u"Triangle "_ustr);

    // tdf#128153 Paragraph Style Normal (Web) should not overwrite the 11pt directly applied fontsize.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Fontsize", 11.f, getProperty<float>(xCharRun, u"CharHeight"_ustr));
    // but paragraph Style Normal (Web) should provide the font name
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Font", u"Times New Roman"_ustr, getProperty<OUString>(xCharRun, u"CharFontName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testGridBefore, "gridbefore.docx")
{
    // w:gridBefore is faked by inserting two cells without border (because Writer can't do non-rectangular tables).
    // So check the first cell in the first row is in fact 3rd and that it's more to the right than the second
    // cell on the second row.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns = xTextTable->getColumns();
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 3 ), xTableColumns->getCount());
    OUString textA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/txt/text()"_ostr );
    OUString leftA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/infos/bounds"_ostr, "left"_ostr );
    OUString leftB2 = parseDump("/root/page/body/tab/row[2]/cell[2]/infos/bounds"_ostr, "left"_ostr );
    CPPUNIT_ASSERT_EQUAL( u"A3"_ustr, textA3 );
    CPPUNIT_ASSERT( leftA3.toInt32() > leftB2.toInt32());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116194)
{
    loadAndSave("tdf116194.docx");
    // The problem was that the importer lost consecutive tables with w:gridBefore
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134606)
{
    loadAndSave("tdf134606.docx");
    // The problem was that the importer lost the nested table structure with w:gridBefore
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:tbl"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.docx")
{
    uno::Reference<drawing::XShape> image = getShape(1);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( u"Graphic"_ustr ) >>= graphic;
    Graphic aVclGraphic(graphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(tools::Long(58), aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(320), aBitmap.GetSizePixel().Height());
    Color aColor(aBitmap.GetPixelColor(20, 30));
    CPPUNIT_ASSERT_EQUAL(Color( 0xce, 0xce, 0xce ), aColor);
}

DECLARE_OOXMLEXPORT_TEST(testChartSize, "chart-size.docx")
{
    // When chart was in a TextFrame, its size was too large.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 10954.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6008), getProperty<sal_Int32>(xEmbeddedObjects->getByIndex(0), u"Width"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testInlineGroupshape, "inline-groupshape.docx")
{
    // Inline groupshape was in the background, so it was hidden sometimes by other shapes.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), u"Opaque"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo78883, "fdo78883.docx")
{
    // fdo#78883 : LO was getting hang while opening document
    // Checking there is a single page after loading a doc in LO.
    // Check to make sure the document loaded.  Note that the page number may
    // be 1 or 2 depending on the environment.
    CPPUNIT_ASSERT(getPages() > 0);
}

DECLARE_OOXMLEXPORT_TEST(testFdo79535, "fdo79535.docx")
{
    // fdo#79535 : LO was crashing while opening document
    // Checking there is a single page after loading a doc successfully in LO.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testCaption, "caption.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Caption"_ustr), uno::UNO_QUERY);
    // This was awt::FontSlant_ITALIC: Writer default was used instead of what is in the document.
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, u"CharPosture"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTrackedchanges, "groupshape-trackedchanges.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Shape text was completely missing, ensure inserted text is available.
    uno::Reference<text::XText> xText
        = uno::Reference<text::XTextRange>(xShape, uno::UNO_QUERY_THROW)->getText();
    auto xParagraph = getParagraphOfText(1, xText);

    CPPUNIT_ASSERT(hasProperty(getRun(xParagraph, 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Delete"_ustr,
                         getProperty<OUString>(getRun(xParagraph, 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Deleted"_ustr, getRun(xParagraph, 2)->getString());

    CPPUNIT_ASSERT(hasProperty(getRun(xParagraph, 4), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr,
                         getProperty<OUString>(getRun(xParagraph, 4), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u" Inserted"_ustr, getRun(xParagraph, 5)->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
