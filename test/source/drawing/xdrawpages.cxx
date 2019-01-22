/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/drawing/xdrawpages.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <sal/types.h>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XDrawPages::testInsertNewByIndex()
{
    uno::Reference<drawing::XDrawPages> xDrawPages(init(), uno::UNO_QUERY_THROW);
    const sal_Int32 nCount = xDrawPages->getCount();

    uno::Reference<drawing::XDrawPage> xDP(xDrawPages->insertNewByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDP.is());
    CPPUNIT_ASSERT_EQUAL(nCount + 1, xDrawPages->getCount());
}

void XDrawPages::testRemove()
{
    uno::Reference<drawing::XDrawPages> xDrawPages(init(), uno::UNO_QUERY_THROW);
    const sal_Int32 nCount = xDrawPages->getCount();

    uno::Reference<drawing::XDrawPage> xDP(xDrawPages->insertNewByIndex(0), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xDP.is());

    xDrawPages->remove(xDP);
    CPPUNIT_ASSERT_EQUAL(nCount, xDrawPages->getCount());
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
