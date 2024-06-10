/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <tools/UnitConversion.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/scopeguard.hxx>

#include <bordertest.hxx>

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

CPPUNIT_TEST_FIXTURE(Test, testFdo45553)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        while (xParaEnum->hasMoreElements())
        {
            uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                           uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xRangeEnum
                = xRangeEnumAccess->createEnumeration();
            while (xRangeEnum->hasMoreElements())
            {
                uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
                OUString aStr = xRange->getString();
                if (aStr == "space-before")
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(120)),
                                         getProperty<sal_Int32>(xRange, u"ParaTopMargin"_ustr));
                else if (aStr == "space-after")
                    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(240)),
                                         getProperty<sal_Int32>(xRange, u"ParaBottomMargin"_ustr));
            }
        }
    };
    createSwDoc("fdo45553.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN192129)
{
    auto verify = [this]() {
        // We expect that the result will be 16x16px.
        Size aExpectedSize(16, 16);
        MapMode aMap(MapUnit::Map100thMM);
        aExpectedSize = Application::GetDefaultDevice()->PixelToLogic(aExpectedSize, aMap);

        uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(
            xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        awt::Size aActualSize(xShape->getSize());
        if ((aExpectedSize.Width() - aActualSize.Width) / 2 != 0)
        {
            CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
        }
        if ((aExpectedSize.Height() - aActualSize.Height) / 2 != 0)
        {
            CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
        }
    };
    createSwDoc("n192129.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45543)
{
    auto verify = [this]() { CPPUNIT_ASSERT_EQUAL(u"この文書は"_ustr, getBodyText()); };
    createSwDoc("fdo45543.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo42465)
{
    auto verify = [this]() { CPPUNIT_ASSERT_EQUAL(u"kód"_ustr, getBodyText()); };
    createSwDoc("fdo42465.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45187)
{
    auto verify = [this]() {
        // There should be two shapes.
        CPPUNIT_ASSERT_EQUAL(2, getShapes());

        // They should be anchored to different paragraphs.
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xAnchor0
            = uno::Reference<text::XTextContent>(getShape(1), uno::UNO_QUERY_THROW)->getAnchor();
        uno::Reference<text::XTextRange> xAnchor1
            = uno::Reference<text::XTextContent>(getShape(2), uno::UNO_QUERY_THROW)->getAnchor();
        // Was 0 ("starts at the same position"), should be 1 ("starts before")
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1),
                             xTextRangeCompare->compareRegionStarts(xAnchor0, xAnchor1));
    };
    createSwDoc("fdo45187.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN750757)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

        CPPUNIT_ASSERT_EQUAL(
            false, getProperty<bool>(xParaEnum->nextElement(), u"ParaContextMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            true, getProperty<bool>(xParaEnum->nextElement(), u"ParaContextMargin"_ustr));
    };
    createSwDoc("n750757.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45563)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        int i = 0;
        while (xParaEnum->hasMoreElements())
        {
            xParaEnum->nextElement();
            i++;
        }
        CPPUNIT_ASSERT_EQUAL(4, i);
    };
    createSwDoc("fdo45563.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo43965)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

        // First paragraph: the parameter of \up was ignored
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                       uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(36),
                             getProperty<sal_Int32>(xPropertySet, u"CharEscapement"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(100),
                             getProperty<sal_Int32>(xPropertySet, u"CharEscapementHeight"_ustr));

        // Second paragraph: Word vs Writer border default problem
        CPPUNIT_ASSERT_EQUAL(
            sal_uInt32(26),
            getProperty<table::BorderLine2>(xParaEnum->nextElement(), u"TopBorder"_ustr).LineWidth);

        // Finally, make sure that we have two pages
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo43965.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN751020)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32(convertTwipToMm100(200)),
            getProperty<sal_Int32>(xParaEnum->nextElement(), u"ParaBottomMargin"_ustr));
    };
    createSwDoc("n751020.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79384)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        CPPUNIT_ASSERT_EQUAL(u"Маркеры спискамЫ"_ustr, xTextRange->getString());
    };
    createSwDoc("fdo79384.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47326)
{
    auto verify = [this]() {
        // Length was 15 only, as \super buffered text, then the contents of it got lost.
        CPPUNIT_ASSERT_EQUAL(u"Windows®XP: Cartes:"_ustr, getBodyText());
    };
    createSwDoc("fdo47326.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo46955)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        while (xParaEnum->hasMoreElements())
        {
            uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                           uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xRangeEnum
                = xRangeEnumAccess->createEnumeration();
            while (xRangeEnum->hasMoreElements())
                CPPUNIT_ASSERT_EQUAL(
                    style::CaseMap::UPPERCASE,
                    getProperty<sal_Int16>(xRangeEnum->nextElement(), u"CharCaseMap"_ustr));
        }
    };
    createSwDoc("fdo46955.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo81892)
{
    auto verify = [this]() {
        // table was not centered
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                             getProperty<sal_Int16>(xTable, u"HoriOrient"_ustr));

        // fdo#81893: paragraph with \page was not centered
        uno::Reference<text::XTextRange> xPara(getParagraph(2, u"Performance"_ustr));
        CPPUNIT_ASSERT_EQUAL(
            style::ParagraphAdjust_CENTER,
            static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
    };
    createSwDoc("fdo81892.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo45394)
{
    auto verify = [this]() {
        uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
        OUString aActual = xHeaderText->getString();
        // Encoding in the header was wrong.
        CPPUNIT_ASSERT_EQUAL(u"\u041F\u041A \u0420\u0418\u041A"_ustr, aActual);

        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    };
    createSwDoc("fdo45394.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48104)
{
    auto verify = [this]() { CPPUNIT_ASSERT_EQUAL(2, getPages()); };
    createSwDoc("fdo48104.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47107)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xNumberingStyles(getStyles(u"NumberingStyles"_ustr));
        // Make sure numbered and bullet legacy syntax is recognized, this used to throw a NoSuchElementException
        xNumberingStyles->getByName(u"WWNum1"_ustr);
        xNumberingStyles->getByName(u"WWNum2"_ustr);
    };
    createSwDoc("fdo47107.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

// TODO - First Page Headers Support
// This assumption is false now - we only have "Standard" page style, which should have properties properly set
/*CPPUNIT_TEST_FIXTURE(Test, testFdo44176)
{
    auto verify = [this]() {
        uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
        uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"),
                                                       uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName("Standard"),
                                                     uno::UNO_QUERY);
        sal_Int32 nFirstTop = 0, nDefaultTop = 0, nDefaultHeader = 0;
        xFirstPage->getPropertyValue("TopMargin") >>= nFirstTop;
        xDefault->getPropertyValue("TopMargin") >>= nDefaultTop;
        xDefault->getPropertyValue("HeaderHeight") >>= nDefaultHeader;
        CPPUNIT_ASSERT_EQUAL(nFirstTop, nDefaultTop + nDefaultHeader);
    };
    createSwDoc("fdo44176.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}*/

CPPUNIT_TEST_FIXTURE(Test, testFdo39053)
{
    auto verify = [this]() {
        int nShapes = getShapes();
        CPPUNIT_ASSERT_EQUAL(1, nShapes);
        int nAsCharacter = 0;
        for (int i = 0; i < nShapes; ++i)
            if (getProperty<text::TextContentAnchorType>(getShape(i + 1), u"AnchorType"_ustr)
                == text::TextContentAnchorType_AS_CHARACTER)
                nAsCharacter++;
        // The image in binary format was ignored.
        CPPUNIT_ASSERT_EQUAL(1, nAsCharacter);
    };
    createSwDoc("fdo39053.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48356)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        int i = 0;
        while (xParaEnum->hasMoreElements())
        {
            xParaEnum->nextElement();
            i++;
        }
        // The document used to be imported as two paragraphs.
        CPPUNIT_ASSERT_EQUAL(1, i);
    };
    createSwDoc("fdo48356.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48023)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        // Implicit encoding detection based on locale was missing
        CPPUNIT_ASSERT_EQUAL(
            u"\u041F\u0440\u043E\u0433\u0440\u0430\u043C\u043C\u0438\u0441\u0442"_ustr,
            xTextRange->getString());
    };

    AllSettings aSavedSettings = Application::GetSettings();
    AllSettings aSettings(aSavedSettings);
    aSettings.SetLanguageTag(LanguageTag(u"ru"_ustr));
    Application::SetSettings(aSettings);
    comphelper::ScopeGuard g([&aSavedSettings] { Application::SetSettings(aSavedSettings); });

    createSwDoc("fdo48023.rtf");
    verify();
    saveAndReload(u"Rich Text Format"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48876)
{
    auto verify = [this]() {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
        CPPUNIT_ASSERT_EQUAL(
            style::LineSpacingMode::MINIMUM,
            getProperty<style::LineSpacing>(xParaEnum->nextElement(), u"ParaLineSpacing"_ustr)
                .Mode);
    };
    createSwDoc("fdo48876.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48193)
{
    auto verify = [this]() { CPPUNIT_ASSERT_EQUAL(u"foo1bar"_ustr, getBodyText()); };
    createSwDoc("fdo48193.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo44211)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

        CPPUNIT_ASSERT_EQUAL(u"\u0105\u010D\u0119"_ustr, xTextRange->getString());
    };

    AllSettings aSavedSettings = Application::GetSettings();
    AllSettings aSettings(aSavedSettings);
    aSettings.SetLanguageTag(LanguageTag(u"lt"_ustr));
    Application::SetSettings(aSettings);
    comphelper::ScopeGuard g([&aSavedSettings] { Application::SetSettings(aSavedSettings); });

    createSwDoc("fdo44211.rtf");
    verify();
    saveAndReload(u"Rich Text Format"_ustr);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48037)
{
    auto verify = [this]() {
        uno::Reference<util::XNumberFormatsSupplier> xNumberSupplier(mxComponent,
                                                                     uno::UNO_QUERY_THROW);
        lang::Locale aUSLocale, aFRLocale;
        aUSLocale.Language = "en";
        aFRLocale.Language = "fr";
        sal_Int32 nExpected = xNumberSupplier->getNumberFormats()->addNewConverted(
            u"d MMMM yyyy"_ustr, aUSLocale, aFRLocale);

        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
        sal_Int32 nActual = 0;
        xPropertySet->getPropertyValue(u"NumberFormat"_ustr) >>= nActual;

        CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
    };
    createSwDoc("fdo48037.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47764)
{
    auto verify = [this]() {
        // \cbpat with zero argument should mean the auto (-1) color, not a default color (black)
        CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(getParagraph(1), u"ParaBackColor"_ustr));
    };
    createSwDoc("fdo47764.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo38786)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        // \chpgn was ignored, so exception was thrown
        xFields->nextElement();
    };
    createSwDoc("fdo38786.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN757651)
{
    auto verify = [this]() {
        // The bug was that due to buggy layout the text expanded to two pages.
        if (Application::GetDefaultDevice()->IsFontAvailable(u"Times New Roman"))
            CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("n757651.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49501)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);

        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, u"IsLandscape"_ustr));
        sal_Int32 nExpected(convertTwipToMm100(567));
        CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, u"LeftMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, u"RightMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, u"TopMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, u"BottomMargin"_ustr));
    };
    createSwDoc("fdo49501.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49271)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(25.f, getProperty<float>(getParagraph(2), u"CharHeight"_ustr));
    };
    createSwDoc("fdo49271.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo50539)
{
    auto verify = [this]() {
        // \chcbpat with zero argument should mean the auto (-1) color, not a default color (black)
        CPPUNIT_ASSERT_EQUAL(COL_AUTO,
                             getProperty<Color>(getRun(getParagraph(1), 1), u"CharBackColor"_ustr));
    };
    createSwDoc("fdo50539.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo50665)
{
    auto verify = [this]() {
        // Access the second run, which is a textfield
        uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
        // This used to be the default, as character properties were ignored.
        CPPUNIT_ASSERT_EQUAL(u"Book Antiqua"_ustr,
                             getProperty<OUString>(xRun, u"CharFontName"_ustr));
    };
    createSwDoc("fdo50665.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49659)
{
    auto verify = [this]() {
        // Both tables were ignored: 1) was in the header, 2) was ignored due to missing empty par at the end of the doc
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

        // The graphic was also empty
        uno::Reference<beans::XPropertySet> xGraphic
            = getProperty<uno::Reference<beans::XPropertySet>>(getShape(1), u"Graphic"_ustr);
        CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL,
                             getProperty<sal_Int8>(xGraphic, u"GraphicType"_ustr));
    };
    createSwDoc("fdo49659.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo46966)
{
    auto verify = [this]() {
        /*
         * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
         *
         * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
         */
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(720)),
                             getProperty<sal_Int32>(xPropertySet, u"TopMargin"_ustr));
    };
    createSwDoc("fdo46966.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76633)
{
    auto verify = [this]() {
        // check that there is only a graphic object, not an additional rectangle
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
        uno::Reference<lang::XServiceInfo> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xShape.is());
        CPPUNIT_ASSERT(xShape->supportsService(u"com.sun.star.text.TextGraphicObject"_ustr));
    };
    createSwDoc("fdo76633.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48033)
{
    auto verify = [this]() {
        /*
         * The problem was that the picture (48033) or OLE object (53594) was in the first cell,
         * instead of the second one.
         *
         * oTable = ThisComponent.TextTables(0)
         * oParas = oTable.getCellByName("B1").Text.createEnumeration
         * oPara = oParas.nextElement
         * oRuns = oPara.createEnumeration
         * oRun = oRuns.nextElement
         * xray oRun.TextPortionType ' Frame, was Text
         */
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                             getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    };
    createSwDoc("fdo48033.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo53594)
{
    auto verify = [this]() {
        /*
         * The problem was that the picture (48033) or OLE object (53594) was in the first cell,
         * instead of the second one.
         *
         * oTable = ThisComponent.TextTables(0)
         * oParas = oTable.getCellByName("B1").Text.createEnumeration
         * oPara = oParas.nextElement
         * oRuns = oPara.createEnumeration
         * oRun = oRuns.nextElement
         * xray oRun.TextPortionType ' Frame, was Text
         */
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                             getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    };
    createSwDoc("fdo53594.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo36089)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(sal_Int16(-50), getProperty<sal_Int16>(getRun(getParagraph(1), 2),
                                                                    u"CharEscapement"_ustr));
    };
    createSwDoc("fdo36089.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48446)
{
    auto verify = [this]() { getParagraph(1, u"\u0418\u043C\u044F"_ustr); };
    createSwDoc("fdo48446.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47495)
{
    auto verify = [this]() {
        // Used to have 4 paragraphs, as a result the original bugdoc had 2 pages instead of 1.
        // Word 2013 shows 1 paragraph
        CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    };
    createSwDoc("fdo47495.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testAllGapsWord)
{
    auto verify = [this]() {
        BorderTest borderTest;
        BorderTest::testTheBorders(mxComponent, false);
    };
    createSwDoc("all_gaps_word.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52389)
{
    auto verify = [this]() {
        // The last '!' character at the end of the document was lost
        CPPUNIT_ASSERT_EQUAL(u"dania!"_ustr, getBodyText());
    };
    createSwDoc("fdo52389.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49655)
{
    auto verify = [this]() {
        /*
         * 49655 :
         * The problem was that the table was not imported due to the '  ' string in the middle of the table definition.
         *
         * xray ThisComponent.TextTables.Count 'was 0
         */
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    };
    createSwDoc("fdo49655.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo62805)
{
    auto verify = [this]() {
        /*
         * 62805 :
         * The problem was that the table was not imported due to the absence of \pard after \row.
         * The table was instead in a group (the '}' replace the \pard).
         */
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    };
    createSwDoc("fdo62805.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52475)
{
    auto verify = [this]() {
        // The problem was that \chcbpat0 resulted in no color, instead of COL_AUTO.
        CPPUNIT_ASSERT_EQUAL(COL_AUTO,
                             getProperty<Color>(getRun(getParagraph(1), 3), u"CharBackColor"_ustr));
    };
    createSwDoc("fdo52475.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo55493)
{
    auto verify = [this]() {
        // The problem was that the width of the PNG was detected as 15,24cm, instead of 3.97cm
        uno::Reference<drawing::XShape> xShape = getShape(1);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3969), xShape->getSize().Width);
    };
    createSwDoc("fdo55493.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCopyPastePageStyle)
{
    createSwDoc("copypaste-pagestyle.rtf");
    // The problem was that RTF import during copy&paste did not ignore page styles.
    // Once we have more copy&paste tests, makes sense to refactor this to some helper method.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/copypaste-pagestyle-paste.rtf",
          u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(21001),
        getProperty<sal_Int32>(xPropertySet, u"Width"_ustr)); // Was letter, i.e. 21590
}

CPPUNIT_TEST_FIXTURE(Test, testCopyPasteFootnote)
{
    createSwDoc("copypaste-footnote.rtf");
    // The RTF import did not handle the case when the position wasn't the main document XText, but something different, e.g. a footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    paste(u"rtfexport/data/copypaste-footnote-paste.rtf",
          u"com.sun.star.comp.Writer.RtfFilter"_ustr, xTextRange);

    CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo63428)
{
    createSwDoc("hello.rtf");
    // Pasting content that contained an annotation caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/fdo63428.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);

    // Additionally, commented range was imported as a normal comment.
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr, getProperty<OUString>(getRun(getParagraph(1), 2),
                                                                   u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr, getProperty<OUString>(getRun(getParagraph(1), 4),
                                                                      u"TextPortionType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69384)
{
    auto verify = [this]() {
        // Ensure non-default style is loaded
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Text body justified"_ustr);
        // Ensure default styles were modified, vs testFdo69384Inserted where it is not
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"ParagraphStyles"_ustr)->getByName(u"Text body"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(68.f, getProperty<float>(xPropertySet, u"CharHeight"_ustr));
    };
    createSwDoc("fdo69384-paste.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69384Inserted)
{
    createSwDoc("hello.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/fdo69384-paste.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);

    // During insert of the RTF document we do not change pre-existing styles
    // vs testFdo69384 where it is
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Text body"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xPropertySet, u"CharHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo61193)
{
    createSwDoc("hello.rtf");
    // Pasting content that contained a footnote caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/fdo61193.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf108123)
{
    createSwDoc("hello.rtf");
    // This crashed, the shape push/pop and table manager stack went out of
    // sync -> we tried to de-reference an empty stack.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/tdf108123.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);
}

CPPUNIT_TEST_FIXTURE(Test, testShptxtPard)
{
    auto verify = [this]() {
        // The problem was that \pard inside \shptxt caused loss of shape text
        uno::Reference<text::XText> xText(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"shape text"_ustr, xText->getString());
    };
    createSwDoc("shptxt-pard.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDoDhgt)
{
    auto verify = [this]() {
        int nShapes = getShapes();
        CPPUNIT_ASSERT_EQUAL(3, nShapes);
        for (int i = 0; i < nShapes; ++i)
        {
            Color nFillColor = getProperty<Color>(getShape(i + 1), u"FillColor"_ustr);
            if (nFillColor == 0xc0504d) // red
                CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                     getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
            else if (nFillColor == 0x9bbb59) // green
                CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                                     getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
            else if (nFillColor == 0x4f81bd) // blue
                CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                                     getProperty<sal_Int32>(getShape(i + 1), u"ZOrder"_ustr));
        }
    };
    createSwDoc("do-dhgt.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDplinehollow)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
        table::BorderLine2 line(getProperty<table::BorderLine2>(xPropertySet, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, line.LineStyle);
    };
    createSwDoc("dplinehollow.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testLeftmarginDefault)
{
    auto verify = [this]() {
        // The default left/right margin was incorrect when the top margin was set to zero.
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32(2540),
            getProperty<sal_Int32>(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
                                   u"LeftMargin"_ustr));
    };
    createSwDoc("leftmargin-default.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDppolyline)
{
    auto verify = [this]() {
        // This was completely ignored, for now, just make sure we have all 4 lines.
        CPPUNIT_ASSERT_EQUAL(4, getShapes());
    };
    createSwDoc("dppolyline.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo56512)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                             uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xTextRange(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"\u05E2\u05D5\u05E1\u05E7 \u05DE\u05D5\u05E8\u05E9\u05D4 "_ustr,
                             xTextRange->getString());
    };
    createSwDoc("fdo56512.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo52989)
{
    auto verify = [this]() {
        // Same as n#192129, but for JPEG files.
        uno::Reference<drawing::XShape> xShape = getShape(1);
        OString aMessage
            = "xShape->getSize().Width() = " + OString::number(xShape->getSize().Width);

        // This was 2, should be 423 (or 369?).
        CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), xShape->getSize().Width >= 273);
    };
    createSwDoc("fdo52989.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo54473)
{
    auto verify = [this]() {
        // The problem was that character styles were not imported due to a typo.
        CPPUNIT_ASSERT_EQUAL(u"Anot"_ustr,
                             getProperty<OUString>(getRun(getParagraph(1), 1, u"Text "_ustr),
                                                   u"CharStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"ForeignTxt"_ustr,
                             getProperty<OUString>(getRun(getParagraph(1), 3, u"character "_ustr),
                                                   u"CharStyleName"_ustr));
    };
    createSwDoc("fdo54473.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo49934)
{
    auto verify = [this]() {
        // Column break without columns defined should be a page break, but it was just ignored.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo49934.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo57886)
{
    auto verify = [this]() {
        // Was 'int from <?> to <?> <?>'.
        CPPUNIT_ASSERT_EQUAL(u"int from {firstlower} to {firstupper} {firstbody}"_ustr,
                             getFormula(getRun(getParagraph(1), 1)));
    };
    createSwDoc("fdo57886.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58076)
{
    auto verify = [this]() {
        // An additional section was created, so the default page style didn't have the custom margins.
        uno::Reference<beans::XPropertySet> xStyle(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2251), getProperty<sal_Int32>(xStyle, u"LeftMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1752), getProperty<sal_Int32>(xStyle, u"RightMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, u"TopMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, u"BottomMargin"_ustr));
    };
    createSwDoc("fdo58076.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo57678)
{
    auto verify = [this]() {
        // Paragraphs of the two tables were not converted to tables.
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                             uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
    };
    createSwDoc("fdo57678.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo54612)
{
    auto verify = [this]() {
        // \dpptx without a \dppolycount caused a crash.
        CPPUNIT_ASSERT_EQUAL(8, getShapes());
    };
    createSwDoc("fdo54612.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58933)
{
    auto verify = [this]() {
        // The problem was that the table had an additional cell in its first line.
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        // This was 4.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getCellNames().getLength());
    };
    createSwDoc("fdo58933.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo44053)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
        // The with of the table's A1 and A2 cell should equal.
        CPPUNIT_ASSERT_EQUAL(getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)[0]
                                 .Position,
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(1), u"TableColumnSeparators"_ustr)[0]
                                 .Position);
    };
    createSwDoc("fdo44053.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo48440)
{
    auto verify = [this]() {
        // Page break was ignored.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo48440.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58646line)
{
    auto verify = [this]() {
        // \line symbol was ignored
        getParagraph(1, u"foo\nbar"_ustr);
    };
    createSwDoc("fdo58646line.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78502)
{
    auto verify = [this]() {
        // ";" separators were inserted as text
        getParagraph(1, u"foo"_ustr);
    };
    createSwDoc("fdo78502.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58646)
{
    auto verify = [this]() {
        // Page break was ignored inside a continuous section, on title page.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("fdo58646.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo59419)
{
    auto verify = [this]() {
        // Junk to be ignored broke import of the table.
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    };
    createSwDoc("fdo59419.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testHexCRLF)
{
    auto verify = [this]() {
        // hex-escaped \r and \n should create a paragraph break
        getParagraph(1, u"foo"_ustr);
        getParagraph(2, u"bar"_ustr);
        getParagraph(3, u"baz"_ustr);
        getParagraph(4, u""_ustr);
        getParagraph(5, u"quux"_ustr);
    };
    createSwDoc("hexcrlf.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo58076_2)
{
    auto verify = [this]() {
        // Position of the picture wasn't correct.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(8345)),
                             getProperty<sal_Int32>(getShape(1), u"HoriOrientPosition"_ustr));
    };
    createSwDoc("fdo58076-2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo59638)
{
    auto verify = [this]() {
        // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xLevels(
            xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aProps;
        xLevels->getByIndex(0) >>= aProps; // 1st level

        for (beans::PropertyValue const& rProp : aProps)
        {
            if (rProp.Name == "BulletChar")
            {
                // Was '*', should be 'o'.
                CPPUNIT_ASSERT_EQUAL(u"\uF0B7"_ustr, rProp.Value.get<OUString>());
                return;
            }
        }
        CPPUNIT_FAIL("no BulletChar property");
    };
    createSwDoc("fdo59638.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo60722)
{
    auto verify = [this]() {
        // The problem was that the larger shape was over the smaller one, and not the other way around.
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"ZOrder"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"larger"_ustr, getProperty<OUString>(xShape, u"Description"_ustr));

        xShape.set(getShape(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xShape, u"ZOrder"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"smaller"_ustr, getProperty<OUString>(xShape, u"Description"_ustr));

        // Color of the line was blue, and it had zero width.
        xShape.set(getShape(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), getProperty<sal_uInt32>(xShape, u"LineWidth"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xShape, u"LineColor"_ustr));
    };
    createSwDoc("fdo60722.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDoDhgtOld)
{
    auto verify = [this]() {
        // The file contains 3 shapes which have the same dhgt (z-order).
        // Test that the order is 1) a 2) black rectangle 3) b, and not something else
        uno::Reference<text::XText> xShape(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, u"ZOrder"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"a"_ustr, xShape->getString());

        xShape.set(getShape(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xShape, u"ZOrder"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<Color>(xShape, u"FillColor"_ustr));

        xShape.set(getShape(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xShape, u"ZOrder"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"b"_ustr, xShape->getString());
    };
    createSwDoc("do-dhgt-old.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo61909)
{
    auto verify = [this]() {
        uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
        // Was the Writer default font.
        CPPUNIT_ASSERT_EQUAL(u"Courier New"_ustr,
                             getProperty<OUString>(xTextRange, u"CharFontName"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(xTextRange, u"CharBackColor"_ustr));
    };
    createSwDoc("fdo61909.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo62288)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
        // Margins were inherited from the previous cell, even there was a \pard there.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaLeftMargin"_ustr));
    };
    createSwDoc("fdo62288.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo37716)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xFrames(xTextFramesSupplier->getTextFrames(),
                                                        uno::UNO_QUERY);
        // \nowrap got ignored, so Surround was text::WrapTextMode_PARALLEL
        CPPUNIT_ASSERT_EQUAL(
            text::WrapTextMode_NONE,
            getProperty<text::WrapTextMode>(xFrames->getByIndex(0), u"Surround"_ustr));
    };
    createSwDoc("fdo37716.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo51916)
{
    // Complex nested table caused a crash.
    createSwDoc("fdo51916.rtf");
    saveAndReload(mpFilter);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
