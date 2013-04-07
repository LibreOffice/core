/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/container/xnamecontainer.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include "cppunit/extensions/HelperMacros.h"
#include <iostream>

using namespace com::sun::star::uno;

namespace apitest {

XNameContainer::XNameContainer(): maNameToRemove(RTL_CONSTASCII_USTRINGPARAM("XNameContainer"))
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
