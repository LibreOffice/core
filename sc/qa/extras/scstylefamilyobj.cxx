/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xnamecontainer.hxx>
#include <test/container/xnamereplace.hxx>
#include <test/lang/xserviceinfo.hxx>
#include <cppu/unotype.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

using namespace css;

namespace sc_apitest
{
class ScStyleFamilyObj : public UnoApiTest,
                         public apitest::XElementAccess,
                         public apitest::XIndexAccess,
                         public apitest::XNameAccess,
                         public apitest::XNameContainer,
                         public apitest::XNameReplace,
                         public apitest::XServiceInfo
{
public:
    ScStyleFamilyObj();

    virtual uno::Reference<uno::XInterface> init() override;
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(ScStyleFamilyObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XNameAccess
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);

    // XNameContainer
    CPPUNIT_TEST(testInsertByName);
    CPPUNIT_TEST(testInsertByNameDuplicate);
    CPPUNIT_TEST(testInsertByNameEmptyName);
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST(testRemoveByNameNoneExistingElement);

    // XNameReplace
    CPPUNIT_TEST(testReplaceByName);

    // XServiceInfo
    CPPUNIT_TEST(testGetImplementationName);
    CPPUNIT_TEST(testGetSupportedServiceNames);
    CPPUNIT_TEST(testSupportsService);

    CPPUNIT_TEST_SUITE_END();
};

/* TODO: this c/should be derived/defined from the real style count, default
 * implemented plus sc/res/xml/styles.xml */
constexpr sal_Int32 kScStyleFamilyObjCount = 20;

ScStyleFamilyObj::ScStyleFamilyObj()
    : UnoApiTest("/sc/qa/extras/testdocuments")
    , XElementAccess(cppu::UnoType<style::XStyle>::get())
    , XIndexAccess(kScStyleFamilyObjCount)
    , XNameAccess("ScStyleFamilyObj")
    , XNameContainer("ScStyleFamilyObj")
    , XNameReplace("ScStyleFamilyObj")
    , XServiceInfo("ScStyleFamilyObj", "com.sun.star.style.StyleFamily")
{
}

uno::Reference<uno::XInterface> ScStyleFamilyObj::init()
{
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);

    uno::Reference<style::XStyleFamiliesSupplier> xSFS(xDoc, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA(xSFS->getStyleFamilies(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xNA, uno::UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xNA_SF(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<uno::XInterface> xCS(xMSF->createInstance("com.sun.star.style.CellStyle"),
                                        uno::UNO_SET_THROW);
    // XNameContainer
    XNameContainer::setElement(uno::Any(xMSF->createInstance("com.sun.star.style.CellStyle")));
    // XNameReplace
    XNameReplace::setElement(uno::Any(xMSF->createInstance("com.sun.star.style.CellStyle")));

    uno::Reference<container::XNameContainer> xNC(xNA_SF, uno::UNO_QUERY_THROW);
    xNC->insertByName("ScStyleFamilyObj", uno::Any(xCS));

    return xNA_SF;
}

void ScStyleFamilyObj::setUp()
{
    UnoApiTest::setUp();
    // create calc document
    mxComponent = loadFromDesktop("private:factory/scalc");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScStyleFamilyObj);

} // namespace sc_apitest

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
