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
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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

#include <test/unoapi_test.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <rtl/oustringostreaminserter.hxx>
#include <vector>

namespace ScDataPilotDescriptorBase
{

#define NUMBER_OF_TESTS 9

class ScXDataPilotDescriptor : public UnoApiTest
{
public:

    virtual void setUp();
    virtual void tearDown();

    void testSourceRange();
    void testTag();
    void testGetFilterDescriptor();
    void testGetDataPilotFields();
    void testGetColumnFields();
    void testGetRowFields();
    void testGetPageFields();
    void testGetDataFields();
    void testGetHiddenFields();
    CPPUNIT_TEST_SUITE(ScXDataPilotDescriptor);
    CPPUNIT_TEST(testSourceRange);
    CPPUNIT_TEST(testTag);
    CPPUNIT_TEST(testGetFilterDescriptor);
    CPPUNIT_TEST(testGetDataPilotFields);
    CPPUNIT_TEST(testGetColumnFields);
    CPPUNIT_TEST(testGetRowFields);
    CPPUNIT_TEST(testGetPageFields);
    CPPUNIT_TEST(testGetDataFields);
    CPPUNIT_TEST(testGetHiddenFields);
    CPPUNIT_TEST_SUITE_END();


private:

    uno::Reference< sheet::XDataPilotDescriptor > init();

    void testGetDataPilotFields_Impl( uno::Reference< sheet::XDataPilotDescriptor > xDescr );

    void checkName( uno::Reference< container::XIndexAccess > xIndex, sal_Int32 nIndex );
    static std::vector<rtl::OUString> maFieldNames;
    static int nTest;
    static uno::Reference< lang::XComponent > xComponent;
};

std::vector< rtl::OUString > ScXDataPilotDescriptor::maFieldNames;
int ScXDataPilotDescriptor::nTest = 0;
uno::Reference< lang::XComponent > ScXDataPilotDescriptor::xComponent;

void ScXDataPilotDescriptor::testTag()
{
    rtl::OUString aTag(RTL_CONSTASCII_USTRINGPARAM("DataPilotDescriptor_Tag"));
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    xDescr->setTag(aTag);
    rtl::OUString aNewTag = xDescr->getTag();
    CPPUNIT_ASSERT( aTag == aNewTag );
}

void ScXDataPilotDescriptor::testSourceRange()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    table::CellRangeAddress aOldAddress = xDescr->getSourceRange();

    table::CellRangeAddress aAddress;
    aAddress.Sheet = 1;
    aAddress.StartColumn = 1;
    aAddress.StartRow = 1;
    aAddress.EndColumn = 5;
    aAddress.EndRow = 5;
    xDescr->setSourceRange(aAddress);

    table::CellRangeAddress aReturn;
    aReturn = xDescr->getSourceRange();

    CPPUNIT_ASSERT(aAddress.Sheet == aReturn.Sheet);
    CPPUNIT_ASSERT(aAddress.StartColumn == aReturn.StartColumn);
    CPPUNIT_ASSERT(aAddress.StartRow == aReturn.StartRow);
    CPPUNIT_ASSERT(aAddress.EndColumn == aReturn.EndColumn);
    CPPUNIT_ASSERT(aAddress.EndRow == aReturn.EndRow);

    //restore old settings
    xDescr->setSourceRange(aOldAddress);
}

void ScXDataPilotDescriptor::testGetFilterDescriptor()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< sheet::XSheetFilterDescriptor > xSheetFilterDescr = xDescr->getFilterDescriptor();
    CPPUNIT_ASSERT(xSheetFilterDescr.is());
}

