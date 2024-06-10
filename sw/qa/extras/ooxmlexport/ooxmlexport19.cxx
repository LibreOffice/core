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
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <tools/UnitConversion.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <svx/svdpage.hxx>

#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <flyfrms.hxx>
#include <sortedobjs.hxx>
#include <txtfrm.hxx>
#include <IDocumentMarkAccess.hxx>
#include <xmloff/odffields.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testFdo78939, "fdo78939.docx")
{
    // fdo#78939 : LO hanged while opening issue document

    // Whenever a para-style was applied to a Numbering format level,
    // LO incorrectly also changed the para-style...

    // check that file opens and does not hang while opening and also
    // check that an incorrect numbering style is not applied ...
    CPPUNIT_ASSERT_EQUAL(OUString(),
                         getProperty<OUString>(getParagraph(1), u"NumberingStyleName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFootnote, "footnote.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xFootnote(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aFootnote = xFootnote->getString();
    // Ensure there are no additional newlines after "bar".
    CPPUNIT_ASSERT(aFootnote.endsWith("bar"));
}

DECLARE_OOXMLEXPORT_TEST(testTableBtlrCenter, "table-btlr-center.docx")
{
    // Note that this is btLr text, so layout and doc model horizontal/vertical is the opposite of
    // each other.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A2"_ustr);
    // Cell vertical alignment was CENTER, should be NONE.
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::NONE,
                         getProperty<sal_Int16>(xCell, u"VertOrient"_ustr));

    // Cell horizontal alignment should be CENTER.
    uno::Reference<text::XText> xCellText(xCell, uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int32>(getParagraphOfText(1, xCellText), u"ParaAdjust"_ustr);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(style::ParagraphAdjust_CENTER), nActual);
}

DECLARE_OOXMLEXPORT_TEST(testFdo80555, "fdo80555.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);
    // Shape was wrongly placed at X=0, Y=0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3318), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(247), xShape->getPosition().Y);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104418)
{
    loadAndReload("tdf104418.odt");
    // Problem was that <w:hideMark> cell property was ignored.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();

    // Without the fix in place, this test would have failed with
    // - Expected: 750
    // - Actual  : 1499
    CPPUNIT_ASSERT_EQUAL(sal_Int64(750),
                         getProperty<sal_Int64>(xTableRows->getByIndex(0), u"Height"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testHidemark, "hidemark.docx")
{
    // Problem was that <w:hideMark> cell property was ignored.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // Height should be minimal
    CPPUNIT_ASSERT_EQUAL(convertTwipToMm100(MINLAY),
                         getProperty<sal_Int64>(xTableRows->getByIndex(1), u"Height"_ustr));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xTableRows->getByIndex(1), u"SizeType"_ustr));

    //tdf#104876: Width was not recognized during import when table size was 'auto'
    CPPUNIT_ASSERT_MESSAGE("table size is less than 7000?",
                           sal_Int32(7000) > getProperty<sal_Int32>(xTextTable, u"Width"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testHidemarkb, "tdf99616_hidemarkb.docx")
{
    // Problem was that the smallest possible height was forced, not the min specified size.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
    // Height should be .5cm
    CPPUNIT_ASSERT_EQUAL(sal_Int64(501),
                         getProperty<sal_Int64>(xTableRows->getByIndex(1), u"Height"_ustr));
    // Size type was MIN, should be FIX to avoid considering the end of paragraph marker.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX,
                         getProperty<sal_Int16>(xTableRows->getByIndex(1), u"SizeType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testBnc891663, "bnc891663.docx")
{
    // The image should be inside a cell, so the text in the following cell should be below it.
    int imageTop
        = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds"_ostr,
                    "top"_ostr)
              .toInt32();
    int imageHeight
        = parseDump("/root/page/body/tab/row[1]/cell[2]/txt[1]/anchored/fly/infos/bounds"_ostr,
                    "height"_ostr)
              .toInt32();
    int textNextRowTop
        = parseDump("/root/page/body/tab/row[2]/cell[1]/txt[1]/infos/bounds"_ostr, "top"_ostr)
              .toInt32();
    CPPUNIT_ASSERT(textNextRowTop >= imageTop + imageHeight);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123104, "tdf123104.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"E1"_ustr);
    // See SwXCell::getPropertyValue(), we really put 'long' into an Any there.
    // Without the accompanying fix in place, this test would have failed with 'Expected: 3;Actual :
    // 2', i.e. the vertical merge covered one less cell, resulting in a cell with white background.
    CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(3),
                         getProperty<tools::Long>(xCell, u"RowSpan"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testFdo85542, "fdo85542.docx")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"B1"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"B2"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"B3"_ustr));
    // B1
    uno::Reference<text::XTextContent> xContent1(xBookmarksByName->getByName(u"B1"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange1 = xContent1->getAnchor();
    CPPUNIT_ASSERT_EQUAL(u"ABB"_ustr, xRange1->getString());
    // B2
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName(u"B2"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2 = xContent2->getAnchor();
    CPPUNIT_ASSERT_EQUAL(u"BBC"_ustr, xRange2->getString());
    // B3 -- testing a collapsed bookmark
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName(u"B3"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3 = xContent3->getAnchor();
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    uno::Reference<text::XText> xText = xRange3->getText();
    uno::Reference<text::XTextCursor> xNeighborhoodCursor = xText->createTextCursor();
    xNeighborhoodCursor->gotoRange(xRange3, false);
    xNeighborhoodCursor->goLeft(1, false);
    xNeighborhoodCursor->goRight(2, true);
    CPPUNIT_ASSERT_EQUAL(u"AB"_ustr, xNeighborhoodCursor->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf65955)
{
    loadAndReload("tdf65955.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"a"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"b"_ustr));
    // a
    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName(u"a"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3 = xContent3->getAnchor();
    CPPUNIT_ASSERT_EQUAL(xRange3->getString(), OUString());
    // b
    uno::Reference<text::XTextContent> xContent2(xBookmarksByName->getByName(u"b"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange2 = xContent2->getAnchor();
    CPPUNIT_ASSERT_EQUAL(u"r"_ustr, xRange2->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf65955_2)
{
    loadAndReload("tdf65955_2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"test"_ustr));

    uno::Reference<text::XTextContent> xContent3(xBookmarksByName->getByName(u"test"_ustr),
                                                 uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xRange3 = xContent3->getAnchor();
    CPPUNIT_ASSERT_EQUAL(u"foo bar"_ustr, xRange3->getString());
}

DECLARE_OOXMLEXPORT_TEST(testChtOutlineNumberingOoxml, "chtoutline.docx")
{
    static constexpr OUStringLiteral aExpectedNumbering = u"\u7b2c 1 \u7ae0";

    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedNumbering),
                         getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(mathtype, "mathtype.docx")
{
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xTextEmbeddedObjectsSupplier(mxComponent,
                                                                                    uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEmbeddedObjects(
        xTextEmbeddedObjectsSupplier->getEmbeddedObjects(), uno::UNO_QUERY);
    // This failed as the Model property was empty.
    auto xModel = getProperty<uno::Reference<lang::XServiceInfo>>(xEmbeddedObjects->getByIndex(0),
                                                                  u"Model"_ustr);
    CPPUNIT_ASSERT(xModel->supportsService(u"com.sun.star.formula.FormulaProperties"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf8255)
{
    auto verify = [this]() {
        // A full-page-wide multi-page floating table should be allowed to split:
        uno::Reference<text::XTextFramesSupplier> xDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFrame(
            xDocument->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
        bool bIsSplitAllowed{};
        xFrame->getPropertyValue(u"IsSplitAllowed"_ustr) >>= bIsSplitAllowed;
        CPPUNIT_ASSERT(bIsSplitAllowed);
    };
    createSwDoc("tdf8255.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

DECLARE_OOXMLEXPORT_TEST(testTdf87460, "tdf87460.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    // This was 0: endnote was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xEndnotes->getCount());

    // Also make sure that <w:tblpPr> is mapped to a text frame (hosting the table):
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTextFrames = xTextFramesSupplier->getTextFrames();
    // Without the fix in place, this test would have failed, the table was inline.
    CPPUNIT_ASSERT(xTextFrames->hasByName(u"Frame1"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf90611, "tdf90611.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was 11.
    CPPUNIT_ASSERT_EQUAL(
        10.f, getProperty<float>(getParagraphOfText(1, xFootnoteText), u"CharHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89702, "tdf89702.docx")
{
    // Get the first paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, u"NumberingRules"_ustr);
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aCharStyleName = std::find_if(std::cbegin(aLevel), std::cend(aLevel),
                                           [](const beans::PropertyValue& rValue) {
                                               return rValue.Name == "CharStyleName";
                                           })
                                  ->Value.get<OUString>();

    // Make sure that the font name is Arial, this was Verdana.
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(aCharStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xStyle, u"CharFontName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf86374, "tdf86374.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    // btLr text direction was imported as FIX, it should be MIN to have enough space for the additionally entered paragraphs.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::MIN,
                         getProperty<sal_Int16>(xTableRows->getByIndex(0), u"SizeType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf87924, "tdf87924.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(
        xPropertySet->getPropertyValue(u"CustomShapeGeometry"_ustr));
    // This was -270, the text rotation angle was set when it should not be rotated.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         aGeometry[u"TextPreRotateAngle"_ustr].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testIndents, "indents.docx")
{
    //expected left margin and first line indent values
    static const sal_Int32 indents[] = { 0, 0, -2000, 0, -2000, 1000, -1000, -1000, 2000, -1000 };
    uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(),
                                                                  uno::UNO_QUERY);
    // list of paragraphs
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    size_t paraIndex = 0;
    do
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo;
        if (xParaEnum->nextElement() >>= xServiceInfo)
        {
            uno::Reference<beans::XPropertySet> const xPropertySet(xServiceInfo,
                                                                   uno::UNO_QUERY_THROW);
            sal_Int32 nIndent = 0;
            sal_Int32 nFirstLine = 0;
            xPropertySet->getPropertyValue(u"ParaLeftMargin"_ustr) >>= nIndent;
            xPropertySet->getPropertyValue(u"ParaFirstLineIndent"_ustr) >>= nFirstLine;
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
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState(u"ParaFirstLineIndent"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95377, "tdf95377.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1000),
                         getProperty<sal_Int32>(xParagraph, u"ParaRightMargin"_ustr));

    xParagraph.set(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-501),
                         getProperty<sal_Int32>(xParagraph, u"ParaFirstLineIndent"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2501),
                         getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState(u"ParaFirstLineIndent"_ustr));

    xParagraph.set(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-250),
                         getProperty<sal_Int32>(xParagraph, u"ParaFirstLineIndent"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(250),
                         getProperty<sal_Int32>(xParagraph, u"ParaLeftMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE,
                         xParagraph->getPropertyState(u"ParaFirstLineIndent"_ustr));

    //default style has numbering enabled.  Styles inherit numbering unless specifically disabled
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc,
                "//body/txt/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,
                3); //first three paragraphs have numbering
    assertXPath(
        pXmlDoc,
        "//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,
        "expand"_ostr, u"a."_ustr);
    assertXPath(
        pXmlDoc,
        "//body/txt[2]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,
        "expand"_ostr, u"b."_ustr);
    assertXPath(
        pXmlDoc,
        "//body/txt[3]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,
        "expand"_ostr, u"c."_ustr);
    assertXPath(
        pXmlDoc,
        "/root/page/body/txt[4]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr,
        0); //last paragraph style disables numbering
}

DECLARE_OOXMLEXPORT_TEST(testTdf95376, "tdf95376.docx")
{
    uno::Reference<beans::XPropertyState> xParagraph(getParagraph(2), uno::UNO_QUERY);
    // This was beans::PropertyState_DIRECT_VALUE: indentation-from-numbering
    // did not have priority over indentation-from-paragraph-style, due to a
    // filter workaround that's not correct here.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DEFAULT_VALUE,
                         xParagraph->getPropertyState(u"ParaFirstLineIndent"_ustr));

    //tdf#131321 - paragraph styles lost their numbering. Bullet+space inherits WWNum1 from Bullet
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Bullet+space"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(!(getProperty<OUString>(xStyle, u"NumberingStyleName"_ustr)).isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf92124, "tdf92124.docx")
{
    // Get the second paragraph's numbering style's 1st level's suffix.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, u"NumberingRules"_ustr);
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(0) >>= aLevel; // 1st level
    OUString aSuffix
        = std::find_if(std::cbegin(aLevel), std::cend(aLevel),
                       [](const beans::PropertyValue& rValue) { return rValue.Name == "Suffix"; })
              ->Value.get<OUString>();
    // Make sure it's empty as the source document contains <w:suff w:val="nothing"/>.
    CPPUNIT_ASSERT(aSuffix.isEmpty());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90153, "tdf90153.docx")
{
    // This was at-para, so the line-level VertOrientRelation was lost, resulting in an incorrect vertical position.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf93919, "tdf93919.docx")
{
    // This was 0, left margin was not inherited from the list style.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                         getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf91417, "tdf91417.docx")
{
    // The first paragraph should contain a link to "http://www.google.com/"
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextCursor> xTextCursor = xTextDocument->getText()->createTextCursor();
    uno::Reference<beans::XPropertySet> xCursorProps(xTextCursor, uno::UNO_QUERY);
    OUString aValue;
    xCursorProps->getPropertyValue(u"HyperLinkURL"_ustr) >>= aValue;
    CPPUNIT_ASSERT_EQUAL(u"http://www.google.com/"_ustr, aValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf90810, "tdf90810short.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XFootnotesSupplier> xFootnoteSupp(xTextDocument, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnoteIdxAcc = xFootnoteSupp->getFootnotes();
    uno::Reference<text::XFootnote> xFootnote(xFootnoteIdxAcc->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xFootnoteText(xFootnote, uno::UNO_QUERY);
    OUString sFootnoteText = xFootnoteText->getString();
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

CPPUNIT_TEST_FIXTURE(Test, testTdf94374)
{
    createSwDoc("hello.docx");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    // This failed: it wasn't possible to insert a DOCX document into an existing Writer one.
    paste(u"ooxmlexport/data/tdf94374.docx", u"com.sun.star.comp.Writer.WriterFilter"_ustr, xEnd);
}

DECLARE_OOXMLEXPORT_TEST(testTdf83300, "tdf83300.docx")
{
    // tdf#143722: This was 'TOC Heading', upon updating a TOC the heading replaced its Word-default blue + Calibri style
    // with a Writer-default black + Liberation Sans one
    CPPUNIT_ASSERT_EQUAL(u"Contents Heading"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
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

    // Additionally, the bookmark names should not change (they got a "1" appended when copied)
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"referentiegegevens"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"referentiegegevens_bk"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97417, "section_break_numbering.docx")
{
    uno::Reference<beans::XPropertySet> xProps(getParagraph(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("1st page: first paragraph erroneous numbering",
                           !xProps->getPropertyValue(u"NumberingRules"_ustr).hasValue());
    // paragraph with numbering and section break was removed by writerfilter
    // but its numbering was copied to all following paragraphs
    CPPUNIT_ASSERT_MESSAGE("2nd page: first paragraph missing numbering",
                           getProperty<uno::Reference<container::XIndexAccess>>(
                               getParagraph(2), u"NumberingRules"_ustr)
                               .is());
    xProps = uno::Reference<beans::XPropertySet>(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("2nd page: second paragraph erroneous numbering",
                           !xProps->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf94043, "tdf94043.docx")
{
    auto xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), u"TextSection"_ustr);
    auto xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(xTextColumns, u"SeparatorLineWidth"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95213, "tdf95213.docx")
{
    // Get the second paragraph's numbering style's 2nd level's character style name.
    uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
    auto xLevels
        = getProperty<uno::Reference<container::XIndexAccess>>(xParagraph, u"NumberingRules"_ustr);
    uno::Sequence<beans::PropertyValue> aLevel;
    xLevels->getByIndex(1) >>= aLevel; // 2nd level
    OUString aName = std::find_if(std::cbegin(aLevel), std::cend(aLevel),
                                  [](const beans::PropertyValue& rValue) {
                                      return rValue.Name == "CharStyleName";
                                  })
                         ->Value.get<OUString>();

    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"CharacterStyles"_ustr)->getByName(aName),
                                               uno::UNO_QUERY);
    // This was awt::FontWeight::BOLD.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xStyle, u"CharWeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf97371, "tdf97371.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pShape = pPage->GetObj(0);
    SdrObject* pTextBox = pPage->GetObj(1);
    tools::Long nDiff = std::abs(pShape->GetSnapRect().Top() - pTextBox->GetSnapRect().Top());
    // The top of the two shapes were 410 and 3951, now it should be 3950 and 3951.
    CPPUNIT_ASSERT(nDiff < 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(tools::Long(3900), pShape->GetSnapRect().Top(), 100);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99140)
{
    auto verify = [this]() {
        // A multi-page floating table appeared only on the first page.
        SwDoc* pDoc = getSwDoc();
        SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
        auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
        CPPUNIT_ASSERT(pPage1);
        const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
        auto pPage1Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage1Objs[0]);
        CPPUNIT_ASSERT(pPage1Fly);
        SwFrame* pTab1 = pPage1Fly->GetLower();
        // This was text::HoriOrientation::NONE, the second table was too wide due to this.
        CPPUNIT_ASSERT_EQUAL(static_cast<tools::Long>(9622), pTab1->getFrameArea().Width());
        SwFrame* pRow1 = pTab1->GetLower();
        SwFrame* pCell1 = pRow1->GetLower();
        auto pText1 = dynamic_cast<SwTextFrame*>(pCell1->GetLower());
        CPPUNIT_ASSERT(pText1);
        CPPUNIT_ASSERT_EQUAL(u"Table2:A1"_ustr, pText1->GetText());

        auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
        CPPUNIT_ASSERT(pPage2);
        const SwSortedObjs& rPage2Objs = *pPage2->GetSortedObjs();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage2Objs.size());
        auto pPage2Fly = dynamic_cast<SwFlyAtContentFrame*>(rPage2Objs[0]);
        CPPUNIT_ASSERT(pPage2Fly);
        SwFrame* pTab2 = pPage2Fly->GetLower();
        SwFrame* pRow2 = pTab2->GetLower();
        SwFrame* pCell2 = pRow2->GetLower();
        auto pText2 = dynamic_cast<SwTextFrame*>(pCell2->GetLower());
        CPPUNIT_ASSERT(pText2);
        CPPUNIT_ASSERT_EQUAL(u"Table2:A2"_ustr, pText2->GetText());
    };
    createSwDoc("tdf99140.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTableMarginAdjustment)
{
    loadAndReload("table.fodt");
    // Writer, (new) Word: margin 0 means table border starts at 0
    // (old) Word: margin 0 means paragraph in table starts at 0

    auto const xTable(getParagraphOrTable(1));
    // shifted very slightly to account for half of the thin border width, so 4, not 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));

    // Now that compatibilityMode is set to 2013's 15 (new), expect the new values,
    // since LO is exporting in the NEW way now instead of the OLD way.
    // This was 55 when using 2007's compatibilityMode of 12 (old)

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "//w:tbl[1]/w:tblPr[1]/w:tblInd[1]"_ostr, "type"_ostr, u"dxa"_ustr);
    assertXPath(pXmlDoc, "//w:tbl[1]/w:tblPr[1]/w:tblInd[1]"_ostr, "w"_ostr, u"0"_ustr);

    // tdf#143982: automatic tables should export as something better than just left-and-size
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xTable, u"RelativeWidth"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf119760_tableInTablePosition, "tdf119760_tableInTablePosition.docx")
{
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

        assertXPath(pXmlDoc, "//w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]/w:tblPr[1]/w:tblInd[1]"_ostr,
                    "type"_ostr, u"dxa"_ustr);
        assertXPath(pXmlDoc, "//w:tbl[1]/w:tr[1]/w:tc[1]/w:tbl[1]//w:tblPr[1]/w:tblInd[1]"_ostr,
                    "w"_ostr, u"0"_ustr);
    }

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // For compatibilityMode 15: margin 0 means table border starts at 0,
    // shifted to account for half of the thick border width, so 106, not 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(106), getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTableCellMargin, "table-cell-margin.docx")
{
    sal_Int32 const cellLeftMarginFromOffice[] = { 250, 100, 0, 0 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify left margin of 1st cell :
        //  * Office left margins are measured relative to the right of the border
        //  * LO left spacing is measured from the center of the border
        uno::Reference<table::XCell> xCell = xTable1->getCellByName(u"A1"_ustr);
        uno::Reference<beans::XPropertySet> xPropSet(xCell, uno::UNO_QUERY_THROW);
        sal_Int32 aLeftMargin = -1;
        xPropSet->getPropertyValue(u"LeftBorderDistance"_ustr) >>= aLeftMargin;
        uno::Any aLeftBorder = xPropSet->getPropertyValue(u"LeftBorder"_ustr);
        table::BorderLine2 aLeftBorderLine;
        aLeftBorder >>= aLeftBorderLine;
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Incorrect left spacing computed from docx cell margin", cellLeftMarginFromOffice[i],
            aLeftMargin - 0.5 * aLeftBorderLine.LineWidth, 1);
        // The 'a' in the fourth table should not be partly hidden by the border
        if (i == 3)
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect cell left padding",
                                                 0.5 * aLeftBorderLine.LineWidth, aLeftMargin, 1);
            // tdf#119885: cell's edit area must touch right border
            sal_Int32 aRightMargin = -1;
            xPropSet->getPropertyValue(u"RightBorderDistance"_ustr) >>= aRightMargin;
            uno::Any aRightBorder = xPropSet->getPropertyValue(u"RightBorder"_ustr);
            table::BorderLine2 aRightBorderLine;
            aRightBorder >>= aRightBorderLine;
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect cell right padding",
                                                 0.5 * aRightBorderLine.LineWidth, aRightMargin, 1);
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, TestPuzzleExport)
{
    loadAndReload("TestPuzzleExport.odt");
    // See tdf#148342 for details
    // Get the doc
    uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY_THROW);
    auto pSwDoc = dynamic_cast<SwXTextDocument*>(xTextDoc.get());
    CPPUNIT_ASSERT(pSwDoc);
    // Create a metafile
    auto pMeta = pSwDoc->GetDocShell()->GetPreviewMetaFile();
    CPPUNIT_ASSERT(pMeta);
    MetafileXmlDump aDumper;
    auto pMetaXml = dumpAndParse(aDumper, *pMeta);
    CPPUNIT_ASSERT(pMetaXml);
    // After parsing check that node...
    auto pXNode = getXPathNode(
        pMetaXml, "/metafile/push/push/push/push[6]/push/push/polypolygon/polygon"_ostr);
    CPPUNIT_ASSERT(pXNode);
    auto pNode = pXNode->nodesetval->nodeTab[0];
    CPPUNIT_ASSERT(pNode);
    auto it = pNode->children;
    int nCount = 0;
    // .. and count the children
    while (it != nullptr)
    {
        nCount++;
        it = it->next;
    }
    // In case of puzzle there will be so many... Without the fix there was a rectangle with 4 points.
    CPPUNIT_ASSERT_GREATER(300, nCount);
}

// tdf#106742 for DOCX with compatibility level <= 14 (MS Word up to and incl. ver.2010), we should use cell margins when calculating table left border position
DECLARE_OOXMLEXPORT_TEST(testTablePosition14, "table-position-14.docx")
{
    sal_Int32 const aXCoordsFromOffice[] = { 2500, -1000, 0, 0 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify X coord
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                       uno::UNO_QUERY);
        xCtrl->select(uno::Any(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                              uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor = xTextViewCursorSupplier->getViewCursor();
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
                                             aXCoordsFromOffice[i], pos.X, 1);
    }
}

// tdf#106742 for DOCX with compatibility level > 14 (MS Word since ver.2013),
// we should NOT use cell margins when calculating table left border position. But we do need to use border width.
DECLARE_OOXMLEXPORT_TEST(testTablePosition15, "table-position-15.docx")
{
    sal_Int32 const aXCoordsFromOffice[] = { 2751, -899, 1, 212 };

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    for (int i = 0; i < 4; i++)
    {
        uno::Reference<text::XTextTable> xTable1(xTables->getByIndex(i), uno::UNO_QUERY);

        // Verify X coord
        uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                       uno::UNO_QUERY);
        xCtrl->select(uno::Any(xTable1));
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                              uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursor> xCursor = xTextViewCursorSupplier->getViewCursor();
        awt::Point pos = xCursor->getPosition();
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Incorrect X coord computed from docx",
                                             aXCoordsFromOffice[i], pos.X, 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf107359, "tdf107359-char-pitch.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

    bool bGridSnapToChars;
    xPropertySet->getPropertyValue(u"GridSnapToChars"_ustr) >>= bGridSnapToChars;
    CPPUNIT_ASSERT_EQUAL(false, bGridSnapToChars);

    sal_Int32 nRubyHeight;
    xPropertySet->getPropertyValue(u"GridRubyHeight"_ustr) >>= nRubyHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nRubyHeight);

    sal_Int32 nBaseHeight;
    xPropertySet->getPropertyValue(u"GridBaseHeight"_ustr) >>= nBaseHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(18 * 20)), nBaseHeight);

    sal_Int32 nBaseWidth;
    xPropertySet->getPropertyValue(u"GridBaseWidth"_ustr) >>= nBaseWidth;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(24 * 20)), nBaseWidth);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77236_MissingSolidFill)
{
    loadAndSave("tdf77236_MissingSolidFill.docx");
    // tdf#77236: solidFill of VML shape was not exported if the colors of line and style were the same
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(
        pXmlDoc,
        "//mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wps:wsp/wps:spPr/a:ln/a:solidFill"_ostr,
        1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf105875_VmlShapeRotationWithFlip,
                         "tdf105875_VmlShapeRotationWithFlip.docx")
{
    // tdf#105875: check whether the rotation of the VML bezier shape is ok (with flip too)
    // TODO: fix export too
    if (isExported())
        return;

    {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             xPropertySet->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>());
    }

    {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(2), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            sal_Int32(220 * 100),
            xPropertySet->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>(), 1);
    }

    {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(3), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            sal_Int32(320 * 100),
            xPropertySet->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>(), 1);
    }

    {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(4), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            sal_Int32(140 * 100),
            xPropertySet->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>(), 1);
    }

    {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(5), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(
            sal_Int32(40 * 100),
            xPropertySet->getPropertyValue(u"RotateAngle"_ustr).get<sal_Int32>(), 1);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133363)
{
    loadAndSave("tdf133363.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // tdf#133363: remove extra auto space between first and second list elements
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[2]/w:pPr/w:spacing"_ostr,
                "before"_ostr, u"0"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc/w:p[3]/w:pPr/w:spacing"_ostr,
                "after"_ostr, u"0"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf138093, "tdf138093.docx")
{
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
        assertXPath(pXmlDoc, "//w:sdt"_ostr, 3);
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName(u"B1"_ustr);
        uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xCell, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs
            = xParagraphsAccess->createEnumeration();
        uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                                 uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);

        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);

        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
        CPPUNIT_ASSERT(bDate);
        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl,
                                                                                uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum
            = xContentControlEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(),
                                                           uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"2017"_ustr, xTextPortionRange->getString());
    }
    else
    {
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getAllMarksCount());

        ::sw::mark::IDateFieldmark* pFieldmark
            = dynamic_cast<::sw::mark::IDateFieldmark*>(*pMarkAccess->getAllMarksBegin());
        CPPUNIT_ASSERT(pFieldmark);
        CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());
        CPPUNIT_ASSERT_EQUAL(u"2017"_ustr, pFieldmark->GetContent());
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf131722, "tdf131722.docx")
{
    if (isExported())
    {
        xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
        assertXPath(pXmlDoc, "//w:sdt"_ostr, 4);
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
        uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xCell, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParagraphs
            = xParagraphsAccess->createEnumeration();
        uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                                 uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);

        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);

        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
        CPPUNIT_ASSERT(bDate);
        uno::Reference<container::XEnumerationAccess> xContentControlEnumAccess(xContentControl,
                                                                                uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentControlEnum
            = xContentControlEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xTextPortionRange(xContentControlEnum->nextElement(),
                                                           uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Enter a date here!"_ustr, xTextPortionRange->getString());
    }
    else
    {
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), pMarkAccess->getFieldmarksCount());

        for (auto aIter = pMarkAccess->getFieldmarksBegin();
             aIter != pMarkAccess->getFieldmarksEnd(); ++aIter)
        {
            ::sw::mark::IDateFieldmark* pFieldmark
                = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
            CPPUNIT_ASSERT(pFieldmark);
            CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());
            CPPUNIT_ASSERT_EQUAL(u"Enter a date here!"_ustr, pFieldmark->GetContent());
        }
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf155945)
{
    createSwDoc("tdf155945.docx");

    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());
    // Without a fix in place, this would fail with
    // - Expected: 0
    // - Actual  : 423
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133560)
{
    createSwDoc("lastEmptyLineWithDirectFormatting.docx");

    CPPUNIT_ASSERT_EQUAL(4, getParagraphs());
    // Without a fix in place, this would fail with
    // - Expected: 12
    // - Actual  : 48
    CPPUNIT_ASSERT_EQUAL(12.0f, getProperty<float>(getParagraph(4), u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150408_isLvl_RoundTrip)
{
    loadAndSave("listWithLgl.docx");

    // Second level's numbering should use Arabic numbers for first level reference
    auto xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"CH I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(u"Sect 1.01"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(u"CH II"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(u"Sect 2.01"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));

    xmlDocUniquePtr pXml = parseExport(u"word/numbering.xml"_ustr);
    assertXPath(pXml, "/w:numbering/w:abstractNum"_ostr); // Only one list
    // The second list level must keep the isLgl element
    assertXPath(pXml, "/w:numbering/w:abstractNum/w:lvl[2]/w:isLgl"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf156372, "tdf156372.doc")
{
    sal_Int32 nHeight
        = parseDump("//page[1]/header/tab/row[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    // Without a fix in place, this would fail with
    // - Expected: 847
    // - Actual  : 1327
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(847), nHeight, 5);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156548)
{
    // Given a document using two bookmarks with similar names longer than 40 characters
    loadAndReload("longBookmarkName.fodt");

    // After the export, the names must be no longer than 40 characters; they must be unique;
    // and the hyperlinks must use the same names, to still point to the correct targets:

    {
        // 1st  paragraph - hyperlink to 4th paragraph
        auto sURL = getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"#A_bookmark_name_longer_than_forty_charac"_ustr, sURL);
        // 4th paragraph - a bookmark
        auto xBookmark = getProperty<uno::Reference<container::XNamed>>(getRun(getParagraph(4), 1),
                                                                        u"Bookmark"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"A_bookmark_name_longer_than_forty_charac"_ustr,
                             xBookmark->getName());
    }

    {
        // 2nd  paragraph - hyperlink to 5th paragraph
        auto sURL = getProperty<OUString>(getRun(getParagraph(2), 1), u"HyperLinkURL"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"#A_bookmark_name_longer_than_forty_chara1"_ustr, sURL);
        // 5th paragraph - a bookmark
        auto xBookmark = getProperty<uno::Reference<container::XNamed>>(getRun(getParagraph(5), 1),
                                                                        u"Bookmark"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"A_bookmark_name_longer_than_forty_chara1"_ustr,
                             xBookmark->getName());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157136)
{
    // Given a document with two content controls - one block, one inline
    createSwDoc("tdf157136_TwoContentControls.docx");

    // Both of them must import with the correct character style

    {
        // 1st paragraph - block content control
        auto xRun = getRun(getParagraph(1), 1);
        auto xContentControl
            = getProperty<css::uno::Reference<css::text::XTextRange>>(xRun, u"ContentControl"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Click or tap here to enter text."_ustr,
                             xContentControl->getString());
        // Without the fix in place, this would fail with
        // - Expected: Placeholder Text
        // - Actual  :
        CPPUNIT_ASSERT_EQUAL(u"Placeholder Text"_ustr,
                             getProperty<OUString>(xRun, u"CharStyleName"_ustr));
    }

    {
        // 2nd paragraph - inline content control
        auto xRun = getRun(getParagraph(2), 1);
        auto xContentControl
            = getProperty<css::uno::Reference<css::text::XTextRange>>(xRun, u"ContentControl"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Click or tap here to enter text."_ustr,
                             xContentControl->getString());
        CPPUNIT_ASSERT_EQUAL(u"Placeholder Text"_ustr,
                             getProperty<OUString>(xRun, u"CharStyleName"_ustr));
    }

    // Test the same after round-trip
    saveAndReload(u"Office Open XML Text"_ustr);

    {
        // 1st paragraph - becomes inline content control after roundtrip
        auto xRun = getRun(getParagraph(1), 1);
        auto xContentControl
            = getProperty<css::uno::Reference<css::text::XTextRange>>(xRun, u"ContentControl"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Click or tap here to enter text."_ustr,
                             xContentControl->getString());
        CPPUNIT_ASSERT_EQUAL(u"Placeholder Text"_ustr,
                             getProperty<OUString>(xRun, u"CharStyleName"_ustr));
    }

    {
        // 2nd paragraph - inline content control
        auto xRun = getRun(getParagraph(2), 1);
        auto xContentControl
            = getProperty<css::uno::Reference<css::text::XTextRange>>(xRun, u"ContentControl"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Click or tap here to enter text."_ustr,
                             xContentControl->getString());
        CPPUNIT_ASSERT_EQUAL(u"Placeholder Text"_ustr,
                             getProperty<OUString>(xRun, u"CharStyleName"_ustr));
    }
}

//Tests for support of fallback for ECMA-376 1st ed to merge the title value with descr attribute
CPPUNIT_TEST_FIXTURE(Test, testTdf148952_2007)
{
    //Given a document with 1 image (with name, alt title and description field populated) in odt format
    createSwDoc("tdf148952.odt");

    OUString rFilterName = u"MS Word 2007 XML"_ustr;

    //Export it to MS word 2007(.docx) format
    saveAndReload(rFilterName);

    // Checks the number of images in the docx file
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + rFilterName.toUtf8();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 1, getShapes());

    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);

    // Check if alt title value is appended to description
    OUString descr;
    xImage->getPropertyValue(u"Description"_ustr) >>= descr;
    CPPUNIT_ASSERT_EQUAL(u"Black\nShapes"_ustr, descr);

    // Check if alt title field is empty
    OUString title;
    xImage->getPropertyValue(u"Title"_ustr) >>= title;
    CPPUNIT_ASSERT_EQUAL(u""_ustr, title);
}

//Tests for support of title attribute for ECMA-376 2nd ed and above
CPPUNIT_TEST_FIXTURE(Test, testTdf148952_2010)
{
    //Given a document with 1 image (with name, alt title and description field populated) in odt format
    createSwDoc("tdf148952.odt");

    OUString rFilterName = u"Office Open XML Text"_ustr;

    //Export it to MS word 2010-365 (.docx) format
    saveAndReload(rFilterName);

    // Checks the number of images in the docx file
    const OString sFailedMessage = OString::Concat("Failed on filter: ") + rFilterName.toUtf8();
    CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), 1, getShapes());

    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);

    // Check if description field contains same value after export
    OUString descr;
    xImage->getPropertyValue(u"Description"_ustr) >>= descr;
    CPPUNIT_ASSERT_EQUAL(u"Shapes"_ustr, descr);

    // Check if alt title field contains same value after export
    OUString title;
    xImage->getPropertyValue(u"Title"_ustr) >>= title;
    CPPUNIT_ASSERT_EQUAL(u"Black"_ustr, title);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
