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

#include <test/xmldocptr.hxx>

using namespace com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/SettingsTable.cxx.
class Test : public UnoApiXmlTest
{
public:
    Test()
        : UnoApiXmlTest("/writerfilter/qa/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDoNotBreakWrappedTables)
{
    // Given a document with <w:doNotBreakWrappedTables>:
    // When importing that document:
    loadFromURL(u"do-not-break-wrapped-tables.docx");

    // Then make sure that the matching compat flag is set:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    bool bDoNotBreakWrappedTables{};
    xSettings->getPropertyValue("DoNotBreakWrappedTables") >>= bDoNotBreakWrappedTables;
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bDoNotBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(Test, testAllowTextAfterFloatingTableBreak)
{
    // Given a document with <w:compatSetting w:name="allowTextAfterFloatingTableBreak">:
    // When importing that document:
    loadFromURL(u"floattable-wrap-on-all-pages.docx");

    // Then make sure that the matching compat flag is set:
    uno::Reference<lang::XMultiServiceFactory> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xDocument->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    bool bAllowTextAfterFloatingTableBreak{};
    xSettings->getPropertyValue("AllowTextAfterFloatingTableBreak")
        >>= bAllowTextAfterFloatingTableBreak;
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bAllowTextAfterFloatingTableBreak);
}

CPPUNIT_TEST_FIXTURE(Test, testAddVerticalFrameOffsetsRTF)
{
    // Given a document with a floating table, immediately followed by an inline table:
    // When importing that document:
    loadFromURL(u"floattable-vertical-frame-offset.rtf");

    // Then make sure the floating and the inline tables don't overlap:
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    css::uno::Reference<qa::XDumper> xDumper(xModel->getCurrentController(), uno::UNO_QUERY);
    OString aDump = xDumper->dump("layout").toUtf8();
    auto pCharBuffer = reinterpret_cast<const xmlChar*>(aDump.getStr());
    xmlDocUniquePtr pXmlDoc(xmlParseDoc(pCharBuffer));
    sal_Int32 nFlyBottom = getXPath(pXmlDoc, "//fly/infos/bounds", "bottom").toInt32();
    sal_Int32 nTableFrameTop = getXPath(pXmlDoc, "//body/tab/infos/bounds", "top").toInt32();
    sal_Int32 nTableTopMargin = getXPath(pXmlDoc, "//body/tab/infos/prtBounds", "top").toInt32();
    sal_Int32 nTableTop = nTableFrameTop + nTableTopMargin;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected greater than: 2747
    // - Actual  : 1449
    // i.e. table top should be ~2748, but was less, leading to an overlap.
    CPPUNIT_ASSERT_GREATER(nFlyBottom, nTableTop);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
