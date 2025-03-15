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

#include <cppunit/TestAssert.h>

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <tools/color.hxx>

#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/XAccessibleComponentTester.hxx>

using namespace css;

/**
 * @brief Checks the component's bounds
 *
 * Checks the X and Y coordinates are non-negative, and that the width and
 * height are greater than 0.
 *
 * Coherence with @c getLocation() is tested in the test for the
 * latter.
 */
void XAccessibleComponentTester::testBounds()
{
    auto bounds = mxComponent->getBounds();
    std::cout << "bounds: " << bounds.Width << "x" << bounds.Height << std::showpos << bounds.X
              << bounds.Y << std::noshowpos << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), bounds.X);
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), bounds.Y);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), bounds.Width);
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), bounds.Height);
}

/**
 * @brief Tests results of XAccessibleComponent::getSize()
 *
 * Succeeds if the size is the same as in bounds.
 */
void XAccessibleComponentTester::testSize()
{
    auto bounds = mxComponent->getBounds();
    auto size = mxComponent->getSize();
    CPPUNIT_ASSERT_EQUAL(bounds.Width, size.Width);
    CPPUNIT_ASSERT_EQUAL(bounds.Height, size.Height);
}

/**
 * @brief Tests results of XAccessibleComponent::getBounds()
 *
 * First checks 4 inner bounds (upper, lower, left and right) of component
 * bounding box to contain at least one point of the component. Second 4 outer
 * bounds are checked to not contain any component points.
 *
 * Succeeds if inner bounds contain component points and outer bounds don't
 * contain any component points.
 */
void XAccessibleComponentTester::testContainsPoint()
{
    auto bounds = mxComponent->getBounds();

    /* upper end */
    int curX = 0;
    while (!mxComponent->containsPoint(awt::Point(curX, 0)) && curX < bounds.Width)
        curX++;
    CPPUNIT_ASSERT_MESSAGE("Upper bound of box contains no component points", curX < bounds.Width);
    std::cout << "Upper bound of box contains point (" << curX << ",0)" << std::endl;
    /* lower end */
    curX = 0;
    while (!mxComponent->containsPoint(awt::Point(curX, bounds.Height - 1)) && curX < bounds.Width)
        curX++;
    CPPUNIT_ASSERT_MESSAGE("Lower bound of box contains no component points", curX < bounds.Width);
    std::cout << "Lower bound of box contains point (" << curX << "," << (bounds.Height - 1) << ")"
              << std::endl;
    /* left end */
    int curY = 0;
    while (!mxComponent->containsPoint(awt::Point(0, curY)) && curY < bounds.Height)
        curY++;
    CPPUNIT_ASSERT_MESSAGE("Left bound of box contains no component points", curY < bounds.Height);
    std::cout << "Left bound of box contains point (0," << curY << ")" << std::endl;
    /* right end */
    curY = 0;
    while (!mxComponent->containsPoint(awt::Point(bounds.Width - 1, curY)) && curY < bounds.Height)
        curY++;
    CPPUNIT_ASSERT_MESSAGE("Right bound of box contains no component points", curY < bounds.Height);
    std::cout << "Right bound of box contains point (" << (bounds.Width - 1) << "," << curY << ")"
              << std::endl;
    /* no match outside the bounds */
    for (int x = -1; x <= bounds.Width; x++)
    {
        CPPUNIT_ASSERT_MESSAGE("Outer upper bound CONTAINS a component point",
                               !mxComponent->containsPoint(awt::Point(x, -1)));
        CPPUNIT_ASSERT_MESSAGE("Outer lower bound CONTAINS a component point",
                               !mxComponent->containsPoint(awt::Point(x, bounds.Height)));
    }
    for (int y = -1; y <= bounds.Height; y++)
    {
        CPPUNIT_ASSERT_MESSAGE("Outer left bound CONTAINS a component point",
                               !mxComponent->containsPoint(awt::Point(-1, y)));
        CPPUNIT_ASSERT_MESSAGE("Outer right bound CONTAINS a component point",
                               !mxComponent->containsPoint(awt::Point(bounds.Width, y)));
    }
}

/**
 * @brief Tests results of XAccessibleComponent::getAccessibleAtPoint()
 *
 * Iterates through all children which implement
 * <code>XAccessibleComponent</code> (if they exist) determines their
 * boundaries and tries to get each child by <code>getAccessibleAtPoint</code>
 * passing point which belongs to the child.
 * Also the point is checked which doesn't belong to child boundary box.
 *
 * Succeeds if in the first cases the right children are returned, and in the
 * second <code>null</code> or another child is returned.
 */
