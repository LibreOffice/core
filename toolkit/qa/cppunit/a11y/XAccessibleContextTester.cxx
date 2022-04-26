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

#include "XAccessibleContextTester.hxx"

#include <cppunit/TestAssert.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include "AccessibilityTools.hxx"

using namespace css::uno;

/**
 * @brief Tries to get every child and checks its parent.
 * @returns @c true if the parent of every child and the tested component are
 *          the same objects.
 */
bool XAccessibleContextTester::testGetAccessibleChild()
{
    int count = mxContext->getAccessibleChildCount();
    for (int i = 0; i < count && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto child = mxContext->getAccessibleChild(i);
        auto childCtx = child->getAccessibleContext();

        std::cout << "  Child " << i << ": " << AccessibilityTools::debugString(childCtx)
                  << std::endl;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("child's parent context is not parent's context!",
                                     childCtx->getAccessibleParent()->getAccessibleContext(),
                                     mxContext);
    }

    return true;
}

/**
 * @brief Calls the method.
 * @returns @c true if the child count is non-negative.
 */
bool XAccessibleContextTester::testGetAccessibleChildCount()
{
    auto childCount = mxContext->getAccessibleChildCount();
    std::cout << childCount << " children found." << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(0, childCount);
    return true;
}

/**
 * @brief Get the accessible description of the component.
 * @returns @c true
 */
bool XAccessibleContextTester::testGetAccessibleDescription()
{
    auto desc = mxContext->getAccessibleDescription();
    std::cout << "The description is '" << desc << "'" << std::endl;
    return true;
}

/**
 * @brief Checks the index in parent
 * @returns @c true if the parent's child and the tested component are the
 *          same objects.
 *
 * Retrieves the index of tested component in its parent.
 * Then gets the parent's child by this index and compares
 * it with tested component.
 */
bool XAccessibleContextTester::testGetAccessibleIndexInParent()
{
    int idx = mxContext->getAccessibleIndexInParent();
    std::cout << "The index in parent is " << idx << std::endl;

    auto parent = mxContext->getAccessibleParent();
    CPPUNIT_ASSERT(parent.is());
    auto parentCtx = parent->getAccessibleContext();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Parent's child context at our index is not us!", mxContext,
                                 parentCtx->getAccessibleChild(idx)->getAccessibleContext());
    return true;
}

/**
 * @brief Get the accessible name of the component.
 * @returns @c true.
 */
bool XAccessibleContextTester::testGetAccessibleName()
{
    auto name = mxContext->getAccessibleName();
    std::cout << "The name is '" << name << "'" << std::endl;
    return true;
}

/**
 * @brief Just gets the parent.
 * @returns @c true if parent is not null.
 */
bool XAccessibleContextTester::testGetAccessibleParent()
{
    // assume that the component is not ROOT
    auto parent = mxContext->getAccessibleParent();
    std::cout << "The parent is " << AccessibilityTools::debugString(parent) << std::endl;
    CPPUNIT_ASSERT_MESSAGE("parent is not set", parent.is());
    return true;
}

/**
 * @brief Just gets the set.
 * @returns @x true if the set is not null.
 */
bool XAccessibleContextTester::testGetAccessibleRelationSet()
{
    auto relSet = mxContext->getAccessibleRelationSet();
    CPPUNIT_ASSERT_MESSAGE("relation set is not set", relSet.is());
    return true;
}

/**
 * @breif Get the accessible role of component.
 * @returns @c true if non-negative number returned.
 */
bool XAccessibleContextTester::testGetAccessibleRole()
{
    sal_Int16 role = mxContext->getAccessibleRole();
    std::cout << "The role is " << role << " (" << AccessibilityTools::getRoleName(role) << ")"
              << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int16>(0), role);
    return true;
}

/**
 * @brief Just gets the set.
 * @returns @c true if the set is not null.
 */
bool XAccessibleContextTester::testGetAccessibleStateSet()
{
    auto stateSet = mxContext->getAccessibleStateSet();
    std::cout << "The state set is: " << AccessibilityTools::debugString(stateSet) << std::endl;
    CPPUNIT_ASSERT_MESSAGE("state set is not set", stateSet.is());
    return true;
}

/**
 * @brief Gets the locale.
 * @returns @c true if @c Country and @c Language fields of locale structure
 *          are not empty.
 */
bool XAccessibleContextTester::testGetLocale()
{
    auto loc = mxContext->getLocale();
    std::cout << "The locale is " << loc.Language << "," << loc.Country << std::endl;
    CPPUNIT_ASSERT_GREATER(0, loc.Language.getLength());
    CPPUNIT_ASSERT_GREATER(0, loc.Country.getLength());
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
