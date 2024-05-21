/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/sheet/xsheetannotations.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#include <com/sun/star/sheet/XSheetAnnotations.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppu/unotype.hxx>

using namespace css;
using namespace css::uno;

namespace sc_apitest {

class ScAnnontationsObj : public UnoApiTest, public apitest::XElementAccess,
                                                 public apitest::XEnumerationAccess,
                                                 public apitest::XIndexAccess,
                                                 public apitest::XSheetAnnotations
{
public:
    ScAnnontationsObj();

    virtual void setUp() override;

    virtual uno::Reference< uno::XInterface > init() override;
    virtual uno::Reference< sheet::XSheetAnnotations > getAnnotations(tools::Long nIndex) override;

    CPPUNIT_TEST_SUITE(ScAnnontationsObj);

    // XElementAccess
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);

    // XEnumerationAccess
    CPPUNIT_TEST(testCreateEnumeration);

    // XIndexAccess
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetCount);

    // XSheetAnnotations
    CPPUNIT_TEST(testCount);
    CPPUNIT_TEST(testIndex);
    CPPUNIT_TEST(testInsertNew);
    CPPUNIT_TEST(testRemoveByIndex);

    CPPUNIT_TEST_SUITE_END();
};


ScAnnontationsObj::ScAnnontationsObj()
       : UnoApiTest(u"/sc/qa/extras/testdocuments"_ustr)
       , XElementAccess(cppu::UnoType<sheet::XSheetAnnotation>::get())
       , XIndexAccess(1)
{
}

uno::Reference< sheet::XSheetAnnotations> ScAnnontationsObj::getAnnotations(tools::Long nIndex)
{
    // get the sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);

    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(nIndex), UNO_QUERY_THROW);

    // get the annotations collection
    uno::Reference< sheet::XSheetAnnotationsSupplier > xAnnotationSupplier(xSheet, UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetAnnotations > xSheetAnnotations( xAnnotationSupplier->getAnnotations(), UNO_SET_THROW);

    return xSheetAnnotations;
}

uno::Reference< uno::XInterface > ScAnnontationsObj::init()
{
    return getAnnotations(0);
}

void ScAnnontationsObj::setUp()
{
    UnoApiTest::setUp();

    // get the test file
    loadFromFile(u"ScAnnotationObj.ods");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnnontationsObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
