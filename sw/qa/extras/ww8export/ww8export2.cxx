/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <pagedesc.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97") {}

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".doc");
    }

};
DECLARE_WW8EXPORT_TEST(testTdf41542_borderlessPadding, "tdf41542_borderlessPadding.odt")
{
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}


DECLARE_WW8EXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.doc")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
