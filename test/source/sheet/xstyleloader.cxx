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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/style/XStyleLoader2.hpp>

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

  // check if targetDocument has myStyle
  uno::Reference< container::XNameAccess > xFamilies(xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);
  uno::Reference< container::XNameContainer > xCellStyles(xFamilies->getByName("CellStyles"), UNO_QUERY_THROW);

  CPPUNIT_ASSERT_MESSAGE("Style not imported", xCellStyles->hasByName("myStyle"));

}

void XStyleLoader::testLoadStylesFromDocument()
{

  uno::Reference< sheet::XSpreadsheetDocument > xTargetDoc(init(), UNO_QUERY_THROW);

  uno::Reference< lang::XComponent > xSourceDoc (getSourceComponent(), UNO_QUERY_THROW);

  uno::Reference< style::XStyleFamiliesSupplier > xFamilySupplier (xTargetDoc, UNO_QUERY_THROW);
  uno::Reference< style::XStyleLoader2 > xTargetStyleLoader (xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);

  uno::Sequence< beans::PropertyValue > aOptions = xTargetStyleLoader->getStyleLoaderOptions();

  xTargetStyleLoader->loadStylesFromDocument(xSourceDoc, aOptions);

  // check if targetDocument has myStyle
  uno::Reference< container::XNameAccess > xFamilies(xFamilySupplier->getStyleFamilies(), UNO_QUERY_THROW);
  uno::Reference< container::XNameContainer > xCellStyles(xFamilies->getByName("CellStyles"), UNO_QUERY_THROW);

  CPPUNIT_ASSERT_MESSAGE("Style not imported", xCellStyles->hasByName("myStyle"));

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */