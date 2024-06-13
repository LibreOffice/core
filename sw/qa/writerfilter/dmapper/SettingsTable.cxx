/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/qa/XDumper.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <test/xmldocptr.hxx>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/SettingsTable.cxx.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDoNotBreakWrappedTables)
{
    // Given a document with <w:doNotBreakWrappedTables>:
    // When importing that document:
    loadFromFile(u"do-not-break-wrapped-tables.docx");

    // Then make sure that the matching compat flag is set:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bDoNotBreakWrappedTables{};
    xSettings->getPropertyValue(u"DoNotBreakWrappedTables"_ustr) >>= bDoNotBreakWrappedTables;
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bDoNotBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(Test, testAllowTextAfterFloatingTableBreak)
{
    // Given a document with <w:compatSetting w:name="allowTextAfterFloatingTableBreak">:
    // When importing that document:
    loadFromFile(u"floattable-wrap-on-all-pages.docx");

    // Then make sure that the matching compat flag is set:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    bool bAllowTextAfterFloatingTableBreak{};
    xSettings->getPropertyValue(u"AllowTextAfterFloatingTableBreak"_ustr)
        >>= bAllowTextAfterFloatingTableBreak;
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bAllowTextAfterFloatingTableBreak);
}

CPPUNIT_TEST_FIXTURE(Test, testAddVerticalFrameOffsetsRTF)
{
    // Given a document with a floating table, immediately followed by an inline table:
    // When importing that document:
    loadFromFile(u"floattable-vertical-frame-offset.rtf");

    // Then make sure the floating and the inline tables don't overlap:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<qa::XDumper> xDumper(xModel->getCurrentController(), uno::UNO_QUERY);
    OString aDump = xDumper->dump(u"layout"_ustr).toUtf8();
    auto pCharBuffer = reinterpret_cast<const xmlChar*>(aDump.getStr());
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(pCharBuffer));
    sal_Int32 nFlyBottom = getXPath(pXmlDoc, "//fly/infos/bounds"_ostr, "bottom"_ostr).toInt32();
    sal_Int32 nTableFrameTop
        = getXPath(pXmlDoc, "//body/tab/infos/bounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nTableTopMargin
        = getXPath(pXmlDoc, "//body/tab/infos/prtBounds"_ostr, "top"_ostr).toInt32();
    sal_Int32 nTableTop = nTableFrameTop + nTableTopMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 2747
    // - Actual  : 1449
    // i.e. table top should be ~2748, but was less, leading to an overlap.
    CPPUNIT_ASSERT_GREATER(nFlyBottom, nTableTop);
}

CPPUNIT_TEST_FIXTURE(Test, testEndnoteAtSectionEnd)
{
    // Given a document with at-section-end endnotes enabled:
    loadFromFile(u"endnote-at-section-end.docx");

    // Go to the second paragraph, which is inside Word's second section:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSection;
    xPara->getPropertyValue("TextSection") >>= xSection;
    bool bEndnoteIsCollectAtTextEnd = false;
    xSection->getPropertyValue("EndnoteIsCollectAtTextEnd") >>= bEndnoteIsCollectAtTextEnd;
    // Without the accompanying fix in place, this test would have failed, endnotes were always at
    // document end.
    CPPUNIT_ASSERT(bEndnoteIsCollectAtTextEnd);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
