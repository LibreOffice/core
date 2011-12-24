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

#include <test/unoapi_test.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>


namespace ScTableSheetsObj
{

class ScXNameContainer : public UnoApiTest
{
    uno::Reference< container::XNameContainer > init();

    void testRemoveByName();

    CPPUNIT_TEST_SUITE(ScXNameContainer);
    CPPUNIT_TEST(testRemoveByName);
    CPPUNIT_TEST_SUITE_END();
};

void ScXNameContainer::testRemoveByName()
{
    rtl::OUString aSheet2(RTL_CONSTASCII_USTRINGPARAM("Sheet2"));
    uno::Reference< container::XNameContainer > xNameContainer = init();
    CPPUNIT_ASSERT(xNameContainer->hasByName(aSheet2));
    xNameContainer->removeByName(aSheet2);
    CPPUNIT_ASSERT(!xNameContainer->hasByName(aSheet2));

    bool bExceptionThrown = false;
    try
    {
        xNameContainer->removeByName(aSheet2);
    }
    catch( const container::NoSuchElementException& )
    {
        std::cout << "Exception Caught" << std::endl;
        bExceptionThrown = true;
    }

    CPPUNIT_ASSERT_MESSAGE("no exception thrown", bExceptionThrown);
}

uno::Reference< container::XNameContainer > ScXNameContainer::init()
{
    uno::Reference< lang::XComponent > xComponent;
    xComponent = loadFromDesktop(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc")));
    CPPUNIT_ASSERT(xComponent.is());
    uno::Reference< sheet::XSpreadsheetDocument> xDoc (xComponent, UNO_QUERY_THROW);
    uno::Reference< container::XNameContainer > xNameContainer ( xDoc->getSheets(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNameContainer.is());
    return xNameContainer;
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScXNameContainer);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
