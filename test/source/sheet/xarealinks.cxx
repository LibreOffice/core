/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xarealinks.hxx>

#include <com/sun/star/sheet/XAreaLinks.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XAreaLinks::testInsertAtPosition()
{
    uno::Reference< sheet::XAreaLinks > xAreaLinks(init(), UNO_QUERY_THROW);

    table::CellAddress aCellAddress(1, 2, 3);
    xAreaLinks->insertAtPosition(aCellAddress, u""_ustr, u"a2:b5"_ustr, u""_ustr, u""_ustr);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to insert new CellAddress",
                                 sal_Int32(1), xAreaLinks->getCount());
}

void XAreaLinks::testRemoveByIndex()
{
    uno::Reference< sheet::XAreaLinks > xAreaLinks(init(), UNO_QUERY_THROW);

    table::CellAddress aCellAddress(1, 2, 3);
    xAreaLinks->insertAtPosition(aCellAddress, u""_ustr, u"a2:b5"_ustr, u""_ustr, u""_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to insert CellAddress for removal",
                                 sal_Int32(1), xAreaLinks->getCount());

    xAreaLinks->removeByIndex(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove CellAddress",
                                 sal_Int32(0), xAreaLinks->getCount());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
