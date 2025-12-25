/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "helper/qahelper.hxx"

#include <patattr.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <generalfunction.hxx>
#include <dpcache.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <dputil.hxx>
#include <attrib.hxx>
#include <dpshttab.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <queryentry.hxx>
#include <queryparam.hxx>
#include <rtl/string.hxx>

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupBy.hpp>

using namespace ::com::sun::star;

class PivotTableFieldsAndItemsExport : public ScModelTestBase
{
public:
    PivotTableFieldsAndItemsExport()
        : ScModelTestBase(u"sc/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testTdf123225PivotTableRowColItems)
{
    createScDoc("ods/tdf123225_pivotTable_row_col_items.ods");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    /* Make sure that we have <rowItems> and <colItems> element under <pivotTableDefinition> after export of the .ods to .xlsx */
    /* otherwise:
      - Excel will fail to open the xlsx document, you will get an error message.
      - Excel will open the file without any errors but:
        * context menu -by right clicking on the pivot table- will have less or more items than it should have after "refresh".
        * if e.g. trying to sort the items you will get "Cannot determine which PivotTable field to sort by" warning.
    */

    // Row items <rowItems>

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);
    // check if <rowItems count="8">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"8");
    // check if <rowItems> has enough <i> depending on count attribute value
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 8);
    // check if first <i> has single <x/> element
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 1);
    // check if <x/> of the first <i> element, exists (note the v=0 is default)
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", "v");

    // Column items <colItems>

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    // check if <colItems count="5">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"5");
    // check if <colItems> has enough <i> depending on count attribute value
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 5);
    // check if first <i> has single <x/> element
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 1);
    // check if <x/> of the first <i> element, exists (note the v=0 is default)
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", "v");
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testTdf123225_CheckGrandTotalIsSet)
{
    createScDoc("ods/tdf123225_pivotTable_no_col_items.ods");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    /* If there is a grand (Total result) there needs to be a row/column item with t="grand"*/

    // Row items <rowItems>

    // check if <rowItems count="3">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"3");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 3);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"2");

    // check if last <i> element, has t="grand"
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]", "t", u"grand");
    // check <x> element
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 1);

    // Column items <colItems>

    // check if <colItems count="1">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    // check if <colItems> has only a single <i> element
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    // check there is no attribute "t"
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", "t");
    // check that it doesn't contain any <x> elements
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testTdf123225PivotTableEmptyRowColItems)
{
    // this doc contains blank row/col items
    // <sharedItems containsBlank="1" count="..."> (xl/pivotCache/pivotCacheDefinition1.xml)
    createScDoc("ods/tdf123225_pivotTable_empty_row_col_items.ods");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    // Row items <rowItems>

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);
    // check if <rowItems count="4">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"4");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 4);

    // Column items <colItems>

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    // check if <colItems count="5">
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"5");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 5);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row1)
{
    /* Input document rowItems:
      <rowItems count="5">
        <i> <x /> <x v="2" /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i> <x v="2" /> <x v="1" /> </i>
        <i> <x v="3" /> <x /> </i>
        <i> <x v="4" /> <x v="2" /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot1_Row.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"5");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 5);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"3");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"4");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row1_Grand)
{
    /* Input document rowItems:
      <rowItems count="6">
        <i> <x /> <x v="2" /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i> <x v="2" /> <x v="1" /> </i>
        <i> <x v="3" /> <x /> </i>
        <i> <x v="4" /> <x v="2" /> </i>
        <i t="grand"> <x /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot1_Row_Grand.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"6");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 6);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"3");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"4");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "t", u"grand");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport,
                     testPivotTable_RowColumnItems_Row1_Grand_Subtotals)
{
    /* Input document rowItems:
      <rowItems count="11">
        <i> <x /> <x v="2" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x v="1" /> </i>
        <i t="default"><x v="2" /> </i>
        <i> <x v="3" /> <x /> </i>
        <i t="default"> <x v="3" /> </i>
        <i> <x v="4" /> <x v="2" /> </i>
        <i t="default"> <x v="4" /> </i>
        <i t="grand"> <x /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot1_Row_Grand_Subtotals.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"11");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 11);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[10]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[11]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[1]", "v", u"3");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]/x:x[1]", "v", u"4");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[10]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[10]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[10]/x:x", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[11]", "t", u"grand");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[11]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[11]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row2)
{
    /* Input document rowItems:
      <rowItems count="5">
        <i>  <x /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i> <x v="2" /> <x v="3" /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"5");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 5);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row2_Grand)
{
    /* Input document rowItems:
      <rowItems count="6">
        <i> <x /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i> <x v="2" /> <x v="3" /> </i>
        <i t="grand"> <x /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row_Grand.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"6");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 6);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "t", u"grand");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row2_Subtotals)
{
    /* Input document rowItems:
      <rowItems count="8">
        <i> <x /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x v="3" /> </i>
        <i t="default"> <x v="2" /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row_Subtotals.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"8");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 8);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]", "t", u"default");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport,
                     testPivotTable_RowColumnItems_Row2_Grand_Subtotals)
{
    /* Input document rowItems:
      <rowItems count="9">
        <i> <x /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x v="3" /> </i>
        <i t="default"> <x v="2" /> </i>
        <i t="grand"> <x /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row_Grand_Subtotals.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"9");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 9);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]", "t", u"grand");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[9]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport, testPivotTable_RowColumnItems_Row2_Compact)
{
    /* Input document rowItems:
      <rowItems count="8">
        <i> <x /> </i>
        <i r="1"> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i> <x v="1" /> </i>
        <i r="1"> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i> <x v="2" /> </i>
        <i r="1"> <x v="3" /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row_Compact.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"8");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 8);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 1);
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "r", u"1");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x[1]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x[1]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x[1]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport,
                     testPivotTable_RowColumnItems_Row2_Subtotals_SortDescendingAll)
{
    /* Input document rowItems:
      <rowItems count="8">
        <i> <x /> <x v="1" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="2" /> </i>
        <i r="1"> <x v="3" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i t="default"> <x v="2" /> </i>
      </rowItems>
    */
    createScDoc("xlsx/pivot/Pivot2_Row_Subtotals_SortDescendingAll.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"8");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 8);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[1]", "v");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]", "t", u"default");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[2]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[3]/x:x[2]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[4]/x:x[1]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[5]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x[1]", "v", u"2");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[6]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[7]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[8]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 0);
}

CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport,
                     testPivotTable_RowColumnItems_Column3_Grand_Subtotals)
{
    /* Input colItems should be the same as output
      <colItems count="9">
        <i> <x /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x v="3" /> </i>
        <i t="default"> <x v="2" /> </i>
        <i t="grand"> <x /> </i>
      </colItems>
    */
    createScDoc("xlsx/pivot/Pivot3_Column_Grand_Subtotals.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 0);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"9");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 9);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x[1]", "v");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]", "t", u"default");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]/x:x[1]", "v", u"2");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]", "t", u"grand");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]/x:x", "v");
}
CPPUNIT_TEST_FIXTURE(PivotTableFieldsAndItemsExport,
                     testPivotTable_RowColumnItems_Column4_Grand_Subtotals_SortDescending)
{
    /* Input colItems should be the same as output
      <colItems count="9">
        <i> <x /> <x v="3" /> </i>
        <i t="default"> <x /> </i>
        <i> <x v="1" /> <x v="1" /> </i>
        <i r="1"> <x v="2" /> </i>
        <i t="default"> <x v="1" /> </i>
        <i> <x v="2" /> <x /> </i>
        <i r="1"> <x v="4" /> </i>
        <i t="default"> <x v="2" /> </i>
        <i t="grand"> <x /> </i>
      </colItems>
    */
    createScDoc("xlsx/pivot/Pivot4_Column_Grand_Subtotals_SortDescending.xlsx");
    save(TestFilter::XLSX);

    xmlDocUniquePtr pSheet = parseExport(u"xl/pivotTables/pivotTable1.xml"_ustr);
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems", "count", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:rowItems/x:i[1]/x:x", 0);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems", "count", u"9");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i", 9);

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]/x:x", 2);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]/x:x", 1);
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]/x:x", 1);

    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x[1]", "v");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[1]/x:x[2]", "v", u"3");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]", "t", u"default");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[2]/x:x", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]/x:x[1]", "v", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[3]/x:x[2]", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[4]/x:x[1]", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[5]/x:x", "v", u"1");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]/x:x[1]", "v", u"2");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[6]/x:x[2]", "v");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]", "r", u"1");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[7]/x:x[1]", "v", u"4");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]", "t", u"default");
    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[8]/x:x", "v", u"2");

    assertXPath(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]", "t", u"grand");
    assertXPathNoAttribute(pSheet, "/x:pivotTableDefinition/x:colItems/x:i[9]/x:x", "v");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
