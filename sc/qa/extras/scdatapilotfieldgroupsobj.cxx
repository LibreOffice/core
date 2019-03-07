/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xnamecontainer.hxx>
#include <test/lang/xserviceinfo.hxx>

#include <comphelper/types.hxx>
#include <rtl/string.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/DataPilotFieldGroupInfo.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/XDataPilotFieldGrouping.hpp>
#include <com/sun/star/sheet/XDataPilotTables.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace css;

namespace sc_apitest
{
class ScDataPilotFieldGroupsObj : public CalcUnoApiTest,
                                  public apitest::XElementAccess,
                                  public apitest::XEnumerationAccess,
                                  public apitest::XIndexAccess,
                                  public apitest::XNameAccess,
                                  public apitest::XNameContainer,
                                  public apitest::XServiceInfo
{
public:
    ScDataPilotFieldGroupsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScDataPilotFieldGroupsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XNameContainer
    CPPUNIT_TEST(testInsertByName);
    CPPUNIT_TEST(testInsertByNameEmptyName);
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST(testRemoveByNameEmptyName);
    CPPUNIT_TEST(testRemoveByNameNoneExistingElement);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();

private:
    static const int m_nMaxFieldIndex = 6;
    uno::Reference<lang::XComponent> m_xComponent;
};

ScDataPilotFieldGroupsObj::ScDataPilotFieldGroupsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<container::XNameAccess>::get())
    , XIndexAccess(1)
    , XNameAccess("Group1")
    , XNameContainer("Group1")
    , XServiceInfo("ScDataPilotFieldGroupsObj", "com.sun.star.sheet.DataPilotFieldGroups")
{
}

uno::Reference<uno::XInterface> ScDataPilotFieldGroupsObj::init()
{
    table::CellRangeAddress aCellRangeAddress(0, 1, 0, m_nMaxFieldIndex - 1, m_nMaxFieldIndex - 1);
    table::CellAddress aCellAddress(0, 7, 8);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("Some Sheet", 0);

    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet1(xIA->getByIndex(1), uno::UNO_QUERY_THROW);

    for (auto i = 1; i < m_nMaxFieldIndex; ++i)
    {
        xSheet0->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet0->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
        xSheet1->getCellByPosition(i, 0)->setFormula("Col" + OUString::number(i));
        xSheet1->getCellByPosition(0, i)->setFormula("Row" + OUString::number(i));
    }

    for (auto i = 1; i < m_nMaxFieldIndex; ++i)
    {
        for (auto j = 1; j < m_nMaxFieldIndex; ++j)
        {
            xSheet0->getCellByPosition(i, j)->setValue(i * (j + 1));
            xSheet1->getCellByPosition(i, j)->setValue(i * (j + 2));
        }
    }

    xSheet0->getCellByPosition(1, 1)->setFormula("aName");
    xSheet0->getCellByPosition(1, 2)->setFormula("otherName");
    xSheet0->getCellByPosition(1, 3)->setFormula("una");
    xSheet0->getCellByPosition(1, 4)->setFormula("otherName");
    xSheet0->getCellByPosition(1, 5)->setFormula("somethingelse");

    xSheet0->getCellByPosition(1, 5);
    xSheet0->getCellByPosition(aCellAddress.Column, aCellAddress.Row + 3);

    uno::Reference<sheet::XDataPilotTablesSupplier> xDPTS(xSheet0, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotTables> xDPT(xDPTS->getDataPilotTables(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD(xDPT->createDataPilotDescriptor(),
                                                     uno::UNO_QUERY_THROW);

    xDPD->setSourceRange(aCellRangeAddress);

    uno::Reference<beans::XPropertySet> xPropertySet0(xDPD->getDataPilotFields()->getByIndex(0),
                                                      uno::UNO_QUERY_THROW);
    xPropertySet0->setPropertyValue("Orientation",
                                    uno::makeAny(sheet::DataPilotFieldOrientation_ROW));

    uno::Reference<beans::XPropertySet> xPropertySet1(xDPD->getDataPilotFields()->getByIndex(1),
                                                      uno::UNO_QUERY_THROW);
    xPropertySet1->setPropertyValue("Function", uno::makeAny(sheet::GeneralFunction_SUM));
    xPropertySet1->setPropertyValue("Orientation",
                                    uno::makeAny(sheet::DataPilotFieldOrientation_DATA));

    uno::Reference<beans::XPropertySet> xPropertySet2(xDPD->getDataPilotFields()->getByIndex(2),
                                                      uno::UNO_QUERY_THROW);
    xPropertySet2->setPropertyValue("Orientation",
                                    uno::makeAny(sheet::DataPilotFieldOrientation_COLUMN));

    xDPT->insertNewByName("DataPilotTable", aCellAddress, xDPD);

    uno::Reference<container::XIndexAccess> xIA_DPT0(xDPTS->getDataPilotTables(),
                                                     uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD0(xIA_DPT0->getByIndex(0),
                                                      uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_RF0(xDPD0->getRowFields(), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XDataPilotFieldGrouping> xDPFG(xIA_RF0->getByIndex(0),
                                                         uno::UNO_QUERY_THROW);
    uno::Sequence<OUString> aElements(2);
    aElements[0] = "aName";
    aElements[1] = "otherName";
    xDPFG->createNameGroup(aElements);

    uno::Reference<container::XNameAccess> xNA_GroupNames;

    uno::Reference<container::XIndexAccess> xIA_DPT1(xDPTS->getDataPilotTables(),
                                                     uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XDataPilotDescriptor> xDPD1(xIA_DPT1->getByIndex(0),
                                                      uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIA_RF1(xDPD1->getRowFields(), uno::UNO_QUERY_THROW);

    sheet::DataPilotFieldGroupInfo aDPFGI;
    for (auto i = 0; i < xIA_RF1->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xIA_RF1->getByIndex(i),
                                                         uno::UNO_QUERY_THROW);
        if (::comphelper::getBOOL(xPropertySet->getPropertyValue("IsGroupField")))
        {
            CPPUNIT_ASSERT(xPropertySet->getPropertyValue("GroupInfo") >>= aDPFGI);
        }
    }

    // set element for testing XNameContainer::insertByName()
    uno::Any aElement;
    setElement(aElement);

    return aDPFGI.Groups;
}

void ScDataPilotFieldGroupsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScDataPilotFieldGroupsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScDataPilotFieldGroupsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
