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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/XAccessibleContextTester.hxx>

/**
 * @brief Tries to get every child and checks its parent.
 *
 * Checks that the parent of every child and the tested component are the same object.
 */
void XAccessibleContextTester::testGetAccessibleChild()
{
    sal_Int64 count = mxContext->getAccessibleChildCount();
    for (sal_Int64 i = 0; i < count && i < AccessibilityTools::MAX_CHILDREN; i++)
    {
        auto child = mxContext->getAccessibleChild(i);
        auto childCtx = child->getAccessibleContext();

        std::cout << "  Child " << i << ": " << AccessibilityTools::debugString(childCtx)
                  << std::endl;

        CPPUNIT_ASSERT_EQUAL_MESSAGE("child's parent context is not parent's context!",
                                     childCtx->getAccessibleParent()->getAccessibleContext(),
                                     mxContext);
    }
}

/**
 * @brief Calls the method.
 *
 * Checks that the child count is non-negative.
 */
void XAccessibleContextTester::testGetAccessibleChildCount()
{
    sal_Int64 childCount = mxContext->getAccessibleChildCount();
    std::cout << childCount << " children found." << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int64>(0), childCount);
}

/**
 * @brief Get the accessible description of the component.
 */
void XAccessibleContextTester::testGetAccessibleDescription()
{
    auto desc = mxContext->getAccessibleDescription();
    std::cout << "The description is '" << desc << "'" << std::endl;
}

/**
 * @brief Checks the index in parent
 *
 * Checks that the parent's child and the tested component are the same objects.
 *
 * Retrieves the index of tested component in its parent.
 * Then gets the parent's child by this index and compares
 * it with tested component.
 */
void XAccessibleContextTester::testGetAccessibleIndexInParent()
{
    sal_Int64 idx = mxContext->getAccessibleIndexInParent();
    std::cout << "The index in parent is " << idx << std::endl;

    auto parent = mxContext->getAccessibleParent();
    CPPUNIT_ASSERT(parent.is());
    auto parentCtx = parent->getAccessibleContext();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Parent's child context at our index is not us!", mxContext,
                                 parentCtx->getAccessibleChild(idx)->getAccessibleContext());
}

/**
 * @brief Get the accessible name of the component.
 */
void XAccessibleContextTester::testGetAccessibleName()
{
    auto name = mxContext->getAccessibleName();
    std::cout << "The name is '" << name << "'" << std::endl;
}

/**
 * @brief Just gets the parent.
 *
 * Checks that the parent is not null.
 */
void XAccessibleContextTester::testGetAccessibleParent()
{
    // assume that the component is not ROOT
    auto parent = mxContext->getAccessibleParent();
    std::cout << "The parent is " << AccessibilityTools::debugString(parent) << std::endl;
    CPPUNIT_ASSERT_MESSAGE("parent is not set", parent.is());
}

/**
 * @brief Just gets the relation set.
 *
 * Checks that the relation set is not null.
 */
void XAccessibleContextTester::testGetAccessibleRelationSet()
{
    auto relSet = mxContext->getAccessibleRelationSet();
    CPPUNIT_ASSERT_MESSAGE("relation set is not set", relSet.is());
}

/**
 * @brief Get the accessible role of component.
 *
 * Checks that the role is a non-negative number.
 */
void XAccessibleContextTester::testGetAccessibleRole()
{
    sal_Int32 role = mxContext->getAccessibleRole();
    std::cout << "The role is " << role << " (" << AccessibilityTools::getRoleName(role) << ")"
              << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), role);
}

/**
 * @brief Just gets the state set.
 *
 * Checks that the state set is not null.
 */
void XAccessibleContextTester::testGetAccessibleStateSet()
{
    sal_Int64 stateSet = mxContext->getAccessibleStateSet();
    std::cout << "The state set is: " << AccessibilityTools::debugAccessibleStateSet(stateSet)
              << std::endl;
}

/**
 * @brief Gets the locale.
 *
 * Checks that @c Country and @c Language fields of locale structure are not empty.
 */
void XAccessibleContextTester::testGetLocale()
{
    auto loc = mxContext->getLocale();
    std::cout << "The locale is " << loc.Language << "," << loc.Country << std::endl;
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), loc.Language.getLength());
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int32>(0), loc.Country.getLength());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
