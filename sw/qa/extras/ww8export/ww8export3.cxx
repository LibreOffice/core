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
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextField.hpp>
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
#include <formatlinebreak.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ww8export/data/"_ustr, u"MS Word 97"_ustr)
    {
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
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));

    // tdf#136983
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last printed date", sal_Int16(2009), xDPS->getDocumentProperties()->getPrintDate().Year);
}

DECLARE_WW8EXPORT_TEST(testTdf100961_fixedDateTime, "tdf100961_fixedDateTime.doc")
{
    // This should be a fixed date/time field, not the current time.
    getParagraph(1, u"05.01.19 04:06:08"_ustr);

    css::uno::Reference<css::text::XTextFieldsSupplier> xSupplier(mxComponent,
                                                                  css::uno::UNO_QUERY_THROW);
    auto xFieldsAccess(xSupplier->getTextFields());
    auto xFields(xFieldsAccess->createEnumeration());

    css::uno::Reference<css::uno::XInterface> xField(xFields->nextElement(), css::uno::UNO_QUERY);
    // Check fixed property was imported and date value was parsed correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xField, u"IsFixed"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xField, u"IsDate"_ustr));
    auto datetime = getProperty<css::util::DateTime>(xField, u"DateTimeValue"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(5), datetime.Day);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), datetime.Month);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2019), datetime.Year);

    xField.set(xFields->nextElement(), css::uno::UNO_QUERY);
    // Check fixed property was imported and time value was parsed correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xField, u"IsFixed"_ustr));
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xField, u"IsDate"_ustr));
    datetime = getProperty<css::util::DateTime>(xField, u"DateTimeValue"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(4), datetime.Hours);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(6), datetime.Minutes);
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), datetime.Seconds);
}

DECLARE_WW8EXPORT_TEST(testTdf147861_customField, "tdf147861_customField.doc")
{
    // These should each be specific values, not a shared DocProperty
    getParagraph(1, u"CustomEditedTitle"_ustr); // edited
    // A couple of \x0\x0 at the end of the import variable thwart an equality comparison
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith(" INSERT Custom Title here"));
    getParagraph(3, u"My Title"_ustr); // edited

    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"CustomEditedTitle"_ustr, xField->getPresentation(false));
    // The " (fixed)" part is unnecessary, but it must be consistent across a round-trip
    CPPUNIT_ASSERT_EQUAL(u"DocInformation:Title (fixed)"_ustr, xField->getPresentation(true));
}

DECLARE_WW8EXPORT_TEST(testTdf148380_createField, "tdf148380_createField.doc")
{
    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This should NOT be "Lorenzo Chavez", or a real date since the user hand-modified the result.
    CPPUNIT_ASSERT_EQUAL(u"Myself - that's who"_ustr, xField->getPresentation(false));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"yesterday at noon"_ustr, xField->getPresentation(false));
}

DECLARE_WW8EXPORT_TEST(testTdf148380_fldLocked, "tdf148380_fldLocked.doc")
{
    getParagraph(2, u"4/5/2022 4:29:00 PM"_ustr);
    getParagraph(4, u"1/23/4567 8:9:10 PM"_ustr);

    // Verify that these are fields, and not just plain text
    // (import only, since export thankfully just dumps these fixed fields as plain text
    if (isExported())
        return;
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    // This should NOT be updated at FILEOPEN to match the last modified time - it is locked.
    CPPUNIT_ASSERT_EQUAL(u"4/5/2022 4:29:00 PM"_ustr, xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(u"DocInformation:Modified (fixed)"_ustr, xField->getPresentation(true));
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1/23/4567 8:9:10 PM"_ustr, xField->getPresentation(false));
    CPPUNIT_ASSERT_EQUAL(u"DocInformation:Last printed (fixed)"_ustr, xField->getPresentation(true));
}

DECLARE_WW8EXPORT_TEST(testTdf138345_paraCharHighlight, "tdf138345_paraCharHighlight.doc")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(9), 1, u"A side benefit is that "_ustr), uno::UNO_QUERY_THROW);
    // Paragraph style paraNoCharBackground cancel paraCharBackground using COL_TRANSPARENT.
    // Before this fix, the paragraph was by default covered with a yellow CharHighlight.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_AUTO), getProperty<sal_Int32>(xRun, u"CharHighlight"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_AUTO), getProperty<sal_Int32>(xRun, u"CharBackColor"_ustr));

    xRun.set(getRun(getParagraph(9), 2), uno::UNO_QUERY_THROW);
    // Character style formatting must not contain a highlight setting at all.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_AUTO), getProperty<sal_Int32>(xRun, u"CharHighlight"_ustr));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_AUTO), getProperty<sal_Int32>(xRun, u"CharBackColor"_ustr));

    //tdf#118711 - don't explicitly specify the default page style at the beginning of the document
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(uno::Any(), xPara->getPropertyValue(u"PageDescName"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104596_wrapInHeaderTable, "tdf104596_wrapInHeaderTable.doc")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    sal_Int32 nRowHeight = getXPath(pXmlDoc, "//header/tab/row[1]/infos/bounds"_ostr, "height"_ostr).toInt32();
    // The fly is supposed to be no-wrap, so the text should come underneath it, not wrap-through,
    // thus making the row much higher. Before, height was 706. Now it is 1067.
    CPPUNIT_ASSERT_MESSAGE("Text must wrap under green box", nRowHeight > 1000);
}

