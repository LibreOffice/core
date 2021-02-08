/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <test/bootstrapfixture.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/XUnitConversion.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace ::com::sun::star;

namespace
{
class ToolkitTest : public test::BootstrapFixture
{
public:
    void testXUnitConversion();

    CPPUNIT_TEST_SUITE(ToolkitTest);
    CPPUNIT_TEST(testXUnitConversion);
    CPPUNIT_TEST_SUITE_END();
};

/**
 * Creates a floating XWindow on the given position and size.
 * @return a floating XWindow
 * @param X the X-Position of the floating XWindow
 * @param Y the Y-Position of the floating XWindow
 * @param width the width of the floating XWindow
 * @param height the height of the floating XWindow
 * @param xMSF the MultiServiceFactory
 */
uno::Reference<awt::XWindowPeer>
createFloatingWindow(uno::Reference<lang::XMultiServiceFactory> const& xMSF, sal_Int32 const nX,
                     sal_Int32 const nY, sal_Int32 const nWidth, sal_Int32 const nHeight)
{
    uno::Reference<awt::XToolkit> const xTk(xMSF->createInstance("com.sun.star.awt.Toolkit"),
                                            uno::UNO_QUERY);

    awt::WindowDescriptor descriptor;
    descriptor.Type = awt::WindowClass_TOP;
    descriptor.WindowServiceName = "modelessdialog";
    descriptor.ParentIndex = -1;
    descriptor.Bounds.X = nX;
    descriptor.Bounds.Y = nY;
    descriptor.Bounds.Width = nWidth;
    descriptor.Bounds.Height = nHeight;
    descriptor.WindowAttributes
        = (awt::WindowAttribute::BORDER + awt::WindowAttribute::MOVEABLE
           + awt::WindowAttribute::SIZEABLE + awt::WindowAttribute::CLOSEABLE
           + awt::VclWindowPeerAttribute::CLIPCHILDREN);

    return xTk->createWindow(descriptor);
}

/**
 * Not really a check,
 * only a simple test call to convertSizeToLogic(...) with different parameters
 */
void checkSize(uno::Reference<awt::XUnitConversion> const& xConv, awt::Size const& rSize,
               sal_Int16 const nMeasureUnit, OUString const& rUnit)
{
    awt::Size const aSizeIn = xConv->convertSizeToLogic(rSize, nMeasureUnit);
    std::cerr << "Window size:\n";
    std::cerr << "Width: " << aSizeIn.Width << " " << rUnit << "\n";
    std::cerr << "Height: " << aSizeIn.Height << " " << rUnit << "\n";
}

/**
 * The real test function
 * 2. try to create an empty window
 * 3. try to convert the WindowPeer to an XWindow
 * 4. try to resize and move the window to another position, so we get a well knowing position and size.
 * 5. run some more tests
 */
void ToolkitTest::testXUnitConversion()
{
#ifdef _WIN32
    HKEY hkey;
    DWORD type;
    DWORD data;
    DWORD size(sizeof(data));
    LONG ret = ::RegOpenKeyW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", &hkey);
    if (ret == ERROR_SUCCESS)
    {
        ret = ::RegQueryValueExW(hkey, L"LogPixels", nullptr, &type,
                                 reinterpret_cast<LPBYTE>(&data), &size);
        if (ret == ERROR_SUCCESS && type == REG_DWORD && data != 96)
        {
            std::cerr << "non-default resolution, skipping textXUnitConversion\n";
            return;
        }
    }
#endif

    // create a window
    sal_Int32 x = 100;
    sal_Int32 y = 100;
    sal_Int32 width = 640;
    sal_Int32 height = 480;
    uno::Reference<awt::XWindowPeer> const xWindowPeer
        = createFloatingWindow(getMultiServiceFactory(), x, y, width, height);
    CPPUNIT_ASSERT(xWindowPeer.is());

    // resize and move the window to a well known position and size
    uno::Reference<awt::XWindow> const xWindow(xWindowPeer, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xWindow.is());

    xWindow->setVisible(true);

    awt::Rectangle aRect = xWindow->getPosSize();
    awt::Point aPoint(aRect.X, aRect.Y);
    awt::Size aSize(aRect.Width, aRect.Height);

    std::cerr << "Window position and size in pixel:\n";
    std::cerr << "X: " << aPoint.X << "\n";
    std::cerr << "Y: " << aPoint.Y << "\n";
    std::cerr << "Width: " << aSize.Width << "\n";
    std::cerr << "Height: " << aSize.Height << "\n";

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Window size wrong", width, aSize.Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Window size wrong", height, aSize.Height);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Window pos wrong", x, aPoint.X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Window pos wrong", y, aPoint.Y);

    uno::Reference<awt::XUnitConversion> const xConv(xWindowPeer, uno::UNO_QUERY);

    // try to get the position of the window in 1/100mm with the XUnitConversion method
    awt::Point const aPointInMM_100TH
        = xConv->convertPointToLogic(aPoint, util::MeasureUnit::MM_100TH);
    std::cerr << "Window position:\n";
    std::cerr << "X: " << aPointInMM_100TH.X << " 1/100mm\n";
    std::cerr << "Y: " << aPointInMM_100TH.Y << " 1/100mm\n";

    // try to get the size of the window in 1/100mm with the XUnitConversion method
    awt::Size const aSizeInMM_100TH = xConv->convertSizeToLogic(aSize, util::MeasureUnit::MM_100TH);
    std::cerr << "Window size:\n";
    std::cerr << "Width: " << aSizeInMM_100TH.Width << " 1/100mm\n";
    std::cerr << "Height: " << aSizeInMM_100TH.Height << " 1/100mm\n";

    // try to get the size of the window in 1/10mm with the XUnitConversion method
    awt::Size const aSizeInMM_10TH = xConv->convertSizeToLogic(aSize, util::MeasureUnit::MM_10TH);
    std::cerr << "Window size:\n";
    std::cerr << "Width: " << aSizeInMM_10TH.Width << " 1/10mm\n";
    std::cerr << "Height: " << aSizeInMM_10TH.Height << " 1/10mm\n";

    // check the size with a delta which must be smaller a given difference
    CPPUNIT_ASSERT_MESSAGE("Size.Width  not correct",
                           std::abs(aSizeInMM_100TH.Width - aSizeInMM_10TH.Width * 10) < 10);
    CPPUNIT_ASSERT_MESSAGE("Size.Height not correct",
                           std::abs(aSizeInMM_100TH.Height - aSizeInMM_10TH.Height * 10) < 10);

    // new
    checkSize(xConv, aSize, util::MeasureUnit::PIXEL, "pixel");
    checkSize(xConv, aSize, util::MeasureUnit::APPFONT, "appfont");
    checkSize(xConv, aSize, util::MeasureUnit::SYSFONT, "sysfont");

    // simply check some more parameters
    checkSize(xConv, aSize, util::MeasureUnit::MM, "mm");
    checkSize(xConv, aSize, util::MeasureUnit::CM, "cm");
    checkSize(xConv, aSize, util::MeasureUnit::INCH_1000TH, "1/1000inch");
    checkSize(xConv, aSize, util::MeasureUnit::INCH_100TH, "1/100inch");
    checkSize(xConv, aSize, util::MeasureUnit::INCH_10TH, "1/10inch");
    checkSize(xConv, aSize, util::MeasureUnit::INCH, "inch");
    checkSize(xConv, aSize, util::MeasureUnit::POINT, "point");
    checkSize(xConv, aSize, util::MeasureUnit::TWIP, "twip");

    // convert the 1/100mm window size back to pixel
    awt::Size const aNewSize
        = xConv->convertSizeToPixel(aSizeInMM_100TH, util::MeasureUnit::MM_100TH);
    std::cerr << "Window size:\n";
    std::cerr << "Width: " << aNewSize.Width << " pixel\n";
    std::cerr << "Height: " << aNewSize.Height << " pixel\n";

    // assure the pixels are the same as we already know
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed: Size from pixel to 1/100mm to pixel", aNewSize.Width,
                                 aSize.Width);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("failed: Size from pixel to 1/100mm to pixel", aNewSize.Height,
                                 aSize.Height);

    // close the window.
    xWindow->dispose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ToolkitTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
