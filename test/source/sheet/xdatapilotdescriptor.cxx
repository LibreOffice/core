/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/sheet/xdatapilotdescriptor.hxx"

#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "cppunit/extensions/HelperMacros.h"

#include <rtl/ustring.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

std::vector< OUString > XDataPilotDescriptor::maFieldNames;

void XDataPilotDescriptor::testTag()
{
    OUString aTag("DataPilotDescriptor_Tag");
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    xDescr->setTag(aTag);
    OUString aNewTag = xDescr->getTag();
    CPPUNIT_ASSERT( aTag == aNewTag );
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

void XDataPilotDescriptor::testGetFilterDescriptor()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetFilterDescriptor > xSheetFilterDescr = xDescr->getFilterDescriptor();
    CPPUNIT_ASSERT(xSheetFilterDescr.is());
}

void XDataPilotDescriptor::testGetDataPilotFields_Impl( uno::Reference< sheet::XDataPilotDescriptor > xDescr)
{
    //this method should only be called once but needs to be called before any of the other tests
    static bool bCalled = false;
    if (bCalled)
        return;
    else
        bCalled = true;

    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataPilotFields(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT( xIndex.is());

    sal_Int32 nCount = xIndex->getCount();

    OUString aOrientation("Orientation");
    for (sal_Int32 i = 0; i < nCount && i < 5; ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex( i ), UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xNamed.is());
        OUString aName = xNamed->getName();
        maFieldNames.push_back(aName);
        CPPUNIT_ASSERT( aName != "Data" );

        uno::Reference< beans::XPropertySet > xPropSet( xNamed, UNO_QUERY_THROW);
        CPPUNIT_ASSERT( xPropSet.is() );

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
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getColumnFields(), UNO_QUERY_THROW);

    checkName( xIndex, 0 );
}

void XDataPilotDescriptor::testGetRowFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getRowFields(), UNO_QUERY_THROW);

    //checkName( xIndex, 1 );
}

void XDataPilotDescriptor::testGetPageFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(), UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getPageFields(), UNO_QUERY_THROW);

    checkName( xIndex, 4 );
}

void XDataPilotDescriptor::testGetDataFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getDataFields(), UNO_QUERY_THROW);

    checkName( xIndex, 2 );
}

void XDataPilotDescriptor::testGetHiddenFields()
{
    uno::Reference< sheet::XDataPilotDescriptor > xDescr(init(),UNO_QUERY_THROW);
    testGetDataPilotFields_Impl( xDescr );
    uno::Reference< container::XIndexAccess > xIndex(xDescr->getHiddenFields(), UNO_QUERY_THROW);

    checkName( xIndex, 3 );
}

void XDataPilotDescriptor::checkName( uno::Reference< container::XIndexAccess > xIndex, sal_Int32 nIndex )
{
    CPPUNIT_ASSERT(xIndex.is());
    CPPUNIT_ASSERT(maFieldNames.size() >= static_cast<size_t>(nIndex));

    for (sal_Int32 i = 0; i < xIndex->getCount(); ++i)
    {
        uno::Reference< container::XNamed > xNamed( xIndex->getByIndex(i), UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(maFieldNames[nIndex], xNamed->getName());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
