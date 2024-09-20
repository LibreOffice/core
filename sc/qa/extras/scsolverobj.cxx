/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/sheet/SolverObjectiveType.hpp>
#include <com/sun/star/sheet/XSolverSettings.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/sheet/ModelConstraint.hpp>
#include <com/sun/star/sheet/SolverConstraintOperator.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace css;

namespace sc_apitest
{
class ScSolverSettingsObj : public UnoApiTest
{
public:
    ScSolverSettingsObj();

    virtual void setUp() override;
    void testXSolverSettings();
    void testCellAddress(const table::CellAddress& rExpected, const uno::Any& rActual);
    void testCellRangeAddress(const uno::Any& rExpected, const uno::Any& rActual);

    CPPUNIT_TEST_SUITE(ScSolverSettingsObj);
    CPPUNIT_TEST(testXSolverSettings);
    CPPUNIT_TEST_SUITE_END();
};

ScSolverSettingsObj::ScSolverSettingsObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

void ScSolverSettingsObj::testCellAddress(const table::CellAddress& rExpected,
                                          const uno::Any& rActual)
{
    table::CellAddress aActualAddress;
    rActual >>= aActualAddress;
    CPPUNIT_ASSERT_EQUAL(rExpected.Column, aActualAddress.Column);
    CPPUNIT_ASSERT_EQUAL(rExpected.Row, aActualAddress.Row);
    CPPUNIT_ASSERT_EQUAL(rExpected.Sheet, aActualAddress.Sheet);
}

void ScSolverSettingsObj::testCellRangeAddress(const uno::Any& rExpected, const uno::Any& rActual)
{
    table::CellRangeAddress aActualAddress;
    table::CellRangeAddress aExpectedAddress;
    rActual >>= aActualAddress;
    rExpected >>= aExpectedAddress;
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress.Sheet, aActualAddress.Sheet);
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress.StartRow, aActualAddress.StartRow);
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress.StartColumn, aActualAddress.StartColumn);
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress.EndRow, aActualAddress.EndRow);
    CPPUNIT_ASSERT_EQUAL(aExpectedAddress.EndColumn, aActualAddress.EndColumn);
}

