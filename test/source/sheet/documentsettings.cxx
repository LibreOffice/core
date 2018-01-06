/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/documentsettings.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void DocumentSettings::testDocumentSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xDocumentSettings(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "ShowZeroValues";
    bool aShowZeroValues = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowZeroValues);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowZeroValues", aShowZeroValues);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowZeroValues);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowZeroValues", !aShowZeroValues);

    propName = "ShowNotes";
    bool aShowNotes = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowNotes);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowNotes", aShowNotes);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowNotes);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowNotes", !aShowNotes);

    propName = "ShowGrid";
    bool aShowGrid = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowGrid);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowGrid", aShowGrid);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowGrid);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowGrid", !aShowGrid);

    propName = "ShowPageBreaks";
    bool aShowPageBreaks = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowPageBreaks", aShowPageBreaks);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aShowPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowPageBreaks", !aShowPageBreaks);

    propName = "HasColumnRowHeaders";
    bool aHasColumnRowHeaders = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aHasColumnRowHeaders);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasColumnRowHeaders", aHasColumnRowHeaders);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aHasColumnRowHeaders);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasColumnRowHeaders",
                           !aHasColumnRowHeaders);

    propName = "HasSheetTabs";
    bool aHasSheetTabs = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aHasSheetTabs);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasSheetTabs", aHasSheetTabs);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aHasSheetTabs);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasSheetTabs", !aHasSheetTabs);

    propName = "IsOutlineSymbolsSet";
    bool aIsOutlineSymbolsSet = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsOutlineSymbolsSet);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsOutlineSymbolsSet", aIsOutlineSymbolsSet);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsOutlineSymbolsSet);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsOutlineSymbolsSet",
                           !aIsOutlineSymbolsSet);

    propName = "IsSnapToRaster";
    bool aIsSnapToRaster = true;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsSnapToRaster);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsSnapToRaster", !aIsSnapToRaster);

    aNewValue <<= true;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsSnapToRaster);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsSnapToRaster", aIsSnapToRaster);

    propName = "RasterIsVisible";
    bool aRasterIsVisible = true;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterIsVisible);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue RasterIsVisible", !aRasterIsVisible);

    aNewValue <<= true;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterIsVisible);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue RasterIsVisible", aRasterIsVisible);

    propName = "RasterResolutionX";
    sal_Int32 aRasterResolutionX = 0;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterResolutionX);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue RasterResolutionX", sal_Int32(1270),
                                 aRasterResolutionX);

    aNewValue <<= sal_Int32(42);
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterResolutionX);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue RasterResolutionX", sal_Int32(42),
                                 aRasterResolutionX);

    propName = "RasterResolutionY";
    sal_Int32 aRasterResolutionY = 0;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterResolutionY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue RasterResolutionY", sal_Int32(1270),
                                 aRasterResolutionY);

    aNewValue <<= sal_Int32(42);
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterResolutionY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue RasterResolutionY", sal_Int32(42),
                                 aRasterResolutionY);

    propName = "RasterSubdivisionX";
    sal_Int32 aRasterSubdivisionX = 0;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterSubdivisionX);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue RasterSubdivisionX", sal_Int32(1),
                                 aRasterSubdivisionX);

    aNewValue <<= sal_Int32(42);
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterSubdivisionX);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue RasterSubdivisionX", sal_Int32(42),
                                 aRasterSubdivisionX);

    propName = "RasterSubdivisionY";
    sal_Int32 aRasterSubdivisionY = 0;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterSubdivisionY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue RasterSubdivisionY", sal_Int32(1),
                                 aRasterSubdivisionY);

    aNewValue <<= sal_Int32(42);
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aRasterSubdivisionY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue RasterSubdivisionY", sal_Int32(42),
                                 aRasterSubdivisionY);

    propName = "IsRasterAxisSynchronized";
    bool aIsRasterAxisSynchronized = false;
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsRasterAxisSynchronized);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsRasterAxisSynchronized",
                           aIsRasterAxisSynchronized);

    aNewValue <<= false;
    xDocumentSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xDocumentSettings->getPropertyValue(propName) >>= aIsRasterAxisSynchronized);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsRasterAxisSynchronized",
                           !aIsRasterAxisSynchronized);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
