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
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers xmloff/source/text/txtprmap.cxx fixes.
class Test : public UnoApiXmlTest
{
public:
    Test();
};
}

Test::Test()
    : UnoApiXmlTest("/xmloff/qa/unit/data/")
{
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableWrapTextAtFlyStartExport)
{
    // Given a document with a floating table:
    mxComponent = loadFromDesktop("private:factory/swriter");
    // Insert a table:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("Rows", static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue("Columns", static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    // Select it:
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    // Wrap in a fly:
    aArgs = {
        comphelper::makePropertyValue("AnchorType", static_cast<sal_uInt16>(0)),
    };
    dispatchCommand(mxComponent, ".uno:InsertFrame", aArgs);
    // Mark it as a floating table that wraps on all pages:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName("Frame1"), uno::UNO_QUERY);
    xFrame->setPropertyValue("IsSplitAllowed", uno::Any(true));
    xFrame->setPropertyValue("WrapTextAtFlyStart", uno::Any(true));

    // When saving to ODT:
    save("writer8");

    // Then make sure we write a floating table, that wraps on all pages:
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//style:graphic-properties' no attribute 'wrap-text-at-frame-start' exist
    // i.e. no floating table was exported.
    assertXPath(pXmlDoc, "//style:graphic-properties", "wrap-text-at-frame-start", "true");
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableWrapTextAtFlyStartImport)
{
    // Given a document with a floating table + wrap on all pages
    // (loext:may-break-between-pages="true" and loext:wrap-text-at-frame-start="true"), when
    // importing that document:
    loadFromURL(u"floattable-wrap-all-pages2.fodt");

    // Then make sure that the matching text frame property is set:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName("Frame1"), uno::UNO_QUERY);
    bool bWrapTextAtFlyStart = false;
    // Without the accompanying fix in place, this test would have failed, the property was false.
    xFrame->getPropertyValue("WrapTextAtFlyStart") >>= bWrapTextAtFlyStart;
    CPPUNIT_ASSERT(bWrapTextAtFlyStart);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
