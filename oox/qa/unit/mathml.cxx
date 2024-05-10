/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

using namespace ::com::sun::star;

/// oox mathml tests.
class OoxMathmlTest : public UnoApiTest
{
public:
    OoxMathmlTest()
        : UnoApiTest(u"/oox/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(OoxMathmlTest, testImportCharacters)
{
    // Without the accompanying fix in place, this failed with an assertion failure on import.
    loadFromFile(u"import-characters.pptx");
}

CPPUNIT_TEST_FIXTURE(OoxMathmlTest, testImportMce)
{
    loadFromFile(u"import-mce.pptx");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both the math object and its replacement image was imported, as two separate objects.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
