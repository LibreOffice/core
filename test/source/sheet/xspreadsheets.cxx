/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xspreadsheets.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;


namespace apitest {

void XSpreadsheets::testInsertNewByName()
{
    uno::Reference< sheet::XSpreadsheets > xSpreadsheets(init(), UNO_QUERY_THROW);

    OUString newSheetName( u"SheetToInsert"_ustr );
    xSpreadsheets->insertNewByName(newSheetName, 0);

    uno::Reference< container::XNameAccess > xNameAccess(xSpreadsheets, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Sheet was not insert",
                           xNameAccess->hasByName(newSheetName));
}

void XSpreadsheets::testInsertNewByNameBadName()
{
    uno::Reference< sheet::XSpreadsheets > xSpreadsheets(init(), UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW_MESSAGE("No exception thrown",
                                 xSpreadsheets->insertNewByName(u"$%#/?\\"_ustr, 0),
                                 uno::RuntimeException);
}

void XSpreadsheets::testMoveByName()
{
    uno::Reference< sheet::XSpreadsheets > xSpreadsheets(init(), UNO_QUERY_THROW);

    // first insert one that should be moved
    xSpreadsheets->insertNewByName(u"SheetToMove"_ustr, 0);
    uno::Reference< container::XNameAccess > xNameAccess(xSpreadsheets, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Sheet to move was not insert",
                           xNameAccess->hasByName(u"SheetToMove"_ustr));

    xSpreadsheets->moveByName(u"SheetToMove"_ustr, 2);

    // get sheet from the new position
    uno::Reference< container::XIndexAccess > oIndexAccess(xSpreadsheets, UNO_QUERY_THROW);
    uno::Any aAny = oIndexAccess->getByIndex(1);
    uno::Reference< container::XNamed > xNamed;
    CPPUNIT_ASSERT(aAny >>= xNamed);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sheet was not moved",
                                 u"SheetToMove"_ustr,
                                 xNamed->getName());
}

void XSpreadsheets::testCopyByName()
{
    uno::Reference< sheet::XSpreadsheets > xSpreadsheets(init(), UNO_QUERY_THROW);

    // insert new sheet to copy
    xSpreadsheets->insertNewByName(u"SheetToCopySrc"_ustr, 0);
    uno::Reference< container::XNameAccess > xNameAccess(xSpreadsheets, UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("Sheet to copy was not insert",
                           xNameAccess->hasByName(u"SheetToCopySrc"_ustr));

    xSpreadsheets->copyByName(u"SheetToCopySrc"_ustr, u"SheetToCopyDst"_ustr, 0);
    CPPUNIT_ASSERT_MESSAGE("Sheet was not copied",
                           xNameAccess->hasByName(u"SheetToCopyDst"_ustr));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
