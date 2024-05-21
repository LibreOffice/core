/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/sheet/databaseimportdescriptor.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/util/XImportable.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScImportDescriptorBaseObj : public UnoApiTest, public apitest::DatabaseImportDescriptor
{
public:
    ScImportDescriptorBaseObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Reference<uno::XInterface> getXImportable() override;

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScImportDescriptorBaseObj);

    // DatabaseImportDescriptor
    CPPUNIT_TEST(testDatabaseImportDescriptorProperties);

    CPPUNIT_TEST_SUITE_END();
};

ScImportDescriptorBaseObj::ScImportDescriptorBaseObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScImportDescriptorBaseObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
    return xSheet;
}

uno::Reference<uno::XInterface> ScImportDescriptorBaseObj::getXImportable()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<util::XImportable> XImportable(xSheet, UNO_QUERY_THROW);
    return XImportable;
}

void ScImportDescriptorBaseObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScImportDescriptorBaseObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
