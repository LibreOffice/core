/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTableCharts.hpp>
#include <com/sun/star/table/XTableChart.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/qa/XDumper.hpp>

#include <test/xmldiff.hxx>

#include <basic/sbxdef.hxx>

#include "docsh.hxx"
#include "patattr.hxx"
#include "scitems.hxx"
#include "document.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

bool checkDumpAgainstFile( const OUString& rDump, const OUString aFilePath, const OUString& rToleranceFile)
{
    OString aOFile = OUStringToOString(aFilePath, RTL_TEXTENCODING_UTF8);
    OString aToleranceFile = OUStringToOString(rToleranceFile, RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_MESSAGE("dump is empty", !rDump.isEmpty());

    OString aDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return doXMLDiff(aOFile.getStr(), aDump.getStr(),
            static_cast<int>(rDump.getLength()), aToleranceFile.getStr());
}

}

class ScChartRegressionTest : public UnoApiTest
{
public:
    ScChartRegressionTest();

    void test();

    CPPUNIT_TEST_SUITE(ScChartRegressionTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

};

void ScChartRegressionTest::test()
{
    OUString aFileName;
    createFileURL( "testChart.ods", aFileName);
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(0), UNO_QUERY_THROW);

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW );

    uno::Reference< chart2::XChartDocument > xChartDoc ( xChartComp, UNO_QUERY_THROW );

    uno::Reference< qa::XDumper > xDumper( xChartDoc, UNO_QUERY_THROW );

    OUString aDump = xDumper->dump();
    std::cout << aDump;
    bool bCompare = checkDumpAgainstFile( aDump, getPathFromSrc("/chart2/qa/unit/data/reference/testChart.xml"), getPathFromSrc("/chart2/qa/unit/data/tolerance.xml") );
    CPPUNIT_ASSERT(bCompare);
}

ScChartRegressionTest::ScChartRegressionTest()
      : UnoApiTest("/chart2/qa/unit/data/ods")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChartRegressionTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
