/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/sequenceashashmap.hxx>

#include <IDocumentSettingAccess.hxx>
#include <docsh.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }
};

DECLARE_WW8EXPORT_TEST(testTdf37778_readonlySection, "tdf37778_readonlySection.doc")
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT( xStorable->isReadonly() );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // The problem was that section protection was being enabled in addition to being read-only.
    // This created an explicit section with protection. There should be just the default, non-explicit section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of Sections", sal_Int32(0), xSections->getCount());

    // tdf#127862: page fill color (in this case white) was lost
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf104596_wrapInHeaderTable, "tdf104596_wrapInHeaderTable.doc")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//header/tab/row[1]/infos/bounds", "height").toInt32();
    // The fly is supposed to be no-wrap, so the text should come underneath it, not wrap-through,
    // thus making the row much higher. Before, height was 706. Now it is 1067.
    CPPUNIT_ASSERT_MESSAGE("Text must wrap under green box", nRowHeight > 1000);
}

DECLARE_WW8EXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.doc")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("MS Word 97", "");
    xFootnotesSupplier.set(mxComponent, uno::UNO_QUERY);
    sal_uInt16 nExpected = style::NumberingType::ARABIC_ZERO;
    auto nActual = getProperty<sal_uInt16>(xFootnotesSupplier->getFootnoteSettings(), "NumberingType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. the numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("MS Word 97", "");
    xFootnotesSupplier.set(mxComponent, uno::UNO_QUERY);
    sal_uInt16 nExpected = style::NumberingType::SYMBOL_CHICAGO;
    auto nActual = getProperty<sal_uInt16>(xFootnotesSupplier->getFootnoteSettings(), "NumberingType");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 63
    // - Actual  : 4
    // i.e. the numbering type was ARABIC, not SYMBOL_CHICAGO.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

DECLARE_WW8EXPORT_TEST(testTdf59674_numberingTabStop, "tdf59674_numberingTabStop.odt")
{
    // This bug is mainly for MS Word. The round-trip looked fine in LO. Test exporting a non-useless value.
    auto xNum1Levels = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(2), "NumberingRules");
    sal_Int32 nTabPos2 = comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["ListtabStopPosition"].get<sal_Int32>();
    xNum1Levels.set(getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(3), "NumberingRules"));
    sal_Int32 nTabPos3 = comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["ListtabStopPosition"].get<sal_Int32>();

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1281), nTabPos2);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), nTabPos3);  //previously 640
}

DECLARE_WW8EXPORT_TEST(testdf79553_lineNumbers, "tdf79553_lineNumbers.doc")
{
    bool bValue = false;
    sal_Int32 nValue = -1;

    uno::Reference< text::XTextDocument > xtextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference< text::XLineNumberingProperties > xLineProperties( xtextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet = xLineProperties->getLineNumberingProperties();

    xPropertySet->getPropertyValue("IsOn") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue("Distance") >>= nValue;
    CPPUNIT_ASSERT_MESSAGE("automatic distance", nValue > 0);
}

DECLARE_WW8EXPORT_TEST(testTdf122429_header, "tdf122429_header.doc")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName("Default Page Style"), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);
}

DECLARE_WW8EXPORT_TEST(testTdf122460_header, "tdf122460_header.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName("Default Page Style"), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, "HeaderIsOn");
    CPPUNIT_ASSERT(headerIsOn);
}

