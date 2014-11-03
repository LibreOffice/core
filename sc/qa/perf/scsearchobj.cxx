/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <test/callgrind.hxx>
#include <test/testdllapi.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

class OOO_DLLPUBLIC_TEST XCalcSearch
{
public:
    virtual css::uno::Reference< css::uno::XInterface > init() = 0;

    // XSearchable
    void testSheetFindAll();

protected:
    ~XCalcSearch() {}

};

}

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScSearchObj : public CalcUnoApiTest, apitest::XCalcSearch
{
public:
    ScSearchObj();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    virtual uno::Reference< uno::XInterface > init() SAL_OVERRIDE;

    CPPUNIT_TEST_SUITE(ScSearchObj);
    CPPUNIT_TEST(testSheetFindAll);
    CPPUNIT_TEST_SUITE_END();

private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;

};

sal_Int32 ScSearchObj::nTest = 0;
uno::Reference< lang::XComponent > ScSearchObj::mxComponent;

ScSearchObj::ScSearchObj()
       : CalcUnoApiTest("sc/qa/perf/testdocuments/")
{
}

uno::Reference< uno::XInterface > ScSearchObj::init()
{
    if (mxComponent.is())
        closeDocument(mxComponent);

    OUString aFileURL;
    createFileURL(OUString("scBigFile.ods"), aFileURL);

    mxComponent = loadFromDesktop(aFileURL);

    CPPUNIT_ASSERT(mxComponent.is());

    return mxComponent;
}

void ScSearchObj::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScSearchObj::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
      if (mxComponent.is())
      {
          closeDocument(mxComponent);
          mxComponent.clear();
      }
    }
    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScSearchObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

namespace apitest {

void XCalcSearch::testSheetFindAll()
{

  uno::Reference< sheet::XSpreadsheetDocument > xDoc(init(), UNO_QUERY_THROW);

  CPPUNIT_ASSERT_MESSAGE("Problem in document loading" , xDoc.is());

  // get sheet
  uno::Reference< container::XIndexAccess > xSheetIndex (xDoc->getSheets(), UNO_QUERY_THROW);
  CPPUNIT_ASSERT_MESSAGE("Problem in xSheetIndex" , xSheetIndex.is());

  uno::Reference< sheet::XSpreadsheet > xSheet( xSheetIndex->getByIndex(0), UNO_QUERY_THROW);

  // create descriptor
  uno::Reference< util::XSearchable > xSearchable(xSheet, UNO_QUERY_THROW);
  uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();

  // search for a value
  xSearchDescr->setSearchString(OUString("value_1"));

  callgrindStart();

  uno::Reference< container::XIndexAccess > xIndex = xSearchable->findAll(xSearchDescr);

  callgrindDump("testSheetFindAll-Search value");

  CPPUNIT_ASSERT(xIndex.is());

  int nCount = xIndex->getCount();

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Count" , 20000, nCount);

// search for style

  uno::Reference< sheet::XSpreadsheet > xSheet2( xSheetIndex->getByIndex(2), UNO_QUERY_THROW);
  uno::Reference< util::XSearchable > xSearchableStyle(xSheet2, UNO_QUERY_THROW);
  xSearchDescr = xSearchableStyle->createSearchDescriptor();

  uno::Reference< beans::XPropertySet > xSearchProp(xSearchDescr,UNO_QUERY_THROW);
  xSearchProp->setPropertyValue(OUString("SearchStyles"), makeAny(true));

  xSearchDescr->setSearchString(OUString("aCellStyle"));

  callgrindStart();

  uno::Reference< container::XIndexAccess > xIndex2 = xSearchableStyle->findAll(xSearchDescr);

  callgrindDump("testSheetFindAll-Search style");

  CPPUNIT_ASSERT(xIndex2.is());

  nCount = xIndex2->getCount();

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Count" , 160, nCount);

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
