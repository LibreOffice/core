/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include <editeng/udlnitem.hxx>

#include <comphelper/propertyvalue.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <helper/qahelper.hxx>
#include <impex.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sc/source/filter/xml/ fixes.
class Test : public ScModelTestBase
{
public:
    Test()
        : ScModelTestBase(u"/sc/qa/filter/xml/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDatabaseLocationRoundTrip)
{
    createScDoc("sheet1.fods");

    save(TestFilter::ODS);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/"
                "table:database-ranges/table:database-range/table:database-source-query",
                "database-name", u"file:///path/to/Database.odb");
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
