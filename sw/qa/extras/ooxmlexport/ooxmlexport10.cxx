/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <sstream>

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <vcl/bitmapaccess.hxx>
#include <unotest/assertion_traits.hxx>
#include <unotools/fltrcfg.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <swtypes.hxx>
#include <drawdoc.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/propertysequence.hxx>
#include <svx/svdpage.hxx>
#include <editeng/unoprnms.hxx>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        if (OString(filename) == "smartart.docx" || OString(filename) == "strict-smartart.docx" )
        {
            std::unique_ptr<Resetter> pResetter(new Resetter(
                [] () {
                    SvtFilterOptions::Get().SetSmartArt2Shape(false);
                }));
            SvtFilterOptions::Get().SetSmartArt2Shape(true);
            return pResetter;
        }
        return nullptr;
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
    /// Copy&paste helper.
    bool paste(const OUString& rFilename, const uno::Reference<text::XTextRange>& xTextRange)
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.WriterFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc("/sw/qa/extras/ooxmlexport/data/") + rFilename, StreamMode::READ);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence(
        {
            {"InputStream", uno::makeAny(xStream)},
            {"InputMode", uno::makeAny(true)},
            {"TextInsertModeRange", uno::makeAny(xTextRange)},
        }));
        return xFilter->filter(aDescriptor);
    }
};

DECLARE_OOXMLEXPORT_TEST(testSmartart, "smartart.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextDocumentPropertySet(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount()); // One groupshape in the doc

    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xGroup->getCount()); // 3 rectangles and an arrow in the group

    uno::Reference<beans::XPropertySet> xPropertySet(xGroup->getByIndex(1), uno::UNO_QUERY);
    sal_Int32 nValue(0);
    xPropertySet->getPropertyValue("FillColor") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), nValue); // If fill color is right, theme import is OK

    uno::Reference<text::XTextRange> xTextRange(xGroup->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Sample"), xTextRange->getString()); // Shape has text

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextRange->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("ParaAdjust") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(style::ParagraphAdjust_CENTER), nValue); // Paragraph properties are imported
}

DECLARE_OOXMLEXPORT_TEST(testFdo69548, "fdo69548.docx")
{
    // The problem was that the last space in target URL was removed
    CPPUNIT_ASSERT_EQUAL(OUString("#this is a bookmark"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

DECLARE_OOXMLEXPORT_TEST(testWpsOnly, "wps-only.docx")
{
    // Document has wp:anchor, not wp:inline, so handle it accordingly.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    text::TextContentAnchorType eValue = getProperty<text::TextContentAnchorType>(xShape, "AnchorType");
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_PARAGRAPH, eValue);

    // Check position, it was 0. This is a shape, so use getPosition(), not a property.
    CPPUNIT_ASSERT_EQUAL(oox::drawingml::convertEmuToHmm(671830), xShape->getPosition().X);

    // Left margin should be 0, as margins are not relevant for <wp:wrapNone/>.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "LeftMargin"));
    // Wrap type was PARALLEL.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(xShape, "Surround"));
    // Confirm that the deprecated (incorrectly spelled) _THROUGHT also matches
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xShape, "Surround"));

    // This should be in front of text.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "Opaque"));
    // And this should be behind the document.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getShape(2), "Opaque"));
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
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());

    // This failed, the right edge of the shape was outside the page
    // boundaries.
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nPageLeft = getXPath(pXmlDoc, "/root/page[2]/infos/bounds", "left").toInt32();
    sal_Int32 nPageWidth = getXPath(pXmlDoc, "/root/page[2]/infos/bounds", "width").toInt32();
    sal_Int32 nShapeLeft
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/SwAnchoredDrawObject/bounds", "left")
              .toInt32();
    sal_Int32 nShapeWidth
        = getXPath(pXmlDoc, "/root/page[2]/body/txt/anchored/SwAnchoredDrawObject/bounds", "width")
              .toInt32();
    // Make sure the shape is within the page bounds.
    CPPUNIT_ASSERT_GREATEREQUAL(nShapeLeft + nShapeWidth, nPageLeft + nPageWidth);
}

DECLARE_OOXMLEXPORT_TEST(textboxWpgOnly, "textbox-wpg-only.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // The relativeFrom attribute was ignored for groupshapes, i.e. these were text::RelOrientation::FRAME.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    // Make sure the shape is not in the background, as we have behindDoc="0" in the doc.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xShape, "Opaque"));

    // The 3 paragraphs on the rectangles inside the groupshape ended up in the
    // body text, make sure we don't have multiple paragraphs there anymore.
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs()); // was 4

    // Character escapement was enabled by default, this was 58.
    uno::Reference<container::XIndexAccess> xGroup(xShape, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xText), 1), "CharEscapementHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testMceWpg, "mce-wpg.docx")
{
    // Make sure that we read the primary branch, if wpg is requested as a feature.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(0), uno::UNO_QUERY)->getText();
    // This was VML1.
    getParagraphOfText(1, xText, "DML1");
}

