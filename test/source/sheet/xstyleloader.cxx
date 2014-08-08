/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xstyleloader.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/style/XStyleLoader2.hpp>

#include <com/sun/star/style/XStyle.hpp>

#include <rtl/ustring.hxx>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {


void XStyleLoader::testLoadStylesFromURL()
{

  uno::Reference< sheet::XSpreadsheetDocument > xTargetDoc(init(), UNO_QUERY_THROW);

  OUString aFileURL = getTestURL();

  uno::Reference< style::XStyleFamiliesSupplier > xFamilySupplier (xTargetDoc, UNO_QUERY_THROW);
  uno::Reference< style::XStyleLoader > xTargetStyleLoader (xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);

  uno::Sequence< beans::PropertyValue > aOptions = xTargetStyleLoader->getStyleLoaderOptions();

  xTargetStyleLoader->loadStylesFromURL(aFileURL, aOptions);

  checkStyleProperties(xFamilySupplier);

}

void XStyleLoader::testLoadStylesFromDocument()
{

  uno::Reference< sheet::XSpreadsheetDocument > xTargetDoc(init(), UNO_QUERY_THROW);

  uno::Reference< lang::XComponent > xSourceDoc (getSourceComponent(), UNO_QUERY_THROW);

  uno::Reference< style::XStyleFamiliesSupplier > xFamilySupplier (xTargetDoc, UNO_QUERY_THROW);
  uno::Reference< style::XStyleLoader2 > xTargetStyleLoader (xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);

  uno::Sequence< beans::PropertyValue > aOptions = xTargetStyleLoader->getStyleLoaderOptions();

  xTargetStyleLoader->loadStylesFromDocument(xSourceDoc, aOptions);

  checkStyleProperties(xFamilySupplier);

}

void XStyleLoader::checkStyleProperties( uno::Reference< style::XStyleFamiliesSupplier > xFamilySupplier)
{
    // check if targetDocument has myStyle
  uno::Reference< container::XNameAccess > xFamilies(xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);
  uno::Reference< container::XNameContainer > xCellStyles(xFamilies->getByName("CellStyles"), UNO_QUERY_THROW);

  CPPUNIT_ASSERT_MESSAGE("Style not imported", xCellStyles->hasByName("myStyle"));

  // test the backgroundcolor is correctly imported
  uno::Reference< style::XStyle > xMyStyle (xCellStyles->getByName("myStyle"), UNO_QUERY_THROW);
  uno::Reference< beans::XPropertySet > xPropSet (xMyStyle, UNO_QUERY_THROW);

  OUString aCellStyleName("CellBackColor");
  uno::Any aBackColor = xPropSet->getPropertyValue(aCellStyleName);
  uno::Any expectedBackColor(sal_Int32(16724787));

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong CellBackColor" , expectedBackColor, aBackColor);

  // test default pageStyle

  uno::Reference< container::XNameContainer > xPageStyles(xFamilies->getByName("PageStyles"), UNO_QUERY_THROW);
  uno::Reference<beans::XPropertySet> xPagePropSet(xPageStyles->getByName("Default"), UNO_QUERY_THROW);

  uno::Any aPageBackColor = xPagePropSet->getPropertyValue("BackColor");
  uno::Any expectedPageBackColor(sal_Int32(13434879));

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong page style BackColor" , expectedPageBackColor, aPageBackColor);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
