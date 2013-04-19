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
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

XIndexAccess::XIndexAccess(sal_Int32 nItems):
    mnItems(nItems)
{
}

void XIndexAccess::testGetCount()
{
    uno::Reference< container::XIndexAccess > xIndexAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mnItems == xIndexAccess->getCount());
}

void XIndexAccess::testGetByIndex()
{
    uno::Reference< container::XIndexAccess > xIndexAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(mnItems == xIndexAccess->getCount());

    if (mnItems > 0)
    {
        Any aAny = xIndexAccess->getByIndex(0);
        CPPUNIT_ASSERT(aAny.hasValue());

        aAny = xIndexAccess->getByIndex(mnItems/2);
        CPPUNIT_ASSERT(aAny.hasValue());

        aAny = xIndexAccess->getByIndex(mnItems-1);
        CPPUNIT_ASSERT(aAny.hasValue());
    }
}

void XIndexAccess::testGetByIndexException()
{
    uno::Reference< container::XIndexAccess > xIndexAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xIndexAccess.is());

    xIndexAccess->getByIndex(mnItems);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
