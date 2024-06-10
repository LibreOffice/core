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
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>

#include <tools/UnitConversion.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/scopeguard.hxx>

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

CPPUNIT_TEST_FIXTURE(Test, testFdo85889pc)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        CPPUNIT_ASSERT_EQUAL(u"\u00B1\u2265\u2264"_ustr, xTextRange->getString());
    };
    createSwDoc("fdo85889-pc.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo85889pca)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        CPPUNIT_ASSERT_EQUAL(u"\u00B1\u2017\u00BE"_ustr, xTextRange->getString());
    };
    createSwDoc("fdo85889-pca.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo85889mac)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        CPPUNIT_ASSERT_EQUAL(u"\u00D2\u00DA\u00DB"_ustr, xTextRange->getString());
    };
    createSwDoc("fdo85889-mac.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo72031)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(u"\uF0C5"_ustr, getRun(getParagraph(1), 1)->getString());
    };

    AllSettings aSavedSettings = Application::GetSettings();
    AllSettings aSettings(aSavedSettings);
    aSettings.SetLanguageTag(LanguageTag(u"ru"_ustr));
    Application::SetSettings(aSettings);
    comphelper::ScopeGuard g([&aSavedSettings] { Application::SetSettings(aSavedSettings); });

    createSwDoc("fdo72031.rtf");
    verify();
    saveAndReload(u"Rich Text Format"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo86750)
{
    auto verify = [this]() {
        // This was 'HYPERLINK#anchor', the URL of the hyperlink had the field type as a prefix, leading to broken links.
        CPPUNIT_ASSERT_EQUAL(u"#anchor"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1),
                                                                    u"HyperLinkURL"_ustr));
    };
    createSwDoc("fdo86750.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88811)
{
    auto verify = [this]() {
        // The problem was that shapes anchored to the paragraph that is moved into a textframe were lost, so this was 2.
        CPPUNIT_ASSERT_EQUAL(4, getShapes());
    };
    createSwDoc("tdf88811.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49893_2)
{
    auto verify = [this]() {
        // Ensure that header text exists on each page (especially on second page)
        CPPUNIT_ASSERT_EQUAL(u"HEADER"_ustr, parseDump("/root/page[1]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"HEADER"_ustr, parseDump("/root/page[2]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"HEADER"_ustr, parseDump("/root/page[3]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"HEADER"_ustr, parseDump("/root/page[4]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(u"HEADER"_ustr, parseDump("/root/page[5]/header/txt/text()"_ostr));
        CPPUNIT_ASSERT_EQUAL(5, getPages()); // Word has 5
    };
    createSwDoc("fdo49893-2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo89496)
{
    auto verify = [this]() {
        // Just ensure that document is loaded and shape exists
        uno::Reference<drawing::XShape> xShape = getShape(1);
        CPPUNIT_ASSERT(xShape.is());
    };
    createSwDoc("fdo89496.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75614)
{
    auto verify = [this]() {
        // Text after the footnote was missing, so this resulted in a css::container::NoSuchElementException.
        CPPUNIT_ASSERT_EQUAL(u"after."_ustr, getRun(getParagraph(1), 3)->getString());
    };
    createSwDoc("tdf75614.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, mathtype)
{
    auto verify = [this]() {
        OUString aFormula = getFormula(getRun(getParagraph(1), 1));
        CPPUNIT_ASSERT(!aFormula.isEmpty());
    };
    createSwDoc("mathtype.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86182)
{
    auto verify = [this]() {
        // Writing mode was the default, i.e. text::WritingMode2::CONTEXT.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB,
                             getProperty<sal_Int16>(getParagraph(1), u"WritingMode"_ustr));
    };
    createSwDoc("tdf86182.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf91074)
{
    auto verify = [this]() {
        // The file failed to load, as the border color was imported using the LineColor UNO property.
        uno::Reference<drawing::XShape> xShape = getShape(1);
        CPPUNIT_ASSERT_EQUAL(
            COL_LIGHTRED, Color(ColorTransparency,
                                getProperty<table::BorderLine2>(xShape, u"TopBorder"_ustr).Color));
    };
    createSwDoc("tdf91074.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90260Nopar)
{
    createSwDoc("hello.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/tdf90260-nopar.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf86814)
{
    auto verify = [this]() {
        // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold (applied paragraph style with direct formatting).
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    };
    createSwDoc("tdf86814.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108505_fieldCharFormat)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());

        // Character formatting can be defined inside the field for part of it. It wasn't being applied.
        // Bold and green are specified. \fldrslt's "bogus result" (with italic/red) should be ignored.
        uno::Reference<text::XTextRange> xRun = getRun(xPara, 3, u"MZ"_ustr);
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xRun, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(xRun, u"CharColor"_ustr));

        CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, getProperty<OUString>(xPara, u"ParaStyleName"_ustr));
    };
    createSwDoc("tdf108505_fieldCharFormat.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108505_fieldCharFormat2)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"C1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xPara = getParagraphOfText(1, xCell->getText());

        const sal_Int32 nRun = isExported() ? 6 : 5;
        const Color aColor = isExported() ? COL_BLACK : COL_AUTO;

        // Character formatting should only be defined by the \fldrslt, and not by prior formatting.
        // Prior formatting is italic, red, 20pt.
        uno::Reference<text::XTextRange> xRun = getRun(xPara, nRun, u"xyz"_ustr);
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xRun, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
        CPPUNIT_ASSERT_EQUAL(aColor, getProperty<Color>(xRun, u"CharColor"_ustr));
    };
    createSwDoc("tdf108505_fieldCharFormat2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

/** Make sure that the document variable "Unused", which is not referenced in the document,
    is imported and exported. */
CPPUNIT_TEST_FIXTURE(Test, testTdf150267)
{
    auto verify = [this]() {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextFieldsSupplier> xSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xTextFieldMasters = xSupplier->getTextFieldMasters();
        CPPUNIT_ASSERT_EQUAL(sal_True, xTextFieldMasters->hasByName(
                                           u"com.sun.star.text.fieldmaster.User.Unused"_ustr));

        auto xFieldMaster
            = xTextFieldMasters->getByName(u"com.sun.star.text.fieldmaster.User.Unused"_ustr);
        CPPUNIT_ASSERT_EQUAL(u"Hello World"_ustr,
                             getProperty<OUString>(xFieldMaster, u"Content"_ustr));
    };
    createSwDoc("tdf150267.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf151370)
{
    auto verify = [this]() {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextFieldsSupplier> xSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xTextFieldMasters = xSupplier->getTextFieldMasters();
        // Here we try to read/write docvar having non-ascii name and value. So it is encoded in Unicode
        OUString sFieldName(
            u"com.sun.star.text.fieldmaster.User."
            "LocalChars\u00c1\u0072\u0076\u00ed\u007a\u0074\u0075\u0072\u006f\u0054"
            "\u00fc\u006b\u00f6\u0072\u0066\u00fa\u0072\u00f3\u0067\u00e9\u0070"_ustr);
        CPPUNIT_ASSERT_EQUAL(sal_True, xTextFieldMasters->hasByName(sFieldName));

        auto xFieldMaster = xTextFieldMasters->getByName(sFieldName);
        CPPUNIT_ASSERT_EQUAL(u"\u00e1\u0072\u0076\u00ed\u007a\u0074\u0075\u0072\u006f\u0074\u00fc"
                             "\u006b\u00f6\u0072\u0066\u00fa\u0072\u00f3\u0067\u00e9\u0070"_ustr,
                             getProperty<OUString>(xFieldMaster, u"Content"_ustr));
    };
    createSwDoc("tdf151370.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108416)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xCharacterStyles(getStyles(u"CharacterStyles"_ustr));
        uno::Reference<beans::XPropertySet> xListLabel(
            xCharacterStyles->getByName(u"ListLabel 1"_ustr), uno::UNO_QUERY);
        // This was awt::FontWeight::BOLD, list numbering got an unexpected bold formatting.
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(xListLabel, u"CharWeight"_ustr));
    };
    createSwDoc("tdf108416.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testBinSkipping)
{
    auto verify = [this]() {
        // before, it was importing '/nMUST NOT IMPORT'
        CPPUNIT_ASSERT_EQUAL(u"text"_ustr, getRun(getParagraph(1), 1)->getString());
    };
    createSwDoc("bin-skipping.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92061)
{
    auto verify = [this]() {
        // This was "C", i.e. part of the footnote ended up in the body text.
        CPPUNIT_ASSERT_EQUAL(u"body-after"_ustr, getRun(getParagraph(1), 3)->getString());
    };
    createSwDoc("tdf92061.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92481)
{
    auto verify = [this]() {
        // This was 0, RTF_WIDOWCTRL was not imported.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(2),
                             getProperty<sal_Int8>(getParagraph(1), u"ParaWidows"_ustr));
    };
    createSwDoc("tdf92481.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94456)
{
    auto verify = [this]() {
        // Paragraph left margin and first line indent wasn't imported correctly.

        // This was 1270.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(762),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
        // This was -635.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-762),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaFirstLineIndent"_ustr));
    };
    createSwDoc("tdf94456.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94435)
{
    auto verify = [this]() {
        // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc.
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                             static_cast<style::ParagraphAdjust>(
                                 getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
    };
    createSwDoc("tdf94435.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf54584)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        // \PAGE was ignored, so no fields were in document -> exception was thrown
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(
            "No fields in document found: field \"\\PAGE\" was not properly read",
            xFields->nextElement());
    };
    createSwDoc("tdf54584.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96308Deftab)
{
    auto verify = [this]() {
        uno::Reference<lang::XMultiServiceFactory> xTextFactory(mxComponent, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDefaults(
            xTextFactory->createInstance(u"com.sun.star.text.Defaults"_ustr), uno::UNO_QUERY);
        // This was 1270 as \deftab was ignored on import.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(284)),
                             getProperty<sal_Int32>(xDefaults, u"TabStopDistance"_ustr));
    };
    createSwDoc("tdf96308-deftab.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testLandscape)
{
    auto verify = [this]() {
        // Check landscape flag.
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        // All pages should have flag orientation
        uno::Reference<container::XNameAccess> pageStyles = getStyles(u"PageStyles"_ustr);

        // get a page cursor
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
            xModel->getCurrentController(), uno::UNO_QUERY);
        uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                                  uno::UNO_QUERY);

        // check that the first page has landscape flag
        xCursor->jumpToFirstPage();
        OUString pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
        uno::Reference<style::XStyle> xStylePage(pageStyles->getByName(pageStyleName),
                                                 uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, u"IsLandscape"_ustr));

        // check that the second page has landscape flag
        xCursor->jumpToPage(2);
        pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
        xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, u"IsLandscape"_ustr));

        // check that the last page has landscape flag
        xCursor->jumpToLastPage();
        pageStyleName = getProperty<OUString>(xCursor, u"PageStyleName"_ustr);
        xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, u"IsLandscape"_ustr));
    };
    createSwDoc("landscape.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf97035)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

        // First cell width of the second row should be 2300
        uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
        CPPUNIT_ASSERT_EQUAL(sal_Int16(2300),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(1), u"TableColumnSeparators"_ustr)[0]
                                 .Position);
    };
    createSwDoc("tdf97035.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf87034)
{
    auto verify = [this]() {
        // This was A1BC34D, i.e. the first "super" text portion was mis-imported,
        // and was inserted instead right before the second "super" text portion.
        CPPUNIT_ASSERT_EQUAL(u"A1B3C4D"_ustr, getParagraph(1)->getString());
    };
    createSwDoc("tdf87034.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testClassificatonPasteLevels)
{
    createSwDoc("classification-confidential.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();

    // Classified source and classified destination, but internal only has a
    // higher level than confidential: nothing should happen.
    OUString aOld = xText->getString();
    paste(u"rtfexport/data/classification-yes.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr,
          xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95707)
{
    auto verify = [this]() {
        // Graphic was replaced with a "Read-Error" placeholder.
        uno::Reference<graphic::XGraphic> xGraphic
            = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), u"Graphic"_ustr);
        CPPUNIT_ASSERT(xGraphic.is());
        CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);
    };
    createSwDoc("tdf95707.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf96275)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(3, xCell->getText());
        // This was text: the shape's frame was part of the 1st paragraph instead of the 3rd one.
        CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                             getProperty<OUString>(getRun(xParagraph, 1), u"TextPortionType"_ustr));
    };
    createSwDoc("tdf96275.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf82073)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // This was -1: the background color was automatic, not black.
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xCell, u"BackColor"_ustr));
    };
    createSwDoc("tdf82073.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf74795)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // This was 0, \trpaddl was ignored on import.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));

        xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
        // Make sure that the scope of the default is only one row.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
    };
    createSwDoc("tdf74795.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137085)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        // \trpaddl0 overrides \trgaph600 (-1058 mm100) and built-in default of 190
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(xTable, u"LeftMargin"_ustr));

        // the \trpaddl0 is applied to all cells
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));

        xCell.set(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
    };
    createSwDoc("tdf137085.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77349)
{
    auto verify = [this]() {
        uno::Reference<container::XNamed> xImage(getShape(1), uno::UNO_QUERY);
        // This was empty: imported image wasn't named automatically.
        CPPUNIT_ASSERT_EQUAL(u"Image1"_ustr, xImage->getName());
    };
    createSwDoc("tdf77349.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf50821)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // This was 0, \trpaddfl was mishandled on import.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(191),
                             getProperty<sal_Int32>(xCell, u"LeftBorderDistance"_ustr));
    };
    createSwDoc("tdf50821.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf100507)
{
    auto verify = [this]() {
        // This was 0: left margin of the first paragraph was lost on import.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6618),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
    };
    createSwDoc("tdf100507.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf44986)
{
    auto verify = [this]() {
        // Check that the table at the second paragraph.
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
        uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
        // Check the first row of the table, it should have two cells (one separator).
        // This was 0: the first row had no separators, so it had only one cell, which was too wide.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)
                                 .getLength());
    };
    createSwDoc("tdf44986.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf90697)
{
    auto verify = [this]() {
        // We want section breaks to be seen as section breaks, not as page breaks,
        // so this document should have only one page, not three.
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf90697.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104317)
{
    auto verify = [this]() {
        // This failed to load, we tried to set CustomShapeGeometry on a line shape.
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
    };
    createSwDoc("tdf104317.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104744)
{
    auto verify = [this]() {
        auto xRules = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), u"NumberingRules"_ustr);
        comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
        // This was 0.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                             aRule[u"IndentAt"_ustr].get<sal_Int32>());
    };
    createSwDoc("tdf104744.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
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

    saveAndReload(u"Rich Text Format"_ustr);
    xFootnotesSupplier.set(mxComponent, uno::UNO_QUERY);
    sal_uInt16 nExpected = style::NumberingType::SYMBOL_CHICAGO;
    auto nActual
        = getProperty<sal_uInt16>(xFootnotesSupplier->getFootnoteSettings(), u"NumberingType"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 63
    // - Actual  : 4
    // i.e. the numbering type was ARABIC, not SYMBOL_CHICAGO.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105852)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
        // All rows but last were merged -> there were only 2 rows
        CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTableRows->getCount());
        // The first row must have 4 cells.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)
                                 .getLength());
        // The third row must have 1 merged cell.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(2), u"TableColumnSeparators"_ustr)
                                 .getLength());
    };
    createSwDoc("tdf105852.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104287)
{
    auto verify = [this]() {
        uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShape.is());
        // This failed, the bitmap had no valid anchor.
        CPPUNIT_ASSERT(xShape->getAnchor().is());
    };
    createSwDoc("tdf104287.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105729)
{
    auto verify = [this]() {
        // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc from style.
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                             static_cast<style::ParagraphAdjust>(
                                 getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
    };
    createSwDoc("tdf105729.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106694)
{
    auto verify = [this]() {
        auto aTabs
            = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), u"ParaTabStops"_ustr);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
        // This was 0, tab position was incorrect, looked like it was missing.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14605), aTabs[0].Position);
    };
    createSwDoc("tdf106694.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107116)
{
    auto verify = [this]() {
        // This was 0, upper border around text (and its distance) was missing.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(120)),
                             getProperty<sal_Int32>(getParagraph(2), u"TopBorderDistance"_ustr));
    };
    createSwDoc("tdf107116.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106950)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xPara(getParagraph(1));
        // This was ParagraphAdjust_LEFT, trying to set CharShadingValue on a
        // paragraph style thrown an exception, and remaining properties were not
        // set.
        CPPUNIT_ASSERT_EQUAL(
            style::ParagraphAdjust_CENTER,
            static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
    };
    createSwDoc("tdf106950.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116371)
{
    loadAndReload("tdf116371.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    auto xShape(getShape(1));
    // Without the accompanying fix in place, this test would have failed with
    // 'Unknown property: RotateAngle', i.e. export lost the rotation, and then
    // import created a Writer picture (instead of a Draw one).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, u"RotateAngle"_ustr), 10);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133437)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(3, getPages());
        CPPUNIT_ASSERT_EQUAL(560, getShapes()); // 285 \shp + 275 \poswX

        xmlDocUniquePtr pDump = parseLayoutDump();
        // Count shapes on first page
        assertXPath(pDump, "/root/page[1]/body/txt[1]/anchored/SwAnchoredDrawObject"_ostr, 79);

        // Second page
        assertXPath(pDump, "/root/page[2]/body/txt[2]/anchored/SwAnchoredDrawObject"_ostr, 120);

        // Third page
        assertXPath(pDump, "/root/page[3]/body/txt[2]/anchored/SwAnchoredDrawObject"_ostr, 86);
    };
    createSwDoc("tdf133437.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128320)
{
    loadAndReload("tdf128320.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Shape does exist in RTF output
    auto xShape(getShape(1));
    CPPUNIT_ASSERT(xShape.is());

    // Let's see what is inside output RTF file
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aRtfContent(read_uInt8s_ToOString(*pStream, pStream->TellEnd()));

    // There are some RTF tokens for shape props
    // They are much more inside, but let's use \shpwr2 as an indicator
    sal_Int32 nPos = aRtfContent.indexOf("\\shpwr2", 0);
    CPPUNIT_ASSERT(nPos > 0);

    // It goes AFTER shape instruction (sadly here we do not check if it is contained inside)
    sal_Int32 nPosShp = aRtfContent.indexOf("\\shpinst", 0);
    CPPUNIT_ASSERT(nPosShp > 0);

    // But there are no more shape properties!
    nPos = aRtfContent.indexOf("\\shpwr2", nPos + 1);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), nPos);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf129513)
{
    auto verify = [this]() {
        // \pagebb after \intbl must not reset the "in table" flag
        CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
        // Make sure the first paragraph is imported in table
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getCellNames().getLength());
        uno::Reference<text::XText> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(u"In table"_ustr, xCell->getString());
    };
    createSwDoc("tdf129513.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138210)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    };
    createSwDoc("tdf138210.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137894)
{
    loadAndReload("tdf137894.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    lang::Locale locale1(
        getProperty<lang::Locale>(getRun(getParagraph(1), 1), u"CharLocaleAsian"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"ja"_ustr, locale1.Language);
    CPPUNIT_ASSERT_EQUAL(u"MS UI Gothic"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1),
                                                                     u"CharFontNameAsian"_ustr));
    CPPUNIT_ASSERT_EQUAL(20.f,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharHeightAsian"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Mangal"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1),
                                                               u"CharFontNameComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(20.f,
                         getProperty<float>(getRun(getParagraph(1), 1), u"CharHeightComplex"_ustr));
    lang::Locale locale2(
        getProperty<lang::Locale>(getRun(getParagraph(2), 1), u"CharLocaleComplex"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"he"_ustr, locale2.Language);
    CPPUNIT_ASSERT_EQUAL(32.f,
                         getProperty<float>(getRun(getParagraph(2), 1), u"CharHeightComplex"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138779)
{
    loadAndReload("tdf138779.docx");
    // The text "2. Kozuka Mincho Pro, 8 pt Ruby ..." has font size 11pt ( was 20pt ).
    CPPUNIT_ASSERT_EQUAL(11.f, getProperty<float>(getRun(getParagraph(2), 14), u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf144437)
{
    loadAndReload("tdf144437.odt");
    SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream);
    OString aRtfContent(read_uInt8s_ToOString(*pStream, pStream->TellEnd()));

    sal_Int32 nTextEndPos = aRtfContent.indexOf("Bookmark here->", 0) + 14;
    CPPUNIT_ASSERT_MESSAGE("Para content wasn't found in file", nTextEndPos > 0);

    sal_Int32 nBmkStartPos = aRtfContent.indexOf("{\\*\\bkmkstart bookmark}", 0);
    CPPUNIT_ASSERT_MESSAGE("Bookmark start wasn't found in file", nBmkStartPos > 0);

    sal_Int32 nBmkEndPos = aRtfContent.indexOf("{\\*\\bkmkend bookmark}", 0);
    CPPUNIT_ASSERT_MESSAGE("Bookmark end wasn't found in file", nBmkEndPos > 0);

    CPPUNIT_ASSERT_MESSAGE("Bookmark started in wrong position", nBmkStartPos > nTextEndPos);
    CPPUNIT_ASSERT_MESSAGE("Bookmark ended in wrong position", nBmkEndPos > nTextEndPos);
    CPPUNIT_ASSERT_MESSAGE("Bookmark start & end are wrong", nBmkEndPos > nBmkStartPos);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131234)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 1, u"Hello"_ustr);

        // Ensure that text has default font attrs in spite of style referenced
        // E.g. 12pt, Times New Roman, black, no bold, no italic, no underline
        CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xRun, u"CharColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Times New Roman"_ustr,
                             getProperty<OUString>(xRun, u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xRun, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::NONE,
                             getProperty<sal_Int16>(xRun, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
    };
    createSwDoc("tdf131234.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf118047)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xPara = getParagraph(1);

        // Ensure that default "Normal" style properties are not applied to text:
        // text remains with fontsize 12pt and no huge margin below
        CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getRun(xPara, 1), u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaBottomMargin"_ustr));

        // Same for header, it should not derive props from "Normal" style
        CPPUNIT_ASSERT_EQUAL(u"Header"_ustr, parseDump("/root/page[1]/header/txt/text()"_ostr));
        sal_Int32 nHeight
            = parseDump("/root/page[1]/header/infos/bounds"_ostr, "height"_ostr).toInt32();
        CPPUNIT_ASSERT_MESSAGE("Header is too large", 1000 > nHeight);
    };
    createSwDoc("tdf118047.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158950)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xRun = getRun(getParagraph(1), 2, u"style"_ustr);

        // Without the fix in place, this test would have failed with
        // - Expected: rgba[ff0000ff]
        // - Actual  : rgba[000000ff]
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xRun, u"CharColor"_ustr));
        CPPUNIT_ASSERT_EQUAL(16.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Courier New"_ustr,
                             getProperty<OUString>(xRun, u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xRun, u"CharWeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::NONE,
                             getProperty<sal_Int16>(xRun, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xRun, u"CharPosture"_ustr));
    };
    createSwDoc("tdf158950.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf104390)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xPara = getParagraph(1);
        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xPara, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();

        // Check font in first run
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(36.f, getProperty<float>(xRun, u"CharHeight"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"Courier New"_ustr,
                             getProperty<OUString>(xRun, u"CharFontName"_ustr));

        // Ensure this run covers whole paragraph text (ignore possible empty "paragraph marker" run)
        CPPUNIT_ASSERT_EQUAL(xPara->getString().getLength(), xRun->getString().getLength());
    };
    createSwDoc("tdf104390.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153681)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent,
                                                                  uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY_THROW);

        // This is outside table
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(1), uno::UNO_QUERY_THROW);
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2
        // - Actual  : 3
        // Generates extra cell
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getColumns()->getCount());
    };
    createSwDoc("tdf153681.odt");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
