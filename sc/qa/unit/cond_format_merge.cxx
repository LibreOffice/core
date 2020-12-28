/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XConditionalFormats.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <test/calc_unoapi_test.hxx>

namespace com::sun::star::lang
{
class XComponent;
}

using namespace css;

class ScCondFormatMergeTest : public CalcUnoApiTest
{
public:
    ScCondFormatMergeTest();

    void testCondFormatMerge();

    CPPUNIT_TEST_SUITE(ScCondFormatMergeTest);
    CPPUNIT_TEST(testCondFormatMerge);
    CPPUNIT_TEST_SUITE_END();
};

ScCondFormatMergeTest::ScCondFormatMergeTest()
    : CalcUnoApiTest("sc/qa/extras/testdocuments/")
{
}

void ScCondFormatMergeTest::testCondFormatMerge()
{
    OUString aFileURL;
    createFileURL(u"cond_format_merge.ods", aFileURL);
    uno::Reference<lang::XComponent> mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT_MESSAGE("Component not loaded", mxComponent.is());

    // get the first sheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
    uno::Any aAny = xProps->getPropertyValue("ConditionalFormats");
    uno::Reference<sheet::XConditionalFormats> xCondFormats;

    CPPUNIT_ASSERT(aAny >>= xCondFormats);
    CPPUNIT_ASSERT(xCondFormats.is());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xCondFormats->getLength());

    uno::Sequence<uno::Reference<sheet::XConditionalFormat>> xCondFormatSeq
        = xCondFormats->getConditionalFormats();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), xCondFormatSeq.getLength());

    int nRanges = 0;
    for (sal_Int32 i = 0, n = xCondFormatSeq.getLength(); i < n; ++i)
    {
        CPPUNIT_ASSERT(xCondFormatSeq[i].is());

        uno::Reference<sheet::XConditionalFormat> xCondFormat = xCondFormatSeq[i];
        CPPUNIT_ASSERT(xCondFormat.is());

        uno::Reference<beans::XPropertySet> xPropSet(xCondFormat, uno::UNO_QUERY_THROW);

        aAny = xPropSet->getPropertyValue("Range");
        uno::Reference<sheet::XSheetCellRanges> xCellRanges;
        CPPUNIT_ASSERT(aAny >>= xCellRanges);
        CPPUNIT_ASSERT(xCellRanges.is());

        uno::Sequence<table::CellRangeAddress> aRanges = xCellRanges->getRangeAddresses();
        CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(1), aRanges.getLength());

        table::CellRangeAddress aRange0 = aRanges[0];
        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aRange0.Sheet);
        CPPUNIT_ASSERT_EQUAL(aRange0.StartColumn, aRange0.EndColumn);

        table::CellRangeAddress aRange1;

        switch (aRange0.StartColumn)
        {
            case 3:
                switch (aRange0.StartRow)
                {
                    case 0: // D1:D2,D5::D8
                        nRanges++;
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRange0.EndRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRanges.getLength());
                        aRange1 = aRanges[1];
                        CPPUNIT_ASSERT_EQUAL(sal_Int16(0), aRange1.Sheet);
                        CPPUNIT_ASSERT_EQUAL(aRange1.StartColumn, aRange1.EndColumn);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRange1.StartColumn);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRange1.StartRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(7), aRange1.EndRow);
                        break;
                    default:
                        CPPUNIT_FAIL("Unexpected range in column D");
                }
                break;
            case 5:
                switch (aRange0.StartRow)
                {
                    case 0: // F1:F2
                        nRanges++;
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRange0.EndRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRanges.getLength());
                        break;
                    case 2: // F3
                        nRanges++;
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aRange0.EndRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRanges.getLength());
                        break;
                    case 3: // F4
                        nRanges++;
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), aRange0.EndRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRanges.getLength());
                        break;
                    case 4: // F5
                        nRanges++;
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(4), aRange0.EndRow);
                        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aRanges.getLength());
                        break;
                    default:
                        CPPUNIT_FAIL("Unexpected range in column F");
                }
                break;
            default:
                CPPUNIT_FAIL("Unexpected range");
        }
    }

    CPPUNIT_ASSERT_EQUAL(5, nRanges);

    closeDocument(mxComponent);
    mxComponent.clear();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCondFormatMergeTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
