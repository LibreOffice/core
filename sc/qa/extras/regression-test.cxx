/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2010 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/intitem.hxx>

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

bool checkDumpAgainstFile( const rtl::OUString& rDump, const rtl::OUString aFilePath, const rtl::OUString& rToleranceFile)
{
    rtl::OString aOFile = rtl::OUStringToOString(aFilePath, RTL_TEXTENCODING_UTF8);
    rtl::OString aToleranceFile = rtl::OUStringToOString(rToleranceFile, RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_MESSAGE("dump is empty", !rDump.isEmpty());

    rtl::OString aDump = rtl::OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return doXMLDiff(aOFile.getStr(), aDump.getStr(),
            static_cast<int>(rDump.getLength()), aToleranceFile.getStr());
}

}

class ScChartRegressionTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    ScChartRegressionTest();

    void createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

    void test();

    CPPUNIT_TEST_SUITE(ScChartRegressionTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
    rtl::OUString m_aBaseString;
};


void ScChartRegressionTest::createFileURL(const rtl::OUString& aFileBase, const rtl::OUString& aFileExtension, rtl::OUString& rFilePath)
{
    rtl::OUString aSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    rtl::OUStringBuffer aBuffer( getSrcRootURL() );
    aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
    aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
    rFilePath = aBuffer.makeStringAndClear();
}

void ScChartRegressionTest::test()
{
    rtl::OUString aFileName;
    createFileURL( "testChart.", "ods", aFileName);
    uno::Reference< com::sun::star::lang::XComponent > xComponent = loadFromDesktop(aFileName);

    CPPUNIT_ASSERT(xComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(xComponent, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDoc.is());

    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xIA.is());

    uno::Reference< table::XTableChartsSupplier > xChartSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartSupplier.is());

    uno::Reference< table::XTableCharts > xCharts = xChartSupplier->getCharts();
    CPPUNIT_ASSERT(xCharts.is());

    uno::Reference< container::XIndexAccess > xIACharts(xCharts, UNO_QUERY_THROW);
    uno::Reference< table::XTableChart > xChart( xIACharts->getByIndex(0), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChart.is());

    uno::Reference< document::XEmbeddedObjectSupplier > xEmbObjectSupplier(xChart, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xEmbObjectSupplier.is());

    uno::Reference< lang::XComponent > xChartComp( xEmbObjectSupplier->getEmbeddedObject(), UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartComp.is());

    uno::Reference< chart2::XChartDocument > xChartDoc ( xChartComp, UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xChartDoc.is());

    uno::Reference< qa::XDumper > xDumper( xChartDoc, UNO_QUERY_THROW );
    CPPUNIT_ASSERT(xDumper.is());

    rtl::OUString aDump = xDumper->dump();
    std::cout << aDump;
    bool bCompare = checkDumpAgainstFile( aDump, getPathFromSrc("/chart2/qa/unit/data/reference/testChart.xml"), getPathFromSrc("/chart2/qa/unit/data/tolerance.xml") );
    CPPUNIT_ASSERT(bCompare);
}

ScChartRegressionTest::ScChartRegressionTest()
      : m_aBaseString(RTL_CONSTASCII_USTRINGPARAM("/chart2/qa/unit/data"))
{
}

void ScChartRegressionTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.SpreadsheetDocument")));
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
    mxDesktop = Reference<com::sun::star::frame::XDesktop>( getMultiServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ))), UNO_QUERY );
    CPPUNIT_ASSERT(mxDesktop.is());
}

void ScChartRegressionTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScChartRegressionTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
