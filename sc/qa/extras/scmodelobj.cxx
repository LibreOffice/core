/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/document/xlinktargetsupplier.hxx>
#include <test/sheet/spreadsheetdocumentsettings.hxx>
#include <test/sheet/xcalculatable.hxx>
#include <test/sheet/xconsolidatable.hxx>
#include <test/sheet/xdocumentauditing.hxx>
#include <test/sheet/xgoalseek.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScModelObj : public UnoApiTest,
                   public apitest::SpreadsheetDocumentSettings,
                   public apitest::XCalculatable,
                   public apitest::XConsolidatable,
                   public apitest::XDocumentAuditing,
                   public apitest::XGoalSeek,
                   public apitest::XLinkTargetSupplier
{
public:
    virtual void setUp() override;

    virtual uno::Reference<uno::XInterface> init() override;
    virtual uno::Sequence<uno::Reference<table::XCell>> getXCells() override;

    ScModelObj();

    CPPUNIT_TEST_SUITE(ScModelObj);

    // SpreadsheetDocumentSettings
    CPPUNIT_TEST(testSpreadsheetDocumentSettingsProperties);

    // XCalculatable
    CPPUNIT_TEST(testCalculate);
    CPPUNIT_TEST(testCalculateAll);
    CPPUNIT_TEST(testEnableAutomaticCalculation);

    // XConsolidatable
    CPPUNIT_TEST(testCreateConsolidationDescriptor);
    CPPUNIT_TEST(testConsolidate);

    // XDocumentAuditing
    CPPUNIT_TEST(testRefreshArrows);

    // XGoalSeek
    CPPUNIT_TEST(testSeekGoal);

    // XLinkTargetSupplier
    CPPUNIT_TEST(testGetLinks);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Sequence<uno::Reference<table::XCell>> m_xCells;
};

ScModelObj::ScModelObj()
    : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
{
}

uno::Reference<uno::XInterface> ScModelObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference<frame::XModel> xModel(xDoc, UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet(xIA->getByIndex(0), UNO_QUERY_THROW);

    m_xCells = { xSheet->getCellByPosition(4, 5), xSheet->getCellByPosition(5, 5),
                 xSheet->getCellByPosition(6, 5) };
    m_xCells[0]->setValue(15);
    m_xCells[1]->setValue(10);
    m_xCells[2]->setFormula(u"= E6 * F6"_ustr);

    return xModel;
}

uno::Sequence<uno::Reference<table::XCell>> ScModelObj::getXCells() { return m_xCells; }

void ScModelObj::setUp()
{
    UnoApiTest::setUp();
    // create a calc document
    loadFromFile(u"ScModelObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScModelObj);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
