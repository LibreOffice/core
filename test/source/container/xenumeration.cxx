/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xenumeration.hxx>

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XEnumeration.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XEnumeration::testHasMoreElements()
{
    uno::Reference<container::XEnumeration> xEnumeration(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xEnumeration->hasMoreElements());
}

void XEnumeration::testNextElement()
{
    uno::Reference<container::XEnumeration> xEnumeration(init(), uno::UNO_QUERY_THROW);

    // unwind all elements from the enumeration
    while (xEnumeration->hasMoreElements())
        CPPUNIT_ASSERT_NO_THROW(xEnumeration->nextElement());

    CPPUNIT_ASSERT_THROW_MESSAGE("Incorrect exception", xEnumeration->nextElement(),
                                 container::NoSuchElementException);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
