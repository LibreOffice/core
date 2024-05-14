/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatapilotdescriptor.hxx>

#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppunit/TestAssert.h>

#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

std::vector< OUString > XDataPilotDescriptor::maFieldNames;

void XDataPilotDescriptor::testTag()
{
    OUString aTag(u"DataPilotDescriptor_Tag"_ustr);
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    xDescr->setTag(aTag);
    OUString aNewTag = xDescr->getTag();
    CPPUNIT_ASSERT_EQUAL( aTag, aNewTag );
}

void XDataPilotDescriptor::testSourceRange()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    table::CellRangeAddress aOldAddress = xDescr->getSourceRange();

    table::CellRangeAddress aAddress;
    aAddress.Sheet = 1;
    aAddress.StartColumn = 1;
    aAddress.StartRow = 1;
    aAddress.EndColumn = 5;
    aAddress.EndRow = 5;
    xDescr->setSourceRange(aAddress);

    table::CellRangeAddress aReturn = xDescr->getSourceRange();

    CPPUNIT_ASSERT_EQUAL(aAddress.Sheet, aReturn.Sheet);
    CPPUNIT_ASSERT_EQUAL(aAddress.StartColumn, aReturn.StartColumn);
    CPPUNIT_ASSERT_EQUAL(aAddress.StartRow, aReturn.StartRow);
    CPPUNIT_ASSERT_EQUAL(aAddress.EndColumn, aReturn.EndColumn);
    CPPUNIT_ASSERT_EQUAL(aAddress.EndRow, aReturn.EndRow);

    //restore old settings
    xDescr->setSourceRange(aOldAddress);
}

void XDataPilotDescriptor::testGetFilterDescriptor()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetFilterDescriptor > xSheetFilterDescr = xDescr->getFilterDescriptor();
    CPPUNIT_ASSERT(xSheetFilterDescr.is());
}

void XDataPilotDescriptor::testGetDataPilotFields_Impl( uno::Reference< sheet::XDataPilotDescriptor > const & xDescr)
{
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataPilotFields(), UNO_SET_THROW);

    sal_Int32 nCount = xIndex->getCount();

    OUString aOrientation(u"Orientation"_ustr);
    for (sal_Int32 i = 0; i < nCount && i < 5; ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex( i ), UNO_QUERY_THROW);
        OUString aName = xNamed->getName();
        maFieldNames.push_back(aName);
        CPPUNIT_ASSERT( aName != "Data" );

        uno::Reference< beans::XPropertySet > xPropSet( xNamed, UNO_QUERY_THROW);

        switch ( i % 5 )
        {
            case 0:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation_COLUMN;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 1:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation_ROW;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 2:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation_DATA;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 3:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation_HIDDEN;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
            case 4:
                {
                    uno::Any aAny;
                    aAny<<= sheet::DataPilotFieldOrientation_PAGE;
                    xPropSet->setPropertyValue(aOrientation, aAny);
                }
                break;
        }
    }
}

void XDataPilotDescriptor::testGetDataPilotFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
}

void XDataPilotDescriptor::testGetColumnFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getColumnFields(), UNO_SET_THROW);

    checkName( xIndex, 0 );
}

void XDataPilotDescriptor::testGetRowFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getRowFields(), UNO_SET_THROW);

    //checkName( xIndex, 1 );
}

void XDataPilotDescriptor::testGetPageFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getPageFields(), UNO_SET_THROW);

    checkName( xIndex, 4 );
}

void XDataPilotDescriptor::testGetDataFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataFields(), UNO_SET_THROW);

    checkName( xIndex, 2 );
}

void XDataPilotDescriptor::testGetHiddenFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getHiddenFields(), UNO_SET_THROW);

    checkName( xIndex, 3 );
}

void XDataPilotDescriptor::checkName( uno::Reference< container::XIndexAccess > const & xIndex, std::size_t nIndex )
{
    CPPUNIT_ASSERT(xIndex.is());
    CPPUNIT_ASSERT(maFieldNames.size() >= nIndex);

    for (sal_Int32 i = 0; i < xIndex->getCount(); ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex(i), UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(maFieldNames[nIndex], xNamed->getName());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
