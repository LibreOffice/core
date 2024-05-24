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

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/DomainMapperTableManager.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTblOverlap)
{
    // Given a document with 2 floating tables, the second is not allowed to overlap:
    // When importing that document:
    loadFromFile(u"floattable-tbl-overlap.docx");

    // Then make sure the second table is marked as "can't overlap":
    uno::Reference<text::XTextFramesSupplier> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xTextDocument->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame(xFrames->getByIndex(1), uno::UNO_QUERY);
    bool bAllowOverlap{};
    CPPUNIT_ASSERT(xFrame->getPropertyValue("AllowOverlap") >>= bAllowOverlap);
    // Without the accompanying fix in place, this test would have failed, the tables were marked as
    // "can overlap".
    CPPUNIT_ASSERT(!bAllowOverlap);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
