/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xviewfreezable.hxx>

#include <com/sun/star/sheet/XViewFreezable.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XViewFreezable::testFreeze()
{
    uno::Reference<sheet::XViewFreezable> xViewFreezable(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_MESSAGE("View has already frozen panes", !xViewFreezable->hasFrozenPanes());

    xViewFreezable->freezeAtPosition(1, 1);
    CPPUNIT_ASSERT_MESSAGE("Unable to freeze pane", xViewFreezable->hasFrozenPanes());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
