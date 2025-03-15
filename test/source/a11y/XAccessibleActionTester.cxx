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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <test/a11y/XAccessibleActionTester.hxx>

/**
 * @brief Calls XAccessibleAction::getAccessibleActionCount.
 *
 * Checks that the action count is non-negative.
 */
void XAccessibleActionTester::testGetAccessibleActionCount()
{
    sal_Int32 nActionCount = mxAction->getAccessibleActionCount();
    std::cout << nActionCount << " actions found." << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(static_cast<sal_Int32>(0), nActionCount);
}

/**
 * @brief Calls XAccessibleAction::doAccessibleAction for all actions.
 *
 * Also checks that calling the method with an invalid index
 * throws the expected exception.
 */
void XAccessibleActionTester::testDoAccessibleAction()
{
    const sal_Int32 nActionCount = mxAction->getAccessibleActionCount();

    try
    {
        mxAction->doAccessibleAction(nActionCount);
        CPPUNIT_FAIL("Didn't throw expected exception.");
    }
    catch (css::lang::IndexOutOfBoundsException&)
    {
    }

    for (sal_Int32 i = 0; i < nActionCount; i++)
    {
        std::cout << "do action " + mxAction->getAccessibleActionDescription(i);
        mxAction->doAccessibleAction(i);
    }
}

/**
 * @brief Calls XAccessibleAction::getAccessibleActionDescription for all actions.
 *
 * Checks that the description is not empty.
 *
 * Also checks that calling the method with an invalid index
 * throws the expected exception.
 */
void XAccessibleActionTester::testGetAccessibleActionDescription()
{
    const sal_Int32 nActionCount = mxAction->getAccessibleActionCount();

    try
    {
        mxAction->getAccessibleActionDescription(nActionCount);
        CPPUNIT_FAIL("Didn't throw expected exception.");
    }
    catch (css::lang::IndexOutOfBoundsException&)
    {
    }

    for (sal_Int32 i = 0; i < nActionCount; i++)
    {
        const OUString sDescription = mxAction->getAccessibleActionDescription(i);
        CPPUNIT_ASSERT_MESSAGE("Action description is empty", !sDescription.isEmpty());
    }
}

/**
 * @brief Calls XAccessibleAction::getAccessibleActionKeyBinding for all actions.
 *
 * Also checks that calling the method with an invalid index
 * throws the expected exception.
 */
void XAccessibleActionTester::testGetAccessibleActionKeyBinding()
{
    const sal_Int32 nActionCount = mxAction->getAccessibleActionCount();

    try
    {
        mxAction->getAccessibleActionKeyBinding(nActionCount);
        CPPUNIT_FAIL("Didn't throw expected exception.");
    }
    catch (css::lang::IndexOutOfBoundsException&)
    {
    }

    for (sal_Int32 i = 0; i < nActionCount; i++)
    {
        mxAction->getAccessibleActionKeyBinding(i);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
