/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>


#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/propertyvalue.hxx>

#include <swmodeltestbase.hxx>
#include <unotxdoc.hxx>

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf135164_cancelledNumbering, "tdf135164_cancelledNumbering.docx")
{
    uno::Reference<beans::XPropertySet> xPara(getParagraph(1, u"TBMM DÖNEMİ"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(2, "Numbering explicitly cancelled"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString(""), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(6, "Default style has roman numbering"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("i"), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf147861_customField, "tdf147861_customField.docx")
{
    // These should each be specific values, not a shared DocProperty
    getParagraph(1, "CustomEditedTitle"); // edited
    // A couple of nulls at the end of the string thwarted all attemps at an "equals" comparison.
    CPPUNIT_ASSERT(getParagraph(2)->getString().startsWith(" INSERT Custom Title here"));
    getParagraph(3, "My Title"); // edited

    // Verify that these are fields, and not just plain text
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    auto xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("CustomEditedTitle"), xField->getPresentation(false));
    // The " (fixed)" part is unnecessary, but it must be consistent across a round-trip
    CPPUNIT_ASSERT_EQUAL(OUString("DocInformation:Title (fixed)"), xField->getPresentation(true));
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf135906, "tdf135906.docx")
{
    // just test round-tripping. The document was exported as corrupt and didn't re-load.
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleNumLevel)
{
    loadAndSave("para-style-num-level.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. a custom list level in a para style was lost on import+export.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Mystyle']/w:pPr/w:numPr/w:ilvl", "val", "1");
}

CPPUNIT_TEST_FIXTURE(Test, testClearingBreak)
{
    // Given a document with a clearing break, when saving to DOCX:
    loadAndSave("clearing-break.docx");

    // Then make sure that the clearing break is not lost:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '/w:document/w:body/w:p/w:r/w:br' number of nodes is incorrect
    // i.e. first the clearing break was turned into a plain break, then it was completely lost.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:br", "clear", "all");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf148494)
{
    loadAndSave("tdf148494.docx");

    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Without the fix in place, this test would have failed with
    // - Expected:  MACROBUTTON AllCaps Hello World
    // - Actual  :  MACROBUTTONAllCaps Hello World
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " MACROBUTTON AllCaps Hello World ");
}

CPPUNIT_TEST_FIXTURE(Test, testContentControlExport)
{
    // Given a document with a content control around one or more text portions:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("ShowingPlaceHolder", uno::makeAny(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // XPath '//w:sdt/w:sdtPr/w:showingPlcHdr' number of nodes is incorrect
    // i.e. the SDT elements were missing on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:showingPlcHdr", 1);
    assertXPath(pXmlDoc, "//w:sdt/w:sdtContent", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testCheckboxContentControlExport)
{
    // Given a document with a checkbox content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, OUString(u"☐"), /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Checkbox", uno::makeAny(true));
    xContentControlProps->setPropertyValue("Checked", uno::makeAny(true));
    xContentControlProps->setPropertyValue("CheckedState", uno::makeAny(OUString(u"☒")));
    xContentControlProps->setPropertyValue("UncheckedState", uno::makeAny(OUString(u"☐")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w14:checkbox/w14:checked' number of nodes is incorrect
    // i.e. <w14:checkbox> and its child elements were lost.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:checked", "val", "1");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:checkedState", "val", "2612");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w14:checkbox/w14:uncheckedState", "val", "2610");
}

CPPUNIT_TEST_FIXTURE(Test, testDropdownContentControlExport)
{
    // Given a document with a dropdown content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "choose an item", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    {
        uno::Sequence<beans::PropertyValues> aListItems = {
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("red"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("R"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("green"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("G"))),
            },
            {
                comphelper::makePropertyValue("DisplayText", uno::Any(OUString("blue"))),
                comphelper::makePropertyValue("Value", uno::Any(OUString("B"))),
            },
        };
        xContentControlProps->setPropertyValue("ListItems", uno::Any(aListItems));
    }
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]' number of nodes is incorrect
    // i.e. the list items were lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]", "displayText", "red");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]", "value", "R");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]", "displayText", "green");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]", "value", "G");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[3]", "displayText", "blue");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem[3]", "value", "B");
}

CPPUNIT_TEST_FIXTURE(Test, testPictureContentControlExport)
{
    // Given a document with a picture content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<beans::XPropertySet> xTextGraphic(
        xMSF->createInstance("com.sun.star.text.TextGraphicObject"), uno::UNO_QUERY);
    xTextGraphic->setPropertyValue("AnchorType",
                                   uno::Any(text::TextContentAnchorType_AS_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xTextGraphic, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xTextContent, false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Picture", uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. <w:picture> was lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:picture", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDateContentControlExport)
{
    // Given a document with a date content control around a text portion:
    mxComponent = loadFromDesktop("private:factory/swriter");
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, "test", /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue("Date", uno::Any(true));
    xContentControlProps->setPropertyValue("DateFormat", uno::Any(OUString("M/d/yyyy")));
    xContentControlProps->setPropertyValue("DateLanguage", uno::Any(OUString("en-US")));
    xContentControlProps->setPropertyValue("CurrentDate", uno::Any(OUString("2022-05-26T00:00:00Z")));
    xContentControlProps->setPropertyValue("PlaceholderDocPart", uno::Any(OUString("DefaultPlaceholder_-1854013437")));
    xContentControlProps->setPropertyValue("DataBindingPrefixMappings", uno::Any(OUString("xmlns:ns0='http://schemas.microsoft.com/vsto/samples' ")));
    xContentControlProps->setPropertyValue("DataBindingXpath", uno::Any(OUString("/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]")));
    xContentControlProps->setPropertyValue("DataBindingStoreItemID", uno::Any(OUString("{241A8A02-7FFD-488D-8827-63FBE74E8BC9}")));
    xContentControlProps->setPropertyValue("Color", uno::Any(OUString("008000")));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save("Office Open XML Text", maTempFile);
    mbExported = true;

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // Without the fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:date/w:dateFormat' number of nodes is incorrect
    // i.e. the <w:date> was lost on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date/w:dateFormat", "val", "M/d/yyyy");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date/w:lid", "val", "en-US");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:date", "fullDate", "2022-05-26T00:00:00Z");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:placeholder/w:docPart", "val", "DefaultPlaceholder_-1854013437");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding", "prefixMappings", "xmlns:ns0='http://schemas.microsoft.com/vsto/samples' ");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding", "xpath", "/ns0:employees[1]/ns0:employee[1]/ns0:hireDate[1]");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:dataBinding", "storeItemID", "{241A8A02-7FFD-488D-8827-63FBE74E8BC9}");
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w15:color", "val", "008000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf137466, "tdf137466.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no futher checks

    // Ensure that we have <w:placeholder><w:docPart v:val="xxxx"/></w:placeholder>
    OUString sDocPart = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:placeholder/w:docPart", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("DefaultPlaceholder_-1854013440"), sDocPart);

    // Ensure that we have <w15:color v:val="xxxx"/>
    OUString sColor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w15:color", "val");
    CPPUNIT_ASSERT_EQUAL(OUString("FF0000"), sColor);
}

DECLARE_OOXMLEXPORT_TEST(testParaListRightIndent, "testParaListRightIndent.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2000), getProperty<sal_Int32>(getParagraph(1), "ParaRightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5001), getProperty<sal_Int32>(getParagraph(2), "ParaRightMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testDontAddNewStyles)
{
    // Given a document that lacks builtin styles, and addition of them is disabled:
    {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(true, pBatch);
        pBatch->commit();
    }
    comphelper::ScopeGuard g([] {
        std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Load::DisableBuiltinStyles::set(false, pBatch);
        pBatch->commit();
    });

    // When saving that document:
    loadAndSave("dont-add-new-styles.docx");

    // Then make sure that export doesn't have additional styles, Caption was one of them:
    xmlDocUniquePtr pXmlDoc = parseExport("word/styles.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. builtin styles were added to the export result, even if we opted out.
    assertXPath(pXmlDoc, "/w:styles/w:style[@w:styleId='Caption']", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf126287, "tdf126287.docx")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf123642_BookmarkAtDocEnd, "tdf123642.docx")
{
    // get bookmark interface
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();

    // check: we have 1 bookmark (previously there were 0)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Bookmark1"));

    // and it is really in exported DOCX (let's ensure)
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    CPPUNIT_ASSERT_EQUAL(OUString("Bookmark1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart[1]", "name"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148361, "tdf148361.docx")
{
    if (mbExported)
    {
        // Block SDT is turned into run SDT on export, so the next import will have this as content
        // control, not as a field.
        OUString aActual = getParagraph(1)->getString();
        // This was "itadmin".
        CPPUNIT_ASSERT_EQUAL(OUString("itadmin"), aActual);
    }
    else
    {
        // Refresh fields and ensure cross-reference to numbered para is okay
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());

        uno::Reference<text::XTextField> xTextField1(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("itadmin"), xTextField1->getPresentation(false));
    }

    OUString aActual = getParagraph(2)->getString();
    // This was "itadmin".
    CPPUNIT_ASSERT_EQUAL(OUString("[Type text]"), aActual);
}

DECLARE_OOXMLEXPORT_TEST(testTdf142407, "tdf142407.docx")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
    sal_Int16 nGridLines;
    xPageStyle->getPropertyValue("GridLines") >>= nGridLines;
    CPPUNIT_ASSERT_EQUAL( sal_Int16(36), nGridLines);   // was 23, left large space before text.
}

DECLARE_OOXMLEXPORT_TEST(testTdf146851_1, "tdf146851_1.docx")
{
    uno::Reference<beans::XPropertySet> xPara;

    xPara.set(getParagraph(1, "qwerty"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));

    xPara.set(getParagraph(2, "asdfg"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1/"), getProperty<OUString>(xPara, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf146851_2, "tdf146851_2.docx")
{
    // Ensure numbering on second para
    uno::Reference<beans::XPropertySet> xPara;
    xPara.set(getParagraph(2, "."), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Schedule"), getProperty<OUString>(xPara, "ListLabelString"));

    // Refresh fields and ensure cross-reference to numbered para is okay
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    uno::Reference<util::XRefreshable>(xFieldsAccess, uno::UNO_QUERY_THROW)->refresh();

    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Schedule"), xTextField->getPresentation(false));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148111, "tdf148111.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    std::vector<OUString> aExpectedValues = {
        // These field values are NOT in order in document: getTextFields did provide
        // fields in a strange but fixed order
        "Title", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Placeholder", "Placeholder", "Placeholder",
        "Placeholder", "Title", "Title", "Title",
        "Title", "Title", "Title", "Title"
    };

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(aExpectedValues[nIndex++], xTextField->getPresentation(false));
    }

    // No more fields
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

DECLARE_OOXMLEXPORT_TEST(testTdf81507, "tdf81507.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    // Ensure that we have <w:text w:multiLine="1"/>
    CPPUNIT_ASSERT_EQUAL(OUString("1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:sdt/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text w:multiLine="0"/>
    CPPUNIT_ASSERT_EQUAL(OUString("0"), getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:sdt/w:sdtPr/w:text", "multiLine"));

    // Ensure that we have <w:text/>
    getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:sdt/w:sdtPr/w:text", "");

    // Ensure that we have no <w:text/> (not quite correct case, but to ensure import/export are okay)
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "/w:document/w:body/w:p[4]/w:sdt/w:sdtPr/w:text");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                           static_cast<sal_Int32>(xmlXPathNodeSetGetLength(pXmlObj->nodesetval)));
    xmlXPathFreeObject(pXmlObj);
}

DECLARE_OOXMLEXPORT_TEST(testTdf139948, "tdf139948.docx")
{
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(1, "No border"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(2, "Border below"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(3, "Borders below and above"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(88),
        getProperty<table::BorderLine2>(getParagraph(4, "Border above"), "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(5, "No border"), "TopBorder").LineWidth);


    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(1), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(2), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(3), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(4), "BottomBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0),
        getProperty<table::BorderLine2>(getParagraph(5), "BottomBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf144563, "tdf144563.docx")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());

    // Refresh all cross-reference fields
    uno::Reference<util::XRefreshable>(xFieldsAccess, uno::UNO_QUERY_THROW)->refresh();

    // Verify values
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    std::vector<OUString> aExpectedValues = {
        // These field values are NOT in order in document: getTextFields did provide
        // fields in a strange but fixed order
        "1", "1", "1", "1", "1/", "1/", "1/", "1)", "1)", "1)", "1.)",
        "1.)", "1.)", "1..", "1..", "1..", "1.", "1.", "1.", "1", "1"
    };

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<text::XTextField> xTextField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(aExpectedValues[nIndex++], xTextField->getPresentation(false));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf144668, "tdf144668.odt")
{
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1, u"level1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[0001]"), getProperty<OUString>(xPara1, "ListLabelString"));

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2, u"level2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("[001]"), getProperty<OUString>(xPara2, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148455_1, "tdf148455_1.docx")
{
    uno::Reference<beans::XPropertySet> xPara2(getParagraph(3, u"1.1.1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1.1.1."), getProperty<OUString>(xPara2, "ListLabelString"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148455_2, "tdf148455_2.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
       return; // initial import, no further checks

    // Find list id for restarted list
    sal_Int32 nListId = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:numPr/w:numId", "val").toInt32();

    xmlDocUniquePtr pNumberingDoc = parseExport("word/numbering.xml");

    // Ensure we have empty lvlOverride for levels 0 - 1
    getXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='0']", "");
    getXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='1']", "");
    // And normal overrride for level 2
    getXPath(pNumberingDoc, "/w:numbering/w:num[@w:numId='" + OString::number(nListId) +"']/w:lvlOverride[@w:ilvl='2']/w:startOverride", "val");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147978enhancedPathABVW)
{
    load(DATA_DIRECTORY, "tdf147978_enhancedPath_commandABVW.odt");
    CPPUNIT_ASSERT(mxComponent);
    save("Office Open XML Text", maTempFile);
    mxComponent->dispose();
    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
    // Make sure the new implemented export for commands A,B,V and W use the correct arc between
    // the given two points, here the short one.
    for (sal_Int16 i = 1 ; i <= 4; ++i)
    {
        uno::Reference<drawing::XShape> xShape = getShape(i);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(506), getProperty<awt::Rectangle>(xShape, "BoundRect").Height);
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf148132, "tdf148132.docx")
{
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(1);
        auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
        // Get level 2 char style
        comphelper::SequenceAsHashMap levelProps(xLevels->getByIndex(1));
        OUString aCharStyleName = levelProps["CharStyleName"].get<OUString>();
        // Ensure that numbering in this paragraph is 24pt bold italic
        // Previously it got overriden by paragraph properties and became 6pt, no bold, no italic
        uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.f, getProperty<float>(xStyle, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
    }
    // And do the same for second paragraph. Numbering should be identical
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(2);
        auto xLevels = getProperty< uno::Reference<container::XIndexAccess> >(xParagraph, "NumberingRules");
        comphelper::SequenceAsHashMap levelProps(xLevels->getByIndex(1));
        OUString aCharStyleName = levelProps["CharStyleName"].get<OUString>();

        uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName(aCharStyleName), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(24.f, getProperty<float>(xStyle, "CharHeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xStyle, "CharWeight"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(xStyle, "CharPosture"));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149200)
{
    loadAndSave("tdf149200.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    // Ensure there is no unexpected invalid structure <w14:textFill>
    // There is just one run property
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, "count(/w:document/w:body/w:p[1]/w:r[1]/w:rPr/*)");
    CPPUNIT_ASSERT(pXmlObj);
    CPPUNIT_ASSERT_EQUAL(double(1), pXmlObj->floatval);
    // And it is a color definition with themeColor
    CPPUNIT_ASSERT_EQUAL(OUString("dark1"), getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[1]/w:rPr/w:color", "themeColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf148273_sectionBulletFormatLeak, "tdf148273_sectionBulletFormatLeak.docx")
{
    // get a paragraph with bullet point after section break
    uno::Reference<text::XTextRange> xParagraph = getParagraph(4);
    uno::Reference<beans::XPropertySet> xProps(xParagraph, uno::UNO_QUERY);

    // Make sure that the bullet has no ListAutoFormat inherited from
    // the empty paragraph before the section break
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. empty paragraph formats from the first section leaked to the bullet's formatting
    uno::Any aValue = xProps->getPropertyValue("ListAutoFormat");
    CPPUNIT_ASSERT_EQUAL(false, aValue.hasValue());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