DECLARE_OOXMLEXPORT_TEST(testMceNested, "mce-nested.docx")
{
    // Vertical position of the shape was incorrect due to incorrect nested mce handling.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // positionV's posOffset from the bugdoc, was 0.
    CPPUNIT_ASSERT(6985 <= getProperty<sal_Int32>(xShape, "VertOrientPosition"));
    // This was -1 (default), make sure the background color is set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x4f81bd), getProperty<sal_Int32>(xShape, "FillColor"));

    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(getShape(2), uno::UNO_QUERY);
    // This was a com.sun.star.drawing.CustomShape, due to incorrect handling of wpg elements after a wps textbox.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.GroupShape"), xShapeDescriptor->getShapeType());

    // Now check the top right textbox.
    uno::Reference<container::XIndexAccess> xGroup(getShape(2), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "[Year]");
    CPPUNIT_ASSERT_EQUAL(48.f, getProperty<float>(getRun(xParagraph, 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xffffff), getProperty<sal_Int32>(getRun(xParagraph, 1), "CharColor"));
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(xParagraph, 1), "CharWeight"));
    CPPUNIT_ASSERT_EQUAL(drawing::TextVerticalAdjust_BOTTOM, getProperty<drawing::TextVerticalAdjust>(xGroup->getByIndex(1), "TextVerticalAdjust"));
}