void ScXDataPilotDescriptor::testGetDataPilotFields_Impl( uno::Reference< sheet::XDataPilotDescriptor > xDescr)
{
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataPilotFields(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT( xIndex.is());

    sal_Int32 nCount = xIndex->getCount();

    rtl::OUString aOrientation(RTL_CONSTASCII_USTRINGPARAM("Orientation"));
    for (sal_Int32 i = 0; i < nCount && i < 5; ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex( i ), UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xNamed.is());
        rtl::OUString aName = xNamed->getName();
        maFieldNames.push_back(aName);
        CPPUNIT_ASSERT( !aName.equalsAscii("Data") );

        uno::Reference< beans::XPropertySet > xPropSet( xNamed, UNO_QUERY_THROW);
        CPPUNIT_ASSERT( xPropSet.is() );

        switch ( i % 5 )
        {
            case 0:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation::DataPilotFieldOrientation_COLUMN;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 1:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation::DataPilotFieldOrientation_ROW;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 2:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation::DataPilotFieldOrientation_DATA;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 3:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation::DataPilotFieldOrientation_HIDDEN;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 4:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation::DataPilotFieldOrientation_PAGE;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            default:
                break;
        }
    }
}

void ScXDataPilotDescriptor::testGetDataPilotFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    testGetDataPilotFields_Impl( xDescr );
}

void ScXDataPilotDescriptor::testGetColumnFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getColumnFields(), UNO_QUERY_THROW);

    checkName( xIndex, 0 );
}

void ScXDataPilotDescriptor::testGetRowFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getRowFields(), UNO_QUERY_THROW);

    //checkName( xIndex, 1 );
}

void ScXDataPilotDescriptor::testGetPageFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getPageFields(), UNO_QUERY_THROW);

    checkName( xIndex, 4 );
}

void ScXDataPilotDescriptor::testGetDataFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataFields(), UNO_QUERY_THROW);

    checkName( xIndex, 2 );
}

void ScXDataPilotDescriptor::testGetHiddenFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr = init();
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getHiddenFields(), UNO_QUERY_THROW);

    checkName( xIndex, 3 );
}

uno::Reference< sheet::XDataPilotDescriptor> ScXDataPilotDescriptor::init()
{
    rtl::OUString aFileURL;
    const rtl::OUString aFileBase(RTL_CONSTASCII_USTRINGPARAM("ScDataPilotTableObj.ods"));
    createFileURL(aFileBase, aFileURL);
    std::cout << rtl::OUStringToOString(aFileURL, RTL_TEXTENCODING_UTF8).getStr() << std::endl;
    if( !xComponent.is())
        xComponent = loadFromDesktop(aFileURL);
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Could not create interface of type XSpreadsheet", xSheet.is());
    uno::Reference< sheet::XDataPilotTablesSupplier > xDPTS(xSheet, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDPTS.is());
    uno::Reference< sheet::XDataPilotTables > xDPT = xDPTS->getDataPilotTables();
    CPPUNIT_ASSERT(xDPT.is());
    uno::Sequence<rtl::OUString> aElementNames = xDPT->getElementNames();

    uno::Reference< sheet::XDataPilotDescriptor > xDPDsc(xDPT->getByName(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataPilotTable"))),UNO_QUERY_THROW);

    testGetDataPilotFields_Impl( xDPDsc );

    CPPUNIT_ASSERT(xDPDsc.is());
    return xDPDsc;
}

void ScXDataPilotDescriptor::checkName( uno::Reference< container::XIndexAccess > xIndex, sal_Int32 nIndex )
{
    CPPUNIT_ASSERT(xIndex.is());
    CPPUNIT_ASSERT(maFieldNames.size() >= static_cast<size_t>(nIndex));

    for (sal_Int32 i = 0; i < xIndex->getCount(); ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex(i), UNO_QUERY_THROW);
        CPPUNIT_ASSERT( xNamed->getName() == maFieldNames[nIndex] );
    }
}

void ScXDataPilotDescriptor::setUp()
{
    nTest += 1;
    UnoApiTest::setUp();
}

void ScXDataPilotDescriptor::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        uno::Reference< util::XCloseable > xCloseable(xComponent, UNO_QUERY_THROW);
        xCloseable->close( false );
    }

    UnoApiTest::tearDown();

    if (nTest == NUMBER_OF_TESTS)
    {
        mxDesktop->terminate();
        uno::Reference< lang::XComponent>(m_xContext, UNO_QUERY_THROW)->dispose();
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXDataPilotDescriptor);

CPPUNIT_PLUGIN_IMPLEMENT();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
