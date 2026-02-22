/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star;

namespace
{
/// Covers xmloff/source/text/txtprmap.cxx fixes.
class Test : public UnoApiTest
{
public:
    Test();
};
}

Test::Test()
    : UnoApiTest(u"/xmloff/qa/unit/data/"_ustr)
{
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableWrapTextAtFlyStartExport)
{
    // Given a document with a floating table:
    loadFromURL(u"private:factory/swriter"_ustr);
    // Insert a table:
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"Rows"_ustr, static_cast<sal_Int32>(1)),
        comphelper::makePropertyValue(u"Columns"_ustr, static_cast<sal_Int32>(1)),
    };
    dispatchCommand(mxComponent, u".uno:InsertTable"_ustr, aArgs);
    // Select it:
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    // Wrap in a fly:
    aArgs = {
        comphelper::makePropertyValue(u"AnchorType"_ustr, static_cast<sal_uInt16>(0)),
    };
    dispatchCommand(mxComponent, u".uno:InsertFrame"_ustr, aArgs);
    // Mark it as a floating table that wraps on all pages:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    xFrame->setPropertyValue(u"IsSplitAllowed"_ustr, uno::Any(true));
    xFrame->setPropertyValue(u"WrapTextAtFlyStart"_ustr, uno::Any(true));

    // When saving to ODT:
    save(TestFilter::ODT);

    // Then make sure we write a floating table, that wraps on all pages:
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//style:graphic-properties' no attribute 'wrap-text-at-frame-start' exist
    // i.e. no floating table was exported.
    assertXPath(pXmlDoc, "//style:graphic-properties", "wrap-text-at-frame-start", u"true");
}

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableWrapTextAtFlyStartImport)
{
    // Given a document with a floating table + wrap on all pages
    // (loext:may-break-between-pages="true" and loext:wrap-text-at-frame-start="true"), when
    // importing that document:
    loadFromFile(u"floattable-wrap-all-pages2.fodt");

    // Then make sure that the matching text frame property is set:
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(
        xTextFramesSupplier->getTextFrames()->getByName(u"Frame1"_ustr), uno::UNO_QUERY);
    bool bWrapTextAtFlyStart = false;
    // Without the accompanying fix in place, this test would have failed, the property was false.
    xFrame->getPropertyValue(u"WrapTextAtFlyStart"_ustr) >>= bWrapTextAtFlyStart;
    CPPUNIT_ASSERT(bWrapTextAtFlyStart);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
