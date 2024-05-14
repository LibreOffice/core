/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/style/xstyleloader.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleLoader2.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/ustring.hxx>
#include <cppunit/TestAssert.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

using namespace comphelper;
using namespace css;
using namespace css::uno;

namespace apitest
{
void XStyleLoader::testLoadStylesFromURL()
{
    uno::Reference<style::XStyleLoader2> xStyleLoader(init(), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getTargetDoc(), uno::UNO_SET_THROW);
    const OUString aFileURL = getTestURL();

    uno::Sequence<beans::PropertyValue> aOptions = xStyleLoader->getStyleLoaderOptions();
    xStyleLoader->loadStylesFromURL(aFileURL, aOptions);

    uno::Reference<style::XStyleFamiliesSupplier> xFamilySupplier(xDoc, UNO_QUERY_THROW);
    checkStyleProperties(xFamilySupplier);
}

void XStyleLoader::testLoadStylesFromDocument()
{
    uno::Reference<style::XStyleLoader2> xStyleLoader(init(), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getTargetDoc(), uno::UNO_SET_THROW);
    uno::Reference<lang::XComponent> xSrcComponent(getSourceComponent(), UNO_SET_THROW);

    uno::Sequence<beans::PropertyValue> aOptions = xStyleLoader->getStyleLoaderOptions();
    xStyleLoader->loadStylesFromDocument(xSrcComponent, aOptions);

    uno::Reference<style::XStyleFamiliesSupplier> xFamilySupplier(xDoc, UNO_QUERY_THROW);
    checkStyleProperties(xFamilySupplier);
}

void XStyleLoader::testLoadStylesFromStream()
{
    uno::Reference<style::XStyleLoader2> xStyleLoader(init(), uno::UNO_QUERY_THROW);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(getTargetDoc(), uno::UNO_SET_THROW);
    const OUString aFileURL = getTestURL();
    const uno::Reference<io::XInputStream> xInputStream
        = OStorageHelper::GetInputStreamFromURL(aFileURL, getProcessComponentContext());

    uno::Sequence<beans::PropertyValue> aOptions = xStyleLoader->getStyleLoaderOptions();
    auto nLength = aOptions.getLength();
    aOptions.realloc(nLength + 1);
    beans::PropertyValue aInputStream;
    aInputStream.Name = "InputStream";
    uno::Any aTmp;
    aTmp <<= xInputStream;
    aInputStream.Value = aTmp;
    aOptions.getArray()[nLength] = aInputStream;

    xStyleLoader->loadStylesFromURL(u"private:stream"_ustr, aOptions);

    uno::Reference<style::XStyleFamiliesSupplier> xFamilySupplier(xDoc, UNO_QUERY_THROW);
    checkStyleProperties(xFamilySupplier);
}

void XStyleLoader::checkStyleProperties(
    uno::Reference<style::XStyleFamiliesSupplier> const& xFamilySupplier)
{
    // check if targetDocument has myStyle
    uno::Reference<container::XNameAccess> xFamilies(xFamilySupplier->getStyleFamilies(),
                                                     UNO_SET_THROW);
    uno::Reference<container::XNameContainer> xCellStyles(xFamilies->getByName(u"CellStyles"_ustr),
                                                          UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("Style not imported", xCellStyles->hasByName(u"myStyle"_ustr));

    // test the backgroundcolor is correctly imported
    uno::Reference<style::XStyle> xMyStyle(xCellStyles->getByName(u"myStyle"_ustr),
                                           UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPropSet(xMyStyle, UNO_QUERY_THROW);

    uno::Any aBackColor = xPropSet->getPropertyValue(u"CellBackColor"_ustr);
    uno::Any expectedBackColor(sal_Int32(16724787));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong CellBackColor", expectedBackColor, aBackColor);

    // test default pageStyle

    uno::Reference<container::XNameContainer> xPageStyles(xFamilies->getByName(u"PageStyles"_ustr),
                                                          UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xPagePropSet(xPageStyles->getByName(u"Default"_ustr),
                                                     UNO_QUERY_THROW);

    uno::Any aPageBackColor = xPagePropSet->getPropertyValue(u"BackColor"_ustr);
    uno::Any expectedPageBackColor(sal_Int32(13434879));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong page style BackColor", expectedPageBackColor,
                                 aPageBackColor);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
