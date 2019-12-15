/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xindexaccess.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XIndexAccess::testGetCount()
{
    uno::Reference<container::XIndexAccess> xIndexAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(m_nItems, xIndexAccess->getCount());
}

void XIndexAccess::testGetByIndex()
{
    uno::Reference<container::XIndexAccess> xIndexAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(m_nItems, xIndexAccess->getCount());

    if (m_nItems > 0)
    {
        Any aAny = xIndexAccess->getByIndex(0);
        CPPUNIT_ASSERT(aAny.hasValue());

        aAny = xIndexAccess->getByIndex(m_nItems / 2);
        CPPUNIT_ASSERT(aAny.hasValue());

        aAny = xIndexAccess->getByIndex(m_nItems - 1);
        CPPUNIT_ASSERT(aAny.hasValue());
    }
}

void XIndexAccess::testGetByIndexException()
{
    uno::Reference<container::XIndexAccess> xIndexAccess(init(), UNO_QUERY_THROW);
    xIndexAccess->getByIndex(m_nItems);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
