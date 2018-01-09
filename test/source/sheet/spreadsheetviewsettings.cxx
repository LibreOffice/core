/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/spreadsheetviewsettings.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void SpreadsheetViewSettings::testSpreadsheetViewSettingsProperties()
{
    uno::Reference<beans::XPropertySet> xSpreadsheetViewSettings(init(), UNO_QUERY_THROW);
    OUString propName;
    uno::Any aNewValue;

    propName = "ShowFormulas";
    bool aShowFormulas = true;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowFormulas);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowFormulas", !aShowFormulas);

    aNewValue <<= true;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowFormulas);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowFormulas", aShowFormulas);

    propName = "ShowZeroValues";
    bool aShowZeroValues = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowZeroValues);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowZeroValues", aShowZeroValues);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowZeroValues);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowZeroValues", !aShowZeroValues);

    propName = "IsValueHighlightingEnabled";
    bool aIsValueHighlightingEnabled = true;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName)
                   >>= aIsValueHighlightingEnabled);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsValueHighlightingEnabled",
                           !aIsValueHighlightingEnabled);

    aNewValue <<= true;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName)
                   >>= aIsValueHighlightingEnabled);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsValueHighlightingEnabled",
                           aIsValueHighlightingEnabled);

    propName = "ShowNotes";
    bool aShowNotes = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowNotes);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowNotes", aShowNotes);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowNotes);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowNotes", !aShowNotes);

    propName = "HasVerticalScrollBar";
    bool aHasVerticalScrollBar = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasVerticalScrollBar);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasVerticalScrollBar",
                           aHasVerticalScrollBar);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasVerticalScrollBar);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasVerticalScrollBar",
                           !aHasVerticalScrollBar);

    propName = "HasHorizontalScrollBar";
    bool aHasHorizontalScrollBar = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName)
                   >>= aHasHorizontalScrollBar);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasHorizontalScrollBar",
                           aHasHorizontalScrollBar);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName)
                   >>= aHasHorizontalScrollBar);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasHorizontalScrollBar",
                           !aHasHorizontalScrollBar);

    propName = "HasSheetTabs";
    bool aHasSheetTabs = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasSheetTabs);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasSheetTabs", aHasSheetTabs);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasSheetTabs);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasSheetTabs", !aHasSheetTabs);

    propName = "IsOutlineSymbolsSet";
    bool aIsOutlineSymbolsSet = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aIsOutlineSymbolsSet);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue IsOutlineSymbolsSet", aIsOutlineSymbolsSet);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aIsOutlineSymbolsSet);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue IsOutlineSymbolsSet",
                           !aIsOutlineSymbolsSet);

    propName = "HasColumnRowHeaders";
    bool aHasColumnRowHeaders = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasColumnRowHeaders);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue HasColumnRowHeaders", aHasColumnRowHeaders);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aHasColumnRowHeaders);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue HasColumnRowHeaders",
                           !aHasColumnRowHeaders);

    propName = "ShowGrid";
    bool aShowGrid = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowGrid);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowGrid", aShowGrid);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowGrid);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowGrid", !aShowGrid);

    propName = "GridColor";
    util::Color aGridColor;
    xSpreadsheetViewSettings->getPropertyValue(propName) >>= aGridColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue GridColor", sal_Int32(12632256),
                                 aGridColor);

    aNewValue <<= sal_Int32(42);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    xSpreadsheetViewSettings->getPropertyValue(propName) >>= aGridColor;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue GridColor", sal_Int32(42),
                                 aGridColor);

    propName = "ShowHelpLines";
    bool aShowHelpLines = true;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowHelpLines);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowHelpLines", !aShowHelpLines);

    aNewValue <<= true;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowHelpLines);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowHelpLines", aShowHelpLines);

    propName = "ShowAnchor";
    bool aShowAnchor = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowAnchor);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowAnchor", aShowAnchor);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowAnchor);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowAnchor", !aShowAnchor);

    propName = "ShowPageBreaks";
    bool aShowPageBreaks = false;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ShowPageBreaks", aShowPageBreaks);

    aNewValue <<= false;
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowPageBreaks);
    CPPUNIT_ASSERT_MESSAGE("Unable to set PropertyValue ShowPageBreaks", !aShowPageBreaks);

    propName = "ShowObjects";
    sal_Int16 aShowObjects = 42;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowObjects);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue ShowObjects", sal_Int16(0),
                                 aShowObjects);

    aNewValue <<= sal_Int16(1);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowObjects);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ShowObjects", sal_Int16(1),
                                 aShowObjects);

    propName = "ShowCharts";
    sal_Int16 aShowCharts = 42;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowCharts);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue ShowCharts", sal_Int16(0),
                                 aShowCharts);

    aNewValue <<= sal_Int16(1);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowCharts);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ShowCharts", sal_Int16(1),
                                 aShowCharts);

    propName = "ShowDrawing";
    sal_Int16 aShowDrawing = 42;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowDrawing);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue ShowDrawing", sal_Int16(0),
                                 aShowDrawing);

    aNewValue <<= sal_Int16(1);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aShowDrawing);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ShowDrawing", sal_Int16(1),
                                 aShowDrawing);

    propName = "ZoomType";
    sal_Int16 aZoomType = 42;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aZoomType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue ZoomType", sal_Int16(3), aZoomType);

    aNewValue <<= sal_Int16(1);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aZoomType);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ZoomType", sal_Int16(1), aZoomType);

    propName = "ZoomValue";
    sal_Int16 aZoomValue = 42;
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aZoomValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get PropertyValue ZoomValue", sal_Int16(116),
                                 aZoomValue);

    aNewValue <<= sal_Int16(1);
    xSpreadsheetViewSettings->setPropertyValue(propName, aNewValue);
    CPPUNIT_ASSERT(xSpreadsheetViewSettings->getPropertyValue(propName) >>= aZoomValue);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set PropertyValue ZoomValue", sal_Int16(1), aZoomValue);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
