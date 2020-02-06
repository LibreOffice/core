/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/DomainMapperTableHandler.cxx.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, test1cellInsidevRightborder)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "1cell-insidev-rightborder.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<text::XTextTablesSupplier> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    table::BorderLine2 aBorder;
    xCell->getPropertyValue("RightBorder") >>= aBorder;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 18
    // i.e. the request to have no table-level right border was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), aBorder.LineWidth);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
