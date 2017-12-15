/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellformatrangessupplier.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sheet/XCellFormatRangesSupplier.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XCellFormatRangesSupplier::testGetCellFormatRanges()
{
    uno::Reference<sheet::XCellFormatRangesSupplier> xCFRS(init(), UNO_QUERY_THROW);

    uno::Reference<container::XIndexAccess> xIA(xCFRS->getCellFormatRanges(), UNO_QUERY_THROW);

    const sal_Int32 nCount = xIA->getCount();
    CPPUNIT_ASSERT_MESSAGE("No items found", sal_Int32(0) != nCount);
    for (auto i = 0; i < nCount; i++)
    {
        CPPUNIT_ASSERT_NO_THROW_MESSAGE("Unable to access element at position " + std::to_string(i),
                                        xIA->getByIndex(i));
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
