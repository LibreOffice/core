/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/document/xactionlockable.hxx>
#include <sal/types.h>

#include <com/sun/star/document/XActionLockable.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XActionLockable::testAddRemoveActionLock()
{
    uno::Reference<document::XActionLockable> xAL(init(), uno::UNO_QUERY_THROW);

    xAL->addActionLock();
    CPPUNIT_ASSERT(xAL->isActionLocked());

    xAL->removeActionLock();
    CPPUNIT_ASSERT(!xAL->isActionLocked());
}

void XActionLockable::testSetResetActionLock()
{
    uno::Reference<document::XActionLockable> xAL(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_NO_THROW(xAL->setActionLocks(0));

    const sal_Int16 nLocks = xAL->resetActionLocks();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nLocks);
    CPPUNIT_ASSERT(!xAL->isActionLocked());
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