void XAccessibleComponentTester::testAccessibleAtPoint()
{
    sal_Int64 count = mxContext->getAccessibleChildCount();
    std::cout << "Found " << count << " children" << std::endl;
    for (sal_Int64 i = 0; i < count && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto child = mxContext->getAccessibleChild(i);
        uno::Reference<accessibility::XAccessibleContext> childContext(
            child->getAccessibleContext(), uno::UNO_SET_THROW);
        std::cout << "* Found child: " << AccessibilityTools::debugString(child) << std::endl;
        std::cout << "  states: "
                  << AccessibilityTools::debugAccessibleStateSet(
                         childContext->getAccessibleStateSet())
                  << std::endl;
        uno::Reference<accessibility::XAccessibleComponent> xChildComponent(childContext,
                                                                            uno::UNO_QUERY);
        std::cout << "  component: " << xChildComponent << std::endl;
        if (!xChildComponent)
            continue;

        auto childBounds = xChildComponent->getBounds();
        if (childBounds.X == -1)
            continue;
        std::cout << "  bounds: " << childBounds.Width << "x" << childBounds.Height << std::showpos
                  << childBounds.X << childBounds.Y << std::noshowpos << std::endl;

        std::cout << "finding the point which lies on the component" << std::endl;
        int curX = 0;
        int curY = 0;
        while (!xChildComponent->containsPoint(awt::Point(curX, curY)) && curX < childBounds.Width)
        {
            curX++;
            curY++;
        }
        if (curX >= childBounds.Width)
        {
            std::cout << "Couldn't find a point with contains" << std::endl;
            continue;
        }
        else
        {
            std::cout << "Child found at point +" << childBounds.X + curX << "+"
                      << childBounds.Y + curY << std::endl;
        }

        // trying the point laying on child
        auto xAccAtPoint
            = mxComponent->getAccessibleAtPoint(awt::Point(childBounds.X, childBounds.Y));
        CPPUNIT_ASSERT_MESSAGE("Child not found at point", xAccAtPoint.is());
        if (!AccessibilityTools::equals(child, xAccAtPoint))
        {
            auto idxExpected = childContext->getAccessibleIndexInParent();
            auto idxResult = xAccAtPoint->getAccessibleContext()->getAccessibleIndexInParent();
            std::cout << "The child found (" << AccessibilityTools::debugString(xAccAtPoint)
                      << ") is not the expected one (" << AccessibilityTools::debugString(child)
                      << ")" << std::endl;
            if (idxExpected < idxResult)
            {
                std::cout << "-- it probably is hidden behind?  Skipping." << std::endl;
            }
            else
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("The child found is NOT the expected one", child,
                                             xAccAtPoint);
            }
        }

        // trying the point NOT laying on child
        xAccAtPoint
            = mxComponent->getAccessibleAtPoint(awt::Point(childBounds.X - 1, childBounds.Y - 1));
        if (xAccAtPoint.is())
        {
            CPPUNIT_ASSERT_MESSAGE("Child found OUTSIDE its bounds",
                                   !AccessibilityTools::equals(child, xAccAtPoint));
        }
    }
}

/**
 * @brief Tests results of XAccessibleComponent::getLocation()
 *
 * Succeeds if the location is the same as location of boundary obtained by
 * the <code>getBounds()</code> method.
 */
void XAccessibleComponentTester::testLocation()
{
    auto bounds = mxComponent->getBounds();
    auto location = mxComponent->getLocation();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid X location", bounds.X, location.X);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Y location", bounds.Y, location.Y);
}

/**
 * @brief Tests results of XAccessibleComponent::getLocationOnScreen()
 * @returns @c true on success.
 *
 * Get the screen location of the component and its parent
 * (if it exists and supports <code>XAccessibleComponent</code>).
 *
 * Succeeds component screen location equals to screen location of its parent
 * plus location of the component relative to the parent.
 */
void XAccessibleComponentTester::testLocationOnScreen()
{
    auto location = mxComponent->getLocationOnScreen();
    std::cout << "location on screen: +" << location.X << "+" << location.Y << std::endl;

    auto xParent = mxContext->getAccessibleParent();
    if (!xParent.is())
        std::cout << "No parent" << std::endl;
    else
    {
        std::cout << "Found parent: " << AccessibilityTools::debugString(xParent) << std::endl;
        uno::Reference<accessibility::XAccessibleComponent> xParentComponent(
            xParent->getAccessibleContext(), uno::UNO_QUERY);
        if (!xParentComponent.is())
            std::cout << "Parent is not a Component" << std::endl;
        else
        {
            auto bounds = mxComponent->getBounds();
            auto parentLocation = xParentComponent->getLocationOnScreen();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid X screen location", parentLocation.X + bounds.X,
                                         location.X);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Invalid Y screen location", parentLocation.Y + bounds.Y,
                                         location.Y);
        }
    }
}

/**
 * Just calls the method.
 */
void XAccessibleComponentTester::testGrabFocus() { mxComponent->grabFocus(); }

/**
 * Just calls the method.
 */
void XAccessibleComponentTester::testGetForeground()
{
    auto color = mxComponent->getForeground();
    std::cout << "foreground color: " << Color(ColorAlpha, color) << std::endl;
}

/**
 * Just calls the method.
 */
void XAccessibleComponentTester::testGetBackground()
{
    auto color = mxComponent->getBackground();
    std::cout << "background color: " << Color(ColorAlpha, color) << std::endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
