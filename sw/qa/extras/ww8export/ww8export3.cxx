/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>

#include <drawdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }
};
DECLARE_WW8EXPORT_TEST(testTdf94009_zeroPgMargin, "tdf94009_zeroPgMargin.odt")
{
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(defaultStyle, "TopMargin"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
