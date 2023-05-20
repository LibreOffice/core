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
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>

#include <docsh.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf77964)
{
    loadAndReload("tdf77964.doc");
    // both images were loading as AT_PARA instead of AS_CHAR. Image2 visually had text wrapping.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShapeByName(u"Image2"), "AnchorType"));
}

DECLARE_WW8EXPORT_TEST(testTdf150197_anlv2ListFormat, "tdf150197_anlv2ListFormat.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(getParagraph(2), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL(OUString("2."), getProperty<OUString>(getParagraph(3), "ListLabelString"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Did you fix me? I should be 2.1", OUString("4.1"),
                                 getProperty<OUString>(getParagraph(4), "ListLabelString"));
}

DECLARE_WW8EXPORT_TEST(testTdf117994_CRnumformatting, "tdf117994_CRnumformatting.doc")
{
    CPPUNIT_ASSERT_EQUAL(OUString("1."), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']", "expand"));
    //Without this fix in place, it would become 200 (and non-bold).
    CPPUNIT_ASSERT_EQUAL(OUString("160"), parseDump("//body/txt[1]/SwParaPortion/SwLineLayout/child::*[@type='PortionType::Number']/SwFont", "height"));
}

DECLARE_WW8EXPORT_TEST(testTdf151548_formFieldMacros, "tdf151548_formFieldMacros.doc")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    for(auto aIter = pMarkAccess->getFieldmarksBegin(); aIter != pMarkAccess->getFieldmarksEnd(); ++aIter)
    {
        const OUString sName = (*aIter)->GetName();
        CPPUNIT_ASSERT(sName == "Check1" || sName == "Check2" || sName == "Text1" || sName == "Dropdown1");
    }
}

DECLARE_WW8EXPORT_TEST(testTdf141649_conditionalText, "tdf141649_conditionalText.doc")
{
    // In MS Word, the IF field is editable and requires manual update, so the most correct
    // result is "manual refresh with F9" inside a text field,
    // but for our purposes, a single instance of "trueResult" is appropriate.
    getParagraph(1, "trueResult");
}

DECLARE_WW8EXPORT_TEST(testTdf90408, "tdf90408.doc")
{
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 16pt", 16.f, getProperty<float>(xRun, "CharHeight"));
    xRun.set(getRun(getParagraph(1), 2, "unchecked"), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 12pt", 12.f, getProperty<float>(xRun, "CharHeight"));
}

DECLARE_WW8EXPORT_TEST(testTdf90408B, "tdf90408B.doc")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    uno::Reference<beans::XPropertySet> xRun(getRun(xPara, 1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("checkbox is 28pt", 28.f, getProperty<float>(xRun, "CharHeight"));
    xRun.set(getRun(xPara, 2, u" Κατάψυξη,  "), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("text is 10pt", 10.f, getProperty<float>(xRun, "CharHeight"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