DECLARE_OOXMLEXPORT_TEST(testMissingPath, "missing-path.docx")
{
    comphelper::SequenceAsHashMap aCustomShapeGeometry(getProperty<beans::PropertyValues>(getShape(1), "CustomShapeGeometry"));
    comphelper::SequenceAsHashMap aPath(aCustomShapeGeometry["Path"].get<beans::PropertyValues>());
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates = aPath["Coordinates"].get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    // This was 0, the coordinate list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), aCoordinates.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testFdo70457, "fdo70457.docx")
{
    // The document contains a rotated bitmap
    // It must be imported as a XShape object with the proper rotation value

    // Check: there is one shape in the doc
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());

    // Check: the angle of the shape is 45º
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4500), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_OOXMLEXPORT_TEST(testLOCrash,"file_crash.docx")
{
    //The problem was libreoffice crash while opening the file.
    getParagraph(1,"Contents");
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560, "fdo72560.docx")
{
    // The problem was libreoffice confuse when there RTL default style for paragraph
    uno::Reference<uno::XInterface> xParaLeftRTL(getParagraph( 1, "RTL LEFT"));
    uno::Reference<uno::XInterface> xParaRightLTR(getParagraph( 2, "LTR RIGHT"));

    // this will test the text direction and alignment for paragraphs
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaLeftRTL, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaLeftRTL, "ParaAdjust" ));

    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::LR_TB, getProperty<sal_Int16>( xParaRightLTR, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32 (style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( xParaRightLTR, "ParaAdjust" ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560b, "fdo72560b.docx")
{
    // The problem was libreoffice confuse when RTL was specified in non-default style
    uno::Reference<uno::XInterface> xParaEndRTL(getParagraph( 2, "RTL END"));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaEndRTL, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaEndRTL, "ParaAdjust" ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560c, "fdo72560c.docx")
{
    // The problem was libreoffice confuse when RTL was specified in DocDefaults
    uno::Reference<uno::XInterface> xParaEndRTL(getParagraph( 2, "RTL END"));
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>( xParaEndRTL, "WritingMode" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_LEFT), getProperty< sal_Int32 >( xParaEndRTL, "ParaAdjust" ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo72560d, "fdo72560d.docx")
{
    // The problem was libreoffice confuse when RTL was specified in "Normal" when not using Normal at all
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( getParagraph(1), "ParaAdjust" ));
    CPPUNIT_ASSERT_EQUAL( sal_Int32(style::ParagraphAdjust_RIGHT), getProperty< sal_Int32 >( getParagraph(1), "ParaAdjust" ));
}

DECLARE_OOXMLEXPORT_TEST(testRPrChangeClosed, "rprchange_closed.docx")
{
    // Redline defined by rPrChanged wasn't removed.
    // First paragraph has an rPrChange element, make sure it doesn't appear in the second paragraph.
    CPPUNIT_ASSERT_EQUAL(false, hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo65090, "fdo65090.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had two cells, instead of a single horizontally merged one.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testFdo69649, "fdo69649.docx")
{
    // The DOCX containing the Table of Contents was not imported with correct page nos
    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes(xIndexSupplier->getDocumentIndexes( ), uno::UNO_QUERY);
    uno::Reference<text::XDocumentIndex> xTOCIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xTOCIndex->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xTextRange->getText( ), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(xText->createTextCursor( ), uno::UNO_QUERY);
    xTextCursor->gotoRange(xTextRange->getStart(),false);
    xTextCursor->gotoRange(xTextRange->getEnd(),true);
    OUString aTocString(xTextCursor->getString());

    // heading 15 on the 15th page
    aTocString = aTocString.copy(aTocString.indexOf("Heading 15.1:\t") + strlen("Heading 15.1:\t"));
    CPPUNIT_ASSERT(aTocString.startsWithIgnoreAsciiCase( "15" ) );
}

DECLARE_OOXMLEXPORT_TEST(testFdo73389,"fdo73389.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // This was 9340, i.e. the width of the inner table was too large.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2842), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupshapeSdt, "dml-groupshape-sdt.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    // The text in the groupshape was missing due to the w:sdt and w:sdtContent wrapper around it.
    CPPUNIT_ASSERT_EQUAL(OUString("sdt and sdtContent inside groupshape"), uno::Reference<text::XTextRange>(xGroupShape->getByIndex(1), uno::UNO_QUERY)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testDmlCharheightDefault, "dml-charheight-default.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was 16: the first run of the second para incorrectly inherited the char height of the first para.
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(getRun(getParagraphOfText(2, xShape->getText()), 1), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeCapitalization, "dml-groupshape-capitalization.docx")
{
    // Capitalization inside a group shape was not imported
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    // 2nd line is written with uppercase letters
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(getRun(getParagraphOfText(2, xText), 1), "CharCaseMap"));
    // 3rd line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(3, xText), 1), "CharCaseMap"));
    // 4th line has written with small capitals
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::SMALLCAPS, getProperty<sal_Int16>(getRun(getParagraphOfText(4, xText), 1), "CharCaseMap"));
    // 5th line has no capitalization
    CPPUNIT_ASSERT_EQUAL(style::CaseMap::NONE, getProperty<sal_Int16>(getRun(getParagraphOfText(5, xText), 1), "CharCaseMap"));
}

DECLARE_OOXMLEXPORT_TEST(testPictureWithSchemeColor, "picture-with-schemecolor.docx")
{
    // At the start of the document, a picture which has a color specified with a color scheme, lost
    // it's color during import.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    Graphic aVclGraphic(xGraphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(341L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(181L, aBitmap.GetSizePixel().Height());
    Color aColor(aBitmap.GetPixelColor(120, 30));
    CPPUNIT_ASSERT_EQUAL(aColor, Color( 0xb1, 0xc8, 0xdd ));
    aColor = aBitmap.GetPixelColor(260, 130);
    CPPUNIT_ASSERT_EQUAL(aColor, Color( 0xb1, 0xc8, 0xdd ));
}

DECLARE_OOXMLEXPORT_TEST(testFdo69656, "Table_cell_auto_width_fdo69656.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8154), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testFloatingTablesAnchor, "floating-tables-anchor.docx")
{
    // Problem was one of the two text frames was anchored to the other text frame
    // Both frames should be anchored to the paragraph with the text "Anchor point"
    uno::Reference<text::XTextContent> xTextContent(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
    uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());

    xTextContent.set(getShape(2), uno::UNO_QUERY);
    xRange.set(xTextContent->getAnchor(), uno::UNO_QUERY);
    xText.set(xRange->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Anchor point"), xText->getString());
}

DECLARE_OOXMLEXPORT_TEST(testAnnotationFormatting, "annotation-formatting.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(2), 2), "TextField");
    uno::Reference<text::XText> xText = getProperty< uno::Reference<text::XText> >(xTextField, "TextRange");
    // Make sure we test the right annotation.
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xText, "days");
    // Formatting was lost: the second text portion was NONE, not SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::SINGLE, getProperty<sal_Int16>(getRun(xParagraph, 1), "CharUnderline"));
}

DECLARE_OOXMLEXPORT_TEST(testDMLGroupShapeRunFonts, "dml-groupshape-runfonts.docx")
{
    // Fonts defined by w:rFonts was not imported and so the font specified by a:fontRef was used.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText    = uno::Reference<text::XTextRange>(xGroup->getByIndex(1), uno::UNO_QUERY)->getText();
    uno::Reference<text::XTextRange> xRun = getRun(getParagraphOfText(1, xText),1);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xRun, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Arial Unicode MS"), getProperty<OUString>(xRun, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(OUString("MS Mincho"), getProperty<OUString>(xRun, "CharFontNameAsian"));
}

DECLARE_OOXMLEXPORT_TEST(testStrict, "strict.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was only 127, pt suffix was ignored, so this got parsed as twips instead of points.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(72 * 20)), getProperty<sal_Int32>(xPageStyle, "BottomMargin"));
    // This was only 1397, same issue
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(792 * 20)), getProperty<sal_Int32>(xPageStyle, "Height"));
    // Text was missing, due to not handling the strict namespaces.
    getParagraph(1, "Hello world!");

    // Header in the document caused a crash on import.
    uno::Reference<text::XText> xHeaderText(xPageStyle->getPropertyValue("HeaderText"), uno::UNO_QUERY);
    getParagraphOfText(1, xHeaderText, "This is a header.");

    // Picture was missing.
    uno::Reference<lang::XServiceInfo> xServiceInfo(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextGraphicObject"));

    // SmartArt was missing.
    xServiceInfo.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.GroupShape"));

    // Chart was missing.
    xServiceInfo.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));

    // Math was missing.
    xServiceInfo.set(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextEmbeddedObject"));
}

DECLARE_OOXMLEXPORT_TEST(testSmartartStrict, "strict-smartart.docx")
{
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    // This was 0, SmartArt was visually missing.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xGroup->getCount()); // 3 ellipses + 3 arrows
}

DECLARE_OOXMLEXPORT_TEST(testLibreOfficeHang, "frame-wrap-auto.docx")
{
    // fdo#72775
    // This was text::WrapTextMode_NONE.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_DYNAMIC, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
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
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width" );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLEXPORT_TEST(testNegativeCellMarginTwips, "negative-cell-margin-twips.docx" )
{
    // Slightly related to cp#1000043, the twips value was negative, which wrapped around somewhere,
    // while MSO seems to ignore that as well.
    OUString width = parseDump( "/root/page/body/tab/row[1]/cell[1]/txt/infos/bounds", "width" );
    CPPUNIT_ASSERT( width.toInt32() > 0 );
}

DECLARE_OOXMLEXPORT_TEST(testFdo38414, "fdo38414.docx" )
{
    // The cells in the last (4th) column were merged properly and so the result didn't have the same height.
    // (Since w:gridBefore is worked around by faking another cell in the row, so column count is thus 5
    // instead of 4, therefore compare height of cells 4 and 5 rather than 3 and 4.)
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 5 ), xTableColumns->getCount());
    OUString height3 = parseDump("/root/page/body/tab/row[1]/cell[4]/infos/bounds", "height" );
    OUString height4 = parseDump("/root/page/body/tab/row[1]/cell[5]/infos/bounds", "height" );
    CPPUNIT_ASSERT_EQUAL( height3, height4 );
}

DECLARE_OOXMLEXPORT_TEST(test_extra_image, "test_extra_image.docx" )
{
    // fdo#74652 Check there is no shape added to the doc during import
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xDraws->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testFdo74401, "fdo74401.docx")
{
    uno::Reference<drawing::XShapes> xGroupShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShape(xGroupShape->getByIndex(1), uno::UNO_QUERY);
    // The triangle (second child) was a TextShape before, so it was shown as a rectangle.
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.CustomShape"), xShape->getShapeType());
}

DECLARE_OOXMLEXPORT_TEST(testGridBefore, "gridbefore.docx")
{
    // w:gridBefore is faked by inserting two cells without border (because Writer can't do non-rectangular tables).
    // So check the first cell in the first row is in fact 3rd and that it's more to the right than the second
    // cell on the second row.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableColumns> xTableColumns(xTextTable->getColumns(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 3 ), xTableColumns->getCount());
    OUString textA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/txt/text()" );
    OUString leftA3 = parseDump("/root/page/body/tab/row[1]/cell[3]/infos/bounds", "left" );
    OUString leftB2 = parseDump("/root/page/body/tab/row[2]/cell[2]/infos/bounds", "left" );
    CPPUNIT_ASSERT_EQUAL( OUString( "A3" ), textA3 );
    CPPUNIT_ASSERT( leftA3.toInt32() > leftB2.toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testMsoBrightnessContrast, "msobrightnesscontrast.docx")
{
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> image(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> imageProperties(image, uno::UNO_QUERY);
    uno::Reference<graphic::XGraphic> graphic;
    imageProperties->getPropertyValue( "Graphic" ) >>= graphic;
    uno::Reference<awt::XBitmap> bitmap(graphic, uno::UNO_QUERY);
    Graphic aVclGraphic(graphic);
    BitmapEx aBitmap(aVclGraphic.GetBitmapEx());
    CPPUNIT_ASSERT_EQUAL(58L, aBitmap.GetSizePixel().Width());
    CPPUNIT_ASSERT_EQUAL(320L, aBitmap.GetSizePixel().Height());
    Color aColor(aBitmap.GetPixelColor(20, 30));
    CPPUNIT_ASSERT_EQUAL(Color( 255, 0xce, 0xce, 0xce ), aColor);
}

DECLARE_OOXMLEXPORT_TEST(testChartSize, "chart-size.docx")
{
    // When chart was in a TextFrame, its size was too large.
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This was 10954.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6008), getProperty<sal_Int32>(xEmbeddedObjects->getByIndex(0), "Width"));

    // Layout modified the document when it had this chart.
    uno::Reference<util::XModifiable> xModifiable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(xModifiable->isModified()));
}

DECLARE_OOXMLEXPORT_TEST(testInlineGroupshape, "inline-groupshape.docx")
{
    // Inline groupshape was in the background, so it was hidden sometimes by other shapes.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "Opaque"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo78883, "fdo78883.docx")
{
    // fdo#78883 : LO was getting hang while opening document
    // Checking there is a single page after loading a doc in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();

    // Check to make sure the document loaded.  Note that the page number may
    // be 1 or 2 depending on the environment.
    CPPUNIT_ASSERT(xCursor->getPage() > sal_Int16(0));
}

DECLARE_OOXMLEXPORT_TEST(testFdo79535, "fdo79535.docx")
{
    // fdo#79535 : LO was crashing while opening document
    // Checking there is a single page after loading a doc successfully in LO.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testBnc875718, "bnc875718.docx")
{
    // The frame in the footer must not accidentally end up in the document body.
    // The easiest way for this to test I've found is checking that
    // xray ThisComponent.TextFrames.GetByIndex( index ).Anchor.Text.ImplementationName
    // is not SwXBodyText but rather SwXHeadFootText
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    for( int i = 0;
         i < xIndexAccess->getCount();
         ++i )
    {
        uno::Reference<text::XTextFrame> frame(xIndexAccess->getByIndex( i ), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> range(frame->getAnchor(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> text(range->getText(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL( OUString( "SwXHeadFootText" ), text->getImplementationName());
    }
    // Also check that the footer contents are not in the body text.
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> text(textDocument->getText(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( OUString( "Text" ), text->getString());
}

DECLARE_OOXMLEXPORT_TEST(testCaption, "caption.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Caption"), uno::UNO_QUERY);
    // This was awt::FontSlant_ITALIC: Writer default was used instead of what is in the document.
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
}

DECLARE_OOXMLEXPORT_TEST(testGroupshapeTrackedchanges, "groupshape-trackedchanges.docx")
{
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    // Shape text was completely missing, ensure inserted text is available.
    CPPUNIT_ASSERT_EQUAL(OUString(" Inserted"), xShape->getString());
}

DECLARE_OOXMLEXPORT_TEST(testFdo78939, "fdo78939.docx")
{
    // fdo#78939 : LO hanged while opening issue document

    // Whenever a para-style was applied to a Numbering format level,
    // LO incorrectly also changed the para-style..

    // check that file opens and does not hang while opening and also
    // check that an incorrect numbering style is not applied ...
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testFootnote, "footnote.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFootnote(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aFootnote = xFootnote->getString();
    // Ensure there are no additional newlines after "bar".
    CPPUNIT_ASSERT(aFootnote.endsWith("bar"));
}

DECLARE_OOXMLEXPORT_TEST(testTableBtlrCenter, "table-btlr-center.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell vertical alignment was NONE, should be CENTER.
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xTable->getCellByName("A2"), "VertOrient"));
}

DECLARE_OOXMLEXPORT_TEST(testFdo80555, "fdo80555.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Shape was wrongly placed at X=0, Y=0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3318), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(247), xShape->getPosition().Y);
}

DECLARE_OOXMLEXPORT_TEST(testHidemark, "hidemark.docx")
{
    // Problem was that <w:hideMark> cell property was ignored.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Height should be minimal
    CPPUNIT_ASSERT_EQUAL(convertTwipToMm100(MINLAY), getProperty<sal_Int64>(xTableRows->getByIndex(1), "Height"));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));

    //tdf#104876: Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_MESSAGE("table size is less than 7000?",sal_Int32(7000) > getProperty<sal_Int32>(xTextTable, "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testHidemarkb, "tdf99616_hidemarkb.docx")
{
    // Problem was that the smallest possible height was forced, not the min specified size.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // Height should be .5cm
    CPPUNIT_ASSERT_EQUAL(sal_Int64(501), getProperty<sal_Int64>(xTableRows->getByIndex(1), "Height"));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xTableRows->getByIndex(1), "SizeType"));
}

DECLARE_OOXMLEXPORT_TEST(testBnc891663, "bnc891663.docx")
{
    // The image should be inside a cell, so the text in the following cell should be below it.
    int imageTop = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "top").toInt32();
    int imageHeight = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds", "height").toInt32();
    int textNextRowTop = parseDump("/root/page/body/tab/row[2]/cell[1]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT( textNextRowTop >= imageTop + imageHeight );
}

DECLARE_OOXMLEXPORT_TEST(testTcwRounding, "tcw-rounding.docx")
{
    // Width of the A1 cell in twips was 3200, due to a rounding error.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3201),  parseDump("/root/page/body/tab/row/cell[1]/infos/bounds", "width").toInt32());
}

DECLARE_OOXMLEXPORT_TEST(testFdo85542, "fdo85542.docx")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B1"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B2"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("B3"));
    // B1
    uno::Reference<text::XTextContent> xContent1(xBookmarksByName->getByName("B1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange1(xContent1->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("ABB"), xRange1->getString());
    // B2
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName("B2"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2(xContent2->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("BBC"), xRange2->getString());
    // B3 -- testing a collapsed bookmark
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("B3"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    uno::Reference<text::XText> xText(xRange3->getText( ), uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xNeighborhoodCursor(xText->createTextCursor( ), uno::UNO_QUERY);
    xNeighborhoodCursor->gotoRange(xRange3, false);
    xNeighborhoodCursor->goLeft(1, false);
    xNeighborhoodCursor->goRight(2, true);
    uno::Reference<text::XTextRange> xTextNeighborhood(xNeighborhoodCursor, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("AB"), xTextNeighborhood->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf65955, "tdf65955.odt")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("a"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("b"));
    // a
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("a"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    // b
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName("b"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2(xContent2->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("r"), xRange2->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf65955_2, "tdf65955_2.odt")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("test"));

    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName("test"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3(xContent3->getAnchor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("foo bar"), xRange3->getString());
}

DECLARE_OOXMLEXPORT_TEST(testChtOutlineNumberingOoxml, "chtoutline.docx")
{
    const sal_Unicode aExpectedPrefix[2] = { 0x7b2c, 0x0020 };
    const sal_Unicode aExpectedSuffix[2] = { 0x0020, 0x7ae0 };
    uno::Reference< text::XChapterNumberingSupplier > xChapterNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference< container::XIndexAccess> xLevels(xChapterNumberingSupplier->getChapterNumberingRules());
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix,aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix,SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_OOXMLEXPORT_TEST(mathtype, "mathtype.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This failed as the Model property was empty.
    auto xModel = getProperty< uno::Reference<lang::XServiceInfo> >(xEmbeddedObjects->getByIndex(0), "Model");
    CPPUNIT_ASSERT(xModel->supportsService("com.sun.star.formula.FormulaProperties"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf8255, "tdf8255.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a full-page-wide multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf87460, "tdf87460.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    // This was 0: endnote was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotes->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90611, "tdf90611.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was 11.
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(getParagraphOfText(1, xFootnoteText), "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89702, "tdf89702.docx")
{
    // Get the first paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aCharStyleName = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    // Make sure that the font name is Arial, this was Verdana.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(xStyle, "CharFontName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf86374, "tdf86374.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // btLr text direction was imported as FIX, it should be MIN to have enough space for the additionally entered paragraphs.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN, getProperty<sal_Int16>(xTableRows->getByIndex(0), "SizeType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf87924, "tdf87924.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // This was -270, the text rotation angle was set when it should not be rotated.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testIndents, "indents.docx")
{
    //expected left margin and first line indent values
    static const sal_Int32 indents[] =
    {
            0,     0,
        -2000,     0,
        -2000,  1000,
        -1000, -1000,
         2000, -1000
    };
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    size_t paraIndex = 0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            uno::Reference<beans::XPropertySet> const xPropertySet(xServiceInfo, uno::UNO_QUERY_THROW);
            sal_Int32 nIndent = 0;
            sal_Int32 nFirstLine = 0;
            xPropertySet->getPropertyValue("ParaLeftMargin") >>= nIndent;
            xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nFirstLine;
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2], nIndent);
            CPPUNIT_ASSERT_EQUAL(indents[paraIndex * 2 + 1], nFirstLine);
            ++paraIndex;
        }
    } while (xParaEnum->hasMoreElements());
}

DECLARE_OOXMLEXPORT_TEST(testTdf92454, "tdf92454.docx")
{
    // The first paragraph had a large indentation / left margin as inheritance
    // in Word and Writer works differently, and no direct value was set to be
    // explicit.
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    // This was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95377, "tdf95377.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000), getProperty<sal_Int32>(xParagraph, "ParaRightMargin"));

    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-501), getProperty<sal_Int32>(xParagraph, "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));

    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-250), getProperty<sal_Int32>(xParagraph, "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(250), getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));

    //default style has numbering enabled.  Styles inherit numbering unless specifically disabled
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//body/txt/Special", 3);  //first three paragraphs have numbering
    assertXPath(pXmlDoc, "//body/txt[1]/Special", "rText", "a.");
    assertXPath(pXmlDoc, "//body/txt[2]/Special", "rText", "b.");
    assertXPath(pXmlDoc, "//body/txt[3]/Special", "rText", "c.");
    assertXPath(pXmlDoc, "/root/page/body/txt[4]/Special", 0); //last paragraph style disables numbering
}

DECLARE_OOXMLEXPORT_TEST(testTdf95376, "tdf95376.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(2), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE: indentation-from-numbering
    // did not have priority over indentation-from-paragraph-style, due to a
    // filter workaround that's not correct here.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE, xParagraph->getPropertyState("ParaFirstLineIndent"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92124, "tdf92124.docx")
{
    // Get the second paragraph's numbering style's 1st level's suffix.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aSuffix = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "Suffix"; })->Value.get<OUString>();
    // Make sure it's empty as the source document contains <w:suff w:val="nothing"/>.
    CPPUNIT_ASSERT(aSuffix.isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90153, "tdf90153.docx")
{
    // This was at-para, so the line-level VertOrientRelation was lost, resulting in an incorrect vertical position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf93919, "tdf93919.docx")
{
    // This was 0, left margin was not inherited from the list style.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf91417, "tdf91417.docx")
{
    // The first paragraph should contain a link to "http://www.google.com/"
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor(xTextDocument->getText()->createTextCursor( ), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCursorProps(xTextCursor, uno::UNO_QUERY);
    OUString aValue;
    xCursorProps->getPropertyValue("HyperLinkURL") >>= aValue;
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.google.com/"), aValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf90810, "tdf90810short.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc(xFootnoteSupp->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    rtl::OUString sFootnoteText = xFootnoteText->getString();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(89), sFootnoteText.getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf89165, "tdf89165.docx")
{
    // This must not hang in layout
}

DECLARE_OOXMLEXPORT_TEST(testTdf95777, "tdf95777.docx")
{
    // This must not fail on open
}

DECLARE_OOXMLEXPORT_TEST(testTdf94374, "hello.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    // This failed: it wasn't possible to insert a DOCX document into an existing Writer one.
    CPPUNIT_ASSERT(paste("tdf94374.docx", xEnd));
}

DECLARE_OOXMLEXPORT_TEST(testTdf83300, "tdf83300.docx")
{
    // This was 'Contents Heading', which (in the original document) implied 'keep with next' on unexpected paragraphs.
    CPPUNIT_ASSERT_EQUAL(OUString("TOC Heading"), getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf78902, "tdf78902.docx")
{
    // This hung in layout.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf95775, "tdf95775.docx")
{
    // This must not fail in layout
}

DECLARE_OOXMLEXPORT_TEST(testTdf92157, "tdf92157.docx")
{
    // A graphic with dimensions 0,0 should not fail on load
}

DECLARE_OOXMLEXPORT_TEST(testTdf97417, "section_break_numbering.docx")
{
    uno::Reference<beans::XPropertySet> xProps(getParagraph(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("1st page: first paragraph erroneous numbering",
        !xProps->getPropertyValue("NumberingRules").hasValue());
    // paragraph with numbering and section break was removed by writerfilter
    // but its numbering was copied to all following paragraphs
    CPPUNIT_ASSERT_MESSAGE("2nd page: first paragraph missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules").is());
    xProps = uno::Reference<beans::XPropertySet>(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("2nd page: second paragraph erroneous numbering",
        !xProps->getPropertyValue("NumberingRules").hasValue());

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf94043, "tdf94043.docx")
{
    auto xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    auto xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), getProperty<sal_Int32>(xTextColumns, "SeparatorLineWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95213, "tdf95213.docx")
{
    // Get the second paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aName = std::find_if(aLevel.begin(), aLevel.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "CharStyleName"; })->Value.get<OUString>();

    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aName), uno::UNO_QUERY);
    // This was awt::FontWeight::BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xStyle, "CharWeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97371, "tdf97371.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pShape = pPage->GetObj(0);
    SdrObject* pTextBox = pPage->GetObj(1);
    long nDiff = std::abs(pShape->GetSnapRect().Top() - pTextBox->GetSnapRect().Top());
    // The top of the two shapes were 410 and 3951, now it should be 3950 and 3951.
    CPPUNIT_ASSERT(nDiff < 10);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99140, "tdf99140.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1: a multi-page floating table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTableProperties(xTables->getByIndex(1), uno::UNO_QUERY);
    // This was text::HoriOrientation::NONE, the second table was too wide due to this.
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT_AND_WIDTH, getProperty<sal_Int16>(xTableProperties, "HoriOrient"));
}

DECLARE_OOXMLEXPORT_TEST( testTableCellMargin, "table-cell-margin.docx" )
{
    sal_Int32 const cellLeftMarginFromOffice[] = { 250, 100, 0, 0 };

    uno::Reference< text::XTextTablesSupplier > xTablesSupplier( mxComponent, uno::UNO_QUERY );
    uno::Reference< frame::XModel >             xModel( mxComponent, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >   xTables( xTablesSupplier->getTextTables(), uno::UNO_QUERY );

    for ( int i = 0; i < 4; i++ )
    {
        uno::Reference< text::XTextTable > xTable1( xTables->getByIndex( i ), uno::UNO_QUERY );

        // Verify left margin of 1st cell :
        //  * Office left margins are measured relative to the right of the border
        //  * LO left spacing is measured from the center of the border
        uno::Reference< table::XCell > xCell = xTable1->getCellByName( "A1" );
        uno::Reference< beans::XPropertySet > xPropSet( xCell, uno::UNO_QUERY_THROW );
        sal_Int32 aLeftMargin = -1;
        xPropSet->getPropertyValue( "LeftBorderDistance" ) >>= aLeftMargin;
        uno::Any aLeftBorder = xPropSet->getPropertyValue( "LeftBorder" );
        table::BorderLine2 aLeftBorderLine;
        aLeftBorder >>= aLeftBorderLine;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Incorrect left spacing computed from docx cell margin",
            cellLeftMarginFromOffice[i], aLeftMargin - 0.5 * aLeftBorderLine.LineWidth, 1 );
        // The 'a' in the fourth table should not be partly hidden by the border
        if ( i == 3 )
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Incorrect cell padding", 0.5 * aLeftBorderLine.LineWidth, aLeftMargin, 1 );
    }
}

// tdf#106742 for DOCX with compatibility level <= 14 (MS Word up to and incl. ver.2010), we should use cell margins when calculating table left border position
DECLARE_OOXMLEXPORT_TEST( testTablePosition14, "table-position-14.docx" )
{
    sal_Int32 const aXCoordsFromOffice[] = { 2500, -1000, 0, 0 };

    uno::Reference< text::XTextTablesSupplier > xTablesSupplier( mxComponent, uno::UNO_QUERY );
    uno::Reference< frame::XModel >             xModel( mxComponent, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >   xTables( xTablesSupplier->getTextTables(), uno::UNO_QUERY );

    for ( int i = 0; i < 4; i++ )
    {
        uno::Reference< text::XTextTable > xTable1( xTables->getByIndex( i ), uno::UNO_QUERY );

        // Verify X coord
        uno::Reference< view::XSelectionSupplier > xCtrl( xModel->getCurrentController(), uno::UNO_QUERY );
        xCtrl->select( uno::makeAny( xTable1 ) );
        uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupplier( xCtrl, uno::UNO_QUERY );
        uno::Reference< text::XTextViewCursor > xCursor( xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY );
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Incorrect X coord computed from docx",
            aXCoordsFromOffice[i], pos.X, 1 );
    }
}

// tdf#106742 for DOCX with compatibility level > 14 (MS Word since ver.2013), we should NOT use cell margins when calculating table left border position
DECLARE_OOXMLEXPORT_TEST( testTablePosition15, "table-position-15.docx" )
{
    sal_Int32 const aXCoordsFromOffice[] = { 2751, -899, 1, 106 };

    uno::Reference< text::XTextTablesSupplier > xTablesSupplier( mxComponent, uno::UNO_QUERY );
    uno::Reference< frame::XModel >             xModel( mxComponent, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >   xTables( xTablesSupplier->getTextTables(), uno::UNO_QUERY );

    for ( int i = 0; i < 4; i++ )
    {
        uno::Reference< text::XTextTable > xTable1( xTables->getByIndex( i ), uno::UNO_QUERY );

        // Verify X coord
        uno::Reference< view::XSelectionSupplier > xCtrl( xModel->getCurrentController(), uno::UNO_QUERY );
        xCtrl->select( uno::makeAny( xTable1 ) );
        uno::Reference< text::XTextViewCursorSupplier > xTextViewCursorSupplier( xCtrl, uno::UNO_QUERY );
        uno::Reference< text::XTextViewCursor > xCursor( xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY );
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( "Incorrect X coord computed from docx",
            aXCoordsFromOffice[i], pos.X, 1 );
    }
}

DECLARE_OOXMLEXPORT_TEST( testTdf107359, "tdf107359-char-pitch.docx" )
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

    bool bGridSnapToChars;
    xPropertySet->getPropertyValue("GridSnapToChars") >>= bGridSnapToChars;
    CPPUNIT_ASSERT_EQUAL( false, bGridSnapToChars );

    sal_Int32 nRubyHeight;
    xPropertySet->getPropertyValue("GridRubyHeight") >>= nRubyHeight;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), nRubyHeight );

    sal_Int32 nBaseHeight;
    xPropertySet->getPropertyValue("GridBaseHeight") >>= nBaseHeight;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(convertTwipToMm100(18 * 20)), nBaseHeight );

    sal_Int32 nBaseWidth;
    xPropertySet->getPropertyValue("GridBaseWidth") >>= nBaseWidth;
    CPPUNIT_ASSERT_EQUAL( sal_Int32(convertTwipToMm100(24 * 20)), nBaseWidth );
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
