/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/chart2/CurveStyle.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>

#include <com/sun/star/qa/XDumper.hpp>

#include <test/xmldiff.hxx>

#include <fstream>

class Chart2XShapeTest : public ChartTest
{
public:

    void testFdo75075();

    CPPUNIT_TEST_SUITE(Chart2XShapeTest);
    CPPUNIT_TEST(testFdo75075);
    CPPUNIT_TEST_SUITE_END();

private:

    void compareAgainstReference(const OUString& rReferenceFile, bool bCreateReference = false);

};

namespace {

bool checkDumpAgainstFile( const OUString& rDump, const OUString aFilePath)
{
    OString aOFile = OUStringToOString(aFilePath, RTL_TEXTENCODING_UTF8);

    CPPUNIT_ASSERT_MESSAGE("dump is empty", !rDump.isEmpty());

    OString aDump = OUStringToOString(rDump, RTL_TEXTENCODING_UTF8);
    return doXMLDiff(aOFile.getStr(), aDump.getStr(),
            static_cast<int>(rDump.getLength()), NULL);
}

}

void Chart2XShapeTest::compareAgainstReference(const OUString& rReferenceFile, bool bCreateReference)
{
    uno::Reference< chart::XChartDocument > xChartDoc ( getChartCompFromSheet( 0, mxComponent ), UNO_QUERY_THROW);
    uno::Reference< qa::XDumper > xDumper( xChartDoc, UNO_QUERY_THROW );
    OUString aDump = xDumper->dump();
    OUString aReference = getPathFromSrc("/chart2/qa/extras/xshape/data/reference/") + rReferenceFile;
    if(bCreateReference)
    {
        OString aOFile = OUStringToOString(aReference, RTL_TEXTENCODING_UTF8);
        OString aODump = OUStringToOString(aDump, RTL_TEXTENCODING_UTF8);
        std::ofstream aReferenceFile(aOFile.getStr());
        aReferenceFile << aODump.getStr();
    }
    else
    {
        checkDumpAgainstFile(aDump, aReference);
    }
}

void Chart2XShapeTest::testFdo75075()
{
    load("chart2/qa/extras/xshape/data/ods/", "fdo75075.ods");
    compareAgainstReference("fdo75075.xml");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2XShapeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
