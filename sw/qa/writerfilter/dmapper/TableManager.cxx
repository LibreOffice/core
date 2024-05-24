/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextFramesSupplier.hpp>

using namespace com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/TableManager.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloattableNestedCellStartDOCXImport)
{
    // Given a document with a nested floating table at cell start and an other inner floating table:
    // When importing that document:
    loadFromFile(u"floattable-nested-cellstart.docx");

    // Then make sure that both inner tables are floating:
    uno::Reference<text::XTextFramesSupplier> xFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xFramesSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the first inner table was not floating.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xFrames->getCount());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
