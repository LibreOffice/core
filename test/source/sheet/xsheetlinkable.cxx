/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetlinkable.hxx>

#include <com/sun/star/sheet/SheetLinkMode.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetLinkable::testSheetLinkable()
{
    uno::Reference< sheet::XSheetLinkable > xSheetLinkable(init(), UNO_QUERY_THROW);

    xSheetLinkable->link(getFileURL(), "Sheet1", "", "", sheet::SheetLinkMode_VALUE);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get link mode",
                                 sheet::SheetLinkMode_VALUE, xSheetLinkable->getLinkMode());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get link URL",
                                 getFileURL(), xSheetLinkable->getLinkUrl());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get sheet name",
                                 OUString("Sheet1"), xSheetLinkable->getLinkSheetName());

    xSheetLinkable->setLinkMode(sheet::SheetLinkMode_NONE);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set link mode",
                                 sheet::SheetLinkMode_NONE, xSheetLinkable->getLinkMode());

    xSheetLinkable->setLinkSheetName("Sheet2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set sheet name",
                                 OUString("Sheet2"), xSheetLinkable->getLinkSheetName());

    xSheetLinkable->setLinkUrl(getFileURL());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to set link URL",
                                 getFileURL(), xSheetLinkable->getLinkUrl());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