// Creates a model using the XSolverSettings API checks if it is accessible via the API
void ScSolverSettingsObj::testXSolverSettings()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xSheet, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSolverSettings> xSolverModel(
        xPropSet->getPropertyValue("SolverSettings"), uno::UNO_QUERY_THROW);

    // Objective cell is in G7
    table::CellAddress aObjCell(0, 6, 6);
    xSolverModel->setObjectiveCell(uno::Any(aObjCell));
    xSolverModel->setObjectiveType(sheet::SolverObjectiveType::MAXIMIZE);

    // Variable cells (E3:E17)
    uno::Sequence<uno::Any> aVarCells(1);
    auto pVarCells = aVarCells.getArray();
    pVarCells[0] <<= table::CellRangeAddress(0, 4, 2, 4, 16);
    xSolverModel->setVariableCells(aVarCells);

    // Constraints
    uno::Sequence<sheet::ModelConstraint> aConstraints(2);
    auto pConstraints = aConstraints.getArray();
    pConstraints[0].Left <<= OUString("$E$3:$E$17");
    pConstraints[0].Operator = sheet::SolverConstraintOperator_BINARY;
    pConstraints[1].Left <<= OUString("$G$5");
    pConstraints[1].Operator = sheet::SolverConstraintOperator_LESS_EQUAL;
    pConstraints[1].Right <<= OUString("$G$3");
    xSolverModel->setConstraints(aConstraints);

    // Set solver engine options
    xSolverModel->setEngine(u"com.sun.star.comp.Calc.LpsolveSolver"_ustr);
    uno::Sequence<beans::PropertyValue> aEngineOptions{
        comphelper::makePropertyValue(u"Timeout"_ustr, uno::Any(static_cast<sal_Int32>(10))),
        comphelper::makePropertyValue(u"NonNegative"_ustr, true),
    };
    xSolverModel->setEngineOptions(aEngineOptions);

    // Run the model and check the results
    xSolverModel->saveToFile();
    xSolverModel->setSuppressDialog(true);
    xSolverModel->solve();

    // The correct solution value is 6981 (using LpsolveSolver)
    CPPUNIT_ASSERT_EQUAL(static_cast<double>(6981), xSheet->getCellByPosition(6, 6)->getValue());

    // Check objective function and variable cells
    testCellAddress(aObjCell, xSolverModel->getObjectiveCell());
    CPPUNIT_ASSERT_EQUAL(sheet::SolverObjectiveType::MAXIMIZE, xSolverModel->getObjectiveType());
    uno::Sequence<uno::Any> aSeq = xSolverModel->getVariableCells();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aSeq.getLength());
    testCellRangeAddress(aVarCells[0], aSeq[0]);

    // Check if constraints were set
    uno::Sequence<sheet::ModelConstraint> aSeqConstr = xSolverModel->getConstraints();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeqConstr.getLength());
    CPPUNIT_ASSERT_EQUAL(sheet::SolverConstraintOperator::SolverConstraintOperator_BINARY,
                         aSeqConstr[0].Operator);
    table::CellRangeAddress aLeft1(0, 4, 2, 4, 16);
    table::CellRangeAddress aRight1(0, 0, 0, 0, 0);
    testCellRangeAddress(uno::Any(aLeft1), aSeqConstr[0].Left);
    testCellRangeAddress(uno::Any(aRight1), aSeqConstr[0].Right);
    CPPUNIT_ASSERT_EQUAL(sheet::SolverConstraintOperator::SolverConstraintOperator_LESS_EQUAL,
                         aSeqConstr[1].Operator);
    table::CellRangeAddress aLeft2(0, 6, 4, 6, 4);
    table::CellRangeAddress aRight2(0, 6, 2, 6, 2);
    testCellRangeAddress(uno::Any(aLeft2), aSeqConstr[1].Left);
    testCellRangeAddress(uno::Any(aRight2), aSeqConstr[1].Right);

    // Check solver engine options
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.comp.Calc.LpsolveSolver"_ustr, xSolverModel->getEngine());

    // Check solver engine options
    uno::Sequence<beans::PropertyValue> aEngProps = xSolverModel->getEngineOptions();
    CPPUNIT_ASSERT_EQUAL(OUString("EpsilonLevel"), aEngProps[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(static_cast<sal_Int32>(0)), aEngProps[0].Value);
    CPPUNIT_ASSERT_EQUAL(u"GenSensitivityReport"_ustr, aEngProps[1].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(false), aEngProps[1].Value);
    CPPUNIT_ASSERT_EQUAL(u"Integer"_ustr, aEngProps[2].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(false), aEngProps[2].Value);
    CPPUNIT_ASSERT_EQUAL(u"LimitBBDepth"_ustr, aEngProps[3].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), aEngProps[3].Value);
    CPPUNIT_ASSERT_EQUAL(u"NonNegative"_ustr, aEngProps[4].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), aEngProps[4].Value);
    CPPUNIT_ASSERT_EQUAL(u"Timeout"_ustr, aEngProps[5].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(static_cast<sal_Int32>(10)), aEngProps[5].Value);

    // Save file and reload to check if solver settings are still there
    saveAndReload(u"calc8"_ustr);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc2(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex2(xDoc2->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet2(xIndex2->getByIndex(0), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet2(xSheet2, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSolverSettings> xSolverModel2(
        xPropSet2->getPropertyValue("SolverSettings"), uno::UNO_QUERY_THROW);

    // // Check objective function
    testCellAddress(aObjCell, xSolverModel2->getObjectiveCell());
    CPPUNIT_ASSERT_EQUAL(sheet::SolverObjectiveType::MAXIMIZE, xSolverModel2->getObjectiveType());

    // Check variable cells
    uno::Sequence<uno::Any> aVarCells2 = xSolverModel2->getVariableCells();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aVarCells2.getLength());
    testCellRangeAddress(aVarCells[0], aVarCells2[0]);

    // Check constraints
    uno::Sequence<sheet::ModelConstraint> aSeqConstr2 = xSolverModel2->getConstraints();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeqConstr2.getLength());
    CPPUNIT_ASSERT_EQUAL(sheet::SolverConstraintOperator::SolverConstraintOperator_BINARY,
                         aSeqConstr2[0].Operator);
    testCellRangeAddress(uno::Any(aLeft1), aSeqConstr2[0].Left);
    testCellRangeAddress(uno::Any(aRight1), aSeqConstr2[0].Right);
    CPPUNIT_ASSERT_EQUAL(sheet::SolverConstraintOperator::SolverConstraintOperator_LESS_EQUAL,
                         aSeqConstr2[1].Operator);
    testCellRangeAddress(uno::Any(aLeft2), aSeqConstr2[1].Left);
    testCellRangeAddress(uno::Any(aRight2), aSeqConstr2[1].Right);

    // Check solver engine options
    uno::Sequence<beans::PropertyValue> aEngProps2 = xSolverModel2->getEngineOptions();
    CPPUNIT_ASSERT_EQUAL(OUString("EpsilonLevel"), aEngProps2[0].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(static_cast<sal_Int32>(0)), aEngProps2[0].Value);
    CPPUNIT_ASSERT_EQUAL(u"GenSensitivityReport"_ustr, aEngProps2[1].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(false), aEngProps2[1].Value);
    CPPUNIT_ASSERT_EQUAL(u"Integer"_ustr, aEngProps2[2].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(false), aEngProps2[2].Value);
    CPPUNIT_ASSERT_EQUAL(u"LimitBBDepth"_ustr, aEngProps2[3].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), aEngProps2[3].Value);
    CPPUNIT_ASSERT_EQUAL(u"NonNegative"_ustr, aEngProps2[4].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(true), aEngProps2[4].Value);
    CPPUNIT_ASSERT_EQUAL(u"Timeout"_ustr, aEngProps2[5].Name);
    CPPUNIT_ASSERT_EQUAL(uno::Any(static_cast<sal_Int32>(10)), aEngProps2[5].Value);
}

void ScSolverSettingsObj::setUp()
{
    UnoApiTest::setUp();
    loadFromFile(u"knapsack.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSolverSettingsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
