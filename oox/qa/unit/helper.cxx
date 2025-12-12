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
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>

using namespace ::com::sun::star;

namespace
{
/// Covers oox/source/helper/ fixes.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/oox/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testImportTifCrop)
{
    // Given a cropped .TIF file in a document:
    // When loading that document:
    loadFromFile(u"tif-crop.docx");

    // Then make sure the result has the correct crop rectangle:
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    text::GraphicCrop aCrop;
    xShape->getPropertyValue(u"GraphicCrop"_ustr) >>= aCrop;
    // Without the accompanying fix in place, this failed with:
    // - Expected: 3928
    // - Actual  : 24553
    // i.e. lazy-loading lead to some unexpected crop rectangle, the image was not readable.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3928), aCrop.Top);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6660), aCrop.Bottom);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1732), aCrop.Left);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1145), aCrop.Right);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
