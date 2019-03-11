/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnamecontainer.hxx>

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>

using namespace css;

namespace apitest
{
void XNameContainer::testInsertByName()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(!xNameContainer->hasByName(m_aName + "Insert"));
    xNameContainer->insertByName(m_aName + "Insert", m_aElement);
    CPPUNIT_ASSERT(xNameContainer->hasByName(m_aName + "Insert"));
}

void XNameContainer::testInsertByNameEmptyName()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    uno::Any aAny;
    CPPUNIT_ASSERT_THROW(xNameContainer->insertByName("", aAny), lang::IllegalArgumentException);
}

void XNameContainer::testInsertByNameInvalidElement()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    // TODO: Find a way to create an invalid element.
    // CPPUNIT_ASSERT_THROW(xNameContainer->insertByName("Dummy", nullptr),
    //                      lang::IllegalArgumentException);
}

void XNameContainer::testInsertByNameDuplicate()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(!xNameContainer->hasByName(m_aName + "Duplicate"));
    xNameContainer->insertByName(m_aName + "Duplicate", m_aElement);
    CPPUNIT_ASSERT(xNameContainer->hasByName(m_aName + "Duplicate"));

    bool bExceptionThrown = false;
    try
    {
        xNameContainer->insertByName(m_aName + "Duplicate", m_aElement);
    }
    catch (const container::ElementExistException&)
    {
        bExceptionThrown = true;
    }
    catch (const lang::IllegalArgumentException&)
    {
        bExceptionThrown = true;
    }
    CPPUNIT_ASSERT(bExceptionThrown);
}

void XNameContainer::testRemoveByName()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xNameContainer->hasByName(m_aName));
    xNameContainer->removeByName(m_aName);
    CPPUNIT_ASSERT(!xNameContainer->hasByName(m_aName));
}

void XNameContainer::testRemoveByNameEmptyName()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xNameContainer->removeByName(""), lang::IllegalArgumentException);
}

void XNameContainer::testRemoveByNameNoneExistingElement()
{
    uno::Reference<container::XNameContainer> xNameContainer(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_THROW(xNameContainer->removeByName("UnitTest"),
                         container::NoSuchElementException);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