DECLARE_WW8EXPORT_TEST(testFdo53985, "fdo53985.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xTables->getCount()); // Only 4 tables were imported.

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compatibility: Protect form", true, pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xSections->getCount()); // The first paragraph wasn't counted as a section.

    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", true, getProperty<bool>(xSect, "IsProtected"));
    xSect.set(xSections->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section4 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_WW8EXPORT_TEST(testTdf73056_cellMargins, "tdf73056_cellMargins.doc")
{
    uno::Reference< text::XTextTablesSupplier > xTablesSupplier( mxComponent, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >   xTables( xTablesSupplier->getTextTables(), uno::UNO_QUERY );
    uno::Reference< text::XTextTable > xTable1( xTables->getByIndex( 0 ), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable1->getCellByName( "B4" );

    // only the first cell with specific margins was processed, leaving the rest at table defaults. Was 0.
    uno::Reference< beans::XPropertySet > xPropSet( xCell, uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "bottom cell spacing to contents",
        sal_Int32(101), getProperty<sal_Int32>(xPropSet, "BottomBorderDistance" ) );
}

DECLARE_WW8EXPORT_TEST(testTdf135672_tableGrows, "tdf135672_tableGrows.doc")
{
    auto const xTable(getParagraphOrTable(1));
    // This would shift left every round-trip.  First time was -259, next -418.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-101), getProperty<sal_Int32>(xTable, "LeftMargin"));

    // This would grow every round-trip. First time was 11118, next 11435
    //CPPUNIT_ASSERT_EQUAL(sal_Int32(10800), getProperty<sal_Int32>(xTable, "Width"));
}

DECLARE_WW8EXPORT_TEST(testTdf79435_legacyInputFields, "tdf79435_legacyInputFields.docx")
{
    //using .docx input file to verify cross-format compatibility.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(5), 3), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());

    OUString sTmp;
    // Too often the string reader can fail during import - fix that first to prevent round-tripping garbage.
    // (for example BR-1010B.doc from tdf#48097)
    //xParameters->getByName("EntryMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    //xParameters->getByName("Help") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("F1Help"), sTmp);
    //xParameters->getByName("ExitMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    xParameters->getByName("Description") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("StatusHelp"), sTmp);
    //xParameters->getByName("Content") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("Camelcase"), sTmp);
    //xParameters->getByName("Format") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("TITLE CASE"), sTmp);

    sal_uInt16 nMaxLength = 0;
    xParameters->getByName("MaxLength") >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Max Length", sal_uInt16(10), nMaxLength);

    // too bad this is based on character runs - just found try trial and error.
    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(6), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("calculated"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentDate"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 7), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentTime"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("number"), sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 7), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("date"), sTmp);
}

DECLARE_WW8EXPORT_TEST(testTdf134264, "tdf134264.docx")
{
    // Without the fix in place, ADDRESSBLOCK fields would have been lost after RT
    CPPUNIT_ASSERT_EQUAL(OUString("MF"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("M19"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("1815"), getParagraph(3)->getString());
}

DECLARE_WW8EXPORT_TEST(testTdf120225_textControlCrossRef, "tdf120225_textControlCrossRef.doc")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"), getProperty<OUString>(xPropertySet, "TextPortionType"));
    uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xPropertySet, "Bookmark"), uno::UNO_QUERY_THROW);

    // Critical test: does TextField's bookmark name match cross-reference?
    const OUString& sTextFieldName( xBookmark->getName() );
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sTextFieldName, getProperty<OUString>(xPropertySet, "SourceName"));

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    // TextFields should not be turned into real bookmarks.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xBookmarksByIdx->getCount());

    // The actual name isn't critical, but if it fails, it is worth asking why.
    CPPUNIT_ASSERT_EQUAL(OUString("Text1"), sTextFieldName);
}

DECLARE_WW8EXPORT_TEST(testTdf134948, "tdf134948.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Only 1 paragraph", 1, getParagraphs());
}

DECLARE_WW8EXPORT_TEST(testTdf132726, "tdf132726.odt")
{
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, "What sentence has a yellow background? ");
    CPPUNIT_ASSERT_EQUAL( COL_AUTO, Color(getProperty<sal_uInt32>(xRun, "CharBackColor")) );

    xRun = getRun(getParagraph(1), 2, "Why, this sentence of course");
    CPPUNIT_ASSERT_EQUAL( COL_YELLOW, Color(getProperty<sal_uInt32>(xRun, "CharBackColor")) );
}

DECLARE_WW8EXPORT_TEST(testTdf127316_autoEscapement, "tdf127316_autoEscapement.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Normal text "), "CharEscapement"), 0);
    // Negative escapements (subscripts) were decreasing by 1% every round-trip due to bad manual rounding.
    // This should be roughly .2*35% of the ORIGINAL (non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 7% (1.68pt) becomes roughly 10.5% for the 16pt font.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Subscript", -10.f, getProperty<float>(getRun(xPara, 2), "CharEscapement"), 1);
}

DECLARE_WW8EXPORT_TEST(testTdf127316_autoEscapement2, "tdf127316_autoEscapement2.odt")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Base1"), "CharEscapement"), 0);
    // Font is 80% of 40pt or 32pt, original escapement is 6.4pt, so round-trip escapement is 20%.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.f, getProperty<float>(getRun(xPara, 2,"AutoSuperscript"), "CharEscapement"), 1);
    xPara.set( getParagraph(3) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Base3"), "CharEscapement"), 0);
    // font is 20% of 40pt or 8pt, original escapement is 25.6pt, so round-trip escapement is 320%.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(320.f, getProperty<float>(getRun(xPara, 2,"AutoSuperscript"), "CharEscapement"), 3);
}

DECLARE_WW8EXPORT_TEST(testTdf120412_proportionalEscapement, "tdf120412_proportionalEscapement.odt")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 2, "Base"), "CharEscapement"), 0);
    // Import was limiting to 100%. And export based the position on the original height, not the proportional height.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.f, getProperty<float>(getRun(xPara, 3,"Super"), "CharEscapement"), 2);
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_WW8EXPORT_TEST(testTdf133453_realFontSize, "tdf133453_realFontSize.doc")
{
    CPPUNIT_ASSERT_EQUAL( -95.f, getProperty<float>(getRun(getParagraph(1), 2, "2"), "CharEscapement") );
}