DECLARE_WW8EXPORT_TEST(testGutterLeft, "gutter-left.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle;
    getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr) >>= xPageStyle;
    sal_Int32 nGutterMargin{};
    xPageStyle->getPropertyValue(u"GutterMargin"_ustr) >>= nGutterMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1270
    // - Actual  : 0
    // i.e. gutter margin was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), nGutterMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testGutterTop)
{
    createSwDoc("gutter-top.doc");
    saveAndReload(u"MS Word 97"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bGutterAtTop{};
    xSettings->getPropertyValue(u"GutterAtTop"_ustr) >>= bGutterAtTop;
    // Without the accompanying fix in place, this test would have failed, because the gutter was
    // at the left.
    CPPUNIT_ASSERT(bGutterAtTop);
}

DECLARE_WW8EXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.doc")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), u"NumberingRules"_ustr);
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_WW8EXPORT_TEST(testTdf128501, "tdf128501.doc")
{
    if (!isExported())
    {
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor = getShape(1);
        CPPUNIT_ASSERT_EQUAL(u"com.sun.star.drawing.CustomShape"_ustr, xShapeDescriptor->getShapeType());
    }
    else
    {
        uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor = getShape(1);
        // Without the fix in place, this test would have failed with
        // - Expected: FrameShape
        // - Actual  : com.sun.star.drawing.CustomShape
        CPPUNIT_ASSERT_EQUAL(u"FrameShape"_ustr, xShapeDescriptor->getShapeType());
    }
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue(u"NumberingType"_ustr, uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload(u"MS Word 97"_ustr);
    xFootnotesSupplier.set(mxComponent, uno::UNO_QUERY);
    sal_uInt16 nExpected = style::NumberingType::ARABIC_ZERO;
    auto nActual = getProperty<sal_uInt16>(xFootnotesSupplier->getFootnoteSettings(), u"NumberingType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. the numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    createSwDoc();
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue(u"NumberingType"_ustr, uno::Any(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance(u"com.sun.star.text.Footnote"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    saveAndReload(u"MS Word 97"_ustr);
    xFootnotesSupplier.set(mxComponent, uno::UNO_QUERY);
    sal_uInt16 nExpected = style::NumberingType::SYMBOL_CHICAGO;
    auto nActual = getProperty<sal_uInt16>(xFootnotesSupplier->getFootnoteSettings(), u"NumberingType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 63
    // - Actual  : 4
    // i.e. the numbering type was ARABIC, not SYMBOL_CHICAGO.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

DECLARE_WW8EXPORT_TEST(testHyperLinkURLSaving, "tdf120003.doc")
{
    OUString URL = getProperty<OUString>(getShape(1), u"HyperLinkURL"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expected: https://www.libreoffice.org/
    // - Actual  : tps://www.libreoffice.org/
    CPPUNIT_ASSERT_EQUAL(u"https://www.libreoffice.org/"_ustr, URL);
}

DECLARE_WW8EXPORT_TEST(testdf79553_lineNumbers, "tdf79553_lineNumbers.doc")
{
    bool bValue = false;
    sal_Int32 nValue = -1;

    uno::Reference< text::XTextDocument > xtextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference< text::XLineNumberingProperties > xLineProperties( xtextDocument, uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet = xLineProperties->getLineNumberingProperties();

    xPropertySet->getPropertyValue(u"IsOn"_ustr) >>= bValue;
    CPPUNIT_ASSERT_EQUAL(true, bValue);

    xPropertySet->getPropertyValue(u"Distance"_ustr) >>= nValue;
    CPPUNIT_ASSERT_MESSAGE("automatic distance", nValue > 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138302_restartNumbering)
{
    loadAndReload("tdf138302_restartNumbering.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));


    // tdf#143982: automatic tables should export as something better than just left-and-size
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT(getProperty<bool>(xTable, u"IsWidthRelative"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xTable, u"RelativeWidth"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf122429_header, "tdf122429_header.doc")
{
    uno::Reference<container::XNameAccess> pageStyles = getStyles(u"PageStyles"_ustr);
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName(u"Default Page Style"_ustr), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, u"HeaderIsOn"_ustr);
    CPPUNIT_ASSERT(headerIsOn);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122460_header)
{
    loadAndReload("tdf122460_header.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<container::XNameAccess> pageStyles = getStyles(u"PageStyles"_ustr);
    uno::Reference<style::XStyle> pageStyle(pageStyles->getByName(u"Default Page Style"_ustr), uno::UNO_QUERY);
    bool headerIsOn = getProperty<bool>(pageStyle, u"HeaderIsOn"_ustr);
    CPPUNIT_ASSERT(headerIsOn);
}

DECLARE_WW8EXPORT_TEST(testTdf139495_tinyHeader, "tdf139495_tinyHeader.doc")
{
    // In Word 2003, this is one page, but definitely not six pages.
    CPPUNIT_ASSERT(getPages() < 3);
}

DECLARE_WW8EXPORT_TEST(testTdf124937, "tdf124937.doc")
{
    // Check it doesn't crash at import time
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", true, getProperty<bool>(xSect, u"IsProtected"_ustr));
    xSect.set(xSections->getByIndex(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section4 is protected", false, getProperty<bool>(xSect, u"IsProtected"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf73056_cellMargins, "tdf73056_cellMargins.doc")
{
    uno::Reference< text::XTextTablesSupplier > xTablesSupplier( mxComponent, uno::UNO_QUERY );
    uno::Reference< container::XIndexAccess >   xTables( xTablesSupplier->getTextTables(), uno::UNO_QUERY );
    uno::Reference< text::XTextTable > xTable1( xTables->getByIndex( 0 ), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable1->getCellByName( u"B4"_ustr );

    // only the first cell with specific margins was processed, leaving the rest at table defaults. Was 0.
    uno::Reference< beans::XPropertySet > xPropSet( xCell, uno::UNO_QUERY_THROW );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "bottom cell spacing to contents",
        sal_Int32(101), getProperty<sal_Int32>(xPropSet, u"BottomBorderDistance"_ustr ) );
}

DECLARE_WW8EXPORT_TEST(testTdf135672_tableGrows, "tdf135672_tableGrows.doc")
{
    auto const xTable(getParagraphOrTable(1));
    // This would shift left every round-trip.  First time was -259, next -418.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-101), getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));

    // This would grow every round-trip. First time was 11118, next 11435
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10800), getProperty<sal_Int32>(xTable, u"Width"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79435_legacyInputFields)
{
    loadAndReload("tdf79435_legacyInputFields.docx");
    //using .docx input file to verify cross-format compatibility.
    uno::Reference<text::XFormField> xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(5), 3), u"Bookmark"_ustr);
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
    xParameters->getByName(u"Description"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"StatusHelp"_ustr, sTmp);
    //xParameters->getByName("Content") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("Camelcase"), sTmp);
    //xParameters->getByName("Format") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("TITLE CASE"), sTmp);

    sal_uInt16 nMaxLength = 0;
    xParameters->getByName(u"MaxLength"_ustr) >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Max Length", sal_uInt16(10), nMaxLength);

    // too bad this is based on character runs - just found try trial and error.
    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(6), 2), u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"calculated"_ustr, sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 2), u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"currentDate"_ustr, sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(7), 7), u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"currentTime"_ustr, sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 2), u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"number"_ustr, sTmp);

    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(8), 7), u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"date"_ustr, sTmp);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134264)
{
    loadAndReload("tdf134264.docx");
    // Without the fix in place, ADDRESSBLOCK fields would have been lost after RT
    CPPUNIT_ASSERT_EQUAL(u"MF"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"M19"_ustr, getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u"1815"_ustr, getParagraph(3)->getString());
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

    CPPUNIT_ASSERT_EQUAL(u"TextFieldStart"_ustr, getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    uno::Reference<container::XNamed> xBookmark(getProperty< uno::Reference<beans::XPropertySet> >(xPropertySet, u"Bookmark"_ustr), uno::UNO_QUERY_THROW);

    // Critical test: does TextField's bookmark name match cross-reference?
    const OUString& sTextFieldName( xBookmark->getName() );
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sTextFieldName, getProperty<OUString>(xPropertySet, u"SourceName"_ustr));

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    // TextFields should not be turned into real bookmarks.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xBookmarksByIdx->getCount());

    // The actual name isn't critical, but if it fails, it is worth asking why.
    CPPUNIT_ASSERT_EQUAL(u"Text1"_ustr, sTextFieldName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134948)
{
    loadAndReload("tdf134948.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Only 1 paragraph", 1, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132726)
{
    loadAndReload("tdf132726.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, u"What sentence has a yellow background? "_ustr);
    CPPUNIT_ASSERT_EQUAL( COL_AUTO, getProperty<Color>(xRun, u"CharBackColor"_ustr));

    xRun = getRun(getParagraph(1), 2, u"Why, this sentence of course"_ustr);
    CPPUNIT_ASSERT_EQUAL( COL_YELLOW, getProperty<Color>(xRun, u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127316_autoEscapement)
{
    loadAndReload("tdf127316_autoEscapement.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Normal text "_ustr), u"CharEscapement"_ustr), 0);
    // Negative escapements (subscripts) were decreasing by 1% every round-trip due to bad manual rounding.
    // This should be roughly .2*35% of the ORIGINAL (non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 7% (1.68pt) becomes roughly 10.5% for the 16pt font.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Subscript", -10.f, getProperty<float>(getRun(xPara, 2), u"CharEscapement"_ustr), 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127316_autoEscapement2)
{
    loadAndReload("tdf127316_autoEscapement2.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Base1"_ustr), u"CharEscapement"_ustr), 0);
    // Font is 80% of 40pt or 32pt, original escapement is 6.4pt, so round-trip escapement is 20%.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.f, getProperty<float>(getRun(xPara, 2,u"AutoSuperscript"_ustr), u"CharEscapement"_ustr), 1);
    xPara.set( getParagraph(3) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Base3"_ustr), u"CharEscapement"_ustr), 0);
    // font is 20% of 40pt or 8pt, original escapement is 25.6pt, so round-trip escapement is 320%.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(320.f, getProperty<float>(getRun(xPara, 2,u"AutoSuperscript"_ustr), u"CharEscapement"_ustr), 3);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf120412_proportionalEscapement)
{
    loadAndReload("tdf120412_proportionalEscapement.odt");
    uno::Reference<text::XTextRange> xPara = getParagraph(2);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 2, u"Base"_ustr), u"CharEscapement"_ustr), 0);
    // Import was limiting to 100%. And export based the position on the original height, not the proportional height.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.f, getProperty<float>(getRun(xPara, 3,u"Super"_ustr), u"CharEscapement"_ustr), 2);
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_WW8EXPORT_TEST(testTdf133453_realFontSize, "tdf133453_realFontSize.doc")
{
    CPPUNIT_ASSERT_EQUAL( -95.f, getProperty<float>(getRun(getParagraph(1), 2, u"2"_ustr), u"CharEscapement"_ustr) );
}

DECLARE_WW8EXPORT_TEST(testTdf116194, "tdf116194.doc")
{
    CPPUNIT_ASSERT_EQUAL( Color(0xc00000), getProperty<Color>(getRun(getParagraph(1), 1), u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121111_fillStyleNone)
{
    loadAndReload("tdf121111_fillStyleNone.docx");
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Numbering - First level"_ustr),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xb8cce4), getProperty<Color>(xStyle, u"ParaBackColor"_ustr));//R:184 G:204 B:228
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));

    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xText, u"ParaBackColor"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fill", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, u"FillStyle"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128608_fillStyleNoneB)
{
    loadAndReload("tdf128608_fillStyleNoneB.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextRange> xText(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xText, u"ParaBackColor"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fill", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, u"FillStyle"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf132094_transparentPageImage, "tdf132094_transparentPageImage.doc")
{
    uno::Reference<drawing::XShape> image (getShape(1), uno::UNO_QUERY);
    // Don't add fillstyle when none is set.
    // Well, ok, at least make it transparent if you do uselessly set a solid color...
    const bool bFillNone = drawing::FillStyle_NONE == getProperty<drawing::FillStyle>(image, u"FillStyle"_ustr);
    const bool bTransparent = sal_Int16(0) != getProperty<sal_Int16>(image, u"FillTransparence"_ustr);
    CPPUNIT_ASSERT_MESSAGE("no background fill", bTransparent || bFillNone);
}

DECLARE_WW8EXPORT_TEST(testTdf112618_textbox_no_bg, "tdf112618_textbox_no_bg.doc")
{
    Color nTransparence = getProperty<Color>(getShape(2), u"FillTransparence"_ustr);
    CPPUNIT_ASSERT_EQUAL(Color(0x000064), nTransparence);
    CPPUNIT_ASSERT_EQUAL(nTransparence, getProperty<Color>(getShape(2), u"BackColorTransparency"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf101826_xattrTextBoxFill, "tdf101826_xattrTextBoxFill.doc")
{
    //Basic 1 Color Fill: gradient from yellow(FFFF00) to brown(767600) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(1), u"BackColor"_ustr));
    //Basic 2 Color Fill: gradient from yellow(FFFF00) to green(00B050) currently saves as mid-color
    CPPUNIT_ASSERT_MESSAGE("background color", Color(0xFF, 0xFF, 0x00) != getProperty<Color>(getShape(4), u"BackColor"_ustr));
    //Basic Picture Fill: Tux image
    CPPUNIT_ASSERT_EQUAL_MESSAGE("background image", drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(getShape(5), u"FillStyle"_ustr));
    // Basic Pattern fill: many thin, green, vertical stripes on yellow background
    auto eMode = getProperty<drawing::BitmapMode>(getShapeByName(u"Frame2"), u"FillBitmapMode"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tiled pattern", drawing::BitmapMode_REPEAT, eMode);
    // Basic Texture fill: tiled blue denim texture
    eMode = getProperty<drawing::BitmapMode>(getShapeByName(u"Frame6"), u"FillBitmapMode"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tiled texture", drawing::BitmapMode_REPEAT, eMode);
}

DECLARE_WW8EXPORT_TEST(testTdf123433_fillStyleStop, "tdf123433_fillStyleStop.doc")
{
    uno::Reference<text::XTextRange> xText(getParagraph(12));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xText, u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xText, u"ParaBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127862_pageFillStyle)
{
    loadAndReload("tdf127862_pageFillStyle.odt");
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT(drawing::FillStyle_NONE != getProperty<drawing::FillStyle>(xStyle, u"FillStyle"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf128608_tableParaBackColor, "tdf128608_tableParaBackColor.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // ParaBackColor doesn't seem to be used in this case, but keep it here to make sure it stays as AUTO.
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xPara, u"ParaBackColor"_ustr));
    // No paragraph background colour/fill. (The cell background colour should be used.)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("No fillstyle", drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(xPara, u"FillStyle"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf117217_largeTableBackgrounds)
{
    loadAndReload("tdf117217_largeTableBackgrounds.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell 22: light-red == 16711680
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"V1"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("light red", Color(0xE0C2CD), getProperty<Color>(xCell, u"BackColor"_ustr));
    xCell.set(xTable->getCellByName(u"Z1"_ustr), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("light red", Color(0xE0C2CD), getProperty<Color>(xCell, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94009_zeroPgMargin)
{
    loadAndReload("tdf94009_zeroPgMargin.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(defaultStyle, u"TopMargin"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf108518_CRnumformatting, "tdf108518_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(u"6.2.3."_ustr, parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']"_ostr, "expand"_ostr));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(u"220"_ustr, parseDump("//body/txt[4]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']/SwFont"_ostr, "height"_ostr));
}

DECLARE_WW8EXPORT_TEST(testTdf120711_joinedParagraphWithChangeTracking, "tdf120711.doc")
{
    sal_Int16   numFormat = getNumberingTypeOfParagraph(5);
    // last paragraph is not a list item
    CPPUNIT_ASSERT(style::NumberingType::CHAR_SPECIAL != numFormat);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129522_removeShadowStyle)
{
    loadAndReload("tdf129522_removeShadowStyle.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference< container::XNameAccess > paragraphStyles = getStyles(u"ParagraphStyles"_ustr);
    uno::Reference< beans::XPropertySet > xStyleProps(paragraphStyles->getByName(u"Shadow"_ustr), uno::UNO_QUERY_THROW);
    table::ShadowFormat aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    // Shadows were inherited regardless of whether the style disabled them.
    xStyleProps.set(paragraphStyles->getByName(u"Shadow-removed"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"ParaShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);

    uno::Reference< container::XNameAccess > characterStyles = getStyles(u"CharacterStyles"_ustr);
    xStyleProps.set(characterStyles->getByName(u"CharShadow"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_BOTTOM_RIGHT, aShadow.Location);

    xStyleProps.set(characterStyles->getByName(u"CharShadow-removed"_ustr), uno::UNO_QUERY_THROW);
    aShadow = getProperty<table::ShadowFormat>(xStyleProps, u"CharShadowFormat"_ustr);
    CPPUNIT_ASSERT_EQUAL(table::ShadowLocation_NONE, aShadow.Location);
}

DECLARE_WW8EXPORT_TEST(testTdf81705_outlineLevel, "tdf81705_outlineLevel.doc")
{
    // direct formatting resets outline level to body text (0)
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph C", sal_uInt16(0), getProperty<sal_uInt16>(getParagraph(3), u"OutlineLevel"_ustr));
    // myStyle sets outline level to 1.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph D", sal_uInt16(1), getProperty<sal_uInt16>(getParagraph(4), u"OutlineLevel"_ustr));
}

DECLARE_WW8EXPORT_TEST(testBtlrCell, "btlr-cell.doc")
{
    // Without the accompanying fix in place, this test would have failed, as
    // the btlr text direction in the A1 cell was lost on DOC import and
    // export.
    uno::Reference<text::XTextTablesSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTables = xSupplier->getTextTables();
    uno::Reference<text::XTextTable> xTable(xTables->getByName(u"Table1"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR, getProperty<sal_Int16>(xA1, u"WritingMode"_ustr));

    uno::Reference<beans::XPropertySet> xB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    auto nActual = getProperty<sal_Int16>(xB1, u"WritingMode"_ustr);
    CPPUNIT_ASSERT(nActual == text::WritingMode2::LR_TB || nActual == text::WritingMode2::CONTEXT);

    uno::Reference<beans::XPropertySet> xC1(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, getProperty<sal_Int16>(xC1, u"WritingMode"_ustr));
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
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"HoriOrientPosition"_ustr) >>= nPosX);
    CPPUNIT_ASSERT(xShapeProps->getPropertyValue(u"VertOrientPosition"_ustr) >>= nPosY);
    // Allow some tolerance because rounding errors through integer arithmetic
    // in rotation.
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5200.0, static_cast<double>(nPosX), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1152.0, static_cast<double>(nPosY), 1.0);
}

DECLARE_WW8EXPORT_TEST(testImageCommentAtChar, "image-comment-at-char.doc")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment start before the image was lost.
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                         getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 5), u"TextPortionType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126708emf)
{
    loadAndReload("tdf126708_containsemf.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xShape = getShape(1);
    // First check the size of the EMF graphic contained in the shape.
    auto xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(
        xShape, u"Graphic"_ustr);
    auto xSize = getProperty<awt::Size>(xGraphic, u"Size100thMM"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8501), xSize.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(18939), xSize.Width);

    // Now check that the shape itself has a decent size.
    // This size varies slightly when round tripping through doc format.
    xSize = getProperty<awt::Size>(xShape, u"Size"_ustr);
    CPPUNIT_ASSERT(abs(xSize.Height - 7629) <= 6);
    CPPUNIT_ASSERT(abs(xSize.Width - 17000) <= 6);
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrame)
{
    loadAndReload("btlr-frame.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (!isExported())
    {
        return;
    }

    // Without the accompanying fix in place, this test would have failed with a
    // beans.UnknownPropertyException, as the writing direction was lost, so the default direction
    // resulted in a conversion to a Writer text frame.
    uno::Reference<beans::XPropertySet> xFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(9000), getProperty<sal_Int32>(xFrame, u"RotateAngle"_ustr));
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
        aShapeLineDash = getProperty<drawing::LineDash>(xShape, u"LineDash"_ustr);
        bool bIsEqual = aPresetLineDash.Style == aShapeLineDash.Style
                        && aPresetLineDash.Dots == aShapeLineDash.Dots
                        && aPresetLineDash.DotLen == aShapeLineDash.DotLen
                        && aPresetLineDash.Dashes == aShapeLineDash.Dashes
                        && aPresetLineDash.DashLen == aShapeLineDash.DashLen
                        && aPresetLineDash.Distance == aShapeLineDash.Distance;
        CPPUNIT_ASSERT_MESSAGE("LineDash differ", bIsEqual);
    }

    xmlDocUniquePtr pLayout = parseLayoutDump();
    // Ensure that there is no tabstop in the first paragraph (despite chapter numbering's setting)
    // This is a pre-emptive test to ensure something visibly correct is not broken.
    assertXPath(pLayout, "//body/txt[1]//SwFixPortion"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testRtlGutter)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xStandard(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(getProperty<bool>(xStandard, u"RtlGutter"_ustr));
    };

    // Given a document with RTL gutter, when loading it:
    createSwDoc("rtl-gutter.doc");
    // Then make sure the section's gutter is still RTL:
    // Without the accompanying fix in place, this test would have failed as the SPRM was missing.
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    verify();
}

DECLARE_WW8EXPORT_TEST(testTdf94326_notOutlineNumbering, "tdf94326_notOutlineNumbering.doc")
{
    // The directly applied numbering list must not be lost.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"ОБЩИЕ ПОЛОЖЕНИЯ"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf106541_cancelOutline, "tdf106541_cancelOutline.doc")
{
    // The ability to cancel numbering must not be lost.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"Cancelled by style"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(2, u"Cancelled by inherited style"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(4, u"Cancelled by direct paragraph formatting"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104239_chapterNumbering, "tdf104239_chapterNumbering.doc")
{
    uno::Reference<text::XChapterNumberingSupplier> xNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xNumberingRules = xNumberingSupplier->getChapterNumberingRules();

    comphelper::SequenceAsHashMap hashMap(xNumberingRules->getByIndex(0));
    CPPUNIT_ASSERT(hashMap[u"HeadingStyleName"_ustr].get<OUString>().match("Heading 1"));
    sal_uInt16 nNumberingType = style::NumberingType::CHARS_UPPER_LETTER_N;
    CPPUNIT_ASSERT_EQUAL(nNumberingType, hashMap[u"NumberingType"_ustr].get<sal_uInt16>());

    hashMap = xNumberingRules->getByIndex(5);
    CPPUNIT_ASSERT(hashMap[u"HeadingStyleName"_ustr].get<OUString>().match("Heading 6"));
    nNumberingType = style::NumberingType::ARABIC;
    CPPUNIT_ASSERT_EQUAL(nNumberingType, hashMap[u"NumberingType"_ustr].get<sal_uInt16>());
}

DECLARE_WW8EXPORT_TEST(testTdf106541_inheritChapterNumbering, "tdf106541_inheritChapterNumbering.doc")
{
    // The level and numbering are inherited from Heading 1.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(3, u"Letter A"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"a."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf106541_inheritChapterNumberingB, "tdf106541_inheritChapterNumberingB.doc")
{
    // The level and numbering are inherited from Heading 1.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"Chapter 1, level 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(2, u"Chapter 1, level 2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf75748_inheritChapterNumberingC, "tdf75748_inheritChapterNumberingC.doc")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(5, u"Inherited from Heading 3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"II.B.1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104239_numbering, "tdf104239_numbering.doc")
{
    // The paragraph starts with "paraksta Pieņemšanas". [Roundtrip by Word 2016 avoids the problem.]
    uno::Reference<beans::XPropertySet> xPara(getParagraph(51), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"3.3.1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104239_chapterNumberingLevels, "tdf104239_chapterNumberingLevels.doc")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Article I."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(2, u"Heading 2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(3, u"Heading 3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"First"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(4, u"Heading 4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(8, u"Heading 9"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1.1.1.1.1.1.1.1.1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104239_chapterNumberTortureTest, "tdf104239_chapterNumberTortureTest.doc")
{
    // There is no point in identifying what the wrong values where in this test,
    //because EVERYTHING was wrong, and MANY different fixes are required to solve the problems.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"No numId in style or paragraph"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(2, u"Paragraph cancels numbering(0)"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(3, u"First numbered line"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1st.i.a.1.I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(7, u"inheritOnly: inherit outlineLvl and listLvl."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2nd.ii"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr)); // Level 2
    xPara.set(getParagraph(9, u"outline with Body listLvl(9)."_ustr), uno::UNO_QUERY);
    if (!isExported())
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(10, u"outline with Body listLvl(9) #2."_ustr), uno::UNO_QUERY);
    if (!isExported())
        CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(11, u"direct formatting - Body listLvl(9)."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    xPara.set(getParagraph(12, u"direct numId, inherit listLvl."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"2nd.ii.a.1.I"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr)); // Level 5
    xPara.set(getParagraph(13, u"Style numId0 cancels inherited numbering."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf106541_inheritOutlineNumbering, "tdf106541_inheritOutlineNumbering.doc")
{
    // The level and numbering are inherited from Level2.
    uno::Reference<beans::XPropertySet> xPara(getParagraph(2, u"This should be a sub-point."_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf104239_sharedOutlineNumId, "tdf104239_sharedOutlineNumId.doc")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(5, u"Principes"_ustr), uno::UNO_QUERY);
    // This was ".1." previously.
    CPPUNIT_ASSERT_EQUAL(u"2.1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

DECLARE_WW8EXPORT_TEST(testTdf120394, "tdf120394.doc")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.1.2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(10), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_WW8EXPORT_TEST(testTdf142760, "tdf142760.doc")
{
    // Without the fix in place, this test would have failed with
    // - Expected: 2
    // - Actual  : 6
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(1, getShapes());

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

DECLARE_WW8EXPORT_TEST(testTdf134570, "tdf134570.doc")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136814)
{
    loadAndReload("tdf136814.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    sal_Int32 nBorderDistance = static_cast<sal_Int32>(106);

    CPPUNIT_ASSERT_EQUAL(nBorderDistance, getProperty<sal_Int32>(xStyle, u"TopBorderDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(nBorderDistance, getProperty<sal_Int32>(xStyle, u"RightBorderDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(nBorderDistance, getProperty<sal_Int32>(xStyle, u"BottomBorderDistance"_ustr));
    CPPUNIT_ASSERT_EQUAL(nBorderDistance, getProperty<sal_Int32>(xStyle, u"LeftBorderDistance"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79186_noLayoutInCell)
{
    loadAndReload("tdf79186_noLayoutInCell.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    CPPUNIT_ASSERT(!getProperty<bool>(getShape(1), u"IsFollowingTextFlow"_ustr));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), u"SurroundContour"_ustr)); // tdf#140508
}

CPPUNIT_TEST_FIXTURE(Test, testClearingBreak)
{
    auto verify = [this]() {
        uno::Reference<container::XEnumerationAccess> xParagraph(getParagraph(1), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
        xPortions->nextElement();
        xPortions->nextElement();
        // Without the accompanying fix in place, this test would have failed with:
        // An uncaught exception of type com.sun.star.container.NoSuchElementException
        // i.e. the first para was just a fly + text portion, the clearing break was lost.
        uno::Reference<beans::XPropertySet> xPortion(xPortions->nextElement(), uno::UNO_QUERY);
        OUString aPortionType;
        xPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"LineBreak"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xLineBreak;
        xPortion->getPropertyValue(u"LineBreak"_ustr) >>= xLineBreak;
        sal_Int16 eClear{};
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->getPropertyValue(u"Clear"_ustr) >>= eClear;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(SwLineBreakClear::ALL), eClear);
    };

    // Given a document with a clearing break:
    // When loading that file:
    createSwDoc("clearing-break.doc");
    // Then make sure that the clear property of the break is not ignored:
    verify();
    saveAndReload(u"MS Word 97"_ustr);
    // Make sure that the clear property of the break is not ignored during export:
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142840)
{
    loadAndReload("tdf142840.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // Ensure space are replaced by underscore in bookmark name (it was working before, but ensure this)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Chapter_1"_ustr));
    CPPUNIT_ASSERT(!xBookmarksByName->hasByName(u"Chapter 1"_ustr));

    // And hyperlink is referring bookmark with underscore also (this was broken)
    CPPUNIT_ASSERT_EQUAL(u"#Chapter_1"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"HyperLinkURL"_ustr));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
