/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/perf/xcalcsearch.hxx>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <valgrind/callgrind.h>

using namespace css;
using namespace css::uno;

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
  xSearchDescr->setSearchString(OUString("value_1"));

  CALLGRIND_START_INSTRUMENTATION;
  CALLGRIND_ZERO_STATS;

  uno::Reference< container::XIndexAccess > xIndex = xSearchable->findAll(xSearchDescr);

  CALLGRIND_STOP_INSTRUMENTATION;
  CALLGRIND_DUMP_STATS;

  CPPUNIT_ASSERT(xIndex.is());

  int nCount = xIndex->getCount();

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong Count" , 20000, nCount);

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