DECLARE_WW8EXPORT_TEST(testTdf116194, "tdf116194.doc")
{
    CPPUNIT_ASSERT_EQUAL( Color(192,0,0), Color(getProperty<sal_uInt32>(getRun(getParagraph(1), 1), "CharColor")) );
}

DECLARE_WW8EXPORT_TEST(testTdf121111_fillStyleNone, "tdf121111_fillStyleNone.docx")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Numbering - First level"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(184,204,228), Color(getProperty<sal_uInt32>(xStyle, "ParaBackColor")));//R:184 G:204 B:228
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle, "FillStyle"));

    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xText, "ParaBackColor")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fill", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf128608_fillStyleNoneB, "tdf128608_fillStyleNoneB.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xText(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xText, "ParaBackColor")));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fill", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf132094_transparentPageImage, "tdf132094_transparentPageImage.doc")
{
    uno::Reference<drawing::XShape> image (getShape(1), uno::UNO_QUERY);
    // Don't add fillstyle when none is set.
    // Well, ok, at least make it transparent if you do uselessly set a solid color...
    const bool bFillNone = drawing::FillStyle_NONE == getProperty<drawing::FillStyle>(image, "FillStyle");
    const bool bTransparent = sal_Int16(0) != getProperty<sal_Int16>(image, "FillTransparence");
    CPPUNIT_ASSERT_MESSAGE("no background fill", bTransparent || bFillNone);
}

DECLARE_WW8EXPORT_TEST(testTdf112618_textbox_no_bg, "tdf112618_textbox_no_bg.doc")
{
    sal_uInt16 nTransparence = getProperty<sal_Int16>(getShape(2), "FillTransparence");
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(100), nTransparence);
    CPPUNIT_ASSERT_EQUAL(nTransparence, getProperty<sal_uInt16>(getShape(2), "BackColorTransparency"));
}

DECLARE_WW8EXPORT_TEST(testTdf101826_xattrTextBoxFill, "tdf101826_xattrTextBoxFill.doc")
{
    //Basic 1 Color Fill: gradient from yellow(FFFF00) to brown(767600) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(1), "BackColor"));
    //Basic 2 Color Fill: gradient from yellow(FFFF00) to green(00B050) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(4), "BackColor"));
    //Basic Picture Fill: Tux image
    CPPUNIT_ASSERT_EQUAL_MESSAGE("background image", drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(getShape(5), "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf123433_fillStyleStop, "tdf123433_fillStyleStop.doc")
{
    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xText, "ParaBackColor")));
}

DECLARE_WW8EXPORT_TEST(testTdf127862_pageFillStyle, "tdf127862_pageFillStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf128608_tableParaBackColor, "tdf128608_tableParaBackColor.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A4"), uno::UNO_QUERY);

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // ParaBackColor doesn't seem to be used in this case, but keep it here to make sure it stays as AUTO.
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xPara, "ParaBackColor")));
    // No paragraph background colour/fill. (The cell background colour should be used.)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fillstyle", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xPara, "FillStyle"));
}

