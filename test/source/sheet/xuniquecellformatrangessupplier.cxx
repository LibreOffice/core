/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xuniquecellformatrangessupplier.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XUniqueCellFormatRangesSupplier::testGetUniqueCellFormatRanges()
{
    uno::Reference< sheet::XUniqueCellFormatRangesSupplier > xSupplier(init(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndexAccess = xSupplier->getUniqueCellFormatRanges();

    auto count = xIndexAccess->getCount();
    CPPUNIT_ASSERT_MESSAGE("No elements found", count);
    for (auto i = 0; i<count; i++) {
        CPPUNIT_ASSERT_NO_THROW_MESSAGE(OString("Unable to access element " + OString::number(i)).getStr(),
                                        xIndexAccess->getByIndex(i));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
