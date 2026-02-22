/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sheet/DataImportMode.hpp>
#include <address.hxx>
#include <cellsuno.hxx>
#include <docsh.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;
using namespace com::sun::star;

namespace sc_apitest
{
class ScImportDescriptorCellRangeObj : public UnoApiTest
{
public:
    ScImportDescriptorCellRangeObj()
        : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
    {
    }

    void testDatabaseLocation()
    {
        loadFromFile(u"tdf139467.fods");

        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShell* xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
        CPPUNIT_ASSERT(xDocSh);

        // Ensure that a cell range import descriptor for a range with an embedded
        // database query that contains a local file URL database-name survives the
        // import process
        ScRange aRange(0, 0, 0, 2, 6, 0);
        rtl::Reference<ScCellRangeObj> aCellRange = new ScCellRangeObj(xDocSh, aRange);
        uno::Sequence<beans::PropertyValue> aPropValues = aCellRange->createImportDescriptor(false);

        bool found = false;
        for (auto& rPropValue : aPropValues)
        {
            if (rPropValue.Name == "DatabaseName")
            {
                OUString aValue;
                rPropValue.Value >>= aValue;
                CPPUNIT_ASSERT_EQUAL(u"file:///path/to/Database.odb"_ustr, aValue);
                found = true;
            }
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("DatabaseName property not found", true, found);
    }

    CPPUNIT_TEST_SUITE(ScImportDescriptorCellRangeObj);
    CPPUNIT_TEST(testDatabaseLocation);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ScImportDescriptorCellRangeObj);

} // end namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
