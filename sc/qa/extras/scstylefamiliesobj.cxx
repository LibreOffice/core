/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest
{
class ScStyleFamiliesObj : public CalcUnoApiTest,
                           public apitest::XIndexAccess,
                           public apitest::XNameAccess
{
public:
    ScStyleFamiliesObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScStyleFamiliesObj);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScStyleFamiliesObj::ScStyleFamiliesObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XIndexAccess(2)
    , XNameAccess("CellStyles")
{
}

uno::Reference<uno::XInterface> ScStyleFamiliesObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(m_xComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<style::XStyleFamiliesSupplier> xSFS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA(xSFS->getStyleFamilies(), uno::UNO_QUERY_THROW);

    return xNA;
}

void ScStyleFamiliesObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create a calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
    CPPUNIT_ASSERT_MESSAGE("no component", m_xComponent.is());
}

void ScStyleFamiliesObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScStyleFamiliesObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