DECLARE_WW8EXPORT_TEST(testTdf117217_largeTableBackgrounds, "tdf117217_largeTableBackgrounds.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell 22: light-red == 16711680
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("V1"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("light red", sal_Int32(0xE0C2CD), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("Z1"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("light red", sal_Int32(0xE0C2CD), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_WW8EXPORT_TEST(testTdf94009_zeroPgMargin, "tdf94009_zeroPgMargin.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(defaultStyle, "TopMargin"));
}

DECLARE_WW8EXPORT_TEST(testTdf120711_joinedParagraphWithChangeTracking, "tdf120711.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(5);
    // last paragraph is not a list item
    CPPUNIT_ASSERT(style::NumberingType::CHAR_SPECIAL != numFormat);
}

DECLARE_WW8EXPORT_TEST(testTdf129522_removeShadowStyle, "tdf129522_removeShadowStyle.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles("ParagraphStyles");
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName("Shadow"), uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName("Shadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "ParaShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference< container::XNameAccess > characterStyles = getStyles("CharacterStyles");
    xStyleProps.set(characterStyles->getByName("CharShadow"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName("CharShadow-removed"), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, "CharShadowFormat");
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_WW8EXPORT_TEST(testBtlrCell, "btlr-cell.doc")
{
    // Without the accompanying fix in place, this test would have failed, as
    // the btlr text direction in the A1 cell was lost on DOC import and
    // export.
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName("Table1"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xA1, "WritingMode"));

    uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName("B1"), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xB1, "WritingMode");
    CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB || nActual == text::WritingMode2::CONTEXT);

    uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName("C1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, getProperty<sal_Int16>(xC1, "WritingMode"));
}

DECLARE_WW8EXPORT_TEST(testTdf118375export, "tdf118375_240degClockwise.doc")
{
    // The input document has one custom shape, which is rotated 240deg. Check
    // that it has the same position as in Word.
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get XDrawPagesSupplier", xDrawPagesSupplier.is());
    uno::Reference<drawing::XDrawPages> xDrawPages(xDrawPagesSupplier->getDrawPages());
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Could not get xDrawPage", xDrawPage.is());
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get xShape", xShape.is());
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Could not get the shape properties", xShapeProps.is());
    sal_Int32 nPosX = {}, nPosY = {};
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("HoriOrientPosition") >>= nPosX);
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue("VertOrientPosition") >>= nPosY);
    // Allow some tolerance because rounding errors through integer arithmetic
    // in rotation.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5200.0, static_cast<double>(nPosX), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1152.0, static_cast<double>(nPosY), 1.0);
}

DECLARE_WW8EXPORT_TEST(testImageCommentAtChar, "image-comment-at-char.doc")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment start before the image was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));
}

DECLARE_WW8EXPORT_TEST(testTdf126708emf, "tdf126708_containsemf.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xShape = getShape(1);
    // First check the size of the EMF graphic contained in the shape.
    auto xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(
        xShape, "Graphic");
    auto xSize = getProperty<awt::Size>(xGraphic, "Size100thMM");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8501), xSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18939), xSize.Width);

    // Now check that the shape itself has a decent size.
    // This size varies slightly when round tripping through doc format.
    xSize = getProperty<awt::Size>(xShape, "Size");
    CPPUNIT_ASSERT(abs(xSize.Height - 7629) <= 6);
    CPPUNIT_ASSERT(abs(xSize.Width - 17000) <= 6);
}

DECLARE_WW8EXPORT_TEST(testBtlrFrame, "btlr-frame.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (!mbExported)
    {
        return;
    }

    // Without the accompanying fix in place, this test would have failed with a
    // beans.UnknownPropertyException, as the writing direction was lost, so the default direction
    // resulted in a conversion to a Writer text frame.
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), getProperty<sal_Int32>(xFrame, "RotateAngle"));
}

DECLARE_WW8EXPORT_TEST(testPresetDash, "tdf127166_prstDash_Word97.doc")
{
    // Error was, that the 'sys' preset dash styles were neither imported not
    // exported, the mixed styles had wrong dash-dot order, they were imported
    // with absolute values.
    const drawing::LineDash dashParams[] =
    {
        {drawing::DashStyle_RECTRELATIVE, 1, 400, 0, 0, 300}, // dash
        {drawing::DashStyle_RECTRELATIVE, 1, 400, 1, 100, 300}, // dashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 100, 0, 0, 300}, // dot
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 0, 0, 300}, // lgDash
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 1, 100, 300}, // lgDashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 800, 2, 100, 300}, // lgDashDotDot
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 0, 0, 100}, // sysDash
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 1, 100, 100}, // sysDashDot
        {drawing::DashStyle_RECTRELATIVE, 1, 300, 2, 100, 100}, // sysDashDotDot
        {drawing::DashStyle_RECTRELATIVE, 1, 100, 0, 0, 100} // sysDot
    };
    drawing::LineDash aPresetLineDash;
    drawing::LineDash aShapeLineDash;
    for (sal_uInt16 i = 0; i < 10; i++)
    {
        aPresetLineDash = dashParams[i];
        uno::Reference<drawing::XShape> xShape = getShape(i+1);
        aShapeLineDash = getProperty<drawing::LineDash>(xShape, "LineDash");
        bool bIsEqual = aPresetLineDash.Style == aShapeLineDash.Style
                        && aPresetLineDash.Dots == aShapeLineDash.Dots
                        && aPresetLineDash.DotLen == aShapeLineDash.DotLen
                        && aPresetLineDash.Dashes == aShapeLineDash.Dashes
                        && aPresetLineDash.DashLen == aShapeLineDash.DashLen
                        && aPresetLineDash.Distance == aShapeLineDash.Distance;
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsEqual);
    }
}

DECLARE_WW8EXPORT_TEST(testTdf120394, "tdf120394.doc")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(CHAR_ZWSP), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(CHAR_ZWSP), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(10), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString(CHAR_ZWSP), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_WW8EXPORT_TEST(testTdf134570, "tdf134570.doc")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
