/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetconditionalentries.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XSheetConditionalEntries::testAddNew()
{
    uno::Reference<sheet::XSheetConditionalEntries> xSheetConditionalEntries(init(),
                                                                             UNO_QUERY_THROW);
    const sal_Int32 aCount = xSheetConditionalEntries->getCount();

    xSheetConditionalEntries->addNew(createCondition(4));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to add new condition", aCount + 1,
                                 xSheetConditionalEntries->getCount());
}

void XSheetConditionalEntries::testClear()
{
    uno::Reference<sheet::XSheetConditionalEntries> xSheetConditionalEntries(init(),
                                                                             UNO_QUERY_THROW);
    xSheetConditionalEntries->clear();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to clear conditions", sal_Int32(0),
                                 xSheetConditionalEntries->getCount());
}

void XSheetConditionalEntries::testRemoveByIndex()
{
    uno::Reference<sheet::XSheetConditionalEntries> xSheetConditionalEntries(init(),
                                                                             UNO_QUERY_THROW);
    const sal_Int32 aCount = xSheetConditionalEntries->getCount();

    xSheetConditionalEntries->removeByIndex(0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to remove condition", aCount - 1,
                                 xSheetConditionalEntries->getCount());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
