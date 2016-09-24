/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnamecontainer.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest {

XNameContainer::XNameContainer(): maNameToRemove("XNameContainer")
{
}

XNameContainer::XNameContainer(const OUString& rNameToRemove):
        maNameToRemove(rNameToRemove)
{
}

void XNameContainer::testRemoveByName()
{
    uno::Reference< container::XNameContainer > xNameContainer(init(),UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNameContainer->hasByName(maNameToRemove));
    xNameContainer->removeByName(maNameToRemove);
    CPPUNIT_ASSERT(!xNameContainer->hasByName(maNameToRemove));

    bool bExceptionThrown = false;
    try
    {
        xNameContainer->removeByName(maNameToRemove);
    }
    catch( const container::NoSuchElementException& )
    {
        std::cout << "Exception Caught" << std::endl;
        bExceptionThrown = true;
    }

    CPPUNIT_ASSERT_MESSAGE("no exception thrown", bExceptionThrown);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
