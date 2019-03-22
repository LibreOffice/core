/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xviewsplitable.hxx>

#include <com/sun/star/sheet/XViewSplitable.hpp>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XViewSplitable::testSplit()
{
    uno::Reference< sheet::XViewSplitable > xViewSplitable(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("View is already split", !xViewSplitable->getIsWindowSplit());

    xViewSplitable->splitAtPosition(90, 51);

    CPPUNIT_ASSERT_MESSAGE("View wasn't split", xViewSplitable->getIsWindowSplit());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong horizontal pixel position",
                                 sal_Int32(90), xViewSplitable->getSplitHorizontal());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong vertical pixel position",
                                 sal_Int32(51), xViewSplitable->getSplitVertical());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong column",
                                 sal_Int32(1), xViewSplitable->getSplitColumn());
    CPPUNIT_ASSERT_MESSAGE("Wrong row", xViewSplitable->getSplitRow() != 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
