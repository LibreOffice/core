/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xnamecontainer.hxx>
#include <test/container/xnamereplace.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScAutoFormatsObj : public CalcUnoApiTest,
                         public apitest::XNameAccess,
                         public apitest::XNameContainer,
                         public apitest::XNameReplace
{
public:
    ScAutoFormatsObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;
    virtual void tearDown() override;

    CPPUNIT_TEST_SUITE(ScAutoFormatsObj);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XNameContainer
    CPPUNIT_TEST(testInsertByName);
    CPPUNIT_TEST(testInsertByNameEmptyName);
    CPPUNIT_TEST(testInsertByNameDuplicate);
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST(testRemoveByNameNoneExistingElement);

    // XNameReplace
    CPPUNIT_TEST(testReplaceByName);

    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<lang::XComponent> m_xComponent;
};

ScAutoFormatsObj::ScAutoFormatsObj()
    : CalcUnoApiTest("/sc/qa/extras/testdocuments")
    , XNameAccess("Default")
    , XNameContainer("ScAutoFormatsObj")
    , XNameReplace("ScAutoFormatsObj")
{
}

uno::Reference<uno::XInterface> ScAutoFormatsObj::init()
{
    uno::Reference<lang::XMultiServiceFactory> xMSF(m_xComponent, uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xTAF(
        xMSF->createInstance("com.sun.star.sheet.TableAutoFormats"), uno::UNO_QUERY_THROW);

    uno::Reference<container::XNameContainer> xNC(xTAF, uno::UNO_QUERY_THROW);
    if (!xNC->hasByName("ScAutoFormatsObj"))
    {
        xNC->insertByName("ScAutoFormatsObj",
                          uno::makeAny(xMSF->createInstance("com.sun.star.sheet.TableAutoFormat")));
    }
    // XNameContainer
    setElement(uno::makeAny(xMSF->createInstance("com.sun.star.sheet.TableAutoFormat")));
    // XNameReplace
    setReplacementElement(uno::makeAny(xMSF->createInstance("com.sun.star.sheet.TableAutoFormat")));

    return xTAF;
}

void ScAutoFormatsObj::setUp()
{
    CalcUnoApiTest::setUp();
    // create calc document
    m_xComponent = loadFromDesktop("private:factory/scalc");
}

void ScAutoFormatsObj::tearDown()
{
    closeDocument(m_xComponent);
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAutoFormatsObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
