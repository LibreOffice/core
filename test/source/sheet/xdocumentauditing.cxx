/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdocumentauditing.hxx>

#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XDocumentAuditing.hpp>
#include <com/sun/star/sheet/XSheetAuditing.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XDocumentAuditing::dispatch(const uno::Reference<frame::XFrame>& xFrame,
                                 const uno::Sequence<beans::PropertyValue>& rArguments)
{
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext),
                                                           UNO_SET_THROW);
    CPPUNIT_ASSERT(xDispatchHelper.is());

    uno::Reference<frame::XDispatchProvider> xDispatchProvider(xFrame, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDispatchProvider.is());

    xDispatchHelper->executeDispatch(xDispatchProvider, ".uno:AutoRefreshArrows", "", 0,
                                     rArguments);
}

bool XDocumentAuditing::hasRightAmountOfShapes(const uno::Reference<drawing::XDrawPage>& xDrawPage,
                                               sal_Int32 nElementCount, sal_Int32 nShapes)
{
    const sal_Int32 nCount = xDrawPage->getCount();
    if (nCount != nElementCount + nShapes)
        return false;
    else
    {
        if (nShapes >= 0)
        {
            for (sal_Int32 i = nElementCount; i < nCount; i++)
            {
                uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(i), UNO_QUERY_THROW);
                m_Position = xShape->getPosition();
            }
        }
    }
    return true;
}

void XDocumentAuditing::testRefreshArrows()
{
    uno::Reference<sheet::XDocumentAuditing> xDocumentAuditing(init(), UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xDocumentAuditing, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet1(xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet2(xIA->getByIndex(1), UNO_QUERY_THROW);

    uno::Reference<drawing::XDrawPagesSupplier> xDPS(xDocumentAuditing, UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDPS->getDrawPages()->getByIndex(1),
                                                 UNO_QUERY_THROW);

    sal_Int32 nDrawPageElementCount = 0;
    if (xDrawPage->hasElements())
        nDrawPageElementCount = xDrawPage->getCount();

    uno::Sequence<beans::PropertyValue> aPropertyValue(1);
    uno::Any aValue;
    aValue <<= false;
    aPropertyValue[0].Name = "AutoRefreshArrows";
    aPropertyValue[0].Value = aValue;
    uno::Reference<frame::XModel> xModel(xDocumentAuditing, UNO_QUERY_THROW);
    dispatch(xModel->getCurrentController()->getFrame(), aPropertyValue);

    xSheet1->getCellByPosition(6, 6)->setValue(9);
    uno::Reference<container::XNamed> xNA1(xSheet1, UNO_QUERY_THROW);
    OUString sSheet1Name = xNA1->getName();

    xSheet2->getCellByPosition(6, 6)->setValue(16);
    xSheet2->getCellByPosition(6, 7)->setFormula("= SQRT(G7)");

    uno::Reference<sheet::XSheetAuditing> xSheetAuditing(xSheet2, UNO_QUERY_THROW);
    xSheetAuditing->showPrecedents(table::CellAddress(1, 6, 7));
    bool bResult = hasRightAmountOfShapes(xDrawPage, nDrawPageElementCount, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong amount of shapes on page", bResult);
    awt::Point Position0 = m_Position;

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4, xSheet2->getCellByPosition(6, 7)->getValue(), 0.1);
    xSheet2->getCellByPosition(6, 7)->setFormula("= SQRT(" + sSheet1Name + ".G7)");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3, xSheet2->getCellByPosition(6, 7)->getValue(), 0.1);

    bResult = hasRightAmountOfShapes(xDrawPage, nDrawPageElementCount, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong amount of shapes on page", bResult);
    awt::Point Position1 = m_Position;

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Arrow has been refreshed", Position0.X, Position1.X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Arrow has been refreshed", Position0.Y, Position1.Y);

    xDocumentAuditing->refreshArrows();

    bResult = hasRightAmountOfShapes(xDrawPage, nDrawPageElementCount, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong amount of shapes on page", bResult);
    awt::Point Position2 = m_Position;

    CPPUNIT_ASSERT_MESSAGE("Arrow has not been refreshed",
                           Position1.X != Position2.X || Position1.Y != Position2.Y);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
